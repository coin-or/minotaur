//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file MaxInfBrancher.cpp
 * \brief Define methods for hybrid-inference branching name as maxinf.
 * \author Krushna Salunke, IIT Bombay
 */
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <omp.h>

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
#include "MaxInfBrancher.h"

using namespace Minotaur;

const std::string MaxInfBrancher::me_ = "MaxInf Brancher: "; 
MaxInfBrancher::MaxInfBrancher(EnvPtr env, HandlerVector & handlers) 
  : engine_(EnginePtr()),
    eTol_(1e-6),
    maxDepth_(1000),
    maxIterations_(25),
    maxStrongCands_(20),
    minNodeDist_(50),
    thresh_(4),
    trustCutoff_(true),
    status_(NotModifiedByBrancher),
    rel_(RelaxationPtr()),
    x_(0),
    handlers_(handlers),
    init_(false)
{
  timer_  = env->getNewTimer();
  logger_ = env->getLogger();
  stats_  = new MaxInfBrStats();
  stats_->calls = 0;
  stats_->engProbs = 0;
  stats_->strBrCalls = 0;
  stats_->bndChange = 0;
  stats_->iters = 0;
  stats_->time = 0.0;
}


MaxInfBrancher::~MaxInfBrancher()
{
  delete stats_;
  delete timer_;
}

Branches MaxInfBrancher::findBranches(RelaxationPtr rel, NodePtr node, ConstSolutionPtr sol, SolutionPoolPtr s_pool, BrancherStatus & br_status, ModVector& mods) 
{
  Branches branches = 0;
  BrCandPtr br_can = 0;
  const double* x = sol->getPrimal();

  ++(stats_->calls);
  if(!init_) {
    init_ = true;
    initialize(rel);
  }
  rel_ = rel;
  br_status = NotModifiedByBrancher;
  status_ = NotModifiedByBrancher;
  mods_.clear();

  // make a copy of x, because it is overwritten while strong branching.
  x_.resize(rel->getNumVars());
  std::copy(x, x + rel->getNumVars(), x_.begin());

  infScore();
  findCandidates_();
  updateFracCount_();
  updateUnfixedCount_();
/*
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Inference deducted by candidate : \n"
                               << std::setw(20) << "Variable"
                               << std::setw(15) << "Up Score"
                               << std::setw(15) << "Down Score" << std::endl;
   for(BrCandVIter it = relCands_.begin(); it != relCands_.end(); ++it){
    UInt index = (*it)->getPCostIndex();
    double upScore = upScores_[index];
    double downScore = downScores_[index];
    logger_->msgStream(LogError)
        << std::setw(20) << (*it)->getName()
        << std::setw(15) << std::setprecision(6) << upScore
        << std::setw(15) << std::setprecision(6) << downScore << std::endl;
  }
#endif
*/

  if(status_ == PrunedByBrancher) {
    br_status = status_;
    return 0;
  }

  if(status_ == NotModifiedByBrancher) {
    br_can = findBestCandidate_(sol->getObjValue(),
                                s_pool->getBestSolutionValue(), node);
  }

  // status_ might have changed now. Check again.
  if(status_ == NotModifiedByBrancher) {
    // surrounded by br_can :-)
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
    // we found some modifications that can be done to the node. Send these
    // back to the processor.
    if(mods_.size() > 0) {
      mods.insert(mods.end(), mods_.begin(), mods_.end());
    }
    br_status = status_;
 /*
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
*/
  }

  freeCandidates_(br_can);
  if(status_ != NotModifiedByBrancher && br_can) {
    delete br_can;
  }
  return branches;
}

void MaxInfBrancher::initialize(RelaxationPtr rel)
{
  int n = rel->getNumVars();
  // initialize following vectors to zero.
  upScores_ = std::vector<double>(n, 0.0);  // Initialize up scores to 0
  downScores_ = std::vector<double>(n, 0.0); 
  pseudoUp_ = DoubleVector(n, 0.);
  pseudoDown_ = DoubleVector(n, 0.);
  lastStrBranched_ = UIntVector(n, 20000);
  timesUp_ = std::vector<UInt>(n, 0);
  timesDown_ = std::vector<UInt>(n, 0);
  fracCount_ = std::vector<UInt>(n,0); 
  unfixedCount_ = std::vector<UInt>(n,0);
  actualTimesDown_ = std::vector<UInt>(n, 0);
  actualTimesUp_ = std::vector<UInt>(n, 0);
  for (int i=0; i<n; ++i) {
    variableNames_.push_back(rel->getVariable(i)->getName());
   }
  // reserve space.
  relCands_.reserve(n);
  unrelCands_.reserve(n);
  x_.reserve(n);
 
}

void MaxInfBrancher::findCandidates_()
{
  VariableIterator v_iter, v_iter2, best_iter;
  VariableConstIterator cv_iter;
  int index;
  bool is_inf = false; // if true, then node can be pruned.

  BrVarCandSet cands;  // candidates from which to choose one.
  BrVarCandSet cands2; // Temporary set.
  BrCandVector gencands;
  BrCandVector gencands2; // Temporary vector.
  double s_wt = 1e-5;
  double i_wt = 1e-6;
  double score;

  assert(relCands_.empty());
  assert(unrelCands_.empty());

  for(HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel_, x_, mods_, cands2, gencands2, is_inf);
    for(BrVarCandIter it = cands2.begin(); it != cands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    for(BrCandVIter it = gencands2.begin(); it != gencands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    cands.insert(cands2.begin(), cands2.end());
    gencands.insert(gencands.end(), gencands2.begin(), gencands2.end());
    cands2.clear();
    gencands2.clear();
    if(is_inf || mods_.size() > 0) {
      for(BrVarCandIter it = cands.begin(); it != cands.end(); ++it) {
        delete *it;
      }
      for(BrCandVIter it = gencands.begin(); it != gencands.end(); ++it) {
        delete *it;
      }
      if(is_inf) {
        status_ = PrunedByBrancher;
      } else {
        status_ = ModifiedByBrancher;
      }
      return;
    }
  }

  // visit each candidate in and check if it has reliable pseudo costs.
  for(BrVarCandIter it = cands.begin(); it != cands.end(); ++it) {
    index = (*it)->getPCostIndex();
    if((minNodeDist_ > fabs(stats_->calls - lastStrBranched_[index])) ||
       (timesUp_[index] >= thresh_ && timesDown_[index] >= thresh_)) {
      relCands_.push_back(*it);
    } else {
      score = timesUp_[index] + timesDown_[index] -
          s_wt * (pseudoUp_[index] + pseudoDown_[index]) -
          i_wt * std::max((*it)->getDDist(), (*it)->getUDist());
      (*it)->setScore(score);
      unrelCands_.push_back(*it);
    }
  }
  // push all general candidates (that are not variables) as reliable
  // candidates
  for(BrCandVIter it = gencands.begin(); it != gencands.end(); ++it) {
    relCands_.push_back(*it);
  }

  // sort unreliable candidates in the increasing order of their reliability.
  std::sort(unrelCands_.begin(), unrelCands_.end(), CompareScore);

#if SPEW
  logger_->msgStream(LogDebug)
      << me_ << "number of reliable candidates = " << relCands_.size()
      << std::endl
      << me_ << "number of unreliable candidates = " << unrelCands_.size()
      << std::endl;
 // if(logger_->getMaxLevel() == LogDebug2) {
   // writeScores_(logger_->msgStream(LogDebug2));
 // }
#endif

  return;
}

BrCandPtr MaxInfBrancher::findBestCandidate_(const double objval,
                                                  double cutoff, NodePtr node)
{
  double best_score = -INFINITY;
  double score, change_up, change_down, maxchange;
  UInt cnt, maxcnt;
  EngineStatus status_up, status_down;
  BrCandPtr cand, best_cand = 0;

  // first evaluate candidates that have reliable pseudo costs
  for(BrCandVIter it = relCands_.begin(); it != relCands_.end(); ++it) {
    getPCScore_(*it, &change_down, &change_up, &score);
    if(score > best_score) {
      best_score = score;
      best_cand = *it;
      if(change_up > change_down) {
        best_cand->setDir(DownBranch);
      } else {
        best_cand->setDir(UpBranch);
      }
    }
  }

  maxchange = cutoff - objval;
  // now do strong branching on unreliable candidates
  if(unrelCands_.size() > 0) {
    BrCandVIter it;
    engine_->enableStrBrSetup();
    engine_->setIterationLimit(maxIterations_); // TODO: make limit dynamic.
    cnt = 0;
    maxcnt = (node->getDepth() > maxDepth_) ? 0 : maxStrongCands_;
    for(it = unrelCands_.begin(); it != unrelCands_.end() && cnt < maxcnt;
        ++it, ++cnt) {
      cand = *it;
      strongBranch_(cand, change_up, change_down, status_up, status_down);
      change_up = std::max(change_up - objval, 0.0);
      change_down = std::max(change_down - objval, 0.0);
      useStrongBranchInfo_(cand, maxchange, change_up, change_down, status_up,
                           status_down);
      score = getScore_(change_up, change_down);
      lastStrBranched_[cand->getPCostIndex()] = stats_->calls;
#if SPEW
    //  writeScore_(cand, score, change_up, change_down);
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
    }
    engine_->resetIterationLimit();
    engine_->disableStrBrSetup();
    if(NotModifiedByBrancher == status_) {
      // get score of remaining unreliable candidates as well.
      for(; it != unrelCands_.end(); ++it) {
        getPCScore_(*it, &change_down, &change_up, &score);
        if(score > best_score) {
          best_score = score;
          best_cand = *it;
          if(change_up > change_down) {
            best_cand->setDir(DownBranch);
          } else {
            best_cand->setDir(UpBranch);
          }
        }
      }
    }
  }
  return best_cand;
}

void MaxInfBrancher::updateAfterSolve(NodePtr node, ConstSolutionPtr sol)
{
  const double* x = sol->getPrimal();
  NodePtr parent = node->getParent();
  if(parent) {
    BrCandPtr cand = node->getBranch()->getBrCand();
    int index = cand->getPCostIndex();
    if(index > -1) {
      double oldval = node->getBranch()->getActivity();
      double newval = x[index];
      double cost =
          (node->getLb() - parent->getLb()) / (fabs(newval - oldval) + eTol_);
      if(cost < 0. || std::isinf(cost) || std::isnan(cost)) {
        cost = 0.;
      }
      if(newval < oldval) {
        updatePCost_(index, cost, pseudoDown_, timesDown_);
      } else {
        updatePCost_(index, cost, pseudoUp_, timesUp_);
      }
             if (cand->getDir() == UpBranch) {
            actualTimesUp_[cand->getPCostIndex()]++;
      } else if (cand->getDir() == DownBranch)  {
            actualTimesDown_[cand->getPCostIndex()]++;
      }
    }
  }
}


void MaxInfBrancher::writeStats(std::ostream &out) const
{
   if(stats_) {
    out << me_ << "times called                = " << stats_->calls << std::endl
        << me_ << "no. of problems in engine   = " << stats_->engProbs
        << std::endl
        << me_ << "times relaxation solved     = " << stats_->strBrCalls
        << std::endl
        << me_ << "times bounds changed        = " << stats_->bndChange
        << std::endl
        << me_ << "time in solving relaxations = " << stats_->time
        << std::endl;
  }
statsTable();
}


std::string MaxInfBrancher::getName() const
{
  return "MaxInfBrancher";
}

void MaxInfBrancher::freeCandidates_(BrCandPtr no_del)
{
  for(BrCandVIter it = unrelCands_.begin(); it != unrelCands_.end(); ++it) {
    if(no_del != *it) {
      delete *it;
    }
  }
  for(BrCandVIter it = relCands_.begin(); it != relCands_.end(); ++it) {
    if(no_del != *it) {
      delete *it;
    }
  }
  relCands_.clear();
  unrelCands_.clear();
}

bool MaxInfBrancher::getTrustCutoff()
{
  return trustCutoff_;
}

UInt MaxInfBrancher::getIterLim()
{
  return maxIterations_;
}

void MaxInfBrancher::getPCScore_(BrCandPtr cand, double* ch_down,
                                 double* ch_up, double* score)
{
  int index = cand->getPCostIndex();
  int n = rel_->getNumVars();

  // Calculate current average values over all variables in the problem instance
  double s_reli_sum_down = 0.0, s_reli_sum_up = 0.0;
  double s_infer_sum_down = 0.0, s_infer_sum_up = 0.0;
  for (int i = 0; i < n; ++i) {
    s_reli_sum_down += pseudoDown_[i];
    s_reli_sum_up += pseudoUp_[i];
    s_infer_sum_down += downScores_[i];
    s_infer_sum_up += upScores_[i];
  }
  double s_reli_avg_down = (n != 0) ? (s_reli_sum_down / n) : 1.0;
  double s_reli_avg_up = (n != 0) ? (s_reli_sum_up / n) : 1.0;
  double s_infer_avg_down = (n != 0) ? (s_infer_sum_down / n) : 1.0;
  double s_infer_avg_up = (n != 0) ? (s_infer_sum_up / n) : 1.0;

  // Weights
  const double omega_reli = 1.0;
  const double omega_infer = 1e-4;

  if (index > -1) {
    double reliability_down = cand->getDDist() * pseudoDown_[index];
    double reliability_up = cand->getUDist() * pseudoUp_[index];
    double inference_down = (n != 0) ? (downScores_[index] / n) : 0.0;
    double inference_up = (n != 0) ? (upScores_[index] / n) : 0.0;

  // Normalize scores with gfunction(g(.) = x/x+1)
    double g_reli_down = gFunction(s_reli_avg_down != 0.0 ? (reliability_down / s_reli_avg_down) : 0.0);
    double g_reli_up = gFunction(s_reli_avg_up != 0.0 ? (reliability_up / s_reli_avg_up) : 0.0);
    double g_infer_down = gFunction(s_infer_avg_down != 0.0 ? (inference_down / s_infer_avg_down) : 0.0);
    double g_infer_up = gFunction(s_infer_avg_up != 0.0 ? (inference_up / s_infer_avg_up) : 0.0);

    *ch_down = omega_reli * g_reli_down + omega_infer * g_infer_down;
    *ch_up = omega_reli * g_reli_up + omega_infer * g_infer_up;
  } else {
    double inference_down = (n != 0) ? (downScores_[index] / n) : 0.0;
    double inference_up = (n != 0) ? (upScores_[index] / n) : 0.0;

    double g_infer_down = gFunction(s_infer_avg_down != 0.0 ? (inference_down / s_infer_avg_down) : 0.0);
    double g_infer_up = gFunction(s_infer_avg_up != 0.0 ? (inference_up / s_infer_avg_up) : 0.0);

    *ch_down = omega_infer * g_infer_down;
    *ch_up = omega_infer * g_infer_up;
  }
  *score = getScore_(*ch_up, *ch_down);
}

double MaxInfBrancher::getScore_(const double& up_score,
                                 const double& down_score)
{
  if (up_score > down_score) {
    return down_score * 0.8 + up_score * 0.2;
  } else {
    return up_score * 0.8 + down_score * 0.2;
  }
  return 0.;
}

double MaxInfBrancher::gFunction(double x) const
{
  return x / (x + 1);
}
UInt MaxInfBrancher::getThresh() const
{
  return thresh_;
}


void MaxInfBrancher::setTrustCutoff(bool val)
{
  trustCutoff_ = val;
}

void MaxInfBrancher::setEngine(EnginePtr engine)
{
  engine_ = engine;
}

void MaxInfBrancher::setIterLim(UInt k)
{
  maxIterations_ = k;
}

void MaxInfBrancher::setMaxDepth(UInt k)
{
  maxDepth_ = k;
}

void MaxInfBrancher::setMinNodeDist(UInt k)
{
  minNodeDist_ = k;
}

void MaxInfBrancher::setThresh(UInt k)
{
  thresh_ = k;
}

bool MaxInfBrancher::shouldPrune_(const double& chcutoff,
                                       const double& change,
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
    if(trustCutoff_ && change > chcutoff - eTol_) {
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

void MaxInfBrancher::strongBranch_(BrCandPtr cand, double& obj_up,
                                        double& obj_down,
                                        EngineStatus& status_up,
                                        EngineStatus& status_down)
{
  HandlerPtr h = cand->getHandler();
  ModificationPtr mod;

  // first do down.
  mod = h->getBrMod(cand, x_, rel_, DownBranch);
  mod->applyToProblem(rel_);
  //std::cout << "down relax ******\n";
  //rel_->write(std::cout);

  timer_->start();
  status_down = engine_->solve();
  stats_->time += timer_->query();
  timer_->stop();
  ++(stats_->strBrCalls);
  obj_down = engine_->getSolutionValue();
  mod->undoToProblem(rel_);
  delete mod;

  // now go up.
  mod = h->getBrMod(cand, x_, rel_, UpBranch);
  mod->applyToProblem(rel_);
  //std::cout << "up relax ******\n";
  //rel_->write(std::cout);

  timer_->start();
  status_up = engine_->solve();
  stats_->time += timer_->query();
  timer_->stop();
  ++(stats_->strBrCalls);
  obj_up = engine_->getSolutionValue();
  mod->undoToProblem(rel_);
  delete mod;
}

void MaxInfBrancher::updatePCost_(const int& i, const double& new_cost,
                                       DoubleVector& cost, UIntVector& count)
{
  cost[i] = (cost[i] * count[i] + new_cost) / (count[i] + 1);
  count[i] += 1;
}

void MaxInfBrancher::useStrongBranchInfo_(BrCandPtr cand,
                                               const double& chcutoff,
                                               double& change_up,
                                               double& change_down,
                                               const EngineStatus& status_up,
                                               const EngineStatus& status_down)
{
  const UInt index = cand->getPCostIndex();
  bool should_prune_up = false;
  bool should_prune_down = false;
  bool is_rel = true;
  double cost;

  should_prune_down = shouldPrune_(chcutoff, change_down, status_down, &is_rel);
  should_prune_up = shouldPrune_(chcutoff, change_up, status_up, &is_rel);

  if(!is_rel) {
    change_up = 0.;
    change_down = 0.;
  } else if(should_prune_up == true && should_prune_down == true) {
    status_ = PrunedByBrancher;
    stats_->bndChange += 2;
  } else if(should_prune_up) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, DownBranch));
    ++(stats_->bndChange);
  } else if(should_prune_down) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, UpBranch));
    ++(stats_->bndChange);
  } else {
    cost = fabs(change_down) / (fabs(cand->getDDist()) + eTol_);
    updatePCost_(index, cost, pseudoDown_, timesDown_);

    cost = fabs(change_up) / (fabs(cand->getUDist()) + eTol_);
    updatePCost_(index, cost, pseudoUp_, timesUp_);
  }
}

void MaxInfBrancher::writeScore_(BrCandPtr cand, double score,
                                      double change_up, double change_down)
{
  logger_->msgStream(LogDebug2)
      << me_ << "candidate: " << cand->getName()
      << " down change = " << change_down << " up change = " << change_up
      << " score = " << score << std::endl;
}

void MaxInfBrancher::writeScores_(std::ostream& out)
{
  out << me_ << "unreliable candidates:" << std::endl;
  for(BrCandVIter it = unrelCands_.begin(); it != unrelCands_.end(); ++it) {
    if((*it)->getPCostIndex() > -1) {
      out << std::setprecision(6) << (*it)->getName() << "\t"
          << timesDown_[(*it)->getPCostIndex()] << "\t"
          << timesUp_[(*it)->getPCostIndex()] << "\t"
          << pseudoDown_[(*it)->getPCostIndex()] << "\t"
          << pseudoUp_[(*it)->getPCostIndex()] << "\t"
          << x_[(*it)->getPCostIndex()] << "\t"
          << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
          << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t"
          << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" << 0 << "\t" << 0
          << "\t" << (*it)->getScore() << "\t" << (*it)->getScore() << "\t"
          << (*it)->getDDist() << "\t" << 0.0 << "\t" << 1.0 << "\t"
          << std::endl;
    }
  }

  out << me_ << "reliable candidates:" << std::endl;
  for(BrCandVIter it = relCands_.begin(); it != relCands_.end(); ++it) {
    if((*it)->getPCostIndex() > -1) {
      out << (*it)->getName() << "\t" << timesDown_[(*it)->getPCostIndex()]
          << "\t" << timesUp_[(*it)->getPCostIndex()] << "\t"
          << pseudoDown_[(*it)->getPCostIndex()] << "\t"
          << pseudoUp_[(*it)->getPCostIndex()] << "\t"
          << x_[(*it)->getPCostIndex()] << "\t"
          << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
          << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t"
          << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" << 0 << "\t" << 0
          << "\t" << (*it)->getScore() << "\t" << (*it)->getScore() << "\t"
          << (*it)->getDDist() << "\t" << 0.0 << "\t" << 1.0 << "\t"
          << std::endl;
    }
  }
}

void MaxInfBrancher::infScore() {
 size_t numVars = rel_->getNumVars();
 upScores_.resize(numVars, 0.0);
 downScores_.resize(numVars, 0.0);
 std::fill(upScores_.begin(), upScores_.end(), 0.0);
 std::fill(downScores_.begin(), downScores_.end(), 0.0);
 for (ConstraintConstIterator citer = rel_->consBegin(); citer != rel_->consEnd(); ++citer) {
  ConstraintPtr c = *citer;
  FunctionPtr f = c->getFunction();
  LinearFunctionPtr lf = f->getLinearFunction();
  if(f->getType() != Linear) {
   continue;
  }
  ConsType ctype = c->getType();
  int nvars = lf->getNumTerms();
  int nposcoefone = 0, nnegcoefone = 0, nposbin = 0, nnegbin = 0;
  double wt1 = 0, wt2 = 0;
  const double tol = 1e-5;
  double sumnegwt = 0.0;

   for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
     double wt = it->second;
     VariablePtr v = it->first;
     int idx = v->getIndex();  // Get the index of the variable
     if (std::fabs(wt - 1) < tol) ++nposcoefone;
      else if (std::fabs(wt + 1) < tol) ++nnegcoefone;
     if (v->getType() == Binary) {
       if (wt > tol) {
        ++nposbin;
      } else if (wt < -tol) {
        ++nnegbin;
        sumnegwt += std::abs(wt);
      }
    }
      if (f->getNumVars() == 2) {
        if (wt > tol) {
            wt1 = wt;
       } else if (wt < tol) {
            wt2 = wt;
       }
     }
    switch (ctype) {
       case SetPartition:
        infSetPartition_(idx, nvars, wt);
        break;
       case SetPack:
         infSetPack_(idx, nvars, wt);
         break;
       case Cardinality:
          infCardinality_(idx, nvars, nposbin, nnegcoefone, wt, c);
          break;
       case EquationKnapsack:
          infEquationKnapsack_(idx, nvars, wt, c, sumnegwt);
          break;
       case BinPack:
          infBinPack_(idx, nvars, wt, c, sumnegwt);
          break;
       case Knapsack:
          infKnapsack_(idx, nvars, wt, c, nposbin, sumnegwt);
          break;
       case Aggregation:
          infAggregation_(idx, nvars, nposbin, wt1, wt2, c);
          break;
       case Precedence:
          infPrecedence_(idx, nvars, nnegbin, nposbin, c);
          break;
       case VariableBound:
          infVariableBound_(idx, nposbin, wt, wt1, wt2, c);
          break;
       case MixedBinary:
       case General:
          infMixed_(v, idx, wt, c);
          break;
          default:
    break;
            }
        }
    }
}

void MaxInfBrancher::infAggregation_(int idx, int nvars, int nposbin, double wt1, double wt2, ConstraintPtr c) {
  if (nposbin == nvars) {
    if (c->getUb() == wt1 && c->getUb() == wt2) {
      upScores_[idx] += 1; // up
      downScores_[idx] += 1; // down
    } else if ((wt1 || wt2) < c->getUb() && (wt1 || wt2) == c->getUb()) {
      upScores_[idx] += 1;
    }
  }
}

void MaxInfBrancher::infPrecedence_(int idx, int nvars, int nnegbin, int nposbin, ConstraintPtr c) {
  if (nposbin + nnegbin == nvars && c->getUb() == 0) {
    upScores_[idx] += 1; 
    downScores_[idx] += 1; 
  }
}

void MaxInfBrancher::infVariableBound_(int idx, int nposbin, double wt, double wt1, double wt2, ConstraintPtr c) {
  if (nposbin == 2) {
    if (wt == c->getUb()) {
      upScores_[idx] += 1;   
    } else if (wt < c->getUb() && wt1 + wt2 > c->getUb()) {
      upScores_[idx] += 1;
    }
  } else if (nposbin == 1) {
    upScores_[idx] += 1;
    downScores_[idx] += 1;
  }
}

void MaxInfBrancher::infSetPartition_(int idx, int nvars, double wt) {
  if (wt > 0) {
    upScores_[idx] += nvars - 1; 
  } else {
    downScores_[idx] += nvars - 1; 
  }
}

void MaxInfBrancher::infSetPack_(int idx, int nvars, double wt) {
  if (wt > 0) {
    upScores_[idx] += nvars - 1;
  } else {
    downScores_[idx] += nvars - 1; 
  }
}

void MaxInfBrancher::infCardinality_(int idx, int nvars, int nposbin, int nnegcoefone, double wt, ConstraintPtr c) {
  if (c->getUb() == nposbin && wt > 0) {
    upScores_[idx] += nvars - 1;
  } else if (c->getUb() == 2 + nnegcoefone && nposbin == c->getUb() + 1) {
    if (wt > 0) {
      downScores_[idx] += nvars - 1;
    } else {
      upScores_[idx] += nvars - 1;
    }
  }
}

void MaxInfBrancher::infEquationKnapsack_(int idx, int nvars, double wt, ConstraintPtr c, double sumnegwt) {
  double sumwt = 0;
  FunctionPtr f = c->getFunction();
  LinearFunctionPtr lf = f->getLinearFunction();
  for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
    double wtn = it->second;
    sumwt += wtn;
  }
  if (wt == c->getUb() && sumnegwt == 0) {
    upScores_[idx] += nvars - 1; 
  } else if (sumwt == c->getUb()) {
    upScores_[idx] += nvars - 1; 
  } else if (wt < c->getUb()) {
    for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
      double wtn = it->second;
      VariablePtr v2 = it->first;
      int idx2 = v2->getIndex();
      if (c->getUb() - wt == wtn && idx2 != idx) {
        upScores_[idx] += 1;
      }
    }
  }
}

void MaxInfBrancher::infBinPack_(int idx, int nvars, double wt, ConstraintPtr c, double sumnegwt) {
  if (std::abs(wt) == c->getUb() + sumnegwt) {
    downScores_[idx] += nvars - 1; 
  } else if (wt < c->getUb() + sumnegwt) {
    FunctionPtr f = c->getFunction();
    LinearFunctionPtr lf = f->getLinearFunction();
    for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
      double wtn = it->second;
      VariablePtr v2 = it->first;
      int idx2 = v2->getIndex();
      if (std::abs(wt) + std::abs(wtn) > c->getUb() + sumnegwt && idx2 != idx) {
        upScores_[idx] += 1;
      }
    }
  }
}

void MaxInfBrancher::infKnapsack_(int idx, int nvars, double wt, ConstraintPtr c, int nposbin, double sumnegwt) {
  if (wt == c->getUb() && nposbin == nvars) {
    upScores_[idx] += nvars - 1; 
  } else if (wt < c->getUb()) {
    FunctionPtr f = c->getFunction();
    LinearFunctionPtr lf = f->getLinearFunction();
    for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
      double wtn = it->second;
      VariablePtr v2 = it->first;
      int idx2 = v2->getIndex();
      if (c->getUb() - wt < wtn && idx2 != idx) {
        upScores_[idx] += 1;
      }
    }
  } else if (wt - sumnegwt == c->getUb()) {
    upScores_[idx] += nvars - 1; 
  }
}

void MaxInfBrancher::infMixed_(VariablePtr v,int idx, double wt, ConstraintPtr c) {
	
  double conslb = 0;
  double consub = 0;
  FunctionPtr f = c->getFunction();
  LinearFunctionPtr lf = f->getLinearFunction();
  for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
    double wtn = it->second;
    if (wtn > 0) {
      consub += wtn; // Sum positive coefficients (constraint upper bound)
    } else {
      conslb += wtn; // Sum negative coefficients (constraint lower bound)

    }
  }
  conslb = std::abs(conslb);
  consub = std::abs(consub);
  if (v->getType() == Binary) {
    if (conslb != 0) {
      upScores_[idx] += std::abs((conslb - wt) / conslb);
    } else if (c->getUb() != 0) {
      upScores_[idx] += std::abs(wt / c->getUb());
    }
    if (consub != 0 && c->getUb()!= 0) {
      downScores_[idx] += std::abs((std::min(consub - wt, c->getUb()) - std::min(consub, c->getUb())) / std::min(consub, c->getUb()));
    }
  }
}

void MaxInfBrancher::updateFracCount_() {
  for (auto it = relCands_.begin(); it != relCands_.end(); ++it) {
    int index = (*it)->getPCostIndex();
    fracCount_[index] += 1; // Increment the fractional count for the variable index.
  }
  for (auto it = unrelCands_.begin(); it != unrelCands_.end(); ++it) {
    int index = (*it)->getPCostIndex();
    fracCount_[index] += 1; // Increment the fractional count for the variable index.
  }
}

void MaxInfBrancher::updateUnfixedCount_()
{
  VariablePtr v;
  UInt index;
  for (VariableConstIterator it = rel_->varsBegin(); it != rel_->varsEnd();
       ++it) {
    v = (*it);
    index = v->getIndex();
    if (fabs(v->getLb() - v->getUb()) > eTol_) {
      unfixedCount_[index] += 1;
    }
  }
}

void MaxInfBrancher::statsTable()const {
    // Print header
  static bool header = false;
  if (!header) {
   logger_->msgStream(LogExtraInfo) << std::endl;	  
   logger_->msgStream(LogExtraInfo) << "----------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
   logger_->msgStream(LogExtraInfo) << std::left << std::setw(15) << "Variable"
      << std::setw(15) << "Pseudo Up"
      << std::setw(15) << "Pseudo Down"
      << std::setw(15) << "Times Up"
      << std::setw(15) << "Times Down"
      << std::setw(15) << "Fractional"
      << std::setw(15) << "Not Fixed"
      << std::setw(20) << "Actual Times Up"
      << std::setw(20) << "Actual Times Down"
      << std::setw(20) << "Inf Up"
      << std::setw(20) << "Inf Down"
      << std::endl;
   logger_->msgStream(LogExtraInfo) << "----------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
  header = true;
   }
  for (std::vector<double>::size_type i = 0; i < pseudoUp_.size(); ++i) {
   logger_->msgStream(LogExtraInfo)
      << std::left << std::setw(18) << variableNames_[i]
      << std::setw(15) << pseudoUp_[i]
      << std::setw(15) << pseudoDown_[i]
      << std::setw(15) << timesUp_[i]
      << std::setw(15) << timesDown_[i]
      << std::setw(15) << fracCount_[i] // Print fractional count
      << std::setw(15) << unfixedCount_[i] // Print unfixed count
      << std::setw(20) << actualTimesUp_[i]
      << std::setw(20) << actualTimesDown_[i]
      << std::setw(20) << upScores_[i]
      << std::setw(20) << downScores_[i]
      << std::endl;
   }   
  logger_->msgStream(LogExtraInfo) << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl; 
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
