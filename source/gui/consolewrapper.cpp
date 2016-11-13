#include <map>
#include "console.hpp"
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
	connect<InputEvent>(ptr1,ptr2);
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
	disconnect<InputEvent>(ptr1,ptr2);
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
  void scigma_gui_console_write_warning(PythonID objectID, const char* text)
  {PYCLL(Console,objectID,write_warning(text))}  
  void scigma_gui_console_write_comment(PythonID objectID, const char* text)
  {PYCLL(Console,objectID,write_comment(text))}  
  void scigma_gui_console_write_error(PythonID objectID, const char* text)
  {PYCLL(Console,objectID,write_error(text))} 

  void scigma_gui_console_set_theme(PythonID objectID, int theme)
  {PYCLL(Console,objectID,set_theme(Console::Theme(theme)))} 

} /* end extern "C" block */

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
