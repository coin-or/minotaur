// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
// 

/**
 * \file ParBranchAndBound.cpp
 * \brief Define ParBranchAndBound class for a generic parallel branch-and-bound method.
 * \author Prashant Palkar, IIT Bombay
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
#include "Environment.h"
#include "Heuristic.h"
#include "Logger.h"
#include "Node.h"
#include "NodeProcessor.h"
#include "NodeRelaxer.h"
#include "Option.h"
#include "ParBndProcessor.h"
#include "ParBranchAndBound.h"
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

const std::string ParBranchAndBound::me_ = "branch-and-bound: ";

ParBranchAndBound::ParBranchAndBound()
  : env_(EnvPtr()),             // NULL
  nodePrcssr_(),                // NULL
  nodeRlxr_(NodeRelaxerPtr()),  // NULL
  options_(ParBabOptionsPtr()), // NULL
  problem_(ProblemPtr()),       // NULL
  solPool_(SolutionPoolPtr()),  // NULL
  stats_(0),
  status_(NotStarted),
  timer_(0),                    // NULL
  tm_(ParTreeManagerPtr())      // NULL
{
}


ParBranchAndBound::ParBranchAndBound(EnvPtr env, ProblemPtr p)
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
  options_ = (ParBabOptionsPtr) new ParBabOptions(env);
  logger_ = (LoggerPtr) new Logger(options_->logLevel);
}


ParBranchAndBound::~ParBranchAndBound()
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


void ParBranchAndBound::addPreRootHeur(HeurPtr h)
{
  preHeurs_.push_back(h);
}


double ParBranchAndBound::getPerGap() 
{ 
  return tm_->getPerGap(); 
}


double ParBranchAndBound::getLb() 
{
  return tm_->getLb();
}


NodeProcessorPtr ParBranchAndBound::getNodeProcessor()
{
  return nodePrcssr_;
}


NodeRelaxerPtr ParBranchAndBound::getNodeRelaxer()
{
  return nodeRlxr_;
}


SolutionPtr ParBranchAndBound::getSolution()
{
  return solPool_->getBestSolution();
}


SolveStatus ParBranchAndBound::getStatus()
{
  return status_;
}


ParTreeManagerPtr ParBranchAndBound::getTreeManager() 
{
  return tm_;
}


double ParBranchAndBound::getUb() 
{
  return tm_->getUb();
}


std::vector<std::vector<double> > ParBranchAndBound::mapSerialOutput (
  std::vector<std::vector<double> >serVec,
  std::vector<std::vector<double> >parVec)

{
  for (std::vector<int>::size_type p = 0; p < parVec.size(); p++) {
    if (parVec[p][0] == 1 || parVec[p][0] == 2)
      parVec[p].push_back(parVec[p][0]); //nodes 1 and 2 are children of node 0
    else
      parVec[p].push_back(-1);
  }
  std::vector<std::vector<int> >serNotAsg;
  std::vector<int>sertemp;
  bool assigned = false;
  int serMaxNode = 0;
  for (std::vector<int>::size_type i = 0; i < serVec.size(); i++) {
    if (serMaxNode < serVec[i][0])
      serMaxNode = serVec[i][0];
  }
  for (std::vector<int>::size_type s = 1; s < serVec.size() - 1; s++) {
    for (std::vector<int>::size_type p = 0; p < parVec.size(); p++) {
      if (serVec[s][0] == 2 || serVec[s][0] == 1) {
        assigned = true;
        break;
      }
      //locate parent node in new list
      if(serVec[s][1] == parVec[p][parVec[p].size()-1]) {
        for (std::vector<int>::size_type q = 0; q < parVec.size(); q++) {
          if(parVec[p][0] == parVec[q][1]) {
            if(serVec[s][2] == parVec[q][2]) { //if branching var is the same
              if(serVec[s][0] == parVec[q][0]  //if ids are the same
                 //or if children are even
                 || ( (int)serVec[s][0]%2 == 0 && (int)parVec[q][0]%2 == 0 )
                 //or if children are odd, assign id
                 || ( (int)serVec[s][0]%2 == 1 && (int)parVec[q][0]%2 == 1) ) {
                parVec[q][parVec[q].size()-1] = serVec[s][0];
                assigned = true;
                break;
              }
            }
          }
        }
        if (assigned) break;
      }
    }
    if(!assigned) {
      sertemp.push_back(serVec[s][0]);
      sertemp.push_back(serVec[s][2]);
      serNotAsg.push_back(sertemp);
      sertemp.clear();
    } else assigned = false;
  }
  std::cout<<std::endl;
  if(serNotAsg.size()>0) {
    std::cout <<"\nSerial nodes not mapped\n" << "id  brVar\n";
    print2dvec(serNotAsg);
  }
  std::cout <<"\nParallel nodes not mapped\n" << "id  brVar thread\n";
  for (std::vector<int>::size_type p = 0; p < parVec.size(); p++) {
    if(parVec[p][parVec[p].size()-1] == -1)
      std::cout << parVec[p][0] << " " << parVec[p][2] << " "<< parVec[p][4]
        << std::endl;
  }
  std::cout<<std::endl;
  return parVec;
}


UInt ParBranchAndBound::numProcNodes()
{
  return stats_->nodesProc;
}


void ParBranchAndBound::print2dvec(std::vector<std::vector<double> > output)
{
   std::cout << std::endl;
   for (std::vector<std::vector<int> >::size_type i=0; i<output.size(); i++) {
      for (std::vector<int>::size_type j = 0; j < output[i].size(); j++) {
        if (j<3 || j==4 || j ==7 || j == 8)
          std::cout << (int)output[i][j] << ' ';
        else
          std::cout << output[i][j] << ' ';
      }
      std::cout << std::endl;
   }
   std::cout << std::endl;
}


void ParBranchAndBound::print2dvec(std::vector<std::vector<int> > output)
{
   std::cout << std::endl;
   for (std::vector<std::vector<int> >::size_type i=0; i<output.size(); i++) {
      for (std::vector<int>::size_type j = 0; j < output[i].size(); j++) {
        if (j<3 || j==4 || j ==7)
          std::cout << (int)output[i][j] << ' ';
        else
          std::cout << output[i][j] << ' ';
      }
      std::cout << std::endl;
   }
   std::cout << std::endl;
}


NodePtr ParBranchAndBound::processRoot_(bool *should_prune, bool *should_dive,
                                        ParNodeIncRelaxerPtr parNodeRlxr0,
                                        ParBndProcessorPtr nodePrcssr0,
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
    }
  }
  current_node = new_node;

  tm_->updateLb();

  showStatus_(*should_dive);
  *should_prune = prune;
  return current_node;
}

std::vector<std::vector<double> > ParBranchAndBound::
readSerialOutput(const char* myFile)
{
  int numFields = 9;
  std::vector<std::vector<double> > data;
  std::ifstream myReadFile;
  myReadFile.open(myFile);
  while (!myReadFile.eof()) {
    for(int i = 0; i < 1; i++) {
      std::vector<double> tmpVec;
      double temp;
      for (int j = 0; j < numFields; j++) {
        myReadFile >> temp;
        tmpVec.push_back(temp);
      }
      data.push_back(tmpVec);
    }
  }
  return data;
}


void ParBranchAndBound::setLogLevel(LogLevel level) 
{
  logger_->setMaxLevel(level);
}


void ParBranchAndBound::setNodeProcessor(NodeProcessorPtr p)
{
  nodePrcssr_ = p;
}


void ParBranchAndBound::setNodeRelaxer(NodeRelaxerPtr nr)
{
  nodeRlxr_ = nr;
}


void ParBranchAndBound::shouldCreateRoot(bool b)
{
  options_->createRoot = b;
}


bool ParBranchAndBound::shouldPrune_(NodePtr node)
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


bool ParBranchAndBound::shouldStop_()
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


bool ParBranchAndBound::shouldStopPar_(double WallTimeStart, double treeLb)
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


void ParBranchAndBound::showStatus_(bool current_uncounted)
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


void ParBranchAndBound::showParStatus_(UInt off, double treeLb,
                                       double WallTimeStart)
{
  if (timer_->query()-stats_->updateTime > options_->logInterval) {
    //double lb = tm_->updateLb();
    logger_->msgStream(LogInfo) 
      << me_ 
      << std::fixed
      //<< std::setprecision(1)  << "time = "            << timer_->query()
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


int ParBranchAndBound::strToInt(std::string str)
{
    std::string temp;
    int number=0;
    for (unsigned int i=0; i < str.size(); i++)
    {
        //iterate the string to find the first "number" character
        //if found create another loop to extract it
        //and then break the current one
        //thus extracting the FIRST encountered numeric block
        if (isdigit(str[i]))
        {
            for (unsigned int a=i; a<str.size(); a++)
            {
                temp += str[a];
            }
          //the first numeric block is extracted
          break;
        }
    }
    std::istringstream stream(temp);
    stream >> number;
    return number;
}


void ParBranchAndBound::parsolve(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParBndProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  double wallTimeStart = getWallTime();
  bool *should_dive = new bool[numThreads];
  bool *dived_prev = new bool[numThreads];
  bool *should_prune = new bool[numThreads];
  bool *initialized = new bool[numThreads];
  NodePtr *current_node = new NodePtr[numThreads];
  NodePtr *new_node = new NodePtr[numThreads];
  Branches branches;
  WarmStartPtr *ws = new WarmStartPtr[numThreads];
  RelaxationPtr *rel = new RelaxationPtr[numThreads];
  UInt nodeCount;
  double treeLb, nodeLb, minNodeLb;
  double *treeLbTh = new double[numThreads];
  double *nodeLbTh = new double[numThreads];
  double *minNodeLbTh = new double[numThreads];
  bool *shouldRunTh = new bool[numThreads];
  UInt *nodeCountTh = new UInt[numThreads];
  bool iterMode = env_->getOptions()->findBool("mcbnb_iter_mode")->getValue();
  UInt iterCount = 1;
#if 0
  UInt timeCount = 0;
#endif
#if PRINT
  const char* inputFile = "1alan";      //serial bnb tree input filename
  std::vector<std::vector<double> > serialOutput = readSerialOutput(inputFile);
  std::vector<std::vector<double> > parallelOutput;
  std::vector<double> *tmp = new std::vector<double> [numThreads];
#endif

#if USE_OPENMP
#pragma omp parallel for
#endif
  for(UInt i = 0; i < numThreads; i++) {
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
  stats_ = new ParBabStats();

  // initialize solution pool
  // TODO: use user options to set the pool size. For now it is 1.
  solPool_ = (SolutionPoolPtr) new SolutionPool(env_, problem_, 1);

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
      for(UInt i = 0; i < numThreads; i++) {
        if(iterMode == true) shouldRunTh[i] = true;
        while(nodeCountTh[i] > 0 && shouldRunTh[i])
        {
          if(!current_node[i]) {
#if USE_OPENMP
#pragma omp critical
#endif
            {
              current_node[i] = tm_->getCandidate();
              if(current_node[i]) {
                tm_->removeActiveNode(current_node[i]);
                dived_prev[i] = false;
              }
            }
          }
          if(current_node[i]) { 
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
#if PRINT
            tmp[i].push_back(current_node[i]->getId());
            tmp[i].push_back(current_node[i]->getParent()->getId());
            tmp[i].push_back(strToInt(current_node[i]->getBranch()->getBrCand()->getName()));
            tmp[i].push_back(current_node[i]->getLb());
            tmp[i].push_back(i);
#if USE_OPENMP
#pragma omp critical
#endif
            {
              tmp[i].push_back(getWallTime() - wallTimeStart);
            }
#endif
            nodePrcssr[i]->process(current_node[i], rel[i], solPool_, initialized[i]);
#if PRINT
#if USE_OPENMP
#pragma omp critical
#endif
            {
              tmp[i].push_back(getWallTime() - wallTimeStart);
            }
            tmp[i].push_back(current_node[i]->getTbScore());
#endif
#if USE_OPENMP
#pragma omp critical
#endif
            ++stats_->nodesProc;

#if SPEW
            logger_->msgStream(LogDebug1) << me_ << "node lower bound = " <<
              current_node[i]->getLb() << std::endl;
#endif
#if USE_OPENMP
#pragma omp critical
#endif
            {
              if (nodePrcssr[i]->foundNewSolution()) { 
                tm_->setUb(solPool_->getBestSolutionValue());
              }
            }
#if USE_OPENMP
#pragma omp critical
#endif
            should_prune[i] = shouldPrune_(current_node[i]);

            if (should_prune[i]) {
#if SPEW
              logger_->msgStream(LogDebug1) << me_ << "node pruned" << 
                std::endl;
#endif
              parNodeRlxr[i]->reset(current_node[i], false);
#if USE_OPENMP
#pragma omp critical
#endif
              {
#if PRINT
                tmp[i].push_back(current_node[i]->getStatus());
                if(current_node[i]->getStatus()==NodeOptimal)
                {
                  tmp[i].push_back(tm_->getUb());
                }
                else
                  tmp[i].push_back(-1);
#endif
                tm_->pruneNode(current_node[i]);
              }
#if USE_OPENMP
#pragma omp critical
#endif
              {
                new_node[i] = tm_->getCandidate();
                if(new_node[i]) {
                  tm_->removeActiveNode(new_node[i]);
                }
              }
              dived_prev[i] = false;

            } else {
#if PRINT
              tmp[i].push_back(-1);     //prune status
              tmp[i].push_back(-1);     //feasible solution/ub
#endif
              initialized[i] = true;
#if SPEW
              logger_->msgStream(LogDebug1) << me_ << "branching" << 
                std::endl;
#endif
#if USE_OPENMP
#pragma omp critical
#endif
              branches = nodePrcssr[i]->getBranches();

              ws[i] = nodePrcssr[i]->getWarmStart();
              //if (!dived_prev[i]) {
              //tm_->removeActiveNode(current_node[i]);
              //}
              should_dive[i] = tm_->shouldDive();
              if (!branches) {
                std::cout<<" NO BRANCHES \n";
              }
#if USE_OPENMP
#pragma omp critical
#endif
              {
                new_node[i] = tm_->branch(branches, current_node[i], ws[i]);
              }
              assert((should_dive[i] && new_node[i])
                     || (!should_dive[i] && !new_node[i]));
              if (should_dive[i]) {
                dived_prev[i] = true;
              } else {
                parNodeRlxr[i]->reset(current_node[i], false);
#if USE_OPENMP
#pragma omp critical
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
          treeLbTh[i] = tm_->updateLb();
          minNodeLbTh[i] = INFINITY;
#if 0
#if USE_OPENMP
#pragma omp critical
#endif
          if(i==numThreads-1 && (getWallTime()-wallTimeStart) > timeCount) {
            std::cout<<" Idle threads:";
          }
#endif
          for (UInt j = 0; j < numThreads; j++) {
            if (current_node[j]) {
              nodeLbTh[i] = current_node[j]->getLb(); 
              if (nodeLbTh[i] < minNodeLbTh[i])
              {
                minNodeLbTh[i] = nodeLbTh[i];
              }
              nodeCountTh[i]++;
            } 
#if 0
#if USE_OPENMP
#pragma omp critical
#endif
            else {
              if(i==numThreads-1 && (getWallTime()-wallTimeStart) > timeCount) {
                std::cout<<" "<<j;
              }
            }
#endif
          }
#if 0
#if USE_OPENMP
#pragma omp critical
#endif
          if(i==numThreads-1 && (getWallTime()-wallTimeStart) > timeCount) {
            std::cout<<std::endl; timeCount++;}
#endif
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
#if USE_OPENMP
#pragma omp critical
#endif
          {
#if PRINT
            if(!tmp[i].empty())
            {
              parallelOutput.push_back(tmp[i]);
              tmp[i].clear();
            }
#endif
          }
          if(iterMode == true) shouldRunTh[i] = false;
        } //internal while ends
      } //parallel for end
#if USE_OPENMP
#pragma omp single
#endif
      { 
#if 0
        {
          std::cout<<"Started checking stopping conditions "
            << " at thread " << omp_get_thread_num()
            << " at time " << getWallTime() - wallTimeStart<<"\n";
        }
#endif
        iterCount++;
        nodeCount = 0;
        treeLb = tm_->updateLb();
        minNodeLb = INFINITY;
        for (UInt j = 0; j < numThreads; j++) {
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
#if 0
        std::cout<< "# nodes in process: "<< nodeCount <<"; # active nodes: "
          << numActiveNodes << std::endl;
#endif
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
#if PRINT
  print2dvec(serialOutput);
  print2dvec(parallelOutput);
  std::vector<std::vector<double> > finalParOutput = mapSerialOutput(serialOutput, parallelOutput);
  print2dvec(finalParOutput);
#endif
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
#if PRINT
  delete[] tmp;
#endif
}

void ParBranchAndBound::writeStats(std::ostream &out)
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

void ParBranchAndBound::writeParStats(std::ostream &out, ParBndProcessorPtr nodePrcssr[])
{
  out << me_ << "time taken      = " << std::fixed << std::setprecision(2)
    << stats_->timeUsed << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  //Amend code below when mcbnb statistics are finalized: to be done!!!
  nodePrcssr[0]->writeStats(out);
  nodePrcssr[0]->getBrancher()->writeStats(out);
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->writeStats(out);
  }
  solPool_->writeStats(out);
}

double ParBranchAndBound::totalTime()
{
  return stats_->timeUsed;
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

  ParBabStats::ParBabStats()
:nodesProc(0),
  timeUsed(0),
  updateTime(0)
{
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
  ParBabOptions::ParBabOptions()
: createRoot(true),
  logLevel(LogInfo),
  nodeLimit(0),
  perGapLimit(0.),
  solLimit(0),
  timeLimit(0.)

{
}


ParBabOptions::ParBabOptions(EnvPtr env)
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
