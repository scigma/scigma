#ifndef __SCIGMA_NUM_EXTERNALPOINCARESTEPPER_H__
#define __SCIGMA_NUM_EXTERNALPOINCARESTEPPER_H__

#include <vector>
#include "definitions.h"
#include "stepper.h"
#include "odessa.h"

namespace scigma
{
  namespace num
  {

    class ExternalPoincareStepper:public Stepper
    {
    public:
      ExternalPoincareStepper(double maxtime, double dt, int secvar, int secdir, double secval, double tol,
			      size_t nVar, size_t nFunc, Odessa::F f, Odessa::DFDX dfdx, F_t fFunc,
			      double t, const double* x, bool stiff, double aTol, double rTol, size_t maxIter);

      double t() const;
      double x(size_t index) const;
      double func(size_t index) const;
      double jac(size_t index) const;
      void reset(const double* x);

      void advance(size_t n=1);

    private:
      ExternalPoincareStepper();
      ExternalPoincareStepper(const ExternalPoincareStepper&);
      ExternalPoincareStepper& operator=(const ExternalPoincareStepper&);

      double dt_;
      double maxtime_;
      int secvar_;
      int secdir_;
      double secval_;
      double tol_;

      Odessa::F f_;
      F_t fFunc_;

      Odessa odessa_;
      double* x_;
      double* jac_;
      VecD rhs_;
      VecD jac2_;
      VecD func_;

      char padding_[8];
    };



  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_INTERNALPOINCARESTEPPER_H__ */
