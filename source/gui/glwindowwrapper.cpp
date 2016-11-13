#include "glwindow.hpp"
#include "viewingvolume.hpp"
#include "viewingarea.hpp"
#include "glcontext.hpp"
#include "navigator.hpp"
#include "picker.hpp"
#include "bundle.hpp"
#include "sheet.hpp"
#include "console.hpp"
#include "atwpanel.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::gui;

extern "C"
{
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
	connect<MouseButtonEvent>(window,navigator);
	connect<ScrollEvent>(window,navigator);
      }
  }
  void scigma_gui_gl_window_connect_navigator_before(PythonID windowID, PythonID navigatorID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Navigator,navigator,navigatorID);
    if(window&&navigator)
      {
	connect_before<MouseButtonEvent>(window,navigator);
	connect_before<ScrollEvent>(window,navigator);
      }
  }
  void scigma_gui_gl_window_disconnect_navigator(PythonID windowID, PythonID navigatorID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Navigator,navigator,navigatorID);
    if(window&&navigator)
      {
	disconnect<MouseButtonEvent>(window,navigator);
	disconnect<ScrollEvent>(window,navigator);
      }
  }
  void scigma_gui_gl_window_connect_console(PythonID windowID, PythonID consoleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Console,console,consoleID);
    if(window&&console)
      {
	connect<CharEvent>(window,console);
	connect<KeyEvent>(window,console);
      }
  }
  void scigma_gui_gl_window_connect_console_before(PythonID windowID, PythonID consoleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Console,console,consoleID);
    if(window&&console)
      {
	connect_before<CharEvent>(window,console);
	connect_before<KeyEvent>(window,console);
      }
  }
  void scigma_gui_gl_window_disconnect_console(PythonID windowID, PythonID consoleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Console,console,consoleID);
    if(window&&console)
      {
	disconnect<CharEvent>(window,console);
	disconnect<KeyEvent>(window,console);
      }
  }
  void scigma_gui_gl_window_connect_atw_panel(PythonID windowID, PythonID atwPanelID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(ATWPanel,atwPanel,atwPanelID);
    if(window&&atwPanel)
      connect<ResizeEvent>(window->viewing_area(),atwPanel);
  }
  void scigma_gui_gl_window_connect_atw_panel_before(PythonID windowID, PythonID atwPanelID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(ATWPanel,atwPanel,atwPanelID);
    if(window&&atwPanel)
      connect_before<ResizeEvent>(window->viewing_area(),atwPanel);
  }
  void scigma_gui_gl_window_disconnect_atw_panel(PythonID windowID, PythonID atwPanelID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(ATWPanel,atwPanel,atwPanelID);
    if(window&&atwPanel)
      disconnect<ResizeEvent>(window->viewing_area(),atwPanel);
  }

  void scigma_gui_gl_window_connect_picker(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
      connect<MouseButtonEvent>(window,picker);
  }
  void scigma_gui_gl_window_connect_picker_before(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
      connect_before<MouseButtonEvent>(window,picker);
  }
  void scigma_gui_gl_window_disconnect_picker(PythonID windowID, PythonID pickerID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Picker,picker,pickerID);
    if(window&&picker)
      {
	disconnect<MouseButtonEvent>(window,picker);
	disconnect<MouseMotionEvent>(window,picker);
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
  void scigma_gui_gl_window_stall(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->stall())}
  void scigma_gui_gl_window_draw_frame(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->draw_frame())}
  void scigma_gui_gl_window_request_redraw(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->request_redraw())}
  void scigma_gui_gl_window_flush(PythonID objectID)
  {PYCLL(GLWindow,objectID,gl_context()->flush())}

  void scigma_gui_gl_window_add_bundle(PythonID windowID, PythonID BundleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Bundle,bundle,BundleID);
    if(window&&bundle)
	  window->gl_context()->add_drawable(bundle);
  }
  void scigma_gui_gl_window_remove_bundle(PythonID windowID, PythonID BundleID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Bundle,bundle,BundleID);
    if(window&&bundle)
	  window->gl_context()->remove_drawable(bundle);
  }

  void scigma_gui_gl_window_add_sheet(PythonID windowID, PythonID SheetID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Sheet,sheet,SheetID);
    if(window&&sheet)
	  window->gl_context()->add_drawable(sheet);
  }
  void scigma_gui_gl_window_remove_sheet(PythonID windowID, PythonID SheetID)
  {
    PYOBJ(GLWindow,window,windowID);
    PYOBJ(Sheet,sheet,SheetID);
    if(window&&sheet)
	  window->gl_context()->remove_drawable(sheet);
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
  void scigma_gui_gl_window_set_theme(PythonID windowID, int theme)
  {
    PYOBJ(GLWindow,window,windowID);
    if(window)
      window->set_theme(GLWindow::Theme(theme));
  }
  
} /* end extern "C" block */

#pragma clang diagnostic pop
