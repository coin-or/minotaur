// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
// 

/**
 * \file BranchAndBound.cpp
 * \brief Define BranchAndBound class for a generic branch-and-bound method.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iomanip>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Brancher.h"
#include "Environment.h"
#include "Heuristic.h"
#include "Logger.h"
#include "Node.h"
#include "NodeProcessor.h"
#include "NodeRelaxer.h"
#include "Option.h"
#include "Problem.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "TreeManager.h"


//#define DEBUG 1
//#define SPEW 1

using namespace Minotaur;

const std::string BranchAndBound::me_ = "branch-and-bound: ";

BranchAndBound::BranchAndBound()
  : env_(EnvPtr()),               // NULL
    nodePrcssr_(),                // NULL
    nodeRlxr_(NodeRelaxerPtr()),  // NULL
    options_(BabOptionsPtr()),    // NULL
    problem_(ProblemPtr()),       // NULL
    solPool_(SolutionPoolPtr()),  // NULL
    stats_(0),
    status_(NotStarted),
    timer_(0),                    // NULL
    tm_(TreeManagerPtr())         // NULL
{
}


BranchAndBound::BranchAndBound(EnvPtr env, ProblemPtr p) 
  : env_(env),
    nodePrcssr_(),                // NULL
    nodeRlxr_(NodeRelaxerPtr()),  // NULL
    problem_(p),
    solPool_(SolutionPoolPtr()),  // NULL
    stats_(0),
    status_(NotStarted)
{
  timer_ = env->getNewTimer();
  assert (env_);

  tm_ = (TreeManagerPtr) new TreeManager(env);
  options_ = (BabOptionsPtr) new BabOptions(env);
  logger_ = (LoggerPtr) new Logger(options_->logLevel);
}


BranchAndBound::~BranchAndBound()
{
  options_.reset();
  logger_.reset();
  nodePrcssr_.reset();
  nodeRlxr_.reset();
  tm_.reset();
  solPool_.reset();
  problem_.reset();
  env_.reset();
  if (timer_) {
    delete timer_;
  }
  if (stats_) {
    delete stats_;
  }
}


void BranchAndBound::addPreRootHeur(HeurPtr h)
{
  preHeurs_.push_back(h);
}


Double BranchAndBound::getGap() 
{ 
  return tm_->getGap(); 
}


Double BranchAndBound::getLb() 
{
  return tm_->getLb();
}


NodeProcessorPtr BranchAndBound::getNodeProcessor()
{
  return nodePrcssr_;
}


NodeRelaxerPtr BranchAndBound::getNodeRelaxer()
{
  return nodeRlxr_;
}


SolutionPtr BranchAndBound::getSolution()
{
  return solPool_->getBestSolution();
}


SolveStatus BranchAndBound::getStatus()
{
  return status_;
}


TreeManagerPtr BranchAndBound::getTreeManager() 
{
  return tm_;
}


Double BranchAndBound::getUb() 
{
  return tm_->getUb();
}


UInt BranchAndBound::numProcNodes()
{
  return stats_->nodesProc;
}


NodePtr BranchAndBound::processRoot_(Bool *should_prune, Bool *should_dive)
{
  NodePtr current_node = (NodePtr) new Node ();
  NodePtr new_node = NodePtr(); // NULL
  RelaxationPtr rel;
  Bool prune = *should_prune;
  Branches branches;
  WarmStartPtr ws;
#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "creating root node" << 
    std::endl;
#endif
  tm_->insertRoot(current_node);

  //std
  if (options_->createRoot == true) {
    rel = nodeRlxr_->createRootRelaxation(current_node, prune);
  } else {
    rel = nodeRlxr_->getRelaxation();
  }

  // Here you should apply the modifications found in createRootRelaxation

  // solve the root node
#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "processing root node" << 
    std::endl;
#endif
  nodePrcssr_->processRootNode(current_node, rel, solPool_);
  ++stats_->nodesProc;
  if (nodePrcssr_->foundNewSolution()) {
    tm_->setUb(solPool_->getBestSolutionValue());
  }
  
  prune = shouldPrune_(current_node);
  if (prune) {
    nodeRlxr_->reset(current_node, false);
    tm_->pruneNode(current_node);
  } else {
#if SPEW
    logger_->MsgStream(LogDebug1) << me_ << "branching in root" << 
      std::endl;
#endif
    // branch.
    branches = nodePrcssr_->getBranches();
    ws = nodePrcssr_->getWarmStart();
    tm_->removeActiveNode(current_node);
    *should_dive = tm_->shouldDive();
    new_node = tm_->branch(branches, current_node, ws);
    assert((*should_dive && new_node) || (!(*should_dive) && !new_node));
    if (!(*should_dive)) {
      nodeRlxr_->reset(current_node, false);
      new_node = tm_->getCandidate();
      assert(new_node);
    }
  }
  current_node = new_node;

  tm_->updateLb();

  showStatus_(*should_dive);
  *should_prune = prune;
  return current_node;
}


void BranchAndBound::setLogLevel(LogLevel level) 
{
  logger_->SetMaxLevel(level);
}


void BranchAndBound::setNodeProcessor(NodeProcessorPtr p)
{
  nodePrcssr_ = p;
}


void BranchAndBound::setNodeRelaxer(NodeRelaxerPtr nr)
{
  nodeRlxr_ = nr;
}


void BranchAndBound::shouldCreateRoot(Bool b)
{
  options_->createRoot = b;
}


Bool BranchAndBound::shouldPrune_(NodePtr node)
{
  Bool should_prune = false;
  switch (node->getStatus()) {
   case (NodeOptimal):
     should_prune = true;
   case (NodeHitUb):
     should_prune = true;
     // check if we want to search for more solutions
     break;
   case (NodeInfeasible):
     should_prune = true;
     break;
   case (NodeDominated):
     should_prune = true;
     break;
   case (NodeStopped):
     assert(!"implement me!");
     break;
   case (NodeContinue):
     should_prune = false;
     break;
   case (NodeNotProcessed):
     assert(!"node not processed, still being asked if it should be pruned");
     break;
  }
  return should_prune;
}


Bool BranchAndBound::shouldStop_()
{
  Bool stop_bnb = false;
  // double lb = tm_->getLb();
  // ++lb;

  if (tm_->getGap() < 0.0) {
    stop_bnb = true;
    status_ = SolvedOptimal;
    logger_->MsgStream(LogInfo) << me_ << "threshold gap has been "
      << "achieved" << std::endl
      << me_ << "gap = " << tm_->getGap() << std::endl;
  } else if (false) {//TODO: stuck in some wiered position
    stop_bnb = true;
  } else if (timer_->query() > options_->timeLimit) {
    stop_bnb = true;
    status_ = TimeLimitReached;
  } else if (stats_->nodesProc >= options_->nodeLimit) {
    stop_bnb = true;
    status_ = IterationLimitReached;
  } else if (false) { // TODO: memory limit or other limits
    stop_bnb = true;
  }

  return stop_bnb;
}


void BranchAndBound::showStatus_(Bool current_uncounted)
{
  UInt off=0;
  if (current_uncounted) {
    off=1;
  }
  if (timer_->query()-stats_->updateTime > options_->logInterval) {
    logger_->MsgStream(LogInfo) 
      << me_ 
      << std::fixed
      << std::setprecision(2)  << "time = "            << timer_->query()
      << std::setprecision(4)  << " lb = "             << tm_->updateLb()
      << std::setprecision(4)  << " ub = "             << tm_->getUb()
      << " nodes processed = " << tm_->getSize()-tm_->getActiveNodes()-off 
      << " nodes left = "      << tm_->getActiveNodes()+off
      << std::endl;
    stats_->updateTime = timer_->query();
  }
}


void BranchAndBound::solve()
{
  Bool should_dive = false, dived_prev = false;
  Bool should_prune = false;
  NodePtr current_node = NodePtr();
  NodePtr new_node = NodePtr();
  Branches branches;
  WarmStartPtr ws;
  RelaxationPtr rel = RelaxationPtr();
  Bool should_stop = false;

  // initialize timer
  timer_->start();

  // get problem size and statistics to detect problem type.
  problem_->calculateSize();
#if SPEW
  problem_->writeSize(logger_->MsgStream(LogInfo));
#endif

  // initialize statistics
  if (stats_) {
    delete stats_;
  }
  stats_ = new BabStats();

  // initialize solution pool
  // TODO: use user options to set the pool size. For now it is 1.
  solPool_ = (SolutionPoolPtr) new SolutionPool(problem_, 1);

  // call heuristics before the root, if needed 
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->solve(current_node, rel, solPool_);
  }
  tm_->setUb(solPool_->getBestSolutionValue());

  // do the root
  current_node = processRoot_(&should_prune, &dived_prev);

  // stop if done
  if (!current_node) {
    tm_->updateLb();
    if (tm_->getUb() <= -INFINITY) {
      status_ = SolvedUnbounded;
    } else  if (tm_->getUb() < INFINITY) {
      status_ = SolvedOptimal; 
    } else {
      status_ = SolvedInfeasible; 
    }
#if SPEW
    logger_->MsgStream(LogDebug) << me_ << "stopping after root node "
      << std::endl;
#endif
    should_stop = true;
  } else if (shouldStop_()) {
    tm_->updateLb();
    should_stop = true;
  } else {
#if SPEW
    logger_->MsgStream(LogDebug) << std::setprecision(8)
      << me_ << "lb = " << tm_->updateLb() << std::endl
      << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
  }


  // solve root outside the loop. save the useful information.
  while(should_stop == false) {
#if SPEW
    logger_->MsgStream(LogDebug1) << me_ << "processing node "
      << current_node->getId() << std::endl
      << me_ << "depth = " << current_node->getDepth() << std::endl
      << me_ << "did we dive = " << dived_prev << std::endl;
#endif

    should_dive = false;
    rel = nodeRlxr_->createNodeRelaxation(current_node, dived_prev, 
                                          should_prune);
    nodePrcssr_->process(current_node, rel, solPool_);

    ++stats_->nodesProc;
#if SPEW
    logger_->MsgStream(LogDebug1) << me_ << "node lower bound = " << 
      current_node->getLb() << std::endl;
#endif

    if (nodePrcssr_->foundNewSolution()) {
      tm_->setUb(solPool_->getBestSolutionValue());
    }
    
    should_prune = shouldPrune_(current_node);
    if (should_prune) {
#if SPEW
      logger_->MsgStream(LogDebug1) << me_ << "node pruned" << 
        std::endl;
#endif
      nodeRlxr_->reset(current_node, false);
      tm_->pruneNode(current_node);
      if (!dived_prev) {
        tm_->removeActiveNode(current_node);
      }
      new_node = tm_->getCandidate();
      dived_prev = false;
    } else {
#if SPEW
      logger_->MsgStream(LogDebug1) << me_ << "branching" << 
        std::endl;
#endif
      branches = nodePrcssr_->getBranches();
   
      ws = nodePrcssr_->getWarmStart();
      if (!dived_prev) {
        tm_->removeActiveNode(current_node);
      }
      should_dive = tm_->shouldDive();
    
      new_node = tm_->branch(branches, current_node, ws);
      assert((should_dive && new_node) || (!should_dive && !new_node));
      if (should_dive) {
        dived_prev = true;
      } else {
        nodeRlxr_->reset(current_node, false);
        new_node = tm_->getCandidate(); // Can be NULL. The branches that were
                                        // created could have large lb and tm 
                                        // might have eliminated them.
        dived_prev = false;
      }
    }
    current_node = new_node;

    showStatus_(should_dive);

    // stop if done
    if (!current_node) {
      tm_->updateLb();
      if (tm_->getUb() <= -INFINITY) {
        status_ = SolvedUnbounded;
      } else if (tm_->getUb() < INFINITY) {
        status_ = SolvedOptimal; // TODO: get the right status
      } else {
        status_ = SolvedInfeasible; // TODO: get the right status
      }
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "all nodes have "
        << "been processed" << std::endl;
#endif
      break;
    } else if (shouldStop_()) {
      tm_->updateLb();
      break;
    } else {
#if SPEW
      logger_->MsgStream(LogDebug) << std::setprecision(8)
        << me_ << "lb = " << tm_->updateLb() << std::endl 
        << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
    }
  } 
  stats_->timeUsed = timer_->query();
  timer_->stop();
}


void BranchAndBound::writeStats()
{
  logger_->MsgStream(LogInfo)
    << me_ << "time taken      = " << std::fixed << std::setprecision(2)
    << stats_->timeUsed << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  nodePrcssr_->writeStats();
  nodePrcssr_->getBrancher()->writeStats();
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->writeStats();
  }
}


Double BranchAndBound::totalTime()
{
  return stats_->timeUsed;
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

BabStats::BabStats()
  :nodesProc(0),
   timeUsed(0),
   updateTime(0)
{
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
BabOptions::BabOptions()
  : createRoot(true),
    logLevel(LogInfo),
    nodeLimit(0),
    timeLimit(0.)
    
{
}


BabOptions::BabOptions(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();

  logLevel    = (LogLevel) options->findInt("bnb_log_level")->getValue();
  timeLimit   = options->findDouble("bnb_time_limit")->getValue();
  nodeLimit   = options->findInt("bnb_node_limit")->getValue();
  logInterval = options->findDouble("bnb_log_interval")->getValue();
  createRoot  = true;
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
