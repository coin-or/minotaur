// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2024 The Minotaur Team.
// 

// /**
// \file LPRelaxation.cpp
// \brief Implement methods and functions of an LP Relaxation.
// \author Ashutosh Mahajan, Argonne National Laboratory
//
// */

#include "MinotaurConfig.h"
#include "LPRelaxation.h"
#include "Variable.h"


using namespace Minotaur;

LPRelaxation::LPRelaxation(EnvPtr env)
  : Relaxation(env),
  originalProblem_(0) // NULL
{
  jacobian_ = JacobianPtr();
  hessian_ = HessianOfLagPtr();
  logger_ = LoggerPtr();
  size_ = ProblemSizePtr() ;

}


LPRelaxation::LPRelaxation(ProblemPtr problem, EnvPtr env)
  : Relaxation(problem, env)
{
}


// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
