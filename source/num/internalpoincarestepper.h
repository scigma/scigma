#ifndef __SCIGMA_NUM_INTERNALPOINCARESTEPPER_H__
#define __SCIGMA_NUM_INTERNALPOINCARESTEPPER_H__

#include <vector>
#include "definitions.h"
#include "stepper.h"
#include "odessa.h"

namespace scigma
{
  namespace num
  {

    class InternalPoincareStepper:public Stepper
    {
    public:
      InternalPoincareStepper(bool computeJacobian, double maxtime, double dt, int secvar,
			      int secdir, double secval, double tol,
			      const Function& tFunc, const VecF& xFuncs, const VecF& rhsFuncs, 
			      const VecF& funcFuncs, bool stiff, double aTol, double rTol, size_t maxIter);

      double t() const;
      double x(size_t index) const;
      double func(size_t index) const;
      double jac(size_t index) const;
      void reset(const double* x);

      void advance(size_t n=1);

    private:
      InternalPoincareStepper();
      InternalPoincareStepper(const InternalPoincareStepper&);
      InternalPoincareStepper& operator=(const InternalPoincareStepper&);

      double dt_;
      double maxtime_;
      int secvar_;
      int secdir_;
      double secval_;
      double tol_;

      Function tFunc_;
      std::vector<Function> xFuncs_;
      std::vector<Function> rhsFuncs_;
      std::vector<Function> funcFuncs_;

      Odessa odessa_;
      double* x_;
      double* jac_;
      VecD jac2_;

      char padding_[8];
    };



  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_INTERNALPOINCARESTEPPER_H__ */
