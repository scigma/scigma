#include <iostream>
#include <sstream>
#include "definitions.h"
#include "newton.h"
#include "internalpoincarestepper.h"
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {

    InternalPoincareStepper::InternalPoincareStepper(double maxtime, double dt, int secvar, int secdir, double secval, double tol,
						     const Function& tFunc, const VecF& xFuncs, const VecF& rhsFuncs, const VecF& funcFuncs,   
						     bool stiff, double aTol, double rTol, size_t maxIter):
      Stepper(xFuncs.size(),funcFuncs.size(),tFunc.evaluate()),dt_(dt),maxtime_(maxtime),secvar_(secvar),secdir_(secdir),secval_(secval),tol_(tol),
      tFunc_(tFunc), xFuncs_(xFuncs),rhsFuncs_(rhsFuncs),funcFuncs_(funcFuncs),
      odessa_(nVar,odessa_f(tFunc_,xFuncs_,rhsFuncs_),
	      odessa_dfdx(xFuncs_,rhsFuncs_),nVar,NULL,stiff,aTol,rTol,maxIter),x_(odessa_.x()),jac_(odessa_.sensitivity()), jac2_(nVar*nVar)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i!=nVar;++i)
	  x_[i]=xFuncs_[i].evaluate();
      for(size_t i(0);i!=nVar*nVar;++i)
	jac_[i]=(i%(nVar+1))?0:1;
    }
      
    double InternalPoincareStepper::t() const {return odessa_.t();}
    double InternalPoincareStepper::x(size_t index) const {return x_[index];}
    double InternalPoincareStepper::func(size_t index) const {return funcFuncs_[index].evaluate();}
    double InternalPoincareStepper::jac(size_t index) const {return jac2_[index];}
    
    void InternalPoincareStepper::reset(const double* x)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i<nVar;++i)
	x_[i]=x[i];
      for(size_t i(0);i<nVar*nVar;++i)
	jac_[i]=(i%(nVar+1))?0:1;
    }

    void InternalPoincareStepper::advance(size_t n)
    {
      while(n--!=0)
	{
	  odessa_.reset();
	  double t(0.0);
	  double oldValue(secval_); // makes sure that we do not detect the same intersection over and over again by starting just below value
	  double newValue(0);
	  while((t+=dt_)<=maxtime_)
	    {
	      odessa_.integrate(dt_);
	      newValue=x_[secvar_];
	      if(newValue*secdir_>=secval_*secdir_&&oldValue*secdir_<secval_*secdir_)
		break;
	      oldValue=newValue;
	    }
	  if(t>=maxtime_)
	    {
	      std::stringstream ss;
	      ss<<"no intersection detected after "<<maxtime_<<" seconds\n";
	      throw(ss.str());
	    }
	  double oldDT(0);
	  Newton::F diff = [this,&oldDT](const double* DT, double* delta)
	    {
	      odessa_.reset();
	      odessa_.integrate(*DT-oldDT);
	      *delta=x_[secvar_]-secval_;
	      oldDT=*DT;
	    };
	  double DT(dt_*(newValue-secval_)/(oldValue-newValue));
	  if(!newton(1,&DT,diff,true,tol_))
	    {
	      std::stringstream ss;
	      ss<<"intersection detected but Newton iteration did not converge\n";
	      throw(ss.str());
	    }
	}

      tFunc_.set_value(odessa_.t());
      for(size_t i(0);i!=nVar;++i)
	xFuncs_[i].set_value(x_[i]);
      for(size_t i(0);i<nVar*nVar;++i)
	jac2_[i]=jac_[i]-rhsFuncs_[i%nVar].evaluate()/rhsFuncs_[size_t(secvar_)].evaluate()*jac_[size_t(secvar_)+nVar*(i/nVar)];
      for(size_t i(0);i<nVar;++i)
	jac2_[size_t(secvar_)*nVar+i]=jac2_[i*nVar+size_t(secvar_)]=0;
    }
    
  } /* end namespace num */
} /* end namespace scigma */

