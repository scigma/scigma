#ifndef __SCIGMA_NUM_EQUATIONSYSTEM_H__
#define __SCIGMA_NUM_EQUATIONSYSTEM_H__

#include <functional>
#include "../pythonobject.h"
#include "definitions.h"
#include "parser.h"

/*extern "C" void func_(int* NDIM,double* U,int* ICP,double* PAR,int* IJAC,double* F,double* DFDU,double* DFDP);
extern "C" void stpnt_(int* NDIM,double* U, double* PAR,double* T);
extern "C" void bcnd_(int* NDIM,double *PAR,int* ICP, int* NBC, double* U0,double* U1,double* FB,int* IJAC, int* DBC);
extern "C" void icnd_(int* NDIM,double* PAR,int* ICP,int* NINT,double* U, double* UOLD,double* UDOT,double* UPOLD,double* FI,int* IJAC,double* DINT);
extern "C" void fopt_(int* NDIM,double* U,int* ICP,double* PAR, int* IJAC, double* FS, double* DFDU,double* DFDP);
extern "C" void pvls_(int* NDIM,double* U,double* PAR);

extern "C" void f(int* N, double* T, double* Y, double* PAR, double* YDOT); 
extern "C" void jac(int* NEQ, double* T, double* Y, double* PAR, int* ML, int* MU, double* PD, int* NROWPD);
extern "C" void df(int* N, double* T, double* Y, double* PAR, double* DFDP, int* JPAR);

extern "C" void map(int* STEP, double* Y, double* PAR, double* RHS);
extern "C" void imap(int* STEP, double* Y, double* PAR, double* RHS);


extern "C" typedef void (*VECFLD)(int* N, double* T, double* Y, double* PAR, double* YDOT); 
extern "C" typedef void (*JAC)(int* NEQ, double* T, double* Y, double* PAR, int* ML, int* MU, double* PD, int* NROWPD);
extern "C" typedef void (*DF)(int* N, double* T, double* Y, double* PAR, double* DFDP, int* JPAR);

extern "C" typedef void (*MAP)(int* STEP, double* Y, double* PAR, double* RHS);
extern "C" typedef void (*IMAP)(int* STEP, double* Y, double* PAR, double* RHS);

*/

namespace scigma
{
  namespace num
  {

    class EquationSystem: public PythonObject<EquationSystem>
    {

    public:
   
      EquationSystem();
      
      EquationSystem(VecS variables, F f, F dfdx=NULL, VecS functions=VecS(), F func=NULL);
      EquationSystem(VecS variables, VecS parameters, F_p f, F_p dfdx=NULL, F_p dfdp =NULL, VecS functions=VecS(), F_p func=NULL);
      EquationSystem(VecS variables, F_t f, F_t dfdx=NULL, VecS functions=VecS(), F_t func=NULL);
      EquationSystem(VecS variables, VecS parameters, F_pt f, F_pt dfdx=NULL, F_pt dfdp=NULL, VecS functions=VecS(), F_pt func=NULL);

      std::string parse(std::string expression);
      void set(const std::string& name, double value);
      std::string get(const std::string& name);
      
      size_t n_variables() const;
      size_t n_parameters() const;
      size_t n_functions() const;
      size_t n_constants() const;

      const double& time() const;
      const double* variable_values() const;
      const double* parameter_values() const;
      const double* function_values() const;
      const double* constant_values() const;

      const std::string* variable_names() const;
      const std::string* parameter_names() const;
      const std::string* function_names() const;
      const std::string* constant_names() const;

      const std::string* variable_definitions() const;
      const std::string* function_definitions() const;
      const std::string* constant_definitions() const;

      void detach(Function& t, VecF& x, VecF& rhs, VecF& func) const;

      void stall();
      void request_rebuild();
      void flush();

      void clear();

      bool is_autonomous() const;
      bool is_internal() const;

      F f() const;
      F dfdx() const;
      F func() const;
      F_p f_p() const;
      F_p dfdx_p() const;
      F_p dfdp_p() const;
      F_p func_p() const;
      F_t f_t() const;
      F_t dfdx_t() const;
      F_t func_t() const;
      F_pt f_pt() const;
      F_pt dfdx_pt() const;
      F_pt dfdp_pt() const;
      F_pt func_pt() const;

      uint64_t time_stamp() const;

    private:
      EquationSystem(const EquationSystem&);
      EquationSystem& operator=(const EquationSystem&); 

      std::string trim(std::string s) const;
      void rebuild();
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

      size_t stalled_;

      unsigned long long timeStamp_;

      bool rebuildRequested_;

      bool isInternal_;
      bool isAutonomous_;

      char padding_[13];
 
      double t_;
      Function tFunc_;

      VecF varFuncs_;VecS varNames_;VecS varDefs_; VecF rhsFuncs_; VecD varValues_;
      VecF parFuncs_;VecS parNames_; VecD parValues_;
      VecF funcFuncs_;VecS funcNames_;VecS funcDefs_; VecD funcValues_;
      VecF constFuncs_;VecS constNames_;VecS constDefs_; VecD constValues_;
      
      FunctionSet fSet_;

      F f_, dfdx_, func_;
      F_p f_p_, dfdx_p_, dfdp_p_, func_p_;
      F_t f_t_, dfdx_t_, func_t_;
      F_pt f_pt_, dfdx_pt_, dfdp_pt_, func_pt_;

    };
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_EQUATIONSYSTEM_H__ */
