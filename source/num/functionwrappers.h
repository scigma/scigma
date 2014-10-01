#ifndef __SCIGMA_NUM_FUNCTIONWRAPPERS_H__
#define __SCIGMA_NUM_FUNCTIONWRAPPERS_H__

#include <functional>
#include "definitions.h"
#include "odessa.h"

namespace scigma
{
  namespace num
  {
    // create inputs for Odessa from a set of Function objects
    Odessa::F odessa_f(Function& T, VecF& X, VecF& RHS);
    Odessa::DFDX odessa_dfdx(VecF& X, VecF& RHS);

    void build_partial_derivative_matrix(const VecF& vars, const VecF& funcs,std::vector<Function>& matrix);

    
    // Odessa::DFDP odessa_dfdp(EquationSystem& equ, std::vector<std::string> parameters = std::vector<std::string>(0));   

    // create input for map iteration from a set of Function objects
    //    Iterator::F iterator_f(Function& STEP, VecF& X, VecF& RHS);

    /*    // create inputs for Newton for ODEs an instance of EquationSystem
    F system_f(EquationSystem& equ);
    F system_dfdx(EquationSystem& equ);
    F system_f_dfdx(EquationSystem& equ);*/

    /*    F flow_f(size_t nVar, Odessa& odessa, double tau);
    F flow_f_dfdx(size_t nVar, Odessa& odessa, double tau);
    F poincare_f(size_t nVar, Odessa& odessa, double dt, size_t variable, double value, size_t maxSteps=10000);
    F poincare_f_dfdx(size_t nVar, Odessa& odessa,  const F& vector_field, double dt, size_t variable, double value, size_t maxSteps=10000);
    */
    /*
    // create input for Newton for maps from user input
    Newton::F_DFDX newton_map_f_dfdx(int nVar, int (*f)(), double** xLoc, double** rhsLoc, int (*dfdx)()=NULL, double** dfdxLoc=NULL, int period=1, double* work=NULL);
    // create input for Newton for Poincare maps from user input
    Newton::F_DFDX newton_poincare_f_dfdx(Odessa& odessa,double dt,int variable, double value, int period=1, double* work=NULL);
    */
  } /* end namespace num */

} /* end namespace scigma */

#endif /* __SCIGMA_NUM_FUNCTIONWRAPPERS_H__ */
