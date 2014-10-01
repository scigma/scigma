#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <chrono>
#include "equationsystem.h"
#include <iostream>

namespace scigma
{
  namespace num
  {
    EquationSystem::EquationSystem():
      PythonObject<EquationSystem>(this),stalled_(0),timeStamp_(time_stamp()),rebuildRequested_(false),isInternal_(true),isAutonomous_(true)
    {
      padding_[0]=0; // to avoid -Wunused-private-field warning
    }
    
    EquationSystem::EquationSystem(VecS variables, F f, F dfdx, VecS functions, F func):
      PythonObject<EquationSystem>(this),stalled_(0),timeStamp_(time_stamp()),rebuildRequested_(false),
      isInternal_(false),isAutonomous_(true),
      varNames_(variables),varValues_(variables.size()), 
      funcNames_(functions),funcValues_(functions.size()),
      f_(f),dfdx_(dfdx),func_(func)
    {
    }
    
    EquationSystem::EquationSystem(VecS variables, VecS parameters, F_p f, F_p dfdx, F_p dfdp, VecS functions, F_p func):
      PythonObject<EquationSystem>(this),stalled_(0),timeStamp_(time_stamp()),rebuildRequested_(false),
      isInternal_(false),isAutonomous_(true),
      varNames_(variables),varValues_(variables.size()),
      parNames_(parameters),parValues_(parameters.size()),
      funcNames_(functions),funcValues_(functions.size()), 
      f_p_(f),dfdx_p_(dfdx),dfdp_p_(dfdp),func_p_(func)
    {
    }
    
    EquationSystem::EquationSystem(VecS variables, F_t f, F_t dfdx, VecS functions, F_t func):
      PythonObject<EquationSystem>(this),stalled_(0),timeStamp_(time_stamp()),rebuildRequested_(false),
      isInternal_(false),isAutonomous_(false),
      varNames_(variables),varValues_(variables.size()),
      funcNames_(functions), funcValues_(functions.size()),
      f_t_(f),dfdx_t_(dfdx),func_t_(func)
    {
      
    }
    
    EquationSystem::EquationSystem(VecS variables, VecS parameters, F_pt f, F_pt dfdx, F_pt dfdp, VecS functions, F_pt func):
      PythonObject<EquationSystem>(this),stalled_(0),timeStamp_(time_stamp()),rebuildRequested_(false),
      isInternal_(false),isAutonomous_(false),
      varNames_(variables),varValues_(variables.size()),
      parNames_(parameters),parValues_(parameters.size()),
      funcNames_(functions),funcValues_(functions.size()),
      f_pt_(f),dfdx_pt_(dfdx),dfdp_pt_(dfdp),func_pt_(func)
    {
      double x[]={0,0,1};
      double p[]={10,28,2.66};
      double rhs[3];
      f(0,x,p,rhs);
    }
    
    
    std::string EquationSystem::parse(std::string expression)
    {
      if(!isInternal_)
	throw(std::string("cannot use parser for external equations"));
      
      expression = trim(expression);
      try
	{
	  std::string result(parse_expression(expression,fSet_));
	  request_rebuild();
	  return result;
	}
      catch(std::string exception)
	{
	  throw(exception);
	}
    }
    
    void EquationSystem::set(const std::string& name, double value)
    {
      if(isInternal_)
	{
	  std::stringstream ss;
	  ss<<name<<"="<<value<<"\n";
	  parse(ss.str());
	}
      else
	{
	  if(name=="t")
	    {
	      t_=value;
	      return;
	    }
	  for(size_t i(0);i<varNames_.size();++i)
	    {
	      if(varNames_[i]==name)
		{
		  varValues_[i]=value;
		  return;
		}
	    }
	  for(size_t i(0);i<parNames_.size();++i)
	    {
	      if(parNames_[i]==name)
		{
		  parValues_[i]=value;
		  return;
		}
	    }
	  throw(name+" is not a variable or parameter\n");
	}
    }
    
    
    std::string EquationSystem::get(const std::string& name)
    {
      std::stringstream ss;
      if(isInternal_)
	{
	  ss<<"$"<<name<<"\n";
	  return parse(ss.str());
	}
      else
	{
	  if(name=="t")
	    {
	      ss<<time();
	      return ss.str();
	    }
	  for(size_t i(0);i<varNames_.size();++i)
	    {
	      if(varNames_[i]==name)
		{
		  ss<<varValues_[i];
		  return ss.str();
		}
	    }
	  for(size_t i(0);i<parNames_.size();++i)
	    {
	      if(parNames_[i]==name)
		{
		  ss<<parValues_[i];
		  return ss.str();
		}
	    }
	  for(size_t i(0);i<funcNames_.size();++i)
	    {
	      if(funcNames_[i]==name)
		{
		  ss<<funcValues_[i];
		  return ss.str();
		}
	    }
	  for(size_t i(0);i<constNames_.size();++i)
	    {
	      if(constNames_[i]==name)
		{
		  ss<<constValues_[i];
		  return ss.str();
		}
	    }
	  throw(name+" is not defined in the dynamical system\n");
	}
    }

    bool EquationSystem::structure_changed(const FunctionSet& varSet, const FunctionSet& parSet, 
					   const FunctionSet& funcSet, const FunctionSet& constSet) const
    {
      if(varSet.size()!=varNames_.size())
	return true;
      if(parSet.size()!=parNames_.size())
	return true;
      if(funcSet.size()!=funcNames_.size())
	return true;
      if(constSet.size()!=constNames_.size())
	return true;

      for(size_t i(0),nVar(varSet.size());i<nVar;++i)
	if(varSet[i].name!=varNames_[i])
	  return true;

      for(size_t i(0),nPar(parSet.size());i<nPar;++i)
	if(parSet[i].name!=parNames_[i])
	  return true;

      for(size_t i(0),nFunc(funcSet.size());i<nFunc;++i)
	if(funcSet[i].name!=funcNames_[i])
	  return true;

      for(size_t i(0),nConst(constSet.size());i<nConst;++i)
	if(constSet[i].name!=constNames_[i])
	  return true;

      return false;
    }

    uint64_t EquationSystem::time_stamp() const
    {
      return timeStamp_;
    }
    
    void EquationSystem::clear()
    {
      if(isInternal_)
	{
	  fSet_.clear();
	  request_rebuild();
	}
    }
    
    bool EquationSystem::is_internal() const {return isInternal_;}
    bool EquationSystem::is_autonomous() const {return isAutonomous_;}
    
    size_t EquationSystem::n_variables() const {return varNames_.size();}
    size_t EquationSystem::n_parameters() const {return parNames_.size();}
    size_t EquationSystem::n_functions() const {return funcNames_.size();}
    size_t EquationSystem::n_constants() const {return constNames_.size();}
    
    const double& EquationSystem::time() const {return t_;}
    
    const double* EquationSystem::variable_values() const {return varValues_.size()==0?NULL:&varValues_[0];}
    const double* EquationSystem::parameter_values() const {return parValues_.size()==0?NULL:&parValues_[0];}
    const double* EquationSystem::function_values() const {return funcValues_.size()==0?NULL:&funcValues_[0];}
    const double* EquationSystem::constant_values() const {return constValues_.size()==0?NULL:&constValues_[0];}
    
    const std::string* EquationSystem::variable_names() const {return varNames_.size()==0?NULL:&varNames_[0];}
    const std::string* EquationSystem::parameter_names() const {return parNames_.size()==0?NULL:&parNames_[0];}
    const std::string* EquationSystem::function_names() const {return funcNames_.size()==0?NULL:&funcNames_[0];}
    const std::string* EquationSystem::constant_names() const {return constNames_.size()==0?NULL:&constNames_[0];}
    
    const std::string* EquationSystem::variable_definitions() const {return varDefs_.size()==0?NULL:&varDefs_[0];}
    const std::string* EquationSystem::function_definitions() const {return funcDefs_.size()==0?NULL:&funcDefs_[0];}
    const std::string* EquationSystem::constant_definitions() const {return constDefs_.size()==0?NULL:&constDefs_[0];}
    
    void EquationSystem::request_rebuild()
    {
      if(stalled_)
	{
	  rebuildRequested_=true;
	}
      else
	{ 
	  rebuild();
	  rebuildRequested_=false;
	}
    }
    
    void EquationSystem::stall()
    {
      ++stalled_;
    }
    
    void EquationSystem::flush()
    {
      --stalled_;
      if(!stalled_)
	{
	  stalled_=false;
	  request_rebuild();
	}
    }
    
    std::string EquationSystem::trim(std::string s) const
    {   
      if(s.length()==0)
	return "";
      size_t val = 0;
      for (size_t cur = 0; cur < s.length(); ++cur)
	{
	  if((s[cur] != ' ')&&(s[cur] != '\t')&&(s[cur]!='\r'))
	    {
	      if(val>0)// replace "**" by "^"
		{
		  if(s[val-1]=='*'&&s[cur]=='*')
		    {
		      s[val-1]='^';
		      continue;
		    }
		}
	      s[val] = s[cur];
	      ++val;
	    }
	}
      if(!val)
	return "";
      s.resize(val);
      if(s.at(val-1)!='\n')
	s+="\n";
      return s;
    }
    
    void EquationSystem::clear_vectors()
    {
      varFuncs_.clear();varNames_.clear();varDefs_.clear();rhsFuncs_.clear();varValues_.clear();
      parFuncs_.clear();parNames_.clear();parValues_.clear();
      funcFuncs_.clear();funcNames_.clear();funcDefs_.clear();funcValues_.clear();
      constFuncs_.clear();constNames_.clear();constDefs_.clear();constValues_.clear();
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
    
    bool compare_function_info_by_type_and_name(const FunctionInfo& xInfo, const FunctionInfo& yInfo)
    {
      if(';'==xInfo.definition[0]&&';'==yInfo.definition[0])
	return xInfo.name<yInfo.name;
      else if (';'!=xInfo.definition[0]&&';'!=yInfo.definition[0])
	return xInfo.name<yInfo.name;
      else 
	return ';'==yInfo.definition[0];
    }
#pragma clang diagnostic pop   
    
    void EquationSystem::prepare_variables_and_parameters
    (FunctionSet& source, FunctionSet& varSet, FunctionSet& rhsSet, FunctionSet& parSet) const
    {
      FunctionSet::iterator i = source.begin();
      while(i!=source.end())
	{
	  if(i->name[i->name.length()-1]=='\'') // time derivative 
	    {
	      rhsSet.push_back(*i);
	      std::string varname=i->name;size_t l(varname.length());varname[l-1]=0;
	      source.erase(i);
	      FunctionSet::iterator fInfo=get_function_from_set(varname.c_str(),source);
	      if(fInfo!=source.end())
		{
		  varSet.push_back(*fInfo);
		  source.erase(fInfo);
		}
	    }
	  else if(i->f.is_constant())  // variable or parameter 
	    {
	      FunctionSet::iterator fDerivativeInfo=get_time_derivative_from_set(i->name,source);
	      if(fDerivativeInfo!=source.end()) // ode variable
		varSet.push_back(*i);
	      else // parameter
		parSet.push_back(*i);
	      i=source.erase(i);
	    }
	  else // functions depending on other functions
	    ++i;
	}
      std::sort(varSet.begin(),varSet.end(),compare_function_info_by_type_and_name);
      std::sort(rhsSet.begin(),rhsSet.end(),compare_function_info_by_type_and_name);
      std::sort(parSet.begin(),parSet.end(),compare_function_info_by_type_and_name);
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
    bool f_depends_on_function_in_set(Function f, const FunctionSet& fSet)
    {
      FunctionSet::const_iterator j=fSet.begin();
      while(j!=fSet.end())
	{
	  if(f.is_function_of(j->f))
	    break;
	  ++j;
	}
      return (j!=fSet.end());
    }
#pragma clang diagnostic pop
    
    void EquationSystem::prepare_functions_and_constants
    (FunctionSet& source, FunctionSet& funcSet, FunctionSet& constSet, FunctionSet& varSet, Function tFunc) const
    {
      FunctionSet::iterator i = source.begin();
      while(i!=source.end())
	{
	  if(f_depends_on_function_in_set(i->f,varSet)||
	     i->f.is_function_of(tFunc)) // function that varies with time
	    funcSet.push_back(*i);
	  else                // user defined function depending on parameters only
	    constSet.push_back(*i);
	  i=source.erase(i);
	}
      std::sort(funcSet.begin(),funcSet.end(),compare_function_info_by_type_and_name);
      std::sort(constSet.begin(),constSet.end(),compare_function_info_by_type_and_name);
    }
    
    void EquationSystem::build_variables(const FunctionSet& varSet, const FunctionSet& rhsSet)
    {
      size_t nVar(varSet.size());
      varFuncs_.resize(nVar);
      varNames_.resize(nVar);
      varValues_.resize(nVar);
      varDefs_.resize(nVar);
      rhsFuncs_.resize(nVar);
      for(size_t i(0);i<nVar;++i)
	{
	  varFuncs_[i]=varSet[i].f;
	  varNames_[i]=varSet[i].name;
	  varValues_[i]=varSet[i].f.evaluate();
	  varDefs_[i]=rhsSet[i].definition;
	  rhsFuncs_[i]=rhsSet[i].f;
	}
    }
    
    void EquationSystem::build_parameters(const FunctionSet& parSet)      
    {
      size_t nPar(parSet.size());
      parFuncs_.resize(nPar);
      parNames_.resize(nPar);
      parValues_.resize(nPar);
      for(size_t i(0);i<nPar;++i)
	{
	  parFuncs_[i]=parSet[i].f;
	  parNames_[i]=parSet[i].name;
	  parValues_[i]=parSet[i].f.evaluate();
	}
    }
    
    void EquationSystem::build_functions_and_constants(const FunctionSet& info, VecF& funcs, VecS& names, VecS& defs, VecD& values)
    {
      size_t n(info.size());
      funcs.resize(n);
      names.resize(n);
      defs.resize(n);
      values.resize(n);
      for(size_t i(0);i<n;++i)
	{
	  funcs[i]=info[i].f;
	  names[i]=info[i].name;
	  defs[i]=info[i].definition;
	  values[i]=info[i].f.evaluate();
	}
    }
    
    void EquationSystem::rebuild()
    {
      if(!isInternal_)
	{
	  // update values of additional user defined functions if necessary
	  if(func_)
	    func_(&varValues_[0],&funcValues_[0]);
	  else if(func_p_)
	    func_p_(&varValues_[0],&parValues_[0],&funcValues_[0]);
	  else if(func_t_)
	    func_t_(t_,&varValues_[0],&funcValues_[0]);
	  else if(func_pt_)
	    func_pt_(t_,&varValues_[0],&parValues_[0],&funcValues_[0]);
	  return;
	}
      
      FunctionSet tempSet(fSet_);
      
      FunctionSet::iterator fTime(get_function_from_set("t",tempSet));
      if(fTime!=tempSet.end())
	{
	  tFunc_=fTime->f;
	  tempSet.erase(fTime); 
	}
      else tFunc_=0;
      t_=tFunc_.evaluate();
      
      FunctionSet varSet,rhsSet,parSet,funcSet,constSet;
      
      prepare_variables_and_parameters(tempSet,varSet,rhsSet,parSet);
      prepare_functions_and_constants(tempSet,funcSet,constSet,varSet,tFunc_);
      
      if(structure_changed(varSet,parSet,funcSet,constSet))
	{
	  using namespace std::chrono;
	  nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
	  timeStamp_= uint64_t (ns.count());
	}

      clear_vectors();

      build_variables(varSet,rhsSet);
      build_parameters(parSet);
      build_functions_and_constants(funcSet,funcFuncs_,funcNames_,funcDefs_,funcValues_);
      build_functions_and_constants(constSet,constFuncs_,constNames_,constDefs_,constValues_);
      
      //      sort_function_set(fSet_);
      
      isAutonomous_=true;
      for(size_t i(0), nVar(rhsFuncs_.size());i!=nVar;++i)
	if(rhsFuncs_[i].is_function_of(tFunc_))
	  isAutonomous_=false;
      
      
      
    }
    
    void EquationSystem::detach(Function& t, VecF& x, VecF& rhs, VecF& func) const
    {
      FunctionSet fSetClone;
      //clone_sorted_function_set(fSet_,fSetClone);
      clone_function_set(fSet_,fSetClone);
      
      FunctionSet::iterator fTime(get_function_from_set("t",fSetClone));
      if(fTime!=fSetClone.end())
	t=fTime->f;
      else
	t=0;
      size_t nVar(n_variables());
      for(size_t i(0);i<nVar;++i)
	x.push_back(get_function_from_set(varNames_[i],fSetClone)->f);
      for(size_t i(0);i<nVar;++i)
	rhs.push_back(get_function_from_set(varNames_[i]+"'",fSetClone)->f);
      for(size_t i(0),nFunc(n_functions());i<nFunc;++i)
	func.push_back(get_function_from_set(funcNames_[i],fSetClone)->f);
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    
    F EquationSystem::f() const
    {
      auto& f_p(f_p_); auto& f_t(f_t_); auto& f_pt(f_pt_);
      auto& parValues(parValues_); double t(t_);
      if(f_) return f_;
      else if(f_p_) return [f_p,parValues](const double* x, double* rhs){f_p(x,&parValues[0],rhs);};
      else if(f_t_) return [f_t,t](const double* x, double* rhs){f_t(t,x,rhs);};
      else if(f_pt_) return [f_pt,parValues,t](const double* x, double* rhs){f_pt(t,x,&parValues[0],rhs);};
      else return NULL;
    }
    
    F EquationSystem::dfdx() const
    {
      auto& dfdx_p(dfdx_p_); auto& dfdx_t(dfdx_t_); auto& dfdx_pt(dfdx_pt_);
      auto& parValues(parValues_); double t(t_);
      if(dfdx_) return dfdx_; 
      else if(dfdx_p_) return [dfdx_p,parValues](const double* x, double* dfdxval){dfdx_p(x,&parValues[0],dfdxval);};
      else if(dfdx_t_) return [dfdx_t,t](const double* x, double* dfdxval){dfdx_t(t,x,dfdxval);};
      else if(dfdx_pt_) return [dfdx_pt,parValues,t](const double* x, double* dfdxval){dfdx_pt(t,x,&parValues[0],dfdxval);};
      else return NULL;
    }
    
    F EquationSystem::func() const
    {
      auto& func_p(func_p_); auto& func_t(func_t_); auto& func_pt(func_pt_);
      auto& parValues(parValues_); double t(t_);
      if(func_) return func_;
      else if(func_p_) return [func_p,parValues](const double* x, double* funcval){func_p(x,&parValues[0],funcval);};
      else if(func_t_) return [func_t,t](const double* x, double* funcval){func_t(t,x,funcval);};
      else if(func_pt_) return [func_pt,parValues,t](const double* x, double* funcval){func_pt(t,x,&parValues[0],funcval);};
      else return NULL;
    }
    
    F_p EquationSystem::f_p() const
    {
      auto& f(f_); auto& f_t(f_t_); auto& f_pt(f_pt_);
      double t(t_);
      if(f_) return [f](const double*x, const double* p, double* rhs){f(x,rhs);};
      else if(f_p_) return f_p_;
      else if(f_t_) return [f_t,t](const double* x, const double* p, double* rhs){f_t(t,x,rhs);};
      else if(f_pt_) return [f_pt,t](const double* x, const double* p, double* rhs){f_pt(t,x,p,rhs);};
      else return NULL;
    }
    
    F_p EquationSystem::dfdx_p() const
    { 
      auto& dfdx(dfdx_); auto& dfdx_t(dfdx_t_); auto& dfdx_pt(dfdx_pt_);
      double t(t_);
      if(dfdx_) return [dfdx](const double*x, const double* p, double* dfdxval){dfdx(x,dfdxval);};
      else if(dfdx_p_) return dfdx_p_;
      else if(dfdx_t_) return [dfdx_t,t](const double* x, const double* p, double* dfdxval){dfdx_t(t,x,dfdxval);};
      else if(dfdx_pt_) return [dfdx_pt,t](const double* x, const double* p, double* dfdxval){dfdx_pt(t,x,p,dfdxval);};
      else return NULL;
    }
    
    F_p EquationSystem::dfdp_p() const
    {
      auto& dfdp_pt(dfdp_pt_); double t(t_);
      if(dfdp_p_) return dfdp_p_;
      else if(dfdp_pt_) return [dfdp_pt,t](const double* x, const double* p, double* dfdpval){dfdp_pt(t,x,p,dfdpval);};
      else return NULL;
    }
    
    F_p EquationSystem::func_p() const
    {
      auto& func(func_); auto& func_t(func_t_); auto& func_pt(func_pt_);
      double t(t_);
      if(func_) return [func](const double*x, const double* p, double* funcval){func(x,funcval);};
      else if(func_p_) return func_p_;
      else if(func_t_) return [func_t,t](const double* x, const double* p, double* funcval){func_t(t,x,funcval);};
      else if(func_pt_) return [func_pt,t](const double* x, const double* p, double* funcval){func_pt(t,x,p,funcval);};
      else return NULL;
    }
    
    F_t EquationSystem::f_t() const
    {
      auto& f(f_); auto& f_p(f_p_); auto& f_pt(f_pt_);
      auto& parValues(parValues_);
      if(f_) return [f](double t, const double* x, double* rhs){f(x,rhs);};
      else if(f_p_) return [f_p,parValues](double t, const double* x, double* rhs){f_p(x,&parValues[0],rhs);};
      else if(f_t_) return f_t_;
      else if(f_pt_) return [f_pt,parValues](double t, const double* x, double* rhs){f_pt(t,x,&parValues[0],rhs);};
      else return NULL;
    }
    
    F_t EquationSystem::dfdx_t() const
    {
      auto& dfdx(dfdx_); auto& dfdx_p(dfdx_p_); auto& dfdx_pt(dfdx_pt_);
      auto& parValues(parValues_);
      if(dfdx_) return [dfdx](double t, const double* x, double* dfdxval){dfdx(x,dfdxval);};
      else if(dfdx_p_) return [dfdx_p,parValues](double t, const double* x, double* dfdxval){dfdx_p(x,&parValues[0],dfdxval);};
      else if(dfdx_t_) return dfdx_t_;
      else if(dfdx_pt_) return [dfdx_pt,parValues](double t, const double* x, double* dfdxval){dfdx_pt(t,x,&parValues[0],dfdxval);};
      else return NULL;
    }

    F_t EquationSystem::func_t() const
    {
      auto& func(func_); auto& func_p(func_p_); auto& func_pt(func_pt_);
      auto& parValues(parValues_);
      if(func_) return [func](double t, const double* x, double* funcval){func(x,funcval);};
      else if(func_p_) return [func_p,parValues](double t, const double* x, double* funcval){func_p(x,&parValues[0],funcval);};
      else if(func_t_) return func_t_;
      else if(func_pt_) return [func_pt,parValues](double t, const double* x, double* funcval){func_pt(t,x,&parValues[0],funcval);};
      else return NULL;
    }

    F_pt EquationSystem::f_pt() const
    {
      auto& f(f_); auto& f_p(f_p_); auto& f_t(f_t_);
      if(f_) return [f] (double t, const double* x, const double* p, double* rhs){f(x,rhs);};
      else if(f_p_) return [f_p] (double t, const double* x, const double* p, double* rhs){f_p(x,p,rhs);};
      else if(f_t_) return [f_t] (double t, const double* x, const double* p, double* rhs){f_t(t,x,rhs);};
      else if(f_pt_) return f_pt_;
      else return NULL;
    }

    F_pt EquationSystem::dfdx_pt() const
    {
      auto& dfdx(dfdx_); auto& dfdx_p(dfdx_p_); auto& dfdx_t(dfdx_t_);
      if(dfdx_) return [dfdx] (double t, const double* x, const double* p, double* dfdxval){dfdx(x,dfdxval);};
      else if(dfdx_p_) return [dfdx_p] (double t, const double* x, const double* p, double* dfdxval){dfdx_p(x,p,dfdxval);};
      else if(dfdx_t_) return [dfdx_t] (double t, const double* x, const double* p, double* dfdxval){dfdx_t(t,x,dfdxval);};
      else if(dfdx_pt_) return dfdx_pt_;
      else return NULL;
    }

    F_pt EquationSystem::dfdp_pt() const
    {
      auto& dfdp_p(dfdp_p_);
      if(dfdp_p_) return [dfdp_p] (double t, const double* x, const double* p, double* dfdpval){dfdp_p(x,p,dfdpval);};
      else if(dfdp_pt_) return dfdp_pt_;
      else return NULL;
    }

    F_pt EquationSystem::func_pt() const
    {
      auto& func(func_); auto& func_p(func_p_); auto& func_t(func_t_);
      if(func_) return [func] (double t, const double* x, const double* p, double* funcval){func(x,funcval);};
      else if(func_p_) return [func_p] (double t, const double* x, const double* p, double* funcval){func_p(x,p,funcval);};
      else if(func_t_) return [func_t] (double t, const double* x, const double* p, double* funcval){func_t(t,x,funcval);};
      else if(func_pt_) return func_pt_;
      else return NULL;
    }

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

  } /* end namespace num */
} /* end namespace scigma */
