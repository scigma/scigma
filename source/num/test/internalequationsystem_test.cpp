#include <cmath>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <catch.hpp>
#include "../internalequationsystem.hpp"
#include "internalequationsystem_test.hpp"

#define REQUIRE_EXCEPTION(X,Y) {std::string error;try{X;}catch(std::string err){error=err;}REQUIRE(error==Y+error.substr(std::string(Y).size()));}

using namespace scigma::num;

extern const std::string allowed;

extern std::string well_formed_name(int maxLength);
extern std::string ill_formed_name(int maxLength);

bool equals(double value, std::string valString)
{
  double stringVal;
  std::stringstream ss;
  ss<<valString;
  ss>>stringVal;
  if(!(value==Approx(stringVal)))
    {
      std::cerr<<"value: "<<value<<", valString: '"<<valString<<"'"<<std::endl;
      return false;
    }
  return true;
}

SCENARIO ("internal ODEs","[equationsystem][internal]")
{
  std::srand(std::time(NULL));

  GIVEN("an empty InternalEquationSystem")
    {
      InternalEquationSystem eqsys;

      THEN("f(), f_p(), f_t() and f_pt() all return NULL")
	{
	  REQUIRE(!eqsys.f());
	  REQUIRE(!eqsys.f_p());
	  REQUIRE(!eqsys.f_t());
	  REQUIRE(!eqsys.f_pt());
	}
      THEN("dfdx(), dfdx_p(), dfdx_t() and dfdx_pt() all return NULL")
	{
	  REQUIRE(!eqsys.dfdx());
	  REQUIRE(!eqsys.dfdx_p());
	  REQUIRE(!eqsys.dfdx_t());
	  REQUIRE(!eqsys.dfdx_pt());
	}
      THEN("func(), func_p(), func_t() and func_pt() all return NULL")
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
      WHEN("a numerical expression <exp> is parsed")
	THEN("eqsys.parse(<exp>) returns its value as a double")
	{
	  REQUIRE(equals(1.5,eqsys.parse("1.5")));
	  REQUIRE(equals(1-3.1,eqsys.parse("1-3.1")));
	  REQUIRE(equals(std::pow(std::sqrt(std::sin(1.24)),-1.234),eqsys.parse("sqrt(sin(1.24))**(-1.234)")));
	}
      WHEN("time() is called")
	THEN("exception \"cannot get time: system is autonomous\" is thrown")
	REQUIRE_EXCEPTION(eqsys.time(),"cannot get t: system is autonomous");

      WHEN("a (non-existing) variable or parameter is set or retrieved")
	THEN("exception \"no such variable or parameter\" is thrown")
	{
	  REQUIRE_EXCEPTION(eqsys.set("x",1),"no such variable or parameter: x");
	  REQUIRE_EXCEPTION(eqsys.get("x"),"no such variable or parameter: x");
	}
      WHEN("a (non-existing) variable or parameter is deleted")
	THEN("exception ...")
	REQUIRE(eqsys.parse("f1(arg1,arg2)=arg2-arg1")=="");
      WHEN("the parameter x is defined with parse(\"x=2\")")
	{
	  eqsys.parse("x=2");
	  THEN("get(\"x\") returns 2")
	    REQUIRE(eqsys.get("x")==2);
	  THEN("parse(\"$x\") returns \"2\"")
	    REQUIRE(eqsys.parse("$x")=="2");
	  THEN("set(\"x\",3.2) resets x to 3")
	    {
	      eqsys.parse("x=3");
	      REQUIRE(eqsys.parse("$x")=="3");
	    }
	  THEN("parse(\"!x\") deletes x")
	    {
	      eqsys.parse("!x");
	      REQUIRE_EXCEPTION(eqsys.set("x",1),"no such variable or parameter: x");
	    }
	}
      WHEN("the variable x is defined with parse(\"x'=-x\")")
	{
	  eqsys.parse("x'=-x");
	  THEN("the variable value is initialized to 0")
	    REQUIRE(eqsys.get("x")==0);
	  THEN("set(\"x\",-1) resets the variable to -1")
	    {
	      eqsys.set("x",-1);
	      REQUIRE(eqsys.get("x")==-1);
	    }
	}
    }
  GIVEN("a simple non-autonomous ODE x'=x-sin(t)")
    {
      InternalEquationSystem eqsys;
      eqsys.parse("x'=x-sin(t)");
      THEN("is_autonomous() returns false")
	REQUIRE(!eqsys.is_autonomous());
      THEN("time() returns the correct value")
	{
	  eqsys.set("t",-2);
	  REQUIRE(eqsys.time()==-2);
	}
    }
  GIVEN("An autonomous InternalEquationSystem initialized with the script:\n"
	"x'=s*(y-x)\n"
	"y'=x*(r-z)-y\n"
	"z'=x*y-b*z\n"
	"r=28.0\n"
	"s=10.0\n"
	"b=2.66666667\n"
	"f_1=x**2-y**2\n"
	"f_2=sqrt(f1+z)\n"
	"c=r+s*sin(b)\n"
	"x=-1.24\n"
	"y=$c*12\n"
	"z=-tan(23)\n")
    {
      InternalEquationSystem eqsys;
      for(size_t i(0);i<lorenzLines;++i)
	eqsys.parse(lorenz[i]);

      double x[]={3,4,5};
      double p[]={6,7,8};
      double rhs[3];
      double jac[9];
      double dfdp[9];
      double func[2];
      
      THEN("is_autonomous() returns true")
	REQUIRE(eqsys.is_autonomous());
           
      THEN("n_variables(), n_parameters(), n_functions() and n_constants() return the correct values")
	{
	  REQUIRE(eqsys.n_variables()==3);
	  REQUIRE(eqsys.n_parameters()==3);
	  REQUIRE(eqsys.n_functions()==2);
	  REQUIRE(eqsys.n_constants()==1);
	}
      THEN("variable_names(), parameter_names(), function_names() and constant_names() return the correct values in alphabetical order")
	{
	  REQUIRE(eqsys.variable_names()[0]=="x");
	  REQUIRE(eqsys.variable_names()[1]=="y");
	  REQUIRE(eqsys.variable_names()[2]=="z");
	  REQUIRE(eqsys.parameter_names()[0]=="b");
	  REQUIRE(eqsys.parameter_names()[1]=="r");
	  REQUIRE(eqsys.parameter_names()[2]=="s");
	  REQUIRE(eqsys.function_names()[0]=="f_1");
	  REQUIRE(eqsys.function_names()[1]=="f_2");
	  REQUIRE(eqsys.constant_names()[0]=="c");
	}
      THEN("variable_values(), parameter_values(), function_values() and constant_values() return the correct values")
	{
	  REQUIRE(eqsys.variable_values()[0]==-1.24);
	  REQUIRE(eqsys.variable_values()[1]==(28+10*std::sin(2.66666667))*12);
	  REQUIRE(eqsys.variable_values()[2]==-std::tan(23));
	  REQUIRE(eqsys.parameter_values()[0]==2.66666667);
	  REQUIRE(eqsys.parameter_values()[1]==28);
	  REQUIRE(eqsys.parameter_values()[2]==10);
	  REQUIRE(eqsys.function_values()[0]==std::pow(-1.24,2)-std::pow((28+10*std::sin(2.66666667))*12,2)+10);
	  REQUIRE(eqsys.function_values()[1]==std::sqrt(std::abs(std::pow(-1.24,2)-std::pow((28+10*std::sin(2.66666667))*12,2)+10-std::tan(23))));
	  REQUIRE(eqsys.constant_values()[0]==28+10*std::sin(2.66666667));
	}
      WHEN("we try to delete a function (f_1) that another function (f_2) depends on")
	THEN("exception \"f_2 depends on f_1 - delete f_2 first\" is thrown")
	REQUIRE_EXCEPTION(eqsys.parse("!f_1"),"f_2 depends on f_1 - delete f_2 first");
      
      THEN("f() returns a wrapper of the original right hand side function f_p for current parameters")
	{
	  eqsys.f()(x,rhs);
	  REQUIRE(rhs[0]==10*(4-3));
	  REQUIRE(rhs[1]==3*(28-5)-4);
	  REQUIRE(rhs[2]==3*4-2.66666667*5);
	}
      THEN("f_p() returns the original right hand side function f_p")
	{
	  eqsys.f_p()(x,p,rhs);
	  REQUIRE(rhs[0]==8*(4-3));
	  REQUIRE(rhs[1]==3*(7-5)-4);
	  REQUIRE(rhs[2]==3*4-6*5);
	}
      THEN("dfdx() returns a wrapper of the original Jacobian dfdx_pt for current parameters")
	{
	  eqsys.dfdx()(x,jac);
	  REQUIRE(jac[0]==-10);
	  REQUIRE(jac[1]==28-5);
	  REQUIRE(jac[2]==4);
	  REQUIRE(jac[3]==10);
	  REQUIRE(jac[4]==-1);
	  REQUIRE(jac[5]==3);
	  REQUIRE(jac[6]==0);
	  REQUIRE(jac[7]==-3);
	  REQUIRE(jac[8]==-2.66666667);
	}
      THEN("dfdx_p() returns the original Jacobian dfdx_p")
	{
	  eqsys.dfdx_p()(x,p,jac);
	  REQUIRE(jac[0]==-8);
	  REQUIRE(jac[1]==7-5);
	  REQUIRE(jac[2]==4);
	  REQUIRE(jac[3]==8);
	  REQUIRE(jac[4]==-1);
	  REQUIRE(jac[5]==3);
	  REQUIRE(jac[6]==0);
	  REQUIRE(jac[7]==-3);
	  REQUIRE(jac[8]==-6);
	}
      THEN("dfdx_p() returns the original sensitivity function dfdx_p")
	{
	  eqsys.dfdp_p()(x,p,dfdp);
	  REQUIRE(dfdp[0]==0);
	  REQUIRE(dfdp[1]==0);
	  REQUIRE(dfdp[2]==-5);
	  REQUIRE(dfdp[3]==0);
	  REQUIRE(dfdp[4]==3);
	  REQUIRE(dfdp[5]==0);
	  REQUIRE(dfdp[6]==1);
	  REQUIRE(dfdp[7]==0);
	  REQUIRE(dfdp[8]==0);
	}
      THEN("func() returns a wrapper of the original function func_p for the current parameters")
	{
	  eqsys.func()(x,func);
	  REQUIRE(func[0]==std::pow(3,2)-std::pow(4,2)+10);
	  REQUIRE(func[1]==std::sqrt(std::abs((std::pow(3,2)-std::pow(4,2)+10+5))));	  
	}
      THEN("func_p() returns the original function func_p")
	{
	  eqsys.func_p()(x,p,func);
	  REQUIRE(func[0]==std::pow(3,2)-std::pow(4,2)+8);
	  REQUIRE(func[1]==std::sqrt(std::abs((std::pow(3,2)-std::pow(4,2)+8+5))));
	}
    }
  GIVEN("A non-autonomous InternalEquationSystem initialized with the script:\n"
	"x'=-y+a*f1+b*(1-x)\n"
	"y'=x+a*f2+b*(1-y)\n"
	"f1=sin(omega*t)\n"
	"f2=cos(omega*t)\n"
	"t=-2\n"
	"x=0.5\n"
	"y=0.6\n"
	"a=0.001\n"
	"b=0.1\n"
	"omega=5")
    {
      InternalEquationSystem eqsys;
      for(size_t i(0);i<forcedLines;++i)
	eqsys.parse(forced[i]);

      double t(3);
      double x[]={1.1,1.2};
      double p[]={0.002,0.3,10};
      double rhs[2];
      double jac[4];
      double dfdp[6];
      double func[2];
      
      THEN("is_autonomous() returns false")
	REQUIRE(!eqsys.is_autonomous());
           
      THEN("n_variables(), n_parameters(), n_functions() and n_constants() return the correct values")
	{
	  REQUIRE(eqsys.n_variables()==2);
	  REQUIRE(eqsys.n_parameters()==3);
	  REQUIRE(eqsys.n_functions()==2);
	  REQUIRE(eqsys.n_constants()==0);
	}
      THEN("variable_names(), parameter_names(), function_names() and constant_names() return the correct values in alphabetical order")
	{
	  REQUIRE(eqsys.variable_names()[0]=="x");
	  REQUIRE(eqsys.variable_names()[1]=="y");
	  REQUIRE(eqsys.parameter_names()[0]=="a");
	  REQUIRE(eqsys.parameter_names()[1]=="b");
	  REQUIRE(eqsys.parameter_names()[2]=="omega");
	  REQUIRE(eqsys.function_names()[0]=="f1");
	  REQUIRE(eqsys.function_names()[1]=="f2");
	}
      THEN("setting and retrieving t by name works")
	{
	  REQUIRE(eqsys.get("t")==-2);
	  eqsys.set("t",5);
	  REQUIRE(eqsys.get("t")==5);
	}
      THEN("time() returns the correct value")
	{
	  REQUIRE(eqsys.time()==-2);
	}
      THEN("variable_values(), parameter_values(), function_values() and constant_values() return the correct values")
	{
	  REQUIRE(eqsys.variable_values()[0]==0.5);
	  REQUIRE(eqsys.variable_values()[1]==0.6);
	  REQUIRE(eqsys.parameter_values()[0]==0.001);
	  REQUIRE(eqsys.parameter_values()[1]==0.1);
	  REQUIRE(eqsys.parameter_values()[2]==5);
	  REQUIRE(eqsys.function_values()[0]==std::cos(5*-2));
	  REQUIRE(eqsys.function_values()[1]==std::sin(5*-2));
	}

      THEN("f() returns a wrapper of the original right hand side function f_pt for current parameters and time")
	{
	  eqsys.f()(x,rhs);
	  REQUIRE(rhs[0]==-1.2+0.001*std::cos(5*-2)+0.1*(1-1.1));
	  REQUIRE(rhs[1]==1.1+0.001*std::sin(5*-2)+0.1*(1-1.2));
	}
      THEN("f_p() returns a wrapper of the original right hand side function f_pt for the current time")
	{
	  eqsys.f_p()(x,p,rhs);
	  REQUIRE(rhs[0]==-1.2+0.002*std::cos(10*-2)+0.3*(1-1.1));
	  REQUIRE(rhs[1]==1.1+0.002*std::sin(10*-2)+0.3*(1-1.2));
	}
      THEN("f_t() returns a wrapper of the original right hand side function f_pt for the current parameters")
	{
	  eqsys.f_t()(t,x,rhs);
	  REQUIRE(rhs[0]==-1.2+0.001*std::cos(5*t)+0.1*(1-1.1));
	  REQUIRE(rhs[1]==1.1+0.001*std::sin(5*t)+0.1*(1-1.2));
	}
      THEN("f_pt() returns the original right hand side function f_pt")
	{
	  eqsys.f_pt()(t,x,p,rhs);
	  REQUIRE(rhs[0]==-1.2+0.002*std::cos(10*t)+0.3*(1-1.1));
	  REQUIRE(rhs[1]==1.1+0.002*std::sin(10*t)+0.3*(1-1.2));
	}
      THEN("dfdx() returns a wrapper of the original Jacobian dfdx_pt for current parameters and time")
	{
	  eqsys.dfdx()(x,jac);
	  REQUIRE(jac[0]==-0.1);
	  REQUIRE(jac[1]==1);
	  REQUIRE(jac[2]==-1);
	  REQUIRE(jac[3]==-0.1);
	}
      THEN("dfdx_p() returns a wrapper of the original Jacobian dfdx_pt for the current time")
	{
	  eqsys.dfdx_p()(x,p,jac);
	  REQUIRE(jac[0]==-0.3);
	  REQUIRE(jac[1]==1);
	  REQUIRE(jac[2]==-1);
	  REQUIRE(jac[3]==-0.3);
	}
      THEN("dfdx_t() returns a wrapper of the original Jacobian dfdx_pt for the current parameters")
	{
	  eqsys.dfdx_t()(t,x,jac);
	  REQUIRE(jac[0]==-0.1);
	  REQUIRE(jac[1]==1);
	  REQUIRE(jac[2]==-1);
	  REQUIRE(jac[3]==-0.1);
	}
      THEN("dfdx_pt() returns the original Jacobian dfdx_pt")
	{
	  eqsys.dfdx_pt()(t,x,p,jac);
	  REQUIRE(jac[0]==-0.3);
	  REQUIRE(jac[1]==1);
	  REQUIRE(jac[2]==-1);
	  REQUIRE(jac[3]==-0.3);
	}
      THEN("dfdx_p() returns a wrapper of the original sensitivity function dfdx_pt for the current time")
	{
	  eqsys.dfdp_p()(x,p,dfdp);
	  REQUIRE(dfdp[0]==std::cos(10*-2));
	  REQUIRE(dfdp[1]==std::sin(10*-2));
	  REQUIRE(dfdp[2]==1-1.1);
	  REQUIRE(dfdp[3]==1-1.2);
	  REQUIRE(dfdp[4]==-0.002*std::sin(10*-2)*-2);
	  REQUIRE(dfdp[5]==0.002*std::cos(10*-2)*-2);
	}
      THEN("dfdx_pt() returns the original sensitivity function dfdx_pt")
	{
	  eqsys.dfdp_pt()(t,x,p,dfdp);
	  REQUIRE(dfdp[0]==std::cos(10*t));
	  REQUIRE(dfdp[1]==std::sin(10*t));
	  REQUIRE(dfdp[2]==1-1.1);
	  REQUIRE(dfdp[3]==1-1.2);
	  REQUIRE(dfdp[4]==-0.002*std::sin(10*t)*t);
	  REQUIRE(dfdp[5]==0.002*std::cos(10*t)*t);
	}

      THEN("func() returns a wrapper of the original function func_pt for the current parameters")
	{
	  eqsys.func()(x,func);
	  REQUIRE(func[0]==std::cos(5*-2));
	  REQUIRE(func[1]==std::sin(5*-2));
	}
      THEN("func_p() returns a wrapper of the original function func_pt for the current time")
	{
	  eqsys.func_p()(x,p,func);
	  REQUIRE(func[0]==std::cos(10*-2));
	  REQUIRE(func[1]==std::sin(10*-2));
	}
      THEN("func_t() returns a wrapper of the original function func_pt for the current parameters")
	{
	  eqsys.func_t()(t,x,func);
	  REQUIRE(func[0]==std::cos(5*t));
	  REQUIRE(func[1]==std::sin(5*t));
	}
      THEN("func_pt() returns the original function func_pt")
	{
	  eqsys.func_pt()(t,x,p,func);
	  REQUIRE(func[0]==std::cos(10*t));
	  REQUIRE(func[1]==std::sin(10*t));
	}

    }
}
