#include "cosy.h"
#include "viewingvolume.h"
#include "glcontext.h"
#include "glwindow.h"

namespace scigma
{
  namespace gui
  {
    
    Cosy::Cosy(GLWindow* glWindow, int view,bool largeFontsFlag):
      PythonObject<Cosy>(this),
      xRuler_(glWindow,largeFontsFlag?Font::DEJAVU_SERIF_48:Font::DEJAVU_SERIF_24,largeFontsFlag?Font::DEJAVU_SERIF_32:Font::DEJAVU_SERIF_16),
      yRuler_(glWindow,largeFontsFlag?Font::DEJAVU_SERIF_48:Font::DEJAVU_SERIF_24,largeFontsFlag?Font::DEJAVU_SERIF_32:Font::DEJAVU_SERIF_16),
      zRuler_(glWindow,largeFontsFlag?Font::DEJAVU_SERIF_48:Font::DEJAVU_SERIF_24,largeFontsFlag?Font::DEJAVU_SERIF_32:Font::DEJAVU_SERIF_16),
      glWindow_(glWindow),
      view_(view)
    {
      glWindow_->gl_context()->stall();
      glWindow_->gl_context()->add_drawable(&xRuler_);
      glWindow_->gl_context()->add_drawable(&yRuler_);
      if(view&Z_COORDINATE)
	glWindow_->gl_context()->add_drawable(&zRuler_);
      xRuler_.set_label("x");
      yRuler_.set_label("y");
      zRuler_.set_label("z");
      update_rulers(glWindow_->viewing_volume());
      glWindow_->gl_context()->flush();
      xRuler_.connect(this);
      yRuler_.connect(this);
      zRuler_.connect(this);

      glWindow_->viewing_volume()->EventSource<RangeEvent>::Type::connect(this);
      glWindow_->viewing_volume()->EventSource<ShiftEvent>::Type::connect(this);
      glWindow_->viewing_volume()->EventSource<ScaleEvent>::Type::connect(this);
      glWindow_->viewing_volume()->EventSource<ResetEvent>::Type::connect(this);
      glWindow_->viewing_volume()->EventSource<RotateEvent>::Type::connect(this);
    }

    Cosy::~Cosy()
    {
      glWindow_->viewing_volume()->EventSource<RangeEvent>::Type::disconnect(this);
      glWindow_->viewing_volume()->EventSource<ShiftEvent>::Type::disconnect(this);
      glWindow_->viewing_volume()->EventSource<ScaleEvent>::Type::disconnect(this);
      glWindow_->viewing_volume()->EventSource<ResetEvent>::Type::disconnect(this);
      glWindow_->viewing_volume()->EventSource<RotateEvent>::Type::disconnect(this);

      glWindow_->gl_context()->stall();
      glWindow_->gl_context()->remove_drawable(&xRuler_);
      glWindow_->gl_context()->remove_drawable(&yRuler_);
      glWindow_->gl_context()->remove_drawable(&zRuler_);
      glWindow_->gl_context()->flush();
    }
    
    void Cosy::set_view(int view)
    {
      if(!(view_&Z_COORDINATE))
	if(view&Z_COORDINATE)
	  glWindow_->gl_context()->add_drawable(&zRuler_);

      if(view_&Z_COORDINATE)
	if(!(view&Z_COORDINATE))
	  glWindow_->gl_context()->remove_drawable(&zRuler_);
    
      view_=view;
    }
    
    void Cosy::set_label(int coordinate,std::string label)
    {
      if(X_COORDINATE==coordinate)
	xRuler_.set_label(label);
      else if(Y_COORDINATE==coordinate)
	yRuler_.set_label(label);
      else if(Z_COORDINATE==coordinate)
	zRuler_.set_label(label);
      glWindow_->gl_context()->request_redraw();
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
    void Cosy::update_rulers(const ViewingVolume* volume)
    {
      const GLfloat* min(volume->min());
      const GLfloat* max(volume->max());
      const GLfloat* rot(volume->rotation_matrix());

      // pick the corner opposite the user and let all rulers meet there
      GLfloat corners[8][3]={{min[0],min[1],min[2]},{min[0],min[1],max[2]},
			     {min[0],max[1],min[2]},{min[0],max[1],max[2]},
			     {max[0],min[1],min[2]},{max[0],min[1],max[2]},
			     {max[0],max[1],min[2]},{max[0],max[1],max[2]}};
      GLfloat minZ(1e16f);
      int corner(0);
      for(int i(0);i<8;++i)
	{
	  GLfloat Z(rot[2]*corners[i][0]+rot[6]*corners[i][1]+rot[10]*corners[i][2]);
	  if(Z<minZ)
	    {
	      minZ=Z;
	      corner=i;
	    }
	}
      GLfloat x(corners[corner][0]);
      GLfloat y(corners[corner][1]);
      GLfloat z(corners[corner][2]);
      
      GLfloat px1[]={min[0],y,z};GLfloat px2[]={max[0],y,z};
      GLfloat py1[]={x,min[1],z};GLfloat py2[]={x,max[1],z};
      GLfloat pz1[]={x,y,min[2]};GLfloat pz2[]={x,y,max[2]};

      xRuler_.set_end_points(px1,px2);
      yRuler_.set_end_points(py1,py2);
      zRuler_.set_end_points(pz1,pz2);

      xRuler_.set_bounds(min[0],max[0]);
      yRuler_.set_bounds(min[1],max[1]);
      zRuler_.set_bounds(min[2],max[2]);

      xRuler_.update_geometry(volume);
      yRuler_.update_geometry(volume);
      zRuler_.update_geometry(volume);
    }

    bool Cosy::process(ScaleEvent event, const ViewingVolume* volume, GLfloat xFactor, GLfloat yFactor, GLfloat zFactor)
    {
      update_rulers(volume);
      return false;
    }

    bool Cosy::process(ShiftEvent event, const ViewingVolume* volume, GLfloat xShift, GLfloat yShift, GLfloat zShift)
    {
      update_rulers(volume);
      return false;
    }
    
    bool Cosy::process(RotateEvent event, const ViewingVolume* volume, GLfloat x, GLfloat y, GLfloat z, GLfloat angle)
    {
      update_rulers(volume);
      return false;
    }

    bool Cosy::process(RangeEvent event, const ViewingVolume* volume)
    {
      update_rulers(volume);
      return false;
    }
    
    bool Cosy::process(ResetEvent event, const ViewingVolume* volume)
    {
      update_rulers(volume);
      return false;
    }
    
    bool Cosy::process(AdjustEvent event, const Ruler* ruler, GLfloat min, GLfloat max)
    {
      if(&xRuler_==ruler)
	glWindow_->viewing_volume()->set_range(X_COORDINATE,min,max);
      else if(&yRuler_==ruler)
	glWindow_->viewing_volume()->set_range(Y_COORDINATE,min,max);
      else if(&zRuler_==ruler)
	glWindow_->viewing_volume()->set_range(Z_COORDINATE,min,max);
      return true;
    }

    void Cosy::draw(GLContext* glContext)
    {

    }

#pragma clang diagnostic pop
#pragma GCC diagnostic pop
    
  } /* end namespace gui */
} /* end namespace scigma */
