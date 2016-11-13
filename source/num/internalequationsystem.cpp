#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <chrono>
#include "internalequationsystem.hpp"
#include <iostream>

namespace scigma
{
  namespace num
  {
    InternalEquationSystem::InternalEquationSystem():
      stalled_(0),timeStamp_(time_stamp()),rebuildRequested_(false),isAutonomous_(true)
    {
    }
    
    std::string InternalEquationSystem::parse(std::string expression)
    {
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
    
    void InternalEquationSystem::set(const std::string& name, double value)
    {
      if(name=="t"&&!is_autonomous())
	{tFunc_.set_value(value);request_rebuild(true);return;}
      for(size_t i(0),size(varNames_.size());i!=size;++i)
	if(varNames_[i]==name)
	  {varFuncs_[i].set_value(value);request_rebuild(true);return;}
      for(size_t i(0),size(parNames_.size());i!=size;++i)
	if(parNames_[i]==name)
	  {parFuncs_[i].set_value(value);request_rebuild(true);return;}
      throw(std::string("no such variable or parameter: ")+name);
    }
    
    double InternalEquationSystem::get(const std::string& name)
    {
      if(name=="t"&&!is_autonomous())
	return t_;
      for(size_t i(0),size(varNames_.size());i!=size;++i)
	if(varNames_[i]==name)
	  return varValues_[i];
      for(size_t i(0),size(parNames_.size());i!=size;++i)
	if(parNames_[i]==name)
	  return parValues_[i];
      throw(std::string("no such variable or parameter: ")+name);
    }
    
    bool InternalEquationSystem::structure_changed(const FunctionSet& varSet, const FunctionSet& parSet, 
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

    uint64_t InternalEquationSystem::time_stamp() const
    {
      return timeStamp_;
    }
    
    void InternalEquationSystem::clear()
    {
      fSet_.clear();
      request_rebuild();
    }
    
    bool InternalEquationSystem::is_autonomous() const {return isAutonomous_;}
    
    size_t InternalEquationSystem::n_variables() const {return varNames_.size();}
    size_t InternalEquationSystem::n_parameters() const {return parNames_.size();}
    size_t InternalEquationSystem::n_functions() const {return funcNames_.size();}
    size_t InternalEquationSystem::n_constants() const {return constNames_.size();}
    
    const double& InternalEquationSystem::time() const {if(is_autonomous()){throw(std::string("cannot get t: system is autonomous"));}return t_;}
    
    const double* InternalEquationSystem::variable_values() const {return varValues_.size()==0?NULL:&varValues_[0];}
    const double* InternalEquationSystem::parameter_values() const {return parValues_.size()==0?NULL:&parValues_[0];}
    const double* InternalEquationSystem::function_values() {return funcValues_.size()==0?NULL:&funcValues_[0];}
    const double* InternalEquationSystem::constant_values() {return constValues_.size()==0?NULL:&constValues_[0];}
    
    const std::string* InternalEquationSystem::variable_names() const {return varNames_.size()==0?NULL:&varNames_[0];}
    const std::string* InternalEquationSystem::parameter_names() const {return parNames_.size()==0?NULL:&parNames_[0];}
    const std::string* InternalEquationSystem::function_names() const {return funcNames_.size()==0?NULL:&funcNames_[0];}
    const std::string* InternalEquationSystem::constant_names() const {return constNames_.size()==0?NULL:&constNames_[0];}
    
    const std::string* InternalEquationSystem::variable_definitions() const {return varDefs_.size()==0?NULL:&varDefs_[0];}
    const std::string* InternalEquationSystem::function_definitions() const {return funcDefs_.size()==0?NULL:&funcDefs_[0];}
    const std::string* InternalEquationSystem::constant_definitions() const {return constDefs_.size()==0?NULL:&constDefs_[0];}
    
    void InternalEquationSystem::request_rebuild(bool lean)
    {
      if(stalled_)
	{
	  rebuildRequested_=true;
	}
      else
	{ 
	  rebuild(lean);
	  rebuildRequested_=false;
	}
    }
    
    void InternalEquationSystem::stall()
    {
      ++stalled_;
    }
    
    void InternalEquationSystem::flush()
    {
      --stalled_;
      if(!stalled_)
	{
	  stalled_=false;
	  request_rebuild();
	}
    }
    
    std::string InternalEquationSystem::trim(std::string s) const
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
    
    void InternalEquationSystem::clear_vectors()
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
    
    void InternalEquationSystem::prepare_variables_and_parameters
    (FunctionSet& source, FunctionSet& varSet, FunctionSet& rhsSet, FunctionSet& parSet) const
    {
      FunctionSet::iterator i = source.begin();
      while(i!=source.end())
	{
	  if(i->name[i->name.length()-1]=='\'') // time derivative 
	    {
	      rhsSet.push_back(*i);
	      std::string varname=i->name;size_t l(varname.length());varname[l-1]=0;
	      i=source.erase(i);
	      FunctionSet::iterator fInfo=get_function_from_set(varname.c_str(),source);
	      if(fInfo!=source.end())
		{
		  varSet.push_back(*fInfo);
		  source.erase(fInfo);
		  i=source.begin();
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
    
    void InternalEquationSystem::prepare_functions_and_constants
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

    
    void InternalEquationSystem::build_variables(const FunctionSet& varSet, const FunctionSet& rhsSet)
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
    
    void InternalEquationSystem::build_parameters(const FunctionSet& parSet)      
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
    
    void InternalEquationSystem::build_functions_and_constants(const FunctionSet& info, VecF& funcs, VecS& names, VecS& defs, VecD& values)
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
    
    void InternalEquationSystem::rebuild(bool lean)
    {
      if(lean)
	{
	  t_=tFunc_.evaluate();
	  for(size_t i(0),size(varFuncs_.size());i<size;++i)varValues_[i]=varFuncs_[i].evaluate();
	  for(size_t i(0),size(parFuncs_.size());i<size;++i)parValues_[i]=parFuncs_[i].evaluate();
	  for(size_t i(0),size(funcFuncs_.size());i<size;++i)funcValues_[i]=funcFuncs_[i].evaluate();
	  for(size_t i(0),size(constFuncs_.size());i<size;++i)constValues_[i]=constFuncs_[i].evaluate();
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
    
    void InternalEquationSystem::detach(Function& t, VecF& x, VecF& p, VecF& rhs, VecF& func) const
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
      size_t nPar(n_parameters());
      for(size_t i(0);i<nVar;++i)
	x.push_back(get_function_from_set(varNames_[i],fSetClone)->f);
      for(size_t i(0);i<nPar;++i)
	p.push_back(get_function_from_set(parNames_[i],fSetClone)->f);
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

    F InternalEquationSystem::f() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      return
	[nVar,X,RHS](const double* x, double* rhs) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar;++i)
	    rhs[i]=RHS[i].evaluate();
	};
    }
    
    F InternalEquationSystem::dfdx() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      VecF DFDX;
      for(size_t i(0);i<nVar;++i)
	for(size_t j(0);j<nVar;++j)
	  DFDX.push_back(RHS[j].get_partial_derivative(X[i]));

      return
	[nVar,X,DFDX](const double* x, double* jac) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar*nVar;++i)
	      jac[i]=DFDX[i].evaluate();
	};
    }
    
    F InternalEquationSystem::func() const
    {
      size_t nFunc(n_functions());
      if(!nFunc)
	return NULL;

      size_t nVar(n_variables());

      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      return
	[nVar,nFunc,X,FUNC](const double* x, double* fval) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nFunc;++i)
	    fval[i]=FUNC[i].evaluate();
	};
    }
    
    F_p InternalEquationSystem::f_p() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;
      size_t nPar(n_parameters());
      
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      return
	[nVar,nPar,X,P,RHS](const double* x, const double* p, double* rhs) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar;++i)
	    rhs[i]=RHS[i].evaluate();
	};
    }
    
    F_p InternalEquationSystem::dfdx_p() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      size_t nPar(n_parameters());
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      VecF DFDX;
      for(size_t i(0);i<nVar;++i)
	for(size_t j(0);j<nVar;++j)
	  DFDX.push_back(RHS[j].get_partial_derivative(X[i]));

      return
	[nVar,nPar,X,P,DFDX](const double* x, const double* p, double* jac) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar*nVar;++i)
	      jac[i]=DFDX[i].evaluate();
	};
    }
    
    F_p InternalEquationSystem::dfdp_p() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      size_t nPar(n_parameters());
      if(!nPar)
	return NULL;

      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      VecF DFDP;
      for(size_t i(0);i<nPar;++i)
	for(size_t j(0);j<nVar;++j)
	  DFDP.push_back(RHS[j].get_partial_derivative(P[i]));

      return
	[nVar,nPar,X,P,DFDP](const double* x, const double* p, double* dfdp) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar*nPar;++i)
	      dfdp[i]=DFDP[i].evaluate();
	};
    }
    
    F_p InternalEquationSystem::func_p() const
    {
      size_t nFunc(n_functions());
      if(!nFunc)
	return NULL;

      size_t nVar(n_variables());
      size_t nPar(n_parameters());
      
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      return
	[nVar,nPar,nFunc,X,P,FUNC](const double* x, const double* p, double* fval) mutable
	{
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nFunc;++i)
	    fval[i]=FUNC[i].evaluate();
	};
    }
    
    F_t InternalEquationSystem::f_t() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;
      
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);
      
      return
	[nVar,T,X,RHS](double t, const double* x, double* rhs) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar;++i)
	    rhs[i]=RHS[i].evaluate();
	};
    }
    
    F_t InternalEquationSystem::dfdx_t() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      VecF DFDX;
      for(size_t i(0);i<nVar;++i)
	for(size_t j(0);j<nVar;++j)
	  DFDX.push_back(RHS[j].get_partial_derivative(X[i]));

      return
	[nVar,T,X,DFDX](double t, const double* x, double* jac) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nVar*nVar;++i)
	      jac[i]=DFDX[i].evaluate();
	};
    }

    F_t InternalEquationSystem::func_t() const
    {
      size_t nFunc(n_functions());
      if(!nFunc)
	return NULL;

      size_t nVar(n_variables());
      
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      return
	[nVar,nFunc,T,X,FUNC](double t,const double* x, double* fval) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nFunc;++i)
	    fval[i]=FUNC[i].evaluate();
	};
    }

    F_pt InternalEquationSystem::f_pt() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      size_t nPar(n_parameters());
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);
      
      return
	[nVar,nPar,T,X,P,RHS](double t, const double* x, const double* p, double* rhs) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar;++i)
	    rhs[i]=RHS[i].evaluate();
	};
    }

    F_pt InternalEquationSystem::dfdx_pt() const
    {
      size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      size_t nPar(n_parameters());
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      VecF DFDX;
      for(size_t i(0);i<nVar;++i)
	for(size_t j(0);j<nVar;++j)
	  DFDX.push_back(RHS[j].get_partial_derivative(X[i]));

      return
	[nVar,nPar,T,X,P,DFDX](double t, const double* x, const double* p, double* jac) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar*nVar;++i)
	      jac[i]=DFDX[i].evaluate();
	};
    }

    F_pt InternalEquationSystem::dfdp_pt() const
    {
       size_t nVar(n_variables());
      if(!nVar)
	return NULL;

      size_t nPar(n_parameters());
      if(!nPar)
	return NULL;

      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      VecF DFDP;
      for(size_t i(0);i<nPar;++i)
	for(size_t j(0);j<nVar;++j)
	  DFDP.push_back(RHS[j].get_partial_derivative(P[i]));

      return
	[nVar,nPar,T,X,P,DFDP](double t, const double* x, const double* p, double* dfdp) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nVar*nPar;++i)
	      dfdp[i]=DFDP[i].evaluate();
	};
    }

    F_pt InternalEquationSystem::func_pt() const
    {

      size_t nFunc(n_functions());
      if(!nFunc)
	return NULL;

      size_t nVar(n_variables());
      size_t nPar(n_parameters());
      
      Function T; VecF X,P,RHS,FUNC;
      detach(T,X,P,RHS,FUNC);

      return
	[nVar,nPar,nFunc,T,X,P,FUNC](double t, const double* x, const double* p, double* fval) mutable
	{
	  T.set_value(t);
	  for(size_t i(0);i<nVar;++i)
	    X[i].set_value(x[i]);
	  for(size_t i(0);i<nPar;++i)
	    P[i].set_value(p[i]);
	  for(size_t i(0);i<nFunc;++i)
	    fval[i]=FUNC[i].evaluate();
	};
    }

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

  } /* end namespace num */
} /* end namespace scigma */

