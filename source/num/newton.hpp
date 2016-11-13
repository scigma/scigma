#ifndef SCIGMA_NUM_NEWTON_HPP
#define SCIGMA_NUM_NEWTON_HPP

#include <functional>

namespace scigma
{
  namespace num
  {  
    
    class Newton
    {
    public:
      typedef std::function<void(const double*, double*)> F;
    };
    
    bool newton(int nVar, double* x, const Newton::F& f, bool generateJacobian=true, double tol=1e-9);
    void discrete_jacobian(int nVar, double* x, const Newton::F& f, double* dfdx, double* work=NULL);
     
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_NEWTON_HPP */
