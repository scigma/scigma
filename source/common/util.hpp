#ifndef SCIGMA_COMMON_UTIL_HPP
#define SCIGMA_COMMON_UTIL_HPP

#include <string>
#include <vector>

namespace scigma
{
  namespace common
  {

    void substring(std::string& subject, const std::string& search,const std::string& replace);
    void append_tokens(const std::string& s, std::vector<std::string>& tokens, char separator);
    
  } /* end namespace common */
} /* end namespace scigma */
  
#endif /* SCIGMA_COMMON_UTIL_HPP */
  
