#ifndef __SCIGMA_NUM_MAPMANIFOLDSTEPPER_H__
#define __SCIGMA_NUM_MAPMANIFOLDSTEPPER_H__

#include <list>
#include <vector>
#include "stepper.h"
#include "../dat/wave.h"

namespace scigma
{
  namespace num
  {
    
    class MapManifoldStepper:public Stepper
    {
    public:
      MapManifoldStepper(Stepper* mapStepper, dat::Wave* initial, double dsmax, double dsmin, double alpha, double dalpha, size_t nPeriod);
      ~MapManifoldStepper();

      double t() const;
      double x(size_t index) const;
      double func(size_t index) const;
      double jac(size_t index) const;
      void reset(const double* x);
      void advance_once();
      void advance(size_t n=1);

    private:
      double angle(double* q0,double* q1, double* q2);
      double arc(double* q0,double* q1);

      Stepper* mapStepper_;
      
      double ds_;
      double dsmax_;
      double dsmin_;

      double alpha_;
      double dalpha_;
      size_t nPeriod_;

      size_t current_;
      size_t nVar_;
      std::list<std::vector<double> > points_;
      std::vector<double> xBackup_;

      typedef std::list<std::vector<double> >::iterator SegPt;
      SegPt preImage_;
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_MAPMANIFOLDSTEPPER_H__ */
