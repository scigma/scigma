#ifndef __SCIGMA_NUM_ODESSA_H__
#define __SCIGMA_NUM_ODESSA_H__

#include <tinythread.h>
#include <functional>

namespace scigma
{
  namespace num
  {
    class Odessa
    {
    public:

      typedef std::function<void(double, const double*, const double*, double*)> F; 
      typedef std::function<void(double, const double*, const double*, double*)> DFDX; 
      typedef std::function<void(double, const double*, const double*, double*, int)> DFDP; 
      
      Odessa(size_t nVar, F f, DFDX dfdx=NULL, size_t nPar=0, DFDP dfdp=NULL, bool stiff=true, double aTol=1e-9, double rTol=1e-9, size_t steps=20000);
      ~Odessa();

      double& t();
      double* x();
      double* p();
      double* sensitivity();

      const double& t() const;
      const double* x() const;
      const double* p() const;
      const double* sensitivity() const;

      void integrate(double dt, size_t steps=1);
      void reset();

    private:
      Odessa(const Odessa&);
      Odessa& operator=(const Odessa&);

      static void odessa_F(int* n, double* t, double* x, double* p, double* rhs);
      static void odessa_JAC(int* n, double* t, double* x, double* p, int* ml, int* mu, double* dfdx, int* nrowpd);
      static void odessa_DF(int* n, double* t, double* x, double* p, double* dfdp, int* jpar);
      
      size_t nVar_;
      size_t nPar_;

      int nOdessa_[2];

      int itol_,itask_,istate_,liw_,lrw_,mf_;
      double atol_,rtol_;
      int* iwork_;
      double* rwork_;
      double rcomm_[222];
      
      F f_;
      DFDX dfdx_;
      DFDP dfdp_;
      
      double t_;
      double tNext_;
      
      double* x_;
      double* p_;
      double* sensitivity_;

      static Odessa* instance_;
      static tthread::mutex instanceMutex_;
      
      int icomm_[54];
      int iopt_[4]; // last int is just for alignment 

    };
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_ODESSA_H__ */
