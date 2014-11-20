#include <iostream>
#include <cmath>
#include <string>
#include "mapmanifoldstepper.h"

namespace scigma
{
  namespace num
  {
    
    MapManifoldStepper::MapManifoldStepper(Stepper* mapStepper, Segment* segment, double dsmax, double dsmin, double alpha, size_t nPeriod):
      Stepper(false,mapStepper->nVar,mapStepper->nFunc,0),mapStepper_(mapStepper),segment_(segment),
      ds_(dsmax),dsmax_(dsmax),dsmin_(dsmin),alpha_(alpha),nPeriod_(nPeriod),current_(0),xBackup_(new double[mapStepper->nVar])
    {
      for(size_t i(0);i<mapStepper_->nVar;++i)
	xBackup_[i]=mapStepper->x(i);
    }
    
    MapManifoldStepper::~MapManifoldStepper()
    {
      delete mapStepper_;
      delete[] xBackup_;
    }
    
    double MapManifoldStepper::t() const {return segment_->last()[nVar];}
    double MapManifoldStepper::x(size_t index) const {return mapStepper_->x(index);}
    double MapManifoldStepper::func(size_t index) const {return mapStepper_->func(index);}
    double MapManifoldStepper::jac(size_t index) const {return 0*index;}
    void MapManifoldStepper::reset(const double* x){mapStepper_->reset(x);}
    
    double MapManifoldStepper::angle(double* q0,double* q1, double* q2)
    {
      double product(0);
      double d1(0);
      double d2(0);
      for(size_t i(0);i<nVar;++i)
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
      return acos(product);
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
	  mapStepper_->reset(xBackup_);
	  double* rq1(segment_->last());
	  double* rq0(segment_->next_to_last());
	  
	  double lastArc(rq1[nVar]);
      
	  double* q0(rq0),*q1(rq1),*q2(NULL),*q3(NULL);
	  while(ds_>dsmin_)
	    {
	      if(!(q2=segment_->step(mapStepper_,ds_,0.2)))
		throw(std::string("reached minimum bisection distance without convergence\n"));
	      //	  std::cerr<<"after step 1"<<std::endl;
	      if(angle(q0,q1,q2)>alpha_) // Angle too large on first step
		{
		  if(ds_>1e-10)
		    {
		      ds_/=2;
		      segment_->pop_back();
		      continue;
		    }
		}
	      if(!(q3=segment_->step(mapStepper_,ds_,0.2)))
		throw(std::string("reached minimum bisection distance without convergence\n"));
	      //	  std::cerr<<"after step 2"<<std::endl;
	      double a(angle(q1,q2,q3));
	      if(a>alpha_) // angle too large on second step
		{
		  if(ds_>1e-10)
		    {	     
		      ds_/=2; 
		      segment_->pop_back();
		      segment_->pop_back();
		      continue;
		    }
		}
	      segment_->pop_back();
	      if(a<alpha_/2)
		if(ds_<=dsmax_/2)
		  ds_*=2;

	      double distance(q2[nVar]-lastArc);
	      if(distance>dsmax_)
		{
		  mapStepper_->reset(q2);
		  for(size_t i(0);i<mapStepper_->nVar;++i)
		    xBackup_[i]=q2[i];
		  return;
		}
	      else if(distance<dsmin_)
		{
		  throw(std::string("reached minimum arclength step\n"));
		}
		
	      
	      rq0=q0;
	      q0=q1;
	      rq1=q1;
	      q1=q2;
	    }
	  throw(std::string("reached minimum arclength step\n"));
	}
    }
    
    void MapManifoldStepper::advance(size_t n)
    {
      for(size_t i(0);i<n;++i)
	advance_once();
    }
    
  } /* end namespace num */
} /* end namespace scigma */

