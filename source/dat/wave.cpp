#include "wave.h"
#include "../gui/glutil.h"

namespace scigma
{
  namespace dat
  {
    

    Wave::Wave(uint32_t columns, double* values, uint32_t nValues, uint32_t capacity):
      PythonObject<Wave>(this),
      columns_(columns),capacity_((capacity*columns)>1?(capacity*columns):2),bufferCapacity_(capacity_),dataMax_(values?nValues:0),bufferMax_(0),chunkSize_(0x4000),data_(new double[capacity_]),oldData_(data_),glBuffer_(0),references_(0),app_(NULL),timeOfLastTransfer_(-3.0),transferring_(false), usedFullChunk_(false)
    {
      for(uint32_t i(0);i<nValues;++i)
	  data_[i]=values?values[i]:0.0;
    }

    uint32_t Wave::data_max() const {return dataMax_;}
    uint32_t Wave::buffer_max() const {return bufferMax_;}
    uint32_t Wave::data_rows() const {return columns_>0?dataMax_/columns_:0;}
    uint32_t Wave::buffer_rows() const {return columns_>0?bufferMax_/columns_:0;}

    uint32_t Wave::columns() const {return columns_;}
    
    GLuint Wave::gl_buffer() const {return glBuffer_;}

    double Wave::operator[](uint32_t index) const
    {
      while(oldData_!=data_) /* if data_ is currently newly allocated and populated in another thread, wait for the new data,
      			     	   which is guaranteed to be available at data_ once oldData_ and data_ are the same
				   	         */
						 continue;
      return data_[index]; /* technically, this is still not thread safe, there could have been another reallocation already;
      	     		         however, two reallocations in such a short time are very improbable
				 	         */
    }

    Wave::~Wave()
    {
      delete [] data_;
    }
    
    void Wave::grab(gui::Application* app)
    {
      tthread::lock_guard<tthread::mutex> guard(mutex_);
      app_=app;
      if(!glBuffer_)
      {
        glGenBuffers(1,&glBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
	bufferCapacity_=dataMax_>bufferCapacity_?dataMax_:bufferCapacity_;
	glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*bufferCapacity_,NULL,GL_DYNAMIC_DRAW);
	app_->connect_to_idle_threadsafe(this);
	transferring_=true;
      }
      ++references_;
    }
    
    void Wave::release()
    {
      tthread::lock_guard<tthread::mutex> guard(mutex_);
      --references_;
      if(0==references_&&glBuffer_)
      {
        if(transferring_)
	  {
	    app_->disconnect_from_idle_threadsafe(this);
	    transferring_=false;
	  }
	app_=NULL;
	glDeleteBuffers(1,&glBuffer_);
	glBuffer_=0;
      }
      //  delete[] oldData_;
    }

    void Wave::lock()
    {
      mutex_.lock();
    }

    double* Wave::data()
    {
      return data_;
    }

    void Wave::unlock()
    {
      mutex_.unlock();
    }

    void Wave::append(double value)
    {
      append(&value,1);
    }

    void Wave::append_line(double* values)
    {
      append(values,columns_);
    }

    void Wave::append(double* values, uint32_t nValues)
    {
      tthread::lock_guard<tthread::mutex> guard(mutex_);
      if(!transferring_&&glBuffer_)
      {
	app_->connect_to_idle_threadsafe(this);
	transferring_=true;
      }
      if(capacity_<dataMax_+nValues)
	{
	  while(capacity_<dataMax_+nValues)
	    capacity_*=1.5;
	  double* d = new double[capacity_];
	  for(uint32_t i(0);i<dataMax_;++i)
	    d[i]=data_[i];
	  data_=d;
	  delete [] oldData_;
	  oldData_=data_;
	}
      for(uint32_t i(0);i<nValues;++i)
	data_[dataMax_+i]=values[i]; 
      dataMax_+=nValues;
    }
              
    void Wave::remove_last()
    {
      remove_last_n(1);
    }
    
    void Wave::remove_last_line()
    {
      remove_last_n(columns_);
    }

    void Wave::remove_last_n(uint32_t nValues)
    {
      tthread::lock_guard<tthread::mutex> guard(mutex_);
      if(dataMax_<nValues)
	dataMax_=0;
      dataMax_=dataMax_-nValues;
      bufferMax_=bufferMax_>dataMax_?dataMax_:bufferMax_;
    }
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    
    bool Wave::process(gui::IdleEvent event, double time){process(time);return false;}
    
#pragma GCC diagnostic pop
    
    void Wave::process(double time)
    {
      tthread::lock_guard<tthread::mutex> guard(mutex_);
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
      
      if(timeSinceLastTransfer<0.33)  // do not try to upload more than three times per second
	return;
      timeOfLastTransfer_=time;

      /* if its taking too long to load, try to reduce chunkSize_, 
	 but not below 1k, because chances are its due to too many
	 Waves trying to upload rather than too much data in this
	 particular Wave. Also, do not decrease if chunk size if 
	 not enough data is produced (it would have no effect, then).
      */
      if(timeSinceLastTransfer>1.0&&usedFullChunk_)
	{
	  chunkSize_=chunkSize_>0x400?chunkSize_/2:chunkSize_;
	}
	
      if(dataMax_>bufferCapacity_) //create a larger buffer and copy the data, don't add new data for now
	{
	  usedFullChunk_=false;
	  while(dataMax_>bufferCapacity_)
	    bufferCapacity_*=1.5;
	  GLuint newBuffer;
	  glGenBuffers(1,&newBuffer);
	  glBindBuffer(GL_ARRAY_BUFFER,newBuffer);
	  glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*bufferCapacity_,NULL,GL_DYNAMIC_DRAW);
	  void* ptr=glMapBuffer(GL_ARRAY_BUFFER,GL_MAP_WRITE_BIT);
	  if(!ptr)
	    {
	      glDeleteBuffers(1,&newBuffer);
	      return;
	    }
	  GLfloat* floatPtr=reinterpret_cast<GLfloat*>(ptr);
	  for(uint32_t i(0);i<dataMax_;++i)
	    floatPtr[i]=GLfloat(data_[i]);
	  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
	    {
	      glDeleteBuffers(1,&newBuffer);
	      return;
	    }
	  GLuint oldBuffer(glBuffer_);
	  glBuffer_=newBuffer;
	  glDeleteBuffers(1,&oldBuffer);
	  bufferMax_=dataMax_;
	}
      else
	{
	  glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
	  uint32_t newMax(bufferMax_+chunkSize_);
	  if(dataMax_<newMax)
	    {
	      usedFullChunk_=false;
	      newMax=dataMax_;
	    }
	  else
	    usedFullChunk_=true;
#ifdef SCIGMA_USE_OPENGL_3_2
	  void* ptr=glMapBufferRange(GL_ARRAY_BUFFER,bufferMax_*sizeof(GLfloat),newMax*sizeof(GLfloat),GL_MAP_WRITE_BIT);
	  GLfloat* floatPtr=reinterpret_cast<GLfloat*>(ptr);
#else
	  void* ptr=glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	  GLfloat* floatPtr=reinterpret_cast<GLfloat*>(ptr);
	  floatPtr+=bufferMax_;
#endif
	  if(!ptr)
	      return;
	  for(uint32_t i(bufferMax_);i<newMax;++i)
	    floatPtr[i-bufferMax_]=GLfloat(data_[i]);
	  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
	    {
	      bufferMax_=0;
	      return;
	    }
	    bufferMax_=newMax;
	}
      
      if(bufferMax_==dataMax_)
	{
	  app_->EventSource<gui::IdleEvent>::Type::disconnect(this);
	  transferring_=false;
	}

    }
    
  } /* end namespace dat */
} /* end namespace scigma */
