#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../dat/wave.hpp"
#include "mode.hpp"
#include "equationsystem.hpp"
#include "mapstepper.hpp"
#include "integrationstepper.hpp"
#include "poincarestepper.hpp"
#include "mapmanifoldstepper.hpp"
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
    Task* create_iteration_task(std::string identifier, Log* log,
				size_t nSteps, size_t nRays, size_t nConst,
				Stepper** stepperList, Wave* varyingWave,
				size_t nPeriod, size_t showAllIterates,
				size_t existingPoints, size_t initialPointIndex);
  }
}

extern "C"
{
  PythonID scigma_num_map_manifold(const char* identifier, PythonID equationSystemID,PythonID logID,
				   int nSteps, int nRays, PythonID varyingWaveID, PythonID blobID, bool showAllIterates, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return -1;
    PYOBJ(Log,log,logID);if(!log)return -1;
    PYOBJ(Wave,varyingWave,varyingWaveID);if(!varyingWave)return -1;
    PYOBJ(Blob,blob,blobID);if(!blob)return -1;

    std::vector<Stepper*> innerStepperList{size_t(nRays)};
    Stepper** outerStepperList(new Stepper*[size_t(nRays)]);
    
    Mode m((Mode(blob->get_int("mode"))));
    double period(blob->get_double("period"));
    size_t nPeriod((size_t(blob->get_int("nperiod"))));
    double dt(blob->get_double("dt"));
    double ds(blob->get_double("Manifolds.arc"));
    double alpha(blob->get_double("Manifolds.alpha"));
    double maxtime(blob->get_double("maxtime"));
    int secvar(blob->get_int("secvar"));
    int secdir(blob->get_int("secdir"));
    double secval(blob->get_double("secval"));
    bool stiff(blob->get_string("odessa.type")=="stiff"?true:false);
    double aTol(blob->get_double("odessa.atol"));
    double rTol(blob->get_double("odessa.rtol"));
    size_t maxIter((size_t(blob->get_int("odessa.mxiter"))));
    double nTol(blob->get_double("Newton.tol"));
        
    if(nSteps<0)
      {
	nSteps=-nSteps;
	dt=-dt;
	period=-period;
      }
    
    switch(m)
      {
      case MAP:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    innerStepperList[i]=new MapStepper(*eqsys,true);
	  }
	break;
      case ODE:
	{
	  log->push<LOG_ERROR>("calling map manifold in ODE mode\n");
	  return -1;
	}
      case STROBE:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    innerStepperList[i]=new IntegrationStepper(*eqsys,period,stiff,aTol,rTol,size_t(maxIter));
	  }
	break;
      case POINCARE:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    innerStepperList[i] =new PoincareStepper(*eqsys,dt,maxtime,secvar,secdir,secval,nTol,stiff,aTol,rTol, size_t(maxIter));
	  }
	break;
      }

    varyingWave->lock();
    auto const nVarying = varyingWave->size()/size_t(nRays)/2;
    auto const * steadyState = &varyingWave->data()[1];
    for(size_t i(0);i<size_t(nRays);++i)
      {
	auto const * firstSegment = &varyingWave->data()[(size_t(nRays) + i) * nVarying + 1];
	outerStepperList[i]=new MapManifoldStepper(innerStepperList[i], steadyState, firstSegment, ds,1e-12,alpha,1e-4,size_t(nPeriod));
      }
    varyingWave->unlock();
    
    Task* task(create_iteration_task(identifier,log,size_t(nSteps),size_t(nRays),0, outerStepperList, varyingWave, size_t(nPeriod),showAllIterates?1:0, 2, 1));
    task->run(noThread);
    return task->get_python_id();
  }

  PythonID scigma_num_ode_manifold(const char* identifier, PythonID equationSystemID, PythonID logID,
			   int nSteps, int nRays, PythonID varyingWaveID, PythonID blobID, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return -1;
    PYOBJ(Log,log,logID);if(!log)return -1;
    PYOBJ(Wave,varyingWave,varyingWaveID);if(!varyingWave)return -1;
    PYOBJ(Blob,blob,blobID);if(!blob)return -1;
    
    Mode m((Mode(blob->get_int("mode"))));
    double dt(blob->get_double("dt"));
    bool stiff(blob->get_string("odessa.type")=="stiff"?true:false);
    double aTol(blob->get_double("odessa.atol"));
    double rTol(blob->get_double("odessa.rtol"));
    size_t maxIter((size_t(blob->get_int("odessa.mxiter"))));
    
    if(nSteps<0)
      {
	nSteps=-nSteps;
	dt=-dt;
      }

    Stepper** stepperList(new Stepper*[size_t(nRays)]);

    switch(m)
      {
      case MAP:
	log->push<LOG_ERROR>("calling ode manifold in MAP mode\n");
	break;
      case ODE:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    stepperList[i]=new IntegrationStepper(*eqsys,dt,stiff,aTol,rTol,size_t(maxIter));
	  }
	break;
      case STROBE:
	log->push<LOG_ERROR>("calling ode manifold in STROBE mode\n");
	break;
      case POINCARE:
	log->push<LOG_ERROR>("calling ode manifold in POINCARE mode\n");
      }
    Task* task(create_iteration_task(identifier,log,size_t(nSteps), size_t(nRays),0,
				     stepperList,varyingWave,1,0,2,1));
    task->run(noThread);
    return task->get_python_id();
    
  }
  
} /* end extern "C" block */

#pragma clang pop
