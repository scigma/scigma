#ifndef __SCIGMA_GUI_GLWINDOW_H__
#define __SCIGMA_GUI_GLWINDOW_H__

#include <Typelist.h>
#include "definitions.h"
#include "viewingarea.h"
#include "viewingvolume.h"
#include "glcontext.h"
#include "../events.h"
#include "../pythonobject.h"
#include "glwindowevents.h"

namespace scigma
{
  namespace gui
  {
    
    //!A class bundling all functionality of a window.
    /*!@ingroup gui
      This class a container for a GLContext, a ViewingArea and a ViewingVolume instance, which are responsible
      drawing, and changing the size and perspective of the window. Moreover, it is an EventSource for
      MouseButtonEvent, CursorPositionEvent, ScrollEvent and KeyEvent.
    */
    class GLWindow: public PythonObject<GLWindow>,
      public EventSource<MouseButtonEvent>::Type,
      public EventSource<MouseMotionEvent>::Type,
      public EventSource<ScrollEvent>::Type,
      public EventSource<CharEvent>::Type,
      public EventSource<KeyEvent>::Type
      {
	
      public:
	GLWindow();
	~GLWindow();
	GLFWwindow* glfw_window();
	ViewingArea* viewing_area();
	ViewingVolume* viewing_volume();
	GLContext* gl_context();
	
	void forward_mouse_button_event(int button , int action, int mods);
	void forward_cursor_position_event(GLfloat  x, GLfloat y);
	void forward_scroll_event(GLfloat xScroll, GLfloat yScroll);
	void forward_char_event(unsigned int unicode);
	void forward_key_event(int key, int scancode, int action, int mods);

	const GLfloat* cursor_position() const; 
	void set_title(const char* title);
	void set_foreground_color(const GLfloat* color);
	void set_background_color(const GLfloat* color);
	
      private:
	GLWindow(const GLWindow&);
	GLWindow& operator=(const GLWindow&);
	
	GLFWwindow* glfwWindowPointer_;
	GLContext glContext_;
	ViewingArea viewingArea_;
	ViewingVolume viewingVolume_;
      
	GLfloat cursorPosition_[4];
	GLfloat *cursorX_,*cursorY_,*cursorLastX_,*cursorLastY_;
      };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_GLWINDOW_H__ */
