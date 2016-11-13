#ifndef SCIGMA_NUM_INTEGRATIONSTEPPER_HPP
#define SCIGMA_NUM_INTEGRATIONSTEPPER_HPP

#include <vector>
#include "stepper.hpp"
#include "odessa.hpp"
#include "equationsystem.hpp"

namespace scigma
{
  namespace num
  {

    class IntegrationStepper:public Stepper
    {
    public:
      IntegrationStepper(const EquationSystem& eqsys, double dt,bool stiff=true, double aTol=1e-9, double rTol=1e-9, size_t maxIter=20000, bool computeJacobian=false);

      virtual double t() const;
      virtual const double* x() const;
      virtual const double* func() const;
      virtual const double* jac() const;

      virtual void reset(double t,const double *x);
      virtual void advance(size_t n=1);

      virtual size_t n_variables() const;
      virtual size_t n_functions() const;

      
    private:
      IntegrationStepper();
      IntegrationStepper(const IntegrationStepper&);
      IntegrationStepper& operator=(const IntegrationStepper&);

      size_t nVar_;
      size_t nFunc_;
      
      double t0_;
      Odessa odessa_;
      
      double* x_;
      double* p_;
      double* jac_;

      double dt_;
      
      F_t func_t_;
      std::vector<double> funcVals_;

      char padded_[8];
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_INTEGRATIONSTEPPER_HPP */
