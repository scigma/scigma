#ifndef SCIGMA_DAT_WAVE_HPP
#define SCIGMA_DAT_WAVE_HPP

#include <tinythread.h>
#include "../common/pythonobject.hpp"
#include "../common/events.hpp"

using scigma::common::PythonObject;
using scigma::common::EventSource;

namespace scigma
{
  namespace dat
  {

    struct WaveInvalidateEvent
    { //!Argument list for MouseButtonEvent 
      typedef LOKI_TYPELIST_1(size_t) Arguments;};

    template <class T> class AbstractWave:
      public PythonObject<AbstractWave<T>>,
      public EventSource<WaveInvalidateEvent>::Type
      {
    public:
    	AbstractWave(size_t capacity=0x1000);
	~AbstractWave();
	
	void lock() const;
	void unlock() const;

	size_t size() const;
	
	const T* data() const;
	T* data();
	
	void push_back(T value);
	void push_back(const T* values, size_t nValues);

	void pop_back();
	void pop_back(size_t nValues);

	void invalidate_from_index(size_t index);
      
    private:
      AbstractWave(const AbstractWave<T>&);
      AbstractWave& operator=(const AbstractWave<T>&);

      size_t size_;
      size_t capacity_;
      T* data_;
      T* oldData_;

      mutable tthread::mutex mutex_;
    };

    template <class T> AbstractWave<T>::AbstractWave(size_t capacity):
      PythonObject<AbstractWave<T>>(this),size_(0),capacity_(capacity<2?2:capacity),data_(new T[capacity_]),oldData_(data_)
    {}

    template <class T> AbstractWave<T>::~AbstractWave()
    {
      delete [] data_;
    }

    template <class T> size_t AbstractWave<T>::size() const {return size_;}

    template <class T> void AbstractWave<T>::lock() const {mutex_.lock();} 
    template <class T> void AbstractWave<T>::unlock() const {mutex_.unlock();}
    
    template <class T> const T* AbstractWave<T>::data() const {return data_;}
    template <class T> T* AbstractWave<T>::data() {return data_;}

    template <class T> void AbstractWave<T>::push_back(T value)
    {
      push_back(&value,1);
    }

    template <class T> void AbstractWave<T>::push_back(const T* values, size_t nValues)
    {
      if(capacity_<size_+nValues)
	{
	  while(capacity_<size_+nValues)
	    {
	      capacity_=static_cast<size_t>(capacity_*1.5);
	    }
	  T* d = new T[capacity_];
	  for(size_t i(0);i<size_;++i)
	    d[i]=data_[i];
	  data_=d;
	  delete [] oldData_;
	  oldData_=data_;
	}
      for(size_t i(0);i<nValues;++i)
	data_[size_+i]=values[i]; 
      size_+=nValues;
    }

    template <class T> void AbstractWave<T>::pop_back()
    {
      size_=size_>1?size_-1:0;
      emit(size_);
    }

    template <class T> void AbstractWave<T>::pop_back(size_t nValues)
    {
      size_=size_>nValues?size_-nValues:0;
      emit(size_);
    }

    template <class T> void AbstractWave<T>::invalidate_from_index(size_t index)
    {
      emit(index);
    }
    
    
  } /* end namespace dat */
} /* end namespace scigma */

#endif /* SCIGMA_DAT_WAVE_HPP */
