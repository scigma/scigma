#ifndef SCIGMA_NUM_INTERNALEQUATIONSYSTEM_HPP
#define SCIGMA_NUM_INTERNALEQUATIONSYSTEM_HPP

#include <string>
#include <vector>

#include "parser.hpp"
#include "equationsystem.hpp"

namespace scigma
{
  namespace num
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    class InternalEquationSystem: public EquationSystem
#pragma clang diagnostic pop;      
    {
      typedef std::vector<std::string> VecS;
      typedef std::vector<double> VecD;
      typedef std::vector<Function> VecF;
      
    public:
      InternalEquationSystem();

      uint64_t time_stamp() const override;
      std::string parse(std::string expression) override;

      void stall() override;
      void request_rebuild(bool lean=false);
      void flush() override;

      void clear();

      void set(const std::string& name, double value) override;
      double get(const std::string& name) override;
      
      size_t n_variables() const override;
      size_t n_parameters() const override;
      size_t n_functions() const override;
      size_t n_constants() const override;

      const double& time() const override;
      const double* variable_values() const override;
      const double* parameter_values() const override;
      const double* function_values() override;
      const double* constant_values() override;

      const std::string* variable_names() const override;
      const std::string* parameter_names() const override;
      const std::string* function_names() const override;
      const std::string* constant_names() const override;

      const std::string* variable_definitions() const override;
      const std::string* function_definitions() const override;
      const std::string* constant_definitions() const override;

      bool is_autonomous() const override;

      F f() const override;
      F dfdx() const override;
      F func() const override;
      F_p f_p() const override;
      F_p dfdx_p() const override;
      F_p dfdp_p() const override;
      F_p func_p() const override;
      F_t f_t() const override;
      F_t dfdx_t() const override;
      F_t func_t() const override;
      F_pt f_pt() const override;
      F_pt dfdx_pt() const override;
      F_pt dfdp_pt() const override;
      F_pt func_pt() const override;

    private:
      std::string trim(std::string s) const;
      void rebuild(bool lean);
      void clear_vectors();

      void prepare_variables_and_parameters(FunctionSet& source,
					    FunctionSet& varSet, FunctionSet& rhsSet, 
					    FunctionSet& parSet) const; 
      void prepare_functions_and_constants(FunctionSet& source, FunctionSet& funcSet, FunctionSet& constSet,
					   FunctionSet& varSet, Function tFunc) const;

      void build_variables(const FunctionSet& varSet, const FunctionSet& rhsSet);
      void build_parameters(const FunctionSet& parSet);      
      void build_functions_and_constants(const FunctionSet& info, VecF& funcs, VecS& names, VecS& defs, VecD& values);

      bool structure_changed(const FunctionSet& varSet, const FunctionSet& parSet, 
			     const FunctionSet& funcSet, const FunctionSet& constSet) const;

      void detach(Function& t, VecF& x, VecF& p, VecF& rhs, VecF& func) const;

      size_t stalled_;
      unsigned long long timeStamp_;
      
      double t_;
      Function tFunc_;

      VecF varFuncs_;VecS varNames_;VecS varDefs_; VecF rhsFuncs_; VecD varValues_;
      VecF parFuncs_;VecS parNames_; VecD parValues_;
      VecF funcFuncs_;VecS funcNames_;VecS funcDefs_; VecD funcValues_;
      VecF constFuncs_;VecS constNames_;VecS constDefs_; VecD constValues_;
      
      FunctionSet fSet_;

      bool rebuildRequested_;
      bool isAutonomous_;
      
    };
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_INTERNALEQUATIONSYSTEM_HPP */

