#include <iostream>
#include "definitions.h"
#include "internalmapstepper.h"
#include "functionwrappers.h"

namespace scigma
{
  namespace num
  {

    InternalMapStepper::InternalMapStepper(bool computeJacobian, const Function& tFunc, const VecF& xFuncs,
					   const VecF& rhsFuncs, const VecF& funcFuncs): 
      Stepper(computeJacobian, xFuncs.size(),funcFuncs.size(),tFunc.evaluate()),
      tFunc_(tFunc),xFuncs_(xFuncs),rhsFuncs_(rhsFuncs),funcFuncs_(funcFuncs),work_(xFuncs.size()),jacData_(computeJacobian?2*nVar*nVar:0),jac_(computeJacobian?&jacData_[0]:NULL),
      t0_(tFunc_.evaluate())
    {
      if(computeJacobian_)
	{
	  build_partial_derivative_matrix(xFuncs_,rhsFuncs_,jacFuncs_);
	  for(size_t i(0);i!=nVar*nVar;++i)
	    jac_[i]=(i%(nVar+1))?0:1;
	}
    }
      
    double InternalMapStepper::t() const {return tFunc_.evaluate();}
    double InternalMapStepper::x(size_t index) const {return xFuncs_[index].evaluate();}
    double InternalMapStepper::func(size_t index) const {return funcFuncs_[index].evaluate();}
    double InternalMapStepper::jac(size_t index) const {return jac_[index];}

    void InternalMapStepper::reset(const double* x)
    {
      tFunc_.set_value(t0_);
      for(size_t i(0);i<nVar;++i)
	xFuncs_[i].set_value(x[i]);
      if(computeJacobian_)
	{
	  for(size_t i(0);i!=nVar*nVar;++i)
	    jac_[i]=(i%(nVar+1))?0:1;
	}
    }

    void InternalMapStepper::advance(size_t n)
    {
      double* newJac;
      for(size_t i(0);i<n;++i)
	{
	  if(computeJacobian_)
	    {
	      newJac=&jacData_[0]+(jac_-&jacData_[0]+long(nVar*nVar))%(long(2*nVar*nVar));
	      /* df_i(f(x))/dx_j=sum over k df_i/dx_k| f(x) * df_k/dx_j |x 
		 translates to 
		 newJac[i+j*nVar]=jacFuncs_[i+k*nVar]*jac_[k+j*nVar]
	       */
	      for(size_t ii(0);ii<nVar;++ii)
		{
		  for(size_t j(0);j<nVar;++j)
		    {
		      newJac[ii+j*nVar]=0;
		      for(size_t k(0);k<nVar;++k)
			newJac[ii+j*nVar]+=jacFuncs_[ii+k*nVar].evaluate()*jac_[k+j*nVar];
		    }
		}

	      jac_=newJac;
	    }

	  for(size_t j(0);j<nVar;++j)
	    work_[j]=rhsFuncs_[j].evaluate();
	  for(size_t j(0);j<nVar;++j)
	    xFuncs_[j].set_value(work_[j]);
	  tFunc_.set_value(tFunc_.evaluate()+1.0);
	}
    }
    
  } /* end namespace num */
} /* end namespace scigma */

