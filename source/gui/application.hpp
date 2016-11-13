#ifndef SCIGMA_GUI_APPLICATION_HPP
#define SCIGMA_GUI_APPLICATION_HPP

#include <string>
#include <Typelist.h>
#include <tinythread.h>
#include "../common/events.hpp"
#include "../common/log.hpp"
#include "definitions.hpp"
#include "inputevent.hpp"

struct GLFWwindow;

using scigma::common::Log;
using scigma::common::EventSource;
using scigma::common::EventSink;

//!Top level namespace for scigma.
namespace scigma
{
//!Namespace for the gui module.
  namespace gui
  {

    struct LoopEvent
    {
      typedef LOKI_TYPELIST_0 Arguments;
    };

    struct IdleEvent
    {
      typedef LOKI_TYPELIST_1(double) Arguments;
    };
    
    class Application:public EventSource<LoopEvent>::Type,public EventSource<IdleEvent>::Type
    {
      friend class GLWindow;
      
    public:
      
      static Application* get_instance();
      
      //!retrieve error messages
      /*!@ingroup gui
	This function retrieves the first unprocessed error from the error queue. To retrieve all errors, pop_error() should be called in a loop until returning an empty string.  
	@returns a string containing the next message in the error queue,
	or "" if there is none.
      */
      std::string pop_error();
      
      //!generate error messages
      /*!@ingroup gui
	This function pushes a string to the end of the error queue, from where it can be 
	retrieved with the pop_error() function (after all previous errors have been retrieved).
	@param error contains the error message.
	@param file is the name of the source file of the function where the 
	error occurred (optional).
	@param line is the line number where the error occurred (optional). 
      */
      void push_error(std::string error, const char* file=NULL, int line=0);

      //!run the scigma event loop
      /*!@ingroup gui
	This starts the main event loop of scigmagl. 
	@param seconds specifies how long to run. If seconds is zero, the loop will
	return immediately after processing pending events. If seconds is negative, 
	the loop will run indefinitely, or until break_loop() is called.
      */
      void loop(double seconds);
      
      //!break the scigmagl event loop
      /*!@ingroup gui
	Calling this function will cause the event loop to return immediately. If the
	event loop is currently not running, this function does nothing.
      */
      void break_loop();
      void idle(double seconds);
      void sleep();
      void wake();

      bool is_sleeping() const;

    private:
      Application();
      ~Application();
      Application(const Application&);
      Application& operator=(const Application&);

      static void initialize();
      static void terminate();
      
      GLFWwindow* get_master_window();
      void glfw_window_set_callbacks(GLFWwindow* ptr);
      
      static void push_glfw_error(int code, const char* error);
      static Application* theInstance_;
          
      GLFWwindow* masterWindow_;
      Log* log_;

      tthread::mutex loopMutex_;
      tthread::mutex idleMutex_;

      static constexpr double REFRESH_INTERVAL=1/60.;
      size_t idleIndex_;
      bool loopIsRunning_;
      bool sleeping_;

      char padding_[6];
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_APPLICATION_HPP */
