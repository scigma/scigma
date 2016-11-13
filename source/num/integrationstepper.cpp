#include "integrationstepper.hpp"

namespace scigma
{
  namespace num
  {
    IntegrationStepper::IntegrationStepper(const EquationSystem& eqsys, double dt, bool stiff,
					   double aTol, double rTol, size_t maxIter, bool computeJacobian):
      nVar_(eqsys.n_variables()),nFunc_(eqsys.n_functions()),t0_(eqsys.is_autonomous()?0:eqsys.time()),
      odessa_(nVar_,eqsys.f_pt(),eqsys.dfdx_pt(),eqsys.dfdx_t()?nVar_+eqsys.n_parameters():eqsys.n_parameters(),NULL,stiff,aTol,rTol,maxIter,computeJacobian),
      x_(odessa_.x()),p_(odessa_.p()),jac_(odessa_.sensitivity()),dt_(dt),func_t_(eqsys.func_t()),funcVals_(nFunc_)
    {
      odessa_.t()=t0_;
      for(size_t i(0);i!=nVar_;++i)
	x_[i]=eqsys.variable_values()[i];
      for(size_t i(0);i!=eqsys.n_parameters();++i)
	p_[i]=eqsys.parameter_values()[i];
      if(jac_)
	{
	  for(size_t i(0);i!=nVar_*nVar_;++i)
	    jac_[i]=(i%(nVar_+1))?0:1;
	}
    }
    
    double IntegrationStepper::t() const {return odessa_.t();}
    const double* IntegrationStepper::x() const {return x_;}
    const double* IntegrationStepper::func() const {return &funcVals_[0];}
    const double* IntegrationStepper::jac() const {return jac_;}

    void IntegrationStepper::reset(double t, const double* x)
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
    
    void IntegrationStepper::advance(size_t n)
    {
      odessa_.integrate(dt_,n);
      if(nFunc_)
	func_t_(odessa_.t(),x_,&funcVals_[0]);
    }

    size_t IntegrationStepper::n_variables() const {return nVar_;}
    size_t IntegrationStepper::n_functions() const {return nFunc_;}
    
  } /* end namespace num */
} /* end namespace scigma */

