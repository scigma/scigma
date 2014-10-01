#ifndef __SCIGMA_GUI_GRAPH_H__
#define __SCIGMA_GUI_GRAPH_H__

#include <vector>
#include <map>
#include <string>
#include <Typelist.h>
#include "../dat/wave.h"
#include "definitions.h"
#include "marker.h"

namespace scigma
{
  namespace gui
  {
    class GLContext;
    class GLWindow;
    using dat::Wave;

    typedef std::vector<int> IndexArray;
    typedef std::vector<GLuint>AttributeArray;
    typedef std::vector<std::string> ExpressionArray;
    
    class Graph
    {
    public:

      void set_marker_style(Marker::Type marker);
      Marker::Type marker_style() const;

      void set_marker_size(GLfloat size);
      GLfloat marker_size() const;
      
      void set_point_style(Marker::Type point);
      Marker::Type point_style() const;

      void set_point_size(GLfloat size);
      GLfloat point_size() const;

      void set_color(const GLfloat* color);
      const GLfloat* color() const;

      void set_data(Wave* variables, Wave* constants);

      void set_view(const IndexArray& indices);

      void set_n_points(size_t n);

      void set_point_hover_mode(bool flag);
      
      static void rebuild_shaders(GLContext* glContext, 
				  const ExpressionArray& expressions,
				  const ExpressionArray& independentVariables);
      static void set_indexed_colors(const GLfloat* colors,size_t nColors);

    protected:
      Graph(GLWindow* glWindow,std::string identifier, size_t nExpectedPoints, Wave* variableWave, Wave* constantWave,
	    Marker::Type marker, Marker::Type point, GLfloat markerSize, GLfloat pointSize, const GLfloat* color, GLfloat delay);
      ~Graph();
     
      /* for colors, an isoluminant color map that works both on white
	 and black background is hardcoded here
      */
      static const char* colorMapFunction_;

      GLWindow* glWindow_;
      std::string identifier_;

      double doubleClickTime_;
      double lastClickTime_;

      size_t nPoints_;
      long hoverPoint_;

      Wave* variableWave_;
      Wave* constantWave_;

      IndexArray indices_;
      
      Marker::Type marker_;
      Marker::Type point_;
      GLfloat markerSize_;
      GLfloat pointSize_;
      GLfloat color_[4];
            
      GLfloat timeOfFirstDraw_;
      GLfloat delay_;     

      bool attributesInvalid_;
      bool hovering_;
      bool pointHoverIsActive_;
      char padding_[5];

      // maximum number of shaders used by a child class of graph
      static const size_t MAX_SHADERS=32;
      // temporary storage for shader programs, shared by all Graphs
      static GLuint programs_[MAX_SHADERS];
      // storage of the indexed colors, also shared
      static size_t nIndexedColors_;
      static GLfloat indexedColors_[MAX_SHADERS*4];

      static std::map<GLContext*,ExpressionArray> independentVariables_;

      static GLuint dummyBuffer_;
      static size_t nDummyPoints_;

    private:
      Graph(const Graph&);
      Graph& operator=(const Graph&);

      template<class TypeList> static void rebuild_individual_shaders(GLContext* glContext,
								      const std::string& vertexShaderHeader,
								      const std::string& fragmentShaderHeader,
								      const ExpressionArray& transformations,
								      const ExpressionArray& attributes,
								      bool useColorMap)
      {
	TypeList::Head::rebuild_shader(glContext,vertexShaderHeader,
				       fragmentShaderHeader,transformations,attributes,useColorMap);
	  
	rebuild_individual_shaders<typename TypeList::Tail>(glContext,vertexShaderHeader,
							    fragmentShaderHeader,transformations,
							    attributes,useColorMap);
      }

      /* Open GL guarantees 16 vec4 vertex attributes. Graphs may use 14 of them 
	 for input of variable values. The other two are reserved for other uses
	 (like normals in Surface).
      */
      static const size_t MAX_VERTEX_ATTRIBUTES=14;

    };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* __SCIGMA_GUI_GRAPH_H__ */
