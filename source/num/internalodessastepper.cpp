#include <iostream>
#include "internalodessastepper.h"
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {

    InternalOdessaStepper::InternalOdessaStepper(double dt, const Function& tFunc, 
						 const VecF& xFuncs, const VecF& rhsFuncs, const VecF& funcFuncs, 
						 bool stiff, double aTol, double rTol, size_t maxIter):
      Stepper(xFuncs.size(),funcFuncs.size(),tFunc.evaluate()),dt_(dt),tFunc_(tFunc), xFuncs_(xFuncs),rhsFuncs_(rhsFuncs),funcFuncs_(funcFuncs),
      odessa_(nVar,odessa_f(tFunc_,xFuncs_,rhsFuncs_),
	      odessa_dfdx(xFuncs_,rhsFuncs_),nVar,NULL,stiff,aTol,rTol,maxIter),x_(odessa_.x()),jac_(odessa_.sensitivity())
    {
      odessa_.t()=t0_;
      for(size_t i(0);i!=nVar;++i)
	x_[i]=xFuncs_[i].evaluate();
      for(size_t i(0);i!=nVar*nVar;++i)
	jac_[i]=(i%(nVar+1))?0:1;
    }
      
    double InternalOdessaStepper::t() const {return odessa_.t();}
    double InternalOdessaStepper::x(size_t index) const {return x_[index];}
    double InternalOdessaStepper::func(size_t index) const {return funcFuncs_[index].evaluate();}
    double InternalOdessaStepper::jac(size_t index) const {return jac_[index];}

    void InternalOdessaStepper::reset(const double* x)
    {
      odessa_.reset();
      odessa_.t()=t0_;
      for(size_t i(0);i<nVar;++i)
	x_[i]=x[i];
      for(size_t i(0);i<nVar*nVar;++i)
	jac_[i]=(i%(nVar+1))?0:1;
    }

    void InternalOdessaStepper::advance(size_t n)
    {
      odessa_.integrate(dt_,n);

      if(!nFunc)
	return;

      tFunc_.set_value(odessa_.t());
      for(size_t i(0);i!=nVar;++i)
	xFuncs_[i].set_value(x_[i]);
    }
    
  } /* end namespace num */
} /* end namespace scigma */

