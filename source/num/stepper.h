#ifndef __SCIGMA_NUM_STEPPER_H__
#define __SCIGMA_NUM_STEPPER_H__

#include <stddef.h>

namespace scigma
{
  namespace num
  {

    class Stepper
    {
    public:
      Stepper(size_t nVar_, size_t nFunc_, double t0);
      virtual ~Stepper();

      virtual double t() const =0;
      virtual double x(size_t index) const =0;
      virtual double func(size_t index) const = 0;
      virtual double jac(size_t index) const = 0;
      virtual void reset(const double* x)=0;

      virtual void advance(size_t n=1)=0;

      const size_t nVar;
      const size_t nFunc;
   
    protected:
      double t0_;
      char padding_[8];
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_STEPPER_H__ */
