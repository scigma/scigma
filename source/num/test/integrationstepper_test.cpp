#include <iostream>
#include <catch.hpp>
#include "../integrationstepper.hpp"
#include "../internalequationsystem.hpp"
#include "internalequationsystem_test.hpp"

using namespace scigma::num;

SCENARIO("Testing IntegrationStepper against Odessa","[stepper][integration]")
{
  InternalEquationSystem eqsys;
  for(size_t i(0);i<forcedLines;++i)
    eqsys.parse(forced[i]);

  GIVEN("An IntegrationStepper initialized with an EquationSystem and "
	"an Instance of Odessa based on the same EquationSystem")
    {
      IntegrationStepper stepper(eqsys,.1);
      Odessa odessa(eqsys.n_variables(),eqsys.f_pt(),eqsys.dfdx_pt(),eqsys.n_variables());

      THEN("n_variables() returns the correct number of variables")
	REQUIRE(stepper.n_variables()==eqsys.n_variables());
      THEN("n_functions() returns the correct number of functions")
	REQUIRE(stepper.n_functions()==eqsys.n_functions());
      WHEN("IntegrationStepper and Odessa run for the same time interval "
	   "from the same initial conditions")
	{
	  odessa.t()=-2;
	  odessa.x()[0]=0.5;
	  odessa.x()[1]=0.6;
	  odessa.sensitivity()[0]=1;
	  odessa.sensitivity()[1]=0;
	  odessa.sensitivity()[2]=0;
	  odessa.sensitivity()[3]=1;
	  odessa.integrate(.1,1000);
	  stepper.advance(1000);
	  
	  THEN("The evolved values in IntegrationStepper and Odessa are the same")
	    {
	      REQUIRE(stepper.t()==odessa.t());
	      for(size_t i(0);i<stepper.n_variables();++i)
		REQUIRE(stepper.x()[i]==odessa.x()[i]);
	      for(size_t i(0);i<stepper.n_variables()*stepper.n_variables();++i)
		REQUIRE(stepper.jac()[i]==odessa.sensitivity()[i]);
	    }
	  THEN("reset(...) changes the initial conditions")
	    {
	      stepper.reset(eqsys.time(),eqsys.variable_values());
	      REQUIRE(stepper.t()==eqsys.time());
	      for(size_t i(0);i<stepper.n_variables();++i)
		REQUIRE(stepper.x()[i]==eqsys.variable_values()[i]);
	      for(size_t i(0);i<stepper.n_variables()*stepper.n_variables();++i)
		REQUIRE(stepper.jac()[i]==(i%(stepper.n_variables()+1)?0:1));
	    }
	  THEN("rerunning stepper after reset(...) to the same initial "
	       "conditions gives the same results")
	    {
	      stepper.reset(eqsys.time(),eqsys.variable_values());
	      stepper.advance(1000);
	      REQUIRE(stepper.t()==odessa.t());
	      for(size_t i(0);i<stepper.n_variables();++i)
		REQUIRE(stepper.x()[i]==odessa.x()[i]);
	      for(size_t i(0);i<stepper.n_variables()*stepper.n_variables();++i)
		REQUIRE(stepper.jac()[i]==odessa.sensitivity()[i]);
	    }
	}
    }
}
