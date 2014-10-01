#ifndef __SCIGMA_NUM_EIGEN_H__
#define __SCIGMA_NUM_EIGEN_H__

#include "definitions.h"

namespace scigma
{
  namespace num
  {  
    
    const size_t N_EIGEN_TYPES=6;
    const size_t N_FLOQUET_TYPES=9;

    void eigen(size_t nVar, double* a, VecD& evals, VecD& evecs,VecSZ& types);
    void floquet(size_t nVar,  double* a, VecD& multipliers, VecD& evecs, VecSZ& types);
     
  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_EIGEN_H__ */
