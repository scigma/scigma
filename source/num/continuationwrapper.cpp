#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../dat/wave.hpp"
#include "equationsystem.hpp"
#include "auto.hpp"
#include "task.hpp"

#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::num;
typedef scigma::dat::AbstractWave<double> Wave;

namespace scigma
{
  namespace num
  {
     AutoConstants auto_constants_from_blob(Blob* blob)
     {
       AutoConstants AC;
       AC.IRS = blob->get_int("IRS");
       AC.NPR = blob->get_int("NPR");
       AC.DS = blob->get_double("stepsize.ds");
       AC.DSMIN = blob->get_double("stepsize.dsmin");
       AC.DSMAX = blob->get_double("stepsize.dsmax");
       AC.EPSL = blob->get_double("tolerances.epsl");
       AC.EPSU = blob->get_double("tolerances.epsu");
       AC.EPSS = blob->get_double("tolerances.epss");
       AC.RL0 = blob->get_double("bounds.rl0");
       AC.RL1 = blob->get_double("bounds.rl1");
       AC.A0 = blob->get_double("bounds.a0");
       AC.A1 = blob->get_double("bounds.a1");
       AC.NTST = blob->get_int("advanced AUTO.NTST");
       AC.NCOL = blob->get_int("advanced AUTO.NCOL");
       AC.IAD = blob->get_int("advanced AUTO.IAD");
       AC.IADS = blob->get_int("advanced AUTO.IADS");
       AC.ITMX = blob->get_int("advanced AUTO.ITMX");
       AC.ITNW = blob->get_int("advanced AUTO.ITNW");
       AC.NMX = blob->get_int("advanced AUTO.NMX");
       AC.ILP = blob->get_int("advanced AUTO.ILP");
       AC.ISP = blob->get_int("advanced AUTO.ISP");
       AC.ISW = blob->get_int("advanced AUTO.ISW");
       AC.MXBF = blob->get_int("advanced AUTO.MXBF");
       AC.IPS = blob->get_int("advanced AUTO.IPS");
       AC.IIS = blob->get_int("advanced AUTO.IIS");
       AC.IID = blob->get_int("advanced AUTO.IID");
       AC.IPLT = blob->get_int("advanced AUTO.IPLT");
       AC.NUNSTAB = blob->get_int("HOMCONT.NUNSTAB");
       AC.NSTAB = blob->get_int("HOMCONT.NSTAB");
       AC.IEQUIB = blob->get_int("HOMCONT.IEQUIB");
       AC.ITWIST = blob->get_int("HOMCONT.ITWIST");
       AC.ISTART = blob->get_int("HOMCONT.ISTART");

       return AC;
     }
    
  } /* end namespace num */
} /* end namespace scigma */


extern "C"
{
  PythonID scigma_num_auto(const char* identifier, PythonID equationSystemID, PythonID logID,
			   int nSteps, const int* ICP, int nICP, PythonID blobID, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return -1;
    PYOBJ(Log,log,logID);if(!log)return -1;
    PYOBJ(Blob,blob,blobID);if(!blob)return -1;

    AutoConstants AC(auto_constants_from_blob(blob));
    for(size_t i(0), nVar(eqsys->n_variables());i<nVar;++i)
      AC.unames[int(i+1)]=eqsys->variable_names()[i];
    for(size_t i(0), nPar(eqsys->n_parameters());i<nPar;++i)
      AC.parnames[int(i+1)]=eqsys->parameter_names()[i];
    for(int i(0);i<nICP;++i)
      AC.ICP.push_back(ICP[i]+1);
    AC.NDIM=int(eqsys->n_variables());
    AC.NMX=nSteps;
    AC.c=AC.s=AC.dat=AC.sv=identifier;
    
    
    if(noThread)
      {
	Auto aUto(*eqsys,log,AC);
      }

    return -1;
  }
}
  
/* extern "C" block */

#pragma clang pop
