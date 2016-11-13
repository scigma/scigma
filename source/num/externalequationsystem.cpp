#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <sstream>
#include "externalequationsystem.hpp"
#include "../common/util.hpp"

namespace scigma
{
  namespace num
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

    void test_uniqueness(VecS& names)
    {
      std::sort(names.begin(),names.end());
      for(VecS::const_iterator i(++names.begin()),end(names.end());i!=end;++i)
	if(*i==*(i-1))
	  throw(std::string("name is not unique: ")+(*i));
    }

    int test_if_well_formed(const VecS& names)
    {
      size_t index(0);
      for(VecS::const_iterator i(names.begin()),end(names.end());i!=end;++i)
	{
	  if(std::isdigit((*i)[0]))
	    return int(index);
	  for(size_t j(0);j<i->size();++j)
	    if((!std::isalnum((*i)[j]))&&(*i)[j]!='_')
	      return int(index);
	  ++index;
	}
      return -1;
    }

#pragma clang diagnostic pop
    
    ExternalEquationSystem::ExternalEquationSystem(VecS variables,
						   F f, F dfdx,
						   VecS functions,
						   F func):
      varNames_(variables),
      varDefs_(VecS(variables.size(),"external")),
      varValues_(variables.size()), 
      funcNames_(functions),
      funcDefs_(VecS(functions.size(),"external")),
      funcValues_(functions.size()),
      f_(f),dfdx_(dfdx),func_(func),
      isAutonomous_(true),hasChanged_(true)
    {
      if(!varNames_.size())
	throw(std::string("need at least one variable"));
      if(!f)
	throw(std::string("right hand side cannot be NULL"));
      if((!funcNames_.size())&&func)
	throw(std::string("provided evaluation function but no function names"));
      if(funcNames_.size()&&!func)
	throw(std::string("provided function names but no evaluation function"));

      int index;
      if((index=test_if_well_formed(varNames_))!=-1)
	throw(std::string("ill-formed variable name: ")+varNames_[size_t(index)]);
      if((index=test_if_well_formed(funcNames_))!=-1)
	 throw(std::string("ill-formed function name: ")+funcNames_[size_t(index)]);
      VecS names(varNames_);
      names.insert(names.end(),functions.begin(),functions.end());
      test_uniqueness(names);
      using namespace std::chrono;
      nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
      timeStamp_= uint64_t (ns.count());
    }
    
    ExternalEquationSystem::ExternalEquationSystem(VecS variables, VecS parameters,
						   F_p f, F_p dfdx, F_p dfdp,
						   VecS functions,
						   F_p func):
      varNames_(variables),
      varDefs_(VecS(variables.size(),"external")),
      varValues_(variables.size()),
      parNames_(parameters),parValues_(parameters.size()),
      funcNames_(functions),
      funcDefs_(VecS(functions.size(),"external")),
      funcValues_(functions.size()), 
      f_p_(f),dfdx_p_(dfdx),dfdp_p_(dfdp),func_p_(func),
      isAutonomous_(true),hasChanged_(true)
    {
      if(!varNames_.size())
	throw(std::string("need at least one variable"));
      if(!parNames_.size())
	throw(std::string("need at least one parameter"));
      if(!f)
	throw(std::string("right hand side cannot be NULL"));
      if((!funcNames_.size())&&func)
	throw(std::string("provided evaluation function but no function names"));
      if(funcNames_.size()&&!func)
	throw(std::string("provided function names but no evaluation function"));

      int index;
      if((index=test_if_well_formed(varNames_))!=-1)
	throw(std::string("ill-formed variable name: ")+varNames_[size_t(index)]);
      if((index=test_if_well_formed(parNames_))!=-1)
	throw(std::string("ill-formed parameter name: ")+parNames_[size_t(index)]);
      if((index=test_if_well_formed(funcNames_))!=-1)
	 throw(std::string("ill-formed function name: ")+funcNames_[size_t(index)]);
      VecS names(varNames_);
      names.insert(names.end(),parameters.begin(),parameters.end());
      names.insert(names.end(),functions.begin(),functions.end());
      test_uniqueness(names);
      using namespace std::chrono;
      nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
      timeStamp_= uint64_t (ns.count());
    }
    
    ExternalEquationSystem::ExternalEquationSystem(VecS variables,
						   F_t f, F_t dfdx,
						   VecS functions,
						   F_t func):
      varNames_(variables),
      varDefs_(VecS(variables.size(),"external")),
      varValues_(variables.size()),
      funcNames_(functions),
      funcDefs_(VecS(functions.size(),"external")),
      funcValues_(functions.size()),
      f_t_(f),dfdx_t_(dfdx),func_t_(func),
      isAutonomous_(false),hasChanged_(true)
    {
      if(!varNames_.size())
	throw(std::string("need at least one variable"));
      if(!f)
	throw(std::string("right hand side cannot be NULL"));
      if((!funcNames_.size())&&func)
	throw(std::string("provided evaluation function but no function names"));
      if(funcNames_.size()&&!func)
	throw(std::string("provided function names but no evaluation function"));

      int index;
      if((index=test_if_well_formed(varNames_))!=-1)
	throw(std::string("ill-formed variable name: ")+varNames_[size_t(index)]);
      if((index=test_if_well_formed(funcNames_))!=-1)
	 throw(std::string("ill-formed function name: ")+funcNames_[size_t(index)]);
      VecS names(varNames_);
      names.insert(names.end(),functions.begin(),functions.end());
      test_uniqueness(names);
      using namespace std::chrono;
      nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
      timeStamp_= uint64_t (ns.count());
    }
    
    ExternalEquationSystem::ExternalEquationSystem(VecS variables, VecS parameters,
						   F_pt f, F_pt dfdx, F_pt dfdp,
						   VecS functions, F_pt func):
      varNames_(variables),
      varDefs_(VecS(variables.size(),"external")),
      varValues_(variables.size()),
      parNames_(parameters),parValues_(parameters.size()),
      funcNames_(functions),
      funcDefs_(VecS(functions.size(),"external")),
      funcValues_(functions.size()),
      f_pt_(f),dfdx_pt_(dfdx),dfdp_pt_(dfdp),func_pt_(func),
      isAutonomous_(false),hasChanged_(true)
    {
      if(!varNames_.size())
	throw(std::string("need at least one variable"));
      if(!parNames_.size())
	throw(std::string("need at least one parameter"));
      if(!f)
	throw(std::string("right hand side cannot be NULL"));
      if((!funcNames_.size())&&func)
	throw(std::string("provided evaluation function but no function names"));
      if(funcNames_.size()&&!func)
	throw(std::string("provided function names but no evaluation function"));

      int index;
      if((index=test_if_well_formed(varNames_))!=-1)
	throw(std::string("ill-formed variable name: ")+varNames_[size_t(index)]);
      if((index=test_if_well_formed(parNames_))!=-1)
	throw(std::string("ill-formed parameter name: ")+parNames_[size_t(index)]);
      if((index=test_if_well_formed(funcNames_))!=-1)
	 throw(std::string("ill-formed function name: ")+funcNames_[size_t(index)]);
      VecS names(varNames_);
      names.insert(names.end(),parameters.begin(),parameters.end());
      names.insert(names.end(),functions.begin(),functions.end());
      test_uniqueness(names);
      using namespace std::chrono;
      nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
      timeStamp_= uint64_t (ns.count());
    }

    uint64_t ExternalEquationSystem::time_stamp() const
    {
      return timeStamp_;
    }
    
    std::string ExternalEquationSystem::parse(std::string expression)
    {
      std::string exp(trim(expression));
      if(exp.size()>0&&exp[0]=='$')
	{
	  exp[0]=' ';
	  double result(get(exp));
	  std::stringstream ss;
	  ss<<result;
	  return ss.str();
	}
      VecS tokens;
      common::append_tokens(exp,tokens,'=');
      if(tokens.size()==2)
	{
	  std::stringstream ss;
	  ss<<tokens[1];
	  double value;
	  ss>>value;
	  set(tokens[0],value);
	  return "";
	}
      throw(std::string("could not parse expression '")+expression+"'");
    }
    
    void ExternalEquationSystem::set(const std::string& name, double value)
    {
      if(name=="t"&&!is_autonomous())
	{
	  t_=value;
	  hasChanged_=true;
	  return;
	}
      for(size_t i(0);i<varNames_.size();++i)
	{
	  if(varNames_[i]==name)
	    {
	      varValues_[i]=value;
	      hasChanged_=true;
	      return;
	    }
	}
      for(size_t i(0);i<parNames_.size();++i)
	{
	  if(parNames_[i]==name)
	    {
	      parValues_[i]=value;
	      hasChanged_=true;
	      return;
	    }
	}
      throw(name+" is not a variable or parameter\n");
    }
    
    
    double ExternalEquationSystem::get(const std::string& name)
    {
      if(name=="t"&&!is_autonomous())
	return time();

      for(size_t i(0);i<varNames_.size();++i)
	if(varNames_[i]==name)
	  return varValues_[i];

      for(size_t i(0);i<parNames_.size();++i)
	if(parNames_[i]==name)
	   return parValues_[i];

      for(size_t i(0);i<funcNames_.size();++i)
	if(funcNames_[i]==name)
	  {
	    update_function_values();
	    return funcValues_[i];
	  }

      for(size_t i(0);i<constNames_.size();++i)
	if(constNames_[i]==name)
	  return constValues_[i];

      throw(name+" is not defined in the dynamical system\n");
    }

    bool ExternalEquationSystem::is_autonomous() const {return isAutonomous_;}
    
    size_t ExternalEquationSystem::n_variables() const {return varNames_.size();}
    size_t ExternalEquationSystem::n_parameters() const {return parNames_.size();}
    size_t ExternalEquationSystem::n_functions() const {return funcNames_.size();}
    size_t ExternalEquationSystem::n_constants() const {return constNames_.size();}
    
    const double& ExternalEquationSystem::time() const {return t_;}
    
    const double* ExternalEquationSystem::variable_values() const {return varValues_.size()==0?NULL:&varValues_[0];}
    const double* ExternalEquationSystem::parameter_values() const {return parValues_.size()==0?NULL:&parValues_[0];}
    const double* ExternalEquationSystem::function_values() 
    {
      if(!funcValues_.size())
	return NULL;

      update_function_values();
      return &funcValues_[0];
    }
    const double* ExternalEquationSystem::constant_values() {return constValues_.size()==0?NULL:&constValues_[0];}

    void ExternalEquationSystem::update_function_values()
    {
      if(hasChanged_)
	{
	  func()(&varValues_[0],&funcValues_[0]);
	  hasChanged_=false;
	}
    }
    
    const std::string* ExternalEquationSystem::variable_names() const {return varNames_.size()==0?NULL:&varNames_[0];}
    const std::string* ExternalEquationSystem::parameter_names() const {return parNames_.size()==0?NULL:&parNames_[0];}
    const std::string* ExternalEquationSystem::function_names() const {return funcNames_.size()==0?NULL:&funcNames_[0];}
    const std::string* ExternalEquationSystem::constant_names() const {return constNames_.size()==0?NULL:&constNames_[0];}

    const std::string* ExternalEquationSystem::variable_definitions() const {return varDefs_.size()==0?NULL:&varDefs_[0];}
    const std::string* ExternalEquationSystem::function_definitions() const {return funcDefs_.size()==0?NULL:&funcDefs_[0];}
    const std::string* ExternalEquationSystem::constant_definitions() const {return constDefs_.size()==0?NULL:&constDefs_[0];}

    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    F ExternalEquationSystem::f() const
    {
      auto& f_p(f_p_); auto& f_t(f_t_); auto& f_pt(f_pt_);
      auto& parValues(parValues_); double t(t_);
      if(f_) return f_;
      else if(f_p_) return [f_p,parValues](const double* x, double* rhs){f_p(x,&parValues[0],rhs);};
      else if(f_t_) return [f_t,t](const double* x, double* rhs){f_t(t,x,rhs);};
      else if(f_pt_) return [f_pt,parValues,t](const double* x, double* rhs){f_pt(t,x,&parValues[0],rhs);};
      else return NULL;
    }
    
    F ExternalEquationSystem::dfdx() const
    {
      auto& dfdx_p(dfdx_p_); auto& dfdx_t(dfdx_t_); auto& dfdx_pt(dfdx_pt_);
      auto& parValues(parValues_); double t(t_);
      if(dfdx_) return dfdx_; 
      else if(dfdx_p_) return [dfdx_p,parValues](const double* x, double* dfdxval){dfdx_p(x,&parValues[0],dfdxval);};
      else if(dfdx_t_) return [dfdx_t,t](const double* x, double* dfdxval){dfdx_t(t,x,dfdxval);};
      else if(dfdx_pt_) return [dfdx_pt,parValues,t](const double* x, double* dfdxval){dfdx_pt(t,x,&parValues[0],dfdxval);};
      else return NULL;
    }
    
    F ExternalEquationSystem::func() const
    {
      auto& func_p(func_p_); auto& func_t(func_t_); auto& func_pt(func_pt_);
      auto& parValues(parValues_); double t(t_);
      if(func_) return func_;
      else if(func_p_) return [func_p,parValues](const double* x, double* funcval){func_p(x,&parValues[0],funcval);};
      else if(func_t_) return [func_t,t](const double* x, double* funcval){func_t(t,x,funcval);};
      else if(func_pt_) return [func_pt,parValues,t](const double* x, double* funcval){func_pt(t,x,&parValues[0],funcval);};
      else return NULL;
    }
    
    F_p ExternalEquationSystem::f_p() const
    {
      auto& f(f_); auto& f_t(f_t_); auto& f_pt(f_pt_);
      double t(t_);
      if(f_) return [f](const double*x, const double* p, double* rhs){f(x,rhs);};
      else if(f_p_) return f_p_;
      else if(f_t_) return [f_t,t](const double* x, const double* p, double* rhs){f_t(t,x,rhs);};
      else if(f_pt_) return [f_pt,t](const double* x, const double* p, double* rhs){f_pt(t,x,p,rhs);};
      else return NULL;
    }
    
    F_p ExternalEquationSystem::dfdx_p() const
    { 
      auto& dfdx(dfdx_); auto& dfdx_t(dfdx_t_); auto& dfdx_pt(dfdx_pt_);
      double t(t_);
      if(dfdx_) return [dfdx](const double*x, const double* p, double* dfdxval){dfdx(x,dfdxval);};
      else if(dfdx_p_) return dfdx_p_;
      else if(dfdx_t_) return [dfdx_t,t](const double* x, const double* p, double* dfdxval){dfdx_t(t,x,dfdxval);};
      else if(dfdx_pt_) return [dfdx_pt,t](const double* x, const double* p, double* dfdxval){dfdx_pt(t,x,p,dfdxval);};
      else return NULL;
    }
    
    F_p ExternalEquationSystem::dfdp_p() const
    {
      auto& dfdp_pt(dfdp_pt_);
      double t(t_);
      if(dfdp_p_) return dfdp_p_;
      else if(dfdp_pt_) return [dfdp_pt,t](const double* x, const double* p, double* dfdpval){dfdp_pt(t,x,p,dfdpval);};
      else return NULL;
    }
    
    F_p ExternalEquationSystem::func_p() const
    {
      auto& func(func_); auto& func_t(func_t_); auto& func_pt(func_pt_);
      double t(t_);
      if(func_) return [func](const double*x, const double* p, double* funcval){func(x,funcval);};
      else if(func_p_) return func_p_;
      else if(func_t_) return [func_t,t](const double* x, const double* p, double* funcval){func_t(t,x,funcval);};
      else if(func_pt_) return [func_pt,t](const double* x, const double* p, double* funcval){func_pt(t,x,p,funcval);};
      else return NULL;
    }
    
    F_t ExternalEquationSystem::f_t() const
    {
      auto& f(f_); auto& f_p(f_p_); auto& f_pt(f_pt_);
      auto& parValues(parValues_);
      if(f_) return [f](double t, const double* x, double* rhs){f(x,rhs);};
      else if(f_p_) return [f_p,parValues](double t, const double* x, double* rhs){f_p(x,&parValues[0],rhs);};
      else if(f_t_) return f_t_;
      else if(f_pt_) return [f_pt,parValues](double t, const double* x, double* rhs){f_pt(t,x,&parValues[0],rhs);};
      else return NULL;
    }
    
    F_t ExternalEquationSystem::dfdx_t() const
    {
      auto& dfdx(dfdx_); auto& dfdx_p(dfdx_p_); auto& dfdx_pt(dfdx_pt_);
      auto& parValues(parValues_);
      if(dfdx_) return [dfdx](double t, const double* x, double* dfdxval){dfdx(x,dfdxval);};
      else if(dfdx_p_) return [dfdx_p,parValues](double t, const double* x, double* dfdxval){dfdx_p(x,&parValues[0],dfdxval);};
      else if(dfdx_t_) return dfdx_t_;
      else if(dfdx_pt_) return [dfdx_pt,parValues](double t, const double* x, double* dfdxval){dfdx_pt(t,x,&parValues[0],dfdxval);};
      else return NULL;
    }

    F_t ExternalEquationSystem::func_t() const
    {
      auto& func(func_); auto& func_p(func_p_); auto& func_pt(func_pt_);
      auto& parValues(parValues_);
      if(func_) return [func](double t, const double* x, double* funcval){func(x,funcval);};
      else if(func_p_) return [func_p,parValues](double t, const double* x, double* funcval){func_p(x,&parValues[0],funcval);};
      else if(func_t_) return func_t_;
      else if(func_pt_) return [func_pt,parValues](double t, const double* x, double* funcval){func_pt(t,x,&parValues[0],funcval);};
      else return NULL;
    }

    F_pt ExternalEquationSystem::f_pt() const
    {
      auto& f(f_); auto& f_p(f_p_); auto& f_t(f_t_);
      if(f_) return [f] (double t, const double* x, const double* p, double* rhs){f(x,rhs);};
      else if(f_p_) return [f_p] (double t, const double* x, const double* p, double* rhs){f_p(x,p,rhs);};
      else if(f_t_) return [f_t] (double t, const double* x, const double* p, double* rhs){f_t(t,x,rhs);};
      else if(f_pt_) return f_pt_;
      else return NULL;
    }

    F_pt ExternalEquationSystem::dfdx_pt() const
    {
      auto& dfdx(dfdx_); auto& dfdx_p(dfdx_p_); auto& dfdx_t(dfdx_t_);
      if(dfdx_) return [dfdx] (double t, const double* x, const double* p, double* dfdxval){dfdx(x,dfdxval);};
      else if(dfdx_p_) return [dfdx_p] (double t, const double* x, const double* p, double* dfdxval){dfdx_p(x,p,dfdxval);};
      else if(dfdx_t_) return [dfdx_t] (double t, const double* x, const double* p, double* dfdxval){dfdx_t(t,x,dfdxval);};
      else if(dfdx_pt_) return dfdx_pt_;
      else return NULL;
    }

    F_pt ExternalEquationSystem::dfdp_pt() const
    {
      auto& dfdp_p(dfdp_p_);
      if(dfdp_p_) return [dfdp_p] (double t, const double* x, const double* p, double* dfdpval){dfdp_p(x,p,dfdpval);};
      else if(dfdp_pt_) return dfdp_pt_;
      else return NULL;
    }

    F_pt ExternalEquationSystem::func_pt() const
    {
      auto& func(func_); auto& func_p(func_p_); auto& func_t(func_t_);
      if(func_) return [func] (double t, const double* x, const double* p, double* funcval){func(x,funcval);};
      else if(func_p_) return [func_p] (double t, const double* x, const double* p, double* funcval){func_p(x,p,funcval);};
      else if(func_t_) return [func_t] (double t, const double* x, const double* p, double* funcval){func_t(t,x,funcval);};
      else if(func_pt_) return func_pt_;
      else return NULL;
    }

    std::string ExternalEquationSystem::trim(std::string s) const
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

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
    
  } /* end namespace num */
} /* end namespace scigma */

