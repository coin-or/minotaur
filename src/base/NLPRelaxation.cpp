// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file NLPRelaxation.cpp
 * \brief Implement methods and functions of an NLP Relaxation.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include "MinotaurConfig.h"
#include "NLPRelaxation.h"


using namespace Minotaur;

NLPRelaxation::NLPRelaxation(EnvPtr env)
  : Relaxation(env),
    originalProblem_(0) // NULL
{
  jacobian_ = JacobianPtr();
  hessian_ = HessianOfLagPtr();
  logger_ = LoggerPtr();
  size_ = ProblemSizePtr() ;
}


NLPRelaxation::NLPRelaxation(ProblemPtr problem, EnvPtr env)
  : Relaxation(problem, env)
{
}


