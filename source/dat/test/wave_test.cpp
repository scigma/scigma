#include "../wave.hpp"
#include <catch.hpp>
#include <tinythread.h>

typedef scigma::dat::AbstractWave<double> Wave;

SCENARIO("Wave: reading and writing in a single thread","[Wave][single-thread]")
{
  GIVEN("A Wave object initialized with a single value and capacity 1")
    {
      Wave w(1);

      w.push_back(3.14);

      THEN("size() returns 1")
	  REQUIRE(w.size()==1);
      THEN("the value is retrieved with data()[0]")
	  REQUIRE(w.data()[0]==3.14);
      WHEN("a range of values is pushed")
	{
	  double D[]={1,2,3,4,5};
	  w.push_back(D,5);
	  THEN("the size increases by the number of values")
	      REQUIRE(w.size()==6);
	  THEN("the values are retrieved with data()[...]")
	    {
	      const double* ddata=w.data();
	      for(size_t i(0);i<5;++i)
		  REQUIRE(ddata[i+1]==D[i]);
	    }
	}
    }
}

void read(void* data)
{
  Wave* w = static_cast<Wave*>(data);
  while(true)
    {
      w->lock();
      const double* d(w->data());
      size_t s(w->size());
      if(s==600)
	break;
      for(size_t i(0);i<s;++i)
	if(d[i]<0){w=NULL;}
      w->unlock();
    }
}

void write(void* data)
{
  Wave* w = static_cast<Wave*>(data);
  double d[]={1,2,3};
  for(size_t i(0);i<100;++i)
    {
      w->lock();
      w->push_back(d,3);
      w->unlock();
    }
}


SCENARIO("Wave: writing in one thread and reading from another","[Wave][multi-thread]")
{
  GIVEN("An empty Wave object, two threads that write to it and one thread that reads from it")
    {
      Wave w;

      tthread::thread thread1(write,static_cast<void*>(&w));
      tthread::thread thread2(write,static_cast<void*>(&w));
      tthread::thread thread3(read,static_cast<void*>(&w));

      thread1.join();
      thread2.join();
      thread3.join();

      THEN("the threads interact without deadlocks and the final state of the data is as expected")
	{
	  REQUIRE(w.size()==600);
	  const double* d(w.data());
	  for(size_t i(0);i<400;++i)
	    REQUIRE(double(i%3+1)==d[i]);
	}
    }
}
