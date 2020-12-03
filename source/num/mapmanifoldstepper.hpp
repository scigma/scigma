#ifndef SCIGMA_NUM_MAPMANIFOLDSTEPPER_HPP
#define SCIGMA_NUM_MAPMANIFOLDSTEPPER_HPP

#include <list>
#include <vector>
#include "stepper.hpp"

namespace scigma
{
  namespace num
  {
    
    class MapManifoldStepper:public Stepper
    {

    public:
      MapManifoldStepper(Stepper* mapStepper, double const * steadyState, double const * firstSegment, double dsmax, double dsmin, double alpha, double dalpha, size_t nPeriod);
      ~MapManifoldStepper();

      virtual double t() const;
      virtual const double* x() const;
      virtual const double* func() const;
      virtual const double* jac() const;
      virtual void reset(double t, const double* x);
      void advance_once();
      virtual void advance(size_t n=1);

      virtual size_t n_variables() const;
      virtual size_t n_functions() const;

      
    private:
      double angle(double* q0,double* q1, double* q2);
      double arc(double* q0,double* q1);

      Stepper* mapStepper_;

      double t0_;
      double ds_;
      double dsmax_;
      double dsmin_;

      double alpha_;
      double dalpha_;
      size_t nPeriod_;

      size_t current_;
      size_t nVar_;
      size_t nFunc_;
      std::list<std::vector<double> > points_;
      std::vector<double> xBackup_;

      typedef std::list<std::vector<double> >::iterator SegPt;
      SegPt preImage_;
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_MAPMANIFOLDSTEPPER_HPP */
