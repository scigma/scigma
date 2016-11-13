#include <map>
#include "picker.hpp"

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
    class ScigmaGuiPickerEventMonitor:public EventSink<PickEvent>::Type
  {
  private:
    void (*python_callback_)(bool,GLfloat,GLfloat,GLfloat);
  public:
    ScigmaGuiPickerEventMonitor(void (*python_callback)(bool,GLfloat,GLfloat,GLfloat)):python_callback_(python_callback){}
    virtual ~ScigmaGuiPickerEventMonitor();
    virtual bool process(PickEvent event,bool ctrl,GLfloat x, GLfloat y, GLfloat z)
    {python_callback_(ctrl,x,y,z);return true;}
    };
    ScigmaGuiPickerEventMonitor::~ScigmaGuiPickerEventMonitor(){}
  /* this map stores a ScigmaGuiPickerEventMonitor for each Picker when it is created;
     on destruction of the Picker object, these are destroyed as well.
  */
  std::map<Picker*,ScigmaGuiPickerEventMonitor*> scigmaGuiPickerEventMonitorMap;

  PythonID scigma_gui_create_picker(int view,void (*python_callback)(bool,GLfloat,GLfloat,GLfloat))
  {
    Picker* ptr1=new Picker(view);
    ScigmaGuiPickerEventMonitor* ptr2(new ScigmaGuiPickerEventMonitor(python_callback));
    connect<PickEvent>(ptr1,ptr2);
    scigmaGuiPickerEventMonitorMap.insert(std::pair<Picker*,ScigmaGuiPickerEventMonitor*>(ptr1,ptr2));
    return ptr1->get_python_id();
  }
  void scigma_gui_destroy_picker(PythonID objectID)
  {
    PYOBJ(Picker,ptr1,objectID);
    if(ptr1)
      {
	std::map<Picker*,ScigmaGuiPickerEventMonitor*>::iterator i(scigmaGuiPickerEventMonitorMap.find(ptr1));
	ScigmaGuiPickerEventMonitor* ptr2((*i).second);
	scigmaGuiPickerEventMonitorMap.erase(i);
	disconnect<PickEvent>(ptr1,ptr2);
	delete ptr2;
	delete ptr1;
      }
  }
  
} /* end extern "C" block */

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
