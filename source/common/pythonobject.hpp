#ifndef SCIGMA_COMMON_PYTHONOBJECT_HPP
#define SCIGMA_COMMON_PYTHONOBJECT_HPP

#include <vector>
#include <stddef.h>
#include <tinythread.h>

/* Macros to retrieve object pointers from PythonIDs
   and call member functions of these objects.
*/
#define PYOBJ(cls,name,pyID) cls* name(scigma::common::PythonObject<cls>::get_pointer(pyID))
#define PYCLL(cls,pyID,func) PYOBJ(cls,ptr,pyID);if(ptr){ptr->func;}

namespace scigma
{
  namespace common
  {
    typedef int PythonID;
    
    //!Base class for Python-accessible types
    /*!
      This class stores pointers to objects of its members in a vector.
      The position of the object in that vector is the object's PythonID,
      which can be used to access from the Python interpreter.
      @tparam T is the type that is wrapped for use in Python.
    */
    template <class T> class PythonObject
      {
      public:
	//!create a PythonObject from a pointer
	PythonObject(T* ptr)
	  {
	    
	    mutex.lock();
	    if(availablePythonIDs.empty())     
	      {
		id_=int(objects.size());
		objects.push_back(ptr);
	    }
	    else
	      {
		id_=availablePythonIDs.back();
		availablePythonIDs.pop_back();
		objects[size_t(id_)]=ptr;
	      }
	    padding_[0]=0;
	    mutex.unlock();
	  }
	
	//!destroy a PythonObject
	~PythonObject()
	  {
	    mutex.lock();
	    objects[size_t(id_)]=NULL;
	    availablePythonIDs.push_back(id_);
	    mutex.unlock();
	  }
	
	//!get PythonID from this object 
	/*!
	  @return the PythonID of this object
	*/
	PythonID get_python_id()
	{
	  return id_;
	}
	
	//!retrieve object from PythonID 
	/*!
	  @param id is a PythonID
	  @return a pointer to object that belongs to the PythonID id, 
	  or NULL if there is no such object.
	*/
	static T* get_pointer(PythonID id)
	{
	  T* retval(NULL);
	  mutex.lock();
	  if(id<int(objects.size())&&id>=0)
	    retval=objects.at(size_t(id));
	  mutex.unlock();
	  return retval;
	}
	
      private:
	static std::vector<T*> objects;
	static std::vector<PythonID> availablePythonIDs;
	static tthread::mutex mutex;
	
	PythonID id_; 
	char padding_[4];
      };
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    template<class T> std::vector<T*> PythonObject<T>::objects;
    template<class T> std::vector<PythonID> PythonObject<T>::availablePythonIDs;
    template<class T> tthread::mutex PythonObject<T>::mutex;
#pragma clang diagnostic pop
  } /* end namespace common */
} /* end namespace scigma */
  
#endif /* SCIGMA_COMMON_PYTHONOBJECT_HPP */
  
