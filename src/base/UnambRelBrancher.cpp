//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file UnambRelBrancher.cpp
 * \brief Define methods for unambiguous reliability branching.
 * \author Prashant Palkar, IIT Bombay
 */

#include <cmath>
#include <iomanip>
#include <algorithm>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrCand.h"
#include "BrVarCand.h"
#include "Engine.h"
#include "Environment.h"
#include "Handler.h"
#include "Logger.h"
#include "Modification.h"
#include "Node.h"
#include "Option.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "UnambRelBrancher.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"
#include <omp.h>

//#define SPEW 1

using namespace Minotaur;

const std::string UnambRelBrancher::me_ = "unambiguous reliability brancher: ";

UnambRelBrancher::UnambRelBrancher(EnvPtr env, HandlerVector & handlers) 
: engine_(EnginePtr()),             // NULL
  eTol_(1e-6),
  handlers_(handlers),              // Create a copy, the vector is not too big
  init_(false),
  maxDepth_(1000),
  maxIterations_(25),
  maxStrongCands_(20),
  minNodeDist_(50),
  rel_(RelaxationPtr()),            // NULL
  status_(NotModifiedByBrancher),
  thresh_(4),
  trustCutoff_(true),
  x_(0)
{
  timer_ = env->getNewTimer();
  logger_ = env->getLogger();
  stats_ = new UnambRelBrStats();
  stats_->calls = 0;
  stats_->engProbs = 0;
  stats_->strBrCalls = 0;
  stats_->bndChange = 0;
  stats_->iters = 0;
  stats_->strTime = 0.0;
}


UnambRelBrancher::~UnambRelBrancher()
{
  delete stats_;
  delete timer_;
}


BrCandPtr UnambRelBrancher::findBestCandidate_(const double objval, 
                                                  double cutoff, NodePtr node,
                                                  IntVector candsPos, 
                                                  IntVector candsPosRel,
                                                  IntVector candsPosUnrel)
{
  double best_score = -INFINITY;
  double score, change_up, change_down, maxchange;
  UInt cnt, maxcnt;
  EngineStatus status_up, status_down;
  BrCandPtr cand, best_cand = 0;

  // first evaluate candidates that have reliable pseudo costs
  cnt=0;
  for (BrCandVIter it=relCands_.begin(); it!=relCands_.end(); ++it) {
    getPCScore_(*it, &change_down, &change_up, &score, node, candsPosRel, cnt);
    //std::cout << (*it)->getName() << " " << change_up << " " 
      //<< change_down << " " << score << "\n";
    if (score > best_score) {
      best_score = score;
      best_cand = *it;
      if (change_up > change_down) {
        best_cand->setDir(DownBranch);
      } else {
        best_cand->setDir(UpBranch);
      }
    }
    cnt++;
  }

  maxchange = cutoff-objval;
  // now do strong branching on unreliable candidates
  UIntVector lastStrBranched = node->getLastStrongBranched();

  if (unrelCands_.size()>0) {
    BrCandVIter it;
    engine_->enableStrBrSetup();
    engine_->setIterationLimit(maxIterations_); // TODO: make limit dynamic.
    cnt = 0;
    maxcnt = (node->getDepth()>maxDepth_) ? 0 : maxStrongCands_;
    for (it=unrelCands_.begin(); it!=unrelCands_.end() && 
        cnt < maxcnt; ++it, ++cnt) {
      cand = *it;
      //std::cout << "str brnching on " << (*it)->getName() << "\n";
      strongBranch_(cand, change_up, change_down, status_up, status_down);
      change_up    = std::max(change_up - objval, 0.0);
      change_down  = std::max(change_down - objval, 0.0);
      useStrongBranchInfo_(cand, maxchange, change_up, change_down, 
          status_up, status_down, node, candsPos[cnt]);
      score = getScore_(change_up, change_down);
      //lastStrBranched_[cand->getPCostIndex()] = stats_->calls;
#if SPEW
      writeScore_(cand, score, change_up, change_down);
#endif
      if (status_!=NotModifiedByBrancher) {
        break;
      }
      if (score > best_score) {
        best_score = score;
        best_cand = cand;
        if (change_up > change_down) {
          best_cand->setDir(DownBranch);
        } else {
          best_cand->setDir(UpBranch);
        }
      }
    }
    engine_->resetIterationLimit(); 
    engine_->disableStrBrSetup();
    if (NotModifiedByBrancher == status_) {
      // get score of remaining unreliable candidates as well.
      for (;it!=unrelCands_.end(); ++it) {
        getPCScore_(*it, &change_down, &change_up, &score, node, candsPosUnrel, cnt);
        if (score > best_score) {
          best_score = score;
          best_cand = *it;
          if (change_up > change_down) {
            best_cand->setDir(DownBranch);
          } else {
            best_cand->setDir(UpBranch);
          }
        }
      }
    }
  }
  //if (best_cand) {
//#pragma omp critical
    //std::cout << "in rel: node " << node->getId() << " lb " << node->getLb()
      //<< " brCand " << best_cand->getName() << " score " << best_score
      //<< " thread  " << omp_get_thread_num() << "\n";
  //}
  return best_cand;
}


Branches UnambRelBrancher::findBranches(RelaxationPtr rel, NodePtr node, 
                                           ConstSolutionPtr sol,
                                           SolutionPoolPtr s_pool,
                                           BrancherStatus & br_status,
                                           ModVector &mods) 
{
  Branches branches = 0;
  BrCandPtr br_can = 0;
  const double *x = sol->getPrimal();
  
  //position of the branching candidate in brCands (-1 otherwise)
  IntVector candsPos; 

  ++(stats_->calls);
  if (!init_) {
    init_ = true;
    initialize(rel);
  }
  rel_ = rel;
  br_status = NotModifiedByBrancher;
  status_ = NotModifiedByBrancher;
  mods_.clear();

  // make a copy of x, because it is overwritten while strong branching.
  x_.resize(rel->getNumVars());
  std::copy(x, x+rel->getNumVars(), x_.begin());

  IntVector candsPosRel, candsPosUnrel;
  findCandidates_(node, candsPos, candsPosRel, candsPosUnrel);
  if (status_ == PrunedByBrancher) {
    br_status = status_;
    return 0;
  }

  if (status_ == NotModifiedByBrancher) {
    br_can = findBestCandidate_(sol->getObjValue(), 
                                s_pool->getBestSolutionValue(), node,
                                candsPos, candsPosRel, candsPosUnrel);
  }

  // status_ might have changed now. Check again.
  if (status_ == NotModifiedByBrancher) {
    // surrounded by br_can :-)
    branches = br_can->getHandler()->getBranches(br_can, x_, rel_, s_pool); 
    for (BranchConstIterator br_iter=branches->begin(); 
        br_iter!=branches->end(); ++br_iter) {
      (*br_iter)->setBrCand(br_can);
    }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "best candidate = "
      << br_can->getName() << std::endl;
#endif
  } else {
    // we found some modifications that can be done to the node. Send these
    // back to the processor.
    if (mods_.size()>0) {
      mods.insert(mods.end(),mods_.begin(),mods_.end());
    }
    br_status = status_;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "found modifications"
                                 << std::endl;
    if (mods_.size()>0) {
      for (ModificationConstIterator miter=mods_.begin(); miter!=mods_.end();
           ++miter) {
        (*miter)->write(logger_->msgStream(LogDebug));
      }
    } else if (status_==PrunedByBrancher) {
      logger_->msgStream(LogDebug) << me_ << "Pruned." << std::endl;
    } else {
      logger_->msgStream(LogDebug) << me_ << "unexpected status = "
                                   << status_ << std::endl;
    }
#endif
  }
  //writeScores_(std::cout);
  freeCandidates_(br_can);
  if (status_ != NotModifiedByBrancher && br_can) {
    delete br_can;
  }
  return branches;
}


void UnambRelBrancher::findCandidates_(NodePtr node, IntVector & candsPos, 
                                       IntVector & candsPosRel,
                                       IntVector & candsPosUnrel)
{
  VariableIterator v_iter, v_iter2, best_iter;
  VariableConstIterator cv_iter;
  int index = -1, vindex = -1;
  bool is_inf = false;   // if true, then node can be pruned.

  BrVarCandSet cands;       // candidates from which to choose one.
  BrVarCandSet cands2;      // Temporary set.
  BrCandVector gencands;
  BrCandVector gencands2;   // Temporary vector.
  double s_wt = 1e-5;
  double i_wt = 1e-6;
  double score;

  //clear all psedocosts info for filling in fresh ones (not needed?)
  assert(relCands_.empty());
  assert(unrelCands_.empty());

  for (HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel_, x_, mods_, cands2, gencands2, is_inf);
    for (BrVarCandIter it = cands2.begin(); it != cands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    for (BrCandVIter it = gencands2.begin(); it != gencands2.end(); ++it) {
      (*it)->setHandler(*h);
    }
    cands.insert(cands2.begin(), cands2.end());
    gencands.insert(gencands.end(), gencands2.begin(), gencands2.end());
    if (is_inf) {
      relCands_.clear();
      unrelCands_.clear();
      cands2.clear();
      cands.clear();
      gencands2.clear();
      gencands.clear();
      status_ = PrunedByBrancher;
      return;
    } else if (mods_.size()>0) {
      status_ = ModifiedByBrancher;
      relCands_.clear();
      unrelCands_.clear();
      cands2.clear();
      cands.clear();
      gencands2.clear();
      gencands.clear();
      return;
    }
    cands2.clear();
    gencands2.clear();
  }

  // visit each candidate in and check if it has reliable pseudo costs.
  candsPos.resize(cands.size(),-1);

  //check if the a candidate's information is present with the parent node
  //NodePtr parent = node->getParent();
  //if (parent) {
    //node->setBrCands(parent->getBrCands());
    //node->setPCDown(parent->getPCDown());
    //node->setPCUp(parent->getPCUp());
    //node->setTimesDown(parent->getTimesDown());
    //node->setTimesUp(parent->getTimesUp());
    //node->setLastStrongBranched(parent->getLastStrongBranched());
  //}
  UIntVector brCands = node->getBrCands();
  DoubleVector pCDown = node->getPCDown();
  DoubleVector pCUp = node->getPCUp();
  UIntVector timesDown = node->getTimesDown();
  UIntVector timesUp = node->getTimesUp();
  UIntVector lastStrBranched = node->getLastStrongBranched();
  
  std::vector<UInt>::iterator vit;
  UInt candi=0;
  for (BrVarCandIter it=cands.begin(); it!=cands.end(); ++it) {
    index = (*it)->getPCostIndex();
    vit = std::find(brCands.begin(), brCands.end(), index);
    if (vit != brCands.end()) {
      vindex = std::distance(brCands.begin(), vit);
      candsPos[candi] = vindex;
      if ((minNodeDist_ > fabs(node->getDepth()-lastStrBranched[vindex])) ||
          (timesUp[vindex] >= thresh_ && timesDown[vindex] >= thresh_)) {
        relCands_.push_back(*it);
        candsPosRel.push_back(vindex);
      } else {
        //score = 0;
        score = timesUp[vindex] + timesDown[vindex]
          -s_wt*(pCUp[vindex]+pCDown[vindex])
          -i_wt*std::max((*it)->getDDist(), (*it)->getUDist());
        (*it)->setScore(score);
        unrelCands_.push_back(*it);
        //candsPosUnrel.push_back(vindex);
      }
    } else {
      //score = timesUp[index] + timesDown[index]
        //-s_wt*(pseudoUp[index]+pseudoDown_[index])
      score = -i_wt*std::max((*it)->getDDist(), (*it)->getUDist()); //candidate not branched on before
      (*it)->setScore(score);
      unrelCands_.push_back(*it);
      //candsPosUnrel.push_back(-1);
    }
    candi++;
  }

  // push all general candidates (that are not variables) as reliable
  // candidates
  for (BrCandVIter it=gencands.begin(); it!=gencands.end(); ++it) {
    relCands_.push_back(*it);
  }

  // sort unreliable candidates in the increasing order of their reliability.
  std::sort(unrelCands_.begin(), unrelCands_.end(), CompareScore);

  //form the sorted candsPosUnrel array
  candi=0;
  for (BrCandVIter it=unrelCands_.begin(); it!=unrelCands_.end(); ++it) {
    index = (*it)->getPCostIndex();
    vit = std::find(brCands.begin(), brCands.end(), index);
    if (vit != brCands.end()) {
      vindex = std::distance(brCands.begin(), vit);
      candsPosUnrel.push_back(vindex);
    } else {
      candsPosUnrel.push_back(-1);
    }
    candi++;
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_
                               << "number of reliable candidates = " 
                               << relCands_.size() << std::endl 
                               << me_
                               << "number of unreliable candidates = " 
                               << unrelCands_.size() << std::endl;
  if (logger_->getMaxLevel() == LogDebug2) {
    writeScores_(logger_->msgStream(LogDebug2));
  }
#endif

  //Populate containers of reliability brancher with data from node (not needed?)
  
  //writeScores_(std::cout, node, candsPosRel, candsPosUnrel);
  return;
}


void UnambRelBrancher::freeCandidates_(BrCandPtr no_del)
{
  for (BrCandVIter it=unrelCands_.begin(); it!=unrelCands_.end(); ++it) {
    if (no_del != *it) {
      delete *it;
    }
  }
  for (BrCandVIter it=relCands_.begin(); it!=relCands_.end(); ++it) {
    if (no_del != *it) {
      delete *it;
    }
  }
  relCands_.clear();
  unrelCands_.clear();
}


bool UnambRelBrancher::getTrustCutoff()
{
  return trustCutoff_;
}


UInt UnambRelBrancher::getIterLim() 
{
  return maxIterations_;
}


std::string UnambRelBrancher::getName() const
{
  return "Unambiguous Reliablity Brancher";
}


void UnambRelBrancher::getPCScore_(BrCandPtr cand, double *ch_down, 
                                      double *ch_up, double *score, 
                                      NodePtr node, IntVector candsPos,
                                      UInt cnt) 
{
  UIntVector brCands = node->getBrCands();
  DoubleVector pCDown = node->getPCDown();
  DoubleVector pCUp = node->getPCUp();
  //int index = cand->getPCostIndex();
  if (candsPos[cnt]>-1) {
    *ch_down   = cand->getDDist()*pCDown[candsPos[cnt]];
    *ch_up     = cand->getUDist()*pCUp[candsPos[cnt]];
    *score     = getScore_(*ch_up, *ch_down);
  } else {
    *ch_down   = 0.0;
    *ch_up     = 0.0;
    *score     = cand->getScore();
  }
}


double UnambRelBrancher::getScore_(const double & up_score, 
                                      const double & down_score)
{
  if (up_score>down_score) {
    return down_score*0.8 + up_score*0.2;
  } else {
    return up_score*0.8 + down_score*0.2;
  }
  return 0.;
}


UInt UnambRelBrancher::getThresh() const
{
  return thresh_;
}


void UnambRelBrancher::initialize(RelaxationPtr rel)
{
  int n = rel->getNumVars();
  // initialize to zero.
  pseudoUp_ = DoubleVector(n,0.); 
  pseudoDown_ = DoubleVector(n,0.); 
  lastStrBranched_ = UIntVector(n,20000);
  timesUp_ = std::vector<UInt>(n,0); 
  timesDown_ = std::vector<UInt>(n,0); 

  // reserve space.
  relCands_.reserve(n);
  unrelCands_.reserve(n);
  x_.reserve(n);
}


void UnambRelBrancher::setTrustCutoff(bool val)
{
  trustCutoff_ = val;
}


void UnambRelBrancher::setEngine(EnginePtr engine)
{
  engine_ = engine;
}


void UnambRelBrancher::setIterLim(UInt k) 
{
  maxIterations_ = k;
}


void UnambRelBrancher::setMaxDepth(UInt k) 
{
  maxDepth_ = k;
}


void UnambRelBrancher::setMinNodeDist(UInt k) 
{
  minNodeDist_ = k;
}


void UnambRelBrancher::setThresh(UInt k) 
{
  thresh_ = k;
}


bool UnambRelBrancher::shouldPrune_(const double &chcutoff, 
                                       const double &change,
                                       const EngineStatus & status,
                                       bool *is_rel)
{
  switch (status) {
   case (ProvenLocalInfeasible):
     return true;
   case (ProvenInfeasible):
     return true;
   case (ProvenObjectiveCutOff):
     return true;
   case (ProvenLocalOptimal):
   case (ProvenOptimal):
     if (trustCutoff_ && change>chcutoff-eTol_) {
       return true;
     }
     // check feasiblity
     break;
   case (EngineUnknownStatus):
     assert (!"engine status is UnknownStatus in reliability branching!");
     break;
   case (EngineIterationLimit):
     break;
   case (ProvenFailedCQFeas):
   case (ProvenFailedCQInfeas):
     logger_->msgStream(LogInfo) << me_ << "Failed CQ." <<
     " Continuing." << std::endl;
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


void UnambRelBrancher::strongBranch_(BrCandPtr cand, double & obj_up, 
                                        double & obj_down, 
                                        EngineStatus & status_up, 
                                        EngineStatus & status_down)
{
  HandlerPtr h = cand->getHandler();
  ModificationPtr mod;

  // first do down.
  mod = h->getBrMod(cand, x_, rel_, DownBranch);
  mod->applyToProblem(rel_);
  //std::cout << "down relax ******\n";
  //mod->write(std::cout);
  //rel_->write(std::cout);

  timer_->start();
  status_down = engine_->solve();
  stats_->strTime += timer_->query();
  timer_->stop();
  ++(stats_->strBrCalls);
  obj_down = engine_->getSolutionValue();
  mod->undoToProblem(rel_);
  delete mod;

  // now go up.
  mod = h->getBrMod(cand, x_, rel_, UpBranch);
  mod->applyToProblem(rel_);
  //std::cout << "up relax ******\n";
  //mod->write(std::cout);
  //rel_->write(std::cout);

  timer_->start();
  status_up = engine_->solve();
  stats_->strTime += timer_->query();
  timer_->stop();
  ++(stats_->strBrCalls);
  obj_up = engine_->getSolutionValue();
  mod->undoToProblem(rel_);
  delete mod;
}


void UnambRelBrancher::updateAfterSolve(NodePtr node, ConstSolutionPtr sol)
{
  const double *x = sol->getPrimal();
  NodePtr parent = node->getParent();
  if (parent) {
    BrCandPtr cand = node->getBranch()->getBrCand();
    int index = cand->getPCostIndex();
    if (index>-1) {
      UInt vindex;
      node->setBrCands(parent->getBrCands());
      node->setPCDown(parent->getPCDown());
      node->setPCUp(parent->getPCUp());
      node->setTimesDown(parent->getTimesDown());
      node->setTimesUp(parent->getTimesUp());
      node->setLastStrongBranched(parent->getLastStrongBranched());
      
      UIntVector brCands = node->getBrCands();
      DoubleVector pCDown = node->getPCDown();
      DoubleVector pCUp = node->getPCUp();
      UIntVector timesDown = node->getTimesDown();
      UIntVector timesUp = node->getTimesUp();
      std::vector<UInt>::iterator vit;
      bool newCand = false;

      double oldval = node->getBranch()->getActivity();
      double newval = x[index];
      double cost = (node->getLb()-parent->getLb()) / 
                    (fabs(newval - oldval)+eTol_);
      if (cost < 0. || std::isinf(cost) || std::isnan(cost)) {
        cost = 0.;
      }
      
      vit = std::find(brCands.begin(), brCands.end(), index);
      if (vit != brCands.end()) {
        vindex = std::distance(brCands.begin(), vit);
      } else { //this will never happen, right?
        vindex = -1;
        newCand = true;
      }
      if (newval < oldval) {
        updatePCost_(index, vindex, cost, pCDown, timesDown, newCand, true, false, node);
      } else {
        updatePCost_(index, vindex, cost, pCUp, timesUp, newCand, false, false, node);
      }
    } 
  }
}


void UnambRelBrancher::updatePCost_(const int & index, int i, const double & new_cost,
                                    DoubleVector cost, UIntVector count,
                                    bool newCand, bool updateDown, bool strngBrnched, 
                                    NodePtr node)
{
  if (!newCand) {
    if (updateDown) {
      node->updatePCDown(i, (cost[i]*count[i] + new_cost)/(count[i]+1));
      node->updateTimesDown(i,count[i]+1);
    } else {
      node->updatePCUp(i, (cost[i]*count[i] + new_cost)/(count[i]+1));
      node->updateTimesUp(i,count[i]+1);
    }
    if (strngBrnched) {
      node->updateLastStrBranched(i, stats_->calls);
    }
  } else {
    UIntVector brCands = node->getBrCands();
    UInt n = brCands.size()+1;
   
    node->updateBrCands(index);
    if (updateDown) {
      node->updatePCDown(n, new_cost);
      node->updateTimesDown(n, 1);
      node->updatePCUp(n, 0);
      node->updateTimesUp(n, 0);
    } else {
      node->updatePCUp(n, new_cost);
      node->updateTimesUp(n, 1);
      node->updatePCDown(n, 0);
      node->updateTimesDown(n, 0);
    }
    if (strngBrnched) {
      node->updateLastStrBranched(n, stats_->calls);
    } else {
      node->updateLastStrBranched(n, 0);
    }
  }
}


void UnambRelBrancher::useStrongBranchInfo_(BrCandPtr cand, 
                                               const double &chcutoff, 
                                               double &change_up, 
                                               double &change_down,
                                               const EngineStatus & status_up,
                                               const EngineStatus & status_down,
                                               NodePtr node, int candPosition)
{
  const UInt index        = cand->getPCostIndex();
  bool should_prune_up    = false;
  bool should_prune_down  = false;
  bool is_rel = true;
  double cost;

  should_prune_down = shouldPrune_(chcutoff, change_down, status_down, &is_rel);
  should_prune_up   = shouldPrune_(chcutoff, change_up, status_up, &is_rel);

  if (!is_rel) {
    change_up = 0.;
    change_down = 0.;
  } else if (should_prune_up == true && should_prune_down == true) {
    status_ = PrunedByBrancher;
    stats_->bndChange += 2;
  } else if (should_prune_up) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, DownBranch));
    ++(stats_->bndChange);
  } else if (should_prune_down) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, UpBranch));
    ++(stats_->bndChange);
  } else { 
    if (candPosition > -1) {
      cost = fabs(change_down)/(fabs(cand->getDDist())+eTol_);
      //updatePCost_(index, cost, pseudoDown_, timesDown_, false, true, true, node);
      updatePCost_(index, candPosition, cost, node->getPCDown(), node->getTimesDown(), false, true, true, node);
      cost = fabs(change_up)/(fabs(cand->getUDist())+eTol_);
      updatePCost_(index, candPosition, cost, node->getPCUp(), node->getTimesUp(), false, false, true, node);
    } else {
      cost = fabs(change_down)/(fabs(cand->getDDist())+eTol_);
      updatePCost_(index, candPosition, cost, node->getPCDown(), node->getTimesDown(), true, true, true, node);
      cost = fabs(change_up)/(fabs(cand->getUDist())+eTol_);
      updatePCost_(index, node->getBrCands().size() - 1, cost, node->getPCUp(), node->getTimesUp(), false, false, true, node);
    }

  }
}


void UnambRelBrancher::writeScore_(BrCandPtr cand, double score, 
                                      double change_up, double change_down)
{
  logger_->msgStream(LogDebug2) << me_ << "candidate: " << cand->getName() 
                                << " down change = " << change_down
                                << " up change = " << change_up
                                << " score = " << score
                                << std::endl;
}


void UnambRelBrancher::writeScores_(std::ostream &out, NodePtr node,
                                    IntVector candsPosRel, 
                                    IntVector candsPosUnrel)
{
  int cnt=0;
  DoubleVector pCDown = node->getPCDown();
  DoubleVector pCUp = node->getPCUp();
  UIntVector timesDown = node->getTimesDown();
  UIntVector timesUp = node->getTimesUp();

  out << me_ << "unreliable candidates:" << std::endl;
  for (BrCandVIter it=unrelCands_.begin(); it!=unrelCands_.end(); ++it) {
    if ((*it)->getPCostIndex()>-1) {
      out << std::setprecision(6) << (*it)->getName() << "\t";
      if (candsPosUnrel[cnt] > -1) {
        out << timesDown[candsPosUnrel[cnt]] << "\t"
        << timesUp[candsPosUnrel[cnt]] << "\t" 
        << node->getPCDown()[candsPosUnrel[cnt]] << "\t"
        << node->getPCUp()[candsPosUnrel[cnt]] << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t";
      } else {
        out << -1 << "\t"
        << -1 << "\t" 
        << -1 << "\t"
        << -1 << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t";
      }
      out << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" 
                                  << 0 << "\t" << 0 << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getDDist() << "\t"
                                  << 0.0 << "\t"
                                  << 1.0 << "\t" << std::endl;
    }
    cnt++;
  }

  out << me_ << "reliable candidates:" << std::endl;
  cnt = 0;
  for (BrCandVIter it=relCands_.begin(); it!=relCands_.end(); ++it) {
    if ((*it)->getPCostIndex()>-1) {
      out << std::setprecision(6) << (*it)->getName() << "\t";
      if (candsPosRel[cnt] > -1) {
        out << timesDown[candsPosRel[cnt]] << "\t"
        << timesUp[candsPosRel[cnt]] << "\t" 
        << node->getPCDown()[candsPosRel[cnt]] << "\t"
        << node->getPCUp()[candsPosRel[cnt]] << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t";
      } else {
        out << -1 << "\t"
        << -1 << "\t" 
        << -1 << "\t"
        << -1 << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t";
      }
      out << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" 
                                  << 0 << "\t" << 0 << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getDDist() << "\t"
                                  << 0.0 << "\t"
                                  << 1.0 << "\t" << std::endl;
    }
    cnt++;
  }


}


void UnambRelBrancher::writeScores_(std::ostream &out)
{
  out << me_ << "unreliable candidates:" << std::endl;
  for (BrCandVIter it=unrelCands_.begin(); it!=unrelCands_.end(); ++it) {
    if ((*it)->getPCostIndex()>-1) {
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
      out << std::setprecision(6) << (*it)->getName() << "\t" 
                                  << 0 << "\t" << 0 << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getDDist() << "\t"
                                  << 0.0 << "\t"
                                  << 1.0 << "\t" << std::endl;
    }
  }

  out << me_ << "reliable candidates:" << std::endl;
  for (BrCandVIter it=relCands_.begin(); it!=relCands_.end(); ++it) {
    if ((*it)->getPCostIndex()>-1) {
      out << (*it)->getName() << "\t" 
        << timesDown_[(*it)->getPCostIndex()] << "\t"
        << timesUp_[(*it)->getPCostIndex()] << "\t" 
        << pseudoDown_[(*it)->getPCostIndex()] << "\t"
        << pseudoUp_[(*it)->getPCostIndex()] << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t"
        << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" 
          << 0 << "\t" << 0 << "\t"
          << (*it)->getScore() << "\t"
          << (*it)->getScore() << "\t"
          << (*it)->getDDist() << "\t"
          << 0.0 << "\t"
          << 1.0 << "\t" << std::endl;
    }
  }
}


void UnambRelBrancher::writeStats(std::ostream &out) const
{
  if (stats_) {
    out << me_ << "times called                = " << stats_->calls 
      << std::endl
      << me_ << "no. of problems in engine   = " << stats_->engProbs
      << std::endl
      << me_ << "times relaxation solved     = " << stats_->strBrCalls
      << std::endl
      << me_ << "times bounds changed        = " << stats_->bndChange
      << std::endl
      << me_ << "time in solving relaxations = " << stats_->strTime
      << std::endl;
  }
}


