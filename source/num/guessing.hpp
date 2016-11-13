#ifndef SCIGMA_NUM_GUESSING_HPP
#define SCIGMA_NUM_GUESSING_HPP

#include "../common/log.hpp"
#include "../dat/wave.hpp"
#include "newton.hpp"
#include "equationsystem.hpp"
#include "stepper.hpp"
#include "task.hpp"

using scigma::common::Log;
typedef scigma::dat::AbstractWave<double> Wave;

namespace scigma
{
  namespace num
  {
    
    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, Wave* varyingWave,
			       Wave* evWave, double tol, size_t nPeriod, size_t showAllIterates, long secvar);

    Task* create_guessing_task(std::string identifier, Log* log, EquationSystem* eqsys, 
			       Wave* varyingWave, Wave* evWave, double tol);
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_GUESSING_HPP */
