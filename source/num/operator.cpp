#include <cmath>
#include <stdlib.h>
#include "operator.hpp"
#include "function.hpp"

namespace scigma
{
  namespace num
  {

#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"
  
    double ID(double d)
    {
      return d;
    }
    
    Function dID(Function x)
    {
      (void)x;
      return 1;
    }
    
    double PLUS(double* args)
    {
      return args[0]+args[1];
    }
    
    Function dPLUS(Function* args, uint8_t whichArg)
    {
      (void)args;
      (void)whichArg;
      return 1;
    }
    
    double MINUS(double* args)
    {
      return args[0]-args[1];
    }
    
    Function dMINUS(Function* args,  uint8_t whichArg)
    {
      (void)args;
      if(whichArg==0)
	return 1;
      else if(whichArg==1)
	return -1;
      return 0;
    }
    
    double MULT(double* args)
    {
      return args[0]*args[1];
    }
    
    Function dMULT(Function* args, uint8_t whichArg)
    {
      if(whichArg==0)
	return args[1];
      else if(whichArg==1)
	return args[0];
      return 0;
    }
    
    double DIV(double* args)
    {
      return args[0]/args[1];
    }
    
    Function dDIV(Function* args, uint8_t whichArg)
    {
      if(whichArg==0)
	return 1/args[1];
      else if(whichArg==1)
	return -args[0]/(args[1]*args[1]);
      return 0;
    }
    
    double POW(double* args)
    {
      return pow(args[0],args[1]);
    }
    
    Function dPOW(Function* args, uint8_t whichArg)
    {
      Operator power;
      Operator::get_operator("^",power);
      if(whichArg==0)
	{
	  Function F[]={args[0],args[1]-1};
	  return args[1]*power(F);
	}
      else if(whichArg==1)
	{
	  Function F[]={args[0],args[1]};
	  Operator ln;
	  Operator::get_operator("ln",ln);
	  return ln(args[0])*power(F);
	}
      return 0;
    }
    
    double SIGMOID(double* args)
    {
      return 1/(exp(-args[0]/args[1])+1);
    }
    
    Function dSIGMOID(Function* args, uint8_t whichArg)
    {
      Operator EXP;
      Operator::get_operator("exp",EXP);
      Function expo(args[0]/args[1]);
      Function root(EXP(expo)+1);
      Function ret(EXP(expo)/args[1]/root/root);
      if(0==whichArg)
	return ret;
      else
	return ret*args[0]/args[1];
    }
    
    double PULSE(double* args)
    {
      double expo(args[0]/args[1]);
      double root(exp(expo)+1);
      return exp(expo)/args[1]/root/root;
    }
    
    Function dPULSE(Function* args, uint8_t whichArg)
    {
      Operator EXP;
      Operator::get_operator("exp",EXP);
      Function expo(args[0]/args[1]);
      Function e(EXP(expo));
      Function root3(e+1);
      
      if(whichArg==0)
	return e*(e-1)/args[1]/args[1]/root3/root3/root3;
      else
	return e*(args[0]*e-args[1]*e-args[1]-args[0])/args[1]/args[1]/args[1]/root3/root3/root3; 
    }
    
    double SIGN(double d)
    {
      if(d<0.0)
	return -1;
      else if(d>0.0)
	return 1;
      else return 0;
    }

    Function dSIGN(Function x)
    {
      return 0;
    }
    
    double ABS(double d)
    {
      if(d>0.0)
	return d;
      else
	return -d;
    }

    Function dABS(Function x)
    {
      Operator signum;
      Operator::get_operator("sign",signum);
      return signum(x);
    }
    
    double STEP(double* args)
    {
      if(args[1]>args[0])
	return 0.0;
      else if(args[1]<args[0])
	return 1.0;
      else
	return 0.5;
    }
    
    Function dSTEP(Function* args, uint8_t whichArg)
    {
      return 0;
    }

    double ATAN2(double* args)
    {
      return atan2(args[0],args[1]);
    }
    
    Function dATAN2(Function* args, uint8_t whichArg)
    {
      Function rsquare=args[0]*args[0]+args[1]*args[1];
      if(whichArg==0)
	return -args[1]/rsquare;
      else
	return args[0]/rsquare;
    }
    
    Function dSIN(Function x)
    {
      Operator cosinus;
      Operator::get_operator("cos",cosinus);
      return cosinus(x);
    }
    
    Function dCOS(Function x)
    {
      Operator sinus;
      Operator::get_operator("sin",sinus);
      return -sinus(x);
    }
    
    Function dTAN(Function x)
    {
      Operator cosinus;
      Operator::get_operator("cos",cosinus);
      return 1/(cosinus(x)*cosinus(x));
    }
    
    Function dASIN(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return 1/sqroot(1-x*x);
    }
    
    Function dACOS(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return -1/sqroot(1-x*x);
    }
    
    Function dATAN(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return 1/sqroot(x*x+1);
    }
    
    Function dSINH(Function x)
    {
      Operator coshyp;
      Operator::get_operator("cosh",coshyp);
      return coshyp(x);
    }
    
    Function dCOSH(Function x)
    {
      Operator sinhyp;
      Operator::get_operator("sinh",sinhyp);
      return sinhyp(x);
    }
    
    Function dTANH(Function x)
    {
      Operator coshyp;
      Operator::get_operator("cosh",coshyp);
      return 1/(coshyp(x)*coshyp(x));
    }
    
    Function dASINH(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return 1/sqroot(x*x+1);
    }
    
    Function dACOSH(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return 1/sqroot(x*x-1);
    }
    
    Function dATANH(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return 1/(x*x-1);
    }
    
    Function dSQRT(Function x)
    {
      Operator sqroot;
      Operator::get_operator("sqrt",sqroot);
      return 1/(2*sqroot(x));
    }
    
    Function dEXP(Function x)
    {
      Operator expo;
      Operator::get_operator("exp",expo);
      return expo(x);
    }
    
    Function dLOG(Function x)
    {
      return 1/x;
    }
    
    Function dLOG10(Function x)
    {
      return 1/(log(10)*x);
    }

    double FLOOR(double d)
    {
      return floor(d);
    }

    Function dFLOOR(Function x)
    {
      return 0;
    }

    double MOD(double* args)
    {
      return fmod(args[0],args[1]);
    }
    
    Function dMOD(Function* args, uint8_t whichArg)
    {
      if(whichArg==0)
	return 1;
      else
	{
	  Operator fl;
	  Operator::get_operator("floor",fl);
	  return -fl(args[0]/args[1]);
	}
    }

    
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::vector <Operator> Operator::operators;
#pragma clang diagnostic pop
    
    bool Operator::isInitialized(false);
    
    void Operator::init_operators()
    {
      if(isInitialized)
	return;
      define_operator("ID",ID,dID);
      define_operator("+",PLUS,dPLUS,2);
      define_operator("-",MINUS,dMINUS,2);
      define_operator("*",MULT,dMULT,2);
      define_operator("/",DIV,dDIV,2);
      define_operator("^",POW,dPOW,2);
      define_operator("sin",sin,dSIN);
      define_operator("cos",cos,dCOS);
      define_operator("tan",tan,dTAN);
      define_operator("asin",asin, dASIN);
      define_operator("acos", acos, dACOS);
      define_operator("atan", atan, dATAN);
      define_operator("sinh", sinh, dSINH);
      define_operator("cosh", cosh, dCOSH);
      define_operator("tanh", tanh, dTANH);
      define_operator("asinh",asinh,dASINH);
      define_operator("acosh",acosh,dACOSH);
      define_operator("atanh",atanh,dATANH);
      define_operator("sqrt",sqrt,dSQRT);
      define_operator("pow",POW,dPOW,2);
      define_operator("exp",exp,dEXP);
      define_operator("ln",log,dLOG);
      define_operator("log10",log10,dLOG10);
      define_operator("atan2",ATAN2,dATAN2,2);
      define_operator("sigmoid",SIGMOID,dSIGMOID,2);
      define_operator("pulse",PULSE, dPULSE,2);
      define_operator("sign",SIGN,dSIGN);
      define_operator("abs",ABS,dABS);
      define_operator("step",STEP,dSTEP,2);
      define_operator("floor",FLOOR,dFLOOR);
      define_operator("mod",MOD,dMOD,2);
      isInitialized=true;
      atexit(clean_up_operators);
    }
    
    void Operator::clean_up_operators()
    {
      std::vector<Operator>::iterator i = operators.begin();
      while(i!=operators.end())
	delete (i++)->opData;
    }
    
    OperatorData::OperatorData()
    {
      name="";
      f_one=NULL;
      f_arbitrary=NULL;
      df_one=NULL;
      df_arbitrary=NULL;
      nArgs=0;  
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
    OperatorData::OperatorData(std::string name,double (*funct)(double),Function (*dfunct)(Function))
    {
      this->name=name;
      f_one=funct;
      f_arbitrary=NULL;
      df_one=dfunct;
      df_arbitrary=NULL;
      nArgs=1;
    }
    
    OperatorData::OperatorData(std::string name,double (*funct)(double*),Function (*dfunct)(Function*, uint8_t whichArg), uint8_t nArgs)
    {
      this->name=name;
      f_one=NULL;
      f_arbitrary=funct;
      df_one=NULL;
      df_arbitrary=dfunct;
      this->nArgs=nArgs;	
    }
#pragma clang diagnostic pop
    
    Operator::Operator()
    {
      opData=NULL;
    }
    
    Operator::Operator(std::string name,double(*funct)(double),Function (*dfunct)(Function))
    {
      opData=new OperatorData(name,funct,dfunct);
    }
    
    Operator::Operator(std::string name,double(*funct)(double*),Function (*dfunct)(Function*,uint8_t),uint8_t nArgs)
    {
      opData= new OperatorData(name,funct,dfunct,nArgs);
    }
    
    uint8_t Operator::get_number_of_arguments() const
    {
      if(opData)
	return opData->nArgs;
      return 0;
    }
    
    std::string Operator::get_name() const
    {
      if(opData)
	return opData->name;
      return "";
    }
    
    double Operator::operator()(double dArg) const
    {
      if(opData&&opData->f_one)
	return opData->f_one(dArg);
      else
	return 0;
    }
    
    double Operator::operator()(double* dArgs) const
    {
      if(opData)
	{
	  if(opData->f_one)
	    return opData->f_one(*dArgs);
	  else
	    return opData->f_arbitrary(dArgs);
	}
      return 0;
    }
    
    Function Operator::operator()(Function fArg) const
    {
      if(opData)
	return Function(*this,&fArg);
      return Function(0);
    }
    
    Function Operator::operator()(Function* fArgs) const
    {
      if(opData)
	return Function(*this,fArgs);
      return Function(0);
    }
    
    Function Operator::get_partial_derivative(Function x) const
    {
      if(opData&&opData->df_one)
	return opData->df_one(x);
      return Function(0);
    }
    
    Function Operator::get_partial_derivative(Function* args, uint8_t whichArg) const
    {
      if(opData&&whichArg<opData->nArgs)
	{
	  if(opData->df_one&&whichArg==0)
	    return opData->df_one(*args);
	  else
	    return opData->df_arbitrary(args,whichArg);
	}
      return Function(0);
    }
    
    bool Operator::define_operator(std::string name,double(*funct)(double),Function (*dfunct)(Function))
    {
      Operator op;
      if(get_operator(name,op))
	return false;
      op = Operator(name, funct, dfunct);
      operators.push_back(op);
      return true;
    }
    
    bool Operator::define_operator(std::string name,double(*funct)(double*),Function (*dfunct)(Function*,uint8_t),uint8_t nArgs)
    {
      Operator op;
      if(get_operator(name,op))
	return false;
      op = Operator(name, funct, dfunct,nArgs);
      operators.push_back(op);
      return true;
    }
    
    bool Operator::undefine_operator(std::string name)
    {
      std::vector<Operator>::iterator i = operators.begin();
      while(i!=operators.end())
	{
	  if(i->opData&&i->opData->name==name)
	    {
	      delete i->opData;
	      operators.erase(i);
	      return true;
	    }
	  ++i;
	}
      return false;
    }
    
    bool Operator::get_operator(std::string name, Operator &op)
    {
      std::vector<Operator>::iterator i = operators.begin();
      while(i!=operators.end())
	{
	  if(i->opData&&i->opData->name==name)
	    {
	      op=*i;
	      return true;
	    }
	  ++i;
	}
      return false;
    }
    
    bool Operator::operator==(const Operator& op) const
    {
      return(op.opData==opData);
    }
    
  } /* end namespace num */
} /* end namespace scigma */
