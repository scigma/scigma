#ifndef SCIGMA_GUI_MOUSE_HPP
#define SCIGMA_GUI_MOUSE_HPP

#include "../common/pythonobject.hpp"
#include "../common/events.hpp"
#include "glwindowevents.hpp"

using scigma::common::PythonObject;
using scigma::common::EventSink;
using scigma::common::EventSource;

namespace scigma
{
  namespace gui
  {
    struct RightClickEvent{  
      typedef LOKI_TYPELIST_0 Arguments;};

    class Mouse:public PythonObject<Mouse>,
		public EventSink<MouseButtonEvent>::Type,
		public EventSource<RightClickEvent>::Type
    {
    public:
      Mouse();
      virtual ~Mouse();
      
      bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods) override;
      
    private:
      Mouse(const Mouse&);
      Mouse& operator=(const Mouse&);
      
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_MOUSE_HPP */
