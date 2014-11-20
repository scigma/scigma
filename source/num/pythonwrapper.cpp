#include <map>
#include <string>
#include <sstream>
#include <iostream>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include <tinythread.h>
#pragma clang diagnostic pop
#include "../util.h"
#include "definitions.h"
#include "equationsystem.h"
#include "functionwrappers.h"
#include "odessa.h"
#include "stepper.h"
#include "mapmanifoldstepper.h"
#include "newton.h"
#include "segment.h"
#include "util.h"

using namespace scigma;
using namespace scigma::num;
using dat::Wave;

/* Macros to retrieve object pointers from PythonIDs
   and call member functions of these objects.
*/
#define PYOBJ(cls,name,pyID) cls* name(PythonObject<cls>::get_pointer(pyID))
#define PYCLL(cls,pyID,func) PYOBJ(cls,ptr,pyID);if(ptr){ptr->func;}

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
  {EquationSystem* ptr=new EquationSystem; return ptr->get_python_id();}

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
    EquationSystem* ptr=new EquationSystem(varnames,f,dfdx,funcnames,func);
    return ptr->get_python_id();
  }
  PythonID scigma_num_create_equation_system_III(const char* variables, const char* parameters, PyF_p f,
						 PyF_p dfdx, PyF_p dfdp, const char* functions, PyF_p func)
  {
    VecS varnames,parnames,funcnames;
    append_tokens(variables,varnames,'|');
    append_tokens(parameters,parnames,'|');
    append_tokens(functions,funcnames,'|');
    EquationSystem* ptr=new EquationSystem(varnames,parnames,f,dfdx,dfdp,funcnames,func);
    return ptr->get_python_id();
  }
  PythonID scigma_num_create_equation_system_IV(const char* variables, PyF_t f, PyF_t dfdx, 
						const char* functions, PyF_t func)
  {
    VecS varnames,funcnames;
    append_tokens(variables,varnames,'|');
    append_tokens(functions,funcnames,'|');
    EquationSystem* ptr=new EquationSystem(varnames,f,dfdx,funcnames,func);
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
    EquationSystem* ptr=new EquationSystem(varnames,parnames,ff,df,dp,funcnames,fu);
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

  int scigma_num_equation_system_is_internal(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->is_internal()?1:0;
    else 
      return 0;
  }
  
  double scigma_num_equation_system_time(PythonID objectID)
  {
    PYOBJ(EquationSystem,ptr,objectID);
    if(ptr)
      return ptr->time();
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
	    scigmaNumEquationSystemPythonResult=ptr->get(symbol);
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

  void* scigma_num_plot(const char* identifier, PythonID equationSystemID,PythonID logID,
			PythonID varyingWaveID, int mode, int steps, double period, int nPeriod, 
			bool showAllIterates, double dt, 
			double maxtime, int secvar, int secdir, double secval, double tol,
			bool extJac, bool stiff, double aTol, double rTol, int maxIter, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return NULL;
    PYOBJ(Log,log,logID);if(!log)return NULL;
    PYOBJ(Wave,wave,varyingWaveID);if(!wave)return NULL;

    if(steps<0)
      {
	steps=-steps;
	dt=-dt;
	period=-period;
      }

    Stepper* stepper(NULL);
      
    Mode m((Mode(mode)));
    switch(m)
      {
      case MAP:
	stepper=create_map_stepper(*eqsys,false);
	break;
      case ODE:
	stepper=create_ode_stepper(*eqsys,false,dt,extJac,stiff,aTol,rTol,size_t(maxIter));
	break;
      case STROBE:
	stepper=create_ode_stepper(*eqsys,false,period,extJac,stiff,aTol,rTol,size_t(maxIter));
	break;
      case POINCARE:
	stepper=create_poincare_stepper(*eqsys,false,maxtime,dt,secvar,secdir,secval,tol,extJac,stiff,aTol,rTol,size_t(maxIter));
      }
    
    Task* task(create_stepping_task(identifier,log,stepper,size_t(steps),size_t(m==ODE?1:nPeriod),showAllIterates?1:0,wave));
    if(noThread)
      {
	(*task)();
	delete task;
	return NULL;
      }
    else
      {
	return reinterpret_cast<void*>(new tthread::thread(run_task,reinterpret_cast<void*>(task)));
      }

  }

  void* scigma_num_guess(const char* identifier, PythonID equationSystemID,PythonID logID,
			 PythonID varyingWaveID, PythonID evWaveID,int mode, double period, int nPeriod, 
			 bool showAllIterates, double dt, 
			 double maxtime, int secvar, int secdir, double secval, double tol,
			 bool extJac, bool stiff, double aTol, double rTol, int maxIter)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return NULL;
    PYOBJ(Log,log,logID);if(!log)return NULL;
    PYOBJ(Wave,varyingWave,varyingWaveID);if(!varyingWave)return NULL;
    PYOBJ(Wave,evWave,evWaveID);if(!evWave)return NULL;
    Mode m((Mode(mode)));
    
    size_t nVar(eqsys->n_variables());
    size_t nFunc(eqsys->n_functions());

    Newton::F* f(NULL);
    std::function<void(const double*, double*)>* ff(NULL);

    Stepper* stepper(NULL);
    
    Task* task(NULL);

    switch(m)
      {
      case MAP:
	stepper=create_map_stepper(*eqsys,true);
	task=create_guessing_task(identifier,log,stepper,varyingWave,evWave,tol,size_t(nPeriod),showAllIterates?1:0,-1);
	/*	f=create_map_newton_function(*eqsys,size_t(nPeriod),extJac);
	ff=create_additional_function_evaluator(*eqsys);
	task=create_guessing_task(identifier,log,nVar,nFunc,f,ff,varyingWave,evWave,extJac,tol,true);*/
	break;
      case ODE:
	if(!eqsys->is_autonomous())
	  {
	    std::string message(APPLICATION_ERROR_PREFIX);
	    message+="cannot use Newton on non-autonomous dynamical system\n";
	    log->push(message);
	    return NULL;
	  }
	f=create_ode_newton_function(*eqsys,extJac);
	ff=create_additional_function_evaluator(*eqsys);
	task=create_guessing_task(identifier,log,nVar,nFunc,f,ff,varyingWave,evWave,extJac,tol,false);
	break;
      case STROBE:
	stepper=create_ode_stepper(*eqsys,true,period,extJac,stiff,aTol,rTol,size_t(maxIter));
	task=create_guessing_task(identifier,log,stepper,varyingWave,evWave,tol,size_t(nPeriod),showAllIterates?1:0,-1);
	break;
      case POINCARE:
	stepper=create_poincare_stepper(*eqsys,true,maxtime,dt,secvar,secdir,secval,tol,extJac,stiff,aTol,rTol,size_t(maxIter));
	task=create_guessing_task(identifier,log,stepper,varyingWave,evWave,tol,size_t(nPeriod),showAllIterates?1:0,long(secvar));
      }
    (*task)();
    delete task;
    return NULL;
    //    return reinterpret_cast<void*>(new tthread::thread(run_task,reinterpret_cast<void*>(task)));
  }

  void* scigma_num_map_manifold(const char* identifier, PythonID equationSystemID,PythonID logID,
				PythonID varyingWaveID, double eval, double eps,
				int* segmentID, double ds, double alpha, int mode,
				int steps, double period, int nPeriod, bool showAllIterates, double dt, 
				double maxtime, int secvar, int secdir, double secval, double tol,
				bool extJac, bool stiff, double aTol, double rTol, int maxIter, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return NULL;
    PYOBJ(Log,log,logID);if(!log)return NULL;
    PYOBJ(Wave,wave,varyingWaveID);if(!wave)return NULL;

    Stepper* stepper(NULL);

    if(steps<0)
      {
	steps=-steps;
	dt=-dt;
	period=-period;
      }

    Mode m((Mode(mode)));
    switch(m)
      {
      case MAP:
	stepper=create_map_stepper(*eqsys,false);
	break;
      case ODE:
	{
	  std::string message(APPLICATION_ERROR_PREFIX);
	  message+="calling map manifold in ODE mode -> that shouldn't be possible\n";
	  log->push(message);
	  return NULL;
	}
      case STROBE:
	stepper=create_ode_stepper(*eqsys,false,period,extJac,stiff,aTol,rTol,size_t(maxIter));
	break;
      case POINCARE:
	stepper=create_poincare_stepper(*eqsys,false,maxtime,dt,secvar,secdir,secval,tol,extJac,stiff,aTol,rTol,size_t(maxIter));
      }

    PYOBJ(Segment,segment,*segmentID);
    if(!segment) // create a new segment, and store its PythonID in segmentID 
      {
	segment=new Segment(stepper, size_t(nPeriod),eval,wave,eps>0?eps:-eps,1e-12);
	*segmentID=segment->get_python_id();
      }

    Stepper* ManifoldStepper(new MapManifoldStepper(stepper,segment,ds,1e-12,alpha,size_t(nPeriod)));
    
    Task* task(create_stepping_task(identifier,log,ManifoldStepper,size_t(steps),size_t(nPeriod),showAllIterates?1:0,wave));

    if(noThread)
      {
	(*task)();
	delete task;
	return NULL;
      }
    else
      {
	return reinterpret_cast<void*>(new tthread::thread(run_task,reinterpret_cast<void*>(task)));
      }
    }


  void scigma_num_destroy_thread(void* thread)
  {
    if(thread)
      {
	tthread::thread* t(reinterpret_cast<tthread::thread*>(thread));
	t->join();
	delete t;
      }
  }
  
} /* end extern "C" block */

#undef PYOBJ
#undef PYCLL

#pragma clang diagnostic pop
