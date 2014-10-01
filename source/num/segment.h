#ifndef __SCIGMA_NUM_SEGMENT_H__
#define __SCIGMA_NUM_SEGMENT_H__

#include <vector>
#include <stack>
#include <map>
#include "../pythonobject.h"
#include "../dat/wave.h"
#include "stepper.h"
namespace scigma
{
  namespace num
  {

class Segment:public PythonObject<Segment>
    {
    public:
      Segment(Stepper* stepper, size_t nPeriod, double eval, dat::Wave* initial, double eps, double deltaMin=1e-10);
      ~Segment();
      double* step(Stepper* stepper, double delta, double eps);
      double* last();
      double* next_to_last();
      double* rewind_ptr(double* pData);
      double* advance_ptr(double* pData);
      void discard_after(double arc);
      void pop_back();
      size_t size();
      void dump();
    private:
      typedef std::map<double,double*>::iterator SegPt;

      SegPt insert(double* pData);
      SegPt rewind_point(SegPt p);
      SegPt advance_point(SegPt p);
bool test_guess(Stepper* stepper, double* p1Data, double* pData, double* p2Data, double* q1Data, double delta, double& dev, size_t depth,double* data);
bool find_point_in_interval_by_bisection(Stepper* stepper, double* p1Data, double* p2Data, double* q1Data, double delta, double eps, size_t depth,double* data);
bool find_point_in_interval(Stepper* stepper, SegPt p1, SegPt p2, SegPt q1, double delta, double eps, size_t depth,double* data);

      size_t nVar_;
      size_t nPeriod_;
      std::map<double,double*> map_;
      double deltaMin_;

      bool fail_;

      char padding_[7];
    };
    //      void extend_manifold_segment2(std::vector<dat::Wave*>& segment, F& map, double eval, double finalArc, double alphaMin=0.05, double alphaMax=0.1);
  
  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_SEGMENT_H__ */
