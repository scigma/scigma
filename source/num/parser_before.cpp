#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <stdlib.h>
#include "parser.h"

namespace scigma // note: this namespace is closed at the end of parser_after.cpp
{
  namespace num
  {

#pragma clang diagnostic push // note: pop follows at the end of parser_after.cpp
#pragma clang diagnostic ignored "-Wmissing-variable-declarations" 
#pragma clang diagnostic ignored "-Wmissing-prototypes" 
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"      

    const int MAX_FUNARGS_=31;  
    
    struct ValueSequence
    {
    size_t length;
    double values[MAX_FUNARGS_];
  };
    
    struct FunctionSequence
    {
    size_t length;
    Function* functions[MAX_FUNARGS_];
    };
    
    std::istringstream parserStream;
    std::string parserResult;
    std::string parserError;
    std::string currentExpression;
    std::string LHS;
    FunctionSet* currentFunctionSet(NULL);
    FunctionInfo fRedefinedInfo;
    std::vector<Function> requiredFunctions;
    std::vector<Function> newlyDefinedFunctions;
    std::vector<Operator*> opTemp;
    std::vector<Function*> fTemp;
    bool justUpdatingFunctions(false);
    bool odeParserIsInitialized(false);
    
    void yyerror(const char *error);
    int yylex();
    int yyparse();
    
    char* symbuf(NULL);
    int length(0);
    
    void clean_up_ode_parser()
    {
      if(symbuf)
	free(symbuf);
    }
    
    void init_ode_parser()
    {
      Operator::init_operators();
      atexit(clean_up_ode_parser);
      odeParserIsInitialized=true;
    }
    
    std::string parse_expression(std::string expression, FunctionSet& fSet)
    {
      //      std::cout<<"parsing "<<expression;
      if(!odeParserIsInitialized)
	init_ode_parser();
      parserResult="";
      parserError="";
      
      currentExpression=expression;
      currentFunctionSet=&fSet;
      newlyDefinedFunctions.clear();
      
      parserStream.clear();
      parserStream.str(currentExpression);

      yyparse();
      for(unsigned int i =0;i<fTemp.size();++i)
	delete fTemp.at(i);
      fTemp.clear();
      for(unsigned int i =0;i<opTemp.size();++i)
	delete opTemp.at(i);
      opTemp.clear();
      

      if(parserError!="")
	throw(parserError);
      return parserResult;
    }
 
    bool f_does_not_need_redefined_function(FunctionInfo fInfo)
    {
      return ((!fInfo.f.is_function_of(fRedefinedInfo.f))&&std::find(fInfo.requiredFunctions.begin(),fInfo.requiredFunctions.end(),fRedefinedInfo.f)==fInfo.requiredFunctions.end());
    }

    /*  bool f_is_not_a_time_derivative(FunctionInfo fInfo)
    {
      return fInfo.name[fInfo.name.length()-1]!="'";
    }*/
    
    bool y_needs_x(FunctionInfo xInfo, FunctionInfo yInfo)
    {
      
      return yInfo.f.is_function_of(xInfo.f)||(find(yInfo.requiredFunctions.begin(),yInfo.requiredFunctions.end(),xInfo.f)!=yInfo.requiredFunctions.end());

    }
   
    Function wrap_in_ID_if_necessary(FunctionSet& fSet, Function f)
    {
      if(get_function_from_set(f,fSet)!=fSet.end()||
	 (std::find(newlyDefinedFunctions.begin(),newlyDefinedFunctions.end(),f)
	 !=newlyDefinedFunctions.end()))
	{
	  Operator ID;
	  Operator::get_operator("ID",ID);
	  return ID(f);
	}
      return f;
    }

    void redefine_function(std::vector<FunctionInfo>::iterator fInfoIterator, Function f, std::string definition, FunctionSet& fSet)
    {
      // If f is already a Function in the FunctionSet, the ID operator is used to make the new Function different from f
      f = wrap_in_ID_if_necessary(fSet,f);
      // The purpose of the next block is explained further below.
      if(justUpdatingFunctions)
	{
	  fInfoIterator->f=f;
	  fInfoIterator->requiredFunctions=requiredFunctions;
	  requiredFunctions.clear();
	  return;
	}
      // This method must ensure two things: 
      // - First, it must check that the new definition f of the function denoted by fInfoIterator->name
      //   does not depend on its old definition fInfoIterator->f, i.e. rule out cases like x=x+1 or also x=y+z, where z = x^2 and so on.
      if(f.is_function_of(fInfoIterator->f))
	{
	  std::string error=fInfoIterator->name+" must not depend on itself";
	  throw(error);
	}
      // - Second, it must update all functions in the FunctionSet which depend on fInfoIterator->f to now depend on f. 
      //   To achieve that, the FunctionSet is first ordered according to the dependence of the Function instances in it on one
      //   another. Then, f is inserted at the appropriate position and all Function instances that appear later are reevaluated
      //   by the parser, without sorting the FunctionSet again at each step (that's what the justUpdatingFunctions flag is for).
      fRedefinedInfo=*fInfoIterator;
      fSet.erase(fInfoIterator);
      fInfoIterator=std::stable_partition(fSet.begin(),fSet.end(),f_does_not_need_redefined_function);
      //      fInfoIterator=std::stable_partition(fInfoIterator,fSet.end(),f_is_not_a_time_derivative());
      /* now we are guaranteed to insert f after all functions that do not depend on it,
	 but before all functions that do
      */
      fInfoIterator=fSet.insert(fInfoIterator,fRedefinedInfo);
      fInfoIterator->f=f;
      fInfoIterator->definition=definition;
      fInfoIterator->requiredFunctions=requiredFunctions;
      requiredFunctions.clear();
      // sort_function_set(fInfoIterator+1,fSet.end(),y_needs_x);
      FunctionSet::iterator fDepInfoIterator=fInfoIterator+1;
      justUpdatingFunctions=true;  
      while(fDepInfoIterator!=fSet.end())
	{
	  try
	    {
	      parse_expression(fDepInfoIterator->definition,fSet);
	    }
	  catch(std::string error)
	    {
	      // the only exceptions occuring in this loop should arise when the number of arguments of a Function
	      // needed by another Function has changed (e.g. parsing x=y(a,b) fails, if y(a,b)=a*b has been redefined as y(a)=a^2)
	      // in this case, the definition of the Function in question (i.e. y) has to be revoked, and all dependent
	      // Functions based on the new definition of y have to be recompiled:
	      *fInfoIterator=fRedefinedInfo;
	      ++fInfoIterator;
	      while(fInfoIterator!=fDepInfoIterator)
		{
		  // here, no exceptions are expected, because all definitions have been tested before
		  parse_expression(fInfoIterator->definition,fSet);
		}
	      justUpdatingFunctions=false;
	      throw;
	    }
	  ++fDepInfoIterator;
	}
      justUpdatingFunctions=false;
    }
    
    void unset_time_derivative(Function f, FunctionSet& fSet);

    void redefine_function(Function fOld, Function f, std::string definition, FunctionSet& fSet)
    {
      FunctionSet::iterator fInfoIterator(get_function_from_set(fOld, fSet));
      std::string name(fInfoIterator->name);
      redefine_function(fInfoIterator,f,definition,fSet);
      FunctionSet::iterator fInfoIteratorNew(get_function_from_set(name,fSet));
      Function fNew(fInfoIteratorNew->f);
      if(!fNew.is_constant())
	  unset_time_derivative(fNew,fSet);
    }
    
    void redefine_function_with_arguments(Function fOld, Function f, std::string definition, FunctionSequence& fSequence,FunctionSet& fSet)
    {
      f = wrap_in_ID_if_necessary(fSet,f);


      FunctionSet::iterator fInfoIterator(get_function_from_set(fOld,fSet));
      std::string name(fInfoIterator->name);
      
      std::vector<std::string> argNames;
      FunctionSet::iterator fArgInfoIterator;
      for(unsigned int i =0;i<fSequence.length;++i)
	{
	  if((fArgInfoIterator=get_function_from_set(*fSequence.functions[i],fSet))==fSet.end())
	    {
	      std::string error("could not read dummy arguments");
	      throw(error);
	    }
	  if(name==fArgInfoIterator->name)
	    {
	      std::string error("cannot use function as its own dummy argument");
	      throw(error);
	    }
	  argNames.push_back(fArgInfoIterator->name);
	}

      redefine_function(fInfoIterator->f,f,definition,fSet);
      Function* args(new Function[fSequence.length]);
      for(unsigned int j(0);j<fSequence.length;++j)
	args[j]=get_function_from_set(argNames[j],fSet)->f;
      f.set_arguments(args, uint8_t(fSequence.length));
      delete[] args;
    }
    
    Function insert_arguments_into_function(Function f,ValueSequence& vSequence, FunctionSet& fSet)
    {
      f.get_number_of_arguments();
      if(f.get_number_of_arguments()!=vSequence.length)
	{
	  std::string error(get_function_from_set(f,fSet)->name+ " has the wrong number of arguments");
	  throw(error);
	}
      requiredFunctions.push_back(f);
      return f(vSequence.values);
    }
    
    Function insert_arguments_into_function(Function f,FunctionSequence& fSequence, FunctionSet& fSet)
    {
      if(f.get_number_of_arguments()!=fSequence.length)
	{
	  std::string error(get_function_from_set(f,fSet)->name+ " has the wrong number of arguments");
	  throw(error);
	}
      Function* args=new Function[fSequence.length];
      for(unsigned int j=0;j<fSequence.length;++j)
	args[j]=*fSequence.functions[j];
      requiredFunctions.push_back(f);
      Function ret = f(args);
      delete[] args;
      return ret;
    }
    
    void extract_required_functions(Function f, FunctionSet& fSet)
    {
      FunctionSet::iterator fInfoIterator(fSet.begin());
      while(fInfoIterator!=fSet.end())
	{
	  if(f.is_function_of(fInfoIterator->f))
	    requiredFunctions.push_back(fInfoIterator->f);
	  ++fInfoIterator;
	}
    }
    
    void define_function(std::string name,FunctionSet& fSet)
    {
      FunctionInfo fInfo;
      fInfo.name=name;
      fInfo.definition=name+"=0\n";
      fSet.push_back(fInfo);
      newlyDefinedFunctions.push_back(fInfo.f);
    }
    
    void set_as_independent_variable(FunctionSet::iterator fInfoIterator)
    {
      /* this replaces the old definition of a variable
	 by the expression <varname>=<value>, in case
	 it was previously a function of other variables,
	 but keeping the value if is was already a parameter
      */
      std::string newDef=fInfoIterator->definition;
      unsigned long int equalSignPos=newDef.find('=');
      newDef[equalSignPos]=0;newDef=newDef.c_str();
      double value=fInfoIterator->f.evaluate();std::string rhs;
      std::stringstream ss;ss<<value;ss>>rhs;newDef+="="+rhs+"\n";
      fInfoIterator->definition=newDef;
    }
    
    void set_time_derivative(Function f, Function fDot, std::string definition,FunctionSet& fSet)
    {
      fDot = wrap_in_ID_if_necessary(fSet,fDot);

      FunctionSet::iterator fInfoIterator=get_function_from_set(f,fSet);
      set_as_independent_variable(fInfoIterator);
      
      FunctionSet::iterator fDerivativeInfoIterator=get_function_from_set(fInfoIterator->name+"\'", fSet);
      
      if(fDerivativeInfoIterator==fSet.end())
	{
	  FunctionInfo fDerivativeInfo;
	  fDerivativeInfo.name=fInfoIterator->name+"\'";
	  fDerivativeInfo.f=fDot;
	  fDerivativeInfo.definition=definition;
	  fDerivativeInfo.requiredFunctions=requiredFunctions;
	  fSet.push_back(fDerivativeInfo);
	}
      else
	{
	  fDerivativeInfoIterator->f=fDot;
	  fDerivativeInfoIterator->definition=definition;
	  fDerivativeInfoIterator->requiredFunctions=requiredFunctions;
	}
    }
    
    void undefine_function(Function f, FunctionSet& fSet)
    {
      FunctionSet::iterator fInfoIterator=get_function_from_set(f,fSet);
      std::string name=fInfoIterator->name;
      FunctionSet::iterator fDerivativeInfoIterator=get_time_derivative_from_set(f,fSet);
      FunctionSet::iterator otherFunctionInfoIterator=fSet.begin();
      
      while(otherFunctionInfoIterator!=fSet.end())
	{
	  bool ignore(otherFunctionInfoIterator==fInfoIterator||
		      otherFunctionInfoIterator==fDerivativeInfoIterator);

	  if((!ignore)&&y_needs_x(*fInfoIterator,*otherFunctionInfoIterator))
	    {
	      std::string oName(otherFunctionInfoIterator->name);
	      std::string error = oName+" depends on "+name+" - delete "+oName+" first";
	      throw(error);
	    }
	  ++otherFunctionInfoIterator;
	}
      
      fDerivativeInfoIterator=get_time_derivative_from_set(f,fSet);
      if(fDerivativeInfoIterator!=fSet.end())
	fSet.erase(fDerivativeInfoIterator);
      
      fSet.erase(get_function_from_set(f,fSet));
    }
    
    void unset_time_derivative(Function f, FunctionSet& fSet)
    {
      FunctionSet::iterator fDerivativeInfoIterator=get_time_derivative_from_set(f, fSet);
      if(fDerivativeInfoIterator!=fSet.end())
	fSet.erase(fDerivativeInfoIterator);
    }
    
    /*    void sort_function_set(FunctionSet::iterator begin, FunctionSet iterator end)
    {
      std::stable_sort(fSet.begin(),fSet.end(),y_needs_x);
      }*/

    //void clone_sorted_function_set(const FunctionSet& source, FunctionSet& target)
    void clone_function_set(const FunctionSet& source, FunctionSet& target)
    {
      FunctionSet::const_iterator i = source.begin();
      justUpdatingFunctions=true;
      while(i!=source.end())
	{
	  parse_expression(i->definition,target);
	  ++i;
	}
      justUpdatingFunctions=false;
    }
    
    Operator* new_op_ptr(Operator op)
    {
      opTemp.push_back(new Operator(op));
      return opTemp.back();
    }
    
    Function* new_f_ptr(Function f)
    {
      fTemp.push_back(new Function(f));
      return fTemp.back();
    }
    
    FunctionSet::iterator get_function_from_set(std::string name, FunctionSet& fSet)
    {
      FunctionSet::iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->name==name)
	    return i;
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::iterator get_function_from_set(Function f, FunctionSet& fSet)
    {
      FunctionSet::iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->f==f)
	    return i;
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::iterator get_time_derivative_from_set(std::string name, FunctionSet& fSet) 
    {
      FunctionSet::iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->name==name+"\'")
	    return i;
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::iterator get_time_derivative_from_set(Function f, FunctionSet& fSet) 
    {
      FunctionSet::iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->f==f)
	    return get_time_derivative_from_set(i->name,fSet);
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::const_iterator get_function_from_set(std::string name, const FunctionSet& fSet)
    {
      FunctionSet::const_iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->name==name)
	    return i;
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::const_iterator get_function_from_set(Function f, const FunctionSet& fSet)
    {
      FunctionSet::const_iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->f==f)
	    return i;
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::const_iterator get_time_derivative_from_set(std::string name, const FunctionSet& fSet) 
    {
      FunctionSet::const_iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->name==name+"\'")
	    return i;
	  ++i;
	}
      return fSet.end();
    }
    
    FunctionSet::const_iterator get_time_derivative_from_set(Function f, const FunctionSet& fSet) 
    {
      FunctionSet::const_iterator i = fSet.begin();
      while(i!=fSet.end())
	{
	  if(i->f==f)
	    return get_time_derivative_from_set(i->name,fSet);
	  ++i;
	}
      return fSet.end();
    }

    // ignore warnings that appear in bison-generated code
#pragma clang diagnostic ignored "-Wunreachable-code"      
#pragma clang diagnostic ignored "-Wunreachable-code-break"
#pragma clang diagnostic ignored "-Wold-style-cast"      
#pragma clang diagnostic ignored "-Wsign-conversion"    
#pragma clang diagnostic ignored "-Wunused-macros"      
#pragma clang diagnostic ignored "-Wconversion"    
    // get rid of the unused macro warnings in parser.cpp before the inclusion of this file
#ifdef YYTOKENTYPE
    int useMacros1 = YYBISON+YYPURE+YYLSP_NEEDED+NEG;
    std::string useMacros2 = std::string(YYBISON_VERSION)+YYSKELETON_NAME;
#endif
    // namespaces are not closed here, but in parser_after.cpp
    
