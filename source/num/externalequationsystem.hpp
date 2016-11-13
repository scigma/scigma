#ifndef SCIGMA_NUM_EXTERNALEQUATIONSYSTEM_HPP
#define SCIGMA_NUM_EXTERNALEQUATIONSYSTEM_HPP

#include <string>
#include <vector>

#include "equationsystem.hpp"

namespace scigma
{
  namespace num
  {

    typedef std::vector<std::string> VecS;
    typedef std::vector<double> VecD;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    class ExternalEquationSystem: public EquationSystem
    {

    public:
      ExternalEquationSystem(VecS variables, F f, F dfdx=NULL, VecS functions=VecS(), F func=NULL);
      ExternalEquationSystem(VecS variables, VecS parameters, F_p f, F_p dfdx=NULL, F_p dfdp =NULL, VecS functions=VecS(), F_p func=NULL);
      ExternalEquationSystem(VecS variables, F_t f, F_t dfdx=NULL, VecS functions=VecS(), F_t func=NULL);
      ExternalEquationSystem(VecS variables, VecS parameters, F_pt f, F_pt dfdx=NULL, F_pt dfdp=NULL, VecS functions=VecS(), F_pt func=NULL);

      virtual uint64_t time_stamp() const;
      virtual std::string parse(std::string expression);
      
      virtual void set(const std::string& name, double value);
      virtual double get(const std::string& name);
      
      virtual size_t n_variables() const;
      virtual size_t n_parameters() const;
      virtual size_t n_functions() const;
      virtual size_t n_constants() const;

      virtual const double& time() const;
      virtual const double* variable_values() const;
      virtual const double* parameter_values() const;
      virtual const double* function_values();
      virtual const double* constant_values();

      virtual const std::string* variable_names() const;
      virtual const std::string* parameter_names() const;
      virtual const std::string* function_names() const;
      virtual const std::string* constant_names() const;

      virtual const std::string* variable_definitions() const;
      virtual const std::string* function_definitions() const;
      virtual const std::string* constant_definitions() const;

      virtual bool is_autonomous() const;

      virtual F f() const;
      virtual F dfdx() const;
      virtual F func() const;
      virtual F_p f_p() const;
      virtual F_p dfdx_p() const;
      virtual F_p dfdp_p() const;
      virtual F_p func_p() const;
      virtual F_t f_t() const;
      virtual F_t dfdx_t() const;
      virtual F_t func_t() const;
      virtual F_pt f_pt() const;
      virtual F_pt dfdx_pt() const;
      virtual F_pt dfdp_pt() const;
      virtual F_pt func_pt() const;

    private:
      void update_function_values();
      std::string trim(std::string s) const;
      
      double t_;
      unsigned long long timeStamp_;

      VecS varNames_; VecS varDefs_; VecD varValues_;
      VecS parNames_; VecD parValues_;
      VecS funcNames_; VecS funcDefs_; VecD funcValues_;
      VecS constNames_; VecS constDefs_; VecD constValues_;

      F f_, dfdx_, func_;
      F_p f_p_, dfdx_p_, dfdp_p_, func_p_;
      F_t f_t_, dfdx_t_, func_t_;
      F_pt f_pt_, dfdx_pt_, dfdp_pt_, func_pt_;

      bool isAutonomous_;
      bool hasChanged_;
    };
#pragma clang diagnostic pop;
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_EXTERNALEQUATIONSYSTEM_HPP */

