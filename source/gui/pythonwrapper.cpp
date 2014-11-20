#include <map>
#include <algorithm>
#include <iostream>
#include "../util.h"
#include "glutil.h"
#include "definitions.h"
#include "application.h"
#include "glwindow.h"
#include "navigator.h"
#include "console.h"
#include "atwpanel.h"
#include "graph.h"
#include "marker.h"
#include "curve.h"
#include "cosy.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace scigma;
using namespace scigma::gui;

/* Macros to retrieve object pointers from PythonIDs
   and call member functions of these objects.
*/
#define PYOBJ(cls,name,pyID) cls* name(PythonObject<cls>::get_pointer(pyID))
#define PYCLL(cls,pyID,func) PYOBJ(cls,ptr,pyID);if(ptr){ptr->func;}
extern "C"
{

  // wrappers for the Application class
  /* as Python code cannot directly receive Applications's IdleEvent, this helper class
     is used as a proxy.
  */
  class ScigmaGuiApplicationEventMonitor:public EventSink<LoopEvent>::Type,public EventSink<IdleEvent>::Type
  {
  public:

    void(*python_loop_callback)();
    void(*python_idle_callback)();

    ScigmaGuiApplicationEventMonitor()     
      :python_loop_callback(NULL),
       python_idle_callback(NULL){}
    virtual ~ScigmaGuiApplicationEventMonitor();
    virtual bool process(LoopEvent event)
    {python_loop_callback();return false;}
    virtual bool process(IdleEvent event,double time)
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
  void scigma_gui_application_set_loop_callback(void(*python_callback)())
  {
    if(python_callback)
      {
	scigmaGuiApplicationEventMonitor.python_loop_callback=python_callback;
	Application::get_instance()->EventSource<LoopEvent>::Type::connect(&scigmaGuiApplicationEventMonitor);
      }
    else
      {
	Application::get_instance()->EventSource<LoopEvent>::Type::disconnect(&scigmaGuiApplicationEventMonitor);
	scigmaGuiApplicationEventMonitor.python_loop_callback=python_callback;
      }
  }
  void scigma_gui_application_set_idle_callback(void(*python_callback)())
  {
    if(python_callback)
      {
	scigmaGuiApplicationEventMonitor.python_idle_callback=python_callback;
	Application::get_instance()->EventSource<IdleEvent>::Type::connect(&scigmaGuiApplicationEventMonitor);
      }
    else
      {
	Application::get_instance()->EventSource<IdleEvent>::Type::disconnect(&scigmaGuiApplicationEventMonitor);
	scigmaGuiApplicationEventMonitor.python_idle_callback=python_callback;
      }
  }
  
  /* wrappers for functions of the GLWindow class;
     note that these include wrappers for the functionality
     of ViewingArea, ViewingVolume and GLContext, as these
     are always part of a GLWindow instance. 
  */
  // GLWindow functionality
  PythonID scigma_gui_create_gl_window(){GLWindow* ptr=new GLWindow;return ptr->get_python_id();}
  void scigma_gui_destroy_gl_window(PythonID objectID){PYOBJ(GLWindow,ptr,objectID);if(ptr)delete ptr;}

  void scigma_gui_gl_window_connect_navigator(PythonID windowID, PythonID navigatorID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Navigator,navigator,navigatorID);
    if(window&&navigator)
      {
	window->EventSource<MouseButtonEvent>::Type::connect(navigator);
	window->EventSource<ScrollEvent>::Type::connect(navigator);
      }
  }
  void scigma_gui_gl_window_connect_navigator_before(PythonID windowID, PythonID navigatorID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Navigator,navigator,navigatorID);
    if(window&&navigator)
      {
	window->EventSource<MouseButtonEvent>::Type::connect_before(navigator);
	window->EventSource<ScrollEvent>::Type::connect_before(navigator);
      }
  }
  void scigma_gui_gl_window_disconnect_navigator(PythonID windowID, PythonID navigatorID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Navigator,navigator,navigatorID);
    if(window&&navigator)
      {
	window->EventSource<MouseButtonEvent>::Type::disconnect(navigator);
	window->EventSource<ScrollEvent>::Type::disconnect(navigator);
      }
  }
  void scigma_gui_gl_window_connect_console(PythonID windowID, PythonID consoleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Console,console,consoleID);
    if(window&&console)
      {
	window->EventSource<CharEvent>::Type::connect(console);
	window->EventSource<KeyEvent>::Type::connect(console);
      }
  }
  void scigma_gui_gl_window_connect_console_before(PythonID windowID, PythonID consoleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Console,console,consoleID);
    if(window&&console)
      {
	window->EventSource<CharEvent>::Type::connect_before(console);
	window->EventSource<KeyEvent>::Type::connect_before(console);
      }
  }
  void scigma_gui_gl_window_disconnect_console(PythonID windowID, PythonID consoleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Console,console,consoleID);
    if(window&&console)
      {
	window->EventSource<CharEvent>::Type::disconnect(console);
	window->EventSource<KeyEvent>::Type::disconnect(console);
      }
  }
  void scigma_gui_gl_window_connect_atw_panel(PythonID windowID, PythonID atwPanelID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(ATWPanel,atwPanel,atwPanelID);
    if(window&&atwPanel)
      window->viewing_area()->EventSource<ResizeEvent>::Type::connect(atwPanel);
  }
  void scigma_gui_gl_window_connect_atw_panel_before(PythonID windowID, PythonID atwPanelID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(ATWPanel,atwPanel,atwPanelID);
    if(window&&atwPanel)
      window->viewing_area()->EventSource<ResizeEvent>::Type::connect_before(atwPanel);
  }
  void scigma_gui_gl_window_disconnect_atw_panel(PythonID windowID, PythonID atwPanelID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(ATWPanel,atwPanel,atwPanelID);
    if(window&&atwPanel)
      window->viewing_area()->EventSource<ResizeEvent>::Type::disconnect(atwPanel);
  }

  void scigma_gui_gl_window_connect_picker(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
	window->EventSource<MouseButtonEvent>::Type::connect(picker);
  }
  void scigma_gui_gl_window_connect_picker_before(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
	window->EventSource<MouseButtonEvent>::Type::connect_before(picker);
  }
  void scigma_gui_gl_window_disconnect_picker(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
      {
	window->EventSource<MouseButtonEvent>::Type::disconnect(picker);
	window->EventSource<MouseMotionEvent>::Type::disconnect(picker);
      }
  }

  void scigma_gui_gl_window_set_title(PythonID objectID, const char* title)
  {PYCLL(GLWindow,objectID,set_title(title))}

  // ViewingArea functionality
  void scigma_gui_gl_window_set_size(PythonID objectID, GLint width, GLint height)
  {PYCLL(GLWindow,objectID,viewing_area()->set_size(width,height))}
  void scigma_gui_gl_window_set_margin(PythonID objectID, GLint whichMargin, GLfloat pixels)
  {PYCLL(GLWindow,objectID,viewing_area()->set_margin(whichMargin,pixels))}
  void scigma_gui_gl_window_set_margins(PythonID objectID, GLint left, GLint right, GLint bottom, GLint top)
  {PYCLL(GLWindow,objectID,viewing_area()->set_margins(left,right,bottom,top));}
  void scigma_gui_gl_window_blow_up(PythonID objectID, GLfloat left, GLfloat right, GLfloat top, GLfloat bottom)
  {PYCLL(GLWindow,objectID,viewing_area()->blow_up(left,right,top,bottom))}
  void scigma_gui_gl_window_set_z_camera(PythonID objectID, GLfloat z) 
  {PYCLL(GLWindow,objectID,viewing_area()->set_z_camera(z))}
  void scigma_gui_gl_window_set_viewing_angle(PythonID objectID, GLfloat angle) 
  {PYCLL(GLWindow,objectID,viewing_area()->set_viewing_angle(angle))}
  // ViewingVolume functionality
  void scigma_gui_gl_window_set_range(PythonID objectID, GLint coordinate, GLfloat low, GLfloat high)
  {PYCLL(GLWindow,objectID,viewing_volume()->set_range(coordinate,low,high))}
  void scigma_gui_gl_window_shift(PythonID objectID, GLfloat dx, GLfloat dy, GLfloat dz)
  {PYCLL(GLWindow,objectID,viewing_volume()->shift(dx,dy,dz))}
  void scigma_gui_gl_window_shiftII(PythonID objectID, GLint coordinate, GLfloat distance)
  {PYCLL(GLWindow,objectID,viewing_volume()->shift(coordinate,distance))}
  void scigma_gui_gl_window_scale(PythonID objectID, GLint coordinate, GLfloat factor)
  {PYCLL(GLWindow,objectID,viewing_volume()->scale(coordinate,factor))}
  void scigma_gui_gl_window_rotate(PythonID objectID, GLfloat ax, GLfloat ay, GLfloat az, GLfloat angle)
  {PYCLL(GLWindow,objectID,viewing_volume()->rotate(ax,ay,az,angle))}
  void scigma_gui_gl_window_rotateII(PythonID objectID, GLint coordinate,GLfloat angle)
  {PYCLL(GLWindow,objectID,viewing_volume()->rotate(coordinate,angle))}
  void scigma_gui_gl_window_reset(PythonID objectID)
  {PYCLL(GLWindow,objectID,viewing_volume()->reset())}
  void scigma_gui_gl_window_reset_rotation(PythonID objectID)
  {PYCLL(GLWindow,objectID,viewing_volume()->reset_rotation())}
  const GLfloat* scigma_gui_gl_window_min(PythonID objectID)
  {
    PYOBJ(GLWindow,ptr,objectID);
    if(ptr)
      return ptr->viewing_volume()->min();
    else
      return NULL;
  }
  const GLfloat* scigma_gui_gl_window_max(PythonID objectID)
  {
    PYOBJ(GLWindow,ptr,objectID);
    if(ptr)
      return ptr->viewing_volume()->max();
    else
      return NULL;
  }

  /* not yet implemented
     void scigma_gui_gl_window_set_center(PythonID objectID, GLfloat x, GLfloat y, GLfloat z)
     {PYCLL(GLWindow,objectID,viewing_volume()->set_center(x,y,z))}
     void scigma_gui_gl_window_set_screen_direction(PythonID objectID, GLint screenCoordinate, GLfloat x, GLfloat y, GLfloat z)
     {PYCLL(GLWindow,objectID,viewing_volume()->set_screen_direction(screenCoordinate,x,y,z))}
     void scigma_gui_gl_window_set_screen_directionII(PythonID objectID, GLint screenCoordinate, GLint spaceCoordinate)
     {PYCLL(GLWindow,objectID,viewing_volume()->set_screen_direction(screenCoordinate,spaceCoordinate))}
  */
  // GLContext functionality
  void scigma_gui_gl_window_continuous_refresh_needed(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->continuous_refresh_needed())}
  void scigma_gui_gl_window_continuous_refresh_not_needed(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->continuous_refresh_not_needed())}
  void scigma_gui_gl_window_stall(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->stall())}
  void scigma_gui_gl_window_draw_frame(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->draw_frame())}
  void scigma_gui_gl_window_request_redraw(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->request_redraw())}
  void scigma_gui_gl_window_flush(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->flush())}

  void scigma_gui_gl_window_add_curve(PythonID windowID, PythonID CurveID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Curve,Curve,CurveID);
    if(window&&Curve)
	  window->gl_context()->add_drawable(Curve);
  }
  void scigma_gui_gl_window_remove_curve(PythonID windowID, PythonID CurveID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Curve,Curve,CurveID);
    if(window&&Curve)
	  window->gl_context()->remove_drawable(Curve);
  }

  void scigma_gui_gl_window_add_picker(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
      window->gl_context()->add_drawable(picker);
  }
  void scigma_gui_gl_window_remove_picker(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
      window->gl_context()->remove_drawable(picker);
  }
  
  // wrappers for the Navigator class
  PythonID scigma_gui_create_navigator(int view){Navigator* ptr=new Navigator(view);return ptr->get_python_id();}
  void scigma_gui_destroy_navigator(PythonID objectID){PYOBJ(Navigator,ptr,objectID);if(ptr)delete ptr;}
   
  //wrappers for the Console class
  /* as Python code cannot directly receive Console's InputEvent, this helper class
     is used as a proxy.
  */
  class ScigmaGuiConsoleEventMonitor:public EventSink<InputEvent>::Type
  {
  private:
    void (*python_callback_)(const char*);
  public:
    ScigmaGuiConsoleEventMonitor(void (*python_callback)(const char*)):python_callback_(python_callback){}
    virtual ~ScigmaGuiConsoleEventMonitor();
    virtual bool process(InputEvent event,std::string input)
    {python_callback_(input.c_str());return true;}
    };
    ScigmaGuiConsoleEventMonitor::~ScigmaGuiConsoleEventMonitor(){}
  /* this map stores a ScigmaGuiConsoleEventMonitor for each Console when it is created;
     on destruction of the Console object, these are destroyed as well.
  */
    
  std::map<Console*,ScigmaGuiConsoleEventMonitor*> scigmaGuiConsoleEventMonitorMap;
  PythonID scigma_gui_create_console(PythonID windowID, bool largeFontsFlag, void (*python_callback)(const char*))
  {
    PYOBJ(GLWindow,window,windowID);
    if(window)
      {
	Console* ptr1(new Console(window->gl_context(),largeFontsFlag));
	ScigmaGuiConsoleEventMonitor* ptr2(new ScigmaGuiConsoleEventMonitor(python_callback));
	ptr1->EventSource<InputEvent>::Type::connect(ptr2);
	scigmaGuiConsoleEventMonitorMap.insert(std::pair<Console*,ScigmaGuiConsoleEventMonitor*>(ptr1,ptr2));
	return ptr1->get_python_id();
      }
    else
      return -1;
  }
  void scigma_gui_destroy_console(PythonID objectID)
  {
    PYOBJ(Console,ptr1,objectID);
    if(ptr1)
      {
	std::map<Console*,ScigmaGuiConsoleEventMonitor*>::iterator i(scigmaGuiConsoleEventMonitorMap.find(ptr1));
	ScigmaGuiConsoleEventMonitor* ptr2((*i).second);
	scigmaGuiConsoleEventMonitorMap.erase(i);
	ptr1->EventSource<InputEvent>::Type::disconnect(ptr2);
	delete ptr2;
	delete ptr1;
      }
  }
  
  void scigma_gui_console_set_displayed_screen_lines(PythonID objectID, int number)
  {PYCLL(Console,objectID,set_displayed_screen_lines(GLuint(number)))}
  void scigma_gui_console_set_history_fadeout(PythonID objectID, int yesNo)
  {PYCLL(Console,objectID,set_history_fadeout(yesNo?true:false))}
  void scigma_gui_console_write(PythonID objectID, const char* text)
  {PYCLL(Console,objectID,write(text))}
  void scigma_gui_console_write_data(PythonID objectID, const char* text)
  {PYCLL(Console,objectID,write_data(text))}  
  void scigma_gui_console_write_error(PythonID objectID, const char* text)
  {PYCLL(Console,objectID,write_error(text))} 

  // wrappers for the ATWPanel class
  /* as Python code cannot directly receive ATWPanel's Events, this helper class
     is used as a proxy.
  */
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
    virtual bool process(EntryChangeEvent event, const char* id, const void* ptr)
    {python_callback_(id,ptr);return true;}
    virtual bool process(ButtonEvent event, const char* id)
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
	ptr1->EventSource<EntryChangeEvent>::Type::connect(ptr2);
	ptr1->EventSource<ButtonEvent>::Type::connect(ptr2);
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
	ptr1->EventSource<EntryChangeEvent>::Type::disconnect(ptr2);
	ptr1->EventSource<ButtonEvent>::Type::disconnect(ptr2);
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

  // wrappers for the Picker class
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
    ptr1->EventSource<PickEvent>::Type::connect(ptr2);
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
	ptr1->EventSource<PickEvent>::Type::disconnect(ptr2);
	delete ptr2;
	delete ptr1;
      }
  }

  // wrappers for the Graph class
  std::string scigmaGuiGraphError;

  void scigma_gui_graph_set_indexed_colors(const GLfloat* colors, int nColors)
  {
    Graph::set_indexed_colors(colors,size_t(nColors));
  }
  const char* scigma_gui_graph_rebuild_shaders(PythonID glWindowID,
					       const char* expressions, 
					       const char* independentVariables)
  {
    PYOBJ(GLWindow,ptr,glWindowID);
    if(!ptr)
      return NULL;
    
    ExpressionArray exp;
    append_tokens(expressions,exp,'|');

    if(exp.size()<N_COORDINATES-1||exp.size()>N_COORDINATES)
      {
	scigmaGuiGraphError="incorrect number of expressions for graph shader rebuild";
	return scigmaGuiGraphError.c_str();
      }
    else if(exp.size()==N_COORDINATES-1) // empty color expression
      {
	exp.push_back(exp.back());
	exp[C_INDEX]="";
      }

    ExpressionArray var;
    append_tokens(independentVariables,var,'|');
    
    try
      {
	Graph::rebuild_shaders(ptr->gl_context(),exp,var);
      }
    catch(std::string error)
      {
	scigmaGuiGraphError=error;
	return scigmaGuiGraphError.c_str();
      }

      return NULL;
  }

  // wrappers for the Curve class
  class ScigmaGuiCurveClickEventMonitor:public EventSink<CurveClickEvent>::Type
  {
  private:
    void (*python_callback_)(const char* id);
  public:
    ScigmaGuiCurveClickEventMonitor(void (*python_callback)(const char*)):python_callback_(python_callback){}
    virtual ~ScigmaGuiCurveClickEventMonitor();
    virtual bool process(CurveClickEvent event, const char* id)
    {python_callback_(id);return true;}
  };
  ScigmaGuiCurveClickEventMonitor::~ScigmaGuiCurveClickEventMonitor()
  {}
  /* this map stores a ScigmaCurveClickEventMonitor for each Curve when it is created;
     on destruction of the Curve object, these are destroyed as well.
  */
  std::map<Curve*,ScigmaGuiCurveClickEventMonitor*> scigmaGuiCurveClickEventMonitorMap;
  
  PythonID scigma_gui_create_curve(PythonID glWindowID, const char* id, int nExpectedPoints, 
				   PythonID varWaveID, PythonID constWaveID,
				   int marker, int point, GLfloat markerSize, GLfloat pointSize,
				   const GLfloat* color, GLfloat delay, void(*python_callback)(const char*))
  {
    PYOBJ(GLWindow,glWindow,glWindowID);
    PYOBJ(Wave,varWave,varWaveID);
    PYOBJ(Wave,constWave,constWaveID);
    
    if(glWindow&&varWave&&constWave)
      {
	Curve* ptr1=new Curve(glWindow,std::string(id),uint32_t(nExpectedPoints),varWave,constWave,Marker::Type(marker),Marker::Type(point),
			      markerSize,pointSize,color,delay);
	ScigmaGuiCurveClickEventMonitor* ptr2(new ScigmaGuiCurveClickEventMonitor(python_callback));
	ptr1->EventSource<CurveClickEvent>::Type::connect(ptr2);
	scigmaGuiCurveClickEventMonitorMap.insert(std::pair<Curve*,ScigmaGuiCurveClickEventMonitor*>(ptr1,ptr2));
	return ptr1->get_python_id();
      }
    else
      return -1;
  }
  
  void scigma_gui_destroy_curve(PythonID objectID)
    {
      PYOBJ(Curve,ptr1,objectID);
      if(ptr1)
	{
	  std::map<Curve*,ScigmaGuiCurveClickEventMonitor*>::iterator i(scigmaGuiCurveClickEventMonitorMap.find(ptr1));
	  ScigmaGuiCurveClickEventMonitor* ptr2((*i).second);
	  scigmaGuiCurveClickEventMonitorMap.erase(i);
	  ptr1->EventSource<CurveClickEvent>::Type::disconnect(ptr2);
	  delete ptr2;
	  delete ptr1;
	}
    }
  
  void scigma_gui_curve_set_marker_style(PythonID objectID, int marker)
  {PYCLL(Curve,objectID,set_marker_style(Marker::Type(marker)))}
  void scigma_gui_curve_set_marker_size(PythonID objectID, GLfloat size)
  {PYCLL(Curve,objectID,set_marker_size(size))}
  void scigma_gui_curve_set_point_style(PythonID objectID, int point)
  {PYCLL(Curve,objectID,set_point_style(Marker::Type(point)))}
  void scigma_gui_curve_set_point_size(PythonID objectID, GLfloat size)
  {PYCLL(Curve,objectID,set_point_size(size))}
  void scigma_gui_curve_set_color(PythonID objectID, const GLfloat* color)
  {PYCLL(Curve,objectID,set_color(color))}
  void scigma_gui_curve_set_n_points(PythonID objectID, int nPoints)
  {PYCLL(Curve,objectID,set_n_points(uint32_t(nPoints)))}
  void scigma_gui_curve_set_data(PythonID curveID, PythonID varWaveID, PythonID constWaveID)
  {
    PYOBJ(Curve,ptr,curveID);
    PYOBJ(Wave,varWave,varWaveID);
    PYOBJ(Wave,constWave,constWaveID);
    if(!(ptr&&varWave&&constWave))
      return;
    ptr->set_data(varWave,constWave);
  }

  void scigma_gui_curve_set_view(PythonID objectID, int nIndices, const int* indices)
  {
    PYOBJ(Curve,ptr,objectID);
    if(!ptr)
      return;
    IndexArray ind((size_t(nIndices)));
    for(size_t i(0);i<size_t(nIndices);++i)
      ind[i]=indices[i];
    ptr->set_view(ind);
  }

  // wrappers for the Cosy class
  PythonID scigma_gui_create_cosy(PythonID glWindowID, int view, bool largeFontsFlag)
  {
    PYOBJ(GLWindow,glWindow,glWindowID);
    if(!glWindow)
      return-1;
    Cosy* ptr(new Cosy(glWindow,view,largeFontsFlag));
    return ptr->get_python_id();
  }
  
  void scigma_gui_destroy_cosy(PythonID objectID){PYOBJ(Cosy,ptr,objectID);if(ptr)delete ptr;}
  
  void scigma_gui_cosy_set_view(PythonID objectID, int view)
  {PYCLL(Cosy,objectID,set_view(view))}
  void scigma_gui_cosy_set_label(PythonID objectID, int coordinate, const char* label)
  {PYCLL(Cosy,objectID,set_label(coordinate,label))}


} /* end extern "C" block */

#undef PYCLL
#undef PYOBJ

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
