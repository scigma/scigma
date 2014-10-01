#include "drawable.h"
#include "glcontext.h"

namespace scigma
{
  namespace gui
  {
    Drawable::Drawable()
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glGenVertexArrays(1,&vertexArray_);
#endif
    }
    Drawable:: ~Drawable()
    {
#ifdef SCIGMA_USE_OPENGL_3_2
      glDeleteVertexArrays(1,&vertexArray_);
#endif
    }
    void Drawable::initialize_drawing(){}
    void Drawable::terminate_drawing(){}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    void Drawable::on_gl_context_creation(GLContext* glContext){}
    void Drawable::on_gl_context_destruction(GLContext* glContext){}
    void Drawable::on_first_addition(GLContext* glContext){}
    void Drawable::on_last_removal(GLContext* glContext){}
    void Drawable::on_addition(GLContext* glContext){glContext->request_redraw();}
    void Drawable::on_removal(GLContext* glContext){glContext->request_redraw();}
    void Drawable::before_batch_draw(GLContext* glContext){}
    void Drawable::after_batch_draw(GLContext* glContext){}
    size_t Drawable::n_shader_passes(GLContext* glContext){return 1;}
    void Drawable::on_hover_begin(GLContext* glContext){}
    void Drawable::on_hover(GLContext* glContext, GLuint value){}
    void Drawable::on_hover_end(GLContext* glContext){}

#pragma GCC diagnostic pop

  } /* End namespace gui */
} /* end namespace scigma */
