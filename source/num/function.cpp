#include "function.h"

namespace scigma
{
  namespace num
  {
  Function operator+(const double& d, const Function& f)
  {
    Operator plus;
    Operator::get_operator("+",plus);
    Function F[]={d,f};
    return plus(F);
  }
  
  Function operator-(const double& d, const Function& f)
  {
    Operator minus;
    Operator::get_operator("-",minus);
    Function F[]={d,f};
    return minus(F);
  }
  
  Function operator*(const double& d, const Function& f)
  {
    Operator mult;
    Operator::get_operator("*",mult);
    Function F[]={d,f};
    return mult(F);
  }	
  
  Function operator/(const double& d, const Function& f)
  {
    Operator div;
    Operator::get_operator("/",div);
    Function F[]={d,f};
    return div(F);
  }

FunctionData::FunctionData()
{
  nArgs = 0;
  isConstant = true;
  valueIsSet=true;
  value = 0;
  refcount=0;
}

FunctionData::FunctionData(double d)
{
  nArgs = 0;
  isConstant = true;
  valueIsSet=true;
  value = d;
  refcount=0;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
FunctionData::FunctionData(const Operator& op, Function* args)
{
  this->op = op;
  nArgs = 0;
  isConstant = false;
  valueIsSet = false;
  value = 0;
  refcount=0;
  
  for(uint8_t i = 0;i<this->op.get_number_of_arguments();i++)
    deps.push_back(args[i]);
}
#pragma clang diagnostic pop

    FunctionData::FunctionData(const FunctionData& f):op(f.op),value(f.value),deps(f.deps),args(f.args),nArgs(f.nArgs),refcount(0),valueIsSet(f.valueIsSet),isConstant(f.isConstant)
{}

//#include <iostream>

Function::Function()
{
  fData = new FunctionData();
  //std::cerr<<"create FunctionData: "<<std::hex<<(long int)fData<<std::endl<<std::dec;
  ++fData->refcount;
}

Function::Function(double value)
{
  fData = new FunctionData(value);
  //std::cerr<<"create FunctionData: "<<std::hex<<(long int)fData<<std::endl<<std::dec;
  ++fData->refcount;
}

Function::Function(const Operator& op, Function *fArgs)
{
  fData = new FunctionData(op,fArgs);
  //std::cerr<<"create FunctionData: "<<std::hex<<(long int)fData<<std::endl<<std::dec;
  ++fData->refcount;
}

Function::Function(const Function&f)
{
  fData = f.fData;
  ++fData->refcount;
}

Function::~Function()
{
  --fData->refcount;
  if(fData->refcount==0)
    {
      //std::cerr<<"delete FunctionData: "<<std::hex<<(long int)fData<<std::endl<<std::dec;
      delete fData;
    }

}

Function& Function::operator=(const Function& f)
{
  --fData->refcount;
  if(fData->refcount==0)
    {
      //std::cerr<<"delete FunctionData "<<std::hex<<(long int)fData<<std::endl<<std::dec;
      delete fData;
    }
  fData = f.fData;
  ++fData->refcount;
  return *this;
}

uint8_t Function::get_number_of_arguments() const
{
  return fData->nArgs;
}

void Function::set_value(double d)
{
  fData->value=d;
  fData->valueIsSet=true;
}

void Function::unset_value()
{
  if(!fData->isConstant)
    fData->valueIsSet=false;
}

bool Function::value_is_set() const
{
  return fData->valueIsSet;
}

bool Function::is_constant() const
{
  return fData->isConstant;
}

void Function::redefine(Function f)
{
  fData->op=f.fData->op;
  fData->value=f.fData->value;
  fData->valueIsSet=f.fData->valueIsSet;
  fData->isConstant=f.fData->isConstant;
  fData->deps=f.fData->deps;
}

double Function::evaluate() const
{
  if(fData->valueIsSet)
    return fData->value;
  
  double opargs[MAX_OPERATOR_ARGS];
  for(uint8_t i = 0;i<fData->op.get_number_of_arguments();i++)
    opargs[i]=fData->deps[i].evaluate();
  return fData->op(&opargs[0]);
}

double Function::operator()(double* dArgs) const
{
  double constants[MAX_FUNCTION_ARGS];
  for(uint8_t i = 0;i<fData->nArgs;i++)
    {
      if(fData->args[i].is_constant())
	constants[i]=fData->args[i].evaluate();
      fData->args[i].set_value(dArgs[i]);
    }
  double d = evaluate();
  
  for(uint8_t i = 0;i<fData->nArgs;i++)
    {
      fData->args[i].unset_value();
      if(fData->args[i].is_constant())
	fData->args[i].set_value(constants[i]);
    }
  return d;
}

Function Function::operator()(Function* fArgs) const
{
  Function oldArgs[MAX_FUNCTION_ARGS];
  for(uint8_t i = 0;i<fData->nArgs;i++)
    oldArgs[i]=fData->args[i];
  return insert_functions(fArgs,oldArgs,fData->nArgs);
}

Function Function::insert_functions(Function* fArgs, Function* oldArgs, uint8_t nArgs) const
{
  for(uint8_t i = 0;i<nArgs;i++)
    {
      if(oldArgs[i]==*this)
	{
	  return fArgs[i];
	}
    }
  
  if(is_constant())
    return *this;
  
  Function f1[MAX_OPERATOR_ARGS];
  for(uint8_t i = 0;i<fData->op.get_number_of_arguments();i++)
    f1[i]=fData->deps[i].insert_functions(fArgs,oldArgs,nArgs);
  
  Function g = fData->op(f1);
  
  Function f2[MAX_OPERATOR_ARGS];
  for(uint8_t i = 0;i<fData->nArgs;i++)
    f2[i]=fData->args[i].insert_functions(fArgs,oldArgs,nArgs);
  g.set_arguments(f2,fData->nArgs);
  
  for(uint8_t i = 0;i<fData->op.get_number_of_arguments();i++)
    {
      if(!(g.fData->deps[i]==fData->deps[i]))
	return g;
    }
  
  for(uint8_t i = 0;i<fData->nArgs;i++)
    {
      if(!(g.fData->args[i]==fData->args[i]))
	return g;
    }
  
  return *this;
}

void Function::set_arguments(Function* fArgs, uint8_t nArgs)
{
  for(uint8_t i = 0;i<nArgs;i++)
    {
      if(fArgs[i]==*this)
	return;
      for(uint8_t j = 0;j<nArgs;j++)
	if(i!=j&&fArgs[i].is_function_of(fArgs[j]))
	  return;
    }
  fData->args.clear();
  for (uint8_t i = 0;i<nArgs;i++)
    fData->args.push_back(fArgs[i]);
  fData->nArgs=nArgs;
}

Function* Function::get_arguments() const
{
  if(fData->nArgs>0)
    return &fData->args[0];
  else
    return NULL;
}

Function* Function::get_dependencies() const
{
  if(fData->deps.size()>0)
    return &fData->deps[0];
  else
    return NULL;
}

bool Function::is_function_of(Function f) const
{
  for(uint8_t i = 0;i<fData->op.get_number_of_arguments();i++)
    if(fData->deps[i].is_function_of(f))
      return true;
  return (fData==f.fData);
}

Function Function::get_partial_derivative(Function f) const
{
  if(fData==f.fData)
    return Function(1);
  
  if(!is_function_of(f))
    return Function(0);
  
  Function g;
  
  Operator op = fData->op;
  Function* deps = &fData->deps[0];
  
  int Ndep = 0;
  
  for(uint8_t i = 0;i<op.get_number_of_arguments();i++)
    {
      if(deps[i].is_function_of(f))
	{
	  if(Ndep==0)
	    {
	      g=op.get_partial_derivative(deps,i)*deps[i].get_partial_derivative(f);
	      Ndep++;
	    }
	  else
	    g = g+op.get_partial_derivative(deps,i)*deps[i].get_partial_derivative(f);
	}
    }
  return g;
}

bool Function::operator==(const Function& f) const
{
  return fData==f.fData;
}

bool Function::operator!=(const Function& f) const
{
  return fData!=f.fData;
}

Function Function::operator+(const Function& f) const
{
  Operator plus;
  Operator::get_operator("+",plus);
  Function F[]={*this,f};
  return plus(F);
}

Function Function::operator-(const Function& f) const
{
  Operator minus;
  Operator::get_operator("-",minus);
  Function F[]={*this,f};
  return minus(F);
}

Function Function::operator*(const Function& f) const
{
  Operator mult;
  Operator::get_operator("*",mult);
  Function F[]={*this,f};
  return mult(F);
}

Function Function::operator/(const Function& f) const
{
  Operator div;
  Operator::get_operator("/",div);
  Function F[]={*this,f};
  return div(F);
}

Function Function::operator-() const
{
  Operator mult;
  Operator::get_operator("*",mult);
  Function F[]={-1,*this};
  return mult(F);
}

  } /* end namespace num */
} /* end namespace scigma */
