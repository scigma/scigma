#ifndef SCIGMA_GUI_DRAWABLETYPES_HPP
#define SCIGMA_GUI_DRAWABLETYPES_HPP

#include <Typelist.h>
#include "bundle.hpp"
#include "sheet.hpp"
#include "ruler.hpp"
#include "picker.hpp"
#include "spacetext.hpp"
#include "screentext.hpp"

namespace scigma
{
  namespace gui
  {

    //!List of drawable types
    /*!@ingroup gui
      @typedef
      Any class that is used as a static drawable type with GLContext must be registered
      by adding the type to this LOKI_TYPE_LIST_N. Do not forget to adjust the number N
      when extending this. The order in which the classes appear here is the order in which
      they are drawn.
    */
    typedef LOKI_TYPELIST_6(Bundle,Sheet,Ruler,Picker,SpaceText,ScreenText) DrawableTypes;

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_DRAWABLETYPES_HPP */
