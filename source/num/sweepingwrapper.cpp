#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../dat/mesh.hpp"
#include "equationsystem.hpp"
#include "sweeping.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::num;
using scigma::dat::Mesh;


#include <iostream>
extern "C"
{
  
  PythonID scigma_num_sweep(const char* identifier, PythonID equationSystemID,PythonID logID,
			    int nSteps, PythonID meshID, PythonID blobID, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return -1;
    PYOBJ(Log,log,logID);if(!log)return -1;
    PYOBJ(Mesh,mesh,meshID);if(!mesh)return -1;
    PYOBJ(Blob,blob,blobID);if(!blob)return -1;

    if(nSteps<0)
      {
	nSteps=-nSteps;
	double dt(blob->get_double("dt"));
	dt=-dt;
	blob->set_double("dt",dt);
      }
    
    Task* task(create_sweeping_task(identifier,eqsys,log,size_t(nSteps),mesh,blob));
    task->run(noThread);
    return task->get_python_id();
  }
} /* end extern "C" block */
  
#pragma clang pop
