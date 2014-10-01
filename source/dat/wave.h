#ifndef __SCIGMADATWAVE_H__
#define __SCIGMADATWAVE_H__

#include <tinythread.h>
#include "../pythonobject.h"
#include "../gui/definitions.h"
#include "../gui/application.h" 

extern "C"
{
  extern double* scigma_dat_wave_data(scigma::PythonID objectID);
}

namespace scigma
{
  namespace dat
  {
    class Wave:public PythonObject<Wave>,
      public EventSink<gui::IdleEvent>::Type
    {
    public:
      
      Wave(uint32_t columns, double* values=NULL, uint32_t nValues=0, uint32_t lines=0x1000);
      virtual ~Wave();

      uint32_t data_max() const;
      uint32_t buffer_max() const;
      
      uint32_t data_rows() const;
      uint32_t buffer_rows() const;
      
      uint32_t columns() const;

      GLuint gl_buffer() const;

      void grab(gui::Application* app);
      void release();

      void lock();
      /* use data only when you are sure there is no
	 other thread currently appending stuff, or
	 lock/unlock wave around acesses of data!!
      */
      double* data();
      void unlock();
            
      void append(double value);
      void append_line(double* values);
      void append(double* values, uint32_t nValues);
      
      void remove_last();
      void remove_last_line();
      void remove_last_n(uint32_t nValues);

      virtual bool process(gui::IdleEvent event, double time);
      
      double operator[](uint32_t index) const;

    private:
      Wave(const Wave&);
      Wave& operator=(const Wave&);

      void process(double time);

      uint32_t columns_;
      uint32_t capacity_;
      uint32_t bufferCapacity_;
      uint32_t dataMax_;
      uint32_t bufferMax_;
      uint32_t chunkSize_;
      double* data_;
      double* oldData_;

      GLuint glBuffer_;

      uint32_t references_;
      gui::Application* app_;
      
      mutable tthread::mutex mutex_;
      double timeOfLastTransfer_;
      bool transferring_;
      bool usedFullChunk_;

      char padding_[6];

    };

  } /* end namespace dat */
} /* end namespace scigma */

#endif /* __SCIGMADATWAVE_H__ */
