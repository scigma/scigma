#ifndef SCIGMA_NUM_PARSER_HPP
#define SCIGMA_NUM_PARSER_HPP

#include <vector>
#include <string>
#include <ostream>
#include <istream>
#include "function.hpp"

namespace scigma
{

  namespace num
  {

    class FunctionInfo
    {
    public:
      Function f;
      std::string name;
      std::string definition;
      std::vector<Function> requiredFunctions;
    };
    
    typedef std::vector<FunctionInfo> FunctionSet;
    
    FunctionSet::iterator get_function_from_set(std::string name, FunctionSet& fSet);
    FunctionSet::iterator get_function_from_set(Function f, FunctionSet& fSet);
    FunctionSet::iterator get_time_derivative_from_set(std::string name, FunctionSet& fSet);
    FunctionSet::iterator get_time_derivative_from_set(Function f, FunctionSet& fSet);
    FunctionSet::const_iterator get_function_from_set(std::string name, const FunctionSet& fSet);
    FunctionSet::const_iterator get_function_from_set(Function f, const FunctionSet& fSet);
    FunctionSet::const_iterator get_time_derivative_from_set(std::string name, const FunctionSet& fSet);
    FunctionSet::const_iterator get_time_derivative_from_set(Function f, const FunctionSet& fSet);
    std::string parse_expression(std::string expression, FunctionSet& fSet);
    //    void sort_function_set(FunctionSet& fSet);
    //void clone_sorted_function_set(const FunctionSet& source, FunctionSet& target);
    void clone_function_set(const FunctionSet& source, FunctionSet& target);
    
  } /* end namespace num */
} /* end namespace scigma */
  
#endif /* SCIGMA_NUM_PARSER_HPP */
