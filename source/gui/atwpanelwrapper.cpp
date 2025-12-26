#include <map>
#include "atwpanel.hpp"
#include "glwindow.hpp"

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
  class ScigmaGuiATWPanelEventMonitor:
    public EventSink<EntryChangeEvent>::Type,
    public EventSink<ButtonEvent>::Type
  {
  private:
    void (*python_callback_)(const char*, const void*);
  public:
    ScigmaGuiATWPanelEventMonitor(void (*python_callback)(const char*, const void*)):
      python_callback_(python_callback)
    {}
    virtual ~ScigmaGuiATWPanelEventMonitor();
    bool process(EntryChangeEvent event, const char* id, const void* ptr) override
    {python_callback_(id,ptr);return true;}
    bool process(ButtonEvent event, const char* id) override
    {python_callback_(id,NULL);return true;}
  };
  ScigmaGuiATWPanelEventMonitor::~ScigmaGuiATWPanelEventMonitor(){}
  /* this map stores a ScigmaGuiATWEventMonitor for each ATWPanel when it is created;
     on destruction of the ATWPanel object, these are destroyed as well.
  */
  std::map<ATWPanel*,ScigmaGuiATWPanelEventMonitor*> scigmaGuiATWPanelEventMonitorMap;

  PythonID scigma_gui_create_atw_panel(PythonID windowID, const char* title,void (*python_callback)(const char*,const void*))
  {
    PYOBJ(GLWindow,window,windowID);
    if(window)
      {
	ATWPanel* ptr1(new ATWPanel(window->gl_context(),title));
	ScigmaGuiATWPanelEventMonitor* ptr2(new ScigmaGuiATWPanelEventMonitor(python_callback));
	connect<EntryChangeEvent>(ptr1,ptr2);
	connect<ButtonEvent>(ptr1,ptr2);
	scigmaGuiATWPanelEventMonitorMap.insert(std::pair<ATWPanel*,ScigmaGuiATWPanelEventMonitor*>(ptr1,ptr2));
	return ptr1->get_python_id();
      }
    else
      return -1;
  }

  void scigma_gui_destroy_atw_panel(PythonID objectID)
  {
    PYOBJ(ATWPanel,ptr1,objectID);
    if(ptr1)
      {
	std::map<ATWPanel*,ScigmaGuiATWPanelEventMonitor*>::iterator i(scigmaGuiATWPanelEventMonitorMap.find(ptr1));
	ScigmaGuiATWPanelEventMonitor* ptr2((*i).second);
	scigmaGuiATWPanelEventMonitorMap.erase(i);
	disconnect<EntryChangeEvent>(ptr1,ptr2);
	disconnect<ButtonEvent>(ptr1,ptr2);
	delete ptr2;
	delete ptr1;
      }
  }

  void scigma_gui_atw_panel_add_bool(PythonID atwPanelID, const char* identifier,bool forward,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_bool(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_int(PythonID atwPanelID, const char* identifier,bool forward,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_int(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_float(PythonID atwPanelID, const char* identifier,bool forward,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_float(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_double(PythonID atwPanelID, const char* identifier,bool forward,const char* defs)
  {
    PYOBJ(ATWPanel,ptr,atwPanelID);
    if(ptr)
      ptr->add_double(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_color(PythonID atwPanelID, const char* identifier,bool forward,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_color(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_direction(PythonID atwPanelID, const char* identifier,bool forward,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_direction(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_enum(PythonID atwPanelID, const char* identifier,const char* labels, int* values, bool forward,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_enum(identifier,labels,values,forward,defs);
  }

  void scigma_gui_atw_panel_add_string(PythonID atwPanelID, const char* identifier, bool forward, const char* defs)
  {
    PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_string(identifier,forward,defs);
  }

  void scigma_gui_atw_panel_add_button(PythonID atwPanelID, const char* identifier, const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_button(identifier,defs);
  }

  void scigma_gui_atw_panel_remove(PythonID atwPanelID, const char* identifier)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->remove(identifier);
  }

  void scigma_gui_atw_panel_set_bool(PythonID atwPanelID, const char* identifier, int value)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_bool(identifier,value);
  }

  void scigma_gui_atw_panel_set_int(PythonID atwPanelID, const char* identifier, int value)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_int(identifier,value);
  }

  void scigma_gui_atw_panel_set_float(PythonID atwPanelID, const char* identifier, GLfloat value)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_float(identifier,value);
  }

  void scigma_gui_atw_panel_set_double(PythonID atwPanelID, const char* identifier, double value)
  {
    PYOBJ(ATWPanel,ptr,atwPanelID);
    if(ptr)
      ptr->set_double(identifier,value);
  }

  void scigma_gui_atw_panel_set_color(PythonID atwPanelID, const char* identifier, GLfloat* values)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_color(identifier,values);
  }

  void scigma_gui_atw_panel_set_direction(PythonID atwPanelID, const char* identifier,GLfloat* values)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_direction(identifier,values);
  }

  void scigma_gui_atw_panel_set_enum(PythonID atwPanelID, const char* identifier,int value)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_enum(identifier,value);
  }

  void scigma_gui_atw_panel_set_string(PythonID atwPanelID, const char* identifier, const char* entry)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->set_string(identifier,entry);
  }

  void scigma_gui_atw_panel_define(PythonID atwPanelID,const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->define(defs);
  }

  void scigma_gui_atw_panel_define_entry(PythonID atwPanelID, const char* identifier, const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->define(identifier,defs);
  }

  void scigma_gui_atw_panel_add_separator(PythonID atwPanelID, const char* identifier, const char* defs)
  {
     PYOBJ(ATWPanel,ptr,atwPanelID);
     if(ptr)
       ptr->add_separator(identifier,defs);
  }
  
} /* end extern "C" block */

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
