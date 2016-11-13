#ifndef SCIGMA_NUM_STEPPER_HPP
#define SCIGMA_NUM_STEPPER_HPP

#include <stddef.h>

namespace scigma
{
  namespace num
  {
    class Stepper
    {
    public:
      Stepper();
      virtual ~Stepper();

      virtual double t() const =0;
      virtual const double* x() const =0;
      virtual const double* func() const = 0;
      virtual const double* jac() const = 0;
      virtual void reset(double t, const double* x)=0;

      virtual void advance(size_t n=1)=0;

      virtual size_t n_variables() const = 0;
      virtual size_t n_functions() const = 0;
   
    private:
      Stepper(const Stepper&);
      Stepper& operator=(const Stepper&); 
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_STEPPER_HPP*/
