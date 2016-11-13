#include "cosy.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::gui;

extern "C"
{
  PythonID scigma_gui_create_cosy(PythonID glWindowID, int view, bool largeFontsFlag)
  {
    PYOBJ(GLWindow,glWindow,glWindowID);
    if(!glWindow)
      return-1;
    Cosy* ptr(new Cosy(glWindow,view,largeFontsFlag));
    return ptr->get_python_id();
  }
  
  void scigma_gui_destroy_cosy(PythonID objectID){PYOBJ(Cosy,ptr,objectID);if(ptr)delete ptr;}
  
  void scigma_gui_cosy_set_view(PythonID objectID, int view)
  {PYCLL(Cosy,objectID,set_view(view))}
  void scigma_gui_cosy_set_label(PythonID objectID, int coordinate, const char* label)
  {PYCLL(Cosy,objectID,set_label(coordinate,label))}

} /* end extern "C" block */

#pragma clang diagnostic pop
