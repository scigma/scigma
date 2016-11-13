#ifndef SCIGMA_NUM_EIGEN_HPP
#define SCIGMA_NUM_EIGEN_HPP

#include <vector>
#include <string>

namespace scigma
{
  namespace num
  {  

    typedef std::vector<std::string> VecS;
    typedef std::vector<double> VecD;
    typedef std::vector<size_t> VecSZ;

    const size_t N_EIGEN_TYPES=6;
    const size_t N_FLOQUET_TYPES=9;

    void eigen(size_t nVar, double* a, VecD& evals, VecD& evecs,VecSZ& types);
    void floquet(size_t nVar,  double* a, VecD& multipliers, VecD& evecs, VecSZ& types);
     
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_EIGEN_HPP */
