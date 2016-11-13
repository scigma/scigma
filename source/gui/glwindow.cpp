#include <AntTweakBar.h>
#include "glwindow.hpp"
#include "application.hpp"
#include "cosy.hpp"

namespace scigma
{
  namespace gui
  {
    
    GLWindow::GLWindow():PythonObject<GLWindow>(this),
			 glfwWindowPointer_(glfwCreateWindow(600,600,"scigma", NULL,Application::get_instance()->get_master_window())),
			 glContext_(glfwWindowPointer_),
			 viewingArea_(glfwWindowPointer_,&glContext_),
			 viewingVolume_(&glContext_),
			 cursorX_(cursorPosition_),
			 cursorY_(cursorPosition_+1),
			 cursorLastX_(cursorPosition_+2),
			 cursorLastY_(cursorPosition_+3)
    {
      Application::get_instance()->glfw_window_set_callbacks(glfwWindowPointer_);
      glfwSetWindowUserPointer(glfwWindowPointer_,static_cast<void*>(this));
      TwSetCurrentWindow(size_t(&glContext_));
      TwDefine(" GLOBAL iconpos=topright ");
      TwDefine(" GLOBAL iconalign=horizontal ");
      TwDefine(" GLOBAL contained=true ");
      TwDefine(" GLOBAL buttonalign=right");
      TwDefine(" GLOBAL fontsize=3");
      TwDefine(" GLOBAL fontresizable=false");
      TwDefine(" TW_HELP visible=false ");
      *cursorX_=-1e10f;
      *cursorY_=-1e10f;
      *cursorLastX_=-1e10f;
      *cursorLastY_=-1e10f;

      set_theme(DARK);
    }
    
    GLWindow::~GLWindow()
    {
      glContext_.destroy();
      glfwDestroyWindow(glfwWindowPointer_);
    }
    
    void GLWindow::set_title(const char* title)
    {
      glfwSetWindowTitle(glfwWindowPointer_,title);
    }

    void GLWindow::set_theme(Theme theme)
    {
      GLfloat fg[4]={1,1,1,1};
      GLfloat bg[4]={0,0,0,1};

      if(theme==DARK)
	{
	  set_foreground_color(fg);
	  set_background_color(bg);
	}
      else
	{
	  set_foreground_color(bg);
	  set_background_color(fg);
	}
    }
    
    void GLWindow::set_foreground_color(const GLfloat* color)
    {
      glContext_.update_global_uniform_4(GLContext::FOREGROUND_COLOR,color);
      glContext_.request_redraw();
    }
    
    void GLWindow::set_background_color(const GLfloat* color)
    {
      glContext_.update_global_uniform_4(GLContext::BACKGROUND_COLOR,color);
      glContext_.request_redraw();
    }
    
    void GLWindow::forward_mouse_button_event(int button , int action, int mods)
    {
      glContext_.stall();
      EventSource<MouseButtonEvent>::Type::emit(this,button,action,mods);
      glContext_.flush();
    }
    
    void GLWindow::forward_cursor_position_event(GLfloat  x, GLfloat y)
    {
      *cursorLastX_=*cursorX_;
      *cursorLastY_=*cursorY_;
      *cursorX_=x;
      *cursorY_=y;
      if(*cursorLastX_<-1.0e9f)
	return;
      glContext_.stall();
      glContext_.check_for_hover(x,y);
      EventSource<MouseMotionEvent>::Type::emit(this,x,y,*cursorLastX_,*cursorLastY_);
      glContext_.flush();
    }
    
    void GLWindow::forward_scroll_event(GLfloat xScroll, GLfloat yScroll)
    {
      glContext_.stall();
      EventSource<ScrollEvent>::Type::emit(this,xScroll,yScroll);
      glContext_.flush();
    }
    
    void GLWindow::forward_char_event(unsigned int unicode)
    {
      EventSource<CharEvent>::Type::emit(this,unicode);
    }

    const GLfloat* GLWindow::cursor_position() const
    {
      return cursorPosition_;
    }

    void GLWindow::forward_key_event(int key, int scancode, int action, int mods)
    {
      EventSource<KeyEvent>::Type::emit(this,key,scancode,action,mods);
    }
 
    GLFWwindow* GLWindow::glfw_window()
    {
      return glfwWindowPointer_;
    }  
    
    ViewingArea* GLWindow::viewing_area()
    {
      return &viewingArea_;
    }
    
    ViewingVolume* GLWindow::viewing_volume()
    {
      return &viewingVolume_;
    }
    
    GLContext* GLWindow::gl_context()
    {
      return &glContext_;
    }
    
  } /* end namespace gui */
} /* end namespace scigma */
