#include <string>

const std::string lorenz[] = {
  "x'=s*(y-x)\n",
  "y'=x*(r-z)-y\n",
  "z'=x*y-b*z\n",
  "r=28.0\n",
  "s=10.0\n",
  "b=2.66666667\n",
  "f_1=x**2-y**2+s\n",
  "f_2=sqrt(abs(f_1+z))\n",
  "c=r+s*sin(b)\n",
  "x=-1.24\n",
  "y=$c*12\n",
  "z=-tan(23)\n",
};
const size_t lorenzLines(12);

const std::string forced[] = {
  "x'=-y+a*f1+b*(1-x)\n",
  "y'=x+a*f2+b*(1-y)\n",
  "f1=cos(omega*t)\n",
  "f2=sin(omega*t)\n",
  "t=-2\n",
  "x=0.5\n",
  "y=0.6\n",
  "a=0.001\n",
  "b=0.1\n",
  "omega=5\n"
};

const size_t forcedLines(10);
