#include <iostream>
#include "blob.hpp"

namespace scigma
{
  namespace common
  {
    
    Blob::Blob():PythonObject<Blob>(this)
    {}
    
    Blob::~Blob()
    {
      for(std::map<std::string,float*>::iterator i(floatArrayValues_.begin()),
	    end(floatArrayValues_.end());i!=end;++i)
	delete[] i->second;
    }
    
    void Blob::set_bool(const char* identifier, int value){intValues_[identifier]=value;}
    void Blob::set_int(const char* identifier, int value){intValues_[identifier]=value;}
    void Blob::set_float(const char* identifier, float value){floatValues_[identifier]=value;}
    void Blob::set_double(const char* identifier, double value){doubleValues_[identifier]=value;}
    void Blob::set_string(const char* identifier, std::string value){stringValues_[identifier]=value;}
    void Blob::set_color(const char* identifier, float* values)
    {
      float* ptr((floatArrayValues_[identifier]));
      if(ptr)
	delete[] ptr;
      ptr=floatArrayValues_[identifier]=new float[4];
      for(size_t i(0);i<4;++i)
	ptr[i]=values[i];
    }
    void Blob::set_direction(const char* identifier, float* values)
    {
      float* ptr((floatArrayValues_[identifier]));
      if(ptr)
	delete[] ptr;
      ptr=floatArrayValues_[identifier]=new float[3];
      for(size_t i(0);i<3;++i)
	ptr[i]=values[i];
    }
    
    bool Blob::get_bool(const char* identifier){return intValues_[identifier]==0?true:false;}
    int Blob::get_int(const char* identifier){return intValues_[identifier];}
    float Blob::get_float(const char* identifier){return floatValues_[identifier];}
    double Blob::get_double(const char* identifier){return doubleValues_[identifier];}
    std::string Blob::get_string(const char* identifier){return stringValues_[identifier];}
    void Blob::get_color(const char* identifier,float* values) 
    {
      float* ptr((floatArrayValues_[identifier]));
      if(!ptr)
	return;
      for(size_t i(0);i<4;++i)
	values[i]=ptr[i];
    }
    void Blob::get_direction(const char* identifier, float* values) 
    {
      float* ptr((floatArrayValues_[identifier]));
      if(!ptr)
	return;
      for(size_t i(0);i<3;++i)
	values[i]=ptr[i];
    }
  
  } /* end namespace common */
} /* end namespace scigma */
