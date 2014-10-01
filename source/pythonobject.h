#ifndef __SCIGMAPYTHONOBJECT_H__
#define __SCIGMAPYTHONOBJECT_H__

#include <vector>
#include <stddef.h>

namespace scigma
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
	}
      
      //!destroy a PythonObject
      ~PythonObject()
	{
	  objects[size_t(id_)]=NULL;
	  availablePythonIDs.push_back(id_);
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
	if(id<int(objects.size())&&id>=0)
	  return objects.at(size_t(id));
	else
	  return NULL;
      }

    private:
      static std::vector<T*> objects;
      static std::vector<PythonID> availablePythonIDs;

      PythonID id_; 
      char padding_[4];
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
  template<class T> std::vector<T*> PythonObject<T>::objects;
  template<class T> std::vector<PythonID> PythonObject<T>::availablePythonIDs;
#pragma clang diagnostic pop  
} /* end namespace scigma */

#endif /* __SCIGMAPYTHONOBJECT_H__ */
