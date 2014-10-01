#ifndef __SCIGMA_UTIL_H__
#define __SCIGMA_UTIL_H__

#include <vector>
#include <string>

namespace scigma
{

  
  inline void append_tokens(const std::string& s, std::vector<std::string>& tokens, char separator)
  {
    /* This function splits the string s into tokens separated by separator and
       appends them to the vector tokens
    */
      size_t prev = 0;
      size_t next = 0;
      
      while ((next = s.find_first_of(separator, prev)) != std::string::npos)
	{
	  if (next - prev != 0)
	    {
	      tokens.push_back(s.substr(prev, next - prev));
	    }
	  prev = next + 1;
	}
      
      if (prev < s.size())
	{
	  tokens.push_back(s.substr(prev));
	}
    }

} /* end namespace scigma */


#endif /* __SCIGMA_UTIL_H__ */
