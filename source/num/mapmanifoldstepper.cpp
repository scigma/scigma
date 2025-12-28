#include <cstdint>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <algorithm>
#include <tinythread.h>
#include "mapmanifoldstepper.hpp"

namespace scigma
{
  namespace num
  {
    
    MapManifoldStepper::MapManifoldStepper(Stepper* mapStepper, double const * steadyState, double const * firstSegment ,double dsmax, double dsmin, double alpha, double dalpha,size_t nPeriod):
      mapStepper_(mapStepper),t0_(mapStepper_->t()),ds_(dsmax),dsmax_(dsmax),dsmin_(dsmin),alpha_(alpha),dalpha_(dalpha),
      nPeriod_(nPeriod),current_(0),nVar_(mapStepper->n_variables()),nFunc_(mapStepper->n_functions()),xBackup_(mapStepper->n_variables())
    {
      points_.push_back(std::vector<double>(nVar_+1));
      for(size_t i(0);i<nVar_;++i)
	{
	  points_.back()[i]=steadyState[i];
	}

      points_.back()[nVar_]=0;

      points_.push_back(std::vector<double>(nVar_+1));
      for(uint32_t i(0);i<nVar_;++i)
	{
	  points_.back()[i]=firstSegment[i];
	}
      points_.back()[nVar_]=arc(&(points_.front()[0]),&(points_.back()[0]));

      /*      for(size_t i(0);i<nVar;++i)
	      xBackup_[i]=mapStepper->x(i);*/

      preImage_=points_.begin();
    }
    
    MapManifoldStepper::~MapManifoldStepper()
    {
      delete mapStepper_;
    }
    
    double MapManifoldStepper::t() const {return points_.back()[nVar_];}
    const double* MapManifoldStepper::x() const {return mapStepper_->x();}
    const double* MapManifoldStepper::func() const {return mapStepper_->func();}
    const double* MapManifoldStepper::jac() const {return NULL;}
    void MapManifoldStepper::reset(double t, const double* x){mapStepper_->reset(t,x);}

    size_t MapManifoldStepper::n_variables() const {return nVar_;}
    size_t MapManifoldStepper::n_functions() const {return nFunc_;}
    
    double MapManifoldStepper::arc(double* q0,double* q1)
    {
      double d(0);
      for(size_t i(0);i<nVar_;++i)
	d+=(q1[i]-q0[i])*(q1[i]-q0[i]);
      return std::sqrt(d);
    }
    
    double MapManifoldStepper::angle(double* q0,double* q1, double* q2)
    {
      double product(0);
      double d1(0);
      double d2(0);
      for(size_t i(0);i<nVar_;++i)
	{
	  //	  std::cerr.precision(15);
	  //	  std::cerr<<q0[i]<<", "<<q1[i]<<", "<<q2[i]<<std::endl;
	  double diff1(q0[i]-q1[i]),diff2(q1[i]-q2[i]);
	  product+=diff1*diff2;
	  d1+=diff1*diff1;
	  d2+=diff2*diff2;
	}
      product/=sqrt(d1)*sqrt(d2);
      //      std::cerr<<product<<", winkel: "<<acos(product)<<", ";
      return std::acos(product);
    }

    void MapManifoldStepper::advance_once()
    {
      ++current_;
      if(current_%nPeriod_)
	{
	  mapStepper_->advance(1);
	}
      else
	{
	  std::vector<double> work(4*nVar_+4);
	  double* p(&work[0]);
	  double* q(&work[nVar_+1]);
	  double* r(&work[2*nVar_+2]);
	  double* Q(&work[3*nVar_+3]);

	  double addedArcLengthInAllIterations(0.0);  // keep track of total added arc lenght
	  
	  while(addedArcLengthInAllIterations<dsmax_-ds_/2)
	    {
	      double l(0.0);
	      double addedArcLengthInThisIteration(0.0);
	      
	      // find a line segment with |f(p2)-f(preImage)=back|>ds and |f(p1)-f(preImage)=back| <ds
	      SegPt pre(preImage_);
	      if((*pre)[nVar_]>0)//do not increment for initial segment
		++pre;
	      while(pre!=points_.end())
		{
		  //std::cerr<<"finding segment, starting at "<<(*pre)[nVar_]<<std::endl;
		  mapStepper_->reset(t0_,&((*pre)[0]));
		  mapStepper_->advance(nPeriod_);
		  for(size_t j(0);j<nVar_;++j)
		    {
		      Q[j]=mapStepper_->x()[j];
		      //std::cerr<<Q[j]<<", ";
		    }
		  l=arc(&Q[0],&(points_.back()[0]));
		  Q[nVar_]=points_.back()[nVar_]+l;
		  if(l>=ds_)
		    break;
		  //std::cerr<<"arc: "<<l<<std::endl;
		  ++pre;
		}
	      //std::cerr<<"arc: "<<l<<", ds: "<<ds_<<std::endl;
	      if(l<dsmin_)
		throw(std::string("reached minimum arclength step\n"));

	      if(l>=1.2*ds_) // otherwise go immediately to angle-checking
		{
		  for(size_t j(0);j<nVar_+1;++j)
		    {
		      r[j]=(*(pre))[j];
		      /*		      std::cout<<"r["<<j<<"]="<<r[j]<<", ";
					      std::cout.flush();*/
		      p[j]=(*(--pre))[j];
		      /*		      std::cout<<"p["<<j<<"]="<<p[j]<<", ";
					      std::cout.flush();*/
		      ++pre;
		      q[j]=(p[j]+r[j])/2;
		      /*		      std::cout<<"q["<<j<<"]="<<q[j]<<", ";
					      std::cout.flush();*/
		    }

		  //perform bisection algorithm
		  size_t count(0);
		  while(++count<100)
		    {
		      //		      std::cerr<<"bisecting, starting at "<<q[nVar_]<<std::endl;
		      mapStepper_->reset(t0_,q);
		      mapStepper_->advance(nPeriod_);
		      for(size_t j(0);j<nVar_;++j)
			Q[j]=mapStepper_->x()[j];
		      addedArcLengthInThisIteration=arc(&Q[0],&(points_.back()[0]));
		      Q[nVar_]=points_.back()[nVar_]+addedArcLengthInThisIteration;
		      //std::cerr<<"arc: "<<l<<" ds: "<<ds_<<std::endl;
		      if(addedArcLengthInThisIteration<0.8*ds_)
			std::swap(q,p);
		      else if(addedArcLengthInThisIteration>1.2*ds_)
			std::swap(q,r);
		      else
			break;
		      //std::cerr<<"l: "<<L<<" at "<<q[nVar_]<<", desired:"<<ds_<<std::endl;
		      for(size_t j(0);j<nVar_+1;++j)
			q[j]=(p[j]+r[j])/2;
		    }
		  //std::cerr<<"through with L: "<<L<<" at "<<q[nVar_]<<std::endl;
		  if(count==100)
		    {
		      throw(std::string("reached maximum number of bisection steps"));
		    }
		}
	      else
		{
		  addedArcLengthInThisIteration = l;
		}

	      // check if angle is ok:
	      SegPt p2(--points_.end());
	      SegPt p1(--p2);
	      ++p2;
	      
	      double a(angle(&((*p1)[0]),&((*p2)[0]),&Q[0]));
	      if((a>alpha_||ds_*a>dalpha_)&&ds_>1e-4)
		{
		  ds_/=2.;
		  if(ds_<dsmin_)
		    throw(std::string("reached minimum arclength step\n"));
		  continue;
		}

	      std::vector<double> newPt(nVar_+1);
	      for(size_t j(0);j<nVar_+1;++j)
		newPt[j]=Q[j];

	      preImage_=--pre;//points_.end();
	      points_.push_back(newPt);
	      /*std::cerr<<"............."<<std::endl;

	      	      std::cerr<<"adding point "<< newPt[nVar_]<<std::endl;
	      	      for(size_t j(0);j<nVar_;++j)
		  std::cerr<<newPt[j]<<", ";
		  std::cerr<<std::endl<<".........."<<std::endl;*/

	      if(l>=1.2*ds_) // need to insert a new pre-image
		{
		  std::vector<double> prePt(nVar_+1);
		  for(size_t j(0);j<nVar_+1;++j)
		    prePt[j]=q[j];
		  preImage_=points_.insert(++pre,prePt);
		  /*std::cerr<<"adding prepoint "<< prePt[nVar_]<<std::endl;
		  for(size_t j(0);j<nVar_;++j)
		      std::cerr<<prePt[j]<<", ";
		      std::cerr<<std::endl<<".........."<<std::endl;*/
		}
	     	      
	      if(a<0.5*alpha_&&ds_*a<0.1*dalpha_)
		{
		  ds_*=2;
		  if(ds_>dsmax_)
		    ds_/=2;
		}


	      addedArcLengthInAllIterations+=addedArcLengthInThisIteration;
	    }
	}
    }
    
    void MapManifoldStepper::advance(size_t n)
    {
      for(size_t i(0);i<n;++i)
	advance_once();
    }
    
  } /* end namespace num */
} /* end namespace scigma */

