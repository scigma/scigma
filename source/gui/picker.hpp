#ifndef SCIGMA_GUI_PICKER_HPP
#define SCIGMA_GUI_PICKER_HPP

#include <map>
#include "../common/events.hpp"
#include "drawable.hpp"
#include "marker.hpp"
#include "glwindowevents.hpp"
#include "../common/pythonobject.hpp"

using scigma::common::PythonObject;
using scigma::common::EventSource;
using scigma::common::EventSink;

namespace scigma
{
  namespace gui
  {

    class GLWindow;
    
    struct PickEvent{  
      typedef LOKI_TYPELIST_4(bool,GLfloat,GLfloat,GLfloat) Arguments;};

    class Picker:public Drawable, public PythonObject<Picker>,
      public EventSink<MouseButtonEvent>::Type,
      public EventSink<MouseMotionEvent>::Type,
      public EventSource<PickEvent>::Type
	{
	public:
	  Picker(int view);
	  ~Picker() override;

	  static void on_gl_context_creation(GLContext* context);
	  static void on_gl_context_destruction(GLContext* context);

	  static void before_batch_draw(GLContext* context);

	  void draw(GLContext* context) override;

	  bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods) override;
	  bool process(MouseMotionEvent event, GLWindow* w, GLfloat x, GLfloat y, GLfloat xOld, GLfloat yOld) override;

	private:
	  void prepare_attributes();

	  double doubleClickTime_;
	  double lastClickTime_;

	  GLuint glBuffer_;
	  
	  Marker::Type marker_;
	  GLfloat markerSize_;
	  
	  GLfloat points_[12];
	  GLfloat v_[3];
	  GLfloat v0_[3];

	  int view_;

	  bool visible_;

	  char padding_[7];

	  static const char* vSource_;
	  static const char* fSource_;    

	  static const GLuint positionLocation_=0;

	  static std::map<GLContext*,GLint> spriteLocationMap_;
	  static std::map<GLContext*,GLint> sizeLocationMap_;

	  static GLint spriteLocation_;
	  static GLint sizeLocation_;
	};
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_PICKER_HPP */
