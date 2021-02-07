#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "navigator.hpp"

using scigma::common::connect;
using scigma::common::disconnect;

namespace scigma
{
  namespace gui
  {

    Navigator::Navigator(int view):PythonObject<Navigator>(this),view_(view),rightMouseButtonPressed_(false),shiftKeyPressed_(false)
    {
      padding_[0]=0; // to suppress -Wunused-private-field on padding_
    }
    
    Navigator::~Navigator()
    {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    bool Navigator::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(GLFW_MOUSE_BUTTON_LEFT==button)
	{
	  if(GLFW_PRESS==action)
	    {
	      connect<MouseMotionEvent>(w,this);
	      if(GLFW_MOD_SHIFT&mods)
		shiftKeyPressed_=true;
	      return true;
	    }
	  else
	    {
	      disconnect<MouseMotionEvent>(w,this);
	      shiftKeyPressed_=false;
	      return false;
	    }
	}
      else if(GLFW_MOUSE_BUTTON_RIGHT==button)
	{
	  rightMouseButtonPressed_=(GLFW_PRESS==action);
	}
      return false;
    }
    
    bool Navigator::process(MouseMotionEvent event, GLWindow* w, GLfloat x, GLfloat y, GLfloat xOld, GLfloat yOld)
    {
      ViewingArea* area=w->viewing_area();
      ViewingVolume* volume=w->viewing_volume();
      
      if(!shiftKeyPressed_) // rotations
	{
	  if(!(view_&Z_COORDINATE))
	    return true;
	  if(rightMouseButtonPressed_) 
	    {
	      const GLfloat* center=area->center();
	      GLfloat angle(GLfloat(atan2(y-center[1],x-center[0])-atan2(yOld-center[1],xOld-center[0])));
	      volume->rotate(Z_SCREEN_COORDINATE,180.f*angle/GLfloat(M_PI));
	    }
	  else
	    {
	      GLfloat dx(x-xOld), dy(y-yOld);
	      if(dx*dx>0.0f||dy*dy>0.0f)
		volume->rotate(-dy,dx,0,GLfloat(sqrt(dx*dx+dy*dy)/N_SPATIAL_DIMENSIONS));
	    }
	}
      else // shifts
	{
	  const GLfloat* rot=volume->rotation_matrix();
	  const GLfloat* scale=volume->scaling_matrix();

	  if(rightMouseButtonPressed_)
	    {
	      if(!(view_&Z_COORDINATE))
		return true;

              GLfloat distance=(y-yOld)/(area->inset_relative()[1]);

 	      volume->shift(-distance*rot[2]/scale[0], -distance*rot[6]/scale[5], -distance*rot[10]/scale[10]);
	    }
	  else
	    {
	      const GLfloat* insetRelative(area->inset_relative()); 
	      GLfloat ratio(insetRelative[0]/insetRelative[1]);
	      GLfloat dx=ratio>1?ratio*(x-xOld)/insetRelative[0]:(x-xOld)/insetRelative[0];
	      GLfloat dy=ratio>1?(y-yOld)/insetRelative[1]:(y-yOld)/insetRelative[1]/ratio;

	      volume->shift((dx*rot[0]+dy*rot[1])/scale[0],
			    (dx*rot[4]+dy*rot[5])/scale[5],
			    (dx*rot[8]+dy*rot[9])/scale[10]);
	    }
	}
      return true;
    }
    
    bool Navigator::process(ScrollEvent event, GLWindow* w, GLfloat xScroll, GLfloat yScroll)
    {
      GLfloat yFactor(GLfloat(pow(1.05f,yScroll)));
      w->viewing_volume()->scale(X_COORDINATE|Y_COORDINATE|Z_COORDINATE,yFactor);
      return false;
    }
    
#pragma GCC diagnostic pop

  } /* end namespace gui */
} /* end namespace scigma */
