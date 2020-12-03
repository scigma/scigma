#ifndef SCIGMA_GUI_GLBUFFER_HPP
#define SCIGMA_GUI_GLBUFFER_HPP
#include <limits>
#include "../common/pythonobject.hpp"
#include "../common/events.hpp"
#include "../dat/wave.hpp"
#include "definitions.hpp"
#include "application.hpp"
#include <iostream>

using scigma::common::PythonObject;
using scigma::common::EventSink;
using scigma::common::EventSource;
using scigma::common::connect;
using scigma::common::disconnect;
using scigma::dat::AbstractWave;
using scigma::dat::WaveInvalidateEvent;

namespace scigma
{
  namespace gui
  {
    struct GLBufferInvalidateEvent{  
      typedef LOKI_TYPELIST_0 Arguments;};
    
    template <class T,class U> class AbstractGLBuffer:
      public PythonObject< AbstractGLBuffer<T,U> >,
      public EventSink<WaveInvalidateEvent>::Type,
      public EventSink<IdleEvent>::Type,
      public EventSource<GLBufferInvalidateEvent>::Type
      {
    public:
    	AbstractGLBuffer(const AbstractWave<U>* wave, GLsizei initialCapacity=0x4000);
	virtual ~AbstractGLBuffer();
	
	size_t size() const;
	GLuint buffer_ID() const;

	virtual bool process(WaveInvalidateEvent event, size_t size);
	virtual bool process(IdleEvent event, double time);

	void begin_transfer();
	void end_transfer();

	void finalize();

	bool is_transferring();
	
    private:
	AbstractGLBuffer(const AbstractGLBuffer<T,U>&);
	AbstractGLBuffer& operator=(const AbstractGLBuffer<T,U>&);

	const AbstractWave<U>* wave_;
	
	GLsizei size_;
	GLsizei chunkSize_;
	GLsizei capacity_;
	GLuint bufferID_;

	size_t finalSize_;

	double timeOfLastTransfer_;

	bool usedFullChunk_;
	bool isTransferring_;

	char padding_[6];	
    };


    template <class T, class U> AbstractGLBuffer<T,U>::AbstractGLBuffer(const AbstractWave<U>* wave, GLsizei initialCapacity):
      PythonObject<AbstractGLBuffer<T,U>>(this),wave_(wave),size_(0),
      chunkSize_(0x4000),capacity_(initialCapacity>2?initialCapacity:2),
      finalSize_(std::numeric_limits<GLsizei>::max()),
      timeOfLastTransfer_(-1),usedFullChunk_(false),
      isTransferring_(false)
    {
      glGenBuffers(1,&bufferID_);
      glBindBuffer(GL_ARRAY_BUFFER,bufferID_);
      glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(T))*capacity_,NULL,GL_DYNAMIC_DRAW);
    }
    
    template <class T, class U> AbstractGLBuffer<T,U>::~AbstractGLBuffer()
    {
      glDeleteBuffers(1,&bufferID_);
    }
    
    template <class T, class U> size_t AbstractGLBuffer<T,U>::size() const {return size_t(size_);}
    template <class T, class U> GLuint AbstractGLBuffer<T,U>::buffer_ID() const {return bufferID_;}

    template <class T, class U> void AbstractGLBuffer<T,U>::begin_transfer()
    {
      connect<WaveInvalidateEvent>(wave_,this);
      connect<IdleEvent>(Application::get_instance(),this);
      isTransferring_=true;
    }
    template <class T, class U> void AbstractGLBuffer<T,U>::end_transfer()
    {
      disconnect<IdleEvent>(Application::get_instance(),this);
      disconnect<WaveInvalidateEvent>(wave_,this);
      isTransferring_=false;
    }

    template <class T, class U> void AbstractGLBuffer<T,U>::finalize()
    {
      wave_->lock();
      finalSize_=wave_->size();
      wave_->unlock();
    }
    
    template <class T, class U> bool AbstractGLBuffer<T,U>::is_transferring(){return isTransferring_;}
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    template <class T, class U> bool AbstractGLBuffer<T,U>::process(WaveInvalidateEvent event, size_t size)
    {
      size_=size_>GLsizei(size)?GLsizei(size):size_;
      return false;
    }
    
    template <class T, class U> bool AbstractGLBuffer<T,U>::process(IdleEvent event, double time)
    {
      if(size_>=GLsizei(finalSize_))
	{
	  end_transfer();
	  return false;
	}
      wave_->lock();
      GLsizei dataMax(GLsizei(wave_->size()));

      if(dataMax<=size_)
	{
	  wave_->unlock();
	  return false;
	}

      const U* data(wave_->data());
      
      double timeSinceLastTransfer(time-timeOfLastTransfer_);
      
      /* if we are very fast, maybe we can afford to load a little
	 more data per chunk - don't increase though, if there is
	 not enough data produced anyway (because we did not even
	 completely use the last chunk)
      */
      if(timeSinceLastTransfer<0.1&&usedFullChunk_)
	{
	  chunkSize_*=2;
	  usedFullChunk_=false;
	}
      
      /*      if(timeSinceLastTransfer<0.05)  // do not try to upload too often
	{
	  wave_->unlock();
	  return false;
	  }*/
      
      timeOfLastTransfer_=time;

      /* if its taking too long to load, try to reduce chunkSize_, 
	 but not below 1k, because chances are its due to too many
	 transfers going on rather than too much data in this
	 particular GLBuffer. Also, do not decrease if chunk size if 
	 not enough new data is available (it would have no effect, then).
      */
      if(timeSinceLastTransfer>1.0&&usedFullChunk_)
	{
	  chunkSize_=chunkSize_>0x400?chunkSize_/2:chunkSize_;
	}

      
      if(dataMax>capacity_) //create a larger buffer and copy the data, don't add new data for now
	{
	  usedFullChunk_=false;
	  GLsizei newCapacity(capacity_);
	  while(dataMax>newCapacity)
	    newCapacity=static_cast<GLsizei>(newCapacity*1.5);
	  GLuint newBufferID;
	  glGenBuffers(1,&newBufferID);
	  glBindBuffer(GL_ARRAY_BUFFER,newBufferID);
	  glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(T))*newCapacity,NULL,GL_DYNAMIC_DRAW);
	  void* ptr=glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	  if(!ptr)
	    {
	      glDeleteBuffers(1,&newBufferID);
	      wave_->unlock();
	      return false;
	    }
	  T* TPtr=reinterpret_cast<T*>(ptr);
	  for(size_t i(0);i<size_t(dataMax);++i)
	    TPtr[i]=T(data[i]);
	  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
	    {
	      glDeleteBuffers(1,&newBufferID);
	      wave_->unlock();
	      return false;
	    }
	  capacity_=newCapacity;
	  GLuint oldBufferID(bufferID_);
	  bufferID_=newBufferID;
	  emit();
	  glDeleteBuffers(1,&oldBufferID);
	  size_=dataMax;
	}
      else
	{
	  
	  glBindBuffer(GL_ARRAY_BUFFER,bufferID_);
	  GLsizei newSize(size_+chunkSize_);
	  if(dataMax<newSize)
	    {
	      usedFullChunk_=false;
	      newSize=dataMax;
	    }
	  else
	    usedFullChunk_=true;

	  
	  /*#ifdef SCIGMA_USE_OPENGL_3_2
	  void* ptr=glMapBufferRange(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(T))*size_,GLsizeiptr(sizeof(T))*newSize,GL_MAP_WRITE_BIT);
	  T* TPtr=reinterpret_cast<T*>(ptr);
	  #else*/
	  void* ptr=glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	  T* TPtr=reinterpret_cast<T*>(ptr);
	  TPtr+=size_;
	  //#endif
	  if(!ptr)
	    {
	      wave_->unlock();
	      return false;
	    }
	  for(size_t i((size_t(size_)));i<size_t(newSize);++i)
	    TPtr[i-size_t(size_)]=T(data[i]);
	  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
	    {
	      size_=0;
	      wave_->unlock();
	      return false;
	    }
	    size_=newSize;
	}
      
      wave_->unlock();
      return false;
    }

#pragma GCC diagnostic pop
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_GLBUFFER_HPP */
