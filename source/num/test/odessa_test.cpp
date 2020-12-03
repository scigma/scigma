#include "../odessa.hpp"
#include <catch.hpp>
#include <tinythread.h>
#include <vector>

extern "C" void odessa_(void (*F)(int*,double*,double*,double*,double*),void(*DF)(int*, double*, double*, double*, double*, int*), 
			int* NEQ, double* Y, double* RPAR, double* TIN,double* TOUT,  int* ITOL,  double* RTOL,
			double* ATOL, int* ITASK, int* ISTATE, int* IOPT, double* RWORK, int* LRW, int* IWORK, 
			int* LIW,void (*JAC)(int*, double*, double*, double*, int*, int*,double*, int*), int* MF, double* RCOMM, int* ICOMM);

double p[]={8./3.,28,10};

extern "C" void F(int* n, double* t, double* x, double* p_not_used, double* rhs) 
{
  rhs[0]=p[2]*(x[1]-x[0]);
  rhs[1]=x[0]*(p[1]-x[2]*(*t));
  rhs[2]=x[0]*x[1]-p[0]*x[2];
}

extern "C" void DFDX(int* n, double* t, double* x, double* p_not_used, int* ml, int* mu, double* dfdx, int* nrowpd)
{
  dfdx[0]=-p[2];dfdx[3]=p[2];
  dfdx[1]=p[1]-x[2]*(*t);dfdx[7]=-x[0]*(*t);
  dfdx[2]=x[1];dfdx[5]=x[0];dfdx[8]=-p[0];
}

void f(double t, const double* x, double* rhs)
{
  double T(t);
  return F(NULL, &T,const_cast<double*>(x), NULL ,rhs );
}

void dfdx(double t, const double* x, double* dfdx)
{
  double T(t);
  return DFDX(NULL, &T,const_cast<double*>(x),NULL,NULL,NULL,dfdx,NULL);
}

SCENARIO ("integration with Odessa in a single thread","[odessa][single-thread]")
{
  double aTol(1e-9),rTol(1e-9);
  int iTol(1),iTask(1),iState(1);
  int iOpt[]={1,0,0};
  bool stiff(true);

  double rComm[222];
  int iComm[54];
  
  GIVEN("Lorenz Equations, modified to be time dependent")
    {
      size_t nVar(3);
      int NEQ[]={int(nVar),0};
      
      int lrw=int(22+15*(nVar+1)*nVar+nVar*nVar+nVar);
      std::vector<double> rWork(lrw);
      
      double tIn(1);
      double tOut(2);
      double x[]={1,2,3,1,0,0,0,1,0,0,0,1};
      
      WHEN("Odessa is run without the Jacobian and the sensitivity function")
	{
	  int liw(int(20+nVar));
	  std::vector<int> iWork(liw);

	  for(size_t i(4);i<9;++i)
		  rWork[i]=iWork[i]=0;
	  iWork[5]=20000;
	  
	  int mf=22;
	  	  
	  scigma::num::Odessa odessa(nVar,f);
	  odessa.t()=1;
	  for(size_t i(0);i<nVar;++i)
	    odessa.x()[i]=x[i];
	  
	  odessa.integrate(1);
	  odessa_(F,NULL,NEQ,x,NULL,&tIn,&tOut,&iTol,&rTol,&aTol,&iTask,&iState,iOpt,&rWork[0],&lrw,&iWork[0],&liw,NULL,&mf,rComm,iComm);
	  
	  THEN("C++ elapsed t() value matches raw Fortran elapsed t value")
	      REQUIRE(odessa.t()==tOut);
	  THEN("C++ evolved x() state matches raw Fortran odessa evolved x state")
	    for(size_t i(0);i<nVar;++i)
	      REQUIRE(odessa.x()[i]==x[i]);
	  THEN("C++ sensitivity() returns NULL")
	    REQUIRE(odessa.sensitivity()==NULL);
	}
      WHEN("Odessa is run with the Jacobian but without the sensitivity function")
	{
	  iOpt[1]=0;

	  int liw(20+nVar);
	  std::vector<int> iWork(liw);
	  
	  for(size_t i(4);i<9;++i)
	    rWork[i]=iWork[i]=0;
	  iWork[5]=20000;
	 
	  int mf=21;

	  double tIn(1);
	  double tOut(2);
	  double x[]={1,2,3,1,0,0,0,1,0,0,0,1};

	  scigma::num::Odessa odessa(nVar,f,dfdx);
	  odessa.t()=1;
	  for(size_t i(0);i<nVar;++i)
	    odessa.x()[i]=x[i];

	  odessa.integrate(1);
	  odessa_(F,NULL,NEQ,x,NULL,&tIn,&tOut,&iTol,&rTol,&aTol,&iTask,&iState,iOpt,&rWork[0],&lrw,&iWork[0],&liw,DFDX,&mf,rComm,iComm);

	  THEN("C++ t() value matches raw Fortran elapsed t value")
	      REQUIRE(odessa.t()==tOut);
	  THEN("C++ x() values match raw Fortran odessa evolved x values")
	    for(size_t i(0);i<nVar;++i)
	      REQUIRE(odessa.x()[i]==x[i]);
	  THEN("C++ sensitivity() returns NULL")
	    REQUIRE(odessa.sensitivity()==NULL);
	}
      WHEN("Odessa is run with the Jacobian and the sensitivity function")
	{
	  iOpt[1]=1;
	  
	  int liw(21+nVar+nVar);
	  std::vector<int> iWork(liw);
	  for(size_t i(4);i<9;++i)
	    rWork[i]=iWork[i]=0;
	  iWork[5]=20000;
	 
	  int mf=21;

	  NEQ[1] = nVar;
	  
	  double tIn(1);
	  double tOut(2);
	  double x[]={1,2,3,1,0,0,0,1,0,0,0,1};
	  
	  scigma::num::Odessa odessa(nVar,f,dfdx,stiff,aTol,rTol,20000,true);
	  odessa.t()=1;
	  for(size_t i(0);i<nVar*(nVar+1);++i)
	    odessa.x()[i]=x[i];

	  double P[]={0,0,0};
	  
	  odessa.integrate(1);
	  odessa_(F,NULL,NEQ,x,P,&tIn,&tOut,&iTol,&rTol,&aTol,&iTask,&iState,iOpt,&rWork[0],&lrw,&iWork[0],&liw,DFDX,&mf,rComm,iComm);

	  THEN("C++ t() value matches raw Fortran elapsed t value")
	    REQUIRE(odessa.t()==tOut);
	  THEN("C++ x() values match raw Fortran odessa evolved x values")
	    for(size_t i(0);i<nVar;++i)
	      REQUIRE(odessa.x()[i]==x[i]);
	  THEN("C++ sensitivity() values match raw Fortran odessa evolved sensitivity values")
	    for(size_t i(0);i<nVar*nVar;++i)
	      REQUIRE(odessa.sensitivity()[i]==x[i+nVar]);
	}
    }
}

SCENARIO ("Odessa: switching between integrations in multiple threads","[odessa][multi-thread]")
{
  GIVEN("Four instances of Odessa with the same equations and initial conditions")
    {
      size_t nVar(3);
      
      double x[]={1,2,3,1,0,0,0,1,0,0,0,1};

      scigma::num::Odessa* odessa[4];
      for(size_t j(0);j<4;++j)
	{
	  odessa[j]=new scigma::num::Odessa(nVar,f,dfdx,false,1e-9,1e-9,20000,true);
	  odessa[j]->t()=1;
	  for(size_t i(0);i<nVar*(nVar+1);++i)
	    odessa[j]->x()[i]=x[i];
	}
      WHEN("Three of them are run intermittently")
	{
	  for(size_t i(0);i<10;++i)
	    {
	      odessa[0]->integrate(1);
	      odessa[1]->integrate(1);
	      odessa[2]->integrate(1);
	    }
	  THEN("The results are the same as for the fourth instance, which runs single")
	    {
	      for(size_t i(0);i<10;++i)
		 odessa[3]->integrate(1);

	      for(size_t j(0);j<3;++j)
		{
		  for(size_t i(0);i<nVar;++i)
		    REQUIRE(odessa[j]->x()[i]==odessa[3]->x()[i]);
		  for(size_t i(0);i<nVar*nVar;++i)
		    REQUIRE(odessa[j]->sensitivity()[i]==odessa[3]->sensitivity()[i]);
		}
	    }
	}
    }
}

void run(void* data)
{
  double* results = static_cast<double*>(data);

  size_t nVar(3);
  
  double x[]={1,2,3,1,0,0,0,1,0,0,0,1};
  
  scigma::num::Odessa odessa(nVar,f,dfdx,false,1e-9,1e-9,20000,true);
  odessa.t()=1;
  for(size_t i(0);i<nVar*(nVar+1);++i)
    odessa.x()[i]=x[i];
  
  odessa.integrate(10);
  
  for(size_t i(0);i<nVar;++i)
    results[i]=odessa.x()[i];
  for(size_t i(0);i<nVar*nVar;++i)
    results[i+nVar]=odessa.sensitivity()[i];
}

SCENARIO("Odessa: multithreaded integration","[odessa][parallel]")
{
  GIVEN("Four threads running instances of Odessa with the same equations and initial conditions")
    {
      double* data=new double[48];
      tthread::thread thread1(run,static_cast<void*>(data));
      tthread::thread thread2(run,static_cast<void*>(data+12));
      tthread::thread thread3(run,static_cast<void*>(data+24));
      tthread::thread thread4(run,static_cast<void*>(data+36));

      thread1.join();
      thread2.join();
      thread3.join();
      thread4.join();
      
      THEN("All instances produce the same output")
	{
	  for(size_t i(0);i<12;++i)
	    {
	      REQUIRE(data[i]==data[i+12]);
	      REQUIRE(data[i+24]==data[i+24]);
	      REQUIRE(data[i+36]==data[i+36]);
	    }
	}
    }
}

