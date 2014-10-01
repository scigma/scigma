#ifndef __SCIGMA_GUI_INPUTEVENT_H__
#define __SCIGMA_GUI_INPUTEVENT_H__

#include <Typelist.h>

namespace scigma
{
  namespace gui
  {

    //!This event is emitted when a line has been entered on a Console.
    /*!@ingroup gui */
    struct InputEvent
    { //!Argument list for ResizeEvent 
      typedef LOKI_TYPELIST_1(std::string) Arguments;};

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_INPUTEVENT_H__ */
