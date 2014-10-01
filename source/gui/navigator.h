#ifndef __SCIGMA_GUI_NAVIGATOR_H__
#define __SCIGMA_GUI_NAVIGATOR_H__

#include "../pythonobject.h"
#include "glwindow.h"

namespace scigma
{
  namespace gui
  {
    class Navigator:public PythonObject<Navigator>,
      public EventSink<MouseButtonEvent>::Type,
      public EventSink<MouseMotionEvent>::Type,
      public EventSink<ScrollEvent>::Type
      {
      public:
	Navigator(int view);
	virtual ~Navigator();

	virtual bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods);
	virtual bool process(MouseMotionEvent event, GLWindow* w, GLfloat x, GLfloat y, GLfloat xOld, GLfloat yOld);
	virtual bool process(ScrollEvent event, GLWindow* w, GLfloat xScroll, GLfloat yScroll);

      private:
	Navigator(const Navigator&);
	Navigator& operator=(const Navigator&);

	int view_;

	bool rightMouseButtonPressed_;
	bool shiftKeyPressed_;
	char padding_[2];
      };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_NAVIGATOR_H__ */
