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

      double t() const override;
      const double* x() const override;
      const double* func() const override;
      const double* jac() const override;

      void reset(double t,const double *x) override;
      void advance(size_t n=1) override;

      virtual size_t n_variables() const override;
      virtual size_t n_functions() const override;

      
    private:
      IntegrationStepper();
      IntegrationStepper(const IntegrationStepper&);
      IntegrationStepper& operator=(const IntegrationStepper&);

      size_t nVar_;
      size_t nFunc_;
      
      double t0_;
      Odessa odessa_;
      
      double* x_;
      double* jac_;

      double dt_;

      std::vector<double> funcVals_;
      F_t func_t_;
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_INTEGRATIONSTEPPER_HPP */
