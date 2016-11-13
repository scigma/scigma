#ifndef SCIGMA_NUM_AUTO_HPP
#define SCIGMA_NUM_AUTO_HPP

#include <functional>
#include <map>
#include <vector>
#include <string>
#include "../common/log.hpp"
#include "equationsystem.hpp"


using scigma::common::Log;

extern "C"
{
  void scgaut_(int* id,int* ibr, int* mtot, int* itp, int* lab, double* u, double* par);
  void func_(int* id, int* ndim,double* u,int* icp,double* par,int* ijac,
	     double* f,double* dfdu,double* dfdp);
  void stpnt_(int* id, int* ndim,double* u, double* par,double* t);
  void bcnd_(int* id, int* ndim,double *par,int* icp, int* nbc,
	     double* u0,double* u1,double* fb,int* ijac, int* dbc);
  void icnd_(int* id, int* ndim,double* par,int* icp,int* nint,
	     double* u, double* uold,double* udot,double* upold,
	     double* fi,int* ijac,double* dint);
  void fopt_(int* id, int* ndim,double* u,int* icp,double* par, int* ijac,
	     double* fs, double* dfdu,double* dfdp);
  void pvls_(int* id, int* ndim,double* u,double* par);
}

namespace scigma
{
  namespace num
  {

    class AutoConstants
    {
    public:
      AutoConstants();
      
      std::string c;
      std::string s, dat, sv;
      int NDIM,NBC,NINT,NPAR;
      int JAC;
      int NTST,NCOL;
      int IAD,IADS;
      int ITMX,NWTN,ITNW;
      double EPSL,EPSU,EPSS;
      double DS,DSMIN,DSMAX;
      std::map<int,double> THL;
      std::map<int,double> THU;
      std::vector<std::string> STOP;
      double RL0,RL1;
      double A0,A1;
      std::vector<int> ICP;
      int NMX;
      int ILP;
      std::vector<std::string> SP;
      int ISP,ISW;
      int MXBF;
      int IRS;
      std::map<int, double> U;
      std::map<int, double> PAR;
      std::string TY;
      std::map<int, std::string> unames;
      std::map<int, std::string> parnames;
      int IPS;
      int NPR;
      int IBR, LAB;
      int IIS, IID;
      std::map<int, double> UZR;
      std::map<int, double> UZSTOP;
      int IPLT;
      int NUNSTAB, NSTAB,IEQUIB, ITWIST, ISTART;

      std::vector<int> IREV, IFIXED, IPSI;
      
    };

    
    class Auto
    {
      friend void ::scgaut_(int* id,int* ibr, int* mtot, int* itp, int* lab, double* u, double* par);
      friend void ::func_(int* id, int* ndim,double* u,int* icp,double* par,int* ijac,
		      double* f,double* dfdu,double* dfdp);
      friend void ::stpnt_(int* id, int* ndim,double* u, double* par,double* t);
      friend void ::bcnd_(int* id, int* ndim,double *par,int* icp, int* nbc,
		      double* u0,double* u1,double* fb,int* ijac, int* dbc);
      friend void ::icnd_(int* id, int* ndim,double* par,int* icp,int* nint,
		      double* u, double* uold,double* udot,double* upold,
		      double* fi,int* ijac,double* dint);
      friend void ::fopt_(int* id, int* ndim,double* u,int* icp,double* par, int* ijac,
		      double* fs, double* dfdu,double* dfdp);
      friend void ::pvls_(int* id, int* ndim,double* u,double* par);

      
    public:
      
      typedef std::function<void(const double*, const double*, double*)> F; 
      typedef std::function<void(const double*, const double*, double*)> DFDX; 
      typedef std::function<void(const double*, const double*, double*)> DFDP; 
      
      Auto(const EquationSystem& eqsys, Log* log, AutoConstants& autoConstants);
      ~Auto();

      const double* x() const;
      const double* p() const;
      
      AutoConstants& ac();
      
    private:
      Auto(const Auto&);
      Auto& operator=(const Auto&);

      AutoConstants autoConstants_;
      
      F f_;
      DFDX dfdx_;
      DFDP dfdp_;
      
      size_t id_; 

      std::vector<double> x_;
      std::vector<double> p_;

      Log* log_;
      
      static std::vector<Auto*> instances_;
      static std::vector<size_t> availableIDs_;
      
      static Auto* instance_from_id(size_t id);
    };
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_AUTO_HPP */

