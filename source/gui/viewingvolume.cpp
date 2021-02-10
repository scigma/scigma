#include <cfloat>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "viewingvolume.hpp"
#include "glcontext.hpp"

namespace scigma
{
  namespace gui
  {

    ViewingVolume::ViewingVolume(GLContext* context):PythonObject<ViewingVolume>(this),
						     currentRotationMatrix_(matrixMemory_),
						     previousRotationMatrix_(matrixMemory_+N_4X4_ELEMENTS),
						     scalingMatrix_(matrixMemory_+2*N_4X4_ELEMENTS), 
						     translationMatrix_(matrixMemory_+3*N_4X4_ELEMENTS),
						     glContext_(context)
    {
      reset();
    }
    
    void ViewingVolume::set_range(GLint coordinate, GLfloat low, GLfloat high)
    {
      if(low>high) // enforce that min is smaller than max
	{
	  GLfloat d = low;
	  low = high;
	  high = d;
	}
      if((high-low)<10*FLT_EPSILON*std::abs(low)&&(high-low)<10*FLT_EPSILON*std::abs(high)) // no zero range
	return;

      for(unsigned int i=0;i<N_COORDINATES;++i) 
	{
	  if(coordinate&(1<<i))
	    {
	      min_[i]=low;
	      max_[i]=high;
	      center_[i]=(low+high)/2;
	      relative_[i]=(high-low)/2;
	      if(i<N_SPATIAL_DIMENSIONS)
		{
		  scalingMatrix_[i*N_ROWS+i]=1/relative_[i];
		  translationMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+i]=-center_[i];
		}
	    }    
	}
      EventSource<RangeEvent>::Type::emit(this);
      glContext_->update_global_uniform_4x4(GLContext::TRANSLATION_MATRIX,translationMatrix_);
      glContext_->update_global_uniform_4x4(GLContext::SCALING_MATRIX,scalingMatrix_);
      GLfloat ctMinMax[] = {min_[C_INDEX], max_[C_INDEX], min_[T_INDEX], max_[T_INDEX]};
      glContext_->update_global_uniform_4(GLContext::CTMINMAX_VECTOR, ctMinMax);
      glContext_->request_redraw();
    }
    
    void ViewingVolume::shift(GLfloat dx, GLfloat dy, GLfloat dz)
    {
      center_[X_INDEX]-=dx;min_[X_INDEX]-=dx;max_[X_INDEX]-=dx;
      translationMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+X_INDEX]+=dx;
      center_[Y_INDEX]-=dy;min_[Y_INDEX]-=dy;max_[Y_INDEX]-=dy;
      translationMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+Y_INDEX]+=dy;
      center_[Z_INDEX]-=dz;min_[Z_INDEX]-=dz;max_[Z_INDEX]-=dz;
      translationMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+Z_INDEX]+=dz;
      EventSource<ShiftEvent>::Type::emit(this,dx,dy,dz);
      glContext_->update_global_uniform_4x4(GLContext::TRANSLATION_MATRIX,translationMatrix_);
      glContext_->request_redraw();
    }
    
    void ViewingVolume::shift(GLint coordinate, GLfloat distance)
    {
      for(unsigned int i=0;i<N_COORDINATES;++i)
	{
	  if(1<<i==coordinate)
	    {
	      center_[i]-=distance;
	      min_[i]-=distance;
	      max_[i]-=distance;
	      if(i<N_SPATIAL_DIMENSIONS)
		{
		  translationMatrix_[N_SPATIAL_DIMENSIONS*N_ROWS+i]+=distance;
		  EventSource<ShiftEvent>::Type::emit(this,
						      X_INDEX==i?distance:0,
						      Y_INDEX==i?distance:0,
						      Z_INDEX==i?distance:0);
		  glContext_->update_global_uniform_4x4(GLContext::TRANSLATION_MATRIX,translationMatrix_);
		  glContext_->request_redraw();
		}
	      return;
	    }
	}
      
      /* if shifting along screen coordinates, compute a vector of length distance 
	 in the direction of the shift and call shift(x,y,z) with this vector      */
      if(X_SCREEN_COORDINATE==coordinate)
	shift(distance*currentRotationMatrix_[0]*relative_[X_COORDINATE],distance*currentRotationMatrix_[4]*relative_[Y_COORDINATE],distance*currentRotationMatrix_[8]*relative_[Z_COORDINATE]);
      else if(Y_SCREEN_COORDINATE==coordinate)
	shift(distance*currentRotationMatrix_[1]*relative_[X_COORDINATE],distance*currentRotationMatrix_[5]*relative_[Y_COORDINATE],distance*currentRotationMatrix_[9]*relative_[Z_COORDINATE]);
      else if(Z_SCREEN_COORDINATE==coordinate)
	shift(distance*currentRotationMatrix_[2]*relative_[X_COORDINATE],distance*currentRotationMatrix_[6]*relative_[Y_COORDINATE],distance*currentRotationMatrix_[10]*relative_[Z_COORDINATE]);
    }
    
    void ViewingVolume::scale(GLint coordinate, GLfloat factor)
    {
      if (factor<0)    // enforce that min < max
	factor=-factor;
      
      bool spatialDimensionScaled(false);
      GLfloat factors[3];
      
      for(unsigned int i=0;i<N_COORDINATES;++i)
	{
	  if(coordinate&(1<<i))
	    {
	      if((relative_[i]*factor<10*FLT_EPSILON*std::abs(max_[i]))&&(relative_[i]*factor<10*FLT_EPSILON*std::abs(min_[i])))
		{
		  factors[i]=1;
		  continue;
		}
	      relative_[i]*=factor;
	      min_[i]=center_[i]-relative_[i];
	      max_[i]=center_[i]+relative_[i];
	      if(i<N_SPATIAL_DIMENSIONS)
		{
		  spatialDimensionScaled=true;
		  scalingMatrix_[i*N_ROWS+i]=1/relative_[i];
		  factors[i]=factor;
		}
	    }
	}
      if(spatialDimensionScaled)
	{
	  EventSource<ScaleEvent>::Type::emit(this,factors[X_INDEX],factors[Y_INDEX],factors[Z_INDEX]);   
	  glContext_->update_global_uniform_4x4(GLContext::SCALING_MATRIX,scalingMatrix_);
	  glContext_->request_redraw();
	}
    }
    
    void ViewingVolume::rotate(GLfloat ax, GLfloat ay, GLfloat az, GLfloat angle)
    {
      GLfloat length(GLfloat(sqrt(ax*ax+ay*ay+az*az))); 
      ax/=length;ay/=length;az/=length;                            // normalize axis vector
      GLfloat s(GLfloat(sin(angle/180*GLfloat(M_PI)))),c(GLfloat(cos(angle/180*GLfloat(M_PI))));
      GLfloat t(1-c);
      GLfloat additionalRotationMatrix[]={ax*ax*t+c, ax*ay*t+az*s, ax*az*t-ay*s, 0,
					  ay*ax*t-az*s, ay*ay*t+c, ay*az*t+ax*s, 0,
					  az*ax*t+ay*s, az*ay*t-ax*s, az*az*t+c, 0,
					  0, 0, 0, 1};
      
      // apply incremental rotation defined by the parameters to the current rotation matrix
      for(unsigned int i=0;i!=N_SPATIAL_DIMENSIONS;++i)
	for(unsigned int j=0;j!=N_SPATIAL_DIMENSIONS*N_ROWS;j+=N_ROWS)
	  previousRotationMatrix_[i+j]=additionalRotationMatrix[i]*currentRotationMatrix_[j]+additionalRotationMatrix[i+N_ROWS]*currentRotationMatrix_[j+1]+additionalRotationMatrix[i+2*N_ROWS]*currentRotationMatrix_[j+2];
      
      // use the updated matrix as current rotation matrix from now on
      GLfloat* dummy=previousRotationMatrix_;
      previousRotationMatrix_=currentRotationMatrix_;
      currentRotationMatrix_=dummy;
      
      EventSource<RotateEvent>::Type::emit(this,ax,ay,az,angle);
      glContext_->update_global_uniform_4x4(GLContext::ROTATION_MATRIX,currentRotationMatrix_);
      glContext_->request_redraw();
    }
    
    void ViewingVolume::rotate(GLint coordinate,GLfloat angle)
    {
      switch(coordinate)
	{
	case X_SCREEN_COORDINATE:
	  rotate(1,0,0,angle);
	  return;
	case Y_SCREEN_COORDINATE:
	  rotate(0,1,0,angle);
	  return;
	case Z_SCREEN_COORDINATE:
	  rotate(0,0,1,angle);
	  return;
	case X_COORDINATE:
	  rotate(currentRotationMatrix_[0],currentRotationMatrix_[1],currentRotationMatrix_[2],angle);
	  return;
	case Y_COORDINATE:
	  rotate(currentRotationMatrix_[4],currentRotationMatrix_[5],currentRotationMatrix_[6],angle);
	  return;
	case Z_COORDINATE:
	  rotate(currentRotationMatrix_[8],currentRotationMatrix_[9],currentRotationMatrix_[10],angle);
	}
    }
    
    void ViewingVolume::reset()
    {
      set_range(ALL_COORDINATES,-1,1);
      for(unsigned int i = 0;i<N_4X4_ELEMENTS;i++)   // make rotation, scale, translation matrices diag(1,1,1,1)
	translationMatrix_[i]=scalingMatrix_[i]=currentRotationMatrix_[i]=previousRotationMatrix_[i]=i%5?0:1;
      
      EventSource<ResetEvent>::Type::emit(this);
      glContext_->update_global_uniform_4x4(GLContext::SCALING_MATRIX,scalingMatrix_);
      glContext_->update_global_uniform_4x4(GLContext::TRANSLATION_MATRIX,translationMatrix_);
      glContext_->update_global_uniform_4x4(GLContext::ROTATION_MATRIX,currentRotationMatrix_);
      GLfloat ctMinMax[] = {min_[C_INDEX], max_[C_INDEX], min_[T_INDEX], max_[T_INDEX]};
      glContext_->update_global_uniform_4(GLContext::CTMINMAX_VECTOR, ctMinMax);
      
      glContext_->request_redraw();
    }
    
    void ViewingVolume::reset_rotation()
    {
      for(unsigned int i = 0;i<N_4X4_ELEMENTS;i++)   // make rotation  matrix diag(1,1,1,1)
	currentRotationMatrix_[i]=previousRotationMatrix_[i]=i%5?0:1;
      
      EventSource<ResetEvent>::Type::emit(this);
      glContext_->update_global_uniform_4x4(GLContext::ROTATION_MATRIX,currentRotationMatrix_);
      glContext_->request_redraw();
    }

    void ViewingVolume::set_center(GLfloat x, GLfloat y, GLfloat z)
    {
      shift(x-center_[X_INDEX],y-center_[Y_INDEX],z-center_[Z_INDEX]);
    }
    
    /*void ViewingVolume::set_screen_direction(GLint screenCoordinate, GLfloat x, GLfloat y, GLfloat z)
    {
      // not yet implemented
    }
    
    void ViewingVolume::set_screen_direction(GLint screenCoordinate, GLint spaceCoordinate)
    {
      // not yet implemented
    }*/
    
    const GLfloat* ViewingVolume::min() const {return min_;}
    const GLfloat* ViewingVolume::max() const {return max_;}
    const GLfloat* ViewingVolume::center() const {return center_;}
    const GLfloat* ViewingVolume::relative() const {return relative_;}
    const GLfloat* ViewingVolume::screen_direction(GLint screenCoordinate) const {return currentRotationMatrix_+N_ROWS*GLuint(screenCoordinate);}
    const GLfloat* ViewingVolume::rotation_matrix() const {return currentRotationMatrix_;}
    const GLfloat* ViewingVolume::scaling_matrix() const{return scalingMatrix_;}
    const GLfloat* ViewingVolume::translation_matrix() const{return translationMatrix_;}
    
    void ViewingVolume::transform(const GLfloat* in, GLfloat* out) const
    {
      GLfloat p[N_SPATIAL_DIMENSIONS];
      for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	p[i]=(in[i]-center_[i])/relative_[i];
      out[0]=currentRotationMatrix_[0]*p[0]+currentRotationMatrix_[4]*p[1]+currentRotationMatrix_[8]*p[2];
      out[1]=currentRotationMatrix_[1]*p[0]+currentRotationMatrix_[5]*p[1]+currentRotationMatrix_[9]*p[2];
      out[2]=currentRotationMatrix_[2]*p[0]+currentRotationMatrix_[6]*p[1]+currentRotationMatrix_[10]*p[2];
    }
    
    void ViewingVolume::untransform(const GLfloat* in, GLfloat* out) const
    {
      GLfloat p[N_SPATIAL_DIMENSIONS];
      p[0]=currentRotationMatrix_[0]*in[0]+currentRotationMatrix_[1]*in[1]+currentRotationMatrix_[2]*in[2];
      p[1]=currentRotationMatrix_[4]*in[0]+currentRotationMatrix_[5]*in[1]+currentRotationMatrix_[6]*in[2];
      p[2]=currentRotationMatrix_[8]*in[0]+currentRotationMatrix_[9]*in[1]+currentRotationMatrix_[10]*in[2];
      for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	out[i]=p[i]*relative_[i]+center_[i];
    }
    
  } /* end namespace gui */
} /* end namespace scigma */
