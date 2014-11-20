#include "definitions.h"
#include "util.h"
#include "functionwrappers.h"
#include "internalmapstepper.h"
#include "internalodessastepper.h"
#include "internalpoincarestepper.h"
#include "externalodessastepper.h"
#include "externalpoincarestepper.h"
#include "newton.h"
#include "eigen.h"
#include <iostream>

extern "C" int ESCAPE_COUNT;

namespace scigma
{
  namespace num
  {

    const char* APPLICATION_SUCCESS_PREFIX="__succ__";
    const char* APPLICATION_FAIL_PREFIX="__fail__";
    const char* APPLICATION_ERROR_PREFIX="__error_";
    
    
    void run_task(void* taskPtr)
    {
      Task* task(reinterpret_cast<Task*>(taskPtr));
      (*task)();
      delete task;
    }
    
    Stepper* create_map_stepper(const EquationSystem& eqsys, bool computeJacobian)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  return new InternalMapStepper(computeJacobian,tFunc,xFuncs,rhsFuncs,funcFuncs);
	}
      return NULL;
    }
    
    Stepper* create_ode_stepper(const EquationSystem& eqsys, bool computeJacobian,double dt,
				bool extJac,bool stiff, double aTol, double rTol, size_t maxIter)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  return new InternalOdessaStepper(computeJacobian,dt,tFunc,xFuncs,rhsFuncs,funcFuncs,stiff,aTol,rTol,maxIter);
	}
      else
	{
#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"
	  F_t f_(eqsys.f_t());
	  Odessa::F f([f_](double t, const double* x, const double* p, double* rhs)
	    {
	      f_(t,x,rhs);
	    });
	  F_t dfdx_(eqsys.dfdx_t());
	  Odessa::DFDX dfdx([dfdx_](double t, const double* x, const double* p, double* jac)
	    {
	      dfdx_(t,x,jac);
	    });
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
	  double t(eqsys.time());
	  const double* x(eqsys.variable_values());
	  size_t nVar(eqsys.n_variables());
	  size_t nFunc(eqsys.n_functions());
	  F_t fFunc(eqsys.func_t());
	  return new ExternalOdessaStepper(computeJacobian,dt,nVar,nFunc,f,extJac?dfdx:NULL,fFunc,t,x,stiff,aTol,rTol,maxIter);
	}
    }

    Stepper* create_poincare_stepper(const EquationSystem& eqsys, bool computeJacobian,double maxtime, double dt,
				     int secvar, int secdir, double secval, double tol,
				     bool extJac, bool stiff, double aTol, double rTol, size_t maxIter)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  return new InternalPoincareStepper(computeJacobian,maxtime,dt,secvar,secdir,secval,tol,
					     tFunc,xFuncs,rhsFuncs,funcFuncs,
					     stiff,aTol,rTol,maxIter);

	}
      else
	{
#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"
	  F_t f_(eqsys.f_t());
	  Odessa::F f([f_](double t, const double* x, const double* p, double* rhs)
		      {
	      f_(t,x,rhs);
		      });
	  F_t dfdx_(eqsys.dfdx_t());
	  Odessa::DFDX dfdx([dfdx_](double t, const double* x, const double* p, double* jac)
			    {
			      dfdx_(t,x,jac);
			    });
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
	  double t(eqsys.time());
	  const double* x(eqsys.variable_values());
	  size_t nVar(eqsys.n_variables());
	  size_t nFunc(eqsys.n_functions());
	  F_t fFunc(eqsys.func_t());
	  return new ExternalPoincareStepper(computeJacobian,maxtime,dt,secvar,secdir,secval,tol,nVar,nFunc,f,extJac?dfdx:NULL,fFunc,t,x,stiff,aTol,rTol,maxIter);
	}
      return NULL;
    }
    
    Task* create_stepping_task(std::string identifier, Log* log, Stepper* stepper, size_t nSteps, size_t nPeriod, size_t showAllIterates, dat::Wave* wave)
    {
      Task* task = new Task
	([=]() mutable
	 {
	   int escapeCount(ESCAPE_COUNT);
	   std::string message;
	   size_t i;
	   size_t factor(showAllIterates?nPeriod:1);
	   for(i=0;i<nSteps*factor;++i)
	     {
	       try
		 {
		   stepper->advance(nPeriod/factor);
		 }
	       catch(std::string error)
		 {
		   message=APPLICATION_ERROR_PREFIX;
		   message+=error;
		   log->push(message);
		   break;
		 }
	       wave->append(stepper->t());
	       for(size_t j(0);j<stepper->nVar;++j)
		 wave->append(stepper->x(j));
	       for(size_t j(0);j<stepper->nFunc;++j)
		 wave->append(stepper->func(j));
	       
	       if(escapeCount!=ESCAPE_COUNT)
		 break;
	     }
	   if(i>0)
	     {
	       message=APPLICATION_SUCCESS_PREFIX;
	       message+=identifier;
	     }
	   else
	     {
	       message=APPLICATION_FAIL_PREFIX;
	       message+=identifier;
	     }
	   log->push(message);
	   delete stepper;
	 });
      return task;
    }

    Newton::F* create_map_newton_function(const EquationSystem& eqsys, size_t(nPeriod), bool extJac)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  size_t nVar(xFuncs.size());
	  
	  VecF jacobian;
	  build_partial_derivative_matrix(xFuncs,rhsFuncs,jacobian);
	  return new Newton::F
	    ([=](const double* x, double* rhs) mutable
	     {
	       /*	       VecD work1(nVar*nVar);VecD work2(nVar*nVar);
	       double* result(nPeriod==1?(rhs+nVar):&work1[0]);
	       double* oldJac(&work2[0]);
	       double* newJac(rhs+nVar);*/
	       
	       double t0(tFunc.evaluate());

	       for(size_t n(0);n<nPeriod;++n)
		 {
		   tFunc.set_value(t0+n);
		   for(size_t i(0);i<nVar;++i)
		     xFuncs[i].set_value(x[i]);
		   for(size_t i(0);i<nVar;++i)
		     rhs[i]=rhsFuncs[i].evaluate()-x[i];
		   for(size_t i(0);i<nVar*nVar;++i)
		     rhs[i+nVar]=jacobian[i].evaluate();
		   for(size_t i(0);i<nVar;++i)
		     rhs[nVar+i*(nVar+1)]-=1.0;
		 }
	     });
	}
      else
	if(extJac)
	  void();
      return NULL;
    }

    Newton::F* create_ode_newton_function(const EquationSystem& eqsys, bool extJac)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  size_t nVar(xFuncs.size());
	  
	  VecF jacobian;
	  build_partial_derivative_matrix(xFuncs,rhsFuncs,jacobian);
	  return new Newton::F
	    ([=](const double* x, double* rhs) mutable
	     {
		   for(size_t i(0);i<nVar;++i)
		     xFuncs[i].set_value(x[i]);
		   for(size_t i(0);i<nVar;++i)
		     rhs[i]=rhsFuncs[i].evaluate();
		   for(size_t i(0);i<nVar*nVar;++i)
		     rhs[i+nVar]=jacobian[i].evaluate();
	     });
	}
      else
	{
	  if(!extJac)
	    return new Newton::F(eqsys.f());
	  else
	    {
	      F f_(eqsys.f());
	      F dfdx_(eqsys.dfdx());
	      size_t nVar(eqsys.n_variables());
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
	      return new Newton::F([f_,dfdx_,nVar](const double* x, double* rhs)
		{
		  f_(x,rhs);
		  dfdx_(x,rhs+nVar);
		});
#pragma clang diagnostic pop
	    }
	}
      return NULL;
    }

    F* create_additional_function_evaluator(const EquationSystem& eqsys)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  size_t nVar(xFuncs.size());
	  size_t nFunc(funcFuncs.size());

	  return new F
	    ([=](const double* x, double* funcs) mutable
	     {
	       for(size_t i(0);i<nVar;++i)
		 xFuncs[i].set_value(x[i]);
	       for(size_t i(0);i<nFunc;++i)
		 funcs[i]=funcFuncs[i].evaluate();
	     });
	}
      else
	{
	  return new F(eqsys.func());
	}
      return NULL;
    }
    
    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, dat::Wave* varyingWave,
			       dat::Wave* evWave, double tol, size_t nPeriod, size_t showAllIterates, long secvar)
    {
      Task* task = new Task
	([=]() mutable
	 {

	   size_t nVar(stepper->nVar);	   

	   auto f = [nVar,stepper,nPeriod,tol](const double* x, double* rhs)
	   {
	     stepper->reset(x);
	     stepper->advance(nPeriod);
	     for(size_t i(0);i<nVar;++i)
	       rhs[i]=stepper->x(i)-x[i];
	     for(size_t i(0);i<nVar*nVar;++i)
	       rhs[i+nVar]=stepper->jac(i);	     
	     // subtract 1 from the diagonal of the Jacobian
	     for(size_t i(0);i<nVar;++i)
	       rhs[nVar+i*(nVar+1)]-=1.0;
	   };

	   double* x(new double[nVar]);
	   for(size_t i(0);i<nVar;++i)
	       x[i]=(*varyingWave)[uint32_t(i)+1];
	   varyingWave->remove_last_line();
	   std::string message;
	   bool success(false);
	   try
	     {
	       success=newton(int(nVar),x,f,false,tol);
	     }
	   catch(std::string error)
	     {
	       success=false;
	       message=APPLICATION_ERROR_PREFIX;
	       message+=error;
	       log->push(message);
	     }
	   if(!success)
	     {
	       message=APPLICATION_ERROR_PREFIX;
	       message+="Newton iteration did not converge\n";
	       log->push(message);
	       message=APPLICATION_FAIL_PREFIX;
	       message+=identifier;
	     }
	   else
	     {
	       // get eigenvalue and eigenvector info
	       VecD evals,evecs;
	       VecSZ types;
	       double* jac(new double[nVar*nVar]);
	       for(size_t i(0);i<nVar*nVar;++i)
		 jac[i]=stepper->jac(i);
	       if(secvar>=0) // in Poincare mode, restore the artificial zero-multiplier to 1
		 jac[size_t(secvar)*(nVar+1)]=1;
	       floquet(nVar,jac,evals,evecs,types);
	       evWave->append(&evals[0],uint32_t(nVar*2));
	       evWave->append(&evecs[0],uint32_t(nVar*nVar));
	       for(size_t i(0);i<types.size();++i)
		 evWave->append(double(types[i]));
	       delete[] jac;

	       message=APPLICATION_SUCCESS_PREFIX;
	       message+=identifier;

	       size_t factor(showAllIterates?nPeriod:1);
	       stepper->reset(x);
	       for(size_t j(0);j<factor;++j)
		 {
		   try
		     {
		       stepper->advance(nPeriod/factor);
		     }
		   catch(std::string error)
		     {
		       message=APPLICATION_ERROR_PREFIX;
		       message+=error;
		       log->push(message);
		       message=APPLICATION_FAIL_PREFIX;
		       message+=identifier;
		       break;
		     }
		   varyingWave->append(stepper->t());
		   for(size_t i(0);i<nVar;++i)
		     varyingWave->append(stepper->x(i));
		   for(size_t i(0);i<stepper->nFunc;++i)
		     varyingWave->append(stepper->func(i));
		 }
	     }
	   delete[] x;
	   
	   log->push(message);
	 });
      return task;

    }  

					      
    
    Task* create_guessing_task(std::string identifier, Log* log, size_t nVar, size_t nFunc,
			       Newton::F* f, std::function<void(const double*,double*)>* ff,
			       dat::Wave* varyingWave, dat::Wave* evWave, bool extJac, double tol, bool mapMode)
    {
    
      int genJac(extJac?0:1);
      int map(mapMode?1:0);

      Task* task = new Task
	([nVar,nFunc,f,ff,varyingWave,evWave,tol,genJac,map,identifier,log]() mutable
	 {
	   double* x(new double[nVar]);
	   double* funcs(new double[nFunc]);
 	   double t((*varyingWave)[0]);
	   for(size_t i(0);i<nVar;++i)
	     x[i]=(*varyingWave)[uint32_t(i)+1];
	   
	   varyingWave->remove_last_line();
	   std::string message;
	   bool success(false);
	   try
	     {
	       success=newton(int(nVar),x,*f,genJac,tol);
	     }
	   catch(std::string error)
	     {
	       success=false;
	       message=APPLICATION_ERROR_PREFIX;
	       message+=error;
	       log->push(message);
	     }
	   if(!success)
	     {
	       message=APPLICATION_ERROR_PREFIX;
	       message+="Newton iteration did not converge\n";
	       log->push(message);
	       message=APPLICATION_FAIL_PREFIX;
	       message+=identifier;
	     }
	   else
	     {
	       // get eigenvalue and eigenvector info
	       double* rhs(new double[nVar*(nVar+1)]);
	       (*f)(x,rhs);
	       double* jac=rhs+nVar;
	       VecD evals,evecs;
	       VecSZ types;
	       if(map)
		 {
		   for(size_t i(0);i<nVar;++i)
		     jac[i*(nVar+1)]+=1.0;
		   floquet(nVar,jac,evals,evecs,types);
		 }
	       else
		 eigen(nVar,jac,evals,evecs,types);

	       evWave->append(&evals[0],uint32_t(nVar*2));
	       evWave->append(&evecs[0],uint32_t(nVar*nVar));
	       for(size_t i(0);i<types.size();++i)
		 evWave->append(double(types[i]));
	       for(size_t i(0),size(size_t(evWave->data_max()));i<size;++i)
		 std::cout<<evWave->data()[i]<<std::endl;
	       delete[] rhs;
	       message=APPLICATION_SUCCESS_PREFIX;
	       message+=identifier;
	     }
	   varyingWave->append(t);
	   for(size_t i(0);i<nVar;++i)
	     varyingWave->append(x[i]);
	   if(nFunc)
	     (*ff)(x,funcs);
	   for(size_t i(0);i<nFunc;++i)
	     varyingWave->append(funcs[i]);
	   log->push(message);
	   delete f;
	   delete ff;
	   delete[] x;
	   delete[] funcs;
	 });
      return task;
    }
	
  } /* end namespace num */
} /* end namespace scigma */
 
