#include <fstream>
#include <sstream>
#include <iostream>
#include <tinythread.h>
#include "../dat/wave.hpp"
#include "auto.hpp"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"  
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"


using namespace scigma::common;
using namespace scigma::num;
typedef scigma::dat::AbstractWave<double> Wave;

namespace scigma
{
  namespace num
  {
    std::map<Auto*,Wave*> autoBranches;
    tthread::mutex branchMutex;
  }
}

extern "C"
{
  /* this routine is called whenever AUTO writes information about a new point */
  void scgaut_(int* id,int* ibr, int* mtot, int* itp, int* lab, double* u, double* par)
  {
    tthread::lock_guard<tthread::mutex> guard(branchMutex);
    
    Auto* instance(scigma::num::Auto::instance_from_id(size_t(*id)));
    AutoConstants& ac(instance->ac());
    size_t nVar((size_t(ac.NDIM)));
    size_t nPar(ac.parnames.size());

    /* special point */
    if(*lab)
      {
	Wave* oldBranch(autoBranches[instance]);
	if(oldBranch)
	  {
	    /* if we are at a special point that is not an endpoint, add it to the old branch as well */
	    if(*lab&&(*itp**itp!=81))
	      {
		oldBranch->lock();
		oldBranch->push_back(u,nVar);
		for(size_t i(0);i<ac.ICP.size();++i)
		  oldBranch->push_back(par[ac.ICP[i]-1]);
		oldBranch->unlock();
	      }
	    /* notify scigma that old branch has been finished */
	    instance->log_->push<LOG_SUCCESS>(ac.c);

	    /* create Waves for the point itself (constant) 
	       but not, if it is the very first point (no oldBranch)
	       (we already got this one), either as a 
	       fixed point or from a previous continuation */
	    Wave* varVals=new Wave(nVar+ac.ICP.size());
	    
	    varVals->lock();
	    varVals->push_back(u,nVar);
	    for(size_t i(0);i<ac.ICP.size();++i)
	      varVals->push_back(par[ac.ICP[i]-1]);
	    varVals->unlock();

	    Wave* constVals= new Wave(ac.parnames.size()-ac.ICP.size());
	    constVals->lock();
	    size_t index(0);
	    for(size_t i(0);i<ac.parnames.size();++i)
	      {
		bool varies(false);
		for(size_t j(0);j<ac.ICP.size();++j)
		  if(ac.ICP[j]==int(i+1))
		    varies=true;
		if(!varies)
		  constVals->push_back(par[i]);
	      }
	    constVals->unlock();
	  
	    
	    std::stringstream ss1;
	    ss1<<ac.c<<"|"<<varVals->get_python_id()<<"|"<<constVals->get_python_id()<<"|"<<*mtot<<"|"<<
	      *itp<<"|"<<*lab;
	    instance->log_->push<LOG_SUCCESS>(ss1.str());
	  }
      }

    /* start a new branch after special point (!= EP), or when
       we are on the first point of a new branch
    */
    if((*lab&&(*itp**itp!=81))||(*mtot**mtot==1))
      {
	/* create new branch */
	autoBranches[instance]=new Wave(size_t(ac.NMX)*(nVar+ac.ICP.size()));
	
	/* create constant values for the new branch */
	Wave* constVals= new Wave(ac.parnames.size()-ac.ICP.size());
	constVals->lock();
	size_t index(0);
	for(size_t i(0);i<ac.parnames.size();++i)
	  {
	    bool varies(false);
	    for(size_t j(0);j<ac.ICP.size();++j)
	      if(ac.ICP[j]==int(i+1))
		varies=true;
	    if(!varies)
	      constVals->push_back(par[i]);
	  }
	constVals->unlock();
	
	std::stringstream ss2;
	ss2<<ac.c<<"|"<<autoBranches[instance]->get_python_id()<<"|"<<constVals->get_python_id()<<"|"<<*mtot<<"|"<<
	  0<<"|"<<*lab;
	instance->log_->push<LOG_SUCCESS>(ss2.str());
      }
      
    Wave* newBranch(autoBranches[instance]);
    newBranch->lock();
    newBranch->push_back(u,nVar);
    for(size_t i(0);i<ac.ICP.size();++i)
      newBranch->push_back(par[ac.ICP[i]-1]);
    newBranch->unlock();
  }

  void auto_entry(int* id, const char* cFile);

  void func_(int* id, int* ndim,double* u,int* icp,double* par,int* ijac,double* f,double* dfdu,double* dfdp)
  {
    Auto* instance(Auto::instance_from_id(size_t(*id)));

    instance->f_(u,par,f);
    if(*ijac*(*ijac)==1) 
      instance->dfdx_(u,par,dfdu);
    if(*ijac==1)
      instance->dfdp_(u,par,dfdp);
  }

  void stpnt_(int* id, int* ndim,double* u, double* par,double* t)
  {
    Auto* instance(Auto::instance_from_id(size_t(*id)));
    AutoConstants& ac(instance->ac());
    size_t nVar((size_t(ac.NDIM)));
    size_t nPar(ac.parnames.size());

    const double* x(instance->x());
    const double* p(instance->p());
    
    for(size_t i(0);i<nVar;++i)
      u[i]=x[i];
    for(size_t i(0);i<nPar;++i)
      par[i]=p[i];
  }

  void bcnd_(int* id, int* ndim,double *par,int* icp, int* nbc, double* u0,double* u1,double* fb,int* ijac, int* dbc)
  {
    Auto* instance(Auto::instance_from_id(size_t(*id)));
  }
  
  void icnd_(int* id, int* ndim,double* par,int* icp,int* nint,double* u, double* uold,double* udot,double* upold,double* fi,int* ijac,double* dint)
  {
    Auto* instance(Auto::instance_from_id(size_t(*id)));
  }
  
  void fopt_(int* id, int* ndim,double* u,int* icp,double* par, int* ijac, double* fs, double* dfdu,double* dfdp)
  {
    Auto* instance(Auto::instance_from_id(size_t(*id)));
  }
  
  void pvls_(int* id, int* ndim,double* u,double* par)
  {
  Auto* instance(Auto::instance_from_id(size_t(*id)));
  }
  
  void autocb_(int* id)
  {
    
  }
}

namespace scigma
{
  namespace num
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::vector<Auto*> Auto::instances_;
    std::vector<size_t> Auto::availableIDs_;
#pragma clang diagnostic pop
    
    Auto::Auto(const EquationSystem& eqsys, Log* log, AutoConstants& autoConstants):
      autoConstants_(autoConstants),f_(eqsys.f_p()), dfdx_(eqsys.dfdx_p()), dfdp_(eqsys.dfdp_p()), log_(log)
    {

      for(size_t i(0);i<eqsys.n_variables();++i)
	x_.push_back(eqsys.variable_values()[i]);
      for(size_t i(0);i<eqsys.n_parameters();++i)
	p_.push_back(eqsys.parameter_values()[i]);
      
      autoConstants.JAC=dfdx_?0:-1;
      autoConstants.JAC*=dfdp_?-1:1;
      
      // write AUTO constants into file
      std::stringstream ss;
      ss<<"#AUTO constants file, generated by SCIGMA"<<std::endl;
      ss<<"s='"<<autoConstants_.s<<"', dat='"<<autoConstants_.dat<<"', sv='"<<autoConstants_.sv<<"'"<<std::endl;
      std::map<int,std::string>::iterator h(autoConstants_.unames.begin()),endh(autoConstants_.unames.end());
      ss<<"unames={";if(h!=endh){while(true){ss<<h->first<<":'"<<h->second<<"'";++h;if(h==endh){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      h=autoConstants_.parnames.begin(); endh=autoConstants_.parnames.end();
      ss<<"parnames={";if(h!=endh){while(true){ss<<h->first<<":'"<<h->second<<"'";++h;if(h==endh){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      std::map<int,double>::iterator i=autoConstants_.U.begin(),endi(autoConstants_.U.end());
      ss<<"U={";if(i!=endi){while(true){ss<<i->first<<":"<<i->second;++i;if(i==endi){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      i=autoConstants_.PAR.begin(); endi=autoConstants_.PAR.end();
      ss<<"PAR={";if(i!=endi){while(true){ss<<i->first<<":"<<i->second;++i;if(i==endi){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      ss<<"NDIM="<<autoConstants_.NDIM<<", IPS="<<autoConstants_.IPS<<", IRS="<<autoConstants_.IRS<<", ILP="<<autoConstants_.ILP<<std::endl;
      std::vector<int>::iterator j(autoConstants_.ICP.begin()),endj(autoConstants_.ICP.end());
      ss<<"ICP=[";if(j!=endj){while(true){ss<<*j;++j;if(j==endj){break;}else{ss<<", ";}}}ss<<"]"<<std::endl;
      ss<<"NTST="<<autoConstants_.NTST<<", NCOL="<<autoConstants_.NCOL<<", IAD="<<autoConstants_.IAD<<", ISP="<<autoConstants_.ISP<<std::endl;
      ss<<"ISW="<<autoConstants_.ISW<<", IPLT="<<autoConstants_.IPLT<<", NBC="<<autoConstants_.NBC<<", NINT="<<autoConstants_.NINT<<std::endl;
      ss<<"NMX="<<autoConstants_.NMX<<", NPR="<<autoConstants_.NPR<<", MXBF="<<autoConstants_.MXBF<<", IID="<<autoConstants_.IID<<std::endl;
      ss<<"ITMX="<<autoConstants_.ITMX<<", ITNW="<<autoConstants_.ITNW<<", NWTN="<<autoConstants_.NWTN<<", JAC="<<autoConstants_.JAC<<std::endl;
      ss<<"EPSL="<<autoConstants_.EPSL<<", EPSU="<<autoConstants_.EPSU<<", EPSS="<<autoConstants_.EPSS<<std::endl;
      ss<<"DS="<<autoConstants_.DS<<", DSMIN="<<autoConstants_.DSMIN<<", DSMAX="<<autoConstants_.DSMAX<<std::endl;
      ss<<"NPAR="<<36<<", IADS="<<autoConstants_.IADS<<std::endl;
      i=autoConstants_.THL.begin(); endi=autoConstants_.THL.end();
      ss<<"THL={";if(i!=endi){while(true){ss<<i->first<<":"<<i->second;++i;if(i==endi){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      i=autoConstants_.THU.begin(); endi=autoConstants_.THU.end();
      ss<<"THU={";if(i!=endi){while(true){ss<<i->first<<":"<<i->second;++i;if(i==endi){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      ss<<"RL0="<<autoConstants_.RL0<<", RL1="<<autoConstants_.RL1<<", A0="<<autoConstants_.A0<<", A1="<<autoConstants_.A1<<std::endl;
      i=autoConstants_.UZR.begin(); endi=autoConstants_.UZR.end();
      ss<<"UZR={";if(i!=endi){while(true){ss<<i->first<<":"<<i->second;++i;if(i==endi){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      i=autoConstants_.UZSTOP.begin(); endi=autoConstants_.UZSTOP.end();
      ss<<"UZSTOP={";if(i!=endi){while(true){ss<<i->first<<":"<<i->second;++i;if(i==endi){break;}else{ss<<", ";}}}ss<<"}"<<std::endl;
      std::vector<std::string>::iterator k(autoConstants_.SP.begin()),endk(autoConstants_.SP.end());
      ss<<"SP=[";if(k!=endk){while(true){ss<<"'"<<*k<<"'";++k;if(k==endk){break;}else{ss<<", ";}}}ss<<"]"<<std::endl;
      k=autoConstants_.STOP.begin(); endk=autoConstants_.STOP.end();
      ss<<"STOP=[";if(k!=endk){while(true){ss<<"'"<<*k<<"'";++k;if(k==endk){break;}else{ss<<", ";}}}ss<<"]"<<std::endl;
      ss<<"IIS="<<autoConstants_.IIS<<", IBR="<<autoConstants_.IBR<<", LAB="<<autoConstants_.LAB<<", TY='"<<autoConstants_.TY<<"'"<<std::endl;
      ss<<"NUNSTAB="<<autoConstants_.NUNSTAB<<", NSTAB="<<autoConstants_.NSTAB<<", IEQUIB="<<autoConstants_.IEQUIB;
      ss<<", ITWIST="<<autoConstants_.ITWIST<<", ISTART="<<autoConstants_.ISTART<<std::endl;
      j=autoConstants_.IREV.begin();endj=autoConstants_.IREV.end();
      ss<<"IREV=[";if(j!=endj){while(true){ss<<*j;++j;if(j==endj){break;}else{ss<<", ";}}}ss<<"]"<<std::endl;
      j=autoConstants_.IFIXED.begin();endj=autoConstants_.IFIXED.end();
      ss<<"IFIXED=[";if(j!=endj){while(true){ss<<*j;++j;if(j==endj){break;}else{ss<<", ";}}}ss<<"]"<<std::endl;
      j=autoConstants_.IPSI.begin();endj=autoConstants_.IPSI.end();
      ss<<"IPSI=[";if(j!=endj){while(true){ss<<*j;++j;if(j==endj){break;}else{ss<<", ";}}}ss<<"]"<<std::endl;
      std::ofstream constFile;
      std::string fName("c.");
      fName+=autoConstants_.c;
      constFile.open(fName);
      if(constFile.fail())
	throw std::string("could not open file ")+fName; 
      constFile << ss.str();
      constFile.close();

      // assign a unique ID to this AUTO object
      if(availableIDs_.empty())     
	{
	  id_=instances_.size();
	  instances_.push_back(this);
	}
      else
	{
	  id_=availableIDs_.back();
	  availableIDs_.pop_back();
	  instances_[id_]=this;
	}

      // run AUTO in its own thread (threading not yet implemented)
      int id((int(id_)));
      auto_entry(&id,autoConstants_.c.c_str());
      std::stringstream ss1;
      ss1<<autoConstants_.c<<"|done";
      log_->push<LOG_SUCCESS>(ss1.str());
    }
    
    Auto::~Auto()
    {
      	  instances_[id_]=NULL;
	  availableIDs_.push_back(id_);
    }

    
    const double* Auto::x() const 
    {
      return &x_[0];
    }

    const double* Auto::p() const
    {
      return &p_[0];
    }
    
    AutoConstants& Auto::ac()
    {
      return autoConstants_;
    }
    
    Auto* Auto::instance_from_id(size_t id)
    {
      if(id<instances_.size())
	return instances_.at(id);
      else
	return NULL;
    }


    AutoConstants::AutoConstants():NBC(0),NINT(0),JAC(0),NTST(50),NCOL(4),IAD(3),IADS(1),
				   ITMX(8),NWTN(3),ITNW(5),EPSL(1e-7),EPSU(1e-7),EPSS(1e-5),
				   DS(0.1),DSMIN(1e-5),DSMAX(1),
				   RL0(-1.7976e+308), RL1(1.7976e+308), A0(-1.7976e+308), A1(1.7976e+308),
				   NMX(0),ILP(0),ISP(2),ISW(1),MXBF(10),IRS(0),IPS(0),NPR(0),IBR(0),LAB(0),IIS(3),
				   IID(2),IPLT(0),IEQUIB(1),ITWIST(0),ISTART(5)
    {}
    

  } /* end namespace num */
} /* end namespace scigma */


#pragma clang diagnostic pop
#pragma GCC diagnostic pop
