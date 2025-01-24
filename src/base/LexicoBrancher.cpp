//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2024 The Minotaur Team.
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
  logger_ = (LoggerPtr) new Logger(LogInfo);
  stats_ = new LexicoBrStats();
  stats_->calls = 0;
  stats_->time = 0.0;
}


LexicoBrancher::LexicoBrancher(EnvPtr env, HandlerVector handlers)
{
  logger_ = env->getLogger();
  timer_ = env->getNewTimer();
  stats_ = new LexicoBrStats();
  stats_->calls = 0;
  stats_->time = 0.0;
  handlers_ = handlers;
}


LexicoBrancher::~LexicoBrancher()
{
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
                                      BrancherStatus &br_status,
                                      ModVector &mods) 
{
  Branches branches = 0;
  DoubleVector x(rel->getNumVars());
  BrVarCandSet cands;
  BrCandVector gencands;
  BrVarCandSet cands2;              // Temporary set.
  BrCandVector gencands2;           // Temporary set.
  BrCandPtr best_can = BrCandPtr(); // NULL
  bool is_inf = false;

  timer_->start();
  std::copy(sol->getPrimal(), sol->getPrimal()+rel->getNumVars(), x.begin());
  //sol->write(std::cout);
  
  ++(stats_->calls);
  br_status = NotModifiedByBrancher;

  for (HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel, x, mods, cands2, gencands2, is_inf);
    for (BrVarCandIter it = cands2.begin(); it != cands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    for (BrCandVIter it = gencands2.begin(); it != gencands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    cands.insert(cands2.begin(), cands2.end());
    gencands.insert(gencands.end(), gencands2.begin(), gencands2.end());
    cands2.clear();
    gencands2.clear();
    if (is_inf) {
      cands.clear();
      gencands.clear();
      br_status = PrunedByBrancher;
      return branches;
    } 
  }

  if (mods.size()>0) {
      cands.clear();
      gencands.clear();
      br_status = ModifiedByBrancher;
      return branches;
  }

  if (cands.size()>0) {
    best_can = *(cands.begin());
  } else if (gencands.size()>0) {
    best_can = *(cands.begin());
  }

  if (best_can) {
    best_can->setDir(DownBranch);
    branches = best_can->getHandler()->getBranches(best_can, x, rel, s_pool); 
    for (BranchConstIterator br_iter=branches->begin();
         br_iter!=branches->end(); ++br_iter) {
      (*br_iter)->setBrCand(best_can);
    }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "best candidate = "
      << best_can->getName() << std::endl;
#endif
  } else {
    assert(!"problem finding candidate in LexicoBrancher");
  }
  stats_->time += timer_->query();
  timer_->stop();
  return branches;
}


void LexicoBrancher::writeStats(std::ostream &out) const
{
  if (stats_) {
    out << me_ << "times called = " << stats_->calls << std::endl
      << me_ << "time in brancher = " << stats_->time << std::endl;
  }
}


std::string LexicoBrancher::getName() const
{
  return "LexicoBrancher";
}


