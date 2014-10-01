#ifndef __SCIGMA_NUM_DEFINITIONS_H__
#define __SCIGMA_NUM_DEFINITIONS_H__

#include <string>
#include <vector>
#include <functional>
#include "function.h"

namespace scigma
{
  namespace num
  {
    enum Mode
    {
      MAP=0,
      ODE=1,
      STROBE=2,
      POINCARE=3
    };

    typedef std::function<void(const double*, double*)> F;
    typedef std::function<void(const double*, const double*, double*)> F_p;
    typedef std::function<void(double t, const double*, double*)> F_t;
    typedef std::function<void(double t, const double*, const double*, double*)> F_pt;

    typedef std::vector<std::string> VecS;
    typedef std::vector<Function> VecF;
    typedef std::vector<double> VecD;
    typedef std::vector<size_t> VecSZ;
  }
}

#endif /* __SCIGMA_NUM_DEFINITIONS_H__ */
