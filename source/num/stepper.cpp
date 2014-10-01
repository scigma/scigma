#include "stepper.h"

namespace scigma
{
  namespace num
  {

    Stepper::Stepper(size_t nVar_,size_t nFunc_, double t0):nVar(nVar_),nFunc(nFunc_),t0_(t0)
    {}
    
    Stepper::~Stepper()
    {}
 
 } /* end namespace num */
} /* end namespace scigma */

