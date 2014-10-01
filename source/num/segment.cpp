#include <cassert>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <string>
#include <algorithm>
#include "segment.h"

namespace scigma
{
  namespace num
  {

    Segment::Segment(Stepper* stepper, size_t nPeriod, double eval, dat::Wave* initial,double eps,double deltaMin):
      PythonObject<Segment>(this),nVar_(stepper->nVar),nPeriod_(nPeriod),deltaMin_(deltaMin),fail_(false)
    {
      // insert fixed point
      double* fp=map_[0.0]=new double[nVar_+3];
      for(uint32_t i(0);i<nVar_;++i)
	fp[i]=(*initial)[1+i];
      fp[nVar_]=fp[nVar_+1]=fp[nVar_+2]=0;
      // insert the approximate preimage of the end point of the initial segment:
      double* preInit=map_[eps/eval]=new double[nVar_+3];
      for(uint32_t i(0);i<nVar_;++i)
	preInit[i]=fp[i]+((*initial)[uint32_t(initial->columns())+1+i]-fp[i])/eval;
      preInit[nVar_]=eps/eval;preInit[nVar_+1]=eps/eval/eval;preInit[nVar_+2]=eps;
      // insert end point of initial segment
      double* init=map_[eps]=new double[nVar_+3];
      stepper->reset(preInit);
      stepper->advance(nPeriod);
      for(uint32_t i(0);i<nVar_;++i)
	init[i]=stepper->x(i);
      init[nVar_]=eps;init[nVar_+1]=eps/eval;

      stepper->reset(init);

      /*      std::cout<<"first three:"<<std::endl;
      std::cout.precision(15);
      for(size_t i(0);i<nVar_;++i)
	std::cout<<map_[0.0][i]<<", "<<map_[eps/eval][i]<<", "<<map_[eps][i]<<std::endl;
      */
      padding_[0]=0;
    }

    Segment::~Segment()
    {
      SegPt i(map_.begin()),end(map_.end());
      while(i!=end)
	delete [] (i++)->second;
    }

    void Segment::pop_back()
    {
      SegPt p((++map_.rbegin()).base());
      //      std::cerr<<"remove at "<<p->second[nVar_]<<", ";
      delete[] p->second;
      map_.erase(p);
      //      std::cerr<<size()<<std::endl;
    }
 
    double* Segment::rewind_ptr(double* pData)
    {
      return map_[pData[nVar_+1]];
    }
    
    double* Segment::advance_ptr(double* pData)
    {
      return map_[pData[nVar_+2]];
    }
    
    Segment::SegPt Segment::rewind_point(SegPt p)
    {
      return map_.find(p->second[nVar_+1]);
    }
    
    Segment::SegPt Segment::advance_point(SegPt p)
    {
      return map_.find(p->second[nVar_+2]);
    }     

    Segment::SegPt Segment::insert(double* pData)
    {
      std::pair<SegPt,bool> res(map_.insert(std::make_pair(pData[nVar_],pData)));
      //      std::cerr<<"inserting: ";
      //      for(size_t i(0);i<nVar_;++i)
      //	std::cerr<<pData[i]<<", ";
      //	std::cerr<<"----"<<std::endl;
      if(!res.second) // if the point was already in map_, delete incoming pointer  
	  delete [] pData;
      return res.first;
    }
    
    void Segment::discard_after(double arc)
    {
      SegPt q((++map_.rbegin()).base());
      while(q->second[nVar_]>arc)
	{
	  delete[] q->second;
	  --q;
	}
      map_.erase(++q,map_.end());
    }
    
    /* This routine provides a point q on the manifold which is delta+-eps further along than the 
       last point (q1), if q can be reached by a single iteration of the map from a point within
       the part of the manifold that has already been traced. If not, it returns q = f_(q1).
       The routine returns NULL if there is an internal error in the map function f_, if the search
       for a suitable preimage p of q does not converge, or if the distance |q-q1| falls below
       the threshold of deltaMin_;
    */
    double* Segment::step(Stepper* stepper, double delta, double eps)
    {
      if(fail_)
	return NULL;
      SegPt q1((++map_.rbegin()).base()); // get last point of the manifold;
      SegPt p1(rewind_point(q1)); // preimage of last point 
      
      double dev(delta*eps);
      double* data(new double[nVar_+3]);
      try
	{
	  if(!find_point_in_interval(stepper,p1,map_.end(),q1,delta, dev, 1,data))
	    {
	      //	      std::cerr<<"????"<<std::endl;
	      delete [] data;
	      return NULL;
	    }
	  else
	    return data;
	}
      catch(std::string error)
	{
	  delete [] data;
	  throw error;
	}
    }
    
    bool Segment::test_guess(Stepper* stepper, double* p1Data, double* pData, double* p2Data, 
			     double* q1Data, double delta, double& dev, size_t depth, double* data)
    {
      //      std::cerr<<"test guess at "<<pData[nVar_]<<": "<<std::endl;
      //      std::cerr<<"with x="<<pData[0]<<" and y="<<pData[1]<<std::endl; 
      double ratio(p2Data?(pData[nVar_]-p1Data[nVar_])/(p2Data[nVar_]-p1Data[nVar_]):0.0);
      double* qData(NULL);
      for(size_t i(0);i<depth-1;++i)
	{
	  //std::cerr<<i<<",";
	  qData=qData?qData:new double[nVar_+3];
	  try
	    {
	      stepper->reset(pData);
	      stepper->advance(nPeriod_);
	      for(size_t j(0);j<nVar_;++j)
		qData[j]=stepper->x(j);
	    }
	  catch(std::string error)
	    {
	      delete[] qData;
	      throw error;
	    }
	  p1Data=advance_ptr(p1Data);p2Data=advance_ptr(p2Data);
	  // fake arclength by interpolation
	  //	  std::cerr<<p1Data[nVar_]<<" -,- ";
	  //std::cerr<<p2Data[nVar_]<<" +,+ "<<std::endl;
	  double arc(p1Data[nVar_]+ratio*(p2Data[nVar_]-p1Data[nVar_]));
	  // if(arc>=p1Data[nVar_]+deltaMin_||arc<=p2Data[nVar_]-deltaMin_) // only insert if f(p) is not too close to f(p1) and/or f(p2) 
	  //  {
	      pData[nVar_+2]=arc;
	      qData[nVar_]=arc;
	      qData[nVar_+1]=pData[nVar_];
	      pData=insert(qData)->second; // make sure pData points to the entry map_[arc] 
	      qData=NULL;
	      //  }
	      //else
	      // {
	      // qData[nVar_]=-1.0; // indicates that this point is not part of the manifold (read out in next iteration of the loop)
	      //    pData=qData;
	      // }
	}
      //      std::cerr<<std::endl;
	  
      /* For the last image, which maps beyond the last accepted point on the manifold,
	 calculate the real arc length, i.e the distance to q1. Do not insert it into
	 the manifold yet, because it might violate accuracy conditions on the manifold
	 without being subject to an external check (angle, etc.)
      */
      try
	{
	  stepper->reset(pData);
	  stepper->advance(nPeriod_);
	  for(size_t i(0);i<nVar_;++i)
	    data[i]=stepper->x(i);
	}
      catch(std::string error)
	{
	  delete[] pData;
	  throw error;
	}
      
      double d(0);
      for(size_t j=0;j<nVar_;++j)
	{
	  double diff(data[j]-q1Data[j]);
	  d+=diff*diff;
	}
      d=sqrt(d);
      dev=d-delta; //return the deviation from desired delta in dev
      double arc(q1Data[nVar_]+d);
      pData[nVar_+2]=arc;
      data[nVar_]=arc;
      data[nVar_+1]=pData[nVar_];
      //      std::cerr<<"dev =  "<<dev<<std::endl;
      return true;
    }

    bool Segment::find_point_in_interval_by_bisection(Stepper* stepper,double* p1Data, double* p2Data, double* q1Data, double delta, double eps, size_t depth, double* data)
    {
      //      std::cerr<<"enter bisection between "<<p1Data[nVar_]<<" and "<<(p2Data?p2Data[nVar_]:1e300)<<std::endl;
      double *pData(new double [nVar_+3]);
      for(size_t i(0);i<nVar_+1;++i)
	pData[i]=(p1Data[i]+p2Data[i])/2;
      pData[nVar_+1]=-1.0; // point has no preimage!
      bool recursive(false);
      if(pData[nVar_]-p1Data[nVar_]>=deltaMin_&&p2Data[nVar_]-pData[nVar_]>=deltaMin_)
	{
	  insert(pData);
	  recursive=true;
	}
      double dev(eps);
      try
	{
	  if(!test_guess(stepper,p1Data,pData,p2Data,q1Data,delta,dev,depth,data)) // compute f^depth(p) and insert images into manifold
	    {
	      if(!recursive)
		delete[] pData;
	      return false;
	    }
	}
      catch(std::string error)
	{
	  if(!recursive)
	    delete[] pData;
	  throw error;
	}
      if(dev<eps&&dev>-eps)
	{
	  eps=dev;
	  insert(data);
	  return true;
	}
      if(recursive)
	{
	  if(dev>0)
	    return find_point_in_interval_by_bisection(stepper,p1Data,pData,q1Data,delta,eps,depth,data);
	  else
	    return find_point_in_interval_by_bisection(stepper,pData,p2Data,q1Data,delta,eps,depth,data);
	}
      else
	{
	  delete[] pData;
	  return false;
	}
    }

    /* This function expects two points on the manifold (p1,p2) which bracket a point that maps to a point
       in q in a distance delta from q1 (further along the manifold) after depth iterations of the map,
       while p1 and p2 do not map to a point within a distance of eps from q themselves.
       If there is already a point that does map to q+-eps in the previously recorded manifold
       between p1 and p2, the coordinates of this point are returned.
       Otherwise, the depth is increased and the search is resumed one iteration backwards,
       possibly with a narrowed search window. If the depth cannot be increased any more, because there
       are no more preimages available for the end points, find_point_in_interval_by_bisection is called.
    */
    bool Segment::find_point_in_interval(Stepper* stepper,SegPt p1, SegPt p2, SegPt q1, double delta, double eps, size_t depth, double* data)
    {
      SegPt p(p1);
      double dev(0);
      double *p1Data(p1->second),*p2Data(map_.end()==p2?NULL:p2->second),*q1Data(q1->second);
      while(++p!=p2)
	{
	  if(!test_guess(stepper,p1Data,p->second,p2Data,q1Data,delta,dev,depth,data))
	    return false;
	  if(dev<eps&&dev>-eps) // point is within tolerance
	    {
	      insert(data);
	      return true;
	    }
	  if(dev>eps)
	    break;
	}
      /* If we get here, p1 and p enclose a point that maps to q1+delta. However, the point 
	 must first be constructed from a preimage on an increased depth level, because
	 all available preimages on this depth level map to a point which is too far
	 away from q1+delta
      */
      if(p!=map_.end())
	{
	  p2=rewind_point(p);
	  p1=rewind_point(--p);
	  if(p1!=map_.end()&&p2!=map_.end())
	    {
		return find_point_in_interval(stepper,p1,p2,q1,delta,eps,depth+1,data);
	    }
	  else 
	    {
	      /* If we get here, there is no preimage in the manifold for at least one of the end
		 points of the current segment. This happens if we are on the initial segment or
		 if one of the endpoints was obtained by interpolation rather than iterating f_.
		 If we get here, we add a new point in the middle of the current segment and try 
		 again at the same depth level.
	      */
	      p1=p;
	      p2=(++p);	
	      return find_point_in_interval_by_bisection(stepper,p1->second,p2->second,q1Data,delta,eps,depth,data);	      
	    }
	}
      else
	{
	  /* This should only happen at depth==1; if it happens, the function inserts and 
	     returns the image of the last point, although it falls short of delta; 
	  */
	  assert(depth==1);
	  insert(data);
	  return true;
	}
    }
     
    size_t Segment::size()
    {
      return map_.size();
    }
    
    double* Segment::last()
    {
      return map_.rbegin()->second;
    }

    double* Segment::next_to_last()
    {
      return (++map_.rbegin())->second;
    }

    void Segment::dump()
    {
      SegPt p(map_.begin()), end(map_.end());
      while(p!=end)
	{
	  for(size_t i(0);i<nVar_+3;++i)
	    std::cout<<p->second[i]<<"--\t";
	  std::cout<<std::endl;
	  ++p;
	}
      std::cout<<"size: "<<size()<<std::endl;
    }

    /*    extern double angle(size_t nVar,double* q0,double* q1, double* q2);
    
    double angle(size_t nVar,double* q0,double* q1, double* q2)
    {
      double product(0);
      double d1(0);
      double d2(0);
      for(size_t i(0);i<nVar;++i)
	{
	  std::cerr<<q0[i]<<", "<<q1[i]<<", "<<q2[i]<<std::endl;
	  double diff1(q0[i]-q1[i]),diff2(q1[i]-q2[i]);
	  product+=diff1*diff2;
	  d1+=diff1*diff1;
	  d2+=diff2*diff2;
	}
      product/=sqrt(d1)*sqrt(d2);
      std::cerr<<product<<", "<<acos(product)<<", ";
      return acos(product);
      }*/
    
    /*    void extend_manifold_segment2(std::vector<dat::Wave*>& segment, F& map, double eval, double finalArc, double alphaMin, double alphaMax)
    {
      if(finalArc<0)
	return;
      size_t nVar(segment.size()-1);
      dat::Wave** coords=&segment[0];
      dat::Wave* arcLength(segment[nVar]);
      double* var=new double[nVar];
      double* evec=new double[nVar];
      double delta((*arcLength)[1]);
      for(size_t i(0);i<nVar;++i)
	{
	  var[i]=(*coords[i])[0];
	  evec[i]=((*coords[i])[1]-(*coords[i])[0])/delta;
	}
      Segment s(nVar,map,var,eval,evec,delta);
      
      double* rq0(var);
      double* rq1(s.last());
      double rDelta(delta);

      while(rDelta>1e-12)
	{
	  double* q0(rq0),*q1(rq1),*q2(NULL),*q3(NULL);
	  delta=rDelta;
	  
	  for(size_t i(0);i<100000;++i)//820;++i)
	    {
	      double alpha;
	      std::cerr<<"delta="<<delta<<std::endl;
	      std::cerr<<"step 1:"<<std::endl;
	      if(!(q2 = s.step(delta,0.2)))
		break;
	      if((alpha=angle(nVar,q0,q1,q2))>alphaMax)
		{
		  std::cerr<<"alpha on 1:"<<alpha<<std::endl;
		  s.pop_back();
		  delta/=2;
		  continue;
		}
	      std::cerr<<"step 2:"<<std::endl;
	      if(!(q3 = s.step(delta,0.2)))
		break;
	      if((alpha=angle(nVar,q1,q2,q3))>alphaMax)
		{
		  std::cerr<<"alpha on 2:"<<alpha<<std::endl;
		  s.pop_back();
		  s.pop_back();
		  delta/=2;
		  continue;
		}
	      else
		{
		  std::cerr<<"normal retraction, alpha="<<alpha<<":"<<std::endl;
		  s.pop_back();
		  for(size_t j(0);j<nVar+1;++j)
		    std::cout<<q1[j]<<"\t";
		  std::cout<<std::endl;
		  rq0=q0;
		  q0=q1;
		  rq1=q1;
		  q1=q2;
		  rDelta=delta;
		  if(alpha<alphaMin)
		    delta*=2;
		}
	    }
	  s.discard_after(rq1[nVar]);
	  std::cerr<<"neuer Versuch: delta="<<rDelta/2<<std::endl;
	  rDelta/=2;
	}
          s.dump();

      delete[] evec;
      delete[] var;
      }*/
    

  } /* end namespace num */
} /* end namespace scigma */
