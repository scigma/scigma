#ifndef __SCIGMA_GUI_CURVE_H__
#define __SCIGMA_GUI_CURVE_H__

#include <vector>
#include <string>
#include <map>
#include "../pythonobject.h"
#include "../events.h"
#include "../dat/wave.h"
#include "glwindowevents.h"
#include "definitions.h"
#include "graph.h"
#include "drawable.h"

namespace scigma
{
  namespace gui
  {
    class GLContext;
    class GLWindow;
    using dat::Wave;
    
    struct CurveClickEvent{  
      typedef LOKI_TYPELIST_3(const char*,long,bool) Arguments;};
    
    class Curve:public Graph, public Drawable, public PythonObject<Curve>,
      public EventSink<MouseButtonEvent>::Type,
      public EventSource<CurveClickEvent>::Type
      {
      public:
	
	Curve(GLWindow* glWindow,std::string identifier,size_t nExpectedPoints, Wave* variableWave, Wave* constantWave, Marker::Type marker=Marker::STAR, Marker::Type point=Marker::NONE, GLfloat markerSize=16.0f, GLfloat pointSize=8.0f, const GLfloat* color=NULL, GLfloat delay=0.0f);
	~Curve();

	bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods);
	
	static void on_gl_context_creation(GLContext* glContext);
	static void on_gl_context_destruction(GLContext* glContext);
	
	void on_addition(GLContext* context);
	void on_removal(GLContext* context);
	
	void on_hover_begin(GLContext* glContext);
	void on_hover(GLContext* glContext,GLuint value);
	void on_hover_end(GLContext* glContext);
	
	static void before_batch_draw(GLContext* glContext);
      
	void draw(GLContext* glContext);
      
	static void rebuild_shader(GLContext*glContext, 
				   const std::string& vertexShaderHeader,
				   const std::string& fragmentShaderHeader,
				   const ExpressionArray& transformations,
				   const ExpressionArray& attributes,
				   bool useColorMap);
      private:
	Curve(const Curve&);
	Curve& operator=(const Curve&);
      
	void prepare_attributes();      
      
	static std::map<GLContext*,GLint> spriteLocationMap_;
	static std::map<GLContext*,GLint> sizeLocationMap_;
	static std::map<GLContext*,GLint> colorLocationMap_;
	static std::map<GLContext*,GLint> lighterLocationMap_;
	
	static GLint spriteLocation_;
	static GLint sizeLocation_;
	static GLint colorLocation_;
	static GLint lighterLocation_;
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_CURVE_H__ */
