#include <string>
#include <sstream>
#include "log.hpp"

using namespace scigma::common;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"

extern "C"
{
  // wrappers for the Log class
  PythonID scigma_common_create_log(){Log* ptr=new Log("");return ptr->get_python_id();}
  void scigma_common_destroy_log(PythonID objectID){PYOBJ(Log,ptr,objectID);if(ptr)delete ptr;}

  std::string logMessage;
  
  const char* scigma_common_log_pop(PythonID objectID)
  {
    PYOBJ(Log,ptr,objectID);
    if(ptr)
      {
	std::pair<scigma::common::LogType,std::string> result(ptr->pop());
	std::stringstream ss;
	ss<<result.first;
	ss<<result.second;
	logMessage=ss.str();
	return logMessage.c_str();
      }
    else
      return NULL;
  }

} /* end extern "C" block */

#pragma clang diagnostic pop
