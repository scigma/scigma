#ifndef __SCIGMA_NUM_UTIL_H__
#define __SCIGMA_NUM_UTIL_H__

#include <functional>
#include "../log.h"
#include "../dat/wave.h"
#include "definitions.h"
#include "function.h"
#include "stepper.h"
#include "newton.h"
#include "segment.h"
#include "equationsystem.h"

namespace scigma
{
  namespace num
  {

    extern const char* APPLICATION_SUCCESS_PREFIX;
    extern const char* APPLICATION_FAIL_PREFIX;
    extern const char* APPLICATION_ERROR_PREFIX;


    /* takes a pointer to function with the Task
       signature and runs, then deletes it.
       run_task is meant to be used as first argument
       to the tthread::thread() constructor. Be careful
       to use only heap allocated Tasks as second 
       argument, and do not delete them in the routine
       calling tthrread::thread()
    */
    typedef std::function<void(void)> Task; 
    void run_task(void* taskPtr);
    
    Stepper* create_map_stepper(const EquationSystem& eqsys);
    
    Stepper* create_ode_stepper(const EquationSystem& eqsys, double dt, bool extJac, 
				bool stiff, double aTol, double rTol, size_t maxIter);

    Stepper* create_poincare_stepper(const EquationSystem& eqsys, double maxtime, double dt,
				     int secvar, int secdir, double secval, double tol,
				     bool extJac, bool stiff, double aTol, double rTol, size_t maxIter);
    
    Task* create_stepping_task(std::string identifier, Log* log, Stepper* stepper, size_t nSteps, size_t nPeriod, dat::Wave* wave);

    Newton::F* create_ode_newton_function(const EquationSystem& eqsys, bool extJac);

    Newton::F* create_map_newton_function(const EquationSystem& eqsys, size_t nPeriod, bool extJac);

    F* create_additional_function_evaluator(const EquationSystem& eqsys);

    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, dat::Wave* varyingWave,
			       dat::Wave* evWave, double tol, size_t nPeriod, long secvar);

    Task* create_guessing_task(std::string identifier, Log* log, size_t nVar, size_t nFunc,
			       Newton::F* f, std::function<void(const double*, double*)>* ff,
			       dat::Wave* varyingWave, dat::Wave* evWave, bool extJac, double tol, bool isMap);

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_UTIL_H__ */
