#ifndef __SCIGMA_NUM_MAPMANIFOLDSTEPPER_H__
#define __SCIGMA_NUM_MAPMANIFOLDSTEPPER_H__

#include "stepper.h"
#include "segment.h"

namespace scigma
{
  namespace num
  {

    class MapManifoldStepper:public Stepper
    {
    public:
      MapManifoldStepper(Stepper* mapStepper, Segment* segment, double dsmax, double dsmin, double alpha);
      ~MapManifoldStepper();

      double t() const;
      double x(size_t index) const;
      double func(size_t index) const;
      double jac(size_t index) const;
      void reset(const double* x);
      void advance(size_t n=1);

    private:
      double angle(double* q0,double* q1, double* q2);

      Stepper* mapStepper_;
      Segment* segment_;
      
      double ds_;
      double dsmax_;
      double dsmin_;

      double alpha_;
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_MAPMANIFOLDSTEPPER_H__ */
