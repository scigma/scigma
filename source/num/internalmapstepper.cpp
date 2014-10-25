#include <iostream>
#include "definitions.h"
#include "internalmapstepper.h"

namespace scigma
{
  namespace num
  {

    InternalMapStepper::InternalMapStepper(const Function& tFunc, const VecF& xFuncs,
					   const VecF& rhsFuncs, const VecF& funcFuncs): 
      Stepper(xFuncs.size(),funcFuncs.size(),tFunc.evaluate()),
      tFunc_(tFunc),xFuncs_(xFuncs),rhsFuncs_(rhsFuncs),funcFuncs_(funcFuncs),work_(xFuncs.size()),
      t0_(tFunc_.evaluate())
    {}
      
    double InternalMapStepper::t() const {return tFunc_.evaluate();}
    double InternalMapStepper::x(size_t index) const {return xFuncs_[index].evaluate();}
    double InternalMapStepper::func(size_t index) const {return funcFuncs_[index].evaluate();}
    double InternalMapStepper::jac(size_t index) const {return 0*index;}

    void InternalMapStepper::reset(const double* x)
    {
      tFunc_.set_value(t0_);
      for(size_t i(0);i<nVar;++i)
	xFuncs_[i].set_value(x[i]);
    }

    void InternalMapStepper::advance(size_t n)
    {
      for(size_t i(0);i<n;++i)
	{
	  for(size_t j(0);j<nVar;++j)
	    work_[j]=rhsFuncs_[j].evaluate();
	  for(size_t j(0);j<nVar;++j)
	    xFuncs_[j].set_value(work_[j]);
	  tFunc_.set_value(tFunc_.evaluate()+1.0);
	}
    }
    
  } /* end namespace num */
} /* end namespace scigma */

