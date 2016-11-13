#ifndef SCIGMA_NUM_MAPSTEPPER_HPP
#define SCIGMA_NUM_MAPSTEPPER_HPP

#include <vector>
#include "stepper.hpp"
#include "equationsystem.hpp"

namespace scigma
{
  namespace num
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
    class MapStepper:public Stepper
    {
    public:
      MapStepper(const EquationSystem& eqsys, bool forward=true);
      
      virtual double t() const;
      virtual const double* x() const;
      virtual const double* func() const;
      virtual const double* jac() const;

      virtual void reset(double t,const double *x);
      virtual void advance(size_t n=1);

      virtual size_t n_variables() const;
      virtual size_t n_functions() const;

    private:
      MapStepper();
      MapStepper(const MapStepper&);
      MapStepper& operator=(const MapStepper&);

      size_t nVar_;
      size_t nFunc_;

      double t_;
      size_t forward_;
      
      std::vector<double> xVals_;
      double* x_;
      double* xNew_;
      F_t f_t_;

      std::vector<double> jacVals_;
      double* jac_;
      double* jacNew_;
      double* jacTemp_;
      F_t dfdx_t_;

      std::vector<double> funcVals_;
      F_t func_t_;
    };
#pragma clang diagnostic pop
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_MAPSTEPPER_HPP */
