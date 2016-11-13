#include "../common/util.hpp"
#include "bundle.hpp"
#include "bundleshaders.hpp"
#include "glcontext.hpp"
#include "glwindow.hpp"
#include <vector>
#include <iostream>

using scigma::common::connect;
using scigma::common::connect_before;
using scigma::common::disconnect;

namespace scigma
{
  namespace gui
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::map<GLContext*,GLint> Bundle::nDrawnLocationMap_;
    std::map<GLContext*,GLint> Bundle::nTotalLocationMap_;
    std::map<GLContext*,GLint> Bundle::spriteLocationMap_;
    std::map<GLContext*,GLint> Bundle::sizeLocationMap_;
    std::map<GLContext*,GLint> Bundle::colorLocationMap_;
    std::map<GLContext*,GLint> Bundle::lighterLocationMap_;
    std::map<GLContext*,double> Bundle::shaderTimeStampMap_;
#pragma clang diagnostic pop

    GLint Bundle::nDrawnLocation_;
    GLint Bundle::nTotalLocation_;
    GLint Bundle::spriteLocation_;
    GLint Bundle::sizeLocation_;
    GLint Bundle::colorLocation_;
    GLint Bundle::lighterLocation_;

    GLuint Bundle::program_(0);
    
    Bundle::Bundle(GLWindow* glWindow,
		   GLsizei length, GLsizei nRays, GLsizei nVars,
		   const Wave* varyings, const Wave* constants):
      Graph(glWindow),
      PythonObject<Bundle>(this),
      length_(length), nRays_(nRays),
      nVars_(nVars), nConsts_(GLsizei(constants->size())),
      varyingBuffer_(varyings,length*nRays*nVars),
      varyingAttributesInvalid_(true)
    {
      lastTotal_=nVars_>0?length_:1;
      
      glGenBuffers(1,&isoIndexBuffer_);
      glGenBuffers(1,&rayIndexBuffer_);
      glGenBuffers(1,&rayIndexAttributeBuffer_);

      std::vector<GLuint> isoIndices(size_t(length_*nRays_));
      std::vector<GLuint> rayIndices(size_t(length_*nRays_));
      std::vector<GLuint> rayAttributeIndices(size_t(length_*nRays_));

      for(GLsizei ray(0);ray<nRays_;++ray)
	{
	  for(GLsizei point(0);point<length_;++point)
	    {
	      isoIndices[size_t(ray*length_+point)]=GLuint(ray*length_+point);
	      rayIndices[size_t(ray*length_+point)]=GLuint(point*nRays_+ray);
	      rayAttributeIndices[size_t(point*nRays_+ray)]=GLuint(ray*length_+point);
	    }
	}       
      
      glBindBuffer(GL_ARRAY_BUFFER,isoIndexBuffer_);
      glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(GLuint))*length_*nRays_,
		   static_cast<const GLvoid *>(&isoIndices[0]),GL_STATIC_DRAW);
      isoIndexAttributeBuffer_=isoIndexBuffer_;
      
      glBindBuffer(GL_ARRAY_BUFFER,rayIndexBuffer_);
      glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(GLuint))*length_*nRays_,
		   static_cast<const GLvoid *>(&rayIndices[0]),GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER,rayIndexAttributeBuffer_);
      glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(GLuint))*length_*nRays_,
		   static_cast<const GLvoid *>(&rayAttributeIndices[0]),GL_STATIC_DRAW);

      constants->lock();
      for(size_t i(0), size(constants->size());i<size;++i)
	constants_.push_back(constants->data()[i]);
      constants->unlock();

      
      connect<GLBufferInvalidateEvent>(&varyingBuffer_,this);
      varyingBuffer_.begin_transfer();
      
      padding_[0]=0;
    }

    Bundle::~Bundle()
    {
      glDeleteBuffers(1,&rayIndexAttributeBuffer_);
      glDeleteBuffers(1,&rayIndexBuffer_);
      glDeleteBuffers(1,&isoIndexBuffer_);
    }

    
    void Bundle::finalize()
    {
      varyingBuffer_.finalize();
    }

    void Bundle::on_gl_context_creation(GLContext* glContext)
    {
      /* this is just a dummy program which will be replaced in
	 adjust_shaders_for_view()
      */
      glContext->create_programs<Bundle>(1);
      GLuint program; glContext->get_programs<Bundle>(&program);
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

    void Bundle::on_gl_context_destruction(GLContext* glContext)
    {
      glContext->delete_programs<Bundle>();
      GLERR;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    void Bundle::on_addition(GLContext* glContext)
    {
      glContext->request_redraw();
    }

    void Bundle::on_removal(GLContext* glContext)
    {
      disconnect<MouseButtonEvent>(glWindow_,this);
      glContext->request_redraw();
    }
    
    void Bundle::before_batch_draw(GLContext* glContext)
    {
      glContext->get_programs<Bundle>(&program_);
      glUseProgram(program_);
      nDrawnLocation_=nDrawnLocationMap_[glContext];
      nTotalLocation_=nTotalLocationMap_[glContext];
      spriteLocation_=spriteLocationMap_[glContext];
      colorLocation_=colorLocationMap_[glContext];
      sizeLocation_=sizeLocationMap_[glContext];
      lighterLocation_=lighterLocationMap_[glContext];

      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
    }
    
    
    void Bundle::set_attributes_for_view(const std::vector<int>& indices)
    {
      varyingBaseIndex_.clear();varyingAttributeIndex_.clear();
      constantIndex_.clear();constantAttributeIndex_.clear();
      emptyAttributeIndex_.clear();
      size_t n(indices.size());
      for(size_t i(0);i<n;++i)
	{
	  if(indices[i]>0)
	    {
	      varyingBaseIndex_.push_back((size_t(indices[i]-1)));
	      varyingAttributeIndex_.push_back((GLuint(i+1)));
	    }
	  else if(indices[i]<0)
	    {
	      constantIndex_.push_back((size_t(-indices[i]-1)));
	      constantAttributeIndex_.push_back((GLuint(i+1)));
	    }
	  else
	    emptyAttributeIndex_.push_back(GLuint(i+1));
	}
      varyingAttributesInvalid_=true;
    }
    
    using scigma::common::substring;
    
    void Bundle::adjust_shaders_for_view(const VecS& independentVariables,
					 const VecS& expressions,
					 double timeStamp)
    {
      GLContext* glContext(glWindow_->gl_context());
      if(timeStamp<shaderTimeStampMap_[glContext])
	return;
      shaderTimeStampMap_[glContext]=timeStamp;
      
#ifdef SCIGMA_USE_OPENGL_3_2
      std::string vShader(vertexShaderGL3_);
      std::string fShader(fragmentShaderGL3_);
      std::string vertexIn("in");
#else
      std::string vShader(vertexShaderGL2_);
      std::string fShader(fragmentShaderGL2_);
      std::string vertexIn("attribute");
#endif
      std::string attributes;
      for(VecS::const_iterator i(independentVariables.begin()), end(independentVariables.end());i!=end;++i)
	attributes+=vertexIn+" float "+*i+";\n";
      substring(vShader,"__REPLACE_ATTRIBUTES__",attributes);
      substring(vShader,"__REPLACE_X__",expressions[0]);
      substring(vShader,"__REPLACE_Y__",expressions[1]);
      substring(vShader,"__REPLACE_Z__",expressions[2]);
      substring(vShader,"__REPLACE_TIME__",expressions[4]);
      /* color: defined by uniform or by expression? */
      if(""==expressions[3])
	{
	  substring(vShader,"__REPLACE_COLOR_START__","/*");
	  substring(vShader,"__REPLACE_COLOR_END__","*/");
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_START__","");
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_END__","");
	  substring(fShader,"__REPLACE_COLOR_IN_START__","/*");
	  substring(fShader,"__REPLACE_COLOR_IN_END__","*/");
	}
      else
	{
	  substring(vShader,"__REPLACE_COLOR_START__","");
	  substring(vShader,"__REPLACE_COLOR_END__","");
	  substring(vShader,"__REPLACE_COLOR__",expressions[3]);
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_START__","/*");
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_END__","*/");
	  substring(fShader,"__REPLACE_COLOR_IN_START__","");
	  substring(fShader,"__REPLACE_COLOR_IN_END__","");
	}

      /*      std::cout<<"------------------------------------------------------------------"<<std::endl;
      std::cout<<vShader;
      std::cout<<"------------------------------------------------------------------"<<std::endl;
      std::cout<<fShader;*/

      
      glContext->delete_programs<Bundle>();
      glContext->create_programs<Bundle>(1);
      glContext->get_programs<Bundle>(&program_);

      glBindAttribLocation(program_,0,"index_");
      for(size_t i(0),size(independentVariables.size());i<size;++i)
	glBindAttribLocation(program_,GLuint(i+1),independentVariables[i].c_str());
            
      glContext->compile_program(program_,vShader,"",fShader);
      glContext->link_program(program_);

      /* if color is defined by uniform, get the location */
      if(""==expressions[3])
	colorLocationMap_[glContext]=glGetUniformLocation(program_,"rgba_");
      else
	colorLocationMap_[glContext]=-1;

      GLERR;

      nDrawnLocationMap_[glContext]=glGetUniformLocation(program_,"nDrawn_");
      nTotalLocationMap_[glContext]=glGetUniformLocation(program_,"nTotal_");
      spriteLocationMap_[glContext]=glGetUniformLocation(program_,"sprite_");
      sizeLocationMap_[glContext]=glGetUniformLocation(program_,"size_");
      lighterLocationMap_[glContext]=glGetUniformLocation(program_,"lighter_");

      glUseProgram(program_);
      glUniform1i(glGetUniformLocation(program_,"sampler_"),0);
      
      GLERR;

      varyingAttributesInvalid_=true;
    }

    void Bundle::set_style(Style style)
    {
      if(style!=style_)
	{
	  varyingAttributesInvalid_=true;
	  this->Graph::set_style(style);
	}
    }
    
    void Bundle::draw(GLContext* glContext)
    {
      //      std::cout<<"drawing, ";std::cout.flush();
      /* setup vertex attributes (if necessary) */
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
      if(varyingAttributesInvalid_)
	prepare_varying_attributes();
#else
      prepare_varying_attributes();
#endif
      prepare_constant_attributes();
     
      /* set the color, if we do not use a color map */
      if(-1!=colorLocation_)
	glUniform4fv(colorLocation_,1,color_);
     
      /* display the bundle in a lighter color, if hovering */
      if(hovering_&&pickPoint_<0)
	glUniform1i(lighterLocation_,1);
      else
	glUniform1i(lighterLocation_,0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,isoIndexBuffer_);
      glUniform1i(nTotalLocation_,GLint(nRays_));
      glUniform1i(nDrawnLocation_,GLint(nRays_));
      
      /* determine how many points are drawn */
      GLsizei availablePoints(nVars_?(GLsizei(varyingBuffer_.size())/(nRays_*nVars_))*nRays_:1);

      /* draw markers if we are currently replaying and
	 markers are not switched off */
      
      if((lastDrawn_>=0||Marker::NONE==point_)&&Marker::NONE!=marker_)
	draw_markers(availablePoints);

      if(Marker::NONE==point_)
	return;
      
      switch(style_)
	{
	case LINES:
	  draw_lines(availablePoints);
	  break;
	case ISOLINES:
	  draw_isolines(availablePoints);
	  break;
	case POINTS:
	  draw_points(availablePoints);
	  break;
	case SOLID:
	case WIREFRAME:
	  draw_points(availablePoints);
	  break;
	}

      /* if we are in point picking mode, highlight the
	 point under the mouse cursor */
      if(pickPoint_>=0)
	{
	  glBindTexture(GL_TEXTURE_2D,Marker::cross_hair_texture_id());
	  glUniform1i(spriteLocation_,2);
	  glPointSize(Marker::cross_hair_size());
	  glUniform1f(sizeLocation_,Marker::cross_hair_size());
	  glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT,reinterpret_cast<const void*>(GLsizei(sizeof(GLuint))*pickPoint_));
	}
    }

    void Bundle::draw_isolines(GLsizei availablePoints)
    {
      if(lastDrawn_>=0)
	availablePoints=availablePoints>lastDrawn_*nRays_?lastDrawn_*nRays_:availablePoints;

      glUniform1i(spriteLocation_,0);
      glDrawElements(GL_LINE_STRIP, availablePoints,GL_UNSIGNED_INT,0);
    }

    void Bundle::draw_lines(GLsizei availablePoints)
    {
      /* points that are drawn per ray depend on availability of points in memory */
      GLsizei nDrawn(availablePoints/nRays_);
      /* if we are currently replaying, adjust the number of drawn points accordingly */
      if(lastDrawn_>=0)
	nDrawn=nDrawn>lastDrawn_?lastDrawn_:nDrawn;
      /* this is the index up to which we need to draw */
      GLsizei maxIndex(GLsizei(length_*(nRays_-1)+nDrawn));
      /* inform shader about ray length, and how much of the ray we want do draw */
      glUniform1i(nTotalLocation_,GLint(length_));
      glUniform1i(nDrawnLocation_,GLint(nDrawn));

      /* draw line strip */
      if(nRays_>1)
	{
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,rayIndexBuffer_);
	  glBindBuffer(GL_ARRAY_BUFFER,rayIndexAttributeBuffer_);
	  glVertexAttribPointer(0,1,GL_UNSIGNED_INT,GL_FALSE,0,0);
	}
      glUniform1i(spriteLocation_,0);
      glDrawElements(GL_LINE_STRIP,maxIndex,GL_UNSIGNED_INT,reinterpret_cast<const void*>(0));

      if(nRays_>1)
	{
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,isoIndexBuffer_);
	  glBindBuffer(GL_ARRAY_BUFFER,isoIndexAttributeBuffer_);
	  glVertexAttribPointer(0,1,GL_UNSIGNED_INT,GL_FALSE,0,0);
	}
    }

    void Bundle::draw_points(GLsizei availablePoints)
    {
      if(lastDrawn_>=0)
	availablePoints=availablePoints>lastDrawn_*nRays_?lastDrawn_*nRays_:availablePoints;

      GLfloat factor(1.0f);
      if(1==length_&&nRays_==1&&hovering_&&pickPoint_<=0)
	factor=1.2f;
      glBindTexture(GL_TEXTURE_2D,Marker::texture_id(point_));
      glUniform1i(spriteLocation_,1);
      glPointSize(pointSize_*factor);
      glUniform1f(sizeLocation_,pointSize_*factor);
      glDrawElements(GL_POINTS, availablePoints, GL_UNSIGNED_INT,reinterpret_cast<const void*>(0));
    }

    void Bundle::draw_markers(GLsizei availablePoints)
    {
      if(lastDrawn_>=0)
	availablePoints=availablePoints>lastDrawn_*nRays_?lastDrawn_*nRays_:availablePoints;

      glBindTexture(GL_TEXTURE_2D,Marker::texture_id(marker_));
      glUniform1i(spriteLocation_,1);
      glPointSize(markerSize_);
      glUniform1f(sizeLocation_,markerSize_);
      if(availablePoints>=nRays_)
	  glDrawElements(GL_POINTS, nRays_, GL_UNSIGNED_INT,reinterpret_cast<const void*>(GLsizei(sizeof(GLuint))*(availablePoints-nRays_)));
    }
    
    void Bundle::on_hover_begin(GLContext* glContext)
    {
      hovering_=true;
      glContext->request_redraw();
      connect_before<MouseButtonEvent>(glWindow_,this);
    }
    
    void Bundle::on_hover(GLContext* glContext, GLuint value)
    {
      GLsizei newPickPoint;
      if(glfwGetKey(glWindow_->glfw_window(),GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS
	 ||glfwGetKey(glWindow_->glfw_window(),GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS)
	{
	  newPickPoint=GLsizei(value);
	  if(LINES==style_)
	    {
	      newPickPoint=(newPickPoint%length_)*nRays_+newPickPoint/length_;
	    }
	}
      else
	newPickPoint=-1;
      if(newPickPoint==pickPoint_)
	return;
      pickPoint_=newPickPoint;
      glContext->request_redraw();
    }

    void Bundle::on_hover_end(GLContext* glContext)
    {
      disconnect<MouseButtonEvent>(glWindow_,this);
      hovering_=false;
      pickPoint_=-1;
      glContext->request_redraw();
    }

    bool Bundle::process(GLBufferInvalidateEvent e)
    {
      varyingAttributesInvalid_=true;
      return false;
    }
    
#pragma GCC diagnostic pop

    void Bundle::prepare_varying_attributes()
    {
      /* bind buffer with continuous indexes in memory to first attribute */
      glBindBuffer(GL_ARRAY_BUFFER,isoIndexAttributeBuffer_);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0,1,GL_UNSIGNED_INT,GL_FALSE,0,0);
      GLERR;
      /* bind buffer for all varying values and set respective attributes */
      glBindBuffer(GL_ARRAY_BUFFER,varyingBuffer_.buffer_ID());
      size_t nVaryingAttributes(varyingBaseIndex_.size());
      for(size_t i(0);i<nVaryingAttributes;++i)
	{
	  GLERR;
	  GLuint attLoc(varyingAttributeIndex_[i]);
	  glEnableVertexAttribArray(attLoc);
	  glVertexAttribPointer(attLoc,1,GL_FLOAT, GL_FALSE,
				GLsizei(nVars_*GLsizei(sizeof(GLfloat))), 
				reinterpret_cast<const GLvoid*>(sizeof(GLfloat)*varyingBaseIndex_[i]));
      GLERR;
	}
      varyingAttributesInvalid_=false;
    }

    void Bundle::prepare_constant_attributes()
    {
      GLERR;
      size_t nConstAttributes(constantIndex_.size());
      for(size_t i(0);i<nConstAttributes;++i)
	{
	  GLuint attLoc(constantAttributeIndex_[i]);
	  glDisableVertexAttribArray(attLoc);
	  glVertexAttrib1f(attLoc,GLfloat(constants_[constantIndex_[i]]));
      GLERR;
	}
      size_t nEmptyAttributes(emptyAttributeIndex_.size());
      for(size_t i(0);i<nEmptyAttributes;++i)
	{
	  GLuint attLoc(emptyAttributeIndex_[i]);
	  glDisableVertexAttribArray(attLoc);
	  glVertexAttrib1f(attLoc,GLfloat(0.0));
      GLERR;
	}
    }

  } /* end namespace gui */
} /* end namespace scigma */

