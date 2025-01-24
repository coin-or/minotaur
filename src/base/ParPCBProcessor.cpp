// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file ParPCBProcessor.cpp
 * \brief Define the derived class of NodeProcessor that solves LP
 * or NLP relaxations for parallel algorithms.
 * \author Prashant Palkar and Meenarli Sharma, IIT Bombay
 */
#include <cmath> // for INFINITY
#include <iomanip>
#include <omp.h>
#include "MinotaurConfig.h"
#include "Brancher.h"
#include "Constraint.h"
#include "CutManager.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "Handler.h"
#include "Heuristic.h"
#include "ParPCBProcessor.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Node.h"
#include "Option.h"
#include "ParCutMan.h"
#include "Modification.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "ParTreeManager.h"
#include "WarmStart.h"

using namespace Minotaur;

//#define SPEW 1

const std::string ParPCBProcessor::me_ = "ParPCBProcessor: ";

//ParPCBProcessor::ParPCBProcessor()
  //: contOnErr_(false),
    //cutMan_(0),
    //cutOff_(INFINITY),
    //engine_(EnginePtr()),
    //engineStatus_(EngineUnknownStatus),
    //numSolutions_(0),
    //relaxation_(RelaxationPtr()),
    //ws_(WarmStartPtr()),
    //oATol_(1e-5),
    //oRTol_(1e-5)
//{
  //handlers_.clear();
  //logger_ = (LoggerPtr) new Logger(LogInfo);
  //stats_.bra = 0;
  //stats_.inf = 0;
  //stats_.opt = 0;
  //stats_.prob = 0;
  //stats_.proc = 0;
  //stats_.ub = 0;
//}


ParPCBProcessor::ParPCBProcessor (EnvPtr env, EnginePtr engine,
                            HandlerVector handlers)
: branches_(0),
  contOnErr_(false),
  cutMan_(0),
  //engineStatus_(EngineUnknownStatus),
  numSolutions_(0),
  //relaxation_(RelaxationPtr()),
  //ws_(WarmStartPtr())
  ws_(0)
{
  oATol_ = env->getOptions()->findDouble("solAbs_tol")->getValue();
  oRTol_ = env->getOptions()->findDouble("solRel_tol")->getValue();
  cutOff_ = env->getOptions()->findDouble("obj_cut_off")->getValue();
  engine_ = engine;
  handlers_ = handlers;
  logger_ = env->getLogger();
  presFreq_ = env->getOptions()-> findInt("pres_freq")->getValue();
  stats_.bra = 0;
  stats_.inf = 0;
  stats_.opt = 0;
  stats_.prob = 0;
  stats_.proc = 0;
  stats_.ub = 0;
}


ParPCBProcessor::~ParPCBProcessor()
{
  if (cutMan_) {
    delete cutMan_;
  }
  if (ws_) {
    ws_->decrUseCnt();
    if (0 == ws_->getUseCnt()) {
      delete ws_;
    }
  }
  if (brancher_) {
    delete brancher_;
  }
  if (branches_) {
    delete branches_;
  }
  handlers_.clear();
}


void ParPCBProcessor::addHeur(HeurPtr h)
{
  heurs_.push_back(h);
}


bool ParPCBProcessor::foundNewSolution()
{
  return (numSolutions_ > 0);
}


Branches ParPCBProcessor::getBranches()
{
  ++stats_.bra;

  return branches_;
}


ConstSolutionPtr ParPCBProcessor::getSolution()
{
  return engine_->getSolution();
}


WarmStartPtr ParPCBProcessor::getWarmStart()
{
  return ws_;
}


bool ParPCBProcessor::isFeasible_(NodePtr node, ConstSolutionPtr sol, 
                              SolutionPoolPtr s_pool, bool &should_prune)
{
  should_prune = false;
  bool is_feas = true;
  HandlerIterator h;
  double inf_meas = 0.0;
  // visit each handler and check feasibility. Stop on the first
  // infeasibility.
  for (h = handlers_.begin(); h != handlers_.end(); ++h) {
    is_feas = (*h)->isFeasible(sol, relaxation_, should_prune, inf_meas);
    if (is_feas == false || should_prune == true) {
      break;
    }
  }

  if (is_feas == true && h==handlers_.end()) {
#pragma omp critical (solPool)
    {
      s_pool->addSolution(sol);
    }
    ++numSolutions_;
    node->setStatus(NodeOptimal);
    ++stats_.opt;
    should_prune = true;
  }
  return is_feas;
}


bool ParPCBProcessor::presolveNode_(NodePtr node, SolutionPoolPtr s_pool) 
{
  ModVector p_mods;      // Mods that are applied to the problem
  ModVector r_mods;      // Mods that are applied to the relaxation.
  bool is_inf = false;
  int it = 0;
  int max_iter = 1;
  bool cont = true;

  bool checkForPresolve = (node->getId() == 0 || presFreq_ < 1) ? true : (node->getId()%presFreq_!=0);

  if (node->getTbScore() != 0 && presFreq_ >= 1) {
    checkForPresolve = ((int)node->getTbScore())%presFreq_!=0;
  }

  if (presFreq_ < 1 || checkForPresolve) {
    return false;
  } 
  // TODO: make this more sophisticated: loop several times until no more
  // changes are possible.
  for (it=0; it<max_iter && true==cont; ++it) {
    ++it;
    for (HandlerIterator h = handlers_.begin(); h != handlers_.end() 
         && false==is_inf; ++h) {
      is_inf = (*h)->presolveNode(relaxation_, node, s_pool, p_mods, r_mods);
      for (ModificationConstIterator m_iter=p_mods.begin();
           m_iter!=p_mods.end(); ++m_iter) {
        node->addPMod(*m_iter);
      }
      for (ModificationConstIterator m_iter=r_mods.begin();
           m_iter!=r_mods.end(); ++m_iter) {
        node->addRMod(*m_iter);
      }
      if ((p_mods.size()==0 && r_mods.size()==0) || true==is_inf) {
        cont = false;
      }
      p_mods.clear();
      r_mods.clear();
    }
  }

  if (is_inf) {
    node->setStatus(NodeInfeasible);
    ++stats_.inf;
  }
  return is_inf;
}


void ParPCBProcessor::process(NodePtr node, RelaxationPtr rel,
                          SolutionPoolPtr s_pool)
{
  UInt numVars = rel->getNumVars();
  UIntVector timesUp(numVars, 0), timesDown(numVars, 0);
  DoubleVector pseudoUp(numVars, 0), pseudoDown(numVars, 0); 
  process(node, rel, s_pool, false, timesUp, timesDown, pseudoUp,  pseudoDown,
                               0);
}


void ParPCBProcessor::process(NodePtr node, RelaxationPtr rel,
                              SolutionPoolPtr s_pool, bool initialized,
                              UIntVector timesUp, UIntVector timesDown,
                              DoubleVector pseudoUp, DoubleVector pseudoDown,
                              UInt nodesProc)
{
  bool should_prune = true;
  bool should_resolve;
  BrancherStatus br_status;
  ConstSolutionPtr sol;
  ModVector mods;
  SeparationStatus sep_status = SepaContinue;
  int iter = 0;

  ++stats_.proc;
  relaxation_ = rel;
  numSolutions_ = 0;

  if (branches_) {
    delete branches_;
    branches_ = 0;
  }

  // presolve
  should_prune = presolveNode_(node, s_pool);
  if (should_prune) {
    node->removeWarmStart();
    return;
  }

  // loop for branching and resolving if necessary.
  while (true) {
    ++iter;
    should_resolve = false;

    if (ws_) {
      ws_->decrUseCnt();
      if (0 == ws_->getUseCnt()) {
        delete ws_;
      }
      ws_ = 0;
    }

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "iteration " << iter << std::endl;
#endif

    solveRelaxation_();
    sol = engine_->getSolution();

    // check if the relaxation is infeasible or if the cost is too high.
    // In either case we can prune. Also set lb of node.
    should_prune = shouldPrune_(node, sol->getObjValue(), s_pool);
    if (should_prune) {
      break;
    }

    // update pseudo-cost from last branching.
    if (iter == 1) {
      if (initialized || node->getId() == 0) {
        brancher_->updateAfterSolve(node, sol);
      }
    }
    // check feasibility. if it is feasible, we can still prune this node.
    isFeasible_(node, sol, s_pool, should_prune);
    if (should_prune) {
      break;
    }

    if (iter == 1 && !node->getParent()) {
      // in root, in first iteration, run a heuristic. XXX: better management.
      for (HeurVector::iterator it=heurs_.begin(); it!=heurs_.end(); ++it) {
        (*it)->solve(node, rel, s_pool);
      }
    }

    // the node can not be pruned because of infeasibility or high cost.
    // continue processing.
    tightenBounds_();
    separate_(sol, node, s_pool, &sep_status);

    if (sep_status == SepaPrune) {
      node->setStatus(NodeInfeasible);
      should_resolve = false;
      stats_.inf++;
      break;
    } else if (sep_status == SepaResolve) {
      should_resolve = true;
    } else {
      //save warm start information before branching. This step is expensive.
      ws_ = engine_->getWarmStartCopy();
      if (ws_) {
        ws_->incrUseCnt();
      }
      if (brancher_->getName()=="ParReliabilityBrancher") {
        ParReliabilityBrancherPtr parRelBr;
        parRelBr = dynamic_cast <ParReliabilityBrancher*> (brancher_);
#pragma omp critical (solPool)
        branches_ = parRelBr->findBranches(relaxation_, node, sol, s_pool,
                                            br_status, mods, timesUp, timesDown,
                                            pseudoUp, pseudoDown, nodesProc);
      } else {
#pragma omp critical (solPool)
        branches_ = brancher_->findBranches(relaxation_, node, sol, s_pool,
                                            br_status, mods);
      }

      if (br_status==PrunedByBrancher) {
        should_prune = true;
        node->setStatus(NodeInfeasible);
        stats_.inf++;
        for (ModificationConstIterator miter=mods.begin(); miter!=mods.end();
             ++miter) {
          delete *miter;
        }
        mods.clear();
        break;
      } else if (br_status==ModifiedByBrancher) {
        for (ModificationConstIterator miter=mods.begin(); miter!=mods.end();
             ++miter) {
          node->addRMod(*miter);
          (*miter)->applyToProblem(relaxation_);
        }
        mods.clear();
        should_prune = presolveNode_(node, s_pool);
        if (should_prune) {
          break;
        }
        should_resolve = true;
      } else if (cutMan_) {
        cutMan_->nodeIsBranched(node,sol,branches_->size());
      }
    }
    if (should_resolve == false) {
      break;
    }
  }
  if (cutMan_ ) {
    cutMan_->updatePool(relaxation_,sol);
    cutMan_->updateRel(sol,relaxation_);
  }
  node->removeWarmStart();
  return;
}

void ParPCBProcessor::separate_(ConstSolutionPtr sol, NodePtr node, 
                            SolutionPoolPtr s_pool, SeparationStatus *status) 
{
  ModVector mods;
  HandlerIterator h;
  SeparationStatus st = SepaContinue;
  bool sol_found;
  ModVector p_mods;      // Mods that are applied to the problem
  ModVector r_mods;      // Mods that are applied to the relaxation.

  *status = SepaContinue;
  sol_found = false;
  for (h = handlers_.begin(); h != handlers_.end(); ++h) {
    (*h)->separate(sol, node, relaxation_, cutMan_, s_pool, p_mods, r_mods,
                   &sol_found, &st);
    if (st == SepaPrune) {
      *status = SepaPrune;
      break;
    } else if (st == SepaResolve) {
      *status = SepaResolve;
    }
    for (ModificationConstIterator m_iter=p_mods.begin();
         m_iter!=p_mods.end(); ++m_iter) {
      node->addPMod(*m_iter);
    }
    for (ModificationConstIterator m_iter=r_mods.begin();
         m_iter!=r_mods.end(); ++m_iter) {
      node->addRMod(*m_iter);
    }
    p_mods.clear();
    r_mods.clear();
  }
  if (true == sol_found) {
    ++numSolutions_;
  }
}


void ParPCBProcessor::setCutManager(CutManager* cutman)
{
  cutMan_ = cutman;
}


bool ParPCBProcessor::shouldPrune_(NodePtr node, double solval, 
                               SolutionPoolPtr s_pool)
{
  bool should_prune = false;
  double best_cutoff;
#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "solution value = " << solval
                                << std::endl; 
#endif
  switch (engineStatus_) {
   case (FailedInfeas):
     logger_->msgStream(LogInfo) << me_ << "failed to converge "
                                 << "(infeasible) in node " << node->getId()
                                 << std::endl;
     node->setStatus(NodeInfeasible);
     should_prune = true;
     ++stats_.inf;
     ++stats_.prob;
     break;
   case (ProvenFailedCQInfeas):
     logger_->msgStream(LogInfo) << me_ << "constraint qualification "
                                 << "violated in node " << node->getId()
                                 << std::endl;
     ++stats_.prob;
     // This comment silences g++ warnings
     // fall through
   case (ProvenInfeasible):
   case (ProvenLocalInfeasible):
     node->setStatus(NodeInfeasible);
     should_prune = true;
     ++stats_.inf;
     break;

   case (ProvenObjectiveCutOff):
     node->setStatus(NodeHitUb);
     should_prune = true;
     ++stats_.ub;
     break;

   case (ProvenUnbounded):
     should_prune = false;
     logger_->msgStream(LogDebug2) << me_ << "problem relaxation is "
                                   << "unbounded!" << std::endl;
     assert(!"Relaxation unbounded.");
     break;

   case (FailedFeas):
     logger_->msgStream(LogInfo) << me_ << "Failed to converge " 
                                 << "(feasible) in node " << node->getId()
                                 << std::endl;
     if (node->getParent()) {
       node->setLb(node->getParent()->getLb());
     } else {
       node->setLb(-INFINITY);
     }
     node->setStatus(NodeContinue);
     ++stats_.prob;
     break;
   case (ProvenFailedCQFeas):
     logger_->msgStream(LogInfo) << me_ << "constraint qualification "
                                 << "violated in node " << node->getId()
                                 << std::endl;
     if (node->getParent()) {
       node->setLb(node->getParent()->getLb());
     } else {
       node->setLb(-INFINITY);
     }
     node->setStatus(NodeContinue);
     ++stats_.prob;
     break;
   case (EngineIterationLimit):
     ++stats_.prob;
     logger_->msgStream(LogInfo) << me_ << "engine hit iteration limit, "
                                 << "continuing in node " << node->getId()
                                 << std::endl;
     // continue with this node by following ProvenLocalOptimal case.
     // fall through
   case (ProvenLocalOptimal):
   case (ProvenOptimal):
     node->setLb(solval);
     if (node->getParent() &&
         node->getParent()->getLb() > solval+oATol_ &&
         node->getParent()->getLb() > solval+fabs(solval)*oRTol_ ) {
#pragma omp critical (logger)
       logger_->msgStream(LogError) << me_ << "node lb lower than parent's lb. "
                                    << "Relaxation may not be convex or "
                                    << "engine has an issue."
                                    << " Node: " << node->getId() << " nodelb: " 
                                    << std::setprecision(10) << node->getLb() << " Parent: " 
                                    << node->getParent()->getId() 
                                    << " parentlb: " << node->getParent()->getLb()
                                    << std::endl;
     }
     best_cutoff = s_pool->getBestSolutionValue();
     if (solval >= best_cutoff-oATol_ ||
         solval >= best_cutoff-fabs(best_cutoff)*oRTol_ || solval >= cutOff_) {
       node->setStatus(NodeHitUb);
       should_prune = true;
       ++stats_.ub;
     } else {
       should_prune = false;
       node->setStatus(NodeContinue);
     }
     break;
   case (EngineError):
     if (contOnErr_) {
       logger_->msgStream(LogError) << me_ << "engine reports error, "
                                    << " continuing in node " << node->getId()
                                    << std::endl;
       node->setStatus(NodeContinue);
       if (node->getParent()) {
         node->setLb(node->getParent()->getLb());
       } else {
         node->setLb(-INFINITY);
       }
     } else {
       logger_->msgStream(LogError) << me_ << "engine reports error, "
                                    << "pruning node " << node->getId()
                                    << std::endl;
       should_prune = true;
       node->setStatus(NodeInfeasible);
       ++stats_.inf;
     }
     ++stats_.prob;
     break;
   default:
     break;
  }

  return should_prune;
}


void ParPCBProcessor::solveRelaxation_() 
{
  engineStatus_ = EngineError;
  engine_->solve();
  engineStatus_ = engine_->getStatus();
#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "solving relaxation" << std::endl
                                << me_ << "engine status = " 
                                << engine_->getStatusString() << std::endl;
#endif
}


void ParPCBProcessor::tightenBounds_() 
{

}


void ParPCBProcessor::writeStats(std::ostream &out) const
{
  out << me_ << "nodes processed     = " << stats_.proc << std::endl 
      << me_ << "nodes branched      = " << stats_.bra << std::endl 
      << me_ << "nodes infeasible    = " << stats_.inf << std::endl 
      << me_ << "nodes optimal       = " << stats_.opt << std::endl 
      << me_ << "nodes hit ub        = " << stats_.ub << std::endl 
      << me_ << "nodes with problems = " << stats_.prob << std::endl 
      ;
}


void ParPCBProcessor::writeStats() const
{
  writeStats(logger_->msgStream(LogNone));
}


