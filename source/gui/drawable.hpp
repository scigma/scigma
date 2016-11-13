#ifndef SCIGMA_GUI_DRAWABLE_HPP
#define SCIGMA_GUI_DRAWABLE_HPP

#include <Typelist.h>
#include "definitions.hpp"

namespace scigma
{
  namespace gui
  {

    class GLContext;
    class GLWindow;

    //!Base class for drawable objects.
    /*!@ingroup gui
      This is the interface that any drawable object used with the
      GLContext class must implement. Note that this class is NOT intended to be instantiated
      directly. Instead, inherit the interface directly (like for example the Bundle class).
      Classes inheriting from Drawable must be registered in the DrawableTypes type list
      below, before they will work with GLContext. This forces a recompile of the Scigma gui module.
      Under OpenGL 3.2, this class provides a member vertexArray_, since basically all Drawables objects
      will use a VertexArrayObject.
    */
    class Drawable
      {
      public:
	//!class initialization code goes here
	/*!
	  This function is called once from Application::initialize(). Put all the initialization
	  code for the class in here (e.g. create vertex buffers, textures, programs that are
	  shared across all GLContext instances).
	 */
	static void initialize_drawing();

	//!class termination code goes here
	/*!
	  This function is called once from Application::terminate(). Put all the termination
	  code for the class in here (e.g. delete vertex buffers, textures, programs that are
	  shared across all GLContext instances).
	 */
	static void terminate_drawing();
	
	//!called when a GLContext is created
	/*!
	  This function is called whenever an instance of the class is added to a particular GLContext
	  object for the first time. Put all the per-context initialization code in here (e.g. 
	  create shader programs that depend on the ViewingVolume/ViewingArea global uniforms, etc.).
	  @param context is a pointer to the GLContext the object was added to. 
	*/
	static void on_gl_context_creation(GLContext* glContext);
	
	//!called when a GLContext is destroyed
	/*!
	  This function is called whenever the last instance of the class is removed from a 
	  particular GLContext object. Put all the per-context termination code in here (e.g.
	  delete shader programs that depend on the ViewingVolume/ViewingArea global uniforms, etc.).
	  @param context is a pointer to the GLContext the object was removed from.
	*/
	static void on_gl_context_destruction(GLContext* glContext);

	//!called when first instance is added to a GLContext
	/*!
	  This function is called whenever an instance of the class is added to a particular GLContext
	  object for the first time. Put code here that is only needed once for all instances, but
	  for some reason does not belong in on_gl_context_creation() (e.g allocating a huge buffer,
	  that is only needed if an object is actually drawn).
	  @param context is a pointer to the GLContext the object was added to. 
	*/
	static void on_first_addition(GLContext* glContext);
	
	//!called when last instance is removed from a GLContext
	/*!
	  This function is called whenever the last instance of the class is removed from a 
	  particular GLContext object. Put code here that is only needed once for all instances,
	  but for some reason does not belong in on_gl_context_destruction() (e.g freeing common 
	  resources immediately when the last object of a type is removed from the context, 
	  instead of waiting for the GLcontext to be destroyed).
	  @param context is a pointer to the GLContext the object was removed from.
	*/
	static void on_last_removal(GLContext* glContext);
	
	//!called immediately before all instances of the class are drawn
	/*!
	  This function is called once before all instances of a drawable type are drawn.
	  Use this function to apply OpenGL settings that do not change between different instances
	  of the class (e.g. `glUseProgram(prog)` if the class only uses a single program `prog`,
	  binding common textures etc.).
	  @param context is a pointer to the GLContext that is currently drawn.
	*/
	static void before_batch_draw(GLContext* glContext);

	//!called immediately after all instances of the class are drawn
	/*!
	  This function is called once before all instances of a drawable type are drawn.
	  @param context is a pointer to the GLContext that is currently drawn.
	*/
	static void after_batch_draw(GLContext* glContext);

	
	//!returns the number of shader passes
	/*! 
	  This function is called by GLContext to determine how often before_batch_draw(),
	  the subsequent draw calls and after_batch_draw() should be executed. 
	  Use this in conjunction with multiple	 shaders. The drawable type instance is
	  responsible for keeping track of which shader to run during which call.

	  @return the number of shader passes for this type of drawable object.
	*/
	static size_t n_shader_passes(GLContext* glContext);
	
	//!called whenever an instance of the class is added to a GLContext
	/*!
	  This function is called whenever a static drawable object is added to a GLContext.
	  Use this function to run any per-object initialization code that depends on the GLContext.
	  In particular, this function should call the GLContext::request_redraw() method on the context.
	  @param context is a pointer to the GLContext the object was added to.
	*/
	virtual void on_addition(GLContext* glContext);

	//!called whenever an instance of the class is added to a GLContext
	/*!
	  This function is called whenever a static drawable object is removed from a GLContext.
	  Use this function to apply any per-object termination code that depends on the GLContext.
	  In particular, this function should call the GLContext::request_redraw() method on the context.
	  @param context is a pointer to the GLContext the object was removed from.
	*/
	virtual void on_removal(GLContext* glContext);
	
	//!called when an instance of the class is drawn
	/*!
	  This function is called whenever a static drawable object is drawn. Put all per-object drawing code
	  in here (e.g. binding vertex arrays, buffers etc. that belong exclusively to this object, and calls
	  to `glDrawArrays(...)` etc.).
	  @param context is a pointer to the GLContext that is currently drawn, which the object is a part of.
	*/
	virtual void draw(GLContext* glContext)=0;

	//!called when mouse enters region occupied by the object on screen
	/*!
	  Whenever the mouse moves within 2 pixels distance from the object on screen coming from a pixel
	  further away, this function is called. 
	  @param context is a pointer to the GLContext the object is part of.
	 */
	virtual void on_hover_begin(GLContext* glContext);

	//!called while mouse is moved hovering over the object on screen
	/*!
	  Whenever the mouse moves more than 2 pixels away from the object on screen coming from a pixel
	  inside that radius, this function is called. 
	  @param context is a pointer to the GLContext the object is part of.
	  @param value contains specific information about the object written by the shader.
	 */
	virtual void on_hover(GLContext* glContext, GLuint value);

	//!called when mouse leaves region occupied by the object on screen
	/*!
	  Whenever the mouse moves more than 2 pixels away from the object on screen coming from a pixel
	  inside that radius, this function is called. 
	  @param context is a pointer to the GLContext the object is part of.
	 */
	virtual void on_hover_end(GLContext* glContext);

	Drawable();
	virtual ~Drawable();

#ifdef SCIGMA_USE_OPENGL_3_2
      protected:
	GLuint vertexArray_;
	char padding_[4];
#endif
      };
    
    template <class TypeList> static void initialize_drawables()
      {
	TypeList::Head::initialize_drawing();
	initialize_drawables<typename TypeList::Tail>();
      }
    
    template <class TypeList> static void terminate_drawables()
      {
	TypeList::Head::terminate_drawing();
	terminate_drawables<typename TypeList::Tail>();
      }

    template<> inline void initialize_drawables<LOKI_TYPELIST_0>()
      {
	return;
      }
    
    template<> inline void terminate_drawables<LOKI_TYPELIST_0>()
      {
	return;
      }
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_DRAWABLE_HPP */
