#ifndef __SCIGMA_GUI_GRAPHTYPES_H__
#define __SCIGMA_GUI_GRAPHTYPES_H__

#include <Typelist.h>
#include "curve.h"

namespace scigma
{
  namespace gui
  {

    typedef LOKI_TYPELIST_1(Curve) GraphTypes;

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_DRAWABLETYPES_H__ */
