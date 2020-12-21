#include "mouse.hpp"

using scigma::common::connect;
using scigma::common::disconnect;

namespace scigma
{
  namespace gui
  {

    Mouse::Mouse():PythonObject<Mouse>(this)
    {}
    
    Mouse::~Mouse()
    {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    bool Mouse::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(GLFW_MOUSE_BUTTON_RIGHT==button && GLFW_PRESS==action)
	{
	  EventSource<RightClickEvent>::Type::emit();

	  return true;
	}
      return false;
    }
    
#pragma GCC diagnostic pop

  } /* end namespace gui */
} /* end namespace scigma */
