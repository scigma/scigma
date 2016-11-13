#include <cmath>
#include <cstdlib>
#include <cctype>
#include <catch.hpp>
#include "../externalequationsystem.hpp"
#include "externalequationsystem_test_shared.hpp"

#define REQUIRE_EXCEPTION(X,Y) {std::string error;try{X;}catch(std::string err){error=err;}REQUIRE(error==Y+error.substr(std::string(Y).size()));}

using namespace scigma::num;

SCENARIO ("autonomous external ODEs without parameters","[equationsystem][external]")
{
  std::srand(std::time(NULL));

  GIVEN("a vector of variable names (varNames), the right hand side (f) of the ODE [optional: a Jacobian (dfdx), a set of function names (funcNames), the associated evaluation routine (func)]")
    {
      VecS varNames;
      varNames.push_back("x");
      varNames.push_back("y");
      
      VecS funcNames;
      funcNames.push_back("f_1");
      funcNames.push_back("f_2");
      funcNames.push_back("f_3");

      WHEN("varNames is empty")
	THEN("exception \"need at least one variable\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(VecS(),f)),"need at least one variable");

      THEN("exception \"right hand side cannot be NULL\" is thrown")
	{
	  F nullfunc(NULL);
	  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,nullfunc)),"right hand side cannot be NULL");
	}

      WHEN("varNames and funcNames are well-formed")
	{
	  THEN("no exception \"ill-formed <type> name: <name>\" is thrown")
	    {
	      for(int i(0);i<100;++i)
		{
		  varNames[1]=well_formed_name(10);
		  CAPTURE(varNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f)),"");
		}
	      for(int i(0);i<100;++i)
		{
		  funcNames[0]=well_formed_name(10);
		  CAPTURE(funcNames[0]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f,NULL, funcNames, func)),"");
		}
	    }
	}
      WHEN("any name in varNames or funcNames is ill-formed")
	{
	  THEN("exception \"ill-formed <type> name: <name>\" is thrown")
	    {
	      for(int i(0);i<100;++i)
		{
		  varNames[1]=ill_formed_name(10);
		  CAPTURE(varNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f)),"ill-formed variable name:");
		}
	      varNames[1]="y";
	      for(int i(0);i<100;++i)
		{
		  funcNames[0]=ill_formed_name(10);
		  CAPTURE(funcNames[0]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f,NULL, funcNames, func)),"ill-formed function name:");
		}
	    }
	}
      WHEN("names in varNames and/or funcNames are not unique")
	{
	  THEN("exception \"name is not unique: <name>\" is thrown")
	    {
	      varNames[1]="x";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f)),"name is not unique: x");
	      varNames[1]="y";
	      funcNames[2]="f_2";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f,NULL,funcNames,func)),"name is not unique: f_2");
	      funcNames[2]="x";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f,NULL,funcNames,func)),"name is not unique: x");
	    }
	}

      WHEN("EquationSystem is constructed with varNames and f only")

      WHEN("funcNames is non-empty and func is NULL")
	THEN("exception \"provided function names but no evaluation function\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f,NULL,funcNames)),"provided function names but no evaluation function");

      WHEN("vector funcNames is empty and func is not NULL")
	THEN("exception \"provided evaluation function but no function names\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f,NULL,VecS(),func)),"provided evaluation function but no function names");

      WHEN("the ExternalEquationSystem has been successfully constructed with just varNames and f")
	{
	  ExternalEquationSystem eqsys(varNames,f);
	  double x[]={1.2345,6.7890};
	  eqsys.set("x",1.2345); eqsys.set("y",6.7890);
	  double rhs_1[2]; double rhs_2[2];
	  f(x,rhs_1);

	  THEN("dfdx(), dfdx_p(), dfdx_t()  and  dfdx_pt() return NULL")
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
	  THEN("is_autonomous() returns true")
	    REQUIRE(eqsys.is_autonomous());
	  THEN("n_variables() returns the correct number")
	    REQUIRE(eqsys.n_variables()==2);
	  THEN("n_parameters(), n_functions(), n_constants() return 0")
	    {
	      REQUIRE(eqsys.n_parameters()==0);
	      REQUIRE(eqsys.n_functions()==0);
	      REQUIRE(eqsys.n_constants()==0);
	    }
	  THEN("variable_names() returns the correct names")
	    {
	      REQUIRE(eqsys.variable_names()[0]=="x");
	      REQUIRE(eqsys.variable_names()[1]=="y");
	    }
	  THEN("setting and retrieving variables by name works")
	    {
	      eqsys.set("x",9);
	      eqsys.set("y",0.4);
	      REQUIRE(eqsys.get("x")==9);
	      REQUIRE(eqsys.get("y")==0.4);
	    }
	  AND_THEN("variable_values() returns the correct values")
	    {
	      REQUIRE(eqsys.variable_values()[0]==1.2345);
	      REQUIRE(eqsys.variable_values()[1]==6.7890);
	    }
	  THEN("f() returns the original right hand side function f")
	    {
	      eqsys.f()(x,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_p() returns a wrapper of the original right hand side function f with the additional parameter p")
	    {
	      eqsys.f_p()(x,NULL,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_t() returns a wrapper of the original right hand side function f with the additional parameter t")
	    {
	      eqsys.f_t()(0,x,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_pt() returns a wrapper of the original right hand side function f with the additional parameters t and p")
	    {
	      eqsys.f_pt()(0,x,NULL,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	}
      WHEN("the Jacobian (dfdx) has been provided")
	{
	  ExternalEquationSystem eqsys(varNames,f,dfdx);
	  double x[]={5.4321,0.9876};
	  double jac_1[4]; double jac_2[4];
	  dfdx(x,jac_1);
	  
	  THEN("dfdx() returns the original Jacobian")
	    {
	      eqsys.dfdx()(x,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_p() returns a wrapper of the original Jacobian dfdx with the additional parameter p")
	    {
	      eqsys.dfdx_p()(x,NULL,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_t() returns a wrapper of the original Jacobian dfdx with the additional parameter t")
	    {
	      eqsys.dfdx_t()(0,x,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_pt() returns a wrapper of the original Jacobian dfdx with the additional parameters t and p")
	    {
	      eqsys.dfdx_pt()(0,x,NULL,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	}
      WHEN("additional functions (funcNames, func) have been provided")
	{
	  ExternalEquationSystem eqsys(varNames,f,dfdx,funcNames,func);
	  double x[]={9.8765,4.3210};
	  eqsys.set("x",9.8765); eqsys.set("y",4.3210);
	  double funcvals_1[3]; double funcvals_2[3];
	  func(x,funcvals_1);
	  
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
	      REQUIRE(eqsys.get("f_1")==std::sin(9.8765+4.3210));
	      REQUIRE(eqsys.get("f_2")==std::cos(9.8765+4.3210));
	      REQUIRE(eqsys.get("f_3")==std::tan(9.8765+4.3210));
	    }
	  THEN("function_values() returns the correct values")
	    {
	      REQUIRE(eqsys.function_values()[0]==std::sin(9.8765+4.3210));
	      REQUIRE(eqsys.function_values()[1]==std::cos(9.8765+4.3210));
	      REQUIRE(eqsys.function_values()[2]==std::tan(9.8765+4.3210));
	    }
	  THEN("func() returns the original function func")
	    {
	      eqsys.func()(x,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_p() returns a wrapper of the original function func with the additional parameter p")
	    {
	      eqsys.func_p()(x,NULL,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_t() returns a wrapper of the original function func with the additional parameter t")
	    {
	      eqsys.func_t()(0,x,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_pt() returns a wrapper of the original function func with the additional parameters t and p")
	    {
	      eqsys.func_pt()(0,x,NULL,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	}
    }
}
