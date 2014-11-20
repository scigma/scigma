#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <AntTweakBar.h>
#include "../events.h"
#include "application.h"
#include "glwindow.h"
#include "glutil.h"
#include "drawable.h"
#include "font.h"
#include "marker.h"

extern "C"
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"  
  void window_close_callback(GLFWwindow* w)
  {
    scigma::gui::GLWindow* window = static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w));
    delete window;
  }
  
  /*  void window_iconify_callback(GLFWwindow* w, int iconify)
  {
  }*/
  
  void window_size_callback(GLFWwindow* w, int width, int height)
  {
    scigma::gui::GLWindow* glWindow(static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w))); 
    TwSetCurrentWindow(size_t(glWindow->gl_context()));
    TwWindowSize(width,height);
    glWindow->viewing_area()->set_size(width,height,false);
  }
  
  /*void window_position_callback(GLFWwindow* w, int x, int y)
  {}
  
  void window_refresh_callback(GLFWwindow* w)
  {}
  
  void window_focus_callback(GLFWwindow* w, int focused)
  {}
  
  void framebuffer_size_callback(GLFWwindow* w, int width, int height)
  {}*/  
  void mouse_button_callback(GLFWwindow* w, int button , int action, int mods)
  {
    scigma::gui::GLWindow* glWindow(static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w))); 
    TwSetCurrentWindow(size_t(glWindow->gl_context()));
    if(TwEventMouseButtonGLFW(button,action))
      {
	glWindow->gl_context()->request_redraw();
	if(GLFW_RELEASE==action)
	  static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w))->forward_mouse_button_event(button,action,mods);
      }
    else
      static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w))->forward_mouse_button_event(button,action,mods);
  }
  
  void cursor_position_callback(GLFWwindow* w, double x, double y)
  {
    scigma::gui::GLWindow* glWindow = static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w));
    TwSetCurrentWindow(size_t(glWindow->gl_context()));
    if(TwEventMousePosGLFW(int(x),int(y)))
      glWindow->gl_context()->request_redraw();
    else
      glWindow->forward_cursor_position_event(GLfloat(x), glWindow->viewing_area()->center()[1]*2-GLfloat(y));
  }
  
  void scroll_callback(GLFWwindow* w, double xScroll, double yScroll)
  {
    static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w))->forward_scroll_event(GLfloat(xScroll), GLfloat(yScroll));
  }
  
  void char_callback(GLFWwindow* w, unsigned int unicode)
  {
    scigma::gui::GLWindow* glWindow(static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w)));

 
    TwSetCurrentWindow(size_t(glWindow->gl_context()));
    if(TwEventCharGLFW(int(unicode),GLFW_PRESS))
      glWindow->gl_context()->request_redraw();
    else
      glWindow->forward_char_event(unicode);
  }        
  
  extern int ESCAPE_COUNT;
  int ESCAPE_COUNT(0);

  void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods)
  {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      {
	++ESCAPE_COUNT;
	std::cout<<ESCAPE_COUNT<<std::endl;
      }
    
    scigma::gui::GLWindow* glWindow(static_cast<scigma::gui::GLWindow*>(glfwGetWindowUserPointer(w))); 
    TwSetCurrentWindow(size_t(glWindow->gl_context()));
    if(TwEventKeyGLFW(key,action))
      glWindow->gl_context()->request_redraw();
    else
      glWindow->forward_key_event(key,scancode,action,mods);
  }        
#pragma clang diagnostic pop  
} /* end extern "C" block */

extern "C" bool LARGE_FONTS_FLAG;
bool LARGE_FONTS_FLAG(false);

namespace scigma
{
  namespace gui
  {

    Application* Application::theInstance_(NULL);
    
    Application::Application():masterWindow_(NULL),log_(NULL),idleIndex_(0),loopIsRunning_(false)
    {
      char path[0x1000];
      getcwd(path,0x1000);
      log_=new Log(std::string(path)+"/.scigmaguierrors.log");
    }
    
    Application::~Application()
    {
      delete log_;
    }

    Application* Application::get_instance()
    {
      if(theInstance_)
	return theInstance_;
      
      initialize();
      atexit(terminate);
      return theInstance_;
    }
    
    void Application::initialize()
    {
      theInstance_ = new Application();
      
      glfwSetErrorCallback(push_glfw_error);
      if (!glfwInit())
	return;
#ifdef SCIGMA_USE_OPENGL_3_2
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,2);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#else
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
#endif
      glfwWindowHint(GLFW_VISIBLE,GL_FALSE);
      //glfwWindowHint(GLFW_DECORATED,GL_FALSE);
      theInstance_->masterWindow_=glfwCreateWindow(10,10, "", NULL,NULL);
      glfwWindowHint(GLFW_VISIBLE,GL_TRUE);
      //glfwWindowHint(GLFW_SAMPLES, 4);
      glfwMakeContextCurrent(theInstance_->masterWindow_);
      GLERR;

      glewExperimental = GL_TRUE; 
      GLenum err = glewInit();
      if(GLEW_OK!=err)
	theInstance_->push_error("could not initialize GLEW library",__FILE__,__LINE__);
      glGetError(); // glewInit() apparently sometimes produces a benign invalid enum error 
      // catch that here
      
      Font::initialize();
      Marker::initialize();
      initialize_drawables<DrawableTypes>();
      glFinish();

      std::stringstream ss;
      ss<<"GLOBAL fontscaling=";
      ss<<(LARGE_FONTS_FLAG?2.0:1.0);

      TwDefine(ss.str().c_str());

#ifdef SCIGMA_USE_OPENGL_3_2
      TwInit(TW_OPENGL_CORE, NULL);
#else
      TwInit(TW_OPENGL, NULL);
#endif
     
      GLERR;
    }
    
    void Application::terminate()
    {
      if(!theInstance_)
	return;

      TwTerminate();
      terminate_drawables<DrawableTypes>();
      //      Marker::terminate();
      //Font::terminate();
      glfwDestroyWindow(theInstance_->masterWindow_);
      glfwTerminate();
      
      delete theInstance_;
    }
    
    void Application::glfw_window_set_callbacks(GLFWwindow* w)
    {
      glfwSetWindowCloseCallback(w, window_close_callback);  
      //glfwSetWindowIconifyCallback(w, window_iconify_callback);
      glfwSetWindowSizeCallback(w, window_size_callback);
      /*glfwSetWindowPosCallback(w, window_position_callback);
      glfwSetWindowRefreshCallback(w, window_refresh_callback);
      glfwSetWindowFocusCallback(w, window_focus_callback);  
      glfwSetFramebufferSizeCallback(w, framebuffer_size_callback);*/
      glfwSetMouseButtonCallback(w, mouse_button_callback);
      glfwSetCursorPosCallback(w, cursor_position_callback);
      glfwSetScrollCallback(w, scroll_callback);
      glfwSetCharCallback(w, char_callback);
      glfwSetKeyCallback(w, key_callback);
    }
    
    GLFWwindow* Application::get_master_window()
    {
      return masterWindow_;
    }
    
    void Application::push_glfw_error(int code, const char* error)
    {
      std::ostringstream combine;
      combine<<"GLFW error code "<<code<<": "<<error; 
      theInstance_->push_error(combine.str());
    }
    
    const char* Application::pop_error()
    {
      return log_->pop();
    }
    
    void Application::push_error(std::string error, const char* file, int line)
    {
      log_->push(error,file,line);
    }

    void Application::loop(double seconds)
    {
      if(loopIsRunning_) //nested loop call
	{
	  double lt(glfwGetTime());
	  while(true)
	    {
	      loopMutex_.lock();
	      EventSource<LoopEvent>::Type::emit();
	      loopMutex_.unlock();

	      idleMutex_.lock();
	      size_t nIdleSinks;
	      while((nIdleSinks=EventSource<IdleEvent>::Type::sinks.size())!=0)
		{
		  IdleEvent e;
		  idleIndex_=idleIndex_%nIdleSinks;
		  EventSource<IdleEvent>::Type::sinks[idleIndex_]->process(e,glfwGetTime());
		  ++idleIndex_;
		}
	      idleMutex_.unlock();

	      if(glfwGetTime()>=seconds+lt)
		return;
	    }	      
	}

      loopIsRunning_=true;
      if(seconds<REFRESH_INTERVAL)
	seconds=REFRESH_INTERVAL;
      while(true)
	{
	  double lt(glfwGetTime());
	  if(seconds<REFRESH_INTERVAL)
	    {
	      loopIsRunning_=false;
	      return;
	    }
	  double remainingSecondsAtStart(seconds);	 
	  // refresh
	  loopMutex_.lock();
	  EventSource<LoopEvent>::Type::emit();
	  loopMutex_.unlock();
	  glfwPollEvents();
	  if(!loopIsRunning_)
	    return;
	  // as long as there is time, emit IdleEvents, if there are connected sinks
	  double t(glfwGetTime());
	  seconds-=(t-lt);
	  idleMutex_.lock();
	  size_t nIdleSinks;
	  while((nIdleSinks=EventSource<IdleEvent>::Type::sinks.size())!=0)
	    {
	      IdleEvent e;
	      idleIndex_=idleIndex_%nIdleSinks;
	      EventSource<IdleEvent>::Type::sinks[idleIndex_]->process(e,glfwGetTime());
	      ++idleIndex_;
	      glfwPollEvents();
	      if(!loopIsRunning_)
		  return;
	      lt=t;
	      t=glfwGetTime();
	      seconds-=(t-lt);
	      if(seconds<remainingSecondsAtStart-REFRESH_INTERVAL)
		break;
	    }
	  idleMutex_.unlock();
	  /* if there is still more time, but no EventSinks for IdleEvent any more, insert a waiting period
	     to avoid busy waits
	  */
	  while(seconds>remainingSecondsAtStart-REFRESH_INTERVAL)
	    {
	      double d(seconds-remainingSecondsAtStart+REFRESH_INTERVAL);
	      glfwWaitEventsWithTimeOut(d);
	      if(!loopIsRunning_)
		  return;
	      lt=t;
	      t=glfwGetTime();
	      seconds-=(t-lt);
	    }
	}
    }
    
    void Application::break_loop()
    {
      loopIsRunning_=false;
    }

    void Application::connect_to_loop_threadsafe(EventSink<LoopEvent>::Type* sink)
    {
      tthread::lock_guard<tthread::mutex> guard(loopMutex_);
      EventSource<LoopEvent>::Type::connect(sink);
    }
    void Application::connect_to_idle_threadsafe(EventSink<IdleEvent>::Type* sink)
    {
      tthread::lock_guard<tthread::mutex> guard(idleMutex_);
      EventSource<IdleEvent>::Type::connect(sink);
    }
    void Application::disconnect_from_loop_threadsafe(EventSink<LoopEvent>::Type* sink)
    {
      tthread::lock_guard<tthread::mutex> guard(loopMutex_);
      EventSource<LoopEvent>::Type::disconnect(sink);
    }
    void Application::disconnect_from_idle_threadsafe(EventSink<IdleEvent>::Type* sink)
    {
      tthread::lock_guard<tthread::mutex> guard(idleMutex_);
      EventSource<IdleEvent>::Type::disconnect(sink);
    }

  } /* end namespace gui */
} /* end namespace scigma */
