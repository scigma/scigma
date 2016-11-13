#include <iostream>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <AntTweakBar.h>
#include "viewingarea.hpp"
#include "glcontext.hpp"

namespace scigma
{
  namespace gui
  {
    
    ViewingArea::ViewingArea(GLFWwindow* ptr, GLContext* context):PythonObject<ViewingArea>(this),zCamera_(-1),angle_(0),zScaleOrtho_(0.01f),glfwWindowPointer_(ptr),glContext_(context)
    {
      for(unsigned int i = 0;i<N_4X4_ELEMENTS;i++)   
	projectionMatrix_[i]=inverseProjectionMatrix_[i]=screenToDeviceMatrix_[i]=deviceToScreenMatrix_[i]=i%5?0:1;
      centerAndRelative_[X_INDEX]=centerAndRelative_[Y_INDEX]=0;
      insetCenter_[X_INDEX]=insetCenter_[Y_INDEX]=insetRelative_[X_INDEX]=insetRelative_[Y_INDEX]=0;
      int w,h;
      glfwGetWindowSize(ptr,&w,&h);
      set_size(w,h);
      set_margins(100,100,100,100);
      blow_up(0,0,1,1);
      set_z_camera(-1);
      set_viewing_angle(45);

    }
    
    void ViewingArea::set_size(GLint width, GLint height, bool calledByUser)
    {
      if(calledByUser)
	{
	  glfwSetWindowSize(glfwWindowPointer_,width,height);
	  TwSetCurrentWindow(size_t(glContext_));
	  TwWindowSize(width,height);
	}
      
      GLfloat hCenterAndRelative=GLfloat(width)/2;
      GLfloat vCenterAndRelative=GLfloat(height)/2;
      GLfloat hChange=hCenterAndRelative-centerAndRelative_[X_INDEX];
      GLfloat vChange=vCenterAndRelative-centerAndRelative_[Y_INDEX];
      insetCenter_[X_INDEX]+=hChange;
      insetCenter_[Y_INDEX]+=vChange;
      insetRelative_[X_INDEX]+=hChange;
      insetRelative_[Y_INDEX]+=vChange;
      centerAndRelative_[X_INDEX]=deviceToScreenMatrix_[X_INDEX*N_ROWS+X_INDEX]=hCenterAndRelative;
      centerAndRelative_[Y_INDEX]=deviceToScreenMatrix_[Y_INDEX*N_ROWS+Y_INDEX]=vCenterAndRelative;
      screenToDeviceMatrix_[X_INDEX*N_ROWS+X_INDEX]=1/deviceToScreenMatrix_[X_INDEX*N_ROWS+X_INDEX];
      screenToDeviceMatrix_[Y_INDEX*N_ROWS+Y_INDEX]=1/deviceToScreenMatrix_[Y_INDEX*N_ROWS+Y_INDEX];
      deviceToScreenMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+X_INDEX]=hCenterAndRelative;
      deviceToScreenMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+Y_INDEX]=vCenterAndRelative;
      screenToDeviceMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+X_INDEX]=-1.0f;
      screenToDeviceMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+Y_INDEX]=-1.0f;

      update_projection_matrices();
      EventSource<ResizeEvent>::Type::emit(width,height);
      glContext_->update_global_uniform_4x4(GLContext::SCREEN_TO_DEVICE_MATRIX,screenToDeviceMatrix_);
      glContext_->update_global_uniform_4x4(GLContext::DEVICE_TO_SCREEN_MATRIX,deviceToScreenMatrix_);
      glContext_->set_viewport(0,0,width,height);
    }


    void ViewingArea::set_margins(GLint left,GLint right, GLint bottom, GLint top)
    {
      leftMargin_=left;
      rightMargin_=right;
      bottomMargin_=bottom;
      topMargin_=top;

      insetRelative_[X_INDEX]=centerAndRelative_[X_INDEX]-(leftMargin_+rightMargin_)/2;
      insetRelative_[Y_INDEX]=centerAndRelative_[Y_INDEX]-(bottomMargin_+topMargin_)/2;
      insetCenter_[X_INDEX]=centerAndRelative_[X_INDEX]-(rightMargin_-leftMargin_)/2;
      insetCenter_[Y_INDEX]=centerAndRelative_[Y_INDEX]-(topMargin_-bottomMargin_)/2;
      
      update_projection_matrices();
    }

    void ViewingArea::set_margin(GLint whichMargin, GLfloat pixels)
    {
      if(whichMargin&LEFT_MARGIN)
	leftMargin_=pixels;
      if(whichMargin&RIGHT_MARGIN)
	rightMargin_=pixels;
      if(whichMargin&TOP_MARGIN)
	topMargin_=pixels;
      if(whichMargin&BOTTOM_MARGIN)
	bottomMargin_=pixels;
            
      insetRelative_[X_INDEX]=centerAndRelative_[X_INDEX]-(leftMargin_+rightMargin_)/2;
      insetRelative_[Y_INDEX]=centerAndRelative_[Y_INDEX]-(bottomMargin_+topMargin_)/2;
      insetCenter_[X_INDEX]=centerAndRelative_[X_INDEX]-(rightMargin_-leftMargin_)/2;
      insetCenter_[Y_INDEX]=centerAndRelative_[Y_INDEX]-(topMargin_-bottomMargin_)/2;
      
      update_projection_matrices();
    }
    
    void ViewingArea::blow_up(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top)
    {
      blowUpLeft_=left;
      blowUpRight_=right;
      blowUpBottom_=bottom;
      blowUpTop_=top;
    }
    
    void ViewingArea::determine_z_shift()
    {
      /* Depending on the viewing angle, a shift away from camera is necessary (90 degrees
	 correspond to a viewing distance of 1). In the expression below, zCamera_ determines
	 whether the front face (-1), center (0) or backface (1) is fitted into the window.
      */
      zShift_ = GLfloat(1/tan(angle_/360*M_PI)-zCamera_); 
    }

    void ViewingArea::set_z_camera(GLfloat z)
    {
      zCamera_=z;
      determine_z_shift();
      update_projection_matrices();
    }

    void ViewingArea::set_viewing_angle(GLfloat angle)
    {
      if(angle<5.0)
	{
	  angle_=0.0;
	  /* for orthogonal projection, more entries of the projection matrices are zero
	     than for perspective projection; reset those, and also set the 4,4 element to 1
	  */
	  projectionMatrix_[Z_INDEX*N_ROWS+X_INDEX]=0.0f;
	  projectionMatrix_[Z_INDEX*N_ROWS+Y_INDEX]=0.0f;
	  projectionMatrix_[Z_INDEX*N_ROWS+N_SPATIAL_DIMENSIONS]=inverseProjectionMatrix_[Z_INDEX*N_ROWS+N_SPATIAL_DIMENSIONS]=0.0f;
	  projectionMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+Z_INDEX]=inverseProjectionMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+Z_INDEX]=0.0f;
	  projectionMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+N_SPATIAL_DIMENSIONS]=inverseProjectionMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+N_SPATIAL_DIMENSIONS]=1.0f;
	}
      else 
	{
	  if(angle>175.0)
	    angle_=175.0;
	  else
	    angle_=angle;
	  determine_z_shift();
	}
      update_projection_matrices();
    }  
    
    void ViewingArea::update_projection_matrices()
    {
      // if the inset is asymmetric, shift the viewing volume accordingly
      GLfloat xShift((insetCenter_[X_INDEX]-centerAndRelative_[X_INDEX])/centerAndRelative_[X_INDEX]);
      GLfloat yShift((insetCenter_[Y_INDEX]-centerAndRelative_[Y_INDEX])/centerAndRelative_[Y_INDEX]);
	  
      // if the inset is smaller than the window, rescale the viewing volume accordingly;
      GLfloat xScale((insetRelative_[X_INDEX]/centerAndRelative_[X_INDEX]));
      GLfloat yScale((insetRelative_[Y_INDEX]/centerAndRelative_[Y_INDEX]));
      
      // correct for the aspect ratio of the inset; 
      GLfloat r(insetRelative_[X_INDEX]/insetRelative_[Y_INDEX]);
      if(r>1)
	xScale/=r;
      else
	yScale*=r;

      // check if we have orthogonal projection
      if(!(angle_>0))
	{
	  projectionMatrix_[X_INDEX+N_ROWS*X_INDEX]=xScale;inverseProjectionMatrix_[X_INDEX+N_ROWS*X_INDEX]=1/xScale;
	  projectionMatrix_[Y_INDEX+N_ROWS*Y_INDEX]=yScale;inverseProjectionMatrix_[Y_INDEX+N_ROWS*Y_INDEX]=1/yScale;
	  projectionMatrix_[Z_INDEX+N_ROWS*Z_INDEX]=-zScaleOrtho_;inverseProjectionMatrix_[Z_INDEX+N_ROWS*Z_INDEX]=-1/zScaleOrtho_;
	  
	  projectionMatrix_[X_INDEX+N_ROWS*N_SPATIAL_DIMENSIONS]=xShift;inverseProjectionMatrix_[X_INDEX+N_ROWS*N_SPATIAL_DIMENSIONS]=-xShift/xScale;
	  projectionMatrix_[Y_INDEX+N_ROWS*N_SPATIAL_DIMENSIONS]=yShift;inverseProjectionMatrix_[Y_INDEX+N_ROWS*N_SPATIAL_DIMENSIONS]=-yShift/yScale;
	}
      else // perspective projection
	{
	  /* The original (-1,-1,-1) - (1,1,1) viewing volume must be rescaled to fit into
	     the window depending on its distance from the camera. 
	  */
	  xScale*=(zShift_+zCamera_);
	  yScale*=(zShift_+zCamera_);

	  // set near and far clipping planes (magic numbers right now -> find better way)
	  GLfloat n(zShift_/10);
	  GLfloat f(zShift_*10);

	  /* put together the perspective projection everything together in this order:
	     scale x, y and z in camera space,
	     shift z in camera space,
	     apply perspective transformation (as in gluPerspective, aspect ratio is
	     already built into xScale and yScale)
	     shift x and y in screen space (shifting in camera space would decenter the frustum)
	     the projection matrix below is the product of all these transformations
	  */
	  
	  /* note that zShift_ is positive, with larger values equivalent to a larger distance 
	     from the viewer. In camera space, however, negative values are further away from 
	     the viewer. zShift_ enters the projection matrix below such that a large positive
	     value of zShift_ will create a large negative shift in the z coordinate of the input
	     vector.
	  */
	  
	  a_=(-(f+n)/(f-n));
	  b_=(-2.0f*(f*n)/(f-n));
	  
	  // first column
	  projectionMatrix_[X_INDEX+N_ROWS*X_INDEX]=xScale;inverseProjectionMatrix_[X_INDEX+N_ROWS*X_INDEX]=1/xScale;

	  // second column
	  projectionMatrix_[Y_INDEX+N_ROWS*Y_INDEX]=yScale;inverseProjectionMatrix_[Y_INDEX+N_ROWS*Y_INDEX]=1/yScale;
	  
	  // third column
	  projectionMatrix_[N_ROWS*Z_INDEX+X_INDEX]=-xShift;
	  projectionMatrix_[N_ROWS*Z_INDEX+Y_INDEX]=-yShift;
	  projectionMatrix_[N_ROWS*Z_INDEX+Z_INDEX]=a_;inverseProjectionMatrix_[N_ROWS*Z_INDEX+Z_INDEX]=zShift_/b_;
	  projectionMatrix_[N_ROWS*Z_INDEX+N_SPATIAL_DIMENSIONS]=-1;inverseProjectionMatrix_[N_ROWS*Z_INDEX+N_SPATIAL_DIMENSIONS]=1/b_;
	  
	  // fourth column
	  projectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+X_INDEX]=zShift_*xShift;inverseProjectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+X_INDEX]=-xShift/xScale;
	  projectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+Y_INDEX]=zShift_*yShift;inverseProjectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+Y_INDEX]=-yShift/yScale;
	  projectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+Z_INDEX]=-a_*zShift_+b_;inverseProjectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+Z_INDEX]=a_*zShift_/b_-1;
	  projectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+N_SPATIAL_DIMENSIONS]=zShift_;inverseProjectionMatrix_[N_ROWS*N_SPATIAL_DIMENSIONS+N_SPATIAL_DIMENSIONS]=a_/b_;
	
	}

      glContext_->update_global_uniform_4x4(GLContext::PROJECTION_MATRIX,projectionMatrix_);
      glContext_->update_global_uniform_4x4(GLContext::INVERSE_PROJECTION_MATRIX,inverseProjectionMatrix_);

      glContext_->request_redraw();
    }
    
    const GLfloat* ViewingArea::center() const
    {
      return centerAndRelative_;
    }
    
    const GLfloat* ViewingArea::relative() const
    {
      return centerAndRelative_;
    }
    
    const GLfloat* ViewingArea::inset_center() const
    {
      return insetCenter_;
    }
    
    const GLfloat* ViewingArea::inset_relative() const
    {
      return insetRelative_;
    }
    
    void ViewingArea::unproject(const GLfloat* in, GLfloat* out) const
    {
      // expects values in left-handed coordinates in in (positive zValues point into the screen)!

      /* there are two(three) steps involved:
	 - transform from screen coordinates to device coordinates (z-value remains untouched)
	 (- revert the perspective divide if perspective projection is used)
	 - apply the inverse projection matrix
      */
      GLfloat p[N_SPATIAL_DIMENSIONS];
      for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	p[i]=in[i];
      if(angle_>0) // perspective projection
	{
	  // get the w value used for perspective division
	  GLfloat w(b_/(p[2]+a_));
	   
	  out[0]=(inverseProjectionMatrix_[0]*(screenToDeviceMatrix_[0]*p[0]+screenToDeviceMatrix_[12])+inverseProjectionMatrix_[12])*w;
	  out[1]=(inverseProjectionMatrix_[5]*(screenToDeviceMatrix_[5]*p[1]+screenToDeviceMatrix_[13])+inverseProjectionMatrix_[13])*w;
	  out[2]=(inverseProjectionMatrix_[10]*p[2]+inverseProjectionMatrix_[14])*w;
	}
      else // orthogonal projection
	{
	  out[0]=inverseProjectionMatrix_[0]*(screenToDeviceMatrix_[0]*p[0]+screenToDeviceMatrix_[12])+inverseProjectionMatrix_[12];
	  out[1]=inverseProjectionMatrix_[5]*(screenToDeviceMatrix_[5]*p[1]+screenToDeviceMatrix_[13])+inverseProjectionMatrix_[13];
	  out[2]=inverseProjectionMatrix_[10]*p[2];
	}
    }
    
    void ViewingArea::project(const GLfloat* in, GLfloat* out) const
    {
      // note that this produces coordinates in a left-handed system (z->-z)

      /* there are two(three) steps involved:
	 - apply projection matrix
	 (- apply the perspective divide if perspective projection is used)
	 - transform from device coordinates to screen coordinates (z-value remains untouched)
      */
      GLfloat p[N_SPATIAL_DIMENSIONS];
      for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	p[i]=in[i];
      if(angle_>0) // perspective projection
	{
	  out[0]=projectionMatrix_[0]*p[0]+projectionMatrix_[8]*p[2]+projectionMatrix_[12];
	  out[1]=projectionMatrix_[5]*p[1]+projectionMatrix_[9]*p[2]+projectionMatrix_[13];
	  out[2]=projectionMatrix_[10]*p[2]+projectionMatrix_[14];
	  
	  // get the w value used for perspective divide
	  GLfloat w(-p[2]+zShift_);
	  // apply perspective divide:
	  for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	    out[i]/=w;
	}
      else // orthogonal projection
	{
	  out[0]=projectionMatrix_[0]*p[0]+projectionMatrix_[12];
	  out[1]=projectionMatrix_[5]*p[1]+projectionMatrix_[13];
	  out[2]=projectionMatrix_[10]*p[2];
	}

      out[0]=deviceToScreenMatrix_[0]*out[0]+deviceToScreenMatrix_[12];
      out[1]=deviceToScreenMatrix_[5]*out[1]+deviceToScreenMatrix_[13];  
    }
            
    const GLfloat* ViewingArea::projection_matrix() const {return projectionMatrix_;}
    const GLfloat* ViewingArea::inverse_projection_matrix() const {return inverseProjectionMatrix_;}
    const GLfloat* ViewingArea::screen_to_device_matrix() const {return screenToDeviceMatrix_;}
    const GLfloat* ViewingArea::device_to_screen_matrix() const {return deviceToScreenMatrix_;}
    
  } /* end namespace gui */
} /* end namespace scigma */
