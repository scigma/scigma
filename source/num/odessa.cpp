#include <cfloat>
#include <sstream>
#include <iostream>
#include <cmath>
#include "odessa.hpp"

extern "C" void odessa_(void (*F)(int*,double*,double*,double*,double*),void(*DF)(int*, double*, double*, double*, double*, int*), 
			int* NEQ, double* Y, double* RPAR, double* TIN,double* TOUT,  int* ITOL,  double* RTOL,
			double* ATOL, int* ITASK, int* ISTATE, int* IOPT, double* RWORK, int* LRW, int* IWORK, 
			int* LIW,void (*JAC)(int*, double*, double*, double*, int*, int*,double*, int*), int* MF, double* RCOMM, int* ICOMM);

extern "C" void svcom_(double* rsav,int* isav);
extern "C" void rscom_(double* rsav,int* isav);

namespace scigma
{
  namespace num
  {
    Odessa* Odessa::instance_(NULL);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    tthread::mutex Odessa::instanceMutex_;
#pragma clang diagnostic pop    

    Odessa::Odessa(size_t nVar, F f, DFDX dfdx, bool stiff, double aTol, double rTol, size_t steps, bool computeSensitivity):
      nVar_(nVar),atol_(aTol),rtol_(rTol),
      f_(f),dfdx_(dfdx),
      t_(0),x_(new double[nVar*(1+(computeSensitivity?nVar:0))]),p_(computeSensitivity?new double[nVar]:NULL),sensitivity_(computeSensitivity?&x_[nVar]:NULL)
    {
      auto const nPar = computeSensitivity?nVar:0;
      
      nOdessa_[0]=int(nVar_);
      nOdessa_[1]=int(nPar); // sensitivity analyis is done to obtain the fundamental matrix!
      
      // tolerances are scalars (see odessa.f line 159 for documentation)
      itol_=1;
      // new integration (odessa.f line 178)
      itask_=istate_=1;
      // optional inputs (odessa.f line 720/866)
      iopt_[0]=1; // set to one because we change the number of internal solver steps (iwork_[5])

      if(computeSensitivity)
	{
	  iopt_[1]=1;
	  liw_=int(21+nVar_+nPar);
	  iwork_=new int[size_t(liw_)];

	  for(size_t i(0);i<nPar;++i)
	    p_[i]=0;
	}
      else
	{
	  iopt_[1]=0;
	  liw_=int(20+nVar_);
	  iwork_=new int[size_t(liw_)];
	}

      iopt_[2]=0;   // never using dfdp
      
      if(stiff)
	{
	  if(dfdx_) // Jacobian provided
	    mf_=21;
	  else
	    mf_=22;
	  lrw_=int(22+8*(nPar+1)*nVar_+nVar_*nVar_+nVar_);
	  rwork_=new double[size_t(lrw_)];
	}
      else
	{
	  if(dfdx_) // Jacobian provided
	    mf_=11;
	  else
	    mf_=12;
	  lrw_=int(22+15*(nPar+1)*nVar_+nVar_*nVar_+nVar_);
	  rwork_=new double[size_t(lrw_)];
	}
      for(size_t i(4);i<9;++i)
	rwork_[i]=iwork_[i]=0;
      iwork_[5]=int(steps);
    }
    
    Odessa::~Odessa()
    {
      delete[] rwork_;
      delete[] iwork_;
      delete[] x_;
      if(p_)delete[] p_;
    }

    void Odessa::integrate(double dt, size_t steps)
    {
      if(!(0>dt||0<dt))
	return;
      for(size_t i(0);i<steps;++i)
	{
	  instanceMutex_.lock();
	    instance_=this;
	    if(istate_!=1)
	      {
		tNext_+=dt;
		rscom_(rcomm_,icomm_);
	      }
	    else
	      tNext_=t_+dt;

	    odessa_(odessa_F,NULL, nOdessa_,x_,p_,&t_,&tNext_,&itol_,&rtol_,&atol_,&itask_,&istate_,iopt_,rwork_,&lrw_,iwork_,&liw_,odessa_JAC,&mf_,rcomm_,icomm_);
	    svcom_(rcomm_,icomm_);
	    instanceMutex_.unlock();
	  if(istate_<0) // an error occurred
	    {
	      std::stringstream ss;
	      switch(istate_)
		{
		case -1:
		  ss<<"ODESSA: no convergence after "<<iwork_[5]<<" steps\n";
		  break;
		case -2:
		  ss<<"ODESSA: tolerance ("<<rwork_[13]<<") too small\n";
		  break;
		case -3:
		  ss<<"ODESSA: illegal input detected\n";
		  break;
		case -4:
		  ss<<"ODESSA: error tests fail, maybe singular\n";
		  break;
		case -5:
		  ss<<"ODESSA: Jacobian probably inaccurate\n";
		  break;
		case -6:
		  ss<<"ODESSA: a component with zero absolute tolerance hit zero\n";
		  break;
		default:
		  ss<<"ODESSA: this should not happen\n";
		}
	      throw(ss.str());
	    }
	}
    }

    void Odessa::reset()
    {
      istate_=itask_=1;
    }

    double& Odessa::t(){return t_;}
    double* Odessa::x(){return x_;}
    double* Odessa::sensitivity() {return sensitivity_;}    

    const double& Odessa::t() const {return t_;}
    const double* Odessa::x() const {return x_;}
    const double* Odessa::sensitivity() const {return sensitivity_;}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    void Odessa::odessa_F(int* n, double* t, double* x, double* p, double* rhs) 
    {
      instance_->f_(*t,x,rhs);
    }

    void Odessa::odessa_JAC(int* n, double* t, double* x, double* p, int* ml, int* mu, double* dfdx, int* nrowpd)
    {
      instance_->dfdx_(*t,x,dfdx);
    }

#pragma GCC diagnostic pop

  } /* end namespace num */
} /* end namespace scigma */
