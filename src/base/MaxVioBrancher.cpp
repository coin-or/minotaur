//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file MaxVioBrancher.cpp
 * \brief Define methods for maximum violation branching candidate.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iomanip>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrCand.h"
#include "Environment.h"
#include "Handler.h"
#include "Logger.h"
#include "MaxVioBrancher.h"
#include "Modification.h"
#include "Node.h"
#include "Option.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Timer.h"

//#define SPEW 1

using namespace Minotaur;

const std::string MaxVioBrancher::me_ = "MaxVio brancher: "; 

MaxVioBrancher::MaxVioBrancher(EnvPtr env, HandlerVector & handlers) 
: rel_(RelaxationPtr()),  // NULL
  handlers_(handlers)     // Create a copy, the vector is not too big
{
  timer_ = env->getNewTimer();
  logger_ = (LoggerPtr) new Logger((LogLevel) env->getOptions()->
                                   findInt("br_log_level")->getValue());
  stats_ = new MaxVioBrStats();
  stats_->calls = 0;
  stats_->time = 0.0;
}


MaxVioBrancher::~MaxVioBrancher()
{
  delete stats_;
  delete timer_;
  logger_.reset();
}


Branches MaxVioBrancher::findBranches(RelaxationPtr rel, NodePtr , 
                                      ConstSolutionPtr sol,
                                      SolutionPoolPtr s_pool, 
                                      BrancherStatus & br_status,
                                      ModVector &) 
{
  Branches branches;
  BrCandPtr br_can = BrCandPtr(); //NULL
  const Double *x = sol->getPrimal();

  timer_->start();
  x_.resize(rel->getNumVars());
  std::copy(x, x+rel->getNumVars(), x_.begin());

  //sol->write(std::cout);
  
  ++(stats_->calls);
  rel_ = rel;
  br_status = NotModifiedByBrancher;

  findCandidates_();
  br_can = findBestCandidate_();

  if (br_can) {

    branches = br_can->getHandler()->getBranches(br_can, x_, rel_, s_pool); 
    for (BranchConstIterator br_iter=branches->begin(); 
         br_iter!=branches->end(); ++br_iter) {
      (*br_iter)->setBrCand(br_can);
    }
#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "best candidate = "
                               << br_can->getName() 
                               << " Handler: " << br_can->getHandler()->getName() << std::endl;
#endif
  } else {
    assert(!"problem finding candidate in MaxVioBrancher");
  }
  stats_->time += timer_->query();
  timer_->stop();
  return branches;
}


void MaxVioBrancher::findCandidates_()
{
  BrCandSet cands2;      // Temporary set.
  ModVector mods;        // handlers may ask to modify the problem.
  Bool is_inf = false;

  cands_.clear();
  for (HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel_, x_, mods, cands2, is_inf);
    for (BrCandIter it = cands2.begin(); it != cands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    cands_.insert(cands2.begin(), cands2.end());
    cands2.clear();
  }

#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "candidates: " << std::endl;
  for (BrCandIter it=cands_.begin(); it!=cands_.end(); ++it) {
    logger_->MsgStream(LogDebug)
        << std::setprecision(6) << (*it)->getName() << "\t" 
        << x_[(*it)->getPCostIndex()] << "\t"
        << std::endl;
  }
#endif
  return;
}


BrCandPtr MaxVioBrancher::findBestCandidate_()
{
  BrCandPtr best_cand = BrCandPtr(); // NULL
  Double best_score = -INFINITY;
  Double cand_score;

  for (BrCandIter it = cands_.begin(); it != cands_.end(); ++it) {
    cand_score = std::min((*it)->getDDist(),(*it)->getUDist());
    if (cand_score > best_score) {
      best_score = cand_score;
      best_cand = (*it);
    }
  }

  if (best_cand) {
    if (best_cand->getDDist() > best_cand->getUDist()) {
      best_cand->setDir(DownBranch);
    } else {
      best_cand->setDir(UpBranch);
    }
  }

  return best_cand;
}


void MaxVioBrancher::updateAfterLP(NodePtr , ConstSolutionPtr )
{
}


void MaxVioBrancher::writeStats()
{
  if (stats_) {
    logger_->MsgStream(LogInfo) 
      << me_ << "times called = " << stats_->calls << std::endl
      << me_ << "time in brancher = " << stats_->time << std::endl;
  }
}


std::string MaxVioBrancher::getName() const
{
  return "MaxVioBrancher";
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
