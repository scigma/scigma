#include <stdexcept>

#include "screentext.hpp"
#include "glutil.hpp"
#include "glcontext.hpp"

namespace scigma
{
  namespace gui
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

    std::map<GLContext*,GLint> ScreenText::screenAnchorLocationMap_;
    std::map<GLContext*,GLint> ScreenText::offsetLocationMap_;
    std::map<GLContext*,GLint> ScreenText::fColorLocationMap_;
    std::map<GLContext*,GLint> ScreenText::bColorLocationMap_;

#pragma clang diagnostic pop
    
    GLint ScreenText::screenAnchorLocation_(0);
    GLint ScreenText::offsetLocation_(0);
    GLint ScreenText::fColorLocation_(0);
    GLint ScreenText::bColorLocation_(0);



#ifdef SCIGMA_USE_OPENGL_3_2
    // Vertex shader for rendering strings to screen
    const char* ScreenText::vSource_ = 
      "uniform vec4 screenAnchor;"
      "uniform vec2 offset;"
      "in vec2 position;"
      "in vec2 uvValue;"
      "out vec2 UV;"
      "void main() {gl_Position = vec4((position.x+offset.x)*screenToDeviceMatrix[0].x,(position.y+offset.y)*screenToDeviceMatrix[1].y,0,1)+screenAnchor;UV=uvValue;}";
    // Fragment shader for rendering strings to screen
    const char* ScreenText::fSource_ =
      "uniform sampler2D sampler;"
      "uniform vec4 fColor;"
      "uniform vec4 bColor;"
      "in vec2 UV;"
      "out vec4 color;"
      "void main(){"
      "vec4 fColor_=fColor.w<0?foregroundColor:fColor;"
      "vec4 bColor_=bColor.w<0?backgroundColor:bColor;"
      "if(0.0==uniqueID.z){color = texture(sampler, UV).r*fColor_+(1-texture(sampler,UV).r)*bColor_;if(0.0==color.w)discard;}else{color=vec4(uniqueID.xy,gl_FragCoord.z,1);}}";
#else
    // Vertex shader for rendering strings to screen GLSL 1.2
    const char* ScreenText::vSource_ =
      "uniform vec4 screenAnchor;"
      "uniform vec2 offset;"
      "attribute vec2 position;"
      "attribute vec2 uvValue;"
      "varying vec2 UV;"
      "void main() {gl_Position =vec4((position.x+offset.x)*screenToDeviceMatrix[0].x,(position.y+offset.y)*screenToDeviceMatrix[1].y,0,1)+screenAnchor; UV=uvValue;}";
    // Fragment shader for rendering strings to screen GLSL 1.2
    const char* ScreenText::fSource_ = 
      "uniform sampler2D sampler;"
      "uniform vec4 fColor;"
      "uniform vec4 bColor;"
      "varying vec2 UV;"
      "void main(){"
      "vec4 fColor_=fColor.w<0?foregroundColor:fColor;"
      "vec4 bColor_=bColor.w<0?backgroundColor:bColor;"
      "if(0.0==uniqueID.z){vec4 color=texture2D(sampler, UV).r*fColor_+(1-texture2D(sampler,UV).r)*bColor_;if(0.0==color.w)discard;gl_FragColor=color;}else{gl_FragColor=vec4(uniqueID.xy,gl_FragCoord.z,1);}}";
#endif
    
    ScreenText::ScreenText(Font::Type type):font_(Font::font(type)),bufferSize_(0),usedSize_(0)
    {
      glGenBuffers(1,&glBuffer_);
      fColor_[3]=bColor_[3]=-1.0f;
      offset_[0]=offset_[1]=0.0f;
    }
  
    ScreenText::~ScreenText()
    {
      glDeleteBuffers(1,&glBuffer_);
    }
  
    void ScreenText::set_text(const std::vector<std::string>& text,
			      const std::vector<GLfloat>& x,
			      const std::vector<GLfloat>& y,
			      GLfloat xAnchor,
			      GLfloat yAnchor)
    {

      size_t n(text.size());
      if(0==n)
	return;
      
      set_text(&text[0],&x[0],&y[0],xAnchor,yAnchor,n);     
    }

    void ScreenText::set_text(const std::string* text,
		  const GLfloat* x,
		  const GLfloat* y,
		  GLfloat xAnchor,
		  GLfloat yAnchor,
		  size_t n)
    {
      screenAnchor_=x[0]<0?(y[0]<0?SCREEN_ANCHOR_TOP_RIGHT:SCREEN_ANCHOR_BOTTOM_RIGHT):
	(y[0]<0?SCREEN_ANCHOR_TOP_LEFT:SCREEN_ANCHOR_BOTTOM_LEFT);

      GLfloat height(font_.charHeight);
      const GLfloat* charWidth(font_.charWidth);
      
      std::vector<GLfloat> buffer(0);

      for(size_t i(0);i<n;++i)
	{
	  GLfloat width(font_.string_width(text[i]));
	  GLfloat xOffset(-width*(xAnchor+0.5f));
	  GLfloat yOffset(-height*(yAnchor+0.5f));
	  GLfloat xLeft,xRight((x[i]+xOffset));
	  GLfloat yBottom(y[i]+yOffset),yTop(yBottom+height);    
	  
	  for(size_t j(0);j<text[i].length();++j)
	    {
	      xLeft = xRight;
	      xRight = xLeft+charWidth[GLubyte(text[i].at(j))];
	      
	      buffer.push_back(xLeft);buffer.push_back(yTop);
	      buffer.push_back(xLeft);buffer.push_back(yBottom);
	      buffer.push_back(xRight);buffer.push_back(yTop);
	      buffer.push_back(xRight);buffer.push_back(yBottom);
	      buffer.push_back(xRight);buffer.push_back(yTop);
	      buffer.push_back(xLeft);buffer.push_back(yBottom);
	    }
	}
      
      unsigned char charOffset=font_.charOffset;
      const GLfloat* charTextureWidth(font_.charTextureWidth);
      GLfloat charTextureHeight(font_.charTextureHeight);
      GLfloat charsPerRow(font_.charsPerRow);
      GLfloat charsPerColumn(font_.charsPerColumn);
      
      for(unsigned int i=0;i<n;++i)
	{
	  for(unsigned int j=0;j<text[i].length();++j)
	    {
	      unsigned char c = GLubyte(text[i].at(j)-charOffset);
	      GLfloat uLeft = (c%GLubyte(charsPerRow))/charsPerRow;
	      GLfloat uRight = uLeft+charTextureWidth[c+charOffset];
	      GLfloat vTop = 1-(c/GLubyte(charsPerRow))/charsPerColumn;
	      GLfloat vBottom = vTop-charTextureHeight;
	      
	      buffer.push_back(uLeft);buffer.push_back(vTop);
	      buffer.push_back(uLeft);buffer.push_back(vBottom);
	      buffer.push_back(uRight);buffer.push_back(vTop);
	      buffer.push_back(uRight);buffer.push_back(vBottom);
	      buffer.push_back(uRight);buffer.push_back(vTop);
	      buffer.push_back(uLeft);buffer.push_back(vBottom);
	    }
	}

      usedSize_=buffer.size();

      if(!usedSize_)
	return;
      
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
    }
    
    void ScreenText::set_color(GLfloat* fColor, GLfloat* bColor)
    {
      for(int i(0);i<4;++i)
	{
	  fColor_[i]=fColor[i];
	  bColor_[i]=bColor[i];
	}
    }
    
    void ScreenText::on_gl_context_creation(GLContext* glContext)
    {
      glContext->create_programs<ScreenText>(1);
      GLuint program; glContext->get_programs<ScreenText>(&program);
      glBindAttribLocation(program,positionLocation_,"position");
      glBindAttribLocation(program,uvValueLocation_,"uvValue");
      glContext->compile_program(program,vSource_,"",fSource_);
      glContext->link_program(program);
      glUseProgram(program);
      glUniform1i(glGetUniformLocation(program,"sampler"),0);
      screenAnchorLocationMap_[glContext]=glGetUniformLocation(program,"screenAnchor");
      offsetLocationMap_[glContext]=glGetUniformLocation(program,"offset");
      fColorLocationMap_[glContext]=glGetUniformLocation(program,"fColor");
      bColorLocationMap_[glContext]=glGetUniformLocation(program,"bColor");
      GLERR;
    }
    
    void ScreenText::on_gl_context_destruction(GLContext* glContext)
    {
      glContext->delete_programs<ScreenText>();
      GLERR; 
    }

    void ScreenText::set_offset(GLfloat xOffset,GLfloat yOffset)
    {
      offset_[0]=xOffset;
      offset_[1]=yOffset;
    }

#pragma clang diagnostic push    
#pragma clang diagnostic ignored "-Wold-style-cast"    

    void ScreenText::prepare_attributes()
    {
      glEnableVertexAttribArray(positionLocation_);
      glEnableVertexAttribArray(uvValueLocation_);
      glVertexAttribPointer(positionLocation_, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glVertexAttribPointer(uvValueLocation_, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(usedSize_/2*sizeof(GLfloat)));
    }

#pragma clang diagnostic pop    

    void ScreenText::before_batch_draw(GLContext* glContext)
    {
      GLuint program;
      glContext->get_programs<ScreenText>(&program);
      glUseProgram(program);
      glActiveTexture(GL_TEXTURE0);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      screenAnchorLocation_=screenAnchorLocationMap_[glContext];
      offsetLocation_=offsetLocationMap_[glContext];
      fColorLocation_=fColorLocationMap_[glContext];
      bColorLocation_=bColorLocationMap_[glContext];
      GLERR;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    void ScreenText::draw(GLContext* context)
    {
      if(!usedSize_)
	return;

#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
#else
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);   
      prepare_attributes();
#endif    

      glUniform4fv(fColorLocation_,1,fColor_);
      glUniform4fv(bColorLocation_,1,bColor_);
      glUniform2fv(offsetLocation_,1,offset_);

      switch(screenAnchor_)
	{
	case SCREEN_ANCHOR_BOTTOM_LEFT:
	  glUniform4f(screenAnchorLocation_,-1,-1,0,0);break;
	case SCREEN_ANCHOR_BOTTOM_RIGHT:
	  glUniform4f(screenAnchorLocation_,1,-1,0,0);break;
	case SCREEN_ANCHOR_TOP_RIGHT:
	  glUniform4f(screenAnchorLocation_,1,1,0,0);;break;
	case SCREEN_ANCHOR_TOP_LEFT:
	  glUniform4f(screenAnchorLocation_,-1,1,0,0);break;
        default: throw std::runtime_error("unknown screen anchor location");	
      }
     
      glBindTexture(GL_TEXTURE_2D,font_.textureID);
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glDrawArrays(GL_TRIANGLES, 0, GLsizei(usedSize_/4));

      GLERR;
    }

#pragma GCC diagnostic pop

  } /* end namespace gui */
} /* end namespace scigma */
