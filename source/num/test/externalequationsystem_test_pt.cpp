#include <cmath>
#include <cstdlib>
#include <cctype>
#include <catch.hpp>
#include "../externalequationsystem.hpp"
#include "externalequationsystem_test_shared.hpp"

#define REQUIRE_EXCEPTION(X,Y) {std::string error;try{X;}catch(std::string err){error=err;}REQUIRE(error==Y+error.substr(std::string(Y).size()));}

using namespace scigma::num;

extern const std::string allowed;

extern std::string well_formed_name(int maxLength);
extern std::string ill_formed_name(int maxLength);


SCENARIO ("non-autonomous external ODEs with parameters","[equationsystem][external]")
{
  std::srand(std::time(NULL));

  GIVEN("a vector of variable names (varNames), a vector of parameter names (parNames), the right hand side (f_pt) of the ODE [optional: a Jacobian (dfdx_pt), a function to evaluate the sensitivity (dfdp), a set of function names (funcNames), the associated evaluation routine (func_pt)]")
    {
      VecS varNames;
      varNames.push_back("x");
      varNames.push_back("y");

      VecS parNames;
      parNames.push_back("a");
      parNames.push_back("b");
      parNames.push_back("c");
      
      VecS funcNames;
      funcNames.push_back("f_1");
      funcNames.push_back("f_2");
      funcNames.push_back("f_3");

      WHEN("varNames is empty")
	THEN("exception \"need at least one variable\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(VecS(),parNames,f_pt)),"need at least one variable");

      WHEN("parNames is empty")
	THEN("exception \"need at least one parameter\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,VecS(),f_pt)),"need at least one parameter");

      THEN("exception \"right hand side cannot be NULL\" is thrown")
	{
	  F_pt nullfunc(NULL);
	  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,nullfunc)),"right hand side cannot be NULL");
	}

      WHEN("varNames, parNames and funcNames are well-formed")
	{
	  THEN("no exception \"ill-formed <type> name: <name>\" is thrown")
	    {
	      for(int i(0);i<100;++i)
		{
		  varNames[1]=well_formed_name(10);
		  CAPTURE(varNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt)),"");
		}
	      for(int i(0);i<100;++i)
		{
		  parNames[2]=well_formed_name(10);
		  CAPTURE(parNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt)),"");
		}
	      for(int i(0);i<100;++i)
		{
		  funcNames[0]=well_formed_name(10);
		  CAPTURE(funcNames[0]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames, func_pt)),"");
		}
	    }
	}
      
      WHEN("any name in varNames, parNames or funcNames is ill-formed")
	{
	  THEN("exception \"ill-formed <type> name: <name>\" is thrown")
	    {
	      for(int i(0);i<100;++i)
		{
		  varNames[1]=ill_formed_name(10);
		  CAPTURE(varNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt)),"ill-formed variable name:");
		}
	      varNames[1]="y";
	      for(int i(0);i<100;++i)
		{
		  parNames[2]=ill_formed_name(10);
		  CAPTURE(parNames[2]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt)),"ill-formed parameter name:");
		}
	      parNames[2]="c";
	      for(int i(0);i<100;++i)
		{
		  funcNames[0]=ill_formed_name(10);
		  CAPTURE(funcNames[0]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames, func_pt)),"ill-formed function name:");
		}
	    }
	}
      WHEN("names in varNames/parNames/funcNames are not unique")
	{
	  THEN("exception \"name is not unique: <name>\" is thrown")
	    {
	      varNames[1]="x";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt)),"name is not unique: x");
	      varNames[1]="y";
	      parNames[0]="b";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt)),"name is not unique: b");
	      parNames[0]="a";
	      funcNames[2]="f_2";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames,func_pt)),"name is not unique: f_2");
	      funcNames[2]="f_3";
	      parNames[2]="x";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames,func_pt)),"name is not unique: x");
	      parNames[2]="c";
	      parNames[1]="f_3";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames,func_pt)),"name is not unique: f_3");
	      parNames[1]="b";
	      varNames[0]="f_1";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames,func_pt)),"name is not unique: f_1");
	    }
	}

      WHEN("EquationSystem is constructed without Jacobian (dfdx=NULL)")
	THEN("dfdx(), dfdx_p() and dfdx_pt() return NULL")
	{
	  ExternalEquationSystem eqsys(varNames,parNames,f_pt);
	  REQUIRE(!eqsys.dfdx());
	  REQUIRE(!eqsys.dfdx_p());
	  REQUIRE(!eqsys.dfdx_pt());
	}

      WHEN("funcNames is non-empty and func_pt is NULL")
	THEN("exception \"provided function names but no evaluation function\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,funcNames)),"provided function names but no evaluation function");

      WHEN("vector funcNames is empty and func_pt is not NULL")
	THEN("exception \"provided evaluation function but no function names\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,parNames,f_pt,NULL,NULL,VecS(),func_pt)),"provided evaluation function but no function names");

      WHEN("the ExternalEquationSystem has been successfully constructed with just varNames, parNames and f")
	{
	  ExternalEquationSystem eqsys(varNames,parNames,f_pt);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={1.2345,6.7890};
	  eqsys.set("x",1.2345); eqsys.set("y",6.7890);
	  double p[]={-10,-11,-12};
	  eqsys.set("a",-10); eqsys.set("b",-11); eqsys.set("c", -12);
	  double rhs_1[2]; double rhs_2[2];
	  f_pt(t,x,p,rhs_1);

	  THEN("dfdx(), dfdx_p() dfdx_t() and dfdx_pt() return NULL")
	    {
	      REQUIRE(!eqsys.dfdx());
	      REQUIRE(!eqsys.dfdx_p());
	      REQUIRE(!eqsys.dfdx_t());
	      REQUIRE(!eqsys.dfdx_pt());
	    }
	  THEN("dfdp_p() and dfdp_pt() return NULL")
	    {
	      REQUIRE(!eqsys.dfdp_p());
	      REQUIRE(!eqsys.dfdp_pt());
	    }
	  THEN("func(), func_p(), func_t() and  func_pt() return NULL")
	    {
	      REQUIRE(!eqsys.func());
	      REQUIRE(!eqsys.func_p());
	      REQUIRE(!eqsys.func_t());
	      REQUIRE(!eqsys.func_pt());
	    }
	  THEN("is_autonomous() returns false")
	    REQUIRE(!eqsys.is_autonomous());
	  THEN("n_variables() returns the correct number")
	    REQUIRE(eqsys.n_variables()==2);
	  THEN("n_parameters() returns the correct number")
	    REQUIRE(eqsys.n_parameters()==3);
	  THEN("n_functions(), n_constants() return 0")
	    {
	      REQUIRE(eqsys.n_functions()==0);
	      REQUIRE(eqsys.n_constants()==0);
	    }
	  THEN("variable_names() returns the correct names")
	    {
	      REQUIRE(eqsys.variable_names()[0]=="x");
	      REQUIRE(eqsys.variable_names()[1]=="y");
	    }
	  THEN("parameter_names() returns the correct names")
	    {
	      REQUIRE(eqsys.parameter_names()[0]=="a");
	      REQUIRE(eqsys.parameter_names()[1]=="b");
	      REQUIRE(eqsys.parameter_names()[2]=="c");
	    }
	  THEN("setting and retrieving t by name works")
	    {
	      eqsys.set("t",-2);
	      REQUIRE(eqsys.get("t")==-2);
	    }
	  THEN("time() returns the correct value")
	    {
	      eqsys.set("t",-2);
	      REQUIRE(eqsys.time()==-2);
	    }
	  THEN("setting and retrieving variables by name works")
	    {
	      eqsys.set("x",9);
	      eqsys.set("y",0.4);
	      REQUIRE(eqsys.get("x")==9);
	      REQUIRE(eqsys.get("y")==0.4);
	    }
	  THEN("setting and retrieving parameters by name works")
	    {
	      eqsys.set("a",-1);
	      eqsys.set("b",-.2);
	      eqsys.set("c",-3e-4);
	      REQUIRE(eqsys.get("a")==-1);
	      REQUIRE(eqsys.get("b")==-.2);
	      REQUIRE(eqsys.get("c")==-3e-4);
	    }
	  AND_THEN("variable_values() returns the correct values")
	    {
	      REQUIRE(eqsys.variable_values()[0]==1.2345);
	      REQUIRE(eqsys.variable_values()[1]==6.7890);
	    }
	  AND_THEN("parameter_values() returns the correct values")
	    {
	      REQUIRE(eqsys.parameter_values()[0]==-10);
	      REQUIRE(eqsys.parameter_values()[1]==-11);
	      REQUIRE(eqsys.parameter_values()[2]==-12);
	    }
	  THEN("f() returns a wrapper of the original right hand side function f_pt for current parameters and time")
	    {
	      eqsys.f()(x,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_p() returns a wrapper of the original right hand side function f_pt for the current time")
	    {
	      eqsys.f_p()(x,p,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_t() returns a wrapper of the original right hand side function f_pt for the current parameters")
	    {
	      eqsys.f_t()(t,x,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_pt() returns the original right hand side function f_pt")
	    {
	      eqsys.f_pt()(t,x,p,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	}
      WHEN("the Jacobian (dfdx_pt) has been provided")
	{
	  ExternalEquationSystem eqsys(varNames,parNames,f_pt,dfdx_pt);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={5.4321,0.9876};
	  double p[]={-10,-11,-12};
	  eqsys.set("a",-10); eqsys.set("b",-11); eqsys.set("c",-12);
	  double jac_1[4]; double jac_2[4];
	  dfdx_pt(t,x,p,jac_1);
	  
	  THEN("dfdx() returns a wrapper of the original Jacobian dfdx_pt for current parameters and time")
	    {
	      eqsys.dfdx()(x,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_p() returns a wrapper of the original Jacobian dfdx_pt for the current time")
	    {
	      eqsys.dfdx_p()(x,p,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_t() returns a wrapper of the original Jacobian dfdx_pt for the current parameters")
	    {
	      eqsys.dfdx_t()(t,x,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_pt() returns the original Jacobian dfdx_pt")
	    {
	      eqsys.dfdx_pt()(t,x,p,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	}
      WHEN("the sensitivity function (dfdp_p) has been provided")
	{
	  ExternalEquationSystem eqsys(varNames,parNames,f_pt,dfdx_pt,dfdp_pt);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={5.4321,0.9876};
	  double p[]={-10,-11,-12};
	  eqsys.set("a",-10); eqsys.set("b",-11); eqsys.set("c",-12);
	  double dfdp_1[6]; double dfdp_2[6];
	  dfdp_pt(t,x,p,dfdp_1);
	  
	  THEN("dfdp_p() returns a wrapper of the original sensitivity function dfdx_pt for the current time")
	    {
	      eqsys.dfdp_p()(x,p,dfdp_2);
	      REQUIRE(dfdp_1[0]==dfdp_2[0]);
	      REQUIRE(dfdp_1[1]==dfdp_2[1]);
	      REQUIRE(dfdp_1[2]==dfdp_2[2]);
	      REQUIRE(dfdp_1[3]==dfdp_2[3]);
	      REQUIRE(dfdp_1[4]==dfdp_2[4]);
	    }
	  THEN("dfdp_pt() returns a wrapper of the original sensitivity function dfdx_p with the additional parameter t")
	    {
	      eqsys.dfdp_pt()(t,x,p,dfdp_2);
	      REQUIRE(dfdp_1[0]==dfdp_2[0]);
	      REQUIRE(dfdp_1[1]==dfdp_2[1]);
	      REQUIRE(dfdp_1[2]==dfdp_2[2]);
	      REQUIRE(dfdp_1[3]==dfdp_2[3]);
	      REQUIRE(dfdp_1[4]==dfdp_2[4]);
	    }
	}
      WHEN("additional functions (funcNames, func_pt) have been provided")
	{
	  ExternalEquationSystem eqsys(varNames,parNames,f_pt,dfdx_pt,dfdp_pt,funcNames,func_pt);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={9.8765,4.3210};
	  eqsys.set("x",9.8765); eqsys.set("y",4.3210);
	  double p[]={-10,-11,-12};
	  eqsys.set("a",-10); eqsys.set("b",-11); eqsys.set("c",-12);
	  double funcvals_1[3]; double funcvals_2[3];
	  func_pt(t,x,p,funcvals_1);
	  
	  THEN("n_functions() returns the correct number")
	    REQUIRE(eqsys.n_functions()==3);
	  THEN("function_names() returns the correct names")
	    {
	      REQUIRE(eqsys.function_names()[0]=="f_1");
	      REQUIRE(eqsys.function_names()[1]=="f_2");
	      REQUIRE(eqsys.function_names()[2]=="f_3");
	    }
	  THEN("retrieving functions by name works")
	    {
	      REQUIRE(eqsys.get("f_1")==std::sin(9.8765*t+4.3210)*(-10));
	      REQUIRE(eqsys.get("f_2")==std::cos(9.8765+4.3210)*(-11-t));
	      REQUIRE(eqsys.get("f_3")==std::tan(9.8765+4.3210*t)*(-12));
	    }
	  THEN("function_values() returns the correct values")
	    {
	      REQUIRE(eqsys.function_values()[0]==std::sin(9.8765*t+4.3210)*(-10));
	      REQUIRE(eqsys.function_values()[1]==std::cos(9.8765+4.3210)*(-11-t));
	      REQUIRE(eqsys.function_values()[2]==std::tan(9.8765+4.3210*t)*(-12));
	    }
	  THEN("func() returns a wrapper of the original function func_pt for the current parameters")
	    {
	      eqsys.func()(x,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_p() returns a wrapper of the original function func_pt for the current time")
	    {
	      eqsys.func_p()(x,p,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_t() returns a wrapper of the original function func_pt for the current parameters")
	    {
	      eqsys.func_t()(t,x,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_pt() returns the original function func_pt")
	    {
	      eqsys.func_pt()(t,x,p,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	}
    }
}
