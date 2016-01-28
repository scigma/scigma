#include <set>
#include <sstream>
#include <iostream>
#include "application.h"
#include "graph.h"
#include "glutil.h"
#include "glcontext.h"
#include "graphtypes.h"

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

     GLuint Graph::programs_[MAX_SHADERS];

     size_t Graph::nIndexedColors_(0);
     GLfloat Graph::indexedColors_[MAX_SHADERS*4];

     GLuint Graph::dummyBuffer_(0);
     size_t Graph::nDummyPoints_(0);

 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wglobal-constructors"
 #pragma clang diagnostic ignored "-Wexit-time-destructors"
     std::map<GLContext*,ExpressionArray> Graph::independentVariables_;
 #pragma clang diagnostic pop


     Graph::Graph(GLWindow* glWindow, std::string identifier,size_t nExpectedPoints, Wave* variableWave, Wave* constantWave,
		  Marker::Type marker, Marker::Type point, GLfloat markerSize, GLfloat pointSize, const GLfloat* color, GLfloat delay):
       glWindow_(glWindow),identifier_(identifier),doubleClickTime_(0.25),lastClickTime_(-1.0),hoverPoint_(-1),variableWave_(variableWave),constantWave_(constantWave),marker_(marker),point_(point),
       markerSize_(markerSize),pointSize_(pointSize),timeOfFirstDraw_(-1),delay_(delay),attributesInvalid_(true),hovering_(false),pointHoverIsActive_(true)
     {
       escapeCount_=ESCAPE_COUNT;
       
       if(!dummyBuffer_)
	 glGenBuffers(1,&dummyBuffer_);

       set_n_points(nExpectedPoints);

       variableWave_->grab(Application::get_instance());
       constantWave_->grab(Application::get_instance());

       if(color)
	 set_color(color);
       else
       {
	 GLfloat col[]={1.0f,1.0f,1.0f,1.0f};
	 set_color(col);
       }
     }

     Graph::~Graph()
     {
       variableWave_->release();
       constantWave_->release();
     }
    
    void Graph::set_marker_style(Marker::Type marker){marker_=marker;}
    Marker::Type Graph::marker_style() const {return marker_;}

    void Graph::set_marker_size(GLfloat size){markerSize_=size<1.0f?1.0f:size;}
    GLfloat Graph::marker_size() const{return markerSize_;}
      
    void Graph::set_point_style(Marker::Type marker){point_=marker;}
    Marker::Type Graph::point_style() const{return point_;}
    
    void Graph::set_point_size(GLfloat size){pointSize_=size<1.0f?1.0f:size;}
    GLfloat Graph::point_size() const{return pointSize_;}
    
    void Graph::set_color(const GLfloat* color)
    {
      for(size_t i(0);i<4;++i)
      color_[i]=color[i];
    }
    const GLfloat* Graph::color() const{return color_;}

    void Graph::set_data(Wave* variables, Wave* constants)
    {
      variableWave_->release();
      constantWave_->release();
      variableWave_=variables;
      constantWave_=constants;
      variableWave_->grab(Application::get_instance());
      constantWave_->grab(Application::get_instance());
    }
    
    void Graph::set_view(const IndexArray& indices)
    {
      indices_=indices;
      attributesInvalid_=true;
    }

    void Graph::set_n_points(size_t n)
    {
      nPoints_=n;
      if(nPoints_>nDummyPoints_)
	{
	  glBindBuffer(GL_ARRAY_BUFFER,dummyBuffer_);
	  glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(GLfloat)*nPoints_),NULL,GL_DYNAMIC_DRAW);
	  glBindBuffer(GL_ARRAY_BUFFER,0);
	  nDummyPoints_=nPoints_;
	}
    }

    void Graph::set_point_hover_mode(bool flag)
    {
      pointHoverIsActive_=flag;
      if(!pointHoverIsActive_)
	hoverPoint_=-1;
    }

#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"

    template<> void Graph::rebuild_individual_shaders<LOKI_TYPELIST_0>(GLContext* glContext,
								       const std::string& vertexShaderHeader,
								       const std::string& fragmentShaderHeader,
								       const ExpressionArray& transformations,
								       const ExpressionArray& attributes,
								       bool useColorMap)
    {}
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
    
    void Graph::rebuild_shaders(GLContext* glContext,
				const ExpressionArray& expressions, 
				const ExpressionArray& independentVariables)
    {
      
      if(independentVariables.size()>MAX_VERTEX_ATTRIBUTES)
	{
	  std::stringstream ss;
	  ss<<"number of independent variables exceeds MAX_VERTEX_ATTRIBUTES(="<<MAX_VERTEX_ATTRIBUTES<<")";
	  throw(ss.str());
	}
      
      independentVariables_[glContext]=independentVariables;
      
      // Prepare GLSL header code for vertex attributes, and outputs/inputs to the fragment shader
#ifdef SCIGMA_USE_OPENGL_3_2
      std::string vertexIn("in");
      std::string vertexOut("out"),fragmentIn("in");
#else
      std::string vertexIn("attribute");
      std::string vertexOut("varying"),fragmentIn("varying");
#endif
      
      std::string vertexShaderHeader;
      
      vertexShaderHeader+=vertexIn+" float dummy_;\n";
      for(size_t j(0),size(independentVariables.size());j<size;++j)
	{
	  vertexShaderHeader+=vertexIn+" float "+independentVariables[j]+";\n";
	}
      vertexShaderHeader+="\n";
      vertexShaderHeader+=vertexOut+" vec2 screenPos_;\n";
      vertexShaderHeader+=vertexOut+" vec4 vertexID_;\n";

      std::string fragmentShaderHeader("uniform int sprite_;\n"
				       "uniform float size_;\n"
				       "uniform int lighter_;\n"
				       "uniform sampler2D sampler_;\n\n");
      
      fragmentShaderHeader+=fragmentIn+" vec2 screenPos_;\n";
      fragmentShaderHeader+=fragmentIn+" vec4 vertexID_;\n\n";

      bool useColorMap(false);      
      if(expressions[C_INDEX].size()!=0) // if the color map is used 
	{
	  useColorMap=true;
	  vertexShaderHeader+=vertexOut+" vec4 rgba_;\n\n";
	  fragmentShaderHeader+=fragmentIn+" vec4 rgba_;\n\n";
	}
      else
	fragmentShaderHeader+="uniform vec4 rgba_;\n\n";
      
      vertexShaderHeader+=vertexOut+" float t_;\n\n";
      fragmentShaderHeader+=fragmentIn+" float t_;\n\n";
      
#ifdef SCIGMA_USE_OPENGL_3_2
      fragmentShaderHeader+="out vec4 color_;\n\n";
#endif
      
      rebuild_individual_shaders<GraphTypes>(glContext,
					     vertexShaderHeader,
					     fragmentShaderHeader,
					     expressions,
					     independentVariables,
					     useColorMap);
    }
    
    void Graph::set_indexed_colors(const GLfloat* colors,size_t nColors)
    {
      nIndexedColors_=nColors<MAX_SHADERS?nColors:MAX_SHADERS;
      for(size_t i(0);i<nIndexedColors_;++i)
	indexedColors_[i]=colors[i];
    }
    
  } /* end namespace gui */
} /* end namespace scigma */
