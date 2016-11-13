#include <iostream>
#include <map>
#include "../dat/wave.hpp"
#include "../common/util.hpp"
#include "graph.hpp"
#include "bundle.hpp"
#include "sheet.hpp"
#include "glwindow.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace scigma::common;
using namespace scigma::gui;
using namespace scigma::dat;

typedef AbstractWave<double> Wave;

extern "C"
{

  std::string scigmaGuiGraphError;
  
  class ScigmaGuiGraphEventMonitor:
    public EventSink<GraphClickEvent>::Type,
    public EventSink<GraphDoubleClickEvent>::Type
  {
  private:
    void (*python_callback_)(const int,int,int,int,int);
  public:
    ScigmaGuiGraphEventMonitor(void (*python_callback)(const int,int,int,int,int)):python_callback_(python_callback){}
    virtual ~ScigmaGuiGraphEventMonitor();
    virtual bool process(GraphDoubleClickEvent event, int point)
    {python_callback_(1,0,point,0,0);return true;}
    virtual bool process(GraphClickEvent event, int button, int point,int x, int y)
    {python_callback_(0,button,point,x,y);return true;}
  };
  ScigmaGuiGraphEventMonitor::~ScigmaGuiGraphEventMonitor()
  {}
  /* this map stores a ScigmaGuiGraphEventMonitor for each Graph when it is created;
     on destruction of the Graph object, these are destroyed as well.
  */
  std::map<Graph*,ScigmaGuiGraphEventMonitor*> scigmaGuiGraphEventMonitorMap;


  /* Wrappers for the Bundle class */
  
  PythonID scigma_gui_create_bundle(PythonID glWindowID,
				    int length, int nRays, int nVars, 
				    PythonID varWaveID, PythonID constWaveID,
				    void(*python_callback)(const int,int,int,int,int))
  {
    PYOBJ(GLWindow,glWindow,glWindowID);
    PYOBJ(Wave,varWave,varWaveID);
    PYOBJ(Wave,constWave,constWaveID);
    
    if(glWindow&&varWave&&constWave)
      {
	Bundle* ptr1=new Bundle(glWindow,
				GLsizei(length),GLsizei(nRays), GLsizei(nVars),
				varWave,constWave);

	ScigmaGuiGraphEventMonitor* ptr2(new ScigmaGuiGraphEventMonitor(python_callback));
	connect<GraphClickEvent>(ptr1,ptr2);
	connect<GraphDoubleClickEvent>(ptr1,ptr2);
	scigmaGuiGraphEventMonitorMap.insert(std::pair<Graph*,ScigmaGuiGraphEventMonitor*>(static_cast<Graph*>(ptr1),ptr2));
	return ptr1->get_python_id();
      }
    else
      return -1;
  }
  
  void scigma_gui_destroy_bundle(PythonID objectID)
    {
      PYOBJ(Bundle,ptr1,objectID);
      if(ptr1)
	{
	  std::map<Graph*,ScigmaGuiGraphEventMonitor*>::iterator i(scigmaGuiGraphEventMonitorMap.find(static_cast<Graph*>(ptr1)));
	  ScigmaGuiGraphEventMonitor* ptr2((*i).second);
	  scigmaGuiGraphEventMonitorMap.erase(i);
	  delete ptr2;
	  delete ptr1;
	}
    }

  void scigma_gui_bundle_set_style(PythonID objectID, int style)
  {PYCLL(Bundle,objectID,set_style(Graph::Style(style)))}
  void scigma_gui_bundle_set_marker_style(PythonID objectID, int marker)
  {PYCLL(Bundle,objectID,set_marker_style(Marker::Type(marker)))}
  void scigma_gui_bundle_set_marker_size(PythonID objectID, GLfloat size)
  {PYCLL(Bundle,objectID,set_marker_size(size))}
  void scigma_gui_bundle_set_point_style(PythonID objectID, int point)
  {PYCLL(Bundle,objectID,set_point_style(Marker::Type(point)))}
  void scigma_gui_bundle_set_point_size(PythonID objectID, GLfloat size)
  {PYCLL(Bundle,objectID,set_point_size(size))}
  void scigma_gui_bundle_set_color(PythonID objectID, const GLfloat* color)
  {PYCLL(Bundle,objectID,set_color(color))}
  void scigma_gui_bundle_set_delay(PythonID objectID, GLfloat size)
  {PYCLL(Bundle,objectID,set_delay(size))}
  void scigma_gui_bundle_replay(PythonID objectID)
  {PYCLL(Bundle,objectID,replay())}
  void scigma_gui_bundle_finalize(PythonID objectID)
  {PYCLL(Bundle,objectID,finalize())}

  void scigma_gui_bundle_test(PythonID windowID, PythonID bundleID)
  {
    PYOBJ(GLWindow,ptr,windowID);
    PYOBJ(Bundle,ptr2,bundleID);
    
    ptr2->set_point_size(7);
    
    GLfloat col[]={1,0,0,1}; 
    ptr2->set_color(col);

    std::vector<int> indices;
    indices.push_back(-1);
    indices.push_back(-2);
    indices.push_back(-3);

    ptr2->set_attributes_for_view(indices);

  
    std::vector<std::string> ind;
    std::vector<std::string> exp;
    
    ind.push_back("u");
    ind.push_back("v");
    ind.push_back("pi");

    
    exp.push_back("cos(2*pi*v)*sqrt(1-pow(u-1,2))");
    exp.push_back("sin(2*pi*v)*sqrt(1-pow(u-1,2))");

    exp.push_back("u-1");
    exp.push_back("");
    exp.push_back("0");


    ptr2->adjust_shaders_for_view(ind,exp,0);

    ptr->gl_context()->add_drawable(ptr2);
    
  }

  const char* scigma_gui_bundle_set_view(PythonID objectID, int nIndices, const int* indices,
					 const char* expressions, const char* independentVariables,
					 double timeStamp)
  {
    PYOBJ(Bundle,ptr,objectID);
    if(!ptr)
      return NULL;
    std::vector<int> vIndices;
    for(size_t i(0);i<size_t(nIndices);++i)
	vIndices.push_back(indices[i]);

    ptr->set_attributes_for_view(vIndices);

    std::vector<std::string> exp;
    append_tokens(expressions,exp,'|');

    if(exp.size()<N_COORDINATES-1||exp.size()>N_COORDINATES)
      {
	scigmaGuiGraphError="incorrect number of expressions for graph shader rebuild";
	return scigmaGuiGraphError.c_str();
      }
    else if(exp.size()==N_COORDINATES-1) // empty color expression
      {
	exp.push_back(exp.back());
	exp[C_INDEX]="";
      }

    std::vector<std::string> var;
    append_tokens(independentVariables,var,'|');
    try
      {
	ptr->adjust_shaders_for_view(var,exp,timeStamp);
      }
    catch(std::string error)
      {
	scigmaGuiGraphError=error;
	return scigmaGuiGraphError.c_str();
      }
    return NULL;
  }


  /* Wrappers for the Sheet class */
  
  PythonID scigma_gui_create_sheet(PythonID glWindowID,
				   PythonID meshID, int nVars, PythonID constWaveID,
				   void(*python_callback)(const int,int,int,int,int))
  {
    PYOBJ(GLWindow,glWindow,glWindowID);
    PYOBJ(Mesh,mesh,meshID);
    PYOBJ(Wave,constWave,constWaveID);
    
    if(glWindow&&mesh&&constWave)
      {
	Sheet* ptr1=new Sheet(glWindow,
			      mesh,GLsizei(nVars),constWave);
	ScigmaGuiGraphEventMonitor* ptr2(new ScigmaGuiGraphEventMonitor(python_callback));
	connect<GraphClickEvent>(ptr1,ptr2);
	connect<GraphDoubleClickEvent>(ptr1,ptr2);
	scigmaGuiGraphEventMonitorMap.insert(std::pair<Graph*,ScigmaGuiGraphEventMonitor*>(static_cast<Graph*>(ptr1),ptr2));
	return ptr1->get_python_id();
      }
    else
      return -1;
  }
  
  void scigma_gui_destroy_sheet(PythonID objectID)
    {
      PYOBJ(Sheet,ptr1,objectID);
      if(ptr1)
	{
	  std::map<Graph*,ScigmaGuiGraphEventMonitor*>::iterator i(scigmaGuiGraphEventMonitorMap.find(static_cast<Graph*>(ptr1)));
	  ScigmaGuiGraphEventMonitor* ptr2((*i).second);
	  scigmaGuiGraphEventMonitorMap.erase(i);
	  delete ptr2;
	  delete ptr1;
	}
    }

  void scigma_gui_sheet_set_style(PythonID objectID, int style)
  {PYCLL(Sheet,objectID,set_style(Graph::Style(style)))}
  void scigma_gui_sheet_set_marker_style(PythonID objectID, int marker)
  {PYCLL(Sheet,objectID,set_marker_style(Marker::Type(marker)))}
  void scigma_gui_sheet_set_marker_size(PythonID objectID, GLfloat size)
  {PYCLL(Sheet,objectID,set_marker_size(size))}
  void scigma_gui_sheet_set_point_style(PythonID objectID, int point)
  {PYCLL(Sheet,objectID,set_point_style(Marker::Type(point)))}
  void scigma_gui_sheet_set_point_size(PythonID objectID, GLfloat size)
  {PYCLL(Sheet,objectID,set_point_size(size))}
  void scigma_gui_sheet_set_color(PythonID objectID, const GLfloat* color)
  {PYCLL(Sheet,objectID,set_color(color))}
  void scigma_gui_sheet_set_delay(PythonID objectID, GLfloat size)
  {PYCLL(Sheet,objectID,set_delay(size))}
  void scigma_gui_sheet_replay(PythonID objectID)
  {PYCLL(Sheet,objectID,replay())}
  void scigma_gui_sheet_finalize(PythonID objectID)
  {PYCLL(Sheet,objectID,finalize())}
  void scigma_gui_sheet_set_light_direction(PythonID objectID, const GLfloat* direction)
  {PYCLL(Sheet,objectID,set_light_direction(direction))}
  void scigma_gui_sheet_set_light_parameters(PythonID objectID, const GLfloat* parameters)
  {PYCLL(Sheet,objectID,set_light_parameters(parameters))}

  const char* scigma_gui_sheet_set_view(PythonID objectID, int nIndices, const int* indices,
					     const char* expressions, const char* independentVariables,
					     double timeStamp)
  {
    PYOBJ(Sheet,ptr,objectID);
    if(!ptr)
      return NULL;

    std::vector<int> vIndices;
    for(size_t i(0);i<size_t(nIndices);++i)
	vIndices.push_back(indices[i]);

    ptr->set_attributes_for_view(vIndices);

    std::vector<std::string> exp;
    append_tokens(expressions,exp,'|');

    if(exp.size()<N_COORDINATES-1||exp.size()>N_COORDINATES)
      {
	scigmaGuiGraphError="incorrect number of expressions for graph shader rebuild";
	return scigmaGuiGraphError.c_str();
      }
    else if(exp.size()==N_COORDINATES-1) // empty color expression
      {
	exp.push_back(exp.back());
	exp[C_INDEX]="";
      }

    std::vector<std::string> var;
    append_tokens(independentVariables,var,'|');
    try
      {
	ptr->adjust_shaders_for_view(var,exp,timeStamp);
      }
    catch(std::string error)
      {
	scigmaGuiGraphError=error;
	return scigmaGuiGraphError.c_str();
      }
    return NULL;
  }

} /* end extern "C" block */

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
