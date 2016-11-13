#include <cmath>
#include <cfloat>
#include <iostream>
#include <algorithm>
#include "eigen.hpp"

extern "C" void dgeev_(char* jobvl, char* jobvr, int* n, double* a, int* lda, double* wr, double* wi, double* vl, int* ldvl, double* vr, int* ldvr, double* work, int* lwork, int* info);

namespace scigma
{
  namespace num
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
    void eigen_(int nVar, double* a, double* evals, double* evecs)
    {
      
      /*for(int i(0);i<nVar;++i)
	{
	  for(int j(0);j<nVar;++j)
	    {
	      std::cerr<<a[i*nVar+j]<<", ";
	    }
	  std::cerr<<std::endl;
	  }*/
      char jobvl('N'),jobvr('V'); // compute only right eigenvectors;
      int n(nVar),lda(nVar),ldvl(nVar),ldvr(nVar),lwork(-1),info(0);
      double* wr(evals);
      double* wi(evals+nVar);
      double* vr(evecs);
      double workSize;
      // query for optimal work length first
      dgeev_(&jobvl,&jobvr,&n,a,&lda,wr,wi,NULL,&ldvl,vr,&ldvr,&workSize,&lwork,&info);
      lwork=int(workSize);
      double* work(new double[size_t(lwork)]);
      // get eigenvalues and eigenvectors
      dgeev_(&jobvl,&jobvr,&n,a,&lda,wr,wi,NULL,&ldvl,vr,&ldvr,work,&lwork,&info);
      delete[] work;
    }	  
#pragma clang diagnostic pop

    void eigen(size_t nVar, double* a, VecD& evals, std::vector<double>& evecs, VecSZ& types)
    {
      evals.resize(2*nVar);
      evecs.resize(nVar*nVar);
      VecD evals_(2*nVar);
      VecD evecs_(nVar*nVar);

      eigen_(int(nVar),a,&evals_[0],&evecs_[0]);
    
      /* categorize eigenvalues (and corresponding eigenvectors)
	 into the following types:
	 0: real, stable (<0)
	 1: real, neutral (=0)
	 2: real, unstable (>0)
	 3: complex, stable( real part <0)
	 4: complex, neutral (real part =0)
	 5: complex, unstable (real part >0)
      */
      

      types.resize(N_EIGEN_TYPES,0);
      for(size_t i(0);i<nVar;++i)
	{
	  size_t base((evals_[nVar+i]<0||evals_[nVar+i]>0)?3:0); // set whether real or complex
	  size_t offset(evals_[i]>0?2:(evals_[i]<0?0:1)); // set whether stable, neutral or unstable
	  ++types[base+offset];
	}
      
      // sort eigenvalues by ascending real parts
      VecSZ index(nVar);
      for(size_t i(0);i<nVar;++i)
	index[i]=i;
      auto compare_by_real_part = [&evals_](size_t i, size_t j){return evals_[i]<evals_[j];};
      std::sort(index.begin(),index.end(),compare_by_real_part);
      
      for(size_t i(0);i<nVar;++i)
	{
	  evals[i]=evals_[index[i]];
	  evals[i+nVar]=evals_[index[i]+nVar];
	  for(size_t j(0);j<nVar;++j)
	    evecs[i*nVar+j]=evecs_[index[i]*nVar+j];
	}
    }
    
    void floquet(size_t nVar, double* a, std::vector<double>& multipliers, VecD& evecs, VecSZ& types)
    {
      // adjust jacobian for the Poincare variable
      multipliers.resize(2*nVar);
      evecs.resize(nVar*nVar);
      VecD multipliers_(2*nVar);
      VecD evecs_(nVar*nVar);
      eigen_(int(nVar),a,&multipliers_[0],&evecs_[0]);
      
      /* sort multipliers (and corresponding eigenvectors)
	 into the following types:
	 0: real, positive, stable (<1)
	 1: real, positive, neutral (=1)
	 2: real, positive, unstable (>1)
	 3: complex, stable (modulus <1)
	 4: complex, neutral (modulus =1)
	 5: complex, unstable (modulus >1)
	 6: real, negative, stable (>-1)
	 7: real, negative, neutral (=-1)
	 8: real, negative, unstable (<-1)
      */
      
      types.resize(N_FLOQUET_TYPES,0);
      for(size_t i(0);i<nVar;++i)
	{
	  size_t base((multipliers_[nVar+i]>0||multipliers_[nVar+i]<0)?3:(multipliers_[i]>0?0:6)); // set whether positive real, complex or negative real
	  double modulus(sqrt(multipliers_[i]*multipliers_[i]+multipliers_[nVar+i]*multipliers_[nVar+i])); // set whether stable, neutral or unstable
	  size_t offset(modulus>1?2:(modulus<1?0:1)); 
	  ++types[base+offset];
	}
      
      // sort eigenvalues by ascending modulus
      VecSZ index(nVar);
      for(size_t i(0);i<nVar;++i)
	index[i]=i;
      auto compare_by_modulus= [&multipliers_,nVar](size_t i, size_t j)
	{return multipliers_[i]*multipliers_[i]+multipliers_[i+nVar]*multipliers_[i+nVar]
	 < multipliers_[j]*multipliers_[j]+multipliers_[j+nVar]*multipliers_[j+nVar];};
      std::sort(index.begin(),index.end(),compare_by_modulus);
      
      for(size_t i(0);i<nVar;++i)
	{
	  multipliers[i]=multipliers_[index[i]];
	  multipliers[i+nVar]=multipliers_[index[i]+nVar];
	  for(size_t j(0);j<nVar;++j)
	    evecs[i*nVar+j]=evecs_[index[i]*nVar+j];
	}
    }

  } /* end namespace num */
} /* end namespace scigma */


