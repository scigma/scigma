#include "../common/log.hpp"
#include "../dat/wave.hpp"
#include "iteration.hpp"
#include "equationsystem.hpp"
#include "internalmapstepper.hpp"
#include "internalodessastepper.h"
#include "internalpoincarestepper.h"
#include "externalodessastepper.h"
#include "externalpoincarestepper.h"
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {

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
    
    Stepper* create_integration_stepper(const EquationSystem& eqsys, bool computeJacobian,double dt,
					bool extJac,bool stiff, double aTol, double rTol, size_t maxIter)
    {
      IntegrationStepper(const EquationSystem& eqsys, double dt,bool stiff=true, double aTol=1e-9, double rTol=1e-9, size_t maxIter=20000);
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
		   log->push<Log::ERROR>(error);
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
	     log->push<Log::SUCCESS>(identifier);
	   else
	     log->push<Log::FAIL>(identifier);

	   delete stepper;
	 });
      return task;
    }

    
  } /* end namespace num */
} /* end namespace scigma */
