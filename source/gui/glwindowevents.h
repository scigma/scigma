#ifndef __SCIGMA_GUI_GLWINDOWEVENTS_H__
#define __SCIGMA_GUI_GLWINDOWEVENTS_H__

#include "definitions.h"

namespace scigma
{
  namespace gui
  {
    class GLWindow;
    
    //!This event is emitted when a mouse button is pressed or released. 
    /*!@ingroup gui */
    struct MouseButtonEvent
    { //!Argument list for MouseButtonEvent 
      typedef LOKI_TYPELIST_4(GLWindow*,int,int,int) Arguments;};
    //!This event is emitted when the mouse cursor moves. 
    /*!@ingroup gui */
    struct MouseMotionEvent
    { //!Argument list for MouseMotionEvent 
      typedef LOKI_TYPELIST_5(GLWindow*,GLfloat, GLfloat, GLfloat,GLfloat) Arguments;};
    //!This event is emitted when the scroll wheel is used. 
    /*!@ingroup gui */
    struct ScrollEvent
    { //!Argument list for ScrollEvent 
      typedef LOKI_TYPELIST_3(GLWindow*,GLfloat,GLfloat) Arguments;};
    //!This event is emitted when a key is pressed or released. 
    /*!@ingroup gui */
    struct KeyEvent
    { //!Argument list for KeyEvent 
      typedef LOKI_TYPELIST_5(GLWindow*,int,int,int,int) Arguments;};
    //!This event is emitted when a character is entered. 
    /*!@ingroup gui */
    struct CharEvent
    { //!Argument list for KeyEvent 
      typedef LOKI_TYPELIST_2(GLWindow*,unsigned int) Arguments;};

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_GLWINDOWEVENTS_H__ */
