#include <cmath>
#include <limits>
#include <list>
#include <iostream>
#include "sweeping.hpp"

typedef scigma::dat::AbstractWave<double> Wave;
typedef scigma::dat::AbstractWave<GLint> IWave;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"

namespace scigma
{
  namespace num
  {
    Odessa::F create_extended_system(size_t nVar, double& g,
				     std::vector<double>& grad_g,
				     double& f_grad_g,
				     double** f,
				     double* fabs,
				     F_t f_t
				     )
    {
      auto f_extended([nVar,&g,&grad_g,&f_grad_g,f,fabs,f_t]
		      (double t, const double* x, const double* p, double* rhs) mutable
		      {
			p=NULL;
			
			/* determine value of constraint */
			g=0;
			for(size_t i(0);i<nVar;++i)
			  {
			    grad_g[i]=x[i]-x[i+nVar];
			    grad_g[i+nVar]=-grad_g[i];
			    g+=grad_g[i]*grad_g[i];
			  }
			g=std::sqrt(g);
			
			/* evaluate system for individual points */
			f_t(t,x,rhs);
			f_t(t,x+nVar,rhs+nVar);

			/* scaling factor for arclength integration */
			fabs[0]=fabs[1]=0;
			for(size_t i(0);i<nVar/*2*nVar*/;++i)
			  {
			    fabs[0]+=rhs[i]*rhs[i];
			    fabs[0]+=rhs[i+nVar]*rhs[i+nVar];
			  }
			fabs[0]=std::sqrt(fabs[0]);
			fabs[1]=std::sqrt(fabs[1]);
			
			/* determine auxiliary variables grad_g and f_grad_g */
			f_grad_g=0;
			for(size_t i(0);i<nVar*2;++i)
			  {
			    grad_g[i]/=g;
			    f_grad_g+=rhs[i]*grad_g[i];
			  }
			
			/* evaluate extended system */
			for(size_t i(0);i<nVar;++i)
			  {
			    rhs[i]-=0.5*f_grad_g*grad_g[i];
			    rhs[i]/=fabs[0];
			    rhs[i+nVar]-=0.5*f_grad_g*grad_g[i+nVar];
			    rhs[i+nVar]/=/*fabs[0]*/fabs[0];			    
			  }

			*f=rhs;
		      }
		      );
      return f_extended;
    }

    Odessa::DFDX create_extended_jacobian(size_t nVar, double& g,
					  std::vector<double>& grad_g,
					  double& f_grad_g,
					  double** f,
					  double* fabs,
					  std::vector<double>& dfdx1,
					  std::vector<double>& dfdx2,
					  std::vector<double>& grad_g_jac,
					  F_t dfdx_t)
    {
      auto jac_extended([nVar, &g, &grad_g, &f_grad_g, f, fabs, &dfdx1, &dfdx2, &grad_g_jac, dfdx_t]
			(double t, const double* x, const double* p, double* dfdx) mutable
			{
			  p=NULL;
			  
			  /* evaluate Jacobian for first and second point */ 
			  dfdx_t(t,x,&dfdx1[0]);
			  dfdx_t(t,x+nVar,&dfdx2[0]);
			  
			  /* adjust for arclength scaling and move
			     values to the correct locations */
			  for(size_t k(0);k<nVar;++k)
			    {
			      for(size_t l(0);l<nVar;++l)
				{
				  dfdx[k*2*nVar+l]=(dfdx1[k*nVar+l]-(*f)[k]*(*f)[l]*dfdx1[(nVar+1)*l])/fabs[0];
				  dfdx[(k+nVar)*2*nVar+(l+nVar)]=(dfdx2[k*nVar+l]-(*f)[k+nVar]*(*f)[l+nVar]*dfdx2[(nVar+1)*l])/fabs[0];
				  dfdx[k*2*nVar+(l+nVar)]=-(*f)[k]*(*f)[l+nVar]*dfdx2[(nVar+1)*l]/fabs[0];
				  dfdx[(k+nVar)*2*nVar+l]=-(*f)[k+nVar]*(*f)[l]*dfdx1[(nVar+1)*l]/fabs[0];
				}
			    }
			  			  
			  /* compute grad_g*jac */
			  for(size_t l(0);l<nVar;++l)
			    {
			      grad_g_jac[l]=0;
			      grad_g_jac[l+nVar]=0;
			      for(size_t i(0);i<nVar;++i)
				{
				  grad_g_jac[l]+=grad_g[i]*dfdx1[l*nVar+i];
				  grad_g_jac[l+nVar]+=grad_g[i+nVar]*dfdx2[l*nVar+i];
				}
			    }
			  
			  /* first, third and fourth correction term */
			  for(size_t k(0);k<2*nVar;++k)
			    for(size_t l(0);l<2*nVar;++l)
			      dfdx[k*2*nVar+l]+=-0.5*grad_g[k]*((grad_g_jac[l]+((*f)[l]-(*f)[(l+nVar)%(2*nVar)])/g))
				+grad_g[k]*grad_g[l]*(f_grad_g/g);
			  

			  /* second correction term */
			  for(size_t k(0);k<nVar;++k)
			    {
			      dfdx[k*2*nVar+k]+=f_grad_g/g;
			      dfdx[(k+nVar)*2*nVar+k+nVar]+=f_grad_g/g;
			      dfdx[k*2*nVar+k+nVar]-=f_grad_g/g;
			      dfdx[(k+nVar)*2*nVar+k]-=f_grad_g/g;
			    }

			}
			);
      return jac_extended;
    }


    void build_mesh(std::string identifier, Log* log, size_t nVar,
		    Odessa::F f_extended, Odessa::DFDX jac_extended,
		    size_t nFunc, F_t func_t, size_t nSteps, Mesh* mesh, Blob* blob)
    {
      log->push(identifier);
      func_t=NULL;

      /* throwaway function to compute distances between points */
      auto dFunc([nVar](const double* x1, const double* x2)
		 {
		   double dSquared(0);
		   for(size_t i(0);i<nVar;++i)
		     dSquared+=(x1[i]-x2[i])*(x1[i]-x2[i]);
		   return std::sqrt(dSquared);
		 });
      
      
      double dt(blob->get_double("dt"));
      double dsmax(blob->get_double("Manifolds.arc"));
      double ds(blob->get_double("Manifolds.eps"));
      double fudge(blob->get_double("Manifolds.fudge"));
      ds = std::sqrt(ds*ds);
      bool stiff(blob->get_string("odessa.type")=="stiff"?true:false);
      double aTol(blob->get_double("odessa.atol"));
      double rTol(blob->get_double("odessa.rtol"));
      size_t maxIter((size_t(blob->get_int("odessa.mxiter"))));

      /* create set of initial values from mesh;   
	 in the triangle data, for each value of each mesh point, 
	 there are actually four values (because of the extra 
	 information needed for normals) - this must be taken into
	 account when extracting the data here
      */
      
      const Wave& triangles(mesh->triangle_data());
      const IWave& indices(mesh->iso_indices());
      size_t MAX(std::numeric_limits<size_t>::max());
      
      size_t maxLayer(mesh->available_iso_layer(MAX,MAX,MAX));
      //      std::cerr<<"maxLayer"<<maxLayer<<std::endl;
      size_t start(mesh->max_for_iso_layer(maxLayer-1));
      /* start point of each layer is stored twice in the mesh, 
	 therefore subtract 1 from end! */
      size_t end(mesh->max_for_iso_layer(maxLayer)-1);

      size_t nVarying(1+nVar+nFunc);
      size_t nInitial(end-start);
      
      std::vector<double> initial(nVarying*(end-start));
      /*      std::cerr<<"ninitial "<<nInitial<<std::endl;
      std::cerr<<"start "<<start<<std::endl;
      std::cerr<<"end "<<end<<std::endl;*/
      for(size_t i(0);i<nInitial*nVarying;++i)
	{
	  initial[i]=triangles.data()[(size_t(indices.data()[start+i/nVarying])*nVarying+i%nVarying)*Mesh::NVALS_PER_DIM];
	  //	  std::cerr<<initial[i]<<", ";
	}
      //      std::cerr<<std::endl;

      /* create the initial list of wrappers for the Odessa integration
	 routine: always use two neighbouring points as initial condition */
      std::list<Odessa*> odessaList;

      ds = 0;
      
      for(size_t i(0);i<nInitial;++i)
	{
	  Odessa* odessa(new Odessa(nVar*2,f_extended,jac_extended,0,NULL,
				    stiff,aTol,rTol,maxIter,false));
	  odessaList.push_back(odessa);
	  odessa->t()=initial[i*nVarying];
	  //  std::cout<<0<<"\t";
	  for(size_t j(0);j<nVar;++j)
	    {
	      odessa->x()[j]=initial[i*nVarying+1+j]; //first point
	      odessa->x()[j+nVar]=initial[((i+1)%nInitial)*nVarying+1+j]; //second point
	      //  std::cout<<initial[i*nVarying+1+j]<<"\t";
	    }
	  /*	  for(size_t j(0);j<nVar*2;++j)
	    std::cout<<0<<"\t";
	    std::cout<<std::endl;*/
	  double newds(dFunc(&initial[i*nVarying],&initial[((i+1)%nInitial)*nVarying]));
	  if(newds>ds)
	    ds=newds;
	}
      ds=ds<dsmax?ds:dsmax;
      //      std::cerr<<"ds"<<ds<<std::endl;
      double DS(dt<0?-ds:ds);
      
      /* for each step, do the integration up to ds, then add and remove
	 points according to these two rules
	 1. If the two outer points of neigbouring pairs are closer 
	    than ds, remove the inner points and form a new pair from 
	    the outer two points.
	 2. If the two inner points of neighbouring pairs are further
	    apart than ds, make them into a new pair.
      */
      
      for(size_t i(0);i<nSteps;++i)
	{
	  for(auto iter(odessaList.begin());iter!=odessaList.end();++iter)
	    {
	      try
		{
		  (*iter)->integrate(DS);
		}
	      catch(std::string error)
		{
		  
		}
	    }

	  auto o1(odessaList.begin());
	  auto o2(odessaList.end());
	  //	  auto o3(odessaList.end());

	  std::cerr<<"layer "<<i<<std::endl;

	  //size_t c(0);
	  while(o1!=odessaList.end())
	    {
	      o2=++o1;--o1;
	      if(o2==odessaList.end())
		{
		  o2=odessaList.begin();
		  //	  o3=++odessaList.begin();
		}
	      /*      else
		{
		  o3=++o2;--o2;
		  if(o3==odessaList.end())
		    o3=odessaList.begin();
		    }*/

	      /*	      double d1(dFunc((*o1)->x(),(*o3)->x()));
			      double d2(dFunc((*o1)->x(),(*o2)->x()));*/

	      double d1(dFunc((*o1)->x(),(*o2)->x()+nVar));
	      double d2(dFunc((*o1)->x()+nVar,(*o2)->x()));

	      
	      /*    std::cerr<<"count "<<c++<<", d1 "<<d1<<", d2 "<<d2<<";";

	      std::cout<<c<<"\t";
	      for(size_t j(0);j<3;++j)
	      std::cout<<(*o1)->x()[j]<<"\t";*/
	      
	      if(d1<ds) // remove point
		{
		  //		  std::cerr<<"-;";
		  if(odessaList.size()>3)
		    {
		      /*  for(size_t j(0);j<3;++j)
			  std::cout<<(*o2)->x()[j]<<"\t";*/
		      delete (*o2);
		      o2=odessaList.erase(o2);
		      if(odessaList.end()==o2)
			o2=odessaList.begin();
		      for(size_t j(0);j<nVar;++j)
			(*o2)->x()[j]=(*o1)->x()[j+nVar];
		    }
		}
	      /*	      else
		for(size_t j(0);j<3;++j)
		std::cout<<0<<"\t";*/

	      if(d2>ds) // insert point
		{
		  //		  std::cerr<<"+;";
		  Odessa* o(new Odessa(nVar*2,f_extended,jac_extended,0,NULL,
				       stiff,aTol,rTol,maxIter,false));
		  for(size_t j(0);j<nVar;++j)
		    {
		      /*		      o->t()=(*o1)->t();
		      o->x()[j]=(*o1)->x()[j+nVar];
		      o->x()[j+nVar]=(*o1)->x()[j];*/
		      o->t()=((*o1)->t()+(*o2)->t())/2;
		      o->x()[j]=(*o1)->x()[j+nVar];
		      o->x()[j+nVar]=(*o2)->x()[j];

		    }
		  /*		  for(size_t j(0);j<3;++j)
				  std::cout<<o->x()[j]<<"\t";*/
			odessaList.insert(o2,o);
		}
	      /*	      else
		for(size_t j(0);j<3;++j)
		std::cout<<0<<"\t";*/
	      ++o1;
	      //	      std::cerr<<std::endl;
	      //std::cout<<std::endl;
	    }
	  std::vector<double> positions;
	  for(auto iter(odessaList.begin());iter!=odessaList.end();)
	    {
	      Odessa* o(*iter);
	      auto next(++iter);
	      if(next==odessaList.end())
		next=odessaList.begin();
	      //	      Odessa* p(*next);
	     
	      positions.push_back(o->t());
	      for(size_t j(0);j<nVar;++j)
		{
		  positions.push_back(o->x()[j]);
		  //  o->x()[j+nVar]=p->x()[j];
		  // o->reset();
		  //std::cerr<<o->x()[0]<<"\t"<<o->x()[1]<<"\t"<<o->x()[2]<<std::endl;
		  //		  std::cerr<<1<<"\t"<<i<<o->x()[3]<<"\t"<<o->x()[4]<<"\t"<<o->x()[5]<<std::endl;
		}
	    }
	  mesh->add_strip(positions);
	  if(ds>dsmax)
	    {
	      DS=dt<0?-dsmax:dsmax;
	      ds=dsmax;
	    }
	  else
	    {
	      ds*=fudge;
	      DS*=fudge;
	    }
	  //	  std::cerr<<i<<","<<mesh->iso_indices().size()<<" fudge "<<ds<<","<<DS<<std::endl;
	  
	}
    }


    void build_mesh_2(std::string identifier, Log* log, size_t nVar,
		     Odessa::F f_extended, Odessa::DFDX jac_extended,
		     size_t nFunc, F_t func_t, size_t nSteps, Mesh* mesh, Blob* blob)
    {
      log->push(identifier);
      func_t=NULL;

      /* throwaway function to compute distances between points */
      auto dFunc([nVar](const double* x1, const double* x2)
		 {
		   double dSquared(0);
		   for(size_t i(0);i<nVar;++i)
		     dSquared+=(x1[i]-x2[i])*(x1[i]-x2[i]);
		   return std::sqrt(dSquared);
		 });
      
      
      double dt(blob->get_double("dt"));
      double dsmax(blob->get_double("Manifolds.arc"));
      double ds(blob->get_double("Manifolds.eps"));
      double fudge(blob->get_double("Manifolds.fudge"));
      ds = std::sqrt(ds*ds);
      bool stiff(blob->get_string("odessa.type")=="stiff"?true:false);
      double aTol(blob->get_double("odessa.atol"));
      double rTol(blob->get_double("odessa.rtol"));
      size_t maxIter((size_t(blob->get_int("odessa.mxiter"))));

      /* create set of initial values from mesh;   
	 in the triangle data, for each value of each mesh point, 
	 there are actually four values (because of the extra 
	 information needed for normals) - this must be taken into
	 account when extracting the data here
      */
      
      const Wave& triangles(mesh->triangle_data());
      const IWave& indices(mesh->iso_indices());
      size_t MAX(std::numeric_limits<size_t>::max());
      
      size_t maxLayer(mesh->available_iso_layer(MAX,MAX,MAX));
      //      std::cerr<<"maxLayer"<<maxLayer<<std::endl;
      size_t start(mesh->max_for_iso_layer(maxLayer-1));
      /* start point of each layer is stored twice in the mesh, 
	 therefore subtract 1 from end! */
      size_t end(mesh->max_for_iso_layer(maxLayer)-1);

      size_t nVarying(1+nVar+nFunc);
      size_t nInitial(end-start);
      
      std::vector<double> initial(nVarying*(end-start));
      /*      std::cerr<<"ninitial "<<nInitial<<std::endl;
      std::cerr<<"start "<<start<<std::endl;
      std::cerr<<"end "<<end<<std::endl;*/
      for(size_t i(0);i<nInitial*nVarying;++i)
	{
	  initial[i]=triangles.data()[(size_t(indices.data()[start+i/nVarying])*nVarying+i%nVarying)*Mesh::NVALS_PER_DIM];
	  //	  std::cerr<<initial[i]<<", ";
	}
      //      std::cerr<<std::endl;

      /* create the initial list of wrappers for the Odessa integration
	 routine: always use two neighbouring points as initial condition */
      std::list<Odessa*> odessaList;

      ds = 0;
      
      for(size_t i(0);i<nInitial;++i)
	{
	  Odessa* odessa(new Odessa(nVar,f_extended,jac_extended,0,NULL,
				    stiff,aTol,rTol,maxIter,false));
	  odessaList.push_back(odessa);
	  odessa->t()=initial[i*nVarying];
	  //  std::cout<<0<<"\t";
	  for(size_t j(0);j<nVar;++j)
	      odessa->x()[j]=initial[i*nVarying+1+j]; //first point
	  /*	  for(size_t j(0);j<nVar*2;++j)
	    std::cout<<0<<"\t";
	    std::cout<<std::endl;*/
	  double newds(dFunc(&initial[i*nVarying],&initial[((i+1)%nInitial)*nVarying]));
	  if(newds>ds)
	    ds=newds;
	}
      ds=ds<dsmax?ds:dsmax;
      //      std::cerr<<"ds"<<ds<<std::endl;
      double DS(dt<0?-ds:ds);
      
      /* for each step, do the integration up to ds, then add and remove
	 points according to these two rules
	 1. If the two outer points of neigbouring pairs are closer 
	    than ds, remove the inner points and form a new pair from 
	    the outer two points.
	 2. If the two inner points of neighbouring pairs are further
	    apart than ds, make them into a new pair.
      */
      
      for(size_t i(0);i<nSteps;++i)
	{
	  for(auto iter(odessaList.begin());iter!=odessaList.end();++iter)
	    {
	      try
		{
		  (*iter)->integrate(DS/2);
		}
	      catch(std::string error)
		{
		  
		}
	    }

	  auto o1(odessaList.begin());
	  auto o2(odessaList.end());
	  auto o3(odessaList.end());

	  std::cerr<<"layer "<<i<<std::endl;

	  //  size_t c(0);
	  while(o1!=odessaList.end())
	    {
	      o2=++o1;--o1;
	      if(o2==odessaList.end())
		{
		  o2=odessaList.begin();
		  o3=++odessaList.begin();
		}
	      else
		{
		  o3=++o2;--o2;
		  if(o3==odessaList.end())
		    o3=odessaList.begin();
		}

	      double d1(dFunc((*o1)->x(),(*o3)->x()));
	      double d2(dFunc((*o1)->x(),(*o2)->x()));

	      /*  std::cerr<<"count "<<c++<<", d1 "<<d1<<", d2 "<<d2<<";";

	      std::cout<<c<<"\t";
	      for(size_t j(0);j<3;++j)
	      std::cout<<(*o1)->x()[j]<<"\t";*/
	      
	      if(d1<ds) // remove point
		{
		  //		  std::cerr<<"-;";
		  if(odessaList.size()>3)
		    {
		      /*  for(size_t j(0);j<3;++j)
			  std::cout<<(*o2)->x()[j]<<"\t";*/
		      delete (*o2);
		      o2=odessaList.erase(o2);
		      if(odessaList.end()==o2)
			o2=odessaList.begin();
		    }
		}
	      /*	      else
		for(size_t j(0);j<3;++j)
		std::cout<<0<<"\t";*/

	      if(d2>ds) // insert point
		{
		  //		  std::cerr<<"+;";
		  Odessa* o(new Odessa(nVar,f_extended,jac_extended,0,NULL,
				       stiff,aTol,rTol,maxIter,false));
		  o->t()=((*o1)->t()+(*o2)->t())/2;
		  for(size_t j(0);j<nVar;++j)
		    o->x()[j]=((*o1)->x()[j]+(*o2)->x()[j])/2;		    
		  odessaList.insert(o2,o);
		}
	      /*	      else
		for(size_t j(0);j<3;++j)
		std::cout<<0<<"\t";*/
	      ++o1;
	      //	      std::cerr<<std::endl;
	      //std::cout<<std::endl;
	    }
	  std::vector<double> positions;
	  for(auto iter(odessaList.begin());iter!=odessaList.end();)
	    {
	      Odessa* o(*iter);
	      auto next(++iter);
	      if(next==odessaList.end())
		next=odessaList.begin();
	      //	      Odessa* p(*next);
	     
	      positions.push_back(o->t());
	      for(size_t j(0);j<nVar;++j)
		{
		  positions.push_back(o->x()[j]);
		  //  o->x()[j+nVar]=p->x()[j];
		  // o->reset();
		  //std::cerr<<o->x()[0]<<"\t"<<o->x()[1]<<"\t"<<o->x()[2]<<std::endl;
		  //		  std::cerr<<1<<"\t"<<i<<o->x()[3]<<"\t"<<o->x()[4]<<"\t"<<o->x()[5]<<std::endl;
		}
	    }
	  mesh->add_strip(positions);
	  if(ds>dsmax)
	    {
	      DS=dt<0?-dsmax:dsmax;
	      ds=dsmax;
	    }
	  else
	    {
	      ds*=fudge;
	      DS*=fudge;
	    }
	  //std::cerr<<i<<","<<mesh->iso_indices().size()<<" fudge "<<ds<<","<<DS<<std::endl;
	  
	}
    }

    
    Task* create_sweeping_task(std::string identifier, EquationSystem* eqsys,
			       Log* log, size_t nSteps, Mesh* mesh, Blob* blob)
    {
      
      size_t nVar(eqsys->n_variables());
      
      auto runFunction 
	([=]() mutable
	 {
	   /* variable holding the constraint */
	   //double g(0);
	   /* vector holding the gradient of the constraint */
	   //std::vector<double> grad_g(nVar*2);
	   /* vector holding the Jacobians of the individual points */
	   std::vector<double> dfdx1(nVar*nVar);//, dfdx2(nVar*nVar);
	   /* vector holding grad_g * jac */
	   //std::vector<double> grad_g_jac(nVar*2);
	   /* variable holding the scalar product <f_extended,grad_g> */
	   //double f_grad_g(0);
	   /* pointer to the current value of f */
	   double* f(NULL);
	   /* array holding the length of f at both points*/
	   double fabs[2];
	   double* ff(&fabs[0]);
	   
	   /*	   auto f_extended(create_extended_system(nVar,g,grad_g,f_grad_g,&f,fabs,eqsys->f_t()));
	   auto jac_extended(create_extended_jacobian
	   (nVar,g,grad_g,f_grad_g,&f,fabs,dfdx1,dfdx2,grad_g_jac,eqsys->dfdx_t()));*/



	   auto f_t(eqsys->f_t());
	   auto dfdx_t(eqsys->dfdx_t());
	   auto f_extended([nVar,ff,&f,f_t]
			   (double t, const double* x, const double* p, double* rhs) mutable
			   {
			     /* get rid of warnings about unused parameters */
			     p=NULL;
			     
			     /* determine value of constraint */
			     f_t(t,x,rhs);
			     
			/* scaling factor for arclength integration */
			     ff[0]=0;
			     for(size_t i(0);i<nVar/*2*nVar*/;++i)
			       ff[0]+=rhs[i]*rhs[i];
			     *ff=std::sqrt(ff[0]);
			     
			     for(size_t i(0);i<nVar;++i)
			       {
				 rhs[i]/=ff[0];
			       }
			     f=rhs;
			   }
			   );
	   auto jac_extended([nVar, &f, ff,&dfdx1, dfdx_t]
			     (double t, const double* x, const double* p, double* dfdx) mutable
			     {
			       /* get rid of warnings about unused parameters */
			       p=NULL;
			       
			       /* values of g, grad_g, f_grad_g and f are carried over from f_extended! */
			       
			       /* evaluate Jacobian for first and second point */ 
			       dfdx_t(t,x,&dfdx1[0]);
			       
			       /* adjust for arclength scaling and move
				  values to the correct locations */
			       for(size_t k(0);k<nVar;++k)
				 for(size_t l(0);l<nVar;++l)
				   {
				     dfdx[k*nVar+l]=(dfdx1[k*nVar+l]-(f)[k]*(f)[l]*dfdx1[(nVar+1)*l])/ff[0];
				   }
			}
			);


	   
	   build_mesh_2(identifier, log, nVar, f_extended, jac_extended,
		      eqsys->n_functions(), eqsys->func_t(), nSteps, mesh, blob);

	 });
      
      return new Task(runFunction);
    }

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
    
  } /* end namespace num */
} /* end namespace scigma */
