#ifndef SCIGMA_GUI_VIEWINGVOLUMEEVENTS_HPP
#define SCIGMA_GUI_VIEWINGVOLUMEEVENTS_HPP

namespace scigma
{
  namespace gui
  {
    
    class ViewingVolume;
    
    //!This event is emitted when the scale changes.
    /*!@ingroup gui */
    struct ScaleEvent
    { //!Argument list for ScaleEvent 
      typedef LOKI_TYPELIST_4(const ViewingVolume*,GLfloat,GLfloat,GLfloat) Arguments;};
    //!This event is emitted when the coordinate system is shifted. 
    /*!@ingroup gui */
    struct ShiftEvent
    { //!Argument list for ShiftEvent 
      typedef LOKI_TYPELIST_4(const ViewingVolume*,GLfloat,GLfloat,GLfloat) Arguments;};
    //!This event is emitted when the coordinate system is rotated. 
    /*!@ingroup gui */
    struct RotateEvent
    { //!Argument list for RotateEvent 
      typedef LOKI_TYPELIST_5(const ViewingVolume*,GLfloat,GLfloat,GLfloat,GLfloat) Arguments;};
    //!This event is emitted when the coordinate system is both shifted and the scale changes. 
    /*!@ingroup gui */
    struct RangeEvent
    { //!Argument list for RangeEvent 
      typedef LOKI_TYPELIST_1(const ViewingVolume*) Arguments;};
    //!This event is emitted when the coordinate system is reset. 
    /*!@ingroup gui */
    struct ResetEvent{
      //!Argument list for ResetEvent 
      typedef LOKI_TYPELIST_1(const ViewingVolume*) Arguments;};

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_VIEWINGVOLUMEEVENTS_HPP */
