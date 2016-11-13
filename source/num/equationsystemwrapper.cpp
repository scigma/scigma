#include <string>
#include <sstream>
#include "../common/util.hpp"
#include "internalequationsystem.hpp"
#include "externalequationsystem.hpp"

using namespace scigma::common;
using namespace scigma::num;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"

extern "C"
{

  // this is the location of the string returned to Python by functions that return a char*
  std::string scigmaNumEquationSystemPythonResult;
  
  // wrappers for the EquationSystem class
  PythonID scigma_num_create_equation_system()
  {InternalEquationSystem* ptr=new InternalEquationSystem; return ptr->get_python_id();}
  
  typedef void (*PyF)(const double*, double*);
  typedef void (*PyF_p)(const double*, const double*, double*);
  typedef void (*PyF_t)(double, const double*, double*);
  typedef void (*PyF_pt)(double, const double*, const double*, double*);
  
  PythonID scigma_num_create_equation_system_II(const char* variables, PyF f, PyF dfdx, 
						const char* functions, PyF func)
  {
    VecS varnames,funcnames;
    append_tokens(variables,varnames,'|');
    append_tokens(functions,funcnames,'|');
    ExternalEquationSystem* ptr=new ExternalEquationSystem(varnames,f,dfdx,funcnames,func);
    return ptr->get_python_id();
  }
  PythonID scigma_num_create_equation_system_III(const char* variables, const char* parameters, PyF_p f,
						 PyF_p dfdx, PyF_p dfdp, const char* functions, PyF_p func)
  {
    VecS varnames,parnames,funcnames;
    append_tokens(variables,varnames,'|');
    append_tokens(parameters,parnames,'|');
    append_tokens(functions,funcnames,'|');
    ExternalEquationSystem* ptr=new ExternalEquationSystem(varnames,parnames,f,dfdx,dfdp,funcnames,func);
    return ptr->get_python_id();
  }
  PythonID scigma_num_create_equation_system_IV(const char* variables, PyF_t f, PyF_t dfdx, 
						const char* functions, PyF_t func)
  {
    VecS varnames,funcnames;
    append_tokens(variables,varnames,'|');
    append_tokens(functions,funcnames,'|');
    ExternalEquationSystem* ptr=new ExternalEquationSystem(varnames,f,dfdx,funcnames,func);
    return ptr->get_python_id();
  }
  PythonID scigma_num_create_equation_system_V(const char* variables, const char* parameters, PyF_pt f,
					       PyF_pt dfdx, PyF_pt dfdp, const char* functions, PyF_pt func)
  {
    VecS varnames,parnames,funcnames;
    append_tokens(variables,varnames,'|');
    append_tokens(parameters,parnames,'|');
    append_tokens(functions,funcnames,'|');
    F_pt ff(f);
    F_pt df(dfdx);
    F_pt fu(func);
    F_pt dp(dfdp);
    
    double x[]={0,0,1};
    double p[]={10,28,2.66};
    double rhs[3];
    f(0,x,p,rhs);
    ExternalEquationSystem* ptr=new ExternalEquationSystem(varnames,parnames,ff,df,dp,funcnames,fu);
    return ptr->get_python_id();
  }

  void scigma_num_destroy_equation_system(PythonID objectID)
  {PYOBJ(EquationSystem,ptr,objectID);if(ptr){delete ptr;}}
  
  void scigma_num_equation_system_stall(PythonID objectID)
  {PYCLL(EquationSystem,objectID,stall())}
  void scigma_num_equation_system_flush(PythonID objectID)
  {PYCLL(EquationSystem,objectID,flush())}
  
  const char* scigma_num_equation_system_parse(PythonID objectID, const char* line)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	try
	  {
	    scigmaNumEquationSystemPythonResult=ptr->parse(line);
	  }
	catch(std::string error)
	  {
	    scigmaNumEquationSystemPythonResult="error:";
	    scigmaNumEquationSystemPythonResult+=error;
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
       }
    else
      return NULL;
  }
  
  uint64_t scigma_num_equation_system_time_stamp(PythonID objectID)
  {PYOBJ(EquationSystem,ptr,objectID);if(ptr){return ptr->time_stamp();}return 0;}
  
  int scigma_num_equation_system_is_autonomous(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->is_autonomous()?1:0;
    else 
      return 0;
  }
  
  double scigma_num_equation_system_time(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->is_autonomous()?0:ptr->time();
    else 
      return 0.0;
  }
  
  int scigma_num_equation_system_n_variables(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return int(ptr->n_variables());
    return -1;
  }
  
  const char* scigma_num_equation_system_get(PythonID objectID, const char* symbol)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	try
	  {
	    std::stringstream ss;
	    ss<<ptr->get(symbol);
	    scigmaNumEquationSystemPythonResult=ss.str();
	  }
	catch(std::string error)
	  {
	    scigmaNumEquationSystemPythonResult="error:";
	    scigmaNumEquationSystemPythonResult+=error;
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    return NULL;
  }
  
  const char* scigma_num_equation_system_set(PythonID objectID, const char* symbol, double value)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	try
	  {
	    scigmaNumEquationSystemPythonResult="";
	    ptr->set(symbol,value);
	  }
	catch(std::string error)
	  {
	    scigmaNumEquationSystemPythonResult="error:";
	    scigmaNumEquationSystemPythonResult+=error;
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    return NULL;
  }
  
  const char* scigma_num_equation_system_variables(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_variables());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->variable_names());
	    const double* values(ptr->variable_values());
	    std::stringstream ss;
	    for(size_t i(0);i<n;++i)
	      ss<<"|"<<names[i]<<"="<<values[i];
	    scigmaNumEquationSystemPythonResult=ss.str();
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  }
  const char* scigma_num_equation_system_variable_names(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_variables());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->variable_names());
	    scigmaNumEquationSystemPythonResult="";
	    for(size_t i(0);i<n;++i)
	      scigmaNumEquationSystemPythonResult+="|"+names[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  }
  
  const double* scigma_num_equation_system_variable_values(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->variable_values();
    else
      return NULL;
  } 
  
  int scigma_num_equation_system_n_parameters(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return int(ptr->n_parameters());
    return -1;
  }
  const char* scigma_num_equation_system_parameters(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_parameters());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->parameter_names());
	    const double* values(ptr->parameter_values());
	    std::stringstream ss;
	    for(size_t i(0);i<n;++i)
	      ss<<"|"<<names[i]<<"="<<values[i];
	    scigmaNumEquationSystemPythonResult=ss.str();
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  }
  const char* scigma_num_equation_system_parameter_names(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_parameters());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->parameter_names());
	    scigmaNumEquationSystemPythonResult="";
	    for(size_t i(0);i<n;++i)
	      scigmaNumEquationSystemPythonResult+="|"+names[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  } 
  const double* scigma_num_equation_system_parameter_values(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->parameter_values();
    else
      return NULL;
  } 
  int scigma_num_equation_system_n_functions(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return int(ptr->n_functions());
    return -1;
  }
  const char* scigma_num_equation_system_functions(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_functions());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->function_names());
	    const double* values(ptr->function_values());
	    std::stringstream ss;
	    for(size_t i(0);i<n;++i)
	      ss<<"|"<<names[i]<<"="<<values[i];
	    scigmaNumEquationSystemPythonResult=ss.str();
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  }
  const char* scigma_num_equation_system_function_names(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_functions());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->function_names());
	    scigmaNumEquationSystemPythonResult="";
	    for(size_t i(0);i<n;++i)
	      scigmaNumEquationSystemPythonResult+="|"+names[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  } 
  const double* scigma_num_equation_system_function_values(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->function_values();
    else
      return NULL;
  } 
  int scigma_num_equation_system_n_constants(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return int(ptr->n_constants());
    return -1;
  }
  const char* scigma_num_equation_system_constants(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_constants());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->constant_names());
	    const double* values(ptr->constant_values());
	    std::stringstream ss;
	    for(size_t i(0);i<n;++i)
	      ss<<"|"<<names[i]<<"="<<values[i];
	    scigmaNumEquationSystemPythonResult=ss.str();
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  }
  const char* scigma_num_equation_system_constant_names(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_constants());
	if(0==n)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    const std::string* names(ptr->constant_names());
	    scigmaNumEquationSystemPythonResult="";
	    for(size_t i(0);i<n;++i)
	      scigmaNumEquationSystemPythonResult+="|"+names[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  } 
  const double* scigma_num_equation_system_constant_values(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->constant_values();
    else
      return NULL;
  } 
  const char* scigma_num_equation_system_variable_definitions(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_variables());
	const std::string* definitions(ptr->variable_definitions());
	if(0==n||!definitions)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    scigmaNumEquationSystemPythonResult=definitions[0];	
	    for(size_t i(1);i<n;++i)
	      scigmaNumEquationSystemPythonResult+=std::string("|")+definitions[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
  }
  const char* scigma_num_equation_system_function_definitions(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_functions());
	const std::string* definitions(ptr->function_definitions());
	if(0==n||!definitions)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    scigmaNumEquationSystemPythonResult=definitions[0];	
	    for(size_t i(1);i<n;++i)
	      scigmaNumEquationSystemPythonResult+=std::string("|")+definitions[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
      }
  const char* scigma_num_equation_system_constant_definitions(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      {
	size_t n(ptr->n_constants());
	const std::string* definitions(ptr->constant_definitions());
	if(0==n||!definitions)
	  scigmaNumEquationSystemPythonResult="";
	else
	  {
	    scigmaNumEquationSystemPythonResult=definitions[0];	
	    for(size_t i(1);i<n;++i)
	      scigmaNumEquationSystemPythonResult+=std::string("|")+definitions[i];
	  }
	return scigmaNumEquationSystemPythonResult.c_str();
      }
    else
      return NULL;
      }
  
} /* end extern "C" block */

#pragma clang diagnostic pop
