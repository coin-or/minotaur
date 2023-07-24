//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file StongBrancher.cpp
 * \brief Define methods for stong branching.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 */

#include "StrongBrancher.h"

#include <algorithm>
#include <cmath>

#include "BrCand.h"
#include "BrVarCand.h"
#include "Branch.h"
#include "Engine.h"
#include "Environment.h"
#include "Handler.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "Modification.h"
#include "Node.h"
#include "Option.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"

using namespace Minotaur;

const std::string StrongBrancher::me_ = "strong brancher: ";

StrongBrancher::StrongBrancher(EnvPtr env, HandlerVector& handlers)
  : engine_(EnginePtr()),
    eTol_(1e-6),
    handlers_(handlers),
    maxCands_(0),
    maxIterations_(0),
    p_(0),
    rel_(RelaxationPtr()),
    status_(NotModifiedByBrancher),
    x_(0)
{
  timer_ = env->getTimer();
  logger_ = env->getLogger();
  stats_ = new StrBrStats();
  stats_->calls = 0;
  stats_->engProbs = 0;
  stats_->nodePruned = 0;
  stats_->time = 0.0;
  init_ = false;
  stronger_ = false;
}

StrongBrancher::~StrongBrancher()
{
  delete stats_;
}

void StrongBrancher::doStronger()
{
  stronger_ = true;
}

BrCandPtr StrongBrancher::findBestCandidate_(const double objval,
                                             SolutionPoolPtr s_pool)
{
  double best_score = -INFINITY;
  double score, change_up, change_down, maxchange;
  double cutoff = s_pool->getBestSolutionValue();
  UInt cnt;
  EngineStatus status_up, status_down;
  BrCandPtr cand, best_cand = 0;
  UInt maxTSR;

  maxchange = cutoff - objval;
  BrVarCandIter it;
  // engine_->enableStrBrSetup();
  if(maxIterations_ > 0) {
    engine_->setIterationLimit(maxIterations_);
  }
  if(maxCands_ > 0) {
    maxTSR = findMaxTSR_();
  }
  cnt = 0;
  for(it = cands_.begin();
      it != cands_.end() && (cnt < maxCands_ || maxCands_ == 0); ++it) {
    cand = *it;
    if(maxCands_ > 0 && timesStrBranched_[cand->getPCostIndex()] > maxTSR) {
      continue;
    }
    ++cnt;
    strongBranch_(cand, change_up, change_down, status_up, status_down, s_pool);
    change_up = std::max(change_up - objval, 0.0);
    change_down = std::max(change_down - objval, 0.0);
    useStrongBranchInfo_(cand, maxchange, change_up, change_down, status_up,
                         status_down);
    score = getScore_(change_up, change_down);
#if SPEW
    writeScore_(cand, score, change_up, change_down);
#endif
    if(status_ != NotModifiedByBrancher) {
      break;
    }
    if(score > best_score) {
      best_score = score;
      best_cand = cand;
      if(change_up > change_down) {
        best_cand->setDir(DownBranch);
      } else {
        best_cand->setDir(UpBranch);
      }
    }
    if(maxCands_ > 0) {
      timesStrBranched_[cand->getPCostIndex()] += 1;
    }
  }
  engine_->resetIterationLimit();
  // engine_->disableStrBrSetup();
  return best_cand;
}

Branches StrongBrancher::findBranches(RelaxationPtr rel, NodePtr,
                                      ConstSolutionPtr sol,
                                      SolutionPoolPtr s_pool,
                                      BrancherStatus& br_status,
                                      ModVector& mods)
{
  Branches branches = 0;
  BrCandPtr br_can = 0;
  const double* x = sol->getPrimal();
  bool should_prune = false;

  ++(stats_->calls);
  rel_ = rel;
  br_status = NotModifiedByBrancher;
  status_ = NotModifiedByBrancher;
  mods_.clear();
  if(!init_) {
    init_ = true;
    if(maxCands_ > 0) {
      timesStrBranched_ = UIntVector(rel_->getNumVars(), 0);
    }
  }

  // make a copy of x, because it is overwritten while strong branching.
  x_.resize(rel->getNumVars());
  std::copy(x, x + rel->getNumVars(), x_.begin());

  findCandidates_(should_prune);
  if(status_ == PrunedByBrancher) {
    br_status = status_;
    ++(stats_->nodePruned);
    return 0;
  }

  if(status_ == NotModifiedByBrancher) {
    br_can = findBestCandidate_(sol->getObjValue(), s_pool);
  }

  // status_ might have changed now. Check again.
  if(status_ == NotModifiedByBrancher) {
    // surrounded by br_can :-)
    if(br_can) {
      branches = br_can->getHandler()->getBranches(br_can, x_, rel_, s_pool);
      for(BranchConstIterator br_iter = branches->begin();
          br_iter != branches->end(); ++br_iter) {
        (*br_iter)->setBrCand(br_can);
      }
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "best candidate = " << br_can->getName() << std::endl;
#endif
    } else {
      br_status = NoCandToBranch;
    }
  } else {
    // we found some modifications that can be done to the node. Send these
    // back to the processor.
    if(mods_.size() > 0) {
      mods.insert(mods.end(), mods_.begin(), mods_.end());
    }
    br_status = status_;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "found modifications" << std::endl;
    if(mods_.size() > 0) {
      for(ModificationConstIterator miter = mods_.begin(); miter != mods_.end();
          ++miter) {
        (*miter)->write(logger_->msgStream(LogDebug));
      }
    } else if(status_ == PrunedByBrancher) {
      logger_->msgStream(LogDebug) << me_ << "Pruned." << std::endl;
    } else {
      logger_->msgStream(LogDebug)
          << me_ << "unexpected status = " << status_ << std::endl;
    }
#endif
  }

  freeCandidates_(br_can);
  if(status_ != NotModifiedByBrancher && br_can) {
    delete br_can;
  }
  return branches;
}

void StrongBrancher::findCandidates_(bool& should_prune)
{
  BrVarCandSet cands2;    // Temporary set.
  BrCandVector gencands2; // Temporary set.
  BrCandPtr br_can;
  std::pair<BrVarCandIter, bool> ret;

  should_prune = false;
  cands_.clear();
  gencands_.clear();
  for(HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel_, x_, mods_, cands2, gencands2,
                                 should_prune);
    if(should_prune || mods_.size() > 0) {
      for(BrVarCandIter it = cands2.begin(); it != cands2.end(); ++it) {
        delete *it;
      }
      for(BrCandVIter it = gencands2.begin(); it != gencands2.end(); ++it) {
        delete *it;
      }
      if(should_prune) {
        status_ = PrunedByBrancher;
      } else {
        status_ = ModifiedByBrancher;
      }
      break;
    }
    for(BrVarCandIter it = cands2.begin(); it != cands2.end();) {
      (*it)->setHandler(*h);
      ret = cands_.insert(*it);
      if(false == ret.second) { // already exists.
        br_can = *(ret.first);
        if(br_can->getDDist() + br_can->getUDist() <=
           (*it)->getDDist() + (*it)->getUDist()) {
          br_can->setHandler(*h);
        }
        br_can->setDist(br_can->getDDist() + (*it)->getDDist(),
                        br_can->getUDist() + (*it)->getUDist());
        delete *it;            // free the candidate
        it = cands2.erase(it); // delete the iterator from cands2
      } else {
        ++it;
      }
    }
    for(BrCandVIter it = gencands2.begin(); it != gencands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    gencands_.insert(gencands_.end(), gencands2.begin(), gencands2.end());
    cands2.clear();
    gencands2.clear();
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "candidates: " << std::endl;
  for(BrVarCandIter it = cands_.begin(); it != cands_.end(); ++it) {
    logger_->msgStream(LogDebug)
        << (*it)->getName() << "\t" << (*it)->getDDist() << "\t"
        << (*it)->getUDist() << std::endl;
  }
#endif
  return;
}

UInt StrongBrancher::findMaxTSR_()
{
  UIntVector candElems;
  candElems.clear();

  for(BrVarCandIter it = cands_.begin(); it != cands_.end(); ++it) {
    candElems.push_back(timesStrBranched_[(*it)->getPCostIndex()]);
  }
  std::nth_element(candElems.begin(), candElems.begin() + maxCands_,
                   candElems.end());
  return candElems[maxCands_ - 1];
}

void StrongBrancher::freeCandidates_(BrCandPtr no_del)
{
  for(BrVarCandIter it = cands_.begin(); it != cands_.end(); ++it) {
    if(no_del != *it) {
      delete *it;
    }
  }
  cands_.clear();
}

std::string StrongBrancher::getName() const
{
  return "ReliabilityBrancher";
}

double StrongBrancher::getScore_(const double& up_score,
                                 const double& down_score)
{
  if(up_score > down_score) {
    return down_score * 0.8 + up_score * 0.2;
  } else {
    return up_score * 0.8 + down_score * 0.2;
  }
}

void StrongBrancher::setEngine(EnginePtr engine)
{
  engine_ = engine;
}

void StrongBrancher::setMaxCands(UInt max_cands)
{
  maxCands_ = max_cands;
}

void StrongBrancher::setMaxIter(UInt max_iter)
{
  maxIterations_ = max_iter;
}

void StrongBrancher::setProblem(ProblemPtr p)
{
  p_ = p;
}

bool StrongBrancher::shouldPrune_(const double& chcutoff, const double& change,
                                  const EngineStatus& status, bool* is_rel)
{
  switch(status) {
  case(ProvenLocalInfeasible):
    return true;
  case(ProvenInfeasible):
    return true;
  case(ProvenObjectiveCutOff):
    return true;
  case(ProvenLocalOptimal):
  case(ProvenOptimal):
    if(change > chcutoff - eTol_) {
      return true;
    }
    // check feasiblity
    break;
  case(EngineUnknownStatus):
    assert(!"engine status is UnknownStatus in reliability branching!");
    break;
  case(EngineIterationLimit):
    break;
  case(ProvenFailedCQFeas):
  case(ProvenFailedCQInfeas):
    logger_->msgStream(LogInfo) << me_ << "Failed CQ."
                                << " Continuing." << std::endl;
    *is_rel = false;
    break;
  default:
    logger_->errStream() << me_ << "unexpected engine status. "
                         << "status = " << status << std::endl;
    *is_rel = false;
    stats_->engProbs += 1;
    break;
  }
  return false;
}

void StrongBrancher::strongBranch_(BrCandPtr cand, double& obj_up,
                                   double& obj_down, EngineStatus& status_up,
                                   EngineStatus& status_down,
                                   SolutionPoolPtr s_pool)
{
  HandlerPtr h = cand->getHandler();
  ModificationPtr mod;
  double stime;
  ModificationPtr pmod;
  ModVector p_mods, r_mods;
  NodePtr node = NodePtr();
  bool is_inf;

  // first do down.
  status_down = EngineUnknownStatus;
  mod = h->getBrMod(cand, x_, rel_, DownBranch);
  mod->applyToProblem(rel_);
  if(stronger_) {
#if SPEW
    logger_->msgStream(LogDebug2)
        << me_ << "Stronger branching of " << cand->getName() << std::endl;
#endif
    pmod = mod->fromRel(rel_, p_);
    if(pmod) {
      pmod->applyToProblem(p_);
    }
    p_mods.clear();
    r_mods.clear();
    for(HandlerIterator it = handlers_.begin(); it != handlers_.end(); ++it) {
      is_inf = (*it)->getStrongerMods(rel_, node, s_pool, p_mods, r_mods);
      if(is_inf) {
        status_down = ProvenInfeasible;
        break;
      }
    }
  }
  if(status_down == EngineUnknownStatus) {
    stime = timer_->query();
    status_down = engine_->solve();
    stats_->time += timer_->query() - stime;
  }
  obj_down = engine_->getSolutionValue();
  if(stronger_) {
    h->undoStrongerMods(p_, rel_, p_mods, r_mods);
    if(pmod) {
      pmod->undoToProblem(p_);
    }
    delete pmod;
  }
  mod->undoToProblem(rel_);
  delete mod;

  // now go up.
  mod = h->getBrMod(cand, x_, rel_, UpBranch);
  mod->applyToProblem(rel_);
  if(stronger_) {
    pmod = mod->fromRel(rel_, p_);
    if(pmod) {
      pmod->applyToProblem(p_);
    }
    p_mods.clear();
    r_mods.clear();
    for(HandlerIterator it = handlers_.begin(); it != handlers_.end(); ++it) {
      is_inf = (*it)->getStrongerMods(rel_, node, s_pool, p_mods, r_mods);
      if(is_inf) {
        status_down = ProvenInfeasible;
        break;
      }
    }
  }
  if(status_down == EngineUnknownStatus) {
    stime = timer_->query();
    status_up = engine_->solve();
    stats_->time += timer_->query() - stime;
  }
  obj_up = engine_->getSolutionValue();
  if(stronger_) {
    h->undoStrongerMods(p_, rel_, p_mods, r_mods);
    if(pmod) {
      pmod->undoToProblem(p_);
    }
    delete pmod;
  }
  mod->undoToProblem(rel_);
  delete mod;
}

void StrongBrancher::updateAfterSolve(NodePtr, ConstSolutionPtr) { }

void StrongBrancher::useStrongBranchInfo_(BrCandPtr cand,
                                          const double& chcutoff,
                                          double& change_up,
                                          double& change_down,
                                          const EngineStatus& status_up,
                                          const EngineStatus& status_down)
{
  bool should_prune_up = false;
  bool should_prune_down = false;
  bool is_rel = true;

  should_prune_down = shouldPrune_(chcutoff, change_down, status_down, &is_rel);
  should_prune_up = shouldPrune_(chcutoff, change_up, status_up, &is_rel);

  if(!is_rel) {
    change_up = 0.;
    change_down = 0.;
  } else if(should_prune_up == true && should_prune_down == true) {
    status_ = PrunedByBrancher;
    ++(stats_->nodePruned);
  } else if(should_prune_up) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, DownBranch));
  } else if(should_prune_down) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, UpBranch));
  } else {
    return;
  }
}

void StrongBrancher::writeScore_(BrCandPtr cand, double score, double change_up,
                                 double change_down)
{
  logger_->msgStream(LogDebug2)
      << me_ << "candidate: " << cand->getName()
      << " down change = " << change_down << " up change = " << change_up
      << " score = " << score << std::endl;
}

void StrongBrancher::writeStats(std::ostream& out) const
{
  if(stats_) {
    out << me_ << "times called                       = " << stats_->calls
        << std::endl
        << me_ << "no. of problems in engine          = " << stats_->engProbs
        << std::endl
        << me_ << "times nodes was pruned by brancher = " << stats_->nodePruned
        << std::endl
        << me_ << "time taken in strong branching     = " << stats_->time
        << std::endl;
  }
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
