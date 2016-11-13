#ifndef SCIGMA_NUM_TASK_HPP
#define SCIGMA_NUM_TASK_HPP

#include <functional>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include <tinythread.h>
#pragma clang diagnostic pop
#include "../common/pythonobject.hpp"

using scigma::common::PythonObject;

extern "C" int ESCAPE_COUNT;

namespace scigma
{
  namespace num
  {
    void run_task(void* runFunctionPtr);
    				
    class Task: public PythonObject<Task>
    {
    public:
      Task(std::function<void(void)> runFunction);
      ~Task();
      
      void run(bool noThread);

    private:
      Task(const Task&);
      Task& operator=(const Task&);

      tthread::thread* threadPtr_;
      
      std::function<void(void)> runFunction_;
    };
    
  }
}

#endif /* SCIGMA_NUM_TASK_HPP */
