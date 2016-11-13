#include <cmath>
#include <cstdlib>
#include "../externalequationsystem.hpp"
#include "externalequationsystem_test_shared.hpp"

using namespace scigma::num;

int main()
{
  VecS varNames;
  varNames.push_back("x");
  varNames.push_back("y");
  
  VecS funcNames;
  funcNames.push_back("f_1");
  funcNames.push_back("f_2");
  funcNames.push_back("f_3");

  {
    ExternalEquationSystem eqsys(varNames,f);
    double x[]={1.2345,6.7890};
    eqsys.set("x",1.2345); eqsys.set("y",6.7890);
    double rhs_1[2]; double rhs_2[2];
    f(x,rhs_1);
    eqsys.is_autonomous();
    eqsys.n_variables();
    eqsys.n_parameters();
    eqsys.n_functions();
    eqsys.n_constants();
    eqsys.variable_names();
    eqsys.variable_values();
    eqsys.set("x",9);
    eqsys.set("y",0.4);
    eqsys.get("x");
    eqsys.get("y");
    eqsys.f()(x,rhs_2);
    eqsys.f_p()(x,NULL,rhs_2);
    eqsys.f_t()(0,x,rhs_2);
    eqsys.f_pt()(0,x,NULL,rhs_2);
  }
  {
    ExternalEquationSystem eqsys(varNames,f,dfdx);
    double x[]={5.4321,0.9876};
    double jac_1[4]; double jac_2[4];
    dfdx(x,jac_1);
    eqsys.is_autonomous();
    eqsys.n_variables();
    eqsys.n_parameters();
    eqsys.n_functions();
    eqsys.n_constants();
    eqsys.variable_names();
    eqsys.variable_values();
    eqsys.set("x",9);
    eqsys.set("y",0.4);
    eqsys.get("x");
    eqsys.get("y");
    eqsys.dfdx()(x,jac_2);
    eqsys.dfdx_p()(x,NULL,jac_2);
    eqsys.dfdx_t()(0,x,jac_2);
    eqsys.dfdx_pt()(0,x,NULL,jac_2);
  }
  {
    ExternalEquationSystem eqsys(varNames,f,dfdx,funcNames,func);
    double x[]={9.8765,4.3210};
    eqsys.set("x",9.8765); eqsys.set("y",4.3210);
    double funcvals_1[3]; double funcvals_2[3];
    func(x,funcvals_1);
    eqsys.is_autonomous();
    eqsys.n_variables();
    eqsys.n_parameters();
    eqsys.n_functions();
    eqsys.n_constants();
    eqsys.variable_names();
    eqsys.variable_values();
    eqsys.set("x",9);
    eqsys.set("y",0.4);
    eqsys.get("x");
    eqsys.get("y");
    eqsys.func()(x,funcvals_2);
    eqsys.func_p()(x,NULL,funcvals_2);
    eqsys.func_t()(0,x,funcvals_2);
    eqsys.func_pt()(0,x,NULL,funcvals_2);
  }
  
  return 0;
}
