#include <algorithm>
#include <iostream>
#include "mapstepper.hpp"

namespace scigma
{
  namespace num
  {

    MapStepper::MapStepper(const EquationSystem& eqsys, bool forward):
      nVar_(eqsys.n_variables()), nFunc_(eqsys.n_functions()),
      t_(eqsys.is_autonomous()?0:eqsys.time()), forward_(forward?1:0),
      xVals_(2*nVar_),x_(&xVals_[0]), xNew_(&xVals_[nVar_]),f_t_(eqsys.f_t()),
      jacVals_(eqsys.dfdx_t()?3*nVar_*nVar_:0), jac_(eqsys.dfdx_t()?&jacVals_[0]:NULL),
      jacNew_(eqsys.dfdx_t()?&jacVals_[nVar_*nVar_]:NULL),
      jacTemp_(eqsys.dfdx_t()?&jacVals_[2*nVar_*nVar_]:NULL),
      dfdx_t_(eqsys.dfdx_t()),
      funcVals_(nFunc_),func_t_(eqsys.func_t())
      {
       	for(size_t i(0);i!=nVar_;++i)
	  x_[i]=eqsys.variable_values()[i];
	if(dfdx_t_)
	  for(size_t i(0);i!=nVar_*nVar_;++i)
	    jac_[i]=(i%(nVar_+1))?0:1;
      }
    
    double MapStepper::t() const {return t_;}
    const double* MapStepper::x() const {return x_;}
    const double* MapStepper::func() const {return nFunc_?&funcVals_[0]:NULL;}
    const double* MapStepper::jac() const {return jac_;}

    void MapStepper::reset(double t, const double* x)
    {
      t_=t;
      for(size_t i(0);i<nVar_;++i)
	x_[i]=x[i];

      if(nFunc_)
	func_t_(t_,x_,&funcVals_[0]);

      if(dfdx_t_)
	for(size_t i(0);i!=nVar_*nVar_;++i)
	  jac_[i]=(i%(nVar_+1))?0:1;
    
    }

    void MapStepper::advance(size_t n)
    {
      for(size_t i(0);i<n;++i)
	{
	  if(dfdx_t_)
	    {
	      /* df_i(f(x))/dx_j=sum over k df_i/dx_k| f(x) * df_k/dx_j |x 
		 translates to 
		 newJac[i+j*nVar]=jacFuncs_[i+k*nVar]*jac_[k+j*nVar]
	       */
	      dfdx_t_(t_,x_,jacTemp_);
	      for(size_t ii(0);ii<nVar_;++ii)
		{
		  for(size_t j(0);j<nVar_;++j)
		    {
		      jacNew_[ii+j*nVar_]=0;
		      for(size_t k(0);k<nVar_;++k)
			jacNew_[ii+j*nVar_]+=jacTemp_[ii+k*nVar_]*jac_[k+j*nVar_];
		    }
		}
	      std::swap(jac_,jacNew_);
	    }
	  f_t_(t_,x_,xNew_);
	  std::swap(x_,xNew_);
	  t_+=forward_?1.0:-1.0;
	  if(nFunc_)
	    func_t_(t_,x_,&funcVals_[0]);
	}
    }

    size_t MapStepper::n_variables() const {return nVar_;}
    size_t MapStepper::n_functions() const {return nFunc_;}
    
  } /* end namespace num */
} /* end namespace scigma */

