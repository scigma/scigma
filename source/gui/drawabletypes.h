#ifndef __SCIGMA_GUI_DRAWABLETYPES_H__
#define __SCIGMA_GUI_DRAWABLETYPES_H__

#include <Typelist.h>
#include "curve.h"
#include "ruler.h"
#include "picker.h"
#include "spacetext.h"
#include "screentext.h"

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
    typedef LOKI_TYPELIST_7(DrawableFirst,Curve,Ruler,Picker,SpaceText,ScreenText,DrawableLast) DrawableTypes;

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_DRAWABLETYPES_H__ */
