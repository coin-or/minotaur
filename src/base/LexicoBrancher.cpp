//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file LexicoBrancher.cpp
 * \brief Define methods for Lexicographic branching.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <iostream>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrCand.h"
#include "BrVarCand.h"
#include "Environment.h"
#include "Handler.h"
#include "LexicoBrancher.h"
#include "Logger.h"
#include "Option.h"
#include "Problem.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Timer.h"

//#define SPEW 1

using namespace Minotaur;

const std::string LexicoBrancher::me_ = "Lexicobrancher: "; 

LexicoBrancher::LexicoBrancher()
  : handlers_(0),
    timer_(0)
{
  logger_ = new Logger(LogInfo);
  stats_ = new LexicoBrStats();
  stats_->calls = 0;
  stats_->time = 0.0;
}


LexicoBrancher::LexicoBrancher(EnvPtr env, HandlerVector handlers)
{
  logger_ = new Logger((LogLevel) 
                       env->getOptions()->findInt("br_log_level")->getValue());
  timer_ = env->getNewTimer();
  stats_ = new LexicoBrStats();
  stats_->calls = 0;
  stats_->time = 0.0;
  handlers_ = handlers;
}


LexicoBrancher::~LexicoBrancher()
{
  if (logger_) {
    delete logger_;
  }
  if (timer_) {
    delete timer_;
  }
  if (stats_) {
    delete stats_;
  }
  handlers_.clear();
}


Branches LexicoBrancher::findBranches(RelaxationPtr rel, NodePtr , 
                                      ConstSolutionPtr sol,
                                      SolutionPoolPtr s_pool,
                                      BrancherStatus & br_status, ModVector &) 
{
  Branches branches;
  DoubleVector x(rel->getNumVars());
  BrVarCandSet cands;      // Temporary set.
  BrCandVector gencands;      // Temporary set.
  BrCandPtr best_can = BrCandPtr(); // NULL
  ModVector mods;        // handlers may ask to modify the problem.
  Bool is_inf = false;

  timer_->start();
  std::copy(sol->getPrimal(), sol->getPrimal()+rel->getNumVars(), x.begin());
  //sol->write(std::cout);
  
  ++(stats_->calls);
  br_status = NotModifiedByBrancher;

  for (HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel, x, mods, cands, gencands, is_inf);
    if (cands.size()>0) {
      best_can = *(cands.begin());
      best_can->setHandler(*h);
      best_can->setDir(DownBranch);
      break;
    }
  }

  if (best_can) {
    branches = best_can->getHandler()->getBranches(best_can, x, rel, s_pool); 
#if SPEW
    logger_->MsgStream(LogDebug) << me_ << "best candidate = "
      << best_can->getName() << std::endl;
#endif
  } else {
    assert(!"problem finding candidate in LexicoBrancher");
  }
  stats_->time += timer_->query();
  timer_->stop();
  return branches;
}


void LexicoBrancher::writeStats() 
{
  if (stats_) {
    logger_->MsgStream(LogInfo) 
      << me_ << "times called = " << stats_->calls << std::endl
      << me_ << "time in brancher = " << stats_->time << std::endl;
  }
}


std::string LexicoBrancher::getName() const
{
  return "LexicoBrancher";
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
