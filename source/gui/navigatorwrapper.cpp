#include "navigator.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::gui;

extern "C"
{
  PythonID scigma_gui_create_navigator(int view){Navigator* ptr=new Navigator(view);return ptr->get_python_id();}
  void scigma_gui_destroy_navigator(PythonID objectID){PYOBJ(Navigator,ptr,objectID);if(ptr)delete ptr;}
  
} /* end extern "C" block */

#pragma clang diagnostic pop
