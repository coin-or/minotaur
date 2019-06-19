// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/**
 * \file ParQGBranchAndBound.cpp
 * \brief Define ParQGBranchAndBound class for a generic parallel branch-and-bound method.
 * \author Prashant Palkar, Meenarli Sharma, IIT Bombay
 */

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#if USE_OPENMP
#include <omp.h>
#endif
#include "MinotaurConfig.h"
#include "Brancher.h"
#include "Constraint.h"
#include "CutManager.h"
#include "Environment.h"
#include "Function.h"
#include "Heuristic.h"
#include "Logger.h"
#include "LinearFunction.h"
#include "Node.h"
#include "NodeProcessor.h"
#include "NodeRelaxer.h"
#include "Option.h"
#include "ParCutMan.h"
#include "ParPCBProcessor.h"
#include "ParQGBranchAndBound.h"
#include "ParNodeIncRelaxer.h"
#include "ParTreeManager.h"
#include "Problem.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Branch.h"
#include "BrCand.h"
#include "BrVarCand.h"


//#define SPEW 1
#define PRINT 0

using namespace Minotaur;

const std::string ParQGBranchAndBound::me_ = "branch-and-bound: ";

ParQGBranchAndBound::ParQGBranchAndBound()
  : env_(EnvPtr()),             // NULL
  nodePrcssr_(),                // NULL
  nodeRlxr_(NodeRelaxerPtr()),  // NULL
  options_(ParQGBabOptionsPtr()), // NULL
  problem_(ProblemPtr()),       // NULL
  solPool_(SolutionPoolPtr()),  // NULL
  stats_(0),
  status_(NotStarted),
  timer_(0),                    // NULL
  tm_(ParTreeManagerPtr())      // NULL
{
}


ParQGBranchAndBound::ParQGBranchAndBound(EnvPtr env, ProblemPtr p)
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

  tm_ = (ParTreeManagerPtr) new ParTreeManager(env);
  options_ = (ParQGBabOptionsPtr) new ParQGBabOptions(env);
  logger_ = (LoggerPtr) new Logger(options_->logLevel);
}


ParQGBranchAndBound::~ParQGBranchAndBound()
{
  //options_.reset();
  //logger_.reset();
  options_ = 0;
  if (logger_){
    delete logger_;
  }
  //nodePrcssr_.reset();
  //nodeRlxr_.reset();
  nodeRlxr_ = 0;
  nodePrcssr_ = 0;
  //tm_.reset();
  //solPool_.reset();
  //problem_.reset();
  //env_.reset();
  tm_ = 0;
  solPool_ = 0;
  problem_ = 0;
  env_ = 0;
  if (timer_) {
    delete timer_;
  }
  if (stats_) {
    delete stats_;
  }
}


void ParQGBranchAndBound::addPreRootHeur(HeurPtr h)
{
  preHeurs_.push_back(h);
}


double ParQGBranchAndBound::getPerGap() 
{ 
  return tm_->getPerGap(); 
}


double ParQGBranchAndBound::getLb() 
{
  return tm_->getLb();
}


NodeProcessorPtr ParQGBranchAndBound::getNodeProcessor()
{
  return nodePrcssr_;
}


NodeRelaxerPtr ParQGBranchAndBound::getNodeRelaxer()
{
  return nodeRlxr_;
}


SolutionPtr ParQGBranchAndBound::getSolution()
{
  return solPool_->getBestSolution();
}


SolveStatus ParQGBranchAndBound::getStatus()
{
  return status_;
}


ParTreeManagerPtr ParQGBranchAndBound::getTreeManager() 
{
  return tm_;
}


double ParQGBranchAndBound::getUb() 
{
  return tm_->getUb();
}


UInt ParQGBranchAndBound::numProcNodes()
{
  return stats_->nodesProc;
}


NodePtr ParQGBranchAndBound::processRoot_(bool *should_prune, bool *should_dive,
                                        ParNodeIncRelaxerPtr parNodeRlxr0,
                                        ParPCBProcessorPtr nodePrcssr0,
                                        WarmStartPtr ws0)
{
  NodePtr current_node = (NodePtr) new Node ();
  NodePtr new_node = NodePtr(); // NULL
  RelaxationPtr rel;
  bool prune = *should_prune;
  Branches branches;
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "creating root node" << 
    std::endl;
#endif
  tm_->insertRoot(current_node);

  if (options_->createRoot == true) {
    rel = parNodeRlxr0->createRootRelaxation(current_node, prune);
    rel->setProblem(problem_);
  } else {
    rel = parNodeRlxr0->getRelaxation();
  }

  // solve the root node
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "processing root node" << 
    std::endl;
#endif
  nodePrcssr0->processRootNode(current_node, rel, solPool_);
  ++stats_->nodesProc;
  if (nodePrcssr0->foundNewSolution()) {
    tm_->setUb(solPool_->getBestSolutionValue());
  }

  prune = shouldPrune_(current_node);
  if (prune) {
    parNodeRlxr0->reset(current_node, false);
    tm_->pruneNode(current_node);
  } else {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << "branching in root" << 
      std::endl;
#endif
    // branch.
    branches = nodePrcssr0->getBranches();
    ws0 = nodePrcssr0->getWarmStart();
    tm_->removeActiveNode(current_node);
    *should_dive = tm_->shouldDive();
    if (env_->getOptions()->findString("tb_rule")->getValue() == "twoChild") {
      current_node->setTbScore(1);
    }
    new_node = tm_->branch(branches, current_node, ws0);
    assert((*should_dive && new_node) || (!(*should_dive) && !new_node));
    if (!(*should_dive)) {
      parNodeRlxr0->reset(current_node, false);
      new_node = tm_->getCandidate();
      assert(new_node);
      tm_->removeActiveNode(new_node);
    }
  }
  current_node = new_node;

  tm_->updateLb();

  showStatus_(*should_dive);
  *should_prune = prune;
  return current_node;
}

void ParQGBranchAndBound::setLogLevel(LogLevel level) 
{
  logger_->setMaxLevel(level);
}


void ParQGBranchAndBound::setNodeProcessor(NodeProcessorPtr p)
{
  nodePrcssr_ = p;
}


void ParQGBranchAndBound::setNodeRelaxer(NodeRelaxerPtr nr)
{
  nodeRlxr_ = nr;
}


void ParQGBranchAndBound::shouldCreateRoot(bool b)
{
  options_->createRoot = b;
}


bool ParQGBranchAndBound::shouldPrune_(NodePtr node)
{
  bool should_prune = false;
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


bool ParQGBranchAndBound::shouldStop_()
{
  bool stop_bnb = false;

  if (tm_->getPerGap() <= 0.0) {
    stop_bnb = true;
    status_ = SolvedOptimal;
  } else if ( tm_->getPerGap() <= options_->perGapLimit) {
    stop_bnb = true;
    status_ = SolvedGapLimit;
  } else if (timer_->query() > options_->timeLimit) {
    stop_bnb = true;
    status_ = TimeLimitReached;
  } else if (stats_->nodesProc >= options_->nodeLimit) {
    stop_bnb = true;
    status_ = IterationLimitReached;
  } else if (solPool_->getNumSolsFound()>=options_->solLimit) { 
    stop_bnb = true;
    status_ = SolLimitReached;
  }

  return stop_bnb;
}


bool ParQGBranchAndBound::shouldStopPar_(double WallTimeStart, double treeLb)
{
  bool stop_bnb = false;

  if (tm_->getPerGapPar(treeLb) <= 0.0) {
    stop_bnb = true;
    status_ = SolvedOptimal;
  } else if ( tm_->getPerGapPar(treeLb) <= options_->perGapLimit) {
    stop_bnb = true;
    status_ = SolvedGapLimit;
  } else if ((getWallTime() - WallTimeStart) > options_->timeLimit) {
    stop_bnb = true;
    status_ = TimeLimitReached;
  } else if (stats_->nodesProc >= options_->nodeLimit) {
    stop_bnb = true;
    status_ = IterationLimitReached;
  } else if (solPool_->getNumSolsFound()>=options_->solLimit) { 
    stop_bnb = true;
    status_ = SolLimitReached;
  }
  return stop_bnb;
}


void ParQGBranchAndBound::showStatus_(bool current_uncounted)
{
  UInt off=0;
  if (current_uncounted) {
    off=1;
  }
  if (timer_->query()-stats_->updateTime > options_->logInterval) {
    double lb = tm_->updateLb();
    logger_->msgStream(LogInfo) 
      << me_ 
      << std::fixed
      << std::setprecision(1)  << "time = "            << timer_->query()
      << std::setprecision(4)  << " lb = "             << lb
      << std::setprecision(4)  << " ub = "             << tm_->getUb()
      << std::setprecision(2)  << " gap% = "           << tm_->getPerGap()
      << " nodes processed = " << tm_->getSize()-tm_->getActiveNodes()-off 
      << " left = "            << tm_->getActiveNodes()+off
      << std::endl;
    stats_->updateTime = timer_->query();
  }
}


void ParQGBranchAndBound::showParStatus_(UInt off, double treeLb,
                                       double WallTimeStart)
{
  if (timer_->query()-stats_->updateTime > options_->logInterval) {
    //double lb = tm_->updateLb();
    logger_->msgStream(LogInfo) 
      << me_ 
      << std::fixed
      //<< std::setprecision(1)  << "time = " << timer_->query()
      << std::setprecision(1)  << "time = " << getWallTime() - WallTimeStart
      << std::setprecision(4)  << " lb = "  << treeLb
      << std::setprecision(4)  << " ub = "  << tm_->getUb()
      << std::setprecision(2)  << " gap% = " << tm_->getPerGapPar(treeLb)
      << " nodes processed = " << tm_->getSize()-tm_->getActiveNodes()-off 
      << " left = " << tm_->getActiveNodes()+off
      << std::endl;
    stats_->updateTime = timer_->query();
  }
}


void ParQGBranchAndBound::parsolve(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  double wallTimeStart = getWallTime();
  bool *should_dive = new bool[numThreads];
  bool *dived_prev = new bool[numThreads];
  bool *should_prune = new bool[numThreads];
  bool *initialized = new bool[numThreads];
  NodePtr *current_node = new NodePtr[numThreads];
  NodePtr *new_node = new NodePtr[numThreads];
  Branches *branches = new Branches[numThreads];
  WarmStartPtr *ws = new WarmStartPtr[numThreads];
  RelaxationPtr *rel = new RelaxationPtr[numThreads];
  UInt nodeCount;
  double treeLb, nodeLb, minNodeLb;
  double *treeLbTh = new double[numThreads];
  double *nodeLbTh = new double[numThreads];
  double *minNodeLbTh = new double[numThreads];
  bool *shouldRunTh = new bool[numThreads];
  UInt *nodeCountTh = new UInt[numThreads];
  std::vector<ParCutMan*> cutman(numThreads, new ParCutMan(env_, problem_));
  bool iterMode = env_->getOptions()->findBool("mcbnb_iter_mode")->getValue();
  UInt iterCount = 1;
  UInt *cutsIndex = new UInt[numThreads*numThreads]();
  UInt *numCuts = new UInt[numThreads]();
  UInt numVars = 0;

#if USE_OPENMP
#pragma omp parallel for
#endif
  for(UInt i = 0; i < numThreads; ++i) {
    // declare cut manager
    nodePrcssr[i]->setCutManager(cutman[i]);
    should_dive[i] = false;
    dived_prev[i] = false;
    should_prune[i] = false;
    initialized[i] = false;
    shouldRunTh[i] = true;
    nodeCountTh[i] = 1;
  }

  // initialize timer
  timer_->start();

  logger_->msgStream(LogInfo) << me_ << "starting branch-and-bound ";
  if(numThreads > 1) {
#if USE_OPENMP
  logger_->msgStream(LogInfo) << "using " << numThreads << " out of "
    << omp_get_num_procs() << " processors";
#endif
  }
  logger_->msgStream(LogInfo) << std::endl;
  // get problem size and statistics to detect problem type.
  problem_->calculateSize();
#if SPEW
  problem_->writeSize(logger_->msgStream(LogExtraInfo));
#endif

  // initialize statistics
  if (stats_) {
    delete stats_;
  }
  stats_ = new ParQGBabStats();

  // initialize solution pool
  // TODO: use user options to set the pool size. For now it is 1.
  solPool_ = (SolutionPoolPtr) new SolutionPool(env_, problem_, 1);

  rel[0] = parNodeRlxr[0]->getRelaxation();
  // call heuristics before the root, if needed 
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->solve(current_node[0], rel[0], solPool_);
  }
  tm_->setUb(solPool_->getBestSolutionValue());

  // do the root
  current_node[0] = processRoot_(&should_prune[0], &dived_prev[0],
                                 parNodeRlxr[0], nodePrcssr[0], ws[0]);
  // stop if done
  if (!current_node[0]) { 
    tm_->updateLb();
    if (tm_->getUb() <= -INFINITY) {
      status_ = SolvedUnbounded;
    } else  if (tm_->getUb() < INFINITY) {
      status_ = SolvedOptimal; 
    } else {
      status_ = SolvedInfeasible; 
    }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "stopping after root node "
      << std::endl;
#endif
    nodeCount = 0;
  } else if (shouldStopPar_(wallTimeStart, tm_->getLb())) {
    tm_->updateLb();
    nodeCount = 1;
  } else {
#if SPEW
    logger_->msgStream(LogDebug) << std::setprecision(8)
      << me_ << "lb = " << tm_->updateLb() << std::endl
      << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
    nodeCount = 1;
  }

  // solve root outside the loop. save the useful information.
  bool shouldRun = true;
  initialized[0] = true; //pseudoCosts for thread0 initialized while doing root
  numVars = rel[0]->getNumVars();
  bool isParRel = false;
  if (nodePrcssr[0]->getBrancher()->getName() == "ParReliabilityBrancher") {
    isParRel = true;
  }

  while(nodeCount > 0 && shouldRun) {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << "processing node "
      << current_node[0]->getId() << std::endl
      << me_ << "depth = " << current_node[0]->getDepth() << std::endl
      << me_ << "did we dive = " << dived_prev[0] << std::endl;
#endif

#if USE_OPENMP
#pragma omp parallel 
#endif
    {
#if USE_OPENMP
#pragma omp for
#endif
      for(UInt i = 0; i < numThreads; ++i) {
        FunctionPtr f;
        VariablePtr v;
        LinearFunctionPtr lf, lfnew;
        //brancher related
        ParReliabilityBrancherPtr parRelBr;
        UIntVector tmpTimesUp, tmpTimesDown, timesUp, timesDown, lastStrBranched;
        DoubleVector tmpPseudoUp, tmpPseudoDown, pseudoUp, pseudoDown;
        if (isParRel) {
          timesUp.resize(numVars,0);
          timesDown.resize(numVars,0);
          pseudoUp.resize(numVars,0);
          pseudoDown.resize(numVars,0);
          lastStrBranched.resize(numVars,0);
        }

        if (iterMode == true) shouldRunTh[i] = true;
        while (nodeCountTh[i] > 0 && shouldRunTh[i]) {
          if (current_node[i]) {
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
            {
              if (tm_->shouldPrune_(current_node[i])) {
                parNodeRlxr[i]->reset(current_node[i], false);
                tm_->pruneNode(current_node[i]);
                current_node[i] = NodePtr();
              }
            }
          }
          if (!current_node[i]) {
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
            {
              current_node[i] = tm_->getCandidate();
              if(current_node[i]) {
                tm_->removeActiveNode(current_node[i]);
                dived_prev[i] = false;
              }
            }
          }
          if (current_node[i]) { 
#if SPEW
            logger_->msgStream(LogDebug1) << me_ << "processing node "
              << current_node[i]->getId() << std::endl
              << me_ << "depth = " << current_node[i]->getDepth() << std::endl
              << me_ << "did we dive = " << dived_prev[i] << std::endl;
#endif
            should_dive[i] = false;

            rel[i] = parNodeRlxr[i]->createNodeRelaxation(current_node[i],
                                                          dived_prev[i],
                                                          should_prune[i]);
            for(UInt j = 0; j < numThreads; ++j) {
              if (i!=j || numThreads==1) {
                std::vector<ConstraintPtr > consVec = nodePrcssr[j]->getCutManager()->getPoolCons();
                if (consVec.size() > 0) {
                  for (UInt k=cutsIndex[i*numThreads+j]; k < consVec.size(); ++k) {
                    lf = consVec[k]->getLinearFunction();
                    if (lf) {
                      lfnew = (LinearFunctionPtr) new LinearFunction();
                      for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();++it) {
                        v = it->first;
                        //transform the cut
                        lfnew->addTerm(rel[i]->getRelaxationVar(rel[i]->getOriginalVar(v)),it->second); //rel[j] not needed
                      }
                      f = (FunctionPtr) new Function(lfnew);
                      rel[i]->newConstraint(f, consVec[k]->getLb(), 
                                            consVec[k]->getUb(), consVec[k]->getName());
                    }
                  }
                  cutsIndex[i*numThreads+j] = consVec.size();
                  numCuts[j] = consVec.size();
                  consVec.clear();
                }
                if (isParRel) {
                  parRelBr = dynamic_cast <ParReliabilityBrancher*> (nodePrcssr[j]->getBrancher());
                  tmpTimesUp = parRelBr->getTimesUp();
                  tmpTimesDown = parRelBr->getTimesDown();
                  tmpPseudoUp = parRelBr->getPCUp();
                  tmpPseudoDown = parRelBr->getPCDown();
                  for (UInt l=0; l < tmpTimesDown.size(); ++l) {
                    timesUp[l] += tmpTimesUp[l];
                    timesDown[l] += tmpTimesDown[l];
                    pseudoUp[l] += tmpTimesUp[l]*tmpPseudoUp[l];
                    pseudoDown[l] += tmpTimesDown[l]*tmpPseudoDown[l];
                  }
                }
              }
            }
            nodePrcssr[i]->process(current_node[i], rel[i], solPool_,
                                   initialized[i], timesUp, timesDown,
                                   pseudoUp, pseudoDown, stats_->nodesProc);
#if USE_OPENMP
#pragma omp critical
#endif
            ++stats_->nodesProc;

#if SPEW
            logger_->msgStream(LogDebug1) << me_ << "node lower bound = " <<
              current_node[i]->getLb() << std::endl;
#endif
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
            {
              if (nodePrcssr[i]->foundNewSolution()) { 
                tm_->setUb(solPool_->getBestSolutionValue());
              }
            }
            should_prune[i] = shouldPrune_(current_node[i]);

            if (should_prune[i]) {
#if SPEW
              logger_->msgStream(LogDebug1) << me_ << "node pruned" << 
                std::endl;
#endif
              parNodeRlxr[i]->reset(current_node[i], false);
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
              {
                tm_->pruneNode(current_node[i]);
              }
              //if (!dived_prev[i]) {
//#if USE_OPENMP
//#pragma omp critical (treeManager)
//#endif
                //tm_->removeActiveNode(current_node[i]);
              //}

#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
              {
                new_node[i] = tm_->getCandidate();
                if(new_node[i]) {
                  tm_->removeActiveNode(new_node[i]);
                }
              }
              dived_prev[i] = false;

            } else {
              initialized[i] = true;
#if SPEW
              logger_->msgStream(LogDebug1) << me_ << "branching" << 
                std::endl;
#endif
              branches[i] = nodePrcssr[i]->getBranches();

              ws[i] = nodePrcssr[i]->getWarmStart();

              should_dive[i] = tm_->shouldDive();
              if (!branches[i]) {
                std::cout<<" NO BRANCHES \n";
              }
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
              {
                new_node[i] = tm_->branch(branches[i], current_node[i], ws[i]);
              }
              assert((should_dive[i] && new_node[i])
                     || (!should_dive[i] && !new_node[i]));
              if (should_dive[i]) {
                dived_prev[i] = true;
              } else {
                parNodeRlxr[i]->reset(current_node[i], false);
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
                {
                  new_node[i] = tm_->getCandidate(); // Can be NULL. The
                  // branches that were created could have large lb and tm
                  // might have eliminated them.
                  if(new_node[i]) {
                    tm_->removeActiveNode(new_node[i]);
                  }
                  dived_prev[i] = false;
                }
              }
            }
            current_node[i] = new_node[i];
          } // if (current_node[i]) ends
          //stopping condition at each thread
          nodeCountTh[i] = 0;
#if USE_OPENMP
#pragma omp critical (treeManager)
#endif
          treeLbTh[i] = tm_->updateLb();
          minNodeLbTh[i] = INFINITY;

          for (UInt j=0; j < numThreads; ++j) {
            if (current_node[j]) {
              nodeLbTh[i] = current_node[j]->getLb(); 
              if (nodeLbTh[i] < minNodeLbTh[i])
              {
                minNodeLbTh[i] = nodeLbTh[i];
              }
              nodeCountTh[i]++;
            } 
          }

          if (minNodeLbTh[i] < treeLbTh[i]) {
            treeLbTh[i] = minNodeLbTh[i];
          }
#if USE_OPENMP
#pragma omp critical
#endif
          {
            showParStatus_(nodeCountTh[i], treeLbTh[i], wallTimeStart);
            if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
              tm_->updateLb();
              shouldRunTh[i] = false;
            }
          }
          if(iterMode == true) shouldRunTh[i] = false;
        } //internal while ends
      } //parallel for end
#if USE_OPENMP
#pragma omp single
#endif
      { 
        iterCount++;
        nodeCount = 0;
        treeLb = tm_->updateLb();
        minNodeLb = INFINITY;
        for (UInt j = 0; j < numThreads; ++j) {
          if (current_node[j]) {
            nodeLb = current_node[j]->getLb(); 
            if (nodeLb < minNodeLb)
            {
              minNodeLb = nodeLb;
            }
            nodeCount++;
          }
        }
        if (minNodeLb < treeLb) {
          treeLb = minNodeLb;
        }

        showParStatus_(nodeCount, treeLb, wallTimeStart);

        // update stopping conditions
        if (nodeCount == 0) {
          tm_->updateLb();
          if (tm_->getUb() <= -INFINITY) {
            status_ = SolvedUnbounded;
          } else if (tm_->getUb() < INFINITY) {
            status_ = SolvedOptimal; // TODO: get the right status
          } else {
            status_ = SolvedInfeasible; // TODO: get the right status
          }
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "all nodes have "
            << "been processed" << std::endl;
#endif
        } else if (shouldStopPar_(wallTimeStart, treeLb)) {
          tm_->updateLb();
          shouldRun = false;
        } else {
#if SPEW
          logger_->msgStream(LogDebug) << std::setprecision(8)
            << me_ << "lb = " << tm_->updateLb() << std::endl 
            << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
        }
      } //omp master/single ended
    }   //parallel region ends
  }     //while ends
  logger_->msgStream(LogInfo) << me_ << "stopping branch-and-bound"
    << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  if (iterMode) {
    logger_->msgStream(LogInfo) << me_ << "iterations = " << iterCount
      << std::endl;
  }

  stats_->timeUsed = timer_->query();
  timer_->stop();
  
  for (UInt i=0; i<numThreads; ++i) {
    logger_->msgStream(LogInfo) << me_ << "cuts added by thread " << i <<": "<<numCuts[i] <<"\n";
  }

  delete[] should_dive;
  delete[] dived_prev;
  delete[] should_prune;
  delete[] initialized;
  delete[] current_node;
  delete[] new_node;
  delete[] nodeCountTh;
  delete[] treeLbTh;
  delete[] nodeLbTh;
  delete[] minNodeLbTh;
  delete[] shouldRunTh;
  delete[] ws;
  delete[] rel;
  delete[] branches;
  delete[] numCuts;
  delete[] cutsIndex;
  //delete[] cutman;
}

void ParQGBranchAndBound::writeStats(std::ostream &out)
{
  out << me_ << "time taken      = " << std::fixed << std::setprecision(2)
    << stats_->timeUsed << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  nodePrcssr_->writeStats(out);
  nodePrcssr_->getBrancher()->writeStats(out);
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->writeStats(out);
  }
  solPool_->writeStats(out);
}

void ParQGBranchAndBound::writeParStats(std::ostream &out, ParPCBProcessorPtr nodePrcssr[])
{
  out << me_ << "time taken      = " << std::fixed << std::setprecision(2)
    << stats_->timeUsed << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  //Amend code below when mcqg statistics are finalized: to be done!!!
  nodePrcssr[0]->writeStats(out);
  nodePrcssr[0]->getBrancher()->writeStats(out);
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->writeStats(out);
  }
  solPool_->writeStats(out);
}

double ParQGBranchAndBound::totalTime()
{
  return stats_->timeUsed;
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

  ParQGBabStats::ParQGBabStats()
:nodesProc(0),
  timeUsed(0),
  updateTime(0)
{
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
  ParQGBabOptions::ParQGBabOptions()
: createRoot(true),
  logLevel(LogInfo),
  nodeLimit(0),
  perGapLimit(0.),
  solLimit(0),
  timeLimit(0.)

{
}


ParQGBabOptions::ParQGBabOptions(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();

  logInterval = options->findDouble("bnb_log_interval")->getValue();
  logLevel    = (LogLevel) options->findInt("log_level")->getValue();
  nodeLimit   = options->findInt("bnb_node_limit")->getValue();
  perGapLimit = options->findDouble("obj_gap_percent")->getValue();
  solLimit    = options->findInt("bnb_sol_limit")->getValue();
  timeLimit   = options->findDouble("bnb_time_limit")->getValue();
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
