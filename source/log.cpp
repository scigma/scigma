#include <cstdio>
#include <sstream>
#include "log.h"

namespace scigma
{
  
  Log::Log(std::string fileName):PythonObject<Log>(this),file_(fileName)
  {
    if(""!=file_)
      remove(file_.c_str());
  }

  const char* Log::strip_path(const char* file)
  {
    size_t found = std::string(file).find_last_of("/\\");
    if(std::string::npos!=found)
      return &file[found+1];
    return file;
  }
  
  void Log::push(const std::string& text, const char* file, int line)
  {
    std::ostringstream combine;
    if(file)
      combine<<strip_path(file)<<", "<<"line "<<line<<": "<<text; 
    else
      combine<<text;

    tthread::lock_guard<tthread::mutex> guard(mutex_);
    list_.push_back(combine.str());
    if(""!=file_)
      {
	FILE * pFile;
	pFile = fopen (file_.c_str(), "a" );
	if (pFile!=NULL)
	  {
	    fprintf(pFile,"%s\n",combine.str().c_str());
	    fclose(pFile);
	  }
      }
  }

  const char* Log::pop()
  {
    tthread::lock_guard<tthread::mutex> guard(mutex_);
     if(!list_.empty())
       {
	current_=list_.front();
	list_.pop_front();
	return current_.c_str();
       }
     return NULL;
  }

} /* end namespace scigma */
