#ifndef SCIGMA_NUM_ITERATION_HPP
#define SCIGMA_NUM_ITERATION_HPP

#include "equationsystem.hpp"
#include "stepper.hpp"
#include "task.hpp"

namespace scigma
{
  namespace num
  {

    Task* create_stepping_task(std::string identifier, Log* log, Stepper* stepper, size_t nSteps, size_t nPeriod, size_t showAllIterates, dat::Wave* wave);

  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_ITERATION_HPP */
