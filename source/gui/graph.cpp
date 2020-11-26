#include "graph.hpp"
#include "glwindow.hpp"
#include <iostream>

using scigma::common::connect_before;
using scigma::common::disconnect;

extern "C" int ESCAPE_COUNT;

namespace scigma
{
  namespace gui
  {

    const char* Graph::colorMapFunction_=
      "vec4 colormap_(in float p)\n"
      "{\n"
      "vec4 color;\n"
      "\tif(p<0.0||p>1.0)\n"
      "\t\tcolor= vec4(1,0.0,1,1.0);\n"
      "\telse if (p<0.5)\n"
      "\t\tcolor= mix(vec4(0,0,1.0,1.0),vec4(0.0,1.0,0,1.0),p*2);\n"
      "\telse\n"
      "\t\tcolor= mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0.0,1.0),p*2-1);\n"
      "\treturn color\n;"
      "}\n\n";
    //+*/mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0,1.0),p*2-1)*/*step(p,0.5)*/;\n"
    
    
    /*      "vec4 colormap_(in float p)\n"
	    "{\n"
	    "\tif(p<0.0||p>1.0)\n"
	    "\t\treturn vec4(0.72,0.0,0.72,1.0);\n"
	    "\treturn vec4(\n"
	    "\t\tclamp(-0.95*p*p+0.86*p+0.43+2.2*abs(p-0.35),0,1),\n"
	    "\t\t-1.1*p*p+0.58*p+0.75-0.56*abs(p-0.75),\n"
	    "\t\t-0.32*p*p+0.64*p+0.4-1.2*abs(p-0.5),\n"
	    "\t\t1.0);\n"
	    "}\n\n";*/

    GLuint Graph::dummyBuffer_(0);
    
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wglobal-constructors"
 #pragma clang diagnostic ignored "-Wexit-time-destructors"
    Graph::LightMap Graph::lightDirection_;
 #pragma clang diagnostic pop

    
    Graph::Graph(GLWindow* glWindow):
      glWindow_(glWindow),
      doubleClickTime_(0.25),lastClickTime_(-1.0),
      startTime_(-1),
      marker_(Marker::STAR),point_(Marker::DOT),
      markerSize_(16),pointSize_(1),
      delay_(0),style_(POINTS),
      lastDrawn_(-1),lastTotal_(0xFFFFFFF),
      pickPoint_(-1),hovering_(false)
    {
      if(!dummyBuffer_)
	{
	  glGenBuffers(1,&dummyBuffer_);
	  glBindBuffer(GL_ARRAY_BUFFER,dummyBuffer_);
	  glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(GLfloat)*1),NULL,GL_DYNAMIC_DRAW);
	  glBindBuffer(GL_ARRAY_BUFFER,0);
	}
      
      GLfloat col[]={1.0f,1.0f,1.0f,1.0f};
      set_color(col);
    }
    
    Graph::~Graph()
    {}
    
    void Graph::set_marker_style(Marker::Type marker){marker_=marker;glWindow_->gl_context()->request_redraw();}
    Marker::Type Graph::marker_style() const {return marker_;}

    void Graph::set_marker_size(GLfloat size){markerSize_=size<1.0f?1.0f:size;glWindow_->gl_context()->request_redraw();}
    GLfloat Graph::marker_size() const{return markerSize_;}
    
    void Graph::set_point_style(Marker::Type marker){point_=marker;glWindow_->gl_context()->request_redraw();}
    Marker::Type Graph::point_style() const{return point_;}
    
    void Graph::set_point_size(GLfloat size){pointSize_=size<1.0f?1.0f:size;glWindow_->gl_context()->request_redraw();}
    GLfloat Graph::point_size() const{return pointSize_;}
    
    void Graph::set_color(const GLfloat* color)
    {
      for(size_t i(0);i<4;++i)
	color_[i]=color[i];
      glWindow_->gl_context()->request_redraw();
    }
    const GLfloat* Graph::color() const{return color_;}

    void Graph::set_delay(GLfloat delay){delay_=delay<0.0f?0.0f:delay;}
    GLfloat Graph::delay() const{return delay_;}

    void Graph::set_style(Style style){style_=style;}
    GLfloat Graph::style() const{return style_;}

    void Graph::set_light_direction(GLContext* glContext, const GLfloat* direction)
    {
      std::vector<GLfloat>& dir(lightDirection_[glContext]);
      dir.resize(3);
      for(size_t i(0);i<3;++i)
	dir[i]=direction[i];
    }

    GLfloat* Graph::light_direction(GLContext* glContext)
    {
      std::vector<GLfloat>& dir(lightDirection_[glContext]);
      if(dir.size())
	return &dir[0];
      return NULL;
    }

    void Graph::replay()
    {
      startTime_=glfwGetTime();
      escapeCount = ESCAPE_COUNT;
      if(delay_>0)
	{
	  lastDrawn_=0;
	  connect_before<LoopEvent>(Application::get_instance(),this);
	}
      else
	glWindow_->gl_context()->request_redraw();
	
    }
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    bool Graph::process(LoopEvent event)
    {
      if(ESCAPE_COUNT!=escapeCount)
	{
	  delay_=0;
	}
      
      if(!(delay_>0&&lastDrawn_<lastTotal_))
	{
	  disconnect<LoopEvent>(Application::get_instance(),this);
	  lastDrawn_=-1;
	  glWindow_->gl_context()->request_redraw();
	  return false;
	}
      GLsizei newLastDrawn(GLsizei((glfwGetTime()-startTime_)/double(delay_)));
      if(newLastDrawn!=lastDrawn_)
	{
	  lastDrawn_=newLastDrawn;
	  glWindow_->gl_context()->request_redraw();
	}
      return false;
    }

    bool Graph::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(GLFW_PRESS==action)
	{
	  double time(glfwGetTime());
	  double dt(time-lastClickTime_);
	  lastClickTime_=time;
	  if(dt<=doubleClickTime_&&GLFW_MOUSE_BUTTON_LEFT==button)
	    {
	      if(pickPoint_>=0)
		{
		  EventSource<GraphDoubleClickEvent>::Type::emit(int(pickPoint_));
		}
	      else
		{
		  EventSource<GraphDoubleClickEvent>::Type::emit(-1);
		}
	    }
	  else
	    {
	      const GLfloat* cursor(w->cursor_position());
	      if(pickPoint_>=0)
		{
		  EventSource<GraphClickEvent>::Type::emit(button==GLFW_MOUSE_BUTTON_LEFT?0:1,int(pickPoint_),int(cursor[0]),int(cursor[1]));
		}
	      else
		{
		  EventSource<GraphClickEvent>::Type::emit(button==GLFW_MOUSE_BUTTON_LEFT?0:1,-1,int(cursor[0]),int(cursor[1]));
		}
	    }
	  return true;
	}
      return false;
    }

#pragma GCC diagnostic pop
    
  } /* end namespace gui */
} /* end namespace scigma */
