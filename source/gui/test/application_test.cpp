#include "../application.hpp"
#include <catch.hpp>

using scigma::gui::Application;
using scigma::gui::LoopEvent;
using scigma::gui::IdleEvent;
using scigma::common::EventSource;
using scigma::common::EventSink;
using scigma::common::connect;

class TestEvents:
  public EventSink<LoopEvent>::Type,
  public EventSink<IdleEvent>::Type 
{
public:
  size_t loopCount;
  double idleEventTime;

  TestEvents():loopCount(0),idleEventTime(-1)
  {}
  
  virtual bool process(LoopEvent event)
  {
    ++loopCount;
    return true;
  }
  virtual bool process(IdleEvent event,double time)
  {
    idleEventTime=time;
    return true;
  }
};

  
SCENARIO("Application: initialization and termination","[Application]")
{

  GIVEN("The single instance of Application has been initialized")
    {
      Application* app(Application::get_instance());
      THEN("calling the event loop works without errors")
	{
	  app->loop(0.1);
	  REQUIRE(true);
	}
      WHEN("An EventSink for LoopEvent is connected")
	{
	  TestEvents t;
	  connect<LoopEvent>(app,&t);
	  THEN("for each loop, the LoopEvent is emitted and processed correctly")
	    {
	      app->loop(0);
	      app->loop(0);
	      app->loop(0);
	      REQUIRE(t.loopCount==3);
	    }
	}
      WHEN("An EventSink for IdleEvent is connected")
	{
	  TestEvents t;
	  connect<IdleEvent>(app,&t);

	  THEN("if the loop does not run idle, no IdleEvent is emitted")
	    {
	      app->loop(0);
	      app->loop(0);
	      app->loop(0);
	      REQUIRE(t.idleEventTime==-1);
	    }
	  THEN("if the loop runs idle, IdleEvents are emitted and processed")
	    {
	      app->loop(0.1);
	      REQUIRE(t.idleEventTime!=-1);
	    }
	}
      WHEN("An error is pushed to the Application log with push_error(...)")
	{
	  app->push_error("this is an error message!");
	  THEN("It is retrieved with pop_error()")
	    REQUIRE(app->pop_error()=="this is an error message!");
	}
    }
}
