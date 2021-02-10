#include <string>
#include <iostream>
#include <AntTweakBar.h>
#include "glcontext.hpp"
#include "glutil.hpp"

using scigma::common::connect;

namespace scigma
{
  namespace gui
  {

    const GLuint GLContext::GLOBAL_UNIFORM_OFFSETS[]={
      0,
      1*N_4X4_ELEMENTS*sizeof(GLfloat),
      2*N_4X4_ELEMENTS*sizeof(GLfloat),
      3*N_4X4_ELEMENTS*sizeof(GLfloat),
      4*N_4X4_ELEMENTS*sizeof(GLfloat),
      5*N_4X4_ELEMENTS*sizeof(GLfloat),
      6*N_4X4_ELEMENTS*sizeof(GLfloat),
      7*N_4X4_ELEMENTS*sizeof(GLfloat),
      (7*N_4X4_ELEMENTS+N_ROWS)*sizeof(GLfloat),
      (7*N_4X4_ELEMENTS+2*N_ROWS)*sizeof(GLfloat)};
    
#ifdef SCIGMA_USE_OPENGL_3_2
    const char* GLContext::VERTEX_SHADER_HEADER=
      "#version 150 core\n"
      "layout(std140) uniform globalUniforms\n"
      "{\n"
      "\tmat4 rotationMatrix;\n"
      "\tmat4 scalingMatrix;\n"
      "\tmat4 translationMatrix;\n"
      "\tmat4 projectionMatrix;\n"
      "\tmat4 inverseProjectionMatrix;\n"
      "\tmat4 screenToDeviceMatrix;\n"
      "\tmat4 deviceToScreenMatrix;\n"
      "\tvec4 backgroundColor;\n"
      "\tvec4 foregroundColor;\n"
      "\tvec4 ctMinMax;\n"
      "};\n";
    const char* GLContext::GEOMETRY_SHADER_HEADER=
      "#version 150 core\n"
      "layout(std140) uniform globalUniforms\n"
      "{\n"
      "\tmat4 rotationMatrix;\n"
      "\tmat4 scalingMatrix;\n"
      "\tmat4 translationMatrix;\n"
      "\tmat4 projectionMatrix;\n"
      "\tmat4 inverseProjectionMatrix;\n"
      "\tmat4 screenToDeviceMatrix;\n"
      "\tmat4 deviceToScreenMatrix;\n"
      "\tvec4 backgroundColor;\n"
      "\tvec4 foregroundColor;\n"
      "\tvec4 ctMinMax;\n"
      "};\n";
    const char* GLContext::FRAGMENT_SHADER_HEADER=
      "#version 150 core\n"
      "layout(std140) uniform globalUniforms\n"
      "{\n"
      "\tmat4 rotationMatrix;\n"
      "\tmat4 scalingMatrix;\n"
      "\tmat4 translationMatrix;\n"
      "\tmat4 projectionMatrix;\n"
      "\tmat4 inverseProjectionMatrix;\n"
      "\tmat4 screenToDeviceMatrix;\n"
      "\tmat4 deviceToScreenMatrix;\n"
      "\tvec4 backgroundColor;\n"
      "\tvec4 foregroundColor;\n"
      "\tvec4 ctMinMax;\n"
      "};\n"
      "uniform vec3 uniqueID;\n";
#else
    const char* GLContext::VERTEX_SHADER_HEADER=
      "#version 120\n"
      "#extension GL_EXT_gpu_shader4 : enable\n"
      "uniform mat4 rotationMatrix;\n"
      "uniform mat4 scalingMatrix;\n"
      "uniform mat4 translationMatrix;\n"
      "uniform mat4 projectionMatrix;\n"
      "uniform mat4 inverseProjectionMatrix;\n"
      "uniform mat4 screenToDeviceMatrix;\n"
      "uniform mat4 deviceToScreenMatrix;\n"
      "uniform vec4 backgroundColor;\n"
      "uniform vec4 foregroundColor;\n"
      "uniform vec4 ctMinMax;\n";
    const char* GLContext::GEOMETRY_SHADER_HEADER=NULL;
    const char* GLContext::FRAGMENT_SHADER_HEADER=
      "#version 120\n"
      "#extension GL_ARB_texture_rectangle : enable\n"
      "uniform mat4 rotationMatrix;\n"
      "uniform mat4 scalingMatrix;\n"
      "uniform mat4 translationMatrix;\n"
      "uniform mat4 projectionMatrix;\n"
      "uniform mat4 inverseProjectionMatrix;\n"
      "uniform mat4 screenToDeviceMatrix;\n"
      "uniform mat4 deviceToScreenMatrix;\n"
      "uniform vec4 backgroundColor;\n"
      "uniform vec4 foregroundColor;\n"
      "uniform vec4 ctMinMax;\n";
      "uniform vec3 uniqueID;\n";
#endif
   
    GLContext::GLContext(GLFWwindow* ptr):PythonObject<GLContext>(this),
      currentFrameStartTime_(0.0),
      currentFrameRenderingTime_(0.0),lastHoverTime_(0.0),
      glfwWindowPointer_(ptr),
      stalled_(0),
      hoverIndex_(0xFFFFFFFF),
      redrawRequested_(false),
      colorPicking_(false)
    {
      glfwMakeContextCurrent(glfwWindowPointer_);
      glfwSwapInterval(2);
      // depth values are used to decide if mouse is hovering over an object
      // 1.0 means: no object
      glClearDepth(1.0);
      
      // this is important for the call to glReadPixels() in check_for_hover
      glPixelStorei(GL_PACK_ALIGNMENT,1); 
      colorPicking_=false;
      
#ifdef SCIGMA_USE_OPENGL_3_2
      glGenBuffers(1,&globalUniformBuffer_);
      glBindBuffer(GL_UNIFORM_BUFFER, globalUniformBuffer_);
      glBufferData(GL_UNIFORM_BUFFER,GLOBAL_UNIFORM_BUFFER_SIZE, NULL, GL_STATIC_DRAW);
      glBindBufferRange(GL_UNIFORM_BUFFER,GLOBAL_UNIFORM_BINDING_INDEX,globalUniformBuffer_,0,GLOBAL_UNIFORM_BUFFER_SIZE);
      glBindBuffer(GL_UNIFORM_BUFFER,0);
#else
      GLfloat sizes[2];
      glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, sizes);
      glPointParameterf(GL_POINT_SIZE_MIN, sizes[0]);
      glPointParameterf(GL_POINT_SIZE_MAX, sizes[1]);
#endif
      creation_notify<DrawableTypes>();
    }

    void GLContext::destroy()
    {
      destruction_notify<DrawableTypes>();
#ifdef SCIGMA_USE_OPENGL_3_2
      glfwMakeContextCurrent(glfwWindowPointer_);
      glDeleteBuffers(1,&globalUniformBuffer_);
#endif
    }

    GLContext::~GLContext()
    {}
    
    double GLContext::get_current_frame_start_time()
    {
      return currentFrameStartTime_;
    }

    
    void GLContext::check_for_hover(GLfloat x, GLfloat y)
    {
      double now(glfwGetTime());
      if(now-lastHoverTime_<TARGET_FRAME_TIME)
	return;
      else
	lastHoverTime_=now;
	
      /* In color picking mode, only a 5x5 pixel rectangle is drawn.
	 The nearest object in this rectangle (excluding the corner
	 pixels) is detected, and its on_hover_begin()/on_hover_end
	 methods are called if necessary. 
      */
 
      /* lower left corner of the rectangle, values found by trial and error
	 although it's a bit fishy that we do not need to shift y at all 
      */
      GLint xx(GLint(x-3)),yy((GLint(y)));

      // check if we are inside the window
      if(x-3<0||x+2>width_||y<0||y+5>height_)
	return;
           

      // make sure only a rectangle of 5x5 Pixels around the mouse pointer is drawn
      glScissor(xx,yy,5,5);
      glEnable(GL_SCISSOR_TEST);
      
      // turn off anti-aliasing for color picking (only one color per object!) 
      glDisable(GL_MULTISAMPLE);

      // draw scene in color picking mode
      GLfloat cc[4];
      glGetFloatv(GL_COLOR_CLEAR_VALUE,cc);
      glClearColor(0,0,1,1); // blue bit is set to 1.0 (depth readout)
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      colorPicking_=true;
      draw<DrawableTypes>(0);
#ifdef SCIGMA_OPENGL_3_2
      glBindVertexArray(0);
#endif
      colorPicking_=false;
     
      /* read object information from the color buffer:
	 The red and green byte store the object index (up to 0xFFFF/65535;
	 0xFFFFFFFF is used as "no object" value). The blue byte stores the
	 depth information (in theory, this could be taken directly from 
	 the depth buffer,allowing to use the blue byte for 2^8 times more
	 object indices - this did not work reliably in tests, however).
       */
      GLubyte color[75];
      glReadBuffer(GL_BACK_LEFT);
      glReadPixels(xx,yy,5,5,GL_RGB,GL_UNSIGNED_BYTE,color);

      //find pixel with minimum depth in the rectangle
      size_t i(1);
      size_t minIndex(1);
      GLubyte minDepth(color[i*3+2]);

      //DEBUG OUTPUT
      /*for(int l=0;l<5;++l)
	{
	  for(int n=0;n<5;++n)
	    {
	      std::cerr<<size_t(color[(l*5+n)*3])<<",";
	    }
	  std::cerr<<std::endl;
	}

	for(int l=0;l<5;++l)
	{
	  for(int n=0;n<5;++n)
	    {
	      std::cerr<<size_t(color[(l*5+n)*3+2])<<",";
	    }
	  std::cerr<<std::endl;
	  }*/

      //return;
      

      while(i<4){if(color[(++i)*3+2]<minDepth){minDepth=color[i*3+2];minIndex=i;}}
      ++i;
      while(i<20){if(color[(++i)*3+2]<minDepth){minDepth=color[i*3+2];minIndex=i;}}
      ++i;
      while(i<24){if(color[(++i)*3+2]<minDepth){minDepth=color[i*3+2];minIndex=i;}}

      // mouse is not hovering over an object
      if(0xFF==minDepth)
	{
	  // but recently has been
	  if(hoverIndex_!=0xFFFFFFFF)
	    {
	      end_hover<DrawableTypes>(0);
	    }
	  hoverIndex_=0xFFFFFFFF;
	}
      else // mouse is hovering over an object
	{
	  size_t newHoverIndex(size_t(color[minIndex*3])+size_t(color[minIndex*3+1])*0x100);
	  if(newHoverIndex!=hoverIndex_)
	    {
	      end_hover<DrawableTypes>(0);
	      hoverIndex_=newHoverIndex;
	      begin_hover<DrawableTypes>(0);
	    }
	  glClearColor(1,1,1,1); // any value other than 1,1,1 will be taken as the output of the second hover pass
	  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	  hover<DrawableTypes>(0,xx,yy);
	}

      // restore GL state
      glClearColor(cc[0],cc[1],cc[2],cc[3]);
      glEnable(GL_MULTISAMPLE);
      glDisable(GL_SCISSOR_TEST);
    }
    
    bool GLContext::find_program(GLuint program, std::vector<GLint>::const_iterator* position) const
    {
      for(int i=0;i<N_DRAWABLE_TYPES;++i)
	{
	  for(std::vector<GLint>::const_iterator iter(drawableProgramData[i].begin()),
		end(drawableProgramData[i].end());iter!=end;iter+=PROGRAM_DATA_STRIDE)
	    {
	      if(static_cast<GLuint>(*iter)==program)
		{
		  if(position)
		    *position=iter;
		  return true;
		}
	    }
	}
      return false;
    }
    
    bool GLContext::find_program(GLuint program, std::vector<GLint>::iterator* position)
    {
      for(int i=0;i<N_DRAWABLE_TYPES;++i)
	{
	  for(std::vector<GLint>::iterator iter=drawableProgramData[i].begin(),
		end=drawableProgramData[i].end();iter!=end;iter+=PROGRAM_DATA_STRIDE)
	    {
	      if(static_cast<GLuint>(*iter)==program)
		{
		  if(position)
		    *position=iter;
		  return true;
		}
	    }
	}
      return false;
    }
    
    void GLContext::compile_program(GLuint program,
				    const std::string& vertexShaderBody,
				    const std::string& geometryShaderBody, 
				    const std::string& fragmentShaderBody)
    {
      if(!find_program(program))
	{
	  Application::get_instance()->push_error("program object was not created before",__FILE__,__LINE__);
	  return;
	};
      
      const char* vSource[]={VERTEX_SHADER_HEADER,vertexShaderBody.c_str()};
      const char* gSource[]={GEOMETRY_SHADER_HEADER,geometryShaderBody.c_str()};
      const char* fSource[]={FRAGMENT_SHADER_HEADER,fragmentShaderBody.c_str()};

      GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
      GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      
      glShaderSource(vertexShader, 2, vSource, NULL);
      glCompileShader(vertexShader);
      push_shader_compile_error(vertexShader,__FILE__,__LINE__);
      glAttachShader(program, vertexShader);
      glShaderSource(fragmentShader, 2, fSource, NULL);
      glCompileShader(fragmentShader);
      push_shader_compile_error(fragmentShader,__FILE__,__LINE__);
      glAttachShader(program,fragmentShader);
#ifdef SCIGMA_USE_OPENGL_3_2
      if(geometryShaderBody!="")
	{
	  GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	  glShaderSource(geometryShader, 2, gSource, NULL);
	  glCompileShader(geometryShader);
	  push_shader_compile_error(geometryShader,__FILE__,__LINE__);
	  glAttachShader(program, geometryShader);
	}
#else
      gSource[0]=gSource[1]; // do something with gSource to silence -Wunused-parameter
#endif
    } 
    
    void GLContext::link_program(GLuint program)
    {
      std::vector<GLint>::iterator position;
      if(!find_program(program, &position))
	{
	  Application::get_instance()->push_error("program object was not created before",__FILE__,__LINE__);
	  return;
	};
      
      glLinkProgram(program);
      push_program_link_error(program,__FILE__,__LINE__);
      
      GLuint attachedShaders[4]; GLsizei n;
      glGetAttachedShaders(program,4,&n,attachedShaders);
      for(GLsizei i = 0; i < n ; ++i)
	{
	  glDetachShader(program,attachedShaders[i]);
	  glDeleteShader(attachedShaders[i]);
	}
      GLint uniqueIDLocation=glGetUniformLocation(program,"uniqueID");
      *(position+1)=uniqueIDLocation;
#ifdef SCIGMA_USE_OPENGL_3_2
      GLuint uniformBlockIndex=glGetUniformBlockIndex(program,"globalUniforms");
      glUniformBlockBinding(program,uniformBlockIndex,GLOBAL_UNIFORM_BINDING_INDEX);
#else
      GLint uniformLocations[]={glGetUniformLocation(program,"rotationMatrix"),
				glGetUniformLocation(program,"scalingMatrix"),
				glGetUniformLocation(program,"translationMatrix"),
				glGetUniformLocation(program,"projectionMatrix"),
				glGetUniformLocation(program,"inverseProjectionMatrix"),
				glGetUniformLocation(program,"screenToDeviceMatrix"),
				glGetUniformLocation(program,"deviceToScreenMatrix"),
				glGetUniformLocation(program,"backgroundColor"),
				glGetUniformLocation(program,"foregroundColor"),
				glGetUniformLocation(program,"ctMinMax"};
      int count(0);  
      for(std::vector<GLint>::iterator i=position+2,end=position+PROGRAM_DATA_STRIDE;i!=end;++i)
	*i=uniformLocations[count++];
      glUseProgram(program);
      for(size_t i =0;i<7;++i)
	glUniformMatrix4fv(uniformLocations[i],1,GL_FALSE,&globalUniformMemory[GLOBAL_UNIFORM_OFFSETS[i]]);
      for(size_t i =7;i<9;++i)
	glUniform4fv(uniformLocations[i],1,&globalUniformMemory[GLOBAL_UNIFORM_OFFSETS[i]]);
#endif
    }
    
#ifndef SCIGMA_USE_OPENGL_3_2
    void GLContext::update_4_uniform_in_all_programs(GlobalUniform uniform, const GLfloat* vector)
    {
      for(int i=0;i<N_DRAWABLE_TYPES;++i)
	{
	  for(std::vector<GLint>::const_iterator iter=drawableProgramData[i].begin(),
		end=drawableProgramData[i].end();iter!=end;iter+=PROGRAM_DATA_STRIDE)
	    {
	      glUseProgram(static_cast<GLuint>(*iter));
	      glUniform4fv(*(iter+uniform+2),1,vector);
	    }
	}
      GLERR;
    }

    void GLContext::update_2x2_uniform_in_all_programs(GlobalUniform uniform, const GLfloat* matrix)
    {
      for(int i=0;i<N_DRAWABLE_TYPES;++i)
	{
	  for(std::vector<GLint>::const_iterator iter=drawableProgramData[i].begin(),
		end=drawableProgramData[i].end();iter!=end;iter+=PROGRAM_DATA_STRIDE)
	    {
	      glUseProgram(static_cast<GLuint>(*iter));
	      glUniformMatrix2fv(*(iter+uniform+2),1,GL_FALSE,matrix);
	    }
	}
      GLERR;
    }
    
    void GLContext::update_4x4_uniform_in_all_programs(GlobalUniform uniform, const GLfloat* matrix)
    {
      for(int i=0;i<N_DRAWABLE_TYPES;++i)
	{
	  for(std::vector<GLint>::const_iterator iter=drawableProgramData[i].begin(),
		end=drawableProgramData[i].end();iter!=end;iter+=PROGRAM_DATA_STRIDE)
	    {
	      glUseProgram(static_cast<GLuint>(*iter));
	      glUniformMatrix4fv(*(iter+uniform+2),1,GL_FALSE,matrix);
	    }
	}
      GLERR;
    }
#endif
    
    void GLContext::update_global_uniform_4(GlobalUniform uniform, const GLfloat* vector)
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindBuffer(GL_UNIFORM_BUFFER, globalUniformBuffer_);
      glBufferSubData(GL_UNIFORM_BUFFER,GLOBAL_UNIFORM_OFFSETS[uniform], N_ROWS*sizeof(GLfloat), static_cast<const GLvoid*>(vector));
      glBindBuffer(GL_UNIFORM_BUFFER,0);
#else
      GLfloat* ptr=&globalUniformMemory[GLOBAL_UNIFORM_OFFSETS[uniform]];
      for(size_t i=0;i<N_ROWS;++i)
	ptr[i]=vector[i];
      update_4_uniform_in_all_programs(uniform,vector);
#endif
      if(uniform==BACKGROUND_COLOR)
	glClearColor(vector[0],vector[1],vector[2],vector[3]);

      GLERR;
    }

    void GLContext::update_global_uniform_2x2(GlobalUniform uniform, const GLfloat* matrix)
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindBuffer(GL_UNIFORM_BUFFER, globalUniformBuffer_);
      glBufferSubData(GL_UNIFORM_BUFFER,GLOBAL_UNIFORM_OFFSETS[uniform], N_2X2_ELEMENTS*sizeof(GLfloat), static_cast<const GLvoid*>(matrix));
      glBindBuffer(GL_UNIFORM_BUFFER,0);
#else
      GLfloat* ptr=&globalUniformMemory[GLOBAL_UNIFORM_OFFSETS[uniform]];
      for(size_t i=0;i<N_2X2_ELEMENTS;++i)
	ptr[i]=matrix[i];
      update_2x2_uniform_in_all_programs(uniform,matrix);
#endif
      GLERR;
    }

    void GLContext::update_global_uniform_4x4(GlobalUniform uniform, const GLfloat* matrix)
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindBuffer(GL_UNIFORM_BUFFER, globalUniformBuffer_);
      glBufferSubData(GL_UNIFORM_BUFFER,GLOBAL_UNIFORM_OFFSETS[uniform], N_4X4_ELEMENTS*sizeof(GLfloat), static_cast<const GLvoid*>(matrix));
      glBindBuffer(GL_UNIFORM_BUFFER,0);
#else
      GLfloat* ptr=&globalUniformMemory[GLOBAL_UNIFORM_OFFSETS[uniform]];
      for(size_t i=0;i<N_4X4_ELEMENTS;++i)
	ptr[i]=matrix[i];
      update_4x4_uniform_in_all_programs(uniform,matrix);
#endif
      GLERR;
    }
    
    void GLContext::set_viewport(GLint left, GLint bottom, GLint width, GLint height)
    {
      glfwMakeContextCurrent(glfwWindowPointer_);
      width_=width;height_=height;
      glViewport(left,bottom,width,height);
      request_redraw();
    }
    
    void GLContext::draw_frame()
    {
      glfwMakeContextCurrent(glfwWindowPointer_);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      
      draw<DrawableTypes>(0);
      
#ifdef SCIGMA_OPENGL_3_2
      glBindVertexArray(0);
#else
      glUseProgram(0);	  
#endif
      TwSetCurrentWindow(size_t(this));
      TwDraw();
      GLERR;

    }

    void GLContext::request_redraw()
    {
      if(stalled_)
	{
	  redrawRequested_=true;
	}
      else
	{ 
	  double now(glfwGetTime());
	  if(now-currentFrameStartTime_<TARGET_FRAME_TIME-currentFrameRenderingTime_)
	    {
	      redrawRequested_=true;
	      return;
	    }
	  currentFrameStartTime_=now;

	  draw_frame();
	  glfwSwapBuffers(glfwWindowPointer_);

	  currentFrameRenderingTime_=glfwGetTime()-currentFrameStartTime_;
	  redrawRequested_=false;
	}
    }
    
    void GLContext::stall()
    {
      ++stalled_;
    }
    
    void GLContext::flush()
    {
      --stalled_;
      if(!stalled_&&redrawRequested_)
	request_redraw();
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    template<> void GLContext::creation_notify<LOKI_TYPELIST_0>()
    {}

    template<> void GLContext::destruction_notify<LOKI_TYPELIST_0>()
    {}
      
    template<> void GLContext::draw<LOKI_TYPELIST_0>(size_t objectIndexBase)
    {}

    template<> void GLContext::begin_hover<LOKI_TYPELIST_0>(size_t hoverIndexBase)
    {}

    template<> void GLContext::hover<LOKI_TYPELIST_0>(size_t hoverIndexBase, GLint x, GLint y)
    {}

    template<> void GLContext::end_hover<LOKI_TYPELIST_0>(size_t hoverIndexBase)
    {}

#pragma GCC diagnostic pop

  } /* end namespace gui */
} /* end namespace scigma */
