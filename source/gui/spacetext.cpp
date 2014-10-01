#include "spacetext.h"
#include "glutil.h"
#include "glcontext.h"
#include <iostream>

namespace scigma
{
  namespace gui
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

    std::map<GLContext*,GLint> SpaceText::fColorLocationMap_;
    std::map<GLContext*,GLint> SpaceText::bColorLocationMap_;
    std::map<GLContext*,GLint> SpaceText::globalOffsetLocationMap_;
    std::map<GLContext*,GLint> SpaceText::heightLocationMap_;

#pragma clang diagnostic pop
    
    GLint SpaceText::fColorLocation_(0);
    GLint SpaceText::bColorLocation_(0);
    GLint SpaceText::globalOffsetLocation_(0);
    GLint SpaceText::heightLocation_(0);



#ifdef SCIGMA_USE_OPENGL_3_2
    // Vertex shader for rendering strings to screen
    const char* SpaceText::vSource_ = 
      "uniform vec2 globalOffset;"
      "uniform float height;"
      "in vec4 position;"
      "in vec2 localOffset;"
      "in vec2 uvValue;"
      "in float width;"
      "out vec2 UV;"
      "void main() {"
      "vec2 offset=globalOffset+localOffset;"
      "if(globalOffset.x!=0||globalOffset.y!=0)"
      "{float angle=atan(globalOffset.y,globalOffset.x);offset+=vec2(cos(angle)*width/2,sin(angle)*height/2);}"
      "vec4 devicePos=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*position;"
      "gl_Position =vec4(offset.x*screenToDeviceMatrix[0].x*devicePos.w+devicePos.x,offset.y*screenToDeviceMatrix[1].y*devicePos.w+devicePos.y,devicePos.z,devicePos.w); UV=uvValue;"
      "}";
    // Fragment shader for rendering strings to screen
    const char* SpaceText::fSource_ =
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
    const char* SpaceText::vSource_ =
      "uniform vec2 globalOffset;"
      "uniform float height;"
      "attribute vec4 position;"
      "attribute vec2 localOffset;"
      "attribute vec2 uvValue;"
      "attribute float width;"
      "varying vec2 UV;"
      "void main() {"
      "vec2 offset=globalOffset+localOffset;"
      "if(globalOffset.x!=0||globalOffset.y!=0)"
      "{float angle=atan(globalOffset.y,globalOffset.x);offset+=vec2(cos(angle)*width/2,sin(angle)*height/2);}"
      "vec4 devicePos=projectionMatrix*rotationMatrix*scalingMatrix*translationMatrix*position;"
      /*"devicePos.xy=sign(devicePost.xy)*floor(abs(offset/2.0)+0.5);"*/
      "gl_Position =vec4(offset.x*screenToDeviceMatrix[0].x*devicePos.w+devicePos.x,offset.y*screenToDeviceMatrix[1].y*devicePos.w+devicePos.y,devicePos.z,devicePos.w); UV=uvValue;"
      "}";
    // Fragment shader for rendering strings to screen GLSL 1.2
    const char* SpaceText::fSource_ = 
      "uniform sampler2D sampler;"
      "uniform vec4 fColor;"
      "uniform vec4 bColor;"
      "varying vec2 UV;"
      "void main(){"
      "vec4 fColor_=fColor.w<0?foregroundColor:fColor;"
      "vec4 bColor_=bColor.w<0?backgroundColor:bColor;"
      "if(0.0==uniqueID.z){vec4 color=texture2D(sampler, UV).r*fColor_+(1-texture2D(sampler,UV).r)*bColor_;if(0.0==color.w)discard;gl_FragColor=color;}else{gl_FragColor=vec4(uniqueID.xy,gl_FragCoord.z,1);}}";
#endif
    
    SpaceText::SpaceText(Font::Type type):font_(Font::font(type)),bufferSize_(0),usedSize_(0)
    {
      glGenBuffers(1,&glBuffer_);
      fColor_[3]=bColor_[3]=-1.0f;
      globalOffset_[0]=globalOffset_[1]=0.0f;
    }
  
    SpaceText::~SpaceText()
    {
      glDeleteBuffers(1,&glBuffer_);
    }
  
    void SpaceText::set_text(const std::vector<std::string>& text,
			     const std::vector<GLfloat>& x,
			     const std::vector<GLfloat>& y,
			     const std::vector<GLfloat>& z)
    {
      size_t n(text.size());
      if(0==n)
	return;
      
      set_text(&text[0],&x[0],&y[0],&z[0],n);     
    }

    void SpaceText::set_text(const std::string* text,
			     const GLfloat* x,
			     const GLfloat* y,
			     const GLfloat* z,
			     size_t n)
    {
      GLfloat height(font_.charHeight);
      const GLfloat* charWidth(font_.charWidth);
      
      std::vector<GLfloat> buffer(0);
      std::vector<GLfloat> widths;

      for(size_t i(0);i<n;++i)
	{
	  widths.push_back(font_.string_width(text[i]));
	  size_t nPoints(text[i].length()*6);
	  for(size_t j(0);j<nPoints;++j)
	    {
	      buffer.push_back(x[i]);buffer.push_back(y[i]);
	      buffer.push_back(z[i]);buffer.push_back(1.0f);
	    }
	}

      for(size_t i(0);i<n;++i)
	{
	  GLfloat xOffset(-widths[i]*0.5f);
	  GLfloat yOffset(-height*0.5f);
	  GLfloat xLeft,xRight(xOffset);
	  GLfloat yBottom(yOffset),yTop(yBottom+height);    
	  
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

      for(unsigned int i(0);i<n;++i)
	{
	  size_t nPoints(text[i].length()*6);
	  for(size_t j(0);j<nPoints;++j)
	    buffer.push_back(widths[i]);
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
    }
    
    void SpaceText::set_color(GLfloat* fColor, GLfloat* bColor)
    {
      for(int i(0);i<4;++i)
	{
	  fColor_[i]=fColor[i];
	  bColor_[i]=bColor[i];
	}
    }

    void SpaceText::set_offset(GLfloat xOffset, GLfloat yOffset)
    {
      globalOffset_[0]=xOffset;
      globalOffset_[1]=yOffset;
    }
    
    void SpaceText::on_gl_context_creation(GLContext* glContext)
    {
      glContext->create_programs<SpaceText>(1);
      GLuint program; glContext->get_programs<SpaceText>(&program);
      glBindAttribLocation(program,positionLocation_,"position");
      glBindAttribLocation(program,localOffsetLocation_,"localOffset");
      glBindAttribLocation(program,uvValueLocation_,"uvValue");
      glBindAttribLocation(program,widthLocation_,"width");
      glContext->compile_program(program,vSource_,"",fSource_);
      glContext->link_program(program);
      glUseProgram(program);
      glUniform1i(glGetUniformLocation(program,"sampler"),0);
      fColorLocationMap_[glContext]=glGetUniformLocation(program,"fColor");
      bColorLocationMap_[glContext]=glGetUniformLocation(program,"bColor");
      globalOffsetLocationMap_[glContext]=glGetUniformLocation(program,"globalOffset");
      heightLocationMap_[glContext]=glGetUniformLocation(program,"height");
      GLERR;
    }
    
    void SpaceText::on_gl_context_destruction(GLContext* glContext)
    {
      glContext->delete_programs<SpaceText>();
      GLERR; 
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"

    void SpaceText::prepare_attributes()
    {
      glEnableVertexAttribArray(positionLocation_);
      glEnableVertexAttribArray(localOffsetLocation_);
      glEnableVertexAttribArray(uvValueLocation_);
      glEnableVertexAttribArray(widthLocation_);
      glVertexAttribPointer(positionLocation_, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glVertexAttribPointer(localOffsetLocation_, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(usedSize_/9*4*sizeof(GLfloat)));
      glVertexAttribPointer(uvValueLocation_, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(usedSize_/9*6*sizeof(GLfloat)));
      glVertexAttribPointer(widthLocation_, 1, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(usedSize_/9*8*sizeof(GLfloat)));
    }

#pragma clang diagnostic pop

    void SpaceText::before_batch_draw(GLContext* glContext)
    {
      GLuint program;
      glContext->get_programs<SpaceText>(&program);
      glUseProgram(program);
      glActiveTexture(GL_TEXTURE0);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
      fColorLocation_=fColorLocationMap_[glContext];
      bColorLocation_=bColorLocationMap_[glContext];
      globalOffsetLocation_=globalOffsetLocationMap_[glContext];
      heightLocation_=heightLocationMap_[glContext];
      GLERR;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    void SpaceText::draw(GLContext* context)
    {

#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
#else
      glBindBuffer(GL_ARRAY_BUFFER,glBuffer_);   
      prepare_attributes();
#endif    
      glUniform4fv(fColorLocation_,1,fColor_);
      glUniform4fv(bColorLocation_,1,bColor_);
      glUniform2fv(globalOffsetLocation_,1,globalOffset_);
      glUniform1f(heightLocation_,font_.charHeight);
      glBindTexture(GL_TEXTURE_2D,font_.textureID);      
      glDrawArrays(GL_TRIANGLES, 0, GLsizei(usedSize_/9));
      GLERR;
    }

#pragma GCC diagnostic pop

  } /* end namespace gui */
} /* end namespace scigma */
