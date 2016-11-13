#include "util.hpp"

namespace scigma
{
  namespace common
  {
    
    void substring(std::string& subject, const std::string& search,const std::string& replace)
    {
      size_t pos = 0;
      while((pos = subject.find(search, pos)) != std::string::npos) {
	subject.replace(pos, search.length(), replace);
	pos += replace.length();
      }
    }

    void append_tokens(const std::string& s, std::vector<std::string>& tokens, char separator)
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

  } /* end namespace common */
} /* end namespace scigma */
