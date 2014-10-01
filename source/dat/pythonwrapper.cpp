#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "wave.h"

using namespace scigma;
using namespace scigma::dat;

// Macro to retrieve object pointers from PythonIDs
#define PYOBJ(cls,name,pyID) cls* name(PythonObject<cls>::get_pointer(pyID))

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"

extern "C"
{
  PythonID scigma_dat_create_wave(int width, double* values, int nValues, int capacity)
  {Wave* ptr=new Wave(uint32_t(width),values,uint32_t(nValues),uint32_t(capacity)); return ptr->get_python_id();}
  void scigma_dat_destroy_wave(PythonID objectID)
  {PYOBJ(Wave,ptr,objectID);if(ptr){delete ptr;}}

  int scigma_dat_wave_rows(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(!ptr)
      return -1;
    return int(ptr->data_rows());
  }

  int scigma_dat_wave_columns(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(!ptr)
      return -1;
    return int(ptr->columns());
  }

  void scigma_dat_wave_lock(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->lock();
  }

  void scigma_dat_wave_unlock(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->unlock();
  }
  
  /* use very carefully, access resulting pointer 
     only between calls to lock() and unlock()
  */
  double* scigma_dat_wave_data(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      return ptr->data();
    else
      return NULL;
  }

} /* end extern "C" block */

#pragma clang diagnostic pop

#undef PYOBJ
