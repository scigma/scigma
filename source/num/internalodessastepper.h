#ifndef __SCIGMA_NUM_INTERNALODESSASTEPPER_H__
#define __SCIGMA_NUM_INTERNALODESSASTEPPER_H__

#include <vector>
#include "definitions.h"
#include "stepper.h"
#include "odessa.h"

namespace scigma
{
  namespace num
  {

    class InternalOdessaStepper:public Stepper
    {
    public:
      InternalOdessaStepper(bool computeJacobian, double dt, const Function& tFunc, 
			    const VecF& xFuncs, const VecF& rhsFuncs, const VecF& funcFuncs, 			     
			    bool stiff, double aTol, double rTol, size_t maxIter);

      double t() const;
      double x(size_t index) const;
      double func(size_t index) const;
      double jac(size_t index) const;

      void reset(const double *x);

      void advance(size_t n=1);

    private:
      InternalOdessaStepper();
      InternalOdessaStepper(const InternalOdessaStepper&);
      InternalOdessaStepper& operator=(const InternalOdessaStepper&);

      double dt_;

      Function tFunc_;
      std::vector<Function> xFuncs_;
      std::vector<Function> rhsFuncs_;
      std::vector<Function> funcFuncs_;

      Odessa odessa_;
      double* x_;
      double* jac_;
    };



  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_INTERNALODESSASTEPPER_H__ */
