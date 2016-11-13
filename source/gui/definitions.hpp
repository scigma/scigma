#ifndef SCIGMA_GUI_DEFINITIONS_HPP
#define SCIGMA_GUI_DEFINITIONS_HPP

#include <glew.h>
#include <glfw3.h>

/*!@file definitions.h */

namespace scigma
{
  namespace gui
  {
   
    const unsigned int N_ROWS(4);
    const unsigned int N_COLUMNS(4);
    const unsigned int N_4X4_ELEMENTS(N_ROWS*N_COLUMNS);
    const unsigned int N_2X2_ELEMENTS(4);

    //!Coordinates.
    /*!@ingroup gui
      These values are used to refer to the displayed coordinates (instead of magic numbers). 
      They can be combined with the '|' operator to describe sets of coordinate directions. 
    */
    enum ViewingCoordinate
    {
      //!0x1
      X_COORDINATE=0x1,
      //!0x2
      Y_COORDINATE=0x2,
      //!0x4
      Z_COORDINATE=0x4,
      //!0x8
      C_COORDINATE=0x8,
      //!0x10
      T_COORDINATE=0x10,
      //!0x1F
      ALL_COORDINATES=0x1F
    };
    
    //!Order of coordinates in arrays.
    /*!@ingroup gui
      In arrays containing information about all coordinates, these values are
      the indices for each coordinate. 
    */
    enum CoordinateIndex
    {
      //!0
      X_INDEX=0,
      //!1
      Y_INDEX=1,
      //!2
      Z_INDEX=2,
      //!3
      C_INDEX=3,
      //!4
      T_INDEX=4
    };
    const unsigned int N_COORDINATES(5);
    
    //!Screen coordinates.
    /*!@ingroup gui
      These values are used to refer to the screen coordinates (instead of magic numbers). 
      They can be combined with the '|' operator to describe sets of coordinate directions. 
    */
    enum ScreenCoordinate
    {
      //!0x100      
      X_SCREEN_COORDINATE=0x100,
      //!0x200      
      Y_SCREEN_COORDINATE=0x200,
      //!0x400      
      Z_SCREEN_COORDINATE=0x400
    };
    
    const unsigned int N_SPATIAL_DIMENSIONS(3);
    const unsigned int N_SCREEN_DIMENSIONS(2);


  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_DEFINITIONS_HPP */
