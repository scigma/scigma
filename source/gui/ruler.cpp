#include <sstream>
#include <cmath>
#include "glwindow.hpp"
#include "ruler.hpp"
#include "glutil.hpp"

using scigma::common::connect;
using scigma::common::connect_before;
using scigma::common::disconnect;

namespace scigma
{
  namespace gui
  {

#ifdef SCIGMA_USE_OPENGL_3_2
    // Vertex shader for rendering strings to screen
    const char* Ruler::vSource_ = 
      "in vec3 position;"
      "in vec2 offset;"
      "void main(){"
      "vec4 devicePos=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*vec4(position,1);"
      "devicePos.x+=offset.x*screenToDeviceMatrix[0].x*devicePos.w;"
      "devicePos.y+=offset.y*screenToDeviceMatrix[1].y*devicePos.w;"
      "gl_Position =devicePos;"
      "}";
    // Fragment shader for rendering strings to screen
    const char* Ruler::fSource_ =
      "out vec4 color;"
      "void main(){"
      "if(0.0==uniqueID.z){color=foregroundColor;}else{color=vec4(uniqueID.xy,gl_FragCoord.z,1);}}";
#else
    // Vertex shader for rendering strings to screen GLSL 1.2
    const char* Ruler::vSource_ =
      "attribute vec3 position;"
      "attribute vec2 offset;"
      "void main(){"
      "vec4 devicePos=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*vec4(position,1);"
      "devicePos.x+=offset.x*screenToDeviceMatrix[0].x*devicePos.w;"
      "devicePos.y+=offset.y*screenToDeviceMatrix[1].y*devicePos.w;"
      "gl_Position =devicePos;"
      "}";
    // Fragment shader for rendering strings to screen GLSL 1.2
    const char* Ruler::fSource_ = 
      "void main(){"
      "if(0.0==uniqueID.z){gl_FragColor=foregroundColor;}else{gl_FragColor=vec4(uniqueID.xy,gl_FragCoord.z,1);}}";
#endif

    Ruler::Ruler(GLWindow* glWindow,Font::Type labelFont,Font::Type tickFont):
      min_(0),max_(1),thickness_(0.5),glWindow_(glWindow),labelSpaceText_(labelFont),tickSpaceText_(tickFont),bufferSize_(0),usedSize_(0),attributesInvalid_(true),sliding_(false),hovering_(false)
    {
      glGenBuffers(1,&glBuffer_);
      GLfloat fg[]={0,0,0,-1};
      GLfloat bg[]={0,0,0,0};
      labelSpaceText_.set_color(fg,bg);
      tickSpaceText_.set_color(fg,bg);
    }
    
    Ruler::~Ruler()
    {
      glDeleteBuffers(1,&glBuffer_);
    }

    void Ruler::update_geometry(const ViewingVolume* volume)
    {
      const ViewingArea* area(glWindow_->viewing_area());
      
      /* project p1_ and p2_ to the screen:
	 first apply transformations of the viewing volume
      */
      volume->transform(p1_,q1_);
      volume->transform(p2_,q2_);

      // second, apply the projection matrix:
      area->project(q1_,q1_);
      area->project(q2_,q2_);

      /* compute c_ as unit vector pointing away from the center of the screen 
	 orthogonal to the Ruler,
	 compute d_ as unit vector pointing along the ruler with its orientation
	 from p1 to p2
      */
      d_[0]=q2_[0]-q1_[0];
      d_[1]=q2_[1]-q1_[1];
      length_=GLfloat(sqrt(d_[0]*d_[0]+d_[1]*d_[1]));
      d_[0]/=length_;
      d_[1]/=length_;
      c_[0]=-d_[1];
      c_[1]=d_[0];

      const GLfloat* center(area->center());
      // vectors from the center to the two end points in screen coordinates
      GLfloat c1[]={q1_[0]-center[0],q1_[1]-center[1]};
      GLfloat c2[]={q2_[0]-center[0],q2_[1]-center[1]};
      // make c_ point away from the center
      if((c1[0]+c2[0])*c_[0]+(c1[1]+c2[1])*c_[1]<0)
	{
	  c_[0]=-c_[0];
	  c_[1]=-c_[1];
	}
      attributesInvalid_=true;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
    bool Ruler::process(RotateEvent event, const ViewingVolume* volume, GLfloat ax,GLfloat ay, GLfloat az, GLfloat angle)
    {
      update_geometry(volume);
      return false;
    }

    bool Ruler::process(ResetEvent event, const ViewingVolume* volume)
    {
      update_geometry(volume);
      return false;
    }
    
    bool Ruler::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(thickness_<1.5f)
	return false;

      if(GLFW_MOUSE_BUTTON_LEFT==button)
	{
	  if(GLFW_PRESS==action)
	    {
	      connect<MouseMotionEvent>(w,this);
	      sliding_=true;
	      return true;
	    }
	  else
	    {
	      disconnect<MouseMotionEvent>(w,this);
	      if(!hovering_)
		{
		  disconnect<MouseButtonEvent>(w,this);
		  thickness_=0.5;
		  attributesInvalid_=true;
		  w->gl_context()->request_redraw();
		}
	      sliding_=false;
	    }
	}
      return false;
    }
    
    bool Ruler::process(MouseMotionEvent event, GLWindow* w, GLfloat x, GLfloat y, GLfloat xOld, GLfloat yOld)
    {
      if(!sliding_)
	return false;
    
      GLfloat distance((x-xOld)*d_[0]+(y-yOld)*d_[1]);
      GLfloat diff((max_-min_)*distance/length_);
      emit(this,min_-diff,max_-diff);
      attributesInvalid_=true;
      return true;
    }

    bool Ruler::process(ScrollEvent event, GLWindow* w, GLfloat xScroll, GLfloat yScroll)
    {
      GLfloat yFactor(GLfloat(pow(1.05f,yScroll)));
      GLfloat p((max_-min_)/2);
      GLfloat q((max_+min_)/2);
      emit(this,q-yFactor*p,q+yFactor*p);
      attributesInvalid_=true;
      return true;
    }

#pragma clang diagnostic pop
#pragma GCC diagnostic pop

    void Ruler::set_label(std::string label)
    {
      label_=label;
      attributesInvalid_=true;
    }

    void Ruler::set_end_points(const GLfloat* p1, const GLfloat* p2)
    {
      for(size_t i(0);i<3;++i)
	{
	  p1_[i]=p1[i];
	  p2_[i]=p2[i];
	}
      attributesInvalid_=true;
    }

    void Ruler::set_bounds(GLfloat min, GLfloat max)
    {
      min_=min;
      max_=max;
      attributesInvalid_=true;
    }

    void Ruler::on_gl_context_creation(GLContext* glContext)
    {
      glContext->create_programs<Ruler>(1);
      GLuint program; glContext->get_programs<Ruler>(&program);
      glBindAttribLocation(program,positionLocation_,"position");
      glBindAttribLocation(program,offsetLocation_,"offset");
      glContext->compile_program(program,vSource_,"",fSource_);
      glContext->link_program(program);
      GLERR;
    }
    
    void Ruler::on_gl_context_destruction(GLContext* glContext)
    {
      glContext->delete_programs<Ruler>();
      GLERR; 
    }

    void Ruler::before_batch_draw(GLContext* glContext)
    {
      GLuint program;
      glContext->get_programs<Ruler>(&program);
      glUseProgram(program);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
      GLERR;
    }


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"

    void Ruler::prepare_attributes()
    {
      glEnableVertexAttribArray(positionLocation_);
      glEnableVertexAttribArray(offsetLocation_);
      glVertexAttribPointer(positionLocation_, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glVertexAttribPointer(offsetLocation_, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(usedSize_/5*3*sizeof(GLfloat)));
    }

#pragma clang diagnostic pop

    void Ruler::on_addition(GLContext* context)
    {
      context->add_drawable(&tickSpaceText_);
      context->add_drawable(&labelSpaceText_);
      context->request_redraw();
      connect<RotateEvent>(glWindow_->viewing_volume(),this);
    }
    
    void Ruler::on_removal(GLContext* context)
    {
      context->remove_drawable(&tickSpaceText_);
      context->remove_drawable(&labelSpaceText_);
      disconnect<RotateEvent>(glWindow_->viewing_volume(),this);
      if(sliding_)
	{
	  disconnect<MouseMotionEvent>(glWindow_,this);
	  disconnect<MouseButtonEvent>(glWindow_,this);
	}
      if(hovering_)
	disconnect<ScrollEvent>(glWindow_,this);
      context->request_redraw();
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

    void Ruler::draw(GLContext* context)
    {
      if(attributesInvalid_)
	rebuild();
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
#else
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);   
      prepare_attributes();
#endif
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glDrawArrays(GL_TRIANGLES, 0, GLsizei(usedSize_/5));

      GLERR;
    }
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
    
    void Ruler::rebuild()
    {
      std::vector<GLfloat> buffer,major,minor;
      tick_values(major,minor);

      // endpoints of the ruler
      buffer.insert(buffer.end(),p1_,p1_+3);
      buffer.insert(buffer.end(),p1_,p1_+3);
      buffer.insert(buffer.end(),p2_,p2_+3);
      buffer.insert(buffer.end(),p1_,p1_+3);
      buffer.insert(buffer.end(),p2_,p2_+3);
      buffer.insert(buffer.end(),p2_,p2_+3);

      // label
      GLfloat diff(max_-min_);
      GLfloat dvec[]={p2_[0]-p1_[0],p2_[1]-p1_[1],p2_[2]-p1_[2]};
      GLfloat xLabel(p1_[0]+0.5f*dvec[0]),yLabel(p1_[1]+0.5f*dvec[1]),zLabel(p1_[2]+0.5f*dvec[2]);
      GLfloat labelDistance(50.0f);
      labelSpaceText_.set_text(&label_,&xLabel,&yLabel,&zLabel,1);
      labelSpaceText_.set_offset(c_[0]*labelDistance,c_[1]*labelDistance);

      // major ticks
      std::stringstream ss;
      size_t nTicks(major.size());
      std::vector<std::string> ticks(nTicks);
      std::vector<GLfloat> tickX(nTicks),tickY(nTicks),tickZ(nTicks);
      for(size_t i(0);i<nTicks;++i)
	{
	  GLfloat a((major[i]-min_)/diff);
	  GLfloat p[3]={p1_[0]+a*dvec[0],p1_[1]+a*dvec[1],p1_[2]+a*dvec[2]};
	  ss<<major[i]<<" ";
	  tickX[i]=p[0];tickY[i]=p[1];tickZ[i]=p[2];
	  for(size_t j(0);j<6;++j)
	    buffer.insert(buffer.end(),p,p+3);
	}
      for(size_t i(0);i<nTicks;++i)
	ss>>ticks[i];
      tickSpaceText_.set_text(ticks,tickX,tickY,tickZ);

      // minor ticks
      nTicks=minor.size();
      for(size_t i(0);i<nTicks;++i)
	{
	  GLfloat a((minor[i]-min_)/diff);
	  GLfloat p[3]={p1_[0]+a*dvec[0],p1_[1]+a*dvec[1],p1_[2]+a*dvec[2]};
	  for(size_t j(0);j<6;++j)
	    buffer.insert(buffer.end(),p,p+3);
	}
	
      // offsets for the two triangles forming the ruler
      GLfloat home[]={c_[0]*thickness_,c_[1]*thickness_};
      GLfloat away[]={-c_[0]*thickness_,-c_[1]*thickness_};
      buffer.insert(buffer.end(),home,home+2);
      buffer.insert(buffer.end(),away,away+2);
      buffer.insert(buffer.end(),home,home+2);
      buffer.insert(buffer.end(),away,away+2);
      buffer.insert(buffer.end(),home,home+2);
      buffer.insert(buffer.end(),away,away+2);

      // offsets for the major ticks
      GLfloat tickLength(20.0f);
      nTicks=major.size();
      for(size_t i(0);i<nTicks;++i)
	{
	  buffer.push_back(d_[0]*thickness_);buffer.push_back(d_[1]*thickness_);
	  buffer.push_back(-d_[0]*thickness_);buffer.push_back(-d_[1]*thickness_);
	  buffer.push_back(d_[0]*thickness_+c_[0]*tickLength);buffer.push_back(d_[1]*thickness_+c_[1]*tickLength);
	  buffer.push_back(-d_[0]*thickness_);buffer.push_back(-d_[1]*thickness_);
	  buffer.push_back(d_[0]*thickness_+c_[0]*tickLength);buffer.push_back(d_[1]*thickness_+c_[1]*tickLength);
	  buffer.push_back(-d_[0]*thickness_+c_[0]*tickLength);buffer.push_back(-d_[1]*thickness_+c_[1]*tickLength);
	}
      tickSpaceText_.set_offset(c_[0]*1.2f*tickLength,c_[1]*1.2f*tickLength);

      // offsets for the minor ticks
      tickLength=10.0f;
      nTicks=minor.size();
      for(size_t i(0);i<nTicks;++i)
	{
	  buffer.push_back(d_[0]*thickness_);buffer.push_back(d_[1]*thickness_);
	  buffer.push_back(-d_[0]*thickness_);buffer.push_back(-d_[1]*thickness_);
	  buffer.push_back(d_[0]*thickness_+c_[0]*tickLength);buffer.push_back(d_[1]*thickness_+c_[1]*tickLength);
	  buffer.push_back(-d_[0]*thickness_);buffer.push_back(-d_[1]*thickness_);
	  buffer.push_back(d_[0]*thickness_+c_[0]*tickLength);buffer.push_back(d_[1]*thickness_+c_[1]*tickLength);
	  buffer.push_back(-d_[0]*thickness_+c_[0]*tickLength);buffer.push_back(-d_[1]*thickness_+c_[1]*tickLength);
	}

      usedSize_=buffer.size();
      
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
      if(usedSize_>bufferSize_)
	{
	  bufferSize_=usedSize_;
	  glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(bufferSize_*sizeof(GLfloat)),&buffer[0],GL_DYNAMIC_DRAW);
	}
      else
	{
	  void* ptr(NULL);
	  while(!ptr)
	    ptr=glMapBuffer(GL_ARRAY_BUFFER,GL_READ_WRITE);
	  GLfloat* floatPtr=reinterpret_cast<GLfloat*>(ptr);
	  for(size_t i(0);i<usedSize_;++i)
	    floatPtr[i]=buffer[i];
	  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
	    throw(std::string("could not unmap buffer for bitmap string"));
	}

      glBindBuffer(GL_ARRAY_BUFFER,0);

#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
      prepare_attributes();
      glBindBuffer(GL_ARRAY_BUFFER,0);
      glBindVertexArray(0);
#endif
      GLERR;
      attributesInvalid_=false;
    }

    void Ruler::on_hover_begin(GLContext* context)
    {
      thickness_=1.5;
      attributesInvalid_=true;
      hovering_=true;
      if(!sliding_)
	connect_before<MouseButtonEvent>(glWindow_,this);
      connect_before<ScrollEvent>(glWindow_,this);
      context->request_redraw();
    }

    void Ruler::on_hover_end(GLContext* context)
    {
      attributesInvalid_=true;
      hovering_=false;
      if(!sliding_)
	{
	  thickness_=0.5;
	  disconnect<MouseButtonEvent>(glWindow_,this);
	}
      disconnect<ScrollEvent>(glWindow_,this);
      context->request_redraw();
    }
    
    void Ruler::tick_values(std::vector<GLfloat>& major, std::vector<GLfloat>& minor) const
    {      
      GLfloat factors[]={2.0f,2.5f,2.0f};
      GLfloat magnitude(1),diff(max_-min_);

      unsigned int i(2);
      while(diff<=magnitude)
	{
	  i=(i+1)%3;
	  magnitude/=factors[i];
	}		
      while(diff>=magnitude*factors[i])
	{
	  magnitude*=factors[i];
	  i=(i+2)%3;
	}
	
      unsigned int j(0);
      while(magnitude/diff*length_>=minMajorDist_)
	{
		j++;
		i=(i+1)%3;
		magnitude/=factors[i];
	}
      if(!j)
	return;

      GLfloat spacing(magnitude*factors[i]);
      GLfloat tick;
      
      if(min_<0)
	tick=GLfloat(int(min_/spacing))*spacing;	
      else
	tick=GLfloat(int(min_/spacing)+1)*spacing;
      while(tick<=max_)
	{
	  major.push_back(GLfloat(round(tick/spacing))*spacing);
	  tick+=spacing;
	}
    
      magnitude/=factors[i]*factors[(i+1)%3];
      if(magnitude/diff*length_>=minMinorDist_)
	spacing = magnitude;
      else
	spacing = magnitude*factors[i]*factors[(i+1)%3]/2;
      
      if(min_<0)
	tick=GLfloat(int(min_/spacing))*spacing;	
      else
	tick=GLfloat(int(min_/spacing)+1)*spacing;
      while(tick<=max_)
	{
	  minor.push_back(tick);
	  tick+=spacing;
	}
    }

  } /* end namespace gui */
} /* end namespace scigma */
