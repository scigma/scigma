#include <vector>
#include <limits>
#include "mesh.hpp"
#include "wave.hpp"

using namespace scigma::common;
using namespace scigma::dat;

typedef AbstractWave<double> Wave;
typedef AbstractWave<int> IWave;
typedef AbstractWave<GLbyte> BWave;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

extern "C"
{
  /* Wrappers for Wave */
  
  PythonID scigma_dat_create_wave(int capacity)
  {
    Wave* ptr(NULL);
    if(capacity>0)
      ptr=new Wave(size_t(capacity));
    else
      ptr=new Wave;
    return ptr->get_python_id();
  }
  void scigma_dat_destroy_wave(PythonID objectID)
  {PYOBJ(Wave,ptr,objectID);if(ptr){delete ptr;}}


  void scigma_dat_wave_push_back(PythonID objectID, double* values, int nValues)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->push_back(values,size_t(nValues));
  }

  void scigma_dat_wave_pop_back(PythonID objectID, int nValues)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->pop_back(size_t(nValues));
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

  /* use very carefully, only between calls 
     to lock() and unlock()
  */
  int scigma_dat_wave_size(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(!ptr)
      return -1;
    return int(ptr->size());
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

  /* Wrappers for IWave */
  
  PythonID scigma_dat_create_iwave(int capacity)
  {
    IWave* ptr(NULL);
    if(capacity>0)
      ptr=new IWave(size_t(capacity));
    else
      ptr=new IWave;
    return ptr->get_python_id();
  }
  void scigma_dat_destroy_iwave(PythonID objectID)
  {PYOBJ(IWave,ptr,objectID);if(ptr){delete ptr;}}


  void scigma_dat_iwave_push_back(PythonID objectID, int* values, int nValues)
  {
    PYOBJ(IWave,ptr,objectID);
    if(ptr)
      ptr->push_back(values,size_t(nValues));
  }

  void scigma_dat_iwave_pop_back(PythonID objectID, int nValues)
  {
    PYOBJ(IWave,ptr,objectID);
    if(ptr)
      ptr->pop_back(size_t(nValues));
  }
  
  void scigma_dat_iwave_lock(PythonID objectID)
  {
    PYOBJ(IWave,ptr,objectID);
    if(ptr)
      ptr->lock();
  }

  void scigma_dat_iwave_unlock(PythonID objectID)
  {
    PYOBJ(IWave,ptr,objectID);
    if(ptr)
      ptr->unlock();
  }

  /* use very carefully, only between calls 
     to lock() and unlock()
  */
  int scigma_dat_iwave_size(PythonID objectID)
  {
    PYOBJ(IWave,ptr,objectID);
    if(!ptr)
      return -1;
    return int(ptr->size());
  }
  
  /* use very carefully, access resulting pointer 
     only between calls to lock() and unlock()
  */
  int* scigma_dat_iwave_data(PythonID objectID)
  {
    PYOBJ(IWave,ptr,objectID);
    if(ptr)
      return ptr->data();
    else
      return NULL;
  }
  
  /* Wrappers for BWave */
  
  PythonID scigma_dat_create_bwave(int capacity)
  {
    BWave* ptr(NULL);
    if(capacity>0)
      ptr=new BWave(size_t(capacity));
    else
      ptr=new BWave;
    return ptr->get_python_id();
  }
  void scigma_dat_destroy_bwave(PythonID objectID)
  {PYOBJ(BWave,ptr,objectID);if(ptr){delete ptr;}}


  void scigma_dat_bwave_push_back(PythonID objectID, GLbyte* values, int nValues)
  {
    PYOBJ(BWave,ptr,objectID);
    if(ptr)
      ptr->push_back(values,size_t(nValues));
  }

  void scigma_dat_bwave_pop_back(PythonID objectID, int nValues)
  {
    PYOBJ(BWave,ptr,objectID);
    if(ptr)
      ptr->pop_back(size_t(nValues));
  }
  
  void scigma_dat_bwave_lock(PythonID objectID)
  {
    PYOBJ(BWave,ptr,objectID);
    if(ptr)
      ptr->lock();
  }

  void scigma_dat_bwave_unlock(PythonID objectID)
  {
    PYOBJ(BWave,ptr,objectID);
    if(ptr)
      ptr->unlock();
  }

  /* use very carefully, only between calls 
     to lock() and unlock()
  */
  int scigma_dat_bwave_size(PythonID objectID)
  {
    PYOBJ(BWave,ptr,objectID);
    if(!ptr)
      return -1;
    return int(ptr->size());
  }
  
  /* use very carefully, access resulting pointer 
     only between calls to lock() and unlock()
  */
  GLbyte* scigma_dat_bwave_data(PythonID objectID)
  {
    PYOBJ(BWave,ptr,objectID);
    if(ptr)
      return ptr->data();
    else
      return NULL;
  }

  /* Wrappers for Mesh */

  PythonID scigma_dat_create_mesh(int nDim, int nInitial, double* initial)
  {
    std::vector<double> vInitial;
    for(size_t i(0);i<size_t(nInitial*nDim);++i)
      vInitial.push_back(initial[i]);
    Mesh* ptr=new Mesh(size_t(nDim),vInitial);
    return ptr->get_python_id();
  }

  PythonID scigma_dat_extend_mesh(PythonID objectID)
  {
    PYOBJ(Mesh,mesh,objectID);
    if(mesh)
      {
	Mesh* ptr=new Mesh(*mesh);
	return ptr->get_python_id();
      }
    return -1;
  }
  
  void scigma_dat_destroy_mesh(PythonID objectID)
  {PYOBJ(Mesh,ptr,objectID);if(ptr){delete ptr;}}

  int scigma_dat_mesh_n_layers(PythonID objectID)
  {
    PYOBJ(Mesh,ptr,objectID);if(!ptr){return -1;}
    size_t MAX(std::numeric_limits<size_t>::max());
    return(int(ptr->available_iso_layer(MAX,MAX,MAX)+1));
  }

  int scigma_dat_mesh_n_points(PythonID objectID)
  {
    PYOBJ(Mesh,ptr,objectID);if(!ptr){return -1;}
    size_t MAX(std::numeric_limits<size_t>::max());
    int nLayers(int(ptr->available_iso_layer(MAX,MAX,MAX)+1));
    int nIsoPoints(int(ptr->iso_indices().size()));
    return nIsoPoints-nLayers+1;
  }

  PythonID scigma_dat_mesh_points_and_normals(PythonID meshID)
  {
    PYOBJ(Mesh,ptr,meshID);if(!ptr){return -1;}
    return ptr->triangle_data().get_python_id();
  }

  PythonID scigma_dat_mesh_triangle_indices(PythonID meshID)
  {
    PYOBJ(Mesh,ptr,meshID);if(!ptr){return -1;}
    return ptr->triangle_indices().get_python_id();
  }

  PythonID scigma_dat_mesh_iso_indices(PythonID meshID)
  {
    PYOBJ(Mesh,ptr,meshID);if(!ptr){return -1;}
    return ptr->iso_indices().get_python_id();
  }
  PythonID scigma_dat_mesh_iso_end_points(PythonID meshID)
  {
    PYOBJ(Mesh,ptr,meshID);if(!ptr){return -1;}
    return ptr->iso_end_points().get_python_id();
  }
  
} /* end extern "C" block */

#pragma clang diagnostic pop

#undef PYOBJ
