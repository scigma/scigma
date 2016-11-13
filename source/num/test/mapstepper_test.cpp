#include <iostream>
#include <vector>
#include <catch.hpp>
#include "../mapstepper.hpp"
#include "../externalequationsystem.hpp"
#include "externalequationsystem_test_shared.hpp"

using namespace scigma::num;

SCENARIO("Testing MapStepper","[stepper][map]")
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

  
  GIVEN("A MapStepper initialized with an EquationSystem without Jacobian())")
    {
      ExternalEquationSystem eqsys(varNames,f_t,NULL,funcNames,func_t);
      eqsys.set("t",3.0);
      eqsys.set("x",1.2345); eqsys.set("y",6.7890);
      MapStepper stepper(eqsys);
      MapStepper backStepper(eqsys,false);
      
      THEN("n_variables() returns the correct number of variables")
	REQUIRE(stepper.n_variables()==eqsys.n_variables());
      THEN("n_functions() returns the correct number of functions")
	REQUIRE(stepper.n_functions()==eqsys.n_functions());
      WHEN("advance() is called")
	{
	  stepper.advance();
	  THEN("t() is increased by 1.0")
	    REQUIRE(stepper.t()==eqsys.time()+1.0);
	  THEN("new x() and func() values are correctly mapped from the initial conditions")
	    {
	      double rhs[2];
	      f_t(eqsys.time(),eqsys.variable_values(),rhs);
	      std::cerr<<eqsys.time()<<"..."<<eqsys.variable_values()[0]<<"..."<<eqsys.variable_values()[1]<<std::endl;
	      std::cerr<<rhs[0]<<"..."<<rhs[1]<<std::endl;

	      double funcVals[3];
	      func_t(eqsys.time()+1.0,rhs,funcVals);
	      std::cerr<<rhs[0]<<"..."<<rhs[1]<<std::endl;
	      for(size_t i(0);i<stepper.n_variables();++i)
		REQUIRE(stepper.x()[i]==rhs[i]);
	      for(size_t i(0);i<stepper.n_functions();++i)
		REQUIRE(stepper.func()[i]==funcVals[i]);
	    }
	  THEN("reset(...) changes the initial conditions")
	    {
	      stepper.reset(eqsys.time(),eqsys.variable_values());
	      REQUIRE(stepper.t()==eqsys.time());
	      for(size_t i(0);i<stepper.n_variables();++i)
		REQUIRE(stepper.x()[i]==eqsys.variable_values()[i]);
	      for(size_t i(0);i<stepper.n_functions();++i)
		REQUIRE(stepper.func()[i]==eqsys.function_values()[i]);
	    }
	}
    }
}
