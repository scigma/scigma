#ifndef __SCIGMA_NUM_OPERATOR_H__
#define __SCIGMA_NUM_OPERATOR_H__

#include <string>
#include <vector>

namespace scigma
{
  namespace num
  {
  class Function;
  class OperatorData;
  
#define MAX_OPERATOR_ARGS 4
  
  class Operator
  {
  public:
    Operator();
    uint8_t get_number_of_arguments() const;
    std::string get_name() const;
    double operator()(double dArg) const;
    double operator()(double* dArgs) const;
    Function operator()(Function fArg) const;
    Function operator()(Function* fArgs) const;
    Function get_partial_derivative(Function x) const;
    Function get_partial_derivative(Function* args, uint8_t whichArg) const;
    
    static bool get_operator(std::string name,Operator& op);
    static void init_operators();
    
    bool operator==(const Operator& op) const;
    
    static bool is_initialized();
    
  private:
    Operator(std::string name,double(*funct)(double),Function (*dfunct)(Function));
    Operator(std::string name,double(*funct)(double*),Function (*dfunct)(Function*,uint8_t),uint8_t nArgs);
    
    OperatorData *opData;
    
    static bool define_operator(std::string name,double(*funct)(double),Function (*dfunct)(Function));
    static bool define_operator(std::string name,double(*funct)(double*),Function (*dfunct)(Function*,uint8_t),uint8_t nArgs);
    static bool undefine_operator(std::string name);
    
    static void clean_up_operators();
    
    static std::vector<Operator> operators;
    
    static bool isInitialized;
  };
  
  class OperatorData
  {
  public:
    OperatorData();
    OperatorData(std::string name,double (*funct)(double),Function (*dfunct)(Function));
    OperatorData(std::string name,double (*funct)(double*),Function (*dfunct)(Function*, uint8_t whichArg), uint8_t nArgs);
    double (*f_one)(double);
    double (*f_arbitrary)(double*);
    Function (*df_one)(Function);
    Function (*df_arbitrary)(Function*,uint8_t);
    
    std::string name;
    uint8_t nArgs;
    char padding[7];
    
  private:
    OperatorData(const OperatorData& op);
    OperatorData& operator=(const OperatorData& op);
  };
  
  } /* end namespace num */
} /* end namespace scigma */
  
#endif /* __SCIGMA_NUM_OPERATOR_H__ */
  
