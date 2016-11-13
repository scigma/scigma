#include <iostream>
#include "../internalequationsystem.hpp"

using namespace scigma::num;

int main()
{
  InternalEquationSystem eqsys;
  std::cout<<eqsys.parse("pow(27,1/3)")<<std::endl;
}
