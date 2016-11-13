#ifndef SCIGMA_GUI_INPUTEVENT_HPP
#define SCIGMA_GUI_INPUTEVENT_HPP

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

#endif /*SCIGMA_GUI_INPUTEVENT_HPP */
