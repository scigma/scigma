#ifndef __SCIGMA_NUM_EXTERNALODESSASTEPPER_H__
#define __SCIGMA_NUM_EXTERNALODESSASTEPPER_H__

#include <vector>
#include "definitions.h"
#include "stepper.h"
#include "odessa.h"

namespace scigma
{
  namespace num
  {

    class ExternalOdessaStepper:public Stepper
    {
    public:
      ExternalOdessaStepper(double dt, size_t nVar, size_t nFunc, Odessa::F f, Odessa::DFDX dfdx, F_t fFunc,
			    double t, const double* x, bool stiff, double aTol, double rTol, size_t maxIter);

      double t() const;
      double x(size_t index) const;
      double func(size_t index) const;
      double jac(size_t index) const;

      void reset(const double *x);

      void advance(size_t n=1);

    private:
      ExternalOdessaStepper();
      ExternalOdessaStepper(const ExternalOdessaStepper&);
      ExternalOdessaStepper& operator=(const ExternalOdessaStepper&);

      double dt_;

      F_t fFunc_;

      Odessa odessa_;
      double* x_;
      double* jac_;
      VecD func_;

      char padding_[8];
    };



  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_EXTERNALODESSASTEPPER_H__ */
