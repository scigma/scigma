#include "log.h"

using namespace scigma;

// Macro to retrieve object pointers from PythonIDs
#define PYOBJ(cls,name,pyID) cls* name(PythonObject<cls>::get_pointer(pyID))

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"


extern "C"
{
  // wrappers for the Log class
  PythonID scigma_create_log(){Log* ptr=new Log("");return ptr->get_python_id();}
  void scigma_destroy_log(PythonID objectID){PYOBJ(Log,ptr,objectID);if(ptr)delete ptr;}

  const char* scigma_log_pop(PythonID objectID)
  {
    PYOBJ(Log,ptr,objectID);
    if(ptr)
      return ptr->pop();
    else
      return NULL;
  }

  

} /* end extern "C" block */


#undef PYOBJ

#pragma clang diagnostic pop
