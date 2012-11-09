// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
// 

/**
 * \file SolutionPool.cpp
 * \brief Define SolutionPool class for a storing solutions.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>

#include "MinotaurConfig.h"
#include "SolutionPool.h"

using namespace Minotaur;


Double SolutionPool::getBestSolutionValue() const
{
  if (bestSolution_) {
    return bestSolution_->getObjValue();
  } else {
    return INFINITY;
  }
}


SolutionPtr SolutionPool::getBestSolution()
{
  return bestSolution_;
}


void SolutionPool::addSolution(ConstSolutionPtr solution)
{
  // XXX: for now we save only one solution.
  SolutionPtr newsol = (SolutionPtr) new Solution(solution);
  if (sols_.size() > 0) {
    if (sols_[0]->getObjValue() > solution->getObjValue()) {
      sols_[0] = newsol;
      bestSolution_ = newsol;
    }
  } else {
    sols_.push_back(newsol);
    bestSolution_ = newsol;
  }
}


void SolutionPool::addSolution(const Double *x, Double obj_value)
{
  SolutionPtr solution = (SolutionPtr) new Solution(obj_value, x, problem_);
  addSolution(solution);
}


SolutionPool::SolutionPool (ProblemPtr problem, UInt limit)
: bestSolution_(SolutionPtr()), // NULL
  problem_(problem),
  sizeLimit_(limit)
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
