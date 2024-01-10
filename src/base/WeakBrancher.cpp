//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file WeakBrancher.cpp
 * \brief Define methods for Weak branching.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 */

#include "WeakBrancher.h"

#include <cmath>

#include "BrCand.h"
#include "BrVarCand.h"
#include "Branch.h"
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

const std::string WeakBrancher::me_ = "weak brancher: ";

WeakBrancher::WeakBrancher(EnvPtr env, HandlerVector& handlers)
  : eTol_(1e-6),
    handlers_(handlers),
    p_(0),
    rel_(RelaxationPtr()),
    status_(NotModifiedByBrancher),
    x_(0)
{
  timer_ = env->getTimer();
  logger_ = env->getLogger();
  stats_ = new WeakBrStats();
  stats_->calls = 0;
  stats_->nodePruned = 0;
  stats_->time = 0.0;
}

WeakBrancher::~WeakBrancher()
{
  delete stats_;
}

double WeakBrancher::computeObjChange_(DoubleVector oldLb, DoubleVector oldUb)
{
  VariablePtr v;
  double obj_change = 0;
  double change;

  for(VariableConstIterator it = rel_->varsBegin(); it != rel_->varsEnd();
      ++it) {
    v = *it;
    if(rCost_[v->getIndex()] > eTol_) {
      change = rCost_[v->getIndex()] * (v->getLb() - oldLb[v->getIndex()]);
      obj_change += change > 0 ? change : 0;
    } else if(rCost_[v->getIndex()] < eTol_) {
      change = rCost_[v->getIndex()] * (v->getUb() - oldUb[v->getIndex()]);
      obj_change += change > 0 ? change : 0;
    }
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "variable " << v->getName()
                                  << " reduced cost = " << rCost_[v->getIndex()]
                                  << " change = " << change << std::endl;
#endif
  }

  return obj_change;
}

BrCandPtr WeakBrancher::findBestCandidate_(SolutionPoolPtr s_pool)
{
  double best_score = -INFINITY;
  double score, change_up, change_down;
  BrCandPtr cand, best_cand = 0;

  BrVarCandIter it;
  for(it = cands_.begin(); it != cands_.end(); ++it) {
    cand = *it;
    weakBranch_(cand, change_up, change_down, s_pool);
    if(status_ != NotModifiedByBrancher) {
      break;
    }
    score = getScore_(change_up, change_down);
#if SPEW
    writeScore_(cand, score, change_up, change_down);
#endif
    if(score > best_score) {
      best_score = score;
      best_cand = cand;
      if(change_up > change_down) {
        best_cand->setDir(DownBranch);
      } else {
        best_cand->setDir(UpBranch);
      }
    }
  }
  return best_cand;
}

Branches WeakBrancher::findBranches(RelaxationPtr rel, NodePtr,
                                    ConstSolutionPtr sol,
                                    SolutionPoolPtr s_pool,
                                    BrancherStatus& br_status, ModVector& mods)
{
  Branches branches = 0;
  BrCandPtr br_can = 0;
  const double* x = sol->getPrimal();
  bool should_prune = false;
  double stime = timer_->query();

  ++(stats_->calls);
  rel_ = rel;
  br_status = NotModifiedByBrancher;
  status_ = NotModifiedByBrancher;
  mods_.clear();
  rCost_ = sol->getDualOfVars();

  x_.resize(rel->getNumVars());
  std::copy(x, x + rel->getNumVars(), x_.begin());

  findCandidates_(should_prune);
  if(status_ == PrunedByBrancher) {
    br_status = status_;
    ++(stats_->nodePruned);
    return 0;
  }

  if(status_ == NotModifiedByBrancher) {
    br_can = findBestCandidate_(s_pool);
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
  stats_->time += timer_->query() - stime;
  return branches;
}

void WeakBrancher::findCandidates_(bool& should_prune)
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

void WeakBrancher::freeCandidates_(BrCandPtr no_del)
{
  for(BrVarCandIter it = cands_.begin(); it != cands_.end(); ++it) {
    if(no_del != *it) {
      delete *it;
    }
  }
  cands_.clear();
}

std::string WeakBrancher::getName() const
{
  return me_;
}

double WeakBrancher::getScore_(const double& up_score, const double& down_score)
{
  if(up_score > down_score) {
    return down_score * 0.8 + up_score * 0.2;
  } else {
    return up_score * 0.8 + down_score * 0.2;
  }
}

void WeakBrancher::setProblem(ProblemPtr p)
{
  p_ = p;
}

void WeakBrancher::weakBranch_(BrCandPtr cand, double& obj_up, double& obj_down,
                               SolutionPoolPtr s_pool)
{
  HandlerPtr h = cand->getHandler();
  ModificationPtr mod;
  ModificationPtr pmod;
  ModVector p_mods, r_mods;
  NodePtr node = NodePtr();
  DoubleVector oldLb, oldUb;
  bool is_inf_down, is_inf_up;

  for(VariableConstIterator vit = rel_->varsBegin(); vit != rel_->varsEnd();
      ++vit) {
    oldLb.push_back((*vit)->getLb());
    oldUb.push_back((*vit)->getUb());
  }
  // first do down.
  mod = h->getBrMod(cand, x_, rel_, DownBranch);
  mod->applyToProblem(rel_);
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
    is_inf_down = (*it)->getStrongerMods(rel_, node, s_pool, p_mods, r_mods);
    if(is_inf_down) {
      break;
    }
  }
  obj_down = is_inf_down ? 0 : computeObjChange_(oldLb, oldUb);
  h->undoStrongerMods(p_, rel_, p_mods, r_mods);
  if(pmod) {
    pmod->undoToProblem(p_);
  }
  delete pmod;
  mod->undoToProblem(rel_);
  delete mod;

  // now go up.
  mod = h->getBrMod(cand, x_, rel_, UpBranch);
  mod->applyToProblem(rel_);
  pmod = mod->fromRel(rel_, p_);
  if(pmod) {
    pmod->applyToProblem(p_);
  }
  p_mods.clear();
  r_mods.clear();
  for(HandlerIterator it = handlers_.begin(); it != handlers_.end(); ++it) {
    is_inf_up = (*it)->getStrongerMods(rel_, node, s_pool, p_mods, r_mods);
    if(is_inf_up) {
      break;
    }
  }
  obj_up = is_inf_up ? 0 : computeObjChange_(oldLb, oldUb);
  h->undoStrongerMods(p_, rel_, p_mods, r_mods);
  if(pmod) {
    pmod->undoToProblem(p_);
  }
  delete pmod;
  mod->undoToProblem(rel_);
  delete mod;

  if(is_inf_down && is_inf_up) {
    status_ = PrunedByBrancher;
    ++(stats_->nodePruned);
  } else if(is_inf_down) {
    status_ = ModifiedByBrancher;
    mods_.push_back(h->getBrMod(cand, x_, rel_, UpBranch));
  } else if(is_inf_up) {
    status_ = ModifiedByBrancher;
    mods_.push_back(h->getBrMod(cand, x_, rel_, DownBranch));
  }
}

void WeakBrancher::updateAfterSolve(NodePtr, ConstSolutionPtr) { }

void WeakBrancher::writeScore_(BrCandPtr cand, double score, double change_up,
                               double change_down)
{
  logger_->msgStream(LogDebug2)
      << me_ << "candidate: " << cand->getName()
      << " down change = " << change_down << " up change = " << change_up
      << " score = " << score << std::endl;
}

void WeakBrancher::writeStats(std::ostream& out) const
{
  if(stats_) {
    out << me_ << "times called                       = " << stats_->calls
        << std::endl
        << me_ << "times nodes was pruned by brancher = " << stats_->nodePruned
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
