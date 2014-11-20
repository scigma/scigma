#include "stepper.h"

namespace scigma
{
  namespace num
  {

    Stepper::Stepper(bool computeJacobian, size_t nVar_,size_t nFunc_, double t0):
      nVar(nVar_),nFunc(nFunc_),t0_(t0),computeJacobian_(computeJacobian)
    {}
    
    Stepper::~Stepper()
    {}
 
 } /* end namespace num */
} /* end namespace scigma */

