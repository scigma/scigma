#include "guessing.hpp"

#include <iostream>

namespace scigma
{
  namespace num
  {

    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, Wave* varyingWave,
			       Wave* evWave, double tol, size_t nPeriod, size_t showAllIterates, long secvar)
    {
      Task* task = new Task
	([=]() mutable
	 {
	   varyingWave->lock();
	   
	   size_t nVar(stepper->n_variables());
	   size_t nFunc(stepper->n_functions());	   
	   double t(stepper->t());
	   
	   auto f = [nVar,nFunc,stepper,nPeriod,tol,t](const double* x, double* rhs)
	   {
	     stepper->reset(t,x);
	     stepper->advance(nPeriod);
	     for(size_t i(0);i<nVar;++i)
	       rhs[i]=stepper->x()[i]-x[i];
	     for(size_t i(0);i<nVar*nVar;++i)
	       rhs[i+nVar]=stepper->jac()[i];	     
	     // subtract 1 from the diagonal of the Jacobian
	     for(size_t i(0);i<nVar;++i)
	       rhs[nVar+i*(nVar+1)]-=1.0;
	   };

	   double* x(new double[nVar]);
	   for(size_t i(0);i<nVar;++i)
	     x[i]=varyingWave->data()[i+1];
	   varyingWave->pop_back(1+nVar+nFunc);
	   bool success(false);
	   try
	     {
	       success=newton(int(nVar),x,f,false,tol);
	     }
	   catch(std::string error)
	     {
	       success=false;
	       log->push<Log::ERROR>(error);
	     }
	   if(!success)
	     {
	       log->push<Log::ERROR>("Newton iteration did not converge\n");
	     }
	   else
	     {
	       // get eigenvalue and eigenvector info
	       VecD evals,evecs;
	       VecSZ types;
	       double* jac(new double[nVar*nVar]);
	       for(size_t i(0);i<nVar*nVar;++i)
		 jac[i]=stepper->jac()[i];
	       if(secvar>=0) // in Poincare mode, restore the artificial zero-multiplier to 1
		 jac[size_t(secvar)*(nVar+1)]=1;
	       floquet(nVar,jac,evals,evecs,types);
	       evWave->push_back(&evals[0],nVar*2);
	       evWave->push_back(&evecs[0],nVar*nVar);
	       for(size_t i(0);i<types.size();++i)
		 evWave->push_back(double(types[i]));
	       delete[] jac;

	       size_t factor(showAllIterates?nPeriod:1);
	       stepper->reset(t,x);
	       for(size_t j(0);j<factor;++j)
		 {
		   try
		     {
		       stepper->advance(nPeriod/factor);
		     }
		   catch(std::string error)
		     {
		       log->push<Log::ERROR>(error);
		       success=false;
		       break;
		     }
		   varyingWave->push_back(stepper->t());
		   varyingWave->push_back(stepper->x(),nVar);
		   varyingWave->push_back(stepper->func(),nFunc);
		 }
	     }
	   delete[] x;

	   if(success)
	     log->push<Log::SUCCESS>(identifier);
	   else
	     log->push<Log::FAIL>(identifier);

	   varyingWave->unlock();
	 });
      return task;

    }  
					      
    
    Task* create_guessing_task(std::string identifier, Log* log, EquationSystem* eqsys,
			       Wave* varyingWave, Wave* evWave, double tol)
    {

      size_t nVar(eqsys->n_variables());
      size_t nFunc(eqsys->n_functions());
      
      auto f_(eqsys->f());
      auto dfdx_(eqsys->dfdx());

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
      Newton::F f = eqsys->dfdx()?
	Newton::F([f_,dfdx_,nVar](const double* x, double* rhs)
		  {
		    f_(x,rhs);
		    dfdx_(x,rhs+nVar);
		  }):
	Newton::F(f_);
            
      auto ff(eqsys->func());

      bool genJac = eqsys->dfdx()?false:true;
      
      Task* task = new Task
	([nVar,nFunc,f,ff,varyingWave,evWave,tol,genJac,identifier,log]() mutable
	 {
	   varyingWave->lock();
	   double* x(new double[nVar]);
	   double* funcs(new double[nFunc]);
	   double t(varyingWave->data()[0]);
	   for(size_t i(0);i<nVar;++i)
	     x[i]=varyingWave->data()[i+1];
	   varyingWave->pop_back(1+nVar+nFunc);
	   bool success(false);
	   
	   try
	     {
	       success=newton(int(nVar),x,f,genJac,tol);
	     }
	   catch(std::string error)
	     {
	       success=false;
	       log->push<Log::ERROR>(error);
	     }
	   if(!success)
	     log->push<Log::ERROR>("Newton iteration did not converge\n");
	   else
	     {
	       // get eigenvalue and eigenvector info
	       double* rhs(new double[nVar*(nVar+1)]);
	       f(x,rhs);
	       double* jac=rhs+nVar;
	       VecD evals,evecs;
	       VecSZ types;
	       eigen(nVar,jac,evals,evecs,types);
	       evWave->push_back(&evals[0],nVar*2);
	       evWave->push_back(&evecs[0],nVar*nVar);
	       for(size_t i(0);i<types.size();++i)
		 evWave->push_back(double(types[i]));
	       /*	       for(size_t i(0),size(size_t(evWave->data_max()));i<size;++i)
			       std::cout<<evWave->data()[i]<<std::endl;*/
	       delete[] rhs;
	     }
	   varyingWave->push_back(t);
	   varyingWave->push_back(x,nVar);
	   if(nFunc)
	     {
	       ff(x,funcs);
	       varyingWave->push_back(funcs,nFunc);
	     }
	   
	   delete[] x;
	   delete[] funcs;
	   
	   if(success)
	     log->push<Log::SUCCESS>(identifier);
	   else
	     log->push<Log::FAIL>(identifier);

	   varyingWave->unlock();
	   });
      
#pragma clang diagnostic pop

      return task;
      
    }
    
  } /* end namespace num */
} /* end namespace scigma */
