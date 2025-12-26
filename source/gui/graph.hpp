#ifndef SCIGMA_GUI_GRAPH_HPP
#define SCIGMA_GUI_GRAPH_HPP

#include <string>
#include <vector>
#include <map>
#include "../common/events.hpp"
#include "glwindowevents.hpp"
#include "definitions.hpp"
#include "marker.hpp"
#include "application.hpp"

using scigma::common::EventSource;
using scigma::common::EventSink;

namespace scigma
{
  namespace gui
  {
    struct GraphClickEvent{  
      typedef LOKI_TYPELIST_4(int,int,int,int) Arguments;};

    struct GraphDoubleClickEvent{  
      typedef LOKI_TYPELIST_1(int) Arguments;};

    class GLContext;
    class GLWindow;

    class Graph:
      public EventSource<GraphClickEvent>::Type,
      public EventSource<GraphDoubleClickEvent>::Type,
      public EventSink<LoopEvent>::Type,
      public EventSink<MouseButtonEvent>::Type
    {
   
    public:

      typedef std::vector<std::string> VecS;
      typedef std::map<GLContext*, std::vector<GLfloat> > LightMap;
      
      enum Style
	{
	  POINTS,
	  LINES,
	  ISOLINES,
	  WIREFRAME,
	  SOLID
	};

      void set_marker_style(Marker::Type marker) override;
      Marker::Type marker_style() const;
      
      void set_marker_size(GLfloat size) override;
      GLfloat marker_size() const;
      
      void set_point_style(Marker::Type point) override;
      Marker::Type point_style() const;

      void set_point_size(GLfloat size) override;
      GLfloat point_size() const;

      void set_color(const GLfloat* color) override;
      const GLfloat* color() const;
      
      void set_delay(GLfloat delay) override;
      GLfloat delay() const;

      void set_style(Style style) override;
      GLfloat style() const;

      void replay();

      virtual void finalize()=0;

      virtual void set_attributes_for_view(const std::vector<int>& indices)=0;

      virtual void adjust_shaders_for_view(const VecS& independentVariables,
					   const VecS& expressions,
					   double timeStamp)=0;

      static void set_light_direction(GLContext* glContext, const GLfloat* direction);
      static GLfloat* light_direction(GLContext* glContext);

      bool process(LoopEvent Event);
      bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods);
      
    protected:
      Graph(GLWindow* glWindow); 
      virtual ~Graph();
      
      GLWindow* glWindow_;
      
      double doubleClickTime_;
      double lastClickTime_;

      double startTime_;
      
      Marker::Type marker_;
      Marker::Type point_;

      GLfloat markerSize_;
      GLfloat pointSize_;

      GLfloat color_[4];

      GLfloat delay_;
      Style style_;

      GLsizei lastDrawn_;
      GLsizei lastTotal_;
      
      GLsizei pickPoint_;

      bool hovering_;
      
      /* for colors, an isoluminant color map that works both on white
	 and black background is hardcoded here */
      static const char* colorMapFunction_;

      static LightMap lightDirection_;

      /* this is the 1 point buffer for a dummy attribute, necessary because
	 we always need to enable vertex attribute array 0, at least on some 
	 implementations */
      static GLuint dummyBuffer_;

      
    private:
      Graph(const Graph&);
      Graph& operator=(const Graph&);

      char padding_[7];

      int escapeCount;      
    };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_GRAPH_HPP */
