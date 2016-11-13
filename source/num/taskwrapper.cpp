#include "task.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"


using namespace scigma::common;
using namespace scigma::num;

extern "C"
{

  void scigma_num_finish_task(PythonID taskID)
  {
    PYOBJ(Task,task,taskID);
    if(task)
      delete task;
  }

} /* end extern "C" block */

#pragma clang pop
