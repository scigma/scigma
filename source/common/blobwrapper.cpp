#include "blob.hpp"

using namespace scigma::common;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"


extern "C"
{
  PythonID scigma_common_create_blob(){Blob* ptr=new Blob();return ptr->get_python_id();}
  void scigma_common_destroy_blob(PythonID objectID){PYOBJ(Blob,ptr,objectID);if(ptr)delete ptr;}

  void scigma_common_blob_set_bool(PythonID blobID, const char* identifier, int value)
  {
     PYOBJ(Blob,ptr,blobID);
     if(ptr)
       ptr->set_bool(identifier,value);
  }

  void scigma_common_blob_set_int(PythonID blobID, const char* identifier, int value)
  {
     PYOBJ(Blob,ptr,blobID);
     if(ptr)
       ptr->set_int(identifier,value);
  }

  void scigma_common_blob_set_float(PythonID blobID, const char* identifier, float value)
  {
     PYOBJ(Blob,ptr,blobID);
     if(ptr)
       ptr->set_float(identifier,value);
  }

  void scigma_common_blob_set_double(PythonID blobID, const char* identifier, double value)
  {
    PYOBJ(Blob,ptr,blobID);
    if(ptr)
      ptr->set_double(identifier,value);
  }

  void scigma_common_blob_set_color(PythonID blobID, const char* identifier, float* values)
  {
     PYOBJ(Blob,ptr,blobID);
     if(ptr)
       ptr->set_color(identifier,values);
  }

  void scigma_common_blob_set_direction(PythonID blobID, const char* identifier,float* values)
  {
     PYOBJ(Blob,ptr,blobID);
     if(ptr)
       ptr->set_direction(identifier,values);
  }

  void scigma_common_blob_set_string(PythonID blobID, const char* identifier, const char* entry)
  {
     PYOBJ(Blob,ptr,blobID);
     if(ptr)
       ptr->set_string(identifier,entry);
  }

} /* end extern "C" block */

#pragma clang diagnostic pop
