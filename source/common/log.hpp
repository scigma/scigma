#ifndef SCIGMA_COMMMON_LOG_HPP
#define SCIGMA_COMMMON_LOG_HPP

#include <string>
#include <sstream>
#include <deque>
#include <utility>
#include <tinythread.h>
#include "pythonobject.hpp"

namespace scigma
{
  namespace common
  {

    enum LogType
      {
	LOG_SUCCESS=0,
	LOG_FAIL=1,
	LOG_DATA=2,
	LOG_WARNING=3,
	LOG_ERROR=4,
	LOG_DEFAULT=5
      };
    
    class Log:public PythonObject<Log>
    {
    public:
    
      Log();
      Log(std::string fileName);

    private:
      const char* strip_path(const char* file);

    public:
      template <LogType T=LOG_DEFAULT> void push(const std::string& text, const char* file=NULL, int line=0)
      {
	if(text=="")
	  return;
	
	std::ostringstream combine;
	if(file)
	  combine<<strip_path(file)<<", "<<"line "<<line<<": "<<text; 
	else
	  combine<<text;
	
	tthread::lock_guard<tthread::mutex> guard(mutex_);
	list_.push_back(std::pair<LogType,std::string>(T,combine.str()));
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

      std::pair<LogType,std::string> pop();
      
    private:
      
      Log(const Log&);
      Log& operator=(const Log&);
      
      std::deque<std::pair<LogType,std::string> >list_;
      tthread::mutex mutex_;
      std::string file_;
    };

  } /* end namespace common */
} /* end namespace scigma */

#endif /* SCIGMA_COMMON_LOG_HPP */
