#include <iostream>
#include <sstream>
#include "definitions.h"
#include "newton.h"
#include "externalpoincarestepper.h"
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {
    
    ExternalPoincareStepper::ExternalPoincareStepper(double maxtime, double dt, int secvar, int secdir, double secval, double tol,
						     size_t nVar, size_t nFunc, Odessa::F f, Odessa::DFDX dfdx, F_t fFunc,
						     double t, const double* x, bool stiff, double aTol, double rTol, size_t maxIter):
    Stepper(nVar,nFunc,t),dt_(dt),maxtime_(maxtime),secvar_(secvar),secdir_(secdir),secval_(secval),tol_(tol),f_(f),fFunc_(fFunc),
      odessa_(nVar,f,dfdx,nVar,NULL,stiff,aTol,rTol,maxIter),x_(odessa_.x()),jac_(odessa_.sensitivity()),rhs_(nVar),jac2_(nVar*nVar),func_(nFunc)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i!=nVar;++i)
	x_[i]=x[i];
      for(size_t i(0);i!=nVar*nVar;++i)
	jac_[i]=(i%(nVar+1))?0:1;
    }
      
    double ExternalPoincareStepper::t() const {return odessa_.t();}
    double ExternalPoincareStepper::x(size_t index) const {return x_[index];}
    double ExternalPoincareStepper::func(size_t index) const {return func_[index];}
    double ExternalPoincareStepper::jac(size_t index) const {return jac2_[index];}
    
    void ExternalPoincareStepper::reset(const double* x)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i<nVar;++i)
	x_[i]=x[i];
      for(size_t i(0);i<nVar*nVar;++i)
	jac_[i]=(i%(nVar+1))?0:1;
    }

    void ExternalPoincareStepper::advance(size_t n)
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

      f_(odessa_.t(),x_,NULL,&rhs_[0]);
      for(size_t i(0);i<nVar*nVar;++i)
	jac2_[i]=jac_[i]-rhs_[i%nVar]/rhs_[size_t(secvar_)]*jac_[size_t(secvar_)+nVar*(i/nVar)];
      for(size_t i(0);i<nVar;++i)
	jac2_[size_t(secvar_)*nVar+i]=jac2_[i*nVar+size_t(secvar_)]=0;
    
      if(!nFunc)
	return;
      fFunc_(odessa_.t(),x_,&func_[0]);
    }
    
  } /* end namespace num */
} /* end namespace scigma */

