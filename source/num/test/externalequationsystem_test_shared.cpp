#include "externalequationsystem_test_shared.hpp"
#include <cctype>
#include <cstdlib>
#include <cmath>

const std::string allowed("ABCDEFGHIJKLMNOPQRTSUVWXYZabcdefghijklmnopqrstuvwxyz_1234567890");

std::string well_formed_name(int maxLength)
{
  int length(std::rand()%(maxLength-1)+1);
  std::string result;
  for(int i(0);i<length;++i)
    {
      result+=allowed[std::rand()%allowed.size()];
      if(i==0&&std::isdigit(result[0]))
	{
	  result="";
	  --i;
	}
    }
  if(result=="t")
    return "s";
  return result;
}

std::string ill_formed_name(int maxLength)
{
  std::string result(well_formed_name(maxLength));
  char c;
  c = std::rand()%256;
  if(std::isdigit(c))
    {
      result[0]=c;
    }
  else
    {
      if((!std::isalnum(c))&&c!='_'&&c!='\n'&&c!='\r'&&c!='\a'&&c!='\f'&&c!='\v')
	{
	  int pos = std::rand()%result.size();
	  result[pos]=c;
	}
      else
	{
	  result[0]=' ';
	}
    }
  return result;
}

void f(const double* x, double* rhs)
{
  rhs[0]=x[0]+x[1]*x[0];
  rhs[1]=x[0]*x[1]-x[1];
}

void dfdx(const double* x, double* dfdx)
{
  dfdx[0]=1+x[1];dfdx[2]=x[0];
  dfdx[1]=x[1];dfdx[3]=x[0]-1;
}

void func(const double* x, double* funcval)
{
  funcval[0]=std::sin(x[0]+x[1]);
  funcval[1]=std::cos(x[0]+x[1]);
  funcval[2]=std::tan(x[0]+x[1]);
}


void f_p(const double* x, const double* p,  double* rhs)
{
  rhs[0]=x[0]+x[1]*(x[0]+p[1]);
  rhs[1]=x[0]*(x[1]-p[0])-x[1]*p[2];
}

void dfdx_p(const double* x, const double* p, double* dfdx)
{
  dfdx[0]=1+x[1];dfdx[2]=x[0]+p[1];
  dfdx[1]=x[1]-p[0];dfdx[3]=x[0]-p[2];
}

void dfdp_p(const double* x, const double* p, double* dfdp)
{
  dfdp[0]=0;dfdp[2]=x[1];dfdp[4]=0;
  dfdp[1]=-x[0];dfdp[3]=0;dfdp[5]=-x[1];
}

void func_p(const double* x, const double* p, double* funcval)
{
  funcval[0]=std::sin(x[0]+x[1])*p[0];
  funcval[1]=std::cos(x[0]+x[1])*p[1];
  funcval[2]=std::tan(x[0]+x[1])*p[2];
}

void f_t(double t, const double* x, double* rhs)
{
  rhs[0]=t*(x[0]+x[1]*x[0]);
  rhs[1]=(x[0]*x[1]-x[1])/t;
}

void dfdx_t(double t, const double* x, double* dfdx)
{
  dfdx[0]=t*(1+x[1]);dfdx[2]=t*x[0];
  dfdx[1]=x[1]/t;dfdx[3]=(x[0]-1)/t;
}

void func_t(double t, const double* x, double* funcval)
{
  funcval[0]=std::sin(x[0]*t+x[1]);
  funcval[1]=std::cos(x[0]+x[1])-t;
  funcval[2]=std::tan(x[0]+x[1]*t);
}

void f_pt(double t, const double* x, const double* p,  double* rhs)
{
  rhs[0]=t*(x[0]+x[1]*(x[0]+p[1]));
  rhs[1]=(x[0]*(x[1]-p[0])-x[1]*p[2])/t;
}

void dfdx_pt(double t, const double* x, const double* p, double* dfdx)
{
  dfdx[0]=t*(1+x[1]);dfdx[2]=t*(x[0]+p[1]);
  dfdx[1]=(x[1]-p[0])/t;dfdx[3]=(x[0]-p[2])/t;
}

void dfdp_pt(double t, const double* x, const double* p, double* dfdp)
{
  dfdp[0]=0;dfdp[2]=t*x[1];dfdp[4]=0;
  dfdp[1]=-x[0]/t;dfdp[3]=0;dfdp[5]=-x[1]/t;
}

void func_pt(double t, const double* x, const double* p, double* funcval)
{
  funcval[0]=std::sin(x[0]*t+x[1])*p[0];
  funcval[1]=std::cos(x[0]+x[1])*(p[1]-t);
  funcval[2]=std::tan(x[0]+x[1]*t)*p[2];
}
