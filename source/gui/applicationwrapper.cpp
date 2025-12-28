#include "application.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace scigma::common;
using namespace scigma::gui;

extern "C"
{
  /* as Python code cannot directly receive Applications's LoopEvent or IdleEvent,
     this helper class is used as a proxy.
  */
  class ScigmaGuiApplicationEventMonitor:public EventSink<LoopEvent>::Type,public EventSink<IdleEvent>::Type
  {
  public:

    void(*python_loop_callback)();
    void(*python_idle_callback)();
    
    ScigmaGuiApplicationEventMonitor():
      python_loop_callback(NULL),
      python_idle_callback(NULL)
    {}
    virtual ~ScigmaGuiApplicationEventMonitor();
    bool process(LoopEvent event) override
    {python_loop_callback();return false;}
    bool process(IdleEvent event,double time) override
    {python_idle_callback();return false;}

  };
  ScigmaGuiApplicationEventMonitor::~ScigmaGuiApplicationEventMonitor()
  {}
  ScigmaGuiApplicationEventMonitor scigmaGuiApplicationEventMonitor;
  void scigma_gui_application_init()
  {
    static bool initialized(false);
    if(!initialized)
      {
	initialized=true;
	//Application::get_instance()->EventSource<IdleEvent>::Type::connect(new ScigmaGuiApplicationEventMonitor(python_callback));
      }
  }
  void scigma_gui_application_loop(double seconds){Application::get_instance()->loop(seconds);}
  void scigma_gui_application_break_loop(){Application::get_instance()->break_loop();}
  void scigma_gui_application_idle(double seconds){Application::get_instance()->idle(seconds);}
  void scigma_gui_application_sleep(){Application::get_instance()->sleep();}
  void scigma_gui_application_wake(){Application::get_instance()->wake();}
  bool scigma_gui_application_is_sleeping(){return Application::get_instance()->is_sleeping();}
  void scigma_gui_application_set_loop_callback(void(*python_callback)())
  {
    if(python_callback)
      {
	scigmaGuiApplicationEventMonitor.python_loop_callback=python_callback;
	connect<LoopEvent>(Application::get_instance(),&scigmaGuiApplicationEventMonitor);
      }
    else
      {
	disconnect<LoopEvent>(Application::get_instance(),&scigmaGuiApplicationEventMonitor);
	scigmaGuiApplicationEventMonitor.python_loop_callback=python_callback;
      }
  }
  void scigma_gui_application_set_idle_callback(void(*python_callback)())
  {
    if(python_callback)
      {
	scigmaGuiApplicationEventMonitor.python_idle_callback=python_callback;
	connect<LoopEvent>(Application::get_instance(),&scigmaGuiApplicationEventMonitor);
      }
    else
      {
	disconnect<LoopEvent>(Application::get_instance(),&scigmaGuiApplicationEventMonitor);
	scigmaGuiApplicationEventMonitor.python_idle_callback=python_callback;
      }
  }
} /* end extern "C" block */

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

