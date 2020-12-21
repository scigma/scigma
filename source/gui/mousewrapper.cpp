#include <map>
#include "mouse.hpp"

using namespace scigma::common;
using namespace scigma::gui;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"

class ScigmaGuiRightClickEventMonitor:
    public EventSink<RightClickEvent>::Type
  {
  private:
    void (*python_callback_)();
  public:
    ScigmaGuiRightClickEventMonitor(void (*python_callback)()):python_callback_(python_callback){}
    virtual ~ScigmaGuiRightClickEventMonitor();
    virtual bool process(RightClickEvent /*event*/)
    {python_callback_();return true;}
  };
  ScigmaGuiRightClickEventMonitor::~ScigmaGuiRightClickEventMonitor()
  {}
  /* this map stores a ScigmaGuiRightClickEventMonitor for each Mouse when it is created;
     on destruction of the Mouse object, these are destroyed as well.
  */
  std::map<Mouse*,ScigmaGuiRightClickEventMonitor*> scigmaGuiRightClickEventMonitorMap;

extern "C"
{
  PythonID scigma_gui_create_mouse(void (*python_callback)())
  {
    Mouse* ptr=new Mouse();
    ScigmaGuiRightClickEventMonitor* ptr2(new ScigmaGuiRightClickEventMonitor(python_callback));
    connect<RightClickEvent>(ptr,ptr2);
    connect<RightClickEvent>(ptr,ptr2);
    scigmaGuiRightClickEventMonitorMap.insert(std::pair<Mouse*,ScigmaGuiRightClickEventMonitor*>(ptr,ptr2));
    return ptr->get_python_id();
  }
  void scigma_gui_destroy_mouse(PythonID objectID)
  {
    PYOBJ(Mouse,ptr,objectID);
    if(ptr)
      {
	std::map<Mouse*,ScigmaGuiRightClickEventMonitor*>::iterator i(scigmaGuiRightClickEventMonitorMap.find(static_cast<Mouse*>(ptr)));
	ScigmaGuiRightClickEventMonitor* ptr2((*i).second);
	scigmaGuiRightClickEventMonitorMap.erase(i);
	delete ptr2;
	delete ptr;
      }
  }
  
} /* end extern "C" block */

#pragma clang diagnostic pop
