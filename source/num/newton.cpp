#include <cmath>
#include <cfloat>
#include <iostream>
#include <algorithm>
#include <string>
#include "newton.hpp"

extern "C" void dgeco_(double* a, const int* lda, const int* n, int* ipvt, double* rcond, double* z);
extern "C" void dgesl_(double* a, const int* lda, const int* n, const int* ipvt, double* b ,const int* job);

extern "C" void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);
extern "C" void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);


namespace scigma
{
  namespace num
  {
    bool newton(int nVar, double* x, const Newton::F& f, bool generateJacobian, double tol)
    {
      double damping(1);
      const int job(0);
      double rcond,detmin(DBL_EPSILON/FLT_RADIX);

      double* scratch(new double[size_t((2+nVar)*nVar)]);
      int* ipvt(new int[size_t(nVar)]);
      double* rhs(scratch); 
      double* dfdx(scratch+nVar);
      double* z(scratch+nVar*(nVar+1));

      int iterations(20);
      bool retval(false);
      
      while(iterations--!=0)
	{
	  for(int i(0);i<nVar*nVar;++i)
	    dfdx[i]=0.0;
	  
	  /*	  std::cout<<"nVar="<<nVar<<std::endl;
	  for(int i(0);i<nVar;++i)
	    std::cout<<x[i]<<"\t";
	  std::cout<<std::endl;
	  std::cout<<std::endl;*/

	  /*	  for(int i(0);i<nVar;++i)
	    std::cout<<rhs[i]<<"\t";
	  std::cout<<std::endl<<"----"<<std::endl;
	  for(int i(0);i<nVar;++i)
	    {
	      for(int j(0);j<nVar;++j)
		std::cout<<rhs[nVar+j*nVar+i]<<"\t";
	      std::cout<<std::endl;
	    }
	    std::cout<<"das war vorher"<<std::endl;*/

	  f(x,rhs);

	  /*	  for(int i(0);i<nVar;++i)
	    std::cout<<rhs[i]<<"\t";
	  std::cout<<std::endl<<"----"<<std::endl;
	  for(int i(0);i<nVar;++i)
	    {
	      for(int j(0);j<nVar;++j)
		std::cout<<rhs[nVar+j*nVar+i]<<"\t";
	      std::cout<<std::endl;
	    }
	    std::cout<<std::endl;*/
	  
	  double norm(0);
	  for(int i(0);i<nVar;++i)
	    {
	      double abs(sqrt(rhs[i]*rhs[i]));
	      if(abs>norm)
		norm = abs;
	    }
	  //	  std::cout<<"norm:"<<norm<<std::endl;
	  if(norm<tol)
	    {
	      retval=true;
	      break;
	    }
	  if(generateJacobian)
	    discrete_jacobian(nVar,x,f,dfdx,rhs);
	  dgeco_(dfdx,&nVar,&nVar,ipvt,&rcond,z);
	  if(rcond<detmin)
	    throw(std::string("singular matrix in Newton iteration\n"));
	  dgesl_(dfdx,&nVar,&nVar,ipvt,rhs,&job);
	  for(int i(0);i<nVar;++i)
	    x[i]-=damping*rhs[i];
	}
      
      delete[] ipvt;
      delete[] scratch;

      return retval;
    }
   
    void discrete_jacobian(int nVar,double* x, const Newton::F& f, double* dfdx, double* work)
    {
      double* rhs(work?work:new double[size_t(nVar*2)]);
      double* rhs_delta(work?dfdx+nVar*nVar:rhs+nVar);
      double detmin(DBL_EPSILON/FLT_RADIX);

      if(!work)
	f(x,rhs);
    
      for(int j(0);j<nVar;++j)
	{
	  double delta(sqrt(detmin*x[j]*x[j]));
	  if(delta<sqrt(detmin))
	    delta=sqrt(detmin);
	  x[j]+=delta;
	  f(x,rhs_delta);
	  for(int i(0);i<nVar;++i)
	    dfdx[i+nVar*j]=(rhs_delta[i]-rhs[i])/delta;
	  x[j]-=delta;
	}  
     
      if(!work)
	delete []rhs;
    }


  } /* end namespace num */
} /* end namespace scigma */


