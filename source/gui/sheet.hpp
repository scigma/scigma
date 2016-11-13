#ifndef SCIGMA_GUI_SHEET_HPP
#define SCIGMA_GUI_SHEET_HPP

#include <string>
#include "../common/pythonobject.hpp"
#include "../dat/mesh.hpp"
#include "glwindowevents.hpp"
#include "graph.hpp"
#include "drawable.hpp"
#include "glbuffer.hpp"

using scigma::common::PythonObject;
using scigma::dat::Mesh;

namespace scigma
{
  namespace gui
  {

    class GLWindow;

    class Sheet: public Graph,
		 public Drawable,
		 public PythonObject<Sheet>,
		 public EventSink<GLBufferInvalidateEvent>::Type
    {
    private:
      typedef AbstractWave<double> Wave;
      typedef AbstractGLBuffer<GLfloat,double> GLfloatBuffer;
      typedef AbstractGLBuffer<GLuint,GLint> GLuintBuffer;
      typedef AbstractGLBuffer<GLbyte,GLbyte> GLbyteBuffer;
      
    public:
      Sheet(GLWindow* glWindow, 
	    const Mesh* mesh, GLsizei nVars, const Wave* constants);
      ~Sheet();

      // methods from the Drawable interface 
      static void on_gl_context_creation(GLContext* glContext);
      static void on_gl_context_destruction(GLContext* glContext);
      static void before_batch_draw(GLContext* glContext);
      void on_addition(GLContext* context);
      void on_removal(GLContext* context);
      void draw(GLContext* glContext);
      void on_hover_begin(GLContext* glContext);
      void on_hover(GLContext* glContext, GLuint value);
      void on_hover_end(GLContext* glContext);
      
      // virtual methods inherited from Graph
      virtual void finalize();

      virtual void set_attributes_for_view(const std::vector<int>& indices);

      virtual void adjust_shaders_for_view(const VecS& independentVariables,
					   const VecS& expressions,
					   double timeStamp);
      virtual void set_style(Style style);
      void set_light_direction(const GLfloat* direction);
      void set_light_parameters(const GLfloat* parameters);

      using Graph::process;
      bool process(GLBufferInvalidateEvent e);
      
    private:
      void prepare_varying_attributes();
      void prepare_constant_attributes();

      void draw_markers();
      void draw_triangles();
      void draw_isolines();
      void draw_points();
      
      static const std::string vertexShaderGL2_;
      static const std::string vertexShaderGL3_;
      static const std::string fragmentShaderGL2_;
      static const std::string fragmentShaderGL3_;

      static std::map<GLContext*,GLint> spriteLocationMap_;
      static std::map<GLContext*,GLint> sizeLocationMap_;
      static std::map<GLContext*,GLint> colorLocationMap_;
      static std::map<GLContext*,GLint> lighterLocationMap_;
      static std::map<GLContext*,GLint> lightDirLocationMap_;
      static std::map<GLContext*,GLint> lightParamLocationMap_;
      
      static GLint spriteLocation_;
      static GLint sizeLocation_;
      static GLint colorLocation_;
      static GLint lighterLocation_;
      static GLint lightDirLocation_;
      static GLint lightParamLocation_;

      static GLuint program_;
      static std::map<GLContext*,double> shaderTimeStampMap_;
      
      GLsizei nVars_;
      GLsizei nConsts_;

      const Mesh* mesh_;
      
      GLuintBuffer isoIndexBuffer_;
      GLbyteBuffer isoEndPointsBuffer_;
      GLuintBuffer triangleIndexBuffer_;
      
      GLfloatBuffer varyingBuffer_;
      std::vector<GLfloat> constants_;

      std::vector<size_t> varyingBaseIndex_;
      std::vector<size_t> constantIndex_;

      std::vector<GLuint> varyingAttributeIndex_;
      std::vector<GLuint> constantAttributeIndex_;
      std::vector<GLuint> emptyAttributeIndex_;

      GLfloat lightDirection_[4];
      GLfloat lightParameters_[4];
      
      bool varyingAttributesInvalid_;
      
      char padding_[7];
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_SHEET_HPP */
