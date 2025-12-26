#include <iostream>
#include <stdexcept>
#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../dat/wave.hpp"
#include "mode.hpp"
#include "newton.hpp"
#include "eigen.hpp"
#include "equationsystem.hpp"
#include "mapstepper.hpp"
#include "integrationstepper.hpp"
#include "poincarestepper.hpp"
#include "task.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::num;
typedef scigma::dat::AbstractWave<double> Wave;

namespace scigma
{
  namespace num
  {

    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, Wave* varyingWave,
			       Wave* evWave, double tol, size_t nPeriod, size_t showAllIterates, long secvar)
    {
      auto runFunction = 
	([=]() mutable
	 {
	   varyingWave->lock();
	   
	   size_t nVar(stepper->n_variables());
	   size_t nFunc(stepper->n_functions());	   
	   double t(stepper->t());
	   
	   auto f = [nVar,stepper,nPeriod,t](const double* x, double* rhs)
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
	       log->push<LOG_ERROR>(error);
	     }
	   if(!success)
	     {
	       log->push<LOG_ERROR>("Newton iteration did not converge\n");
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
		       log->push<LOG_ERROR>(error);
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
	     log->push<LOG_SUCCESS>(identifier);
	   else
	     log->push<LOG_FAIL>(identifier);

	   varyingWave->unlock();
	   delete stepper;
	 });

      return new Task(runFunction);
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
      
      auto runFunction =
	[nVar,nFunc,f,ff,varyingWave,evWave,tol,genJac,identifier,log]() mutable
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
	      log->push<LOG_ERROR>(error);
	    }
	  if(!success)
	    log->push<LOG_ERROR>("Newton iteration did not converge\n");
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
	    log->push<LOG_SUCCESS>(identifier);
	  else
	    log->push<LOG_FAIL>(identifier);
	  
	  varyingWave->unlock();
	};

#pragma clang diagnostic pop
      
      return new Task(runFunction);
    }
    
  } /* end namespace num */
} /* end namespace scigma */


extern "C"
{
  PythonID scigma_num_guess(const char* identifier, PythonID equationSystemID,PythonID logID,
			    PythonID varyingWaveID, PythonID evWaveID, PythonID blobID,
			    bool showAllIterates, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return -1;
    PYOBJ(Log,log,logID);if(!log)return -1;
    PYOBJ(Wave,varyingWave,varyingWaveID);if(!varyingWave)return -1;
    PYOBJ(Wave,evWave,evWaveID);if(!evWave)return -1;
    PYOBJ(Blob,blob,blobID);if(!blob)return -1;
    
    Mode m((Mode(blob->get_int("mode"))));
    double period(blob->get_double("period"));
    size_t nPeriod((size_t(blob->get_int("nperiod"))));
    double dt(blob->get_double("dt"));
    double maxtime(blob->get_double("maxtime"));
    int secidx(blob->get_int("secidx"));
    int secdir(blob->get_int("secdir"));
    double secval(blob->get_double("secval"));
    bool stiff(blob->get_string("odessa.type")=="stiff"?true:false);
    double aTol(blob->get_double("odessa.atol"));
    double rTol(blob->get_double("odessa.rtol"));
    size_t maxIter((size_t(blob->get_int("odessa.mxiter"))));
    double nTol(blob->get_double("Newton.tol"));

    Task* task(NULL);
    
    switch(m)
      {
      case MAP:
	{
	  MapStepper* mapStepper(new MapStepper(*eqsys,true));
	  task=create_guessing_task(identifier,log,mapStepper,varyingWave,evWave,nTol,size_t(nPeriod),showAllIterates?1:0,-1);
	}
	break;
      case ODE:
	if(!eqsys->is_autonomous())
	  {
	    log->push<LOG_ERROR>("cannot use Newton on non-autonomous dynamical system\n");
	    return -1;
	  }
	task=create_guessing_task(identifier,log,eqsys,varyingWave,evWave,nTol);
	break;
      case STROBE:
	{
	  IntegrationStepper* integrationStepper(new IntegrationStepper(*eqsys,period,stiff,aTol,rTol,size_t(maxIter),true)); 
	  task=create_guessing_task(identifier,log,integrationStepper,varyingWave,evWave,nTol,size_t(nPeriod),showAllIterates?1:0,-1);
	}
	break;
      case POINCARE:
	{
	  PoincareStepper* poincareStepper(new PoincareStepper(*eqsys,dt,maxtime,secidx,secdir,secval,nTol,stiff,aTol,rTol, size_t(maxIter),true));
	  task=create_guessing_task(identifier,log,poincareStepper,varyingWave,evWave,nTol,size_t(nPeriod),showAllIterates?1:0,long(secidx));
	}
	default:
      throw std::runtime_error("unexpected value of mode");
      }

    task->run(noThread);
    return task->get_python_id();
    
  }

} /* end extern "C" block */

#pragma clang pop
