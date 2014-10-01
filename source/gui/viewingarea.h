#ifndef __SCIGMA_GUI_VIEWINGAREA_H__
#define __SCIGMA_GUI_VIEWINGAREA_H__

#include <Typelist.h>
#include "../events.h"
#include "../pythonobject.h"
#include "definitions.h"
#include "viewingareaevents.h"

namespace scigma
{
  namespace gui
  {
    
    class GLContext;
    
    //!A class that handles how a ViewingVolume is displayed.
    /*!@ingroup gui
      ViewingArea manages how the ViewingVolume is projected onto the screen area of the 
      containing window. This includes the setting of margins, the switching between 
      different types of projection (like perspective and orthographic) and the choice of
      the visible region. In OpenGL vernacular, ViewingArea is responsible
      for the projection matrix and the viewport.
    */
    class ViewingArea: public PythonObject<ViewingArea>,
      public EventSource<ResizeEvent>::Type
      {

	friend class GLWindow;
	
      public:
	
	//!Margins
	/*!
	  These values are used to refer to the margins of the ViewingArea (instead of magic numbers). 
	  They can be combined with the '|' operator to describe sets of margins. 
	 */
	enum Margin
	{
	  //!0x1
	  LEFT_MARGIN=0x1, 
	  //!0x2
	  RIGHT_MARGIN=0x2,
	  //!0x4
	  TOP_MARGIN=0x4, 
	  //!0x8
	  BOTTOM_MARGIN=0x8, 
	  //!0xF
	  ALL_MARGINS=0xF 
	};
	
	//!set the window size
	/*!
	  @param width is the new width in pixels
	  @param height is thenew height in pixels.
	  @param calledByUser should always be true (false is used if the ViewingArea is resized with the mouse). 
	*/
	void set_size(GLint width, GLint height, bool calledByUser=true);

	//!set the margins on the four window borders
	/*!
	  The margins specify a smaller rectangle inside the window. The boundaries of the
	  viewing volume are congruent with this rectangle if screen coordinates and viewing
	  coordinates are aligned.    
	  \param whichMargin can be ViewingArea::LEFT_MARGIN, ViewingArea::RIGHT_MARGIN, ViewingArea::TOP_MARGIN or ViewingArea::BOTTOM_MARGIN, or a combination of the four (via bitwise or, ViewingArea::ALL_MARGINS combines all of them). 
	  \param pixels is the margin size in pixels.
	*/
	void set_margin(GLint whichMargin, GLfloat pixels);
	void set_margins(GLint left, GLint right, GLint bottom, GLint top);
	
	//!select region of the window for enlarging/shrinking the field of view. 
	/*!
	  This function blows up the specified region of the window to fill the whole window.
	  For example, to enhance the upper left area of the displayed image by a factor of
	  two, one would call set_visible_region(0, 0.5, 0.5, 1). To view regions that are
	  currently outside the focus of the window, use values smaller than zero or greater
	  than one.
	  \param left is where the enlarged region starts on the left, in units of the window width.
	  \param right is where the enlarged region ends on the right, in units of the window width.
	  \param bottom is where the enlarged region starts at the bottom, in units of the window height.
	  \param top is where the enlarged region ends at the top, in units of the window height.
	*/
	void blow_up(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);

	//!set the camera position along the z axis
	/*!
	  Normally, the camera is located in the (0,0,0) position of camera space. The viewing volume 
	  is shifted such that the rectangle (-1,-1,0)-(1,-1,0)-(1,1,0)-(-1,1,0) is scaled to fit on
	  the screen if z=0.
	  \param z is the z position of the camera; z>0 means the viewing volume is shifted away from the user, 
	  such that the rectangle (-1,-1,-1)-(1,-1,0)-(1,1,0)-(-1,1,0) now fits on the screen.
	*/
	void set_z_camera(GLfloat z);  
	
	//!set the aperture of the camera
	/*!
	  This function sets the viewing angle of the camera for the perspective projection.
	  \param angle is the angle in degrees. If angle is zero, the projection is orthographic.
	*/
	void set_viewing_angle(GLfloat angle);  
	
	const GLfloat* projection_matrix() const;
	const GLfloat* inverse_projection_matrix() const;
	const GLfloat* screen_to_device_matrix() const;
	const GLfloat* device_to_screen_matrix() const;
	
 	const GLfloat* center() const;
	const GLfloat* relative() const;
	const GLfloat* inset_center() const;
	const GLfloat* inset_relative() const;

	void unproject(const GLfloat* in, GLfloat* out) const;
	void project(const GLfloat* in, GLfloat* out) const;
	
      private:
	ViewingArea(GLFWwindow* ptr,GLContext* context);
	ViewingArea(const ViewingArea&);
	ViewingArea& operator=(const ViewingArea&);
	
	void update_projection_matrices();
	void determine_z_shift();

	GLfloat projectionMatrix_[N_4X4_ELEMENTS];
	GLfloat inverseProjectionMatrix_[N_4X4_ELEMENTS];
	GLfloat screenToDeviceMatrix_[N_4X4_ELEMENTS];
	GLfloat deviceToScreenMatrix_[N_4X4_ELEMENTS];
	
	GLfloat centerAndRelative_[N_SCREEN_DIMENSIONS];
	GLfloat insetCenter_[N_SCREEN_DIMENSIONS];
	GLfloat insetRelative_[N_SCREEN_DIMENSIONS];
	GLfloat leftMargin_,rightMargin_,bottomMargin_,topMargin_;
	GLfloat blowUpLeft_,blowUpRight_,blowUpBottom_,blowUpTop_;

	GLfloat zCamera_;
	GLfloat angle_;
	GLfloat zShift_;
	GLfloat zScaleOrtho_;
	
	GLfloat a_,b_;
		
	GLFWwindow* glfwWindowPointer_;
	GLContext* glContext_;
	
      };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_VIEWINGAREA_H__ */
