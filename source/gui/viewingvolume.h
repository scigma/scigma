#ifndef __SCIGMA_GUI_VIEWINGVOLUME_H__
#define __SCIGMA_GUI_VIEWINGVOLUME_H__

#include "../events.h"
#include "../pythonobject.h"
#include "definitions.h"
#include "viewingvolumeevents.h"

namespace scigma
{
  namespace gui
  {
    
    class GLContext;
    
    //!A class that handles translation, rotation and scaling of the displayed data.
    /*!@ingroup gui
      Overall, there are 5 available dimensions: three spatial coordinates x,y,z, the time
      coordinate and a color coordinate. ViewingVolume keeps track of the limits of these 
      coordinates as well as the orientation of the spatial coordinates displayed in a Window.
      It allows to manipulate these parameters as well. In OpenGL vernacular,
      ViewingVolume is responsible for the modelview matrix.
    */
    class ViewingVolume : public PythonObject<ViewingVolume>,
      public EventSource<ScaleEvent>::Type,  
      public EventSource<ShiftEvent>::Type,
      public EventSource<RotateEvent>::Type,  
      public EventSource<RangeEvent>::Type,  
      public EventSource<ResetEvent>::Type
      {
	
	friend class GLWindow;
	
      public:
	
	//!adjust the dimensions of the viewing volume 
	/*!
	  This function sets the upper and lower boundaries of the viewing volume.
	  @param coordinate can be X_COORDINATE, Y_COORDINATE, Z_COORDINATE, T_COORDINATE, 
	  C_COORDINATE or any combination of the five (via 
	  bitwise or, ALL_COORDINATES combines all of them); determines which coordinates 
	  are affected.
	  @param low is the lower bound of the new range.
	  @param high is the upper bound of the new range.
	*/
	void set_range(GLint coordinate, GLfloat low, GLfloat high);
	
	//!shift the center of the viewing volume in an arbitrary direction in 3D space
	/*!	
	  This function shifts the center of the viewing volume by the specified values.
	  @param dx is the x-component of the shift.
	  @param dy is the y-component of the shift.
	  @param dz is the z-component of the shift.
	*/
	void shift(GLfloat dx, GLfloat dy, GLfloat dz);
	
	//!shift the center of the viewing volume in a predefined direction
	/*!
	  This function shifts the viewing volume along the specified axis by the given distance. 
	  @param coordinate can be X_COORDINATE, Y_COORDINATE, Z_COORDINATE, T_COORDINATE,
	  C_COORDINATE, X_SCREEN_COORDINATE, Y_SCREEN_COORDINATE or 
	  Z_SCREEN_COORDINATE; determines the direction along which the viewing volume is shifted.
	  @param distance determines how large the shift is.
	*/
	void shift(GLint coordinate, GLfloat distance);
	
	//!expand or shrink the viewing volume 
	/*!	
	  This function scales the viewing volume along the specified axes by the given factor.
	  @param coordinate can be X_COORDINATE, Y_COORDINATE, Z_COORDINATE, T_COORDINATE, 
	  C_COORDINATE or any combination of the five (via 
	  bitwise or, ALL_COORDINATES combines all of them); determines which coordinates 
	  are affected.
	  @param factor by which the current extension of the viewing volume is scaled.
	*/
	void scale(GLint coordinate, GLfloat factor);
	
	//!rotate the viewing volume around a custom axis
	/*!	
	  This function rotates the viewing volume around the specified axis by the given angle. 
	  @param angle is the angle of the rotation in degrees.
	  @param ax is the x-component of the axis vector.
	  @param ay is the y-component of the axis vector.
	  @param az is the z-component of the axis vector.
	*/
	void rotate(GLfloat ax, GLfloat ay, GLfloat az, GLfloat angle);
	
	//!rotates the viewing volume around a predefined axis
	/*!	
	  This function rotates the viewing volume around the specified axis by the given angle. 
	  @param coordinate can be X_COORDINATE, Y_COORDINATE, Z_COORDINATE, X_SCREEN_COORDINATE,
	  Y_SCREEN_COORDINATE or Z_SCREEN_COORDINATE; determines the axis around which the
	  rotation is performed.
	  @param angle is the angle of the rotation in degrees.
	*/
	void rotate(GLint coordinate,GLfloat angle);
	
	//!reset the viewing volume to standard values 
	/*!	
	  After calling this function, x, y, z, time and color all range from -1 to 1, x-axis and
	  y-axis are aligned with the screen and the z-axis points out of the screen.
	*/
	void reset();

	void reset_rotation();
	
	//!set the center of the viewing volume.
	/*!
	  This function shifts the viewing volume to a new center.
	  @param x is the x coordinate of the new center.
	  @param y is the y coordinate of the new center.
	  @param z is the z coordinate of the new center.
	*/
	void set_center(GLfloat x, GLfloat y, GLfloat z);
	
	//!align an arbitray vector with the screen coordinates
	/*!
	  This function rotates the viewing volume such that the specified vector is aligned with
	  the designated direction on screen.       
	  @param screenCoordinate is either X_SCREEN_COORDINATE, Y_SCREEN_COORDINATE or
	  Z_SCREEN_COODINATE; determines whether (x,y,z) is going to point right, up or towards
	  the screen from the user's perspective.  
	  @param x is the x component of the vector.
	  @param y is the y coordinate of the vector.
	  @param z is the z coordinate of the vector.
	*/
	//void set_screen_direction(GLint screenCoordinate, GLfloat x, GLfloat y, GLfloat z);
	
	//!align a coordinate axis of the viewing volume with the screen coordinates
	/*!
	  This function rotates the viewing volume such that the specified coordinate axis 
	  is aligned with the designated direction on screen.
	  @param screenCoordinate is either X_SCREEN_COORDINATE, Y_SCREEN_COORDINATE or 
	  Z_SCREEN_COODINATE; determins whether the axis defined by spaceCoordinate is going to point
	  right, up or towards the screen from the users perspective.  
	  @param spaceCoordinate is either X_COORDINATE, Y_COORDINATE or Z_COORDINATE.
	*/
	//void set_screen_direction(GLint screenCoordinate, GLint spaceCoordinate);
	
	//!get lower bounds of viewing coordinates
	/*!
	  @return constant pointer to a 7-dimensional array of floats containing the lower
	  bounds for x, y, z, t, r, g and b coordinates.
	*/
	const GLfloat* min() const;
	
	//!get upper bounds of viewing coordinates
	/*!
	  @return constant pointer to a 7-dimensional array of floats containing the upper
	  bounds for x, y, z, t, r, g and b coordinates.
	*/
	const GLfloat* max() const;
	
	//!get center of the viewing volume
	/*!
	  @return constant pointer to a 7-dimensional array of floats containing the center value 
	  for x, y, z, t, r, g and b coordinates (i.e. (max+min)/2).
	*/
	const GLfloat* center() const;
	
	//!get dimensions of the coordinate system
	/*!
	  @return constant pointer to an array of 7 floats containing the
	  difference, divided by two, between the upper and lower bounds for x, y, z,
	  t, r, g and b coordinates (i.e. (max-min)/2). 
	*/    
	const GLfloat* relative() const;
	
	//!get the vector currently aligned with the specified direction in screen coordinates
	/*!
	  @param screenCoordinate is either X_SCREEN_COORDINATE, Y_SCREEN_COORDINATE or
	  Z_SCREEN_COODINATE.
	  @return constant pointer to an array of 3 floats specifying the x, y and z 
	  components of the vector in viewing coordinates.
	*/
	const GLfloat* screen_direction(GLint screenCoordinate) const;
	
	//!get the current rotation matrix of the viewing volume
	/*!
	  @return constant pointer to an array of 16 floats specifying the 4x4 
	  rotation matrix of the viewing volume currently in effect, in column major ordering.
	*/
	const GLfloat* rotation_matrix() const;
	
	//!get the current scaling matrix of the viewing volume
	/*!
	  @return constant pointer to an array of 16 floats specifying the 4x4 
	  scaling matrix of the viewing volume currently in effect, in column major ordering.
	*/
	const GLfloat* scaling_matrix() const;
	
	//!get the current rotation matrix of the viewing volume
	/*!
	  @return constant pointer to an array of 16 floats specifying the 4x4 
	  translation matrix of the viewing volume currently in effect, in column major ordering.
	*/
	const GLfloat* translation_matrix() const;
	
	void transform(const GLfloat* in, GLfloat* out) const;
	void untransform(const GLfloat* in, GLfloat* out) const;

      private:
	ViewingVolume(GLContext* context);
	ViewingVolume(const ViewingVolume&);
	ViewingVolume& operator=(const ViewingVolume&);
	
	GLfloat min_[N_COORDINATES];
	GLfloat max_[N_COORDINATES];
	GLfloat center_[N_COORDINATES];
	GLfloat relative_[N_COORDINATES];
	GLfloat* currentRotationMatrix_;
	GLfloat* previousRotationMatrix_;
	GLfloat* scalingMatrix_;
	GLfloat* translationMatrix_;
	GLfloat matrixMemory_[N_4X4_ELEMENTS*4];
	
	GLContext* glContext_;
      };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_VIEWINGVOLUME_H__ */
