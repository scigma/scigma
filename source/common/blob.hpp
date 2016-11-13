#ifndef SCIGMA_COMMON_BLOB_HPP
#define SCIGMA_COMMON_BLOB_HPP

#include <string>
#include <map>
#include "pythonobject.hpp"

namespace scigma
{
  namespace common
  {
    class Blob:public PythonObject<Blob>
    {
    public:
      Blob();
      ~Blob();
      
      void set_bool(const char* identifier, int value);
      void set_int(const char* identifier, int value);
      void set_float(const char* identifier, float value);
      void set_double(const char* identifier, double value);
      void set_string(const char* identifier, std::string value);
      void set_color(const char* identifier, float* values);
      void set_direction(const char* identifier, float* values);
      
      bool get_bool(const char* identifier);
      int get_int(const char* identifier);
      float get_float(const char* identifier);
      double get_double(const char* identifier);
      std::string get_string(const char* identifier);
      void get_color(const char* identifier,float* values);
      void get_direction(const char* identifier, float* values);
      
    private:
      Blob(const Blob&);
      Blob& operator=(const Blob&);
      
      std::map<std::string,int> intValues_;
      std::map<std::string,float> floatValues_;
      std::map<std::string,float*> floatArrayValues_;
      std::map<std::string,double> doubleValues_;
      std::map<std::string,std::string> stringValues_;
      
    };
    
  } /* end namespace common */
} /* end namespace scigma */    


#endif /* SCIGMA_COMMON_BLOB_HPP */
