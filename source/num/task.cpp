#include <iostream>
#include "task.hpp"

namespace scigma
{
  namespace num
  {

    void run_task(void* runFunctionPtr)
    {
      std::function<void(void)>* f(reinterpret_cast<std::function<void(void)>*>(runFunctionPtr));
      (*f)();
    }

    Task::Task(std::function<void(void)> runFunction):
      PythonObject<Task>(this),
      threadPtr_(NULL),runFunction_(runFunction)
    {
    }

    Task::~Task()
    {
      if(threadPtr_)
	{
	  threadPtr_->join();
	  delete threadPtr_;
	}
    }
    
    void Task::run(bool noThread)
    {
      if(noThread)
	{
	  runFunction_();
	}
      else
	{
	  threadPtr_=new tthread::thread(run_task,reinterpret_cast<void*>(&runFunction_));
	}
    }
  }
}
