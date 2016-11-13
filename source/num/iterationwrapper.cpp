#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../common/util.hpp"
#include "../dat/wave.hpp"
#include "mode.hpp"
#include "equationsystem.hpp"
#include "mapstepper.hpp"
#include "integrationstepper.hpp"
#include "poincarestepper.hpp"
#include "task.hpp"

#include <iostream>

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
				size_t nSteps, size_t nRays, size_t nConst, Stepper** stepperList, Wave* varyingWave,
			        size_t nPeriod, size_t showAllIterates)
    {
      auto runFunction = 
	([=]() mutable
	 {
	   int escapeCount(ESCAPE_COUNT);

	   size_t nVar(stepperList[0]->n_variables());
	   size_t nFunc(stepperList[0]->n_functions());	   
	   varyingWave->lock();
	   size_t nVarying=varyingWave->size()/nRays;

	   // set initial conditions
	   const double* data = varyingWave->data(); 
	   for(size_t i(0);i<nRays;++i)
	       stepperList[i]->reset(data[i*nVarying],&data[i*nVarying+1]);

	   double* constData=new double[nRays*nConst];
	   for(size_t i(0);i<nRays;++i)
	     for(size_t j(0);j<nConst;++j)
	       constData[i*nConst+j]=data[(i+1)*nVarying-nConst+j];
	   
	   varyingWave->unlock();

	   size_t factor(showAllIterates?nPeriod:1);
	   bool* errorList=new bool[nRays];
	   for(size_t i(0);i<nRays;++i)
	     errorList[i]=false;
	   size_t nErrors(0);

	   size_t i(0);

	   for(;i<nSteps*factor;++i)
	     {
	       for(size_t j(0);j<nRays;++j)
		 {
		   try
		     {
		       if(!errorList[j])
			   stepperList[j]->advance(nPeriod/factor);
		     }
		   catch(std::string error)
		     {
		       log->push<LOG_ERROR>(error);
		       errorList[j]=true;
		       ++nErrors;
		       break;
		     }
		   varyingWave->lock();
		   varyingWave->push_back(stepperList[j]->t());
		   for(size_t k(0);k<nVar;++k)
		       varyingWave->push_back(stepperList[j]->x()[k]);
		   for(size_t k(0);k<nFunc;++k)
		     varyingWave->push_back(stepperList[j]->func()[k]);
		   for(size_t k(0);k<nConst;++k)
		       varyingWave->push_back(constData[j*nConst+k]);
		   varyingWave->unlock();
		 }
	       if((escapeCount!=ESCAPE_COUNT)||(nErrors==nRays))
		 break;
	     }

	   if(i>0)
	     log->push<LOG_SUCCESS>(identifier);
	   else
	     log->push<LOG_FAIL>(identifier);

	   for(i=0;i<nRays;++i)
	     delete stepperList[i];
	   delete[] stepperList;
	   delete[] errorList;
	   delete[] constData;
	 });

      return new Task(runFunction);
    }  
					      
  } /* end namespace num */
} /* end namespace scigma */


extern "C"
{
  PythonID scigma_num_plot(const char* identifier, PythonID equationSystemID, PythonID logID,
			   int nSteps, int nRays, PythonID varyingWaveID, const char* varParsC,
			   PythonID blobID, bool showAllIterates, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return -1;
    PYOBJ(Log,log,logID);if(!log)return -1;
    PYOBJ(Wave,varyingWave,varyingWaveID);if(!varyingWave)return -1;
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

    
    if(nSteps<0)
      {
	nSteps=-nSteps;
	dt=-dt;
	period=-period;
      }

    Stepper** stepperList(new Stepper*[size_t(nRays)]);
    std::vector<std::string> varPars;
    append_tokens(varParsC,varPars,'|');

    size_t nVar(eqsys->n_variables());
    size_t nFunc(eqsys->n_functions());
    size_t nVarPar(varPars.size());
    size_t nVarying(varyingWave->size()/size_t(nRays));
    size_t nConst(nVarying-nVar-nFunc-1);

    switch(m)
      {
      case MAP:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    for(size_t j(0);j<nVarPar;++j)
	      eqsys->set(varPars[j],varyingWave->data()[size_t(i)*nVarying+1+nVar+nFunc+j]);
	    stepperList[i]=new MapStepper(*eqsys,true);
	  }
	break;
      case ODE:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    for(size_t j(0);j<nVarPar;++j)
	      eqsys->set(varPars[j],varyingWave->data()[size_t(i)*nVarying+1+nVar+nFunc+j]);
	    stepperList[i]=new IntegrationStepper(*eqsys,dt,stiff,aTol,rTol,size_t(maxIter));
	  }
	break;
      case STROBE:
	for(size_t i(0);i<size_t(nRays);++i)
	  {
	    for(size_t j(0);j<nVarPar;++j)
	      eqsys->set(varPars[j],varyingWave->data()[size_t(i)*nVarying+1+nVar+nFunc+j]);
	    stepperList[i]=new IntegrationStepper(*eqsys,period,stiff,aTol,rTol,size_t(maxIter));
	  }
	break;
      case POINCARE:
	{
	  for(size_t i(0);i<size_t(nRays);++i)
	    {
	      for(size_t j(0);j<nVarPar;++j)
		eqsys->set(varPars[j],varyingWave->data()[size_t(i)*nVarying+1+nVar+nFunc+j]);
	      stepperList[i]=new PoincareStepper(*eqsys,dt,maxtime,secidx,secdir,secval,nTol,stiff,aTol,rTol, size_t(maxIter));
	    }
	}
      }
    Task* task(create_iteration_task(identifier,log,size_t(nSteps), size_t(nRays),nConst,
				     stepperList,varyingWave,size_t(nPeriod),showAllIterates?1:0));
    task->run(noThread);
    return task->get_python_id();
    
  }

} /* end extern "C" block */

#pragma clang pop
