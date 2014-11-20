#include <iostream>
#include "externalodessastepper.h"
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {

    ExternalOdessaStepper::ExternalOdessaStepper(bool computeJacobian, double dt, size_t nVar, size_t nFunc, Odessa::F f, Odessa::DFDX dfdx, F_t fFunc,
						 double t, const double* x, bool stiff, double aTol, double rTol,
						 size_t maxIter):
      Stepper(computeJacobian, nVar,nFunc,t),dt_(dt),fFunc_(fFunc),
      odessa_(nVar,f,dfdx,computeJacobian?nVar:0,NULL,stiff,aTol,rTol,maxIter),
      x_(odessa_.x()),jac_(odessa_.sensitivity()),func_(nFunc)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i!=nVar;++i)
	x_[i]=x[i];
      if(computeJacobian_)
	{
	  for(size_t i(0);i!=nVar*nVar;++i)
	    jac_[i]=(i%(nVar+1))?0:1;
	}
    }
    
    double ExternalOdessaStepper::t() const {return odessa_.t();}
    double ExternalOdessaStepper::x(size_t index) const {return x_[index];}
    double ExternalOdessaStepper::func(size_t index) const {return func_[index];}
    double ExternalOdessaStepper::jac(size_t index) const {return jac_[index];}

    void ExternalOdessaStepper::reset(const double* x)
    {
      odessa_.reset();
      odessa_.t()=t0_;
      for(size_t i(0);i<nVar;++i)
	x_[i]=x[i];
      if(computeJacobian_)
	{
	  for(size_t i(0);i<nVar*nVar;++i)
	    jac_[i]=(i%(nVar+1))?0:1;
	}
    }

    void ExternalOdessaStepper::advance(size_t n)
    {
      odessa_.integrate(dt_,n);
      if(!nFunc)
	return;
      fFunc_(odessa_.t(),x_,&func_[0]);
    }
    
  } /* end namespace num */
} /* end namespace scigma */

