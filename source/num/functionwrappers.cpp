#include <iostream>
#include <sstream>
#include <cmath>
#include <cfloat>
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    Odessa::F odessa_f(Function& T, VecF& X, VecF& RHS)
    {
      size_t nVar(X.size());
      return
	[&,nVar](double t, const double* x, const double* p, double* rhs) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar;++i)
	    {
	      rhs[i]=RHS[i].evaluate();
	    }
	};
    }
    
    Odessa::DFDX odessa_dfdx(VecF& X, VecF& RHS)
    {
      size_t nVar(X.size());
      VecF DFDX;
      build_partial_derivative_matrix(X,RHS,DFDX);
      return
	[&, nVar,DFDX] (double t, const double* x, const double* p, double* dfdx)
	{
	  /* Note that it is not necessary to set the function values in X,
	     or the T value, because each call here is preceded by a call
	     to the routine returned by odessa_f() above.
	  */
	  for(size_t i(0);i<nVar*nVar;++i)
	    dfdx[i]=DFDX[i].evaluate();
	};	
    }

    void build_partial_derivative_matrix(const VecF& vars, const VecF& funcs,VecF& matrix)
    { 
      
      for(size_t i(0),varSize(vars.size());i<varSize;++i)
	for(size_t j(0),funcSize(funcs.size());j<funcSize;++j)
	  matrix.push_back(funcs[j].get_partial_derivative(vars[i]));
    }


    /*    Odessa::DFDP odessa_dfdp(EquationSystem& equ,std::vector<std::string> parameters)
    {
      size_t nVar(equ.n_ode_variables());
      size_t nPar(equ.n_parameters());
      Function& T = equ.time();
      Function* X(equ.ode_variables());
      Function* P(equ.parameters());
      Function* DFDP(equ.ode_inhomogeneity());
      return
	[nVar,nPar,&T,X,P,DFDP](double t, const double* x, const double* p, double* dfdp, int jpar)
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar;++i)
	    dfdp[size_t(jpar)*nPar+i]=DFDP[size_t(jpar)*nPar+i].evaluate();
	};
	} */  


    /*    Iterator::F iterator_f(Function& STEP, VecF& X, VecF& RHS)
    {
      size_t nVar(X.size());
      return
	[=](int step, const double* x, double* rhs) mutable
	{
	  STEP.set_value(step);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar;++i)
	    rhs[i]=RHS[i].evaluate();
	};
	}*/
    
    /*


      F system_dfdx(EquationSystem& equ)
    {
      size_t nVar(equ.n_ode_variables());
      Function* X(equ.ode_variables());
      Function* DFDX(equ.ode_jacobian());
      return
	[nVar,X,DFDX](const double* x, double* dfdx)
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar*nVar;++i)
	    dfdx[i]=DFDX[i].evaluate();
	  return true;
	};
    }
    
    
    F system_f(int(*f)(),double** xLoc, double** rhsLoc, double* tLoc, double** ploc)
    {
      return
	[xLoc,rhsLoc,f](const double* x, double* rhs)
	{*xLoc=const_cast<double*>(x);*rhsLoc=rhs;f();return true;};
    }

    F system_dfdx(int(*dfdx)(),double** xLoc, double** dfdxLoc, double* tLoc, double** ploc)
    {
      return
	[xLoc,dfdxLoc,dfdx](const double* x, double* dfdxData)
	{*xLoc=const_cast<double*>(x);*dfdxLoc=dfdxData;dfdx();return true;};
    }
    
    F system_f_dfdx(size_t nVar, int (*f)(), double** xLoc, double** rhsLoc, int (*dfdx)(), double** dfdxLoc, double* tLoc, double** ploc)
    {
      return 
	[nVar,xLoc,rhsLoc,dfdxLoc,f,dfdx](const double* x, double* rhs)
	{*xLoc=const_cast<double*>(x);*rhsLoc=rhs;*dfdxLoc=rhs+nVar;f();dfdx();return true;};
	}*/

#pragma GCC diagnostic pop

    /*
    F flow_f(size_t nVar, Odessa& odessa, double tau)
    {
      return 
	[nVar,&odessa,tau](const double* x, double* rhs)
	{
	  double* X = odessa.x();
	  if(x)
	    for(size_t i(0);i<nVar;++i)
		X[i]=x[i];
	  odessa.reset();
	  odessa.t()=0;
	  odessa.integrate(tau);
	  if(rhs)
	    for(size_t i(0);i<nVar;++i)
	      rhs[i]=X[i];
	};
    }

    F flow_f_dfdx(size_t nVar, Odessa& odessa, double tau)
    {
      return 
	[nVar,&odessa,tau](const double* x, double* rhs)
	{
	  double* X = odessa.x();
	  if(x)
	    {
	      for(size_t i(0);i<nVar;++i)
		{
		  X[i]=x[i];
		  //  std::cout<<X[i]<<", ";
		}
	      //std::cout<<std::endl;
	      for(size_t i(0);i<nVar*nVar;++i)
		X[nVar+i]=(i%(nVar+1))?0:1;
	    }
	  odessa.reset();
	  odessa.t()=0;
	  odessa.integrate(tau);
	  if(rhs)
	    for(size_t i(0);i<nVar*(nVar+1);++i)
	      rhs[i]=X[i];
	};
    }

    F poincare_f(size_t nVar, Odessa& odessa, double dt, size_t variable, double value, size_t maxSteps)
    {
      return
	[nVar,&odessa,dt,variable,value,maxSteps] (const double* x, double* rhs)
	{
	  double* X = odessa.x();
	  if(x)
	    for(size_t i(0);i<nVar;++i)
	      X[i]=x[i];
	  odessa.reset();
	  odessa.t()=0;
	  size_t step(0);
	  double oldValue(value); // makes sure that we do not detect the same intersection over and over again by starting just below value
	  double newValue(0);
	  while(++step<=maxSteps)
	    {
	      odessa.integrate(dt);
	      newValue=X[variable];
	      if(newValue>=value&&oldValue<value)
		break;
	      oldValue=newValue;
	    }
	  if(step==maxSteps)
	    {
	      std::stringstream ss;
	      ss<<"no intersection detected after "<<maxSteps<<" time steps\n";
	      throw(ss.str());
	    }
	  double oldDT(0);
	  F diff = [&odessa,variable,value,&oldDT](const double* DT, double* delta)
	    {
	      odessa.reset();
	      odessa.integrate(*DT-oldDT);
	      *delta=odessa.x()[variable]-value;
	      oldDT=*DT;
	    };
	  double DT(dt*(newValue-value)/(oldValue-newValue));
	  if(!newton(1,&DT,diff,true,1e-7))
	    {
	      std::stringstream ss;
	      ss<<"intersection detected but Newton iteration did not converge\n";
	      throw(ss.str());
	    }
	  if(rhs)
	    for(size_t i(0);i<nVar;++i)
	      rhs[i]=X[i];
	};

    }
    
    F poincare_f_dfdx(size_t nVar, Odessa& odessa, const F& vector_field, double dt,size_t variable, double value, size_t maxSteps)
    {
      return
	[nVar,&odessa,&vector_field,dt,variable,value,maxSteps] (const double* x, double* rhs)
	{
	  double* X = odessa.x();
	  if(x)
	    {
	      for(size_t i(0);i<nVar;++i)
		X[i]=x[i];
	      for(size_t i(0);i<nVar*nVar;++i)
		X[nVar+i]=(i%(nVar+1))?0:1;
	    }
	  odessa.reset();
	  odessa.t()=0;
	  size_t step(0);
	  double oldValue(value); // makes sure that we do not detect the same intersection over and over again by starting just below value
	  double newValue(0);
	  while(++step<=maxSteps)
	    {
	      odessa.integrate(dt);
	      newValue=X[variable];
	      if(newValue>=value&&oldValue<value)
		break;
	      oldValue=newValue;
	    }
	  if(step==maxSteps)
	    {
	      std::stringstream ss;
	      ss<<"no intersection detected after "<<maxSteps<<" time steps\n";
	      throw(ss.str());
	    }
	  double oldDT(0);
	  F diff = [&odessa,variable,value,&oldDT](const double* DT, double* delta)
	    {
	      odessa.reset();
	      odessa.integrate(*DT-oldDT);
	      *delta=odessa.x()[variable]-value;
	      oldDT=*DT;
	    };
	  double DT(dt*(newValue-value)/(oldValue-newValue));
	  if(!newton(1,&DT,diff,true,1e-7))
	    {
	      std::stringstream ss;
	      ss<<"intersection detected but Newton iteration did not converge\n";
	      throw(ss.str());
	    }
	  if(rhs)
	    {
	      double* XDOT=new double[nVar];	      
	      vector_field(X,XDOT);
	      for(size_t i(0);i<nVar*(nVar+1);++i)
		rhs[i]=X[i];
	      for(size_t i(0);i<nVar*nVar;++i)
		rhs[i+nVar]-=XDOT[i%nVar]/XDOT[variable]*X[nVar+variable+nVar*(i/nVar)];
	      delete[] XDOT;
	      for(size_t i(0);i<nVar;++i)
		rhs[nVar+i+variable*nVar]=0;
	    }
	};
	}*/

  } /* end namespace num */
} /* end namespace scigma */

