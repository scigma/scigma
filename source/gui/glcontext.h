#ifndef __SCIGMA_GUI_GLCONTEXT_H__
#define __SCIGMA_GUI_GLCONTEXT_H__

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "../pythonobject.h"
#include "../events.h"
#include "definitions.h"
#include "drawable.h"
#include "drawabletypes.h"
#include "glutil.h"

namespace scigma
{
  namespace gui
  {
    //!A class that coordinates OpenGL drawing commands for drawable clients.
    /*!
      GLContext acts as a container for drawable objects.
    */
    class GLContext:public PythonObject<GLContext>,
      public EventSink<LoopEvent>::Type
    {
      friend class GLWindow;
      
    public:
      
      //!List of globally available uniforms
      /*!
	These are handles to the globally available uniforms for programs created by calling
	create_programs(), compile_shader() and link_program(). They should be used as first
	argument in calls to update_global_uniform_4(), update_global_uniform_2x2 and
	update_global_uniform_4x4(). 
	respectively.	
       */
      enum GlobalUniform
      {
	/*!0*/ ROTATION_MATRIX,
	/*!1*/ SCALING_MATRIX,
	/*!2*/ TRANSLATION_MATRIX,
	/*!3*/ PROJECTION_MATRIX,
	/*!4*/ INVERSE_PROJECTION_MATRIX,
	/*!5*/ SCREEN_TO_DEVICE_MATRIX,
	/*!6*/ DEVICE_TO_SCREEN_MATRIX,
	/*!7*/ BACKGROUND_COLOR,
	/*!8*/ FOREGROUND_COLOR,
      };
      //!Number of globally available uniforms
      static const size_t N_GLOBAL_UNIFORMS=9;

      void destroy();
     
      virtual bool process(LoopEvent event);
      void check_for_hover(GLfloat x, GLfloat y);
      void continuous_refresh_needed();
      void continuous_refresh_not_needed();
      double get_current_frame_start_time();
      
      //!create programs for drawable objects
      /*!
	If an OpenGL program needs to use global uniforms
	\param n is the number of programs to create. For simpler drawable objects this
	is generally 1.
	\tparam T is a class that implements the same interface as Drawable. Note
	that Drawable (via its subclasses DrawableFirst/Last) is intended as base class
	for dynamically polymorphic custom drawable objects - types T that are integral parts
	of the scigma gui appear as separate entries in DrawableTypes and are neither
	instantiations nor subclasses of DrawableFirst/Last. This enables efficient batch 
	drawing of many instances of the same type.
      */
      template <class T> void create_programs(size_t n)
	{
	  const size_t index((size_t(Loki::TL::IndexOf<DrawableTypes,T>::value)));
	  if(drawableProgramData[index].size()!=0)
	    {
	      Application::get_instance()->push_error("programs were already created, delete old programs first",__FILE__,__LINE__);
	      return;
	    }
	  for(size_t i=0;i<n;++i)
	    {
	      GLuint program=glCreateProgram();
	      drawableProgramData[index].push_back(static_cast<GLint>(program));
	      drawableProgramData[index].push_back(0);
#ifndef SCIGMA_USE_OPENGL_3_2
	      for(size_t j=0;j<N_GLOBAL_UNIFORMS;++j)
		drawableProgramData[index].push_back(0);
#endif
	    }
	}
      
      template <class T> void get_programs(GLuint* programs)
	{
	  const size_t index((size_t(Loki::TL::IndexOf<DrawableTypes,T>::value)));
	  size_t n = drawableProgramData[index].size();
	  if(0==n)
	    {
	      Application::get_instance()->push_error("there are no programs to get",__FILE__,__LINE__);
	      return;
	    }
	  for(size_t i=0;i<n/PROGRAM_DATA_STRIDE;++i)
	    programs[i]=static_cast<GLuint>(drawableProgramData[index].at(i*PROGRAM_DATA_STRIDE));
	}
      
      template <class T> void delete_programs()
	{
	  const size_t index((size_t(Loki::TL::IndexOf<DrawableTypes,T>::value)));
	  size_t n = drawableProgramData[index].size();
	  if(0==n)
	    {
	      Application::get_instance()->push_error("there are no programs to delete",__FILE__,__LINE__);
	      return;
	    }
	  for(size_t i=0;i<n/PROGRAM_DATA_STRIDE;++i)
	    glDeleteProgram(static_cast<GLuint>(drawableProgramData[index].at(i*PROGRAM_DATA_STRIDE)));
	  
	  drawableProgramData[index].clear();
	}
      
      void compile_program(GLuint program, const std::string& vertexShaderBody,
			   const std::string& geometryShaderBody, 
			   const std::string& fragmentShaderBody);
      void link_program(GLuint program);
      
      //!stalls redraws until flush() is called
      /*!
	This function prohibits the GLContext from redrawing, even if request_redraw() is called. 
	The request is stored, however, and executed as soon as flush() is called. The combination
	of stall() and flush() can be used to enclose many request_redraw() statements, which then
	result in a single redraw (for efficiency reasons). stall() calls do stack, so for each 
	sucessive call of stall(), flush() must be called separately before anything is drawn.
	This allows for nested uses of stall() and flush().
      */
      void stall();
      
      //!revokes previous call of stall() 
      /*
	This function cancels the effect of a previously called stall(). If the stall() stack
	is empty (i.e. flush() was called as many times as stall()) and request_redraw() has 
	been called, the scene is redrawn. 
       */
      void flush();
      
      //!redraws, if not stalled
      /*!
	This function draws all drawable objects that are currently part of the GLContext, except if
	drawing has been prohibited by a call to stall().
      */
      void request_redraw();

      //!adds a drawable object
      /*!
	This function adds an instance of a class that implements the same interface as Drawable
	to the GLContext. It is only defined for types T that are member of DrawableTypes.
	@tparam T is the type of the drawable object
	@param object is a pointer to the drawable object
       */
      template <class T> void add_drawable(T *object)
	{
	  void* obj(reinterpret_cast<void*>(object));
	  const size_t index(Loki::TL::IndexOf<DrawableTypes,T>::value);
	  typename std::vector<void*>::iterator i=
	    find(drawables[index].begin(),drawables[index].end(),obj);
	  
	  if(i!=drawables[index].end())
	    {
	      Application::get_instance()->push_error("drawable object was added to window before", __FILE__,__LINE__);
	      return;
	    }
	  else
	    {
	      drawables[index].push_back(obj);
	      if(1==drawables[index].size())
		  T::on_first_addition(this);
	      object->on_addition(this);
	      double x,y;
	      glfwGetCursorPos(glfwWindowPointer_,&x,&y);
	      check_for_hover(GLfloat(x),GLfloat(y-height_));
	    }
	}
     
      //!removes a drawable object 
      /*!
	This function removes an instance of a class that implements the same interface as 
        Drawable from the GLcontext. It is only defined for types T that are member of DrawableTypes.
	@tparam T is the type of the drawable object
	@param object is a pointer to the drawable object
       */
      template <class T> void remove_drawable(T* object)
	{
	  const size_t index(Loki::TL::IndexOf<DrawableTypes,T>::value);
	  std::vector<void*>::iterator i = find(drawables[index].begin(),drawables[index].end(),object);
	  if(i==drawables[index].end())
	    {
	      Application::get_instance()->push_error("drawable object was not previously added to the window", __FILE__,__LINE__);
	    }
	  else 
	    {
	      // if this is the object under the cursor, set the hoverIndex_ to its non-object value
	      size_t hoverIndexBase(0);
	      for(size_t j(1);j<index;++j)
		hoverIndexBase+=drawables[j-1].size();
	      if(long(hoverIndex_-hoverIndexBase)==i-drawables[index].begin())
		{
		  object->on_hover_end(this);
		  hoverIndex_=0xFFFFFFFF;
		}
	      drawables[index].erase(i);
	      object->on_removal(this);
	      if(0==drawables[index].size())
		T::on_last_removal(this);
	      double x,y;
	      glfwGetCursorPos(glfwWindowPointer_,&x,&y);
	      check_for_hover(GLfloat(x),GLfloat(y-height_));
	    }
	}
      
      void update_global_uniform_4(GlobalUniform uniform, const GLfloat* vector);
      void update_global_uniform_2x2(GlobalUniform uniform, const GLfloat* matrix);
      void update_global_uniform_4x4(GlobalUniform uniform, const GLfloat* matrix);
      void set_viewport(GLint left, GLint bottom, GLint width, GLint height);

      static std::string create_fragment_main(const std::string& body,const std::string& colorExpression, 
					      const std::string& hoverExpression,const std::string& colorOutput);
      
    private:
      GLContext(GLFWwindow* ptr);
      GLContext(const GLContext&);
      GLContext& operator=(const GLContext&);
      virtual ~GLContext();
      
      bool find_program(GLuint program, std::vector<GLint>::const_iterator* position=NULL) const;
      bool find_program(GLuint program, std::vector<GLint>::iterator* position=NULL);

      void object_to_color(unsigned char typeIndex, size_t objectIndex, GLfloat* color);

      template<class TypeList> void creation_notify()
	{
	  TypeList::Head::on_gl_context_creation(this);
	  creation_notify<typename TypeList::Tail>();
	}

      template<class TypeList> void destruction_notify()
	{
	  TypeList::Head::on_gl_context_destruction(this);
	  destruction_notify<typename TypeList::Tail>();
	}
                
      template<class TypeList> void draw(size_t objectIndexBase)
	{
	  GLfloat color[3];
	  const unsigned char typeIndex((Loki::TL::Length<DrawableTypes>::value-Loki::TL::Length<TypeList>::value));
	  size_t nType(drawables[typeIndex].size());
	  if(0!=nType)
	    {
	      size_t nPasses(TypeList::Head::n_shader_passes(this));
	      for(size_t i(0);i<nPasses;++i)
		{
		  TypeList::Head::before_batch_draw(this);
		  for(size_t j(0);j<nType;++j)
		    {
		      if(colorPicking_)
			{
			  color[0]=GLfloat((objectIndexBase+j)%0x100)/GLfloat(0xFF);
			  color[1]=GLfloat((objectIndexBase+j)%0x10000-(objectIndexBase+j)%0x100)/GLfloat(0xFFFF);
			  color[2]=1.0;
			}
		      else
			color[2]=0.0;
		      glUniform3fv(drawableProgramData[typeIndex][1],1,color);
		      (reinterpret_cast<typename TypeList::Head*>(drawables[typeIndex][j]))->draw(this);
		    }
		  TypeList::Head::after_batch_draw(this);
		  }
	    }
	  draw<typename TypeList::Tail>(objectIndexBase+nType);
	}

      template<class TypeList> void begin_hover(size_t hoverIndexBase)
	{
	  const size_t typeIndex((Loki::TL::Length<DrawableTypes>::value-Loki::TL::Length<TypeList>::value));
	  size_t nType(drawables[typeIndex].size());
	  if(hoverIndex_<hoverIndexBase+nType)
	    (reinterpret_cast<typename TypeList::Head*>(drawables[typeIndex][hoverIndex_-hoverIndexBase]))->on_hover_begin(this);
	  else
	    begin_hover<typename TypeList::Tail>(hoverIndexBase+nType);
	}
      
      template<class TypeList> void hover(size_t hoverIndexBase, GLint x, GLint y)
	{
	  const size_t typeIndex((Loki::TL::Length<DrawableTypes>::value-Loki::TL::Length<TypeList>::value));
	  size_t nType(drawables[typeIndex].size());
	  if(hoverIndex_<hoverIndexBase+nType)
	    {
	      /*draw the object that is currently under the cursor and read back the buffer a second time, 
		to get specific, vertex dependent information
	      */
	      size_t nPasses(TypeList::Head::n_shader_passes(this));
	      for(size_t i(0);i<nPasses;++i)
		{
		  TypeList::Head::before_batch_draw(this);
		  glUniform3f(drawableProgramData[typeIndex][1],0.0f,0.0f,2.0f);
		  (reinterpret_cast<typename TypeList::Head*>(drawables[typeIndex][hoverIndex_-hoverIndexBase]))->draw(this);
		}
	      GLubyte color[75];
	      glReadPixels(x,y,5,5,GL_RGB,GL_UNSIGNED_BYTE,color);
	      size_t minIndex(0),maxIndex(25);
	      GLuint value(0xFFFFFFFF);
	      for(size_t i(minIndex);i<maxIndex;++i)
		{
		  if(color[i*3]!=0xFF||color[i*3+1]!=0xFF||color[i*3+2]!=0xFF)
		    {
		      value=color[i*3]*0x10000+color[i*3+1]*0x100+color[i*3+2];
		      break;
		    }
		}
	      if(value!=0xFFFFFFFF)
		 (reinterpret_cast<typename TypeList::Head*>(drawables[typeIndex][hoverIndex_-hoverIndexBase]))->on_hover(this,value);
	    }
	  else
	    hover<typename TypeList::Tail>(hoverIndexBase+nType,x,y);
	}
      
      template<class TypeList> void end_hover(size_t hoverIndexBase)
	{
	  const size_t typeIndex((Loki::TL::Length<DrawableTypes>::value-Loki::TL::Length<TypeList>::value));
	  size_t nType(drawables[typeIndex].size());
	  if(hoverIndex_<hoverIndexBase+nType)
	    (reinterpret_cast<typename TypeList::Head*>(drawables[typeIndex][hoverIndex_-hoverIndexBase]))->on_hover_end(this);
	  else
	    end_hover<typename TypeList::Tail>(hoverIndexBase+nType);
	}
      
      static constexpr double TARGET_FRAME_RATE=30;
      static constexpr double TARGET_FRAME_TIME=1/TARGET_FRAME_RATE;

      static const char* VERTEX_SHADER_HEADER;
      static const char* GEOMETRY_SHADER_HEADER;
      static const char* FRAGMENT_SHADER_HEADER;
            
      // buffer accomodates 7 4x4 matrices: translation, scaling, rotation,
      // projection, inverse projection, screen-to-device and device-to-screen
      // and two 4 vectors: background color and foreground color
      static const GLuint GLOBAL_UNIFORM_BUFFER_SIZE=(7*N_4X4_ELEMENTS+
						      2*N_ROWS)*sizeof(GLfloat);
  
      static const GLuint GLOBAL_UNIFORM_OFFSETS[];

      void assign_static_unique_IDs(size_t typeIndex, size_t objectIndex);

      enum{N_DRAWABLE_TYPES=Loki::TL::Length<DrawableTypes>::value};
      std::vector<GLint> drawableProgramData[N_DRAWABLE_TYPES];
      std::vector<void*> drawables[N_DRAWABLE_TYPES];
      
      double currentFrameStartTime_;
      double currentFrameRenderingTime_;
      
      GLFWwindow* glfwWindowPointer_;
      
      GLint width_, height_;
      size_t stalled_;

      size_t hoverIndex_;      
      size_t refreshContinuously_;

#ifdef SCIGMA_USE_OPENGL_3_2
      GLuint globalUniformBuffer_;
      static const GLuint GLOBAL_UNIFORM_BINDING_INDEX=0;
      static const size_t PROGRAM_DATA_STRIDE=2;
      char padding_[2];
#else
      void update_4_uniform_in_all_programs(GlobalUniform uniform, const GLfloat* vector);
      void update_2x2_uniform_in_all_programs(GlobalUniform uniform,const GLfloat* matrix);
      void update_4x4_uniform_in_all_programs(GlobalUniform uniform,const GLfloat* matrix);
      GLfloat globalUniformMemory[GLOBAL_UNIFORM_BUFFER_SIZE];
      static const size_t PROGRAM_DATA_STRIDE=2+N_GLOBAL_UNIFORMS;
      char padding_[6];
#endif
      

      bool redrawRequested_;
      bool colorPicking_;

    };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_GLCONTEXT_H__ */
