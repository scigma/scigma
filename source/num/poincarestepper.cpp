#include <iostream>
#include <sstream>
#include "poincarestepper.hpp"
#include "newton.hpp"

namespace scigma
{
  namespace num
  {
    PoincareStepper::PoincareStepper(const EquationSystem& eqsys, double dt, double maxtime, int secvar, int secdir, double secval,
				     double tol, bool stiff,double aTol, double rTol, size_t maxIter, bool computeJacobian):
      nVar_(eqsys.n_variables()),nFunc_(eqsys.n_functions()),t0_(eqsys.is_autonomous()?0:eqsys.time()),
      odessa_(nVar_,eqsys.f_t(),eqsys.dfdx_t(),stiff,aTol,rTol,maxIter,computeJacobian),
      x_(odessa_.x()),jac_(odessa_.sensitivity()),f_t_(eqsys.f_t()),func_t_(eqsys.func_t()),
      rhs_(nVar_),jac2_(nVar_*nVar_),funcVals_(nFunc_),
      dt_(dt),maxtime_(maxtime), secvar_(secvar), secdir_(secdir), secval_(secval), tol_(tol)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i!=nVar_;++i)
	x_[i]=eqsys.variable_values()[i];
      if(jac_)
	{
	  for(size_t i(0);i!=nVar_*nVar_;++i)
	    jac_[i]=(i%(nVar_+1))?0:1;
	}
    }
    
    double PoincareStepper::t() const {return odessa_.t();}
    const double* PoincareStepper::x() const {return x_;}
    const double* PoincareStepper::func() const {return &funcVals_[0];}
    const double* PoincareStepper::jac() const {return &jac2_[0];}

    void PoincareStepper::reset(double t, const double* x)
    {
      odessa_.reset();
      odessa_.t()=t;
      for(size_t i(0);i<nVar_;++i)
	x_[i]=x[i];
      if(jac_)
	{
	  for(size_t i(0);i<nVar_*nVar_;++i)
	    jac_[i]=(i%(nVar_+1))?0:1;
	}
      if(nFunc_)
	func_t_(odessa_.t(),x_,&funcVals_[0]);
    }
    
    void PoincareStepper::advance(size_t n)
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

      if(jac_)
	{
	  f_t_(odessa_.t(),x_,&rhs_[0]);
	  for(size_t i(0);i<nVar_*nVar_;++i)
	    jac2_[i]=jac_[i]-rhs_[i%nVar_]/rhs_[size_t(secvar_)]*jac_[size_t(secvar_)+nVar_*(i/nVar_)];
	  for(size_t i(0);i<nVar_;++i)
	    jac2_[size_t(secvar_)*nVar_+i]=jac2_[i*nVar_+size_t(secvar_)]=0;
	}

      if(!nFunc_)
	return;
      func_t_(odessa_.t(),x_,&funcVals_[0]);
    }

      
    size_t PoincareStepper::n_variables() const {return nVar_;}
    size_t PoincareStepper::n_functions() const {return nFunc_;}
    
  } /* end namespace num */
} /* end namespace scigma */

