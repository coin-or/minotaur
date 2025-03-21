// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file SolutionPool.cpp
 * \brief Define SolutionPool class for a storing solutions.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "SolutionPool.h"
#include "Timer.h"

using namespace Minotaur;

const std::string SolutionPool::me_ = "SolutionPool: ";

SolutionPool::SolutionPool (EnvPtr env, ProblemPtr problem, UInt limit)
: bestSolution_(SolutionPtr()), // NULL
  numSolsFound_(0),
  problem_(problem),
  sizeLimit_(limit),
  timeBest_(-1),
  timeFirst_(-1)
{
  timer_ = env->getTimer(); // should not be deleted.
}


SolutionPool::~SolutionPool()
{
  for (std::vector<SolutionPtr>::iterator it=sols_.begin(); it!=sols_.end(); 
       ++it) {
    delete *it;
  }
}


void SolutionPool::addSolution(ConstSolutionPtr solution)
{
  SolutionPtr newsol = new Solution(solution);
  ++numSolsFound_;
  if (sols_.size() > 0) {
    if (sols_[0]->getObjValue() > solution->getObjValue()) {
      delete sols_[0];
      sols_[0] = newsol;
      bestSolution_ = newsol;
      timeBest_ = timer_->query();
    } else {
      delete newsol;
    }
  } else {
    sols_.push_back(newsol);
    bestSolution_ = newsol;
    timeFirst_ = timer_->query();
    timeBest_ = timeFirst_;
  }
}


void SolutionPool::addSolution(const double *x, double obj_value)
{
  SolutionPtr solution = new Solution(obj_value, x, problem_);
  addSolution(solution);
  delete solution;
}


SolutionPtr SolutionPool::getBestSolution()
{
  return bestSolution_;
}


double SolutionPool::getBestSolutionValue() const
{
  if (bestSolution_) {
    return bestSolution_->getObjValue();
  } else {
    return INFINITY;
  }
}


UInt SolutionPool::getNumSols() const
{
  return sols_.size();
}


UInt SolutionPool::getNumSolsFound() const
{
  return numSolsFound_;
}


void SolutionPool::writeStats(std::ostream &out) const
{
  out << me_ << "Number of solutions found = " << numSolsFound_ << std::endl
      << me_ << "Time first solution found = " << timeFirst_    << std::endl
      << me_ << "Time best solution found  = " << timeBest_     << std::endl
      ;
}

