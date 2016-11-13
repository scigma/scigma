#ifndef SCIGMA_NUM_SWEEPING_HPP
#define SCIGMA_NUM_SWEEPING_HPP

#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../dat/mesh.hpp"
#include "equationsystem.hpp"
#include "odessa.hpp"
#include "task.hpp"

using scigma::dat::Mesh;
using scigma::common::Log;
using scigma::common::Blob;

namespace scigma
{
  namespace num
  {
    
    Odessa::F create_extended_system(size_t nVar, double& g,
				     std::vector<double>& grad_g,
				     double& f_grad_g,
				     double** f,
				     double* fabs,
				     F_t f_t
				     );
    
    Odessa::DFDX create_extended_jacobian(size_t nVar, double& g,
					  std::vector<double>& grad_g,
					  double& f_grad_g,
					  double** f,
					  double* fabs,
					  std::vector<double>& dfdx1,
					  std::vector<double>& dfdx2,
					  std::vector<double>& grad_g_jac,
					  F_t dfdx_t);
    
    void build_mesh(std::string identifier, Log* log, size_t nVar,
		    Odessa::F f_extended, Odessa::DFDX jac_extended,
		    size_t nFunc, F_t func_t, size_t nSteps, Mesh* mesh, Blob* blob);
    void build_mesh_2(std::string identifier, Log* log, size_t nVar,
		    Odessa::F f_extended, Odessa::DFDX jac_extended,
		    size_t nFunc, F_t func_t, size_t nSteps, Mesh* mesh, Blob* blob);
    
    Task* create_sweeping_task(std::string identifier, EquationSystem* eqsys,
			       Log* log, size_t nSteps, Mesh* mesh, Blob* blob);
  }
}

#endif /* SCIGMA_NUM_SWEEPING_HPP */
