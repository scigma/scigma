#ifndef __SCIGMA_NUM_FUNCTION_H__
#define __SCIGMA_NUM_FUNCTION_H__

#include <vector>
#include "operator.h"

namespace scigma
{
  namespace num
  {
  
  class Operator;
  class FunctionData;
  
#define MAX_FUNCTION_ARGS 4
  
  class Function
  {
  public:
    Function();
    Function(double value);
    Function(const Function& f);
    Function(const Operator& op, Function* args);
    ~Function();
    uint8_t get_number_of_arguments() const;
    double evaluate() const;
    double operator()(double* dArgs) const;
    Function operator()(Function* fArgs) const;
    void set_arguments(Function* args, uint8_t nArgs);
    Function* get_arguments() const;
    Function* get_dependencies() const;
    Function insert_functions(Function* fArgs, Function* oldArgs, uint8_t nArgs) const;
    Function get_partial_derivative(Function f) const;
    bool is_function_of(Function f) const;
    void set_value(double d);
    void unset_value();
    bool value_is_set() const;
    bool is_constant() const;
    void redefine(Function f);
    
    Function& operator=(const Function& f);
    bool operator==(const Function& f) const;
    bool operator!=(const Function& f) const;
    Function operator+(const Function& f) const;
    Function operator-(const Function& f) const;
    Function operator*(const Function& f) const;
    Function operator/(const Function& f) const;
    Function operator-() const;
    
  private:
    FunctionData *fData;
  };
  
  Function operator+(const double&, const Function&);
  Function operator-(const double&, const Function&);
  Function operator*(const double&, const Function&);
  Function operator/(const double&, const Function&);

  
  
  class FunctionData
  {
  public:
    FunctionData();
    FunctionData(double d);
    FunctionData(const Operator& op, Function* args);
    
    Operator op;
    double value;
    
    std::vector <Function> deps;
    std::vector <Function> args;
    
    uint8_t nArgs;
    uint8_t refcount;

    bool valueIsSet;
    bool isConstant;

    char padding[4];

  private:
    FunctionData(const FunctionData& f);
    FunctionData& operator=(const FunctionData& f);
  };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_FUNCTION_H__ */
