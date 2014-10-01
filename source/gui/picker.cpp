#include "definitions.h"
#include "glwindow.h"
#include "picker.h"
#include "graph.h"
#include <iostream>

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace scigma
{
  namespace gui
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

    std::map<GLContext*,GLint> Picker::spriteLocationMap_;
    std::map<GLContext*,GLint> Picker::sizeLocationMap_;

#pragma clang diagnostic pop
    
    GLint Picker::spriteLocation_(0);
    GLint Picker::sizeLocation_(0);

#ifdef SCIGMA_USE_OPENGL_3_2
    const char* Picker::vSource_=
      "in vec4 position;"
      "out vec2 screenPos;"
      "void main(){"
      "vec4 devicePos=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*position;"
      "gl_Position=devicePos;"
      "devicePos=devicePos/devicePos.w;"
      "screenPos=vec2(devicePos.x*deviceToScreenMatrix[0].x+deviceToScreenMatrix[3].x,devicePos.y*deviceToScreenMatrix[1].y+deviceToScreenMatrix[3].y);"
      "}";
    const char* Picker::fSource_=
      "uniform int sprite;"
      "uniform float size;"
      "uniform sampler2D sampler;"
      "in vec2 screenPos;"
      "out vec4 color;\n"
      "void main(){"
      "vec2 pointCoord=(screenPos-gl_FragCoord.xy)/size+vec2(0.5,0.5);"
      "if(0.0==uniqueID.z){color=(sprite==0)?foregroundColor:texture(sampler,pointCoord).r*vec4(1,1,1,1);if(0.0==color.w)discard;}else{color=vec4(uniqueID.xy,gl_FragCoord.z,1);}"
      "}";
#else
    const char* Picker::vSource_=
      "attribute vec4 position;"
      "varying vec2 screenPos;"
      "void main(){"
      "vec4 devicePos=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*position;"
      "gl_Position=devicePos;"
      "devicePos=devicePos/devicePos.w;"
      "screenPos=vec2(devicePos.x*deviceToScreenMatrix[0].x+deviceToScreenMatrix[3].x,devicePos.y*deviceToScreenMatrix[1].y+deviceToScreenMatrix[3].y);"
      "}";
    const char* Picker::fSource_=
      "uniform int sprite;"
      "uniform float size;"
      "uniform sampler2D sampler;"
      "varying vec2 screenPos;"
      "void main(){"
      "vec2 pointCoord=(screenPos-gl_FragCoord.xy)/size+vec2(0.5,0.5);"
      "if(0.0==uniqueID.z){vec4 color=(sprite==0)?foregroundColor:texture2D(sampler,pointCoord).r*vec4(1,1,1,1);if(0.0==color.w)discard;gl_FragColor=color;}else{gl_FragColor=vec4(uniqueID.xy,gl_FragCoord.z,1);}"
      "}";
#endif
    
    Picker::Picker(int view):PythonObject<Picker>(this),doubleClickTime_(0.25),lastClickTime_(-1.0),
			     marker_(Marker::RCROSS), markerSize_(25.0),view_(view),visible_(false)
    {
      glGenBuffers(1,&glBuffer_);
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
      glBufferData(GL_ARRAY_BUFFER,sizeof(points_),NULL,GL_DYNAMIC_DRAW);
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
      prepare_attributes();
      glBindVertexArray(0);
#endif
      glBindBuffer(GL_ARRAY_BUFFER,0);
      GLERR;
      padding_[0]=0;
    }
    
    Picker::~Picker()
    {
      glDeleteBuffers(1,&glBuffer_);
      GLERR;
    }

    void Picker::on_gl_context_creation(GLContext* context)
    { 
      GLuint program(0);
      context->create_programs<Picker>(1);
      context->get_programs<Picker>(&program);
      glBindAttribLocation(program,positionLocation_,"position");
      context->compile_program(program,vSource_,"",fSource_);
      context->link_program(program);  
      glUseProgram(program);
      glUniform1i(glGetUniformLocation(program,"sampler"),0);
      spriteLocationMap_[context]=glGetUniformLocation(program,"sprite");
      sizeLocationMap_[context]=glGetUniformLocation(program,"size");
      GLERR;
    }
    
    void Picker::on_gl_context_destruction(GLContext* context)
    {
      context->delete_programs<Picker>();
    }

    void Picker::prepare_attributes()
    {
      glEnableVertexAttribArray(positionLocation_);
      glVertexAttribPointer(positionLocation_,4, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
    }
    
    void Picker::before_batch_draw(GLContext* context)
    {
      GLuint program(0);
      context->get_programs<Picker>(&program);
      glUseProgram(program);
      glEnable(GL_DEPTH_TEST);
      glActiveTexture(GL_TEXTURE0);
      spriteLocation_=spriteLocationMap_[context];
      sizeLocation_=sizeLocationMap_[context];
      GLERR;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    
    void Picker::draw(GLContext* context)
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
#else
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
      prepare_attributes();
#endif   
      glBindTexture(GL_TEXTURE_2D,Marker::texture_id(marker_));
      glUniform1i(spriteLocation_,0);
      glDrawArrays(GL_LINE_STRIP,0,3);
      glPointSize(markerSize_);
      glUniform1f(sizeLocation_,markerSize_);
      glUniform1i(spriteLocation_,1);
      glDrawArrays(GL_POINTS,1,1);
      GLERR;
    }
    
    bool Picker::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(GLFW_PRESS!=action)
	return false;

      if(GLFW_MOUSE_BUTTON_RIGHT==button)
	{
	  if(visible_)
	    visible_=false;
	
	  w->gl_context()->request_redraw();
	  return true;
	}
      
      if(GLFW_MOUSE_BUTTON_LEFT!=button)
	{
	  if(visible_&&(view_&Z_COORDINATE))
	    w->gl_context()->remove_drawable(this);
	  return false;
	}
      
      double time(glfwGetTime());
      double dt(time-lastClickTime_);
      lastClickTime_=time;
      if(dt>doubleClickTime_)
	return false;
      
      const GLfloat* screenCoords(w->cursor_position());
      
      if(!(view_&Z_COORDINATE))
	{
	  GLfloat point[3];
	  point[2]=0;
	  w->viewing_area()->unproject(screenCoords,point);
	  w->viewing_volume()->untransform(point,point);
	  EventSource<PickEvent>::Type::emit(bool(GLFW_MOD_CONTROL&mods),point[0],point[1],point[2]);
	  return true;
	}
      
      visible_=!visible_;
      
      if(!visible_)
	{
	  w->EventSource<MouseMotionEvent>::Type::disconnect(this);
	  EventSource<PickEvent>::Type::emit(bool(GLFW_MOD_CONTROL&mods),points_[4],points_[5],points_[6]);
	  w->gl_context()->remove_drawable(this);
	}
      else
	{
	  w->EventSource<MouseMotionEvent>::Type::connect_before(this);
	  for(size_t i(0);i<N_SCREEN_DIMENSIONS;++i)
	    {
	      points_[i]=points_[8+i]=screenCoords[i];
	      points_[4+i]=(points_[i]+points_[8+i])/2;
	    }
	  points_[2]=1;points_[6]=0;points_[10]=-1;
	  points_[3]=points_[7]=points_[11]=1;
	  w->viewing_area()->unproject(points_,points_);
	  w->viewing_area()->unproject(points_+4,points_+4);
	  w->viewing_area()->unproject(points_+8,points_+8);
	  w->viewing_volume()->untransform(points_,points_);
	  w->viewing_volume()->untransform(points_+4,points_+4);
	  w->viewing_volume()->untransform(points_+8,points_+8);
	  for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	    {
	      v0_[i]=points_[4+i];
	      v_[i]=points_[8+i]-points_[i];
	    }
	  glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
	  glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(points_),static_cast<const GLvoid*>(points_));
	  glBindBuffer(GL_ARRAY_BUFFER,0);
	  w->gl_context()->add_drawable(this);
	}
      GLERR;
      w->gl_context()->request_redraw();
      return true;
    }

    bool Picker::process(MouseMotionEvent event, GLWindow* w,GLfloat x, GLfloat y, GLfloat xOld, GLfloat yOld)
    {
      GLfloat u0[]={x,y,1};
      GLfloat u[]={x,y,-1};
      w->viewing_area()->unproject(u0,u0);
      w->viewing_area()->unproject(u,u);
      w->viewing_volume()->untransform(u0,u0);
      w->viewing_volume()->untransform(u,u);
      
      for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	{
	  u0[i]=(u[i]+u0[i])/2; // u0 contains now the base point
	  u[i]-=u0[i]; // and u a vector pointing along the current line of sight under the mouse pointer
	}
      
      // now find the closest point of the previously selected line of sight (double click)
     
      GLfloat uu(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
      GLfloat uv(u[0]*v_[0]+u[1]*v_[1]+u[2]*v_[2]);
      GLfloat vv(v_[0]*v_[0]+v_[1]*v_[1]+v_[2]*v_[2]);
      GLfloat d0u((v0_[0]-u0[0])*u[0]+(v0_[1]-u0[1])*u[1]+(v0_[2]-u0[2])*u[2]);
      GLfloat d0v((v0_[0]-u0[0])*v_[0]+(v0_[1]-u0[1])*v_[1]+(v0_[2]-u0[2])*v_[2]);

      /* GLfloat a,b are parameters tracing the two lines (a is the relevant one)
	 equations for a and b are:
	 d0v+a*vv-b*uv=0
	 d0u+a*uv-b*uu=0
      */
      GLfloat d(uu-uv*uv/vv);
      if(d<=0&&d>=0)
	return false;
      GLfloat b((d0u-d0v*uv/vv)/d);
      GLfloat a((b*uv-d0v)/vv);
      
      for(size_t i(0);i<N_SPATIAL_DIMENSIONS;++i)
	points_[4+i]=v0_[i]+a*v_[i];
      
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);
      glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(points_),static_cast<const GLvoid*>(points_));
      glBindBuffer(GL_ARRAY_BUFFER,0);
      GLERR;     
      w->gl_context()->request_redraw();
      return false;
    }

#pragma GCC diagnostic pop
    
  } /* end namespace gui */
} /* end namespace scigma */

