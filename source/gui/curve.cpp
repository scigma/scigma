#include <iostream>
#include "glutil.h"
#include "curve.h"
#include "glwindow.h"
#include "application.h"

extern "C" int ESCAPE_COUNT;

namespace scigma
{
  namespace gui
  {
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::map<GLContext*,GLint> Curve::spriteLocationMap_;
    std::map<GLContext*,GLint> Curve::sizeLocationMap_;
    std::map<GLContext*,GLint> Curve::colorLocationMap_;
    std::map<GLContext*,GLint> Curve::lighterLocationMap_;
    
#pragma clang diagnostic pop
    
    GLint Curve::spriteLocation_(0);
    GLint Curve::sizeLocation_(0);
    GLint Curve::colorLocation_(0);
    GLint Curve::lighterLocation_(0);
        
    Curve::Curve(GLWindow* glWindow, std::string identifier, size_t nExpectedPoints, Wave* variableWave, Wave* constantWave, Marker::Type marker,
		 Marker::Type point, GLfloat markerSize,GLfloat pointSize, const GLfloat* color, GLfloat delay):
      Graph(glWindow,identifier,nExpectedPoints,variableWave,constantWave,marker,point,markerSize,pointSize,color,delay),
      PythonObject<Curve>(this)
    {}
    
    Curve::~Curve()
    {}
    
    void Curve::on_gl_context_creation(GLContext* glContext)
    {
      /* this is just a dummy program which will be replaced in 
	 rebuild_shader()
      */
      glContext->create_programs<Curve>(1);
      GLuint program; glContext->get_programs<Curve>(&program);
      const char* vertexShader="void main(){gl_Position=rotationMatrix*vec4(1,1,1,1);}";
#ifdef SCIGMA_USE_OPENGL_3_2
      const char* fragmentShader="out vec4 color;void main(){color=vec4(1,1,1,1);}";
#else
      const char* fragmentShader="void main(){gl_FragColor=vec4(1,1,1,1);}";
#endif
      glContext->compile_program(program,vertexShader,"",fragmentShader);
      glContext->link_program(program);
      GLERR; 
    }
    
    void Curve::on_gl_context_destruction(GLContext* glContext)
    {
      glContext->delete_programs<Curve>();
      GLERR; 
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast" 

    void Curve::prepare_attributes()
    {
      /* bind a vertex buffer to the dummy_ attribute at location 0,
	 to make sure that something gets drawn (drawing without vertex buffers is not
	 guaranteed to work, apparently)
      */
      glBindBuffer(GL_ARRAY_BUFFER,dummyBuffer_);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0,1,GL_FLOAT,GL_FALSE,0,0);
   
      size_t nIndices(indices_.size());
      size_t nMyVar(0);
      for(size_t i(0);i<nIndices;++i)
	{
	  GLuint attLoc((GLuint(i+1)));
	  if(indices_[i]>0)//it's a variable
	    {
	      glBindBuffer(GL_ARRAY_BUFFER,variableWave_->gl_buffer());
	      size_t index(size_t(indices_[i]-1));
	      ++nMyVar;
	      glEnableVertexAttribArray(attLoc);
	      glVertexAttribPointer(attLoc, 1,GL_FLOAT, GL_FALSE, GLsizei(variableWave_->columns()*sizeof(GLfloat)),
				    (const GLvoid*)(sizeof(GLfloat)*index));
	    }
	  else if(indices_[i]<0)//it's a constant
	    {
	      glDisableVertexAttribArray(attLoc);
	      uint32_t index(uint32_t(-indices_[i]-1));
	      glVertexAttrib1f(attLoc,GLfloat((*constantWave_)[index]));
	    }
	  else //it's not available
	    {
	      glDisableVertexAttribArray(attLoc);
	      glVertexAttrib1f(attLoc,0.0f);
	    }
	}   
      attributesInvalid_=false;
    }  

#pragma clang diagnostic pop
    
    void Curve::on_addition(GLContext* glContext)
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
      prepare_attributes();
      glBindVertexArray(0);
#endif
      delay_=(delay_>0.0f?delay_:GLfloat(1e-12));
      glContext->continuous_refresh_needed();
      GLERR; 
    }
    
    void Curve::on_removal(GLContext* glContext)
    {
      if(hovering_||hoverPoint_>=0)
	glWindow_->EventSource<MouseButtonEvent>::Type::disconnect(this);
      if(delay_>0.0f)
	{
	  glContext->continuous_refresh_not_needed();
	}
      glContext->request_redraw();
    }

    void Curve::before_batch_draw(GLContext* glContext)
    {
      glContext->get_programs<Curve>(programs_);
      glUseProgram(programs_[0]);GLERR; 
      colorLocation_=colorLocationMap_[glContext];
      spriteLocation_=spriteLocationMap_[glContext];
      sizeLocation_=sizeLocationMap_[glContext];
      lighterLocation_=lighterLocationMap_[glContext];
      
      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_DEPTH_TEST); 
      glDisable(GL_BLEND); 
      GLERR; 
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    
    void Curve::draw(GLContext* glContext)
    {
      if(timeOfFirstDraw_<0)
	timeOfFirstDraw_=GLfloat(glfwGetTime());
      uint32_t availablePoints;
      if(!variableWave_)
	availablePoints=1;
      else
	availablePoints=variableWave_->buffer_rows();
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
      if(attributesInvalid_)
        prepare_attributes();
#else
      prepare_attributes();
#endif
      //      glDisableVertexAttribArray(GLuint(glGetAttribLocation(programs_[0],"r")));
      //glVertexAttrib1f(GLuint(glGetAttribLocation(programs_[0],"r")),28);

      if(-1!=colorLocation_)
	glUniform4fv(colorLocation_,1,color_);
      
      uint32_t drawnPoints(availablePoints);
      
      glUniform1i(spriteLocation_,1);
      if(hovering_&&!pointHoverIsActive_)
	glUniform1i(lighterLocation_,1);
      else
	glUniform1i(lighterLocation_,0);

      //deal with delay
      if(delay_>0.0f)
	{
	  GLfloat elapsedTime(GLfloat(glfwGetTime())-timeOfFirstDraw_);
	  drawnPoints=uint32_t(elapsedTime/delay_);
	  if(drawnPoints>availablePoints)
	    {
	      drawnPoints=availablePoints;
	      // check if data is complete
	      if(availablePoints==nPoints_)
		{
		  delay_=0.0f;
		  escapeCount_=ESCAPE_COUNT;
		  glContext->continuous_refresh_not_needed();
		}
	    }
	  if(Marker::NONE!=marker_&&drawnPoints!=availablePoints)
	    {
	      
	      glBindTexture(GL_TEXTURE_2D,Marker::texture_id(marker_));
	      glPointSize(markerSize_);
	      glUniform1f(sizeLocation_,markerSize_);
	      if(drawnPoints>0)
		glDrawArrays(GL_POINTS, GLsizei(drawnPoints-1),1);   
	    }

	  if(escapeCount_!=ESCAPE_COUNT)
	    {
	      delay_=0.0f;
	      nPoints_=availablePoints;
	      drawnPoints=availablePoints;
	      glContext->continuous_refresh_not_needed();
	    }
	}

      if(Marker::NONE!=point_)
	{
	  GLfloat factor(1.0f);
	  if(1==nPoints_&&!pointHoverIsActive_&&hovering_)
	    factor=1.2f;
	  glBindTexture(GL_TEXTURE_2D,Marker::texture_id(point_));
	  glPointSize(pointSize_*factor);
	  glUniform1f(sizeLocation_,pointSize_*factor);
	  glDrawArrays(GL_POINTS, 0,GLsizei(drawnPoints));
	}
      else
	{
	  glUniform1i(spriteLocation_,0);
	  glDrawArrays(GL_LINE_STRIP, 0, GLsizei(drawnPoints));
	}

      if(hoverPoint_>=0)
	{
	  glBindTexture(GL_TEXTURE_2D,Marker::cross_hair_texture_id());
	  glUniform1i(spriteLocation_,2);
	  glPointSize(Marker::cross_hair_size());
	  glUniform1f(sizeLocation_,Marker::cross_hair_size());
	  glDrawArrays(GL_POINTS, GLsizei(hoverPoint_), 1);
	}
    }
    
    bool Curve::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(GLFW_MOUSE_BUTTON_LEFT==button&&GLFW_PRESS==action)
	{
	  double time(glfwGetTime());
	  double dt(time-lastClickTime_);
	  lastClickTime_=time;
	  if(dt>doubleClickTime_)
	    return true;
	  EventSource<CurveClickEvent>::Type::emit(identifier_.c_str());
	  return true;
	}
      return false;
    }
    
    void Curve::rebuild_shader(GLContext* glContext, 
			       const std::string& vertexShaderHeader,
			       const std::string& fragmentShaderHeader,
			       const ExpressionArray& transformations,
			       const ExpressionArray& attributes,
			       bool useColorMap)
    {
      glContext->delete_programs<Curve>();
      glContext->create_programs<Curve>(1);
      glContext->get_programs<Curve>(programs_);
      
      /* this dummy attribute is necessary because we always need to enable 
	 vertex attribute array 0, at least on some implementations
      */
      glBindAttribLocation(programs_[0],0,"dummy_");
      for(size_t i(0),size(attributes.size());i<size;++i)
	glBindAttribLocation(programs_[0],GLuint(i+1),attributes[i].c_str());

      std::string vertexShaderMain("void main()\n{\n");
      vertexShaderMain+="\tvec4 xyz1_=vec4("+transformations[X_INDEX]+", "+
	transformations[Y_INDEX]+", "+
	transformations[Z_INDEX]+",1.0);\n";
      if(useColorMap)
	{
	  vertexShaderMain+="\tfloat p_="+transformations[C_INDEX]+";\n";
	  vertexShaderMain+="\trgba_=colormap_(p_);\n";
	}
      vertexShaderMain+="\tt_="+transformations[T_INDEX]+";\n";      
      vertexShaderMain+="\tvec4 devicePos_=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*xyz1_;\n";
      vertexShaderMain+="\tgl_Position=devicePos_;";
      vertexShaderMain+="\tdevicePos_=devicePos_/devicePos_.w;\n";
      vertexShaderMain+="\tscreenPos_.x=dummy_;\n";
      vertexShaderMain+="\tscreenPos_=vec2(devicePos_.x*deviceToScreenMatrix[0].x+deviceToScreenMatrix[3].x,devicePos_.y*deviceToScreenMatrix[1].y+deviceToScreenMatrix[3].y);\n";
      vertexShaderMain+="\tvertexID_=vec4(float(mod(gl_VertexID/(256*256),256))/256.0,float(mod(gl_VertexID/256,256))/256.0,float(mod(gl_VertexID,256)+0.5)/256.0,1.0);\n";
      vertexShaderMain+="}\n";

      std::string fragmentShaderMain("\tif(t_<0||t_>1)\n"
				     "\t\tdiscard;\n"
				     "\tvec4 rgba2_=rgba_;\n"
				     "\tif(sprite_==2)\n"
				     "\t{\n"
				     "\t\tif(1.5<uniqueID.z)\n"
				     "\t\t\tdiscard;\n"
				     "\t\telse\n"
				     "\t\t\trgba2_=foregroundColor;\n"
				     "\t}\n"
				     "\tif(sprite_!=0)\n"
				     "\t{\n"
				     "\t\tif(size_>2)\n"
				     "\t\t{\n"
				     "\t\t\tvec2 pointCoord_=(screenPos_-gl_FragCoord.xy)/size_+vec2(0.5,0.5);\n"
#ifdef SCIGMA_USE_OPENGL_3_2
				     "\t\t\trgba2_=texture(sampler_,pointCoord_).r*rgba2_;\n"
#else
				     "\t\t\trgba2_=texture2D(sampler_,pointCoord_).r*rgba2_;\n"
#endif
				     "\t\t\tif(rgba2_.a==0)discard;\n"
				     "\t\t}\n"
				     "\t}\n"
				     "\tif(lighter_==1)\n"
				     "\t\trgba2_=rgba2_+(1-rgba2_)*0.5;\n");
      std::string colorExpression("rgba2_");
      std::string hoverExpression("vertexID_");
#ifdef SCIGMA_USE_OPENGL_3_2
      fragmentShaderMain=GLContext::create_fragment_main(fragmentShaderMain,colorExpression,hoverExpression,"color_");
#else
      fragmentShaderMain=GLContext::create_fragment_main(fragmentShaderMain,colorExpression,hoverExpression,"gl_FragColor");
#endif
      
      std::string vertexShader(vertexShaderHeader+
			       (useColorMap?colorMapFunction_:"")+
			       vertexShaderMain);
      std::string fragmentShader(fragmentShaderHeader+
				 fragmentShaderMain);
      
      /*    std::cout<<"Vertex Shader:"<<std::endl;
	    std::cout<<"--------------------------------------------------"<<std::endl;
	    std::cout<<vertexShader;
	    std::cout<<std::endl<<std::endl;
	    std::cout<<"Fragment Shader:"<<std::endl;
	    std::cout<<"--------------------------------------------------"<<std::endl;
	    std::cout<<fragmentShader;
	    std::cout<<std::endl<<std::endl<<std::endl;
      */
      
      glContext->compile_program(programs_[0],vertexShader,"",fragmentShader);
      glContext->link_program(programs_[0]);
      
      if(!useColorMap)
	colorLocationMap_[glContext]=glGetUniformLocation(programs_[0],"rgba_");
      else
	colorLocationMap_[glContext]=-1;

      GLERR; 
      
      spriteLocationMap_[glContext]=glGetUniformLocation(programs_[0],"sprite_");
      sizeLocationMap_[glContext]=glGetUniformLocation(programs_[0],"size_");
      lighterLocationMap_[glContext]=glGetUniformLocation(programs_[0],"lighter_");
      
      glUseProgram(programs_[0]);
      glUniform1i(glGetUniformLocation(programs_[0],"sampler_"),0);
      
      GLERR; 
    }
    
    void Curve::on_hover_begin(GLContext* glContext)
    {
      if(Marker::RCROSS==point_)
	return;
      glWindow_->EventSource<MouseButtonEvent>::Type::connect_before(this);
      if(!pointHoverIsActive_)
	hovering_=true;
      glContext->request_redraw();
    }
    
    void Curve::on_hover(GLContext* glContext,GLuint value)
    {
      if(Marker::RCROSS==point_)
	return;
      if(!pointHoverIsActive_)
	  return;
      hoverPoint_=value;
      glContext->request_redraw();
    }

    void Curve::on_hover_end(GLContext* glContext)
    {
      if(Marker::RCROSS==point_)
	return;
      glWindow_->EventSource<MouseButtonEvent>::Type::disconnect(this);
      hovering_=false;
      hoverPoint_=-1;
      glContext->request_redraw();
    }
    
#pragma clang diagnostic pop
#pragma GCC diagnostic pop;
    
  } /* end namespace gui */
} /* end namespace scigma */
