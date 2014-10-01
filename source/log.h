#ifndef __SCIGMALOG_H__
#define __SCIGMALOG_H__

#include <string>
#include <deque>
#include <tinythread.h>
#include "pythonobject.h"

namespace scigma
{
  class Log:public PythonObject<Log>
  {
  public:
    Log(std::string fileName="");

    void push(const std::string& text,const char* file=NULL, int line=0);
    const char* pop();
    
  private:
    Log();
    Log(const Log&);
    Log& operator=(const Log&);
    const char* strip_path(const char* file);

    std::string current_;
    std::deque<std::string> list_;
    tthread::mutex mutex_;
    std::string file_;
  };
  
} /* end namespace scigma */

#endif /* __SCIGMALOG_H__ */
