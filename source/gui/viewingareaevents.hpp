#ifndef __SCIGMA_GUI_VIEWINGAREAEVENTS_H__
#define __SCIGMA_GUI_VIEWINGAREAEVENTS_H__

namespace scigma
{
  namespace gui
  {
    //!This event is emitted when the ViewingArea is resized. 
    /*!@ingroup gui */
    struct ResizeEvent
    { //!Argument list for ResizeEvent 
      typedef LOKI_TYPELIST_2(int,int) Arguments;};

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_VIEWINGAREAEVENTS_H__ */

