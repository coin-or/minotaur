// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
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
#else
#error "Cannot compile parallel algorithms: turn USE_OpenMP flag ON."
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
#include "ParPCBProcessor.h"
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
#include "WarmStart.h"

//#define SPEW 1
//#define PRINT 0

using namespace Minotaur;

const std::string ParBranchAndBound::me_ = "ParBranchAndBound: ";

ParBranchAndBound::ParBranchAndBound()
  : env_(0),
    nodePrcssr_(),
    nodeRlxr_(0),
    options_(0),
    problem_(0),
    solPool_(0),
    stats_(0),
    status_(NotStarted),
    timer_(0),
    tm_(0)
{
}


ParBranchAndBound::ParBranchAndBound(EnvPtr env, ProblemPtr p)
  : env_(env),
    nodePrcssr_(0),
    nodeRlxr_(0),
    problem_(p),
    solPool_(0),
    stats_(0),
    status_(NotStarted)
{
  timer_ = env->getNewTimer();
  tm_ = (ParTreeManagerPtr) new ParTreeManager(env);
  options_ = (ParBabOptionsPtr) new ParBabOptions(env);
  logger_ = env->getLogger();
}


ParBranchAndBound::~ParBranchAndBound()
{
  problem_ = 0;
  env_ = 0;
  nodeRlxr_ = 0;
  nodePrcssr_ = 0;
  if (options_) {
    delete options_;
  }
  if (solPool_) {
    delete solPool_;
  }
  if (stats_) {
    delete stats_;
  }
  if (timer_) {
    delete timer_;
  }
  if (tm_) {
    delete tm_;
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


void ParBranchAndBound::processRoot_(bool *should_prune, bool *should_dive,
                                        ParNodeIncRelaxerPtr parNodeRlxr0,
                                        ParPCBProcessorPtr nodePrcssr0,
                                        WarmStartPtr ws0, NodePtr &current_node)
{
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
    rel = parNodeRlxr0->createRootRelaxation(current_node, solPool_, prune);
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
    tm_->removeActiveNode(current_node);
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
    if (env_->getOptions()->findString("tb_rule")->getValue() == "twochild") {
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
}

std::vector<std::vector<double> > ParBranchAndBound::
readSerialOutput(const char* myFile)
{
  int numFields = 9;
  std::vector<std::vector<double> > data;
  std::ifstream myReadFile;
  myReadFile.open(myFile);
  while (!myReadFile.eof()) {
    for (int i = 0; i < 1; i++) {
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
    break;
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


// void ParBranchAndBound::showParStatus_(UInt off, double treeLb,
//                                        double WallTimeStart, UInt i)
// {
//   if (timer_->query()-stats_->updateTime > options_->logInterval) {
//     logger_->msgStream(LogInfo) 
//       << me_ 
//       << std::fixed
//       << std::setprecision(1)  << "time PAR= " << getWallTime() - WallTimeStart
//       << std::setprecision(4)  << " lb = "  << treeLb
//       << std::setprecision(4)  << " ub = "  << tm_->getUb()
//       << std::setprecision(2)  << " gap% = " << tm_->getPerGapPar(treeLb)
//       << " nodes processed = " << tm_->getSize()-tm_->getActiveNodes()-off 
//       << " left = " << tm_->getActiveNodes()+off
//       << " thread " << i
//       << std::endl;
//     stats_->updateTime = timer_->query();
//   }
// }
//updating showParstatus
void ParBranchAndBound::showParStatus_(UInt off, double treeLb,
                                      double /*WallTimeStart*/, UInt /*i*/)
{
 static bool header = false; // Ensure the header for the log table is printed only once
 static bool firstRow = true; // Ensure the initial row with default values is printed only once


 if (timer_->query() - stats_->updateTime > options_->logInterval) {
   if (!header) {
     logger_->msgStream(LogInfo) << " " << std::endl;
     logger_->msgStream(LogInfo)
         << "---------------------------------------------------------------------------------------" << std::endl;
     logger_->msgStream(LogInfo)
         << std::setw(7) << "CPU(s)" << std::setw(10) << "Wall(s)"
         << std::setw(16) << "LB" << std::setw(13) << "UB" << std::setw(12)
         << "Gap%" << std::setw(15) << "   Nodes-Proc" << std::setw(14)
         << "   Nodes-Rem" << std::endl;
     logger_->msgStream(LogInfo)
         << "---------------------------------------------------------------------------------------" << std::endl;
     header = true;
   }


   if (firstRow) {
     // Print the initial row with default values
     logger_->msgStream(LogInfo)
         << std::setw(6) << "0.0"
         << std::setw(10) << "0.0"
         << std::setw(17) << "-inf"
         << std::setw(13) << std::setprecision(4) << std::scientific << tm_->getUb()
         << std::setw(12) << "inf"
         << std::setw(15) << "0"
         << std::setw(14) << "0"
         << std::endl;
     firstRow = false;
   }


   logger_->msgStream(LogInfo)
      //  << std::setw(6) << i
      //   << std::setw(10) << std::fixed << std::setprecision(1) << getWallTime() - WallTimeStart
      << std::setw(6)  << std::fixed << std::setprecision(1) << timer_->query()
       << std::setw(10) << std::fixed << std::setprecision(1) << timer_->wQuery()
       << std::setw(17) << std::setprecision(4) << std::scientific << treeLb
       << std::setw(13) << std::setprecision(4) << std::scientific << tm_->getUb()
       << std::setw(12) << std::setprecision(2) << std::scientific << tm_->getPerGapPar(treeLb)
       << std::setw(15) << tm_->getSize() - tm_->getActiveNodes() - off
       << std::setw(14) << tm_->getActiveNodes() + off
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


void ParBranchAndBound::parsolveOppor(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  double wallTimeStart = getWallTime();
  bool *should_dive = new bool[numThreads];
  bool *dived_prev = new bool[numThreads];
  bool *should_prune = new bool[numThreads];
  bool *initialized = new bool[numThreads];
  NodePtr *current_node = new NodePtr[numThreads]();
  NodePtr *new_node = new NodePtr[numThreads]();
  Branches *branches = new Branches[numThreads]();
  WarmStartPtr *ws = new WarmStartPtr[numThreads]();
  RelaxationPtr *rel = new RelaxationPtr[numThreads];
  double *treeLbTh = new double[numThreads];
  double *nodeLbTh = new double[numThreads];
  double *minNodeLbTh = new double[numThreads];
  UInt *nodeCountTh = new UInt[numThreads];
  UInt *nodesProcTh = new UInt[numThreads];
  UInt numVars = 0;
  bool shouldRun = true;

  omp_set_num_threads(numThreads);
//#pragma omp parallel for
  for (UInt i = 0; i < numThreads; ++i) {
    should_dive[i] = false;
    dived_prev[i] = false;
    should_prune[i] = false;
    initialized[i] = false;
    nodeCountTh[i] = 1;
    nodesProcTh[i] = 0;
  }

  // initialize timer
  timer_->start();

  logger_->msgStream(LogInfo) << me_ << "starting branch-and-bound ";
  if (numThreads > 1) {
    logger_->msgStream(LogInfo) << "using " << numThreads << " out of "
      << omp_get_num_procs() << " processors";
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

  rel[0] = parNodeRlxr[0]->getRelaxation();

  // call heuristics before the root, if needed 
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->solve(current_node[0], rel[0], solPool_);
  }
  tm_->setUb(solPool_->getBestSolutionValue());

  // do the root
  current_node[0] = (NodePtr) new Node ();
  processRoot_(&should_prune[0], &dived_prev[0], parNodeRlxr[0],
                  nodePrcssr[0], ws[0], current_node[0]);
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
    shouldRun = false;
  } else if (shouldStopPar_(wallTimeStart, tm_->getLb())) {
    tm_->updateLb();
  } else {
#if SPEW
    logger_->msgStream(LogDebug) << std::setprecision(8)
      << me_ << "lb = " << tm_->updateLb() << std::endl
      << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
  }

  // solve root outside the loop. save the useful information.
  initialized[0] = true; //pseudoCosts for thread0 initialized while doing root
  numVars = rel[0]->getNumVars();
  bool isParRel = false;
  if (nodePrcssr[0]->getBrancher()->getName() == "ParReliabilityBrancher") {
    isParRel = true;
  }

  //bool notRampedUp = true;
  UInt i=0; // thread id
#pragma omp parallel private(i)
  {
    i = omp_get_thread_num();
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
 
    while (nodeCountTh[i] > 0 && shouldRun) {
      if (current_node[i]) {
#pragma omp critical (treeManager)
        {
          if (tm_->shouldPrune_(current_node[i])) {
            parNodeRlxr[i]->reset(current_node[i], false);
#if SPEW
            logger_->msgStream(LogInfo) << me_ << "prune node "
              << current_node[i]->getId() << " thread "
              << omp_get_thread_num() << std::endl;
#endif
            tm_->pruneNode(current_node[i]);
#pragma omp critical (current_node)
            current_node[i] = NodePtr();
          }
        }
      } else {
#pragma omp critical (treeManager)
        {
          current_node[i] = tm_->getCandidate();
          if (current_node[i]) {
            tm_->removeActiveNode(current_node[i]);
          }
        }
        if (current_node[i]) {
          nodesProcTh[i]++;
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug) << me_ << "get node "
            << current_node[i]->getId() << " thread "
            << omp_get_thread_num() << std::endl;
#endif
        }
        dived_prev[i] = false;
      }
      if (current_node[i]) {
#if SPEW
#pragma omp critical (logger)
        logger_->msgStream(LogInfo) << me_ << "process node "
          << current_node[i]->getId() << " thread " << omp_get_thread_num() << std::endl;
          //<< me_ << "depth = " << current_node[i]->getDepth() << std::endl
          //<< me_ << "did we dive = " << dived_prev[i] << std::endl;
#endif
        should_dive[i] = false;

        rel[i] = parNodeRlxr[i]->createNodeRelaxation(current_node[i],
                                                      dived_prev[i],
                                                      should_prune[i]);
        if (isParRel) {
          for (UInt j = 0; j < numThreads; ++j) {
            if (i!=j) {
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
#pragma omp critical (stats)
        {
          ++stats_->nodesProc;
        }
#if SPEW
#pragma omp critical (logger)
        logger_->msgStream(LogDebug1) << me_ << "node " 
          << current_node[i]->getId() << " lower bound = "
          << current_node[i]->getLb() << " thread " 
          << omp_get_thread_num() << std::endl;
#endif
        if (nodePrcssr[i]->foundNewSolution()) {
#pragma omp critical (treeManager)
          {
            tm_->setUb(solPool_->getBestSolutionValue());
          }
        }
        should_prune[i] = shouldPrune_(current_node[i]);

        if (should_prune[i]) {
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogInfo) << me_ << "prune node "
            << current_node[i]->getId() << " thread "
            << omp_get_thread_num() << std::endl;
#endif
          parNodeRlxr[i]->reset(current_node[i], false);
#pragma omp critical (treeManager)
          {
            tm_->pruneNode(current_node[i]);
          }
#pragma omp critical (current_node)
          current_node[i] = NodePtr();
#pragma omp critical (treeManager)
          {
            new_node[i] = tm_->getCandidate();
            if (new_node[i]) {
              //getting and removing node must be in the same critical
              //block otherwise some other thread might take the same node
              tm_->removeActiveNode(new_node[i]);
              nodesProcTh[i]++;
#if SPEW
#pragma omp critical (logger)
              logger_->msgStream(LogDebug) << me_ << "get node "
                << new_node[i]->getId() << " (prune) thread "
                << omp_get_thread_num() << std::endl;
#endif
            }
          }
          dived_prev[i] = false;
        } else {
          initialized[i] = true;
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug) << me_ << "branch at node "
            << current_node[i]->getId() << " thread "
            << omp_get_thread_num() << std::endl;
#endif
          branches[i] = nodePrcssr[i]->getBranches();

          ws[i] = nodePrcssr[i]->getWarmStart();

          should_dive[i] = tm_->shouldDive();
          if (!branches[i]) {
            logger_->msgStream(LogDebug) << " NO BRANCHES \n";
          }
#pragma omp critical (treeManager)
          {
#pragma omp critical (current_node)
            new_node[i] = tm_->branch(branches[i], current_node[i], ws[i]);
#if SPEW
#pragma omp critical (logger)
            logger_->msgStream(LogDebug) << me_ << "get node "
              << new_node[i]->getId() << " (branch) thread " << omp_get_thread_num()
              << std::endl;
#endif
          }
          assert((should_dive[i] && new_node[i])
                 || (!should_dive[i] && !new_node[i]));
          if (should_dive[i]) {
            dived_prev[i] = true;
          } else {
            parNodeRlxr[i]->reset(current_node[i], false);
#pragma omp critical (treeManager)
            {
              new_node[i] = tm_->getCandidate(); // Can be NULL. The
              // branches that were created could have large lb and tm
              // might have eliminated them.
              if (new_node[i]) {
                tm_->removeActiveNode(new_node[i]);
#if SPEW
#pragma omp critical (logger)
                logger_->msgStream(LogDebug) << me_ << "get/remove node "
                  << new_node[i]->getId() << " thread "
                  << omp_get_thread_num() << std::endl;
#endif
              }
              dived_prev[i] = false;
            }
          }
        }
#pragma omp critical (current_node)
        current_node[i] = new_node[i];
      } // if (current_node[i]) ends
      //update lower bound
#pragma omp critical (treeManager)
      {
        treeLbTh[i] = tm_->updateLb();
      }
      minNodeLbTh[i] = INFINITY;
      for (UInt j=0; j < numThreads; ++j) {
#pragma omp critical (current_node)
        {
          if (current_node[j]) {
            nodeLbTh[i] = current_node[j]->getLb();
          }
        }
        if (current_node[j]) {
          if (nodeLbTh[i] < minNodeLbTh[i]) {
            minNodeLbTh[i] = nodeLbTh[i];
          }
        }
      }
      if (minNodeLbTh[i] < treeLbTh[i]) {
        treeLbTh[i] = minNodeLbTh[i];
      }
      //stopping condition at each thread
      nodeCountTh[i] = tm_->anyActiveNodesLeft();
      if (nodeCountTh[i] == 0) {
        for (UInt j=0; j < numThreads; ++j) {
          if (current_node[j]) {
#pragma omp atomic
            nodeCountTh[i]++;
            break;
          }
        }
      }
#pragma omp critical (logger)
      {
        showParStatus_(nodeCountTh[i], treeLbTh[i], wallTimeStart, i);
      }
      if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
#pragma omp critical (treeManager)
        {
          tm_->updateLb();
        }
      }

      // update stopping conditions
      if (nodeCountTh[i] == 0) {
        tm_->updateLb();
        if (tm_->getUb() <= -INFINITY) {
          status_ = SolvedUnbounded;
        } else if (tm_->getUb() < INFINITY) {
          status_ = SolvedOptimal; // TODO: get the right status
        } else {
          status_ = SolvedInfeasible; // TODO: get the right status
        }
#if SPEW
#pragma omp single
        logger_->msgStream(LogDebug) << me_ << "all nodes have "
          << "been processed" << " thread " << i << std::endl;
#endif
      } else if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
        tm_->updateLb();
        shouldRun = false;
      //} else if (notRampedUp && (nodeCount == numThreads)) {
        //tm_->updateLb();
        //notRampedUp = false;
//#pragma omp critical (logger)
        //logger_->msgStream(LogExtraInfo) << me_
          //<< "ramp-up time = "
          //<< getWallTime() - wallTimeStart << std::endl;
        ////shouldRun = false;
      } else {
#if SPEW
#pragma omp critical (logger)
        //logger_->msgStream(LogInfo) << "nodesCount " << nodeCountThread << " thread " << i << std::endl;
        logger_->msgStream(LogDebug) << std::setprecision(8)
          << me_ << "lb = " << tm_->updateLb() << std::endl
          << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
      }
    } //while ends
#if SPEW
#pragma omp single
    {
      for (UInt j=0; j < numThreads; ++j) {
        logger_->msgStream(LogInfo) << "nodesProc " << nodesProcTh[j] << " thread " << j << std::endl;
      }
    }
#endif
  }   //parallel region ends
  logger_->msgStream(LogExtraInfo) << me_ << "stopping branch-and-bound"
    << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  solPool_->writeStats(logger_->msgStream(LogExtraInfo));

  for (UInt k = 0; k < numThreads; ++k) {
    logger_->msgStream(LogExtraInfo) << me_ << "nodes processed by thread "
      << k << " = " << nodesProcTh[k] << std::endl;
  }

  stats_->timeUsed = timer_->query();
  timer_->stop();

  delete[] should_dive;
  delete[] dived_prev;
  delete[] should_prune;
  delete[] initialized;
  for (UInt j=0; j < numThreads; j++) {
    if (current_node[j]) {
      delete current_node[j]; current_node[j] = 0;
    }
    if (new_node[j]) {
      new_node[j] = 0;
    }
    if (ws[j]) {
      ws[j] = 0;
    }
    if (branches[j]) {
      branches[j] = 0;
    }
  }
  delete[] current_node;
  delete[] new_node;
  delete[] nodeCountTh;
  delete[] nodesProcTh;
  delete[] treeLbTh;
  delete[] nodeLbTh;
  delete[] minNodeLbTh;
  delete[] ws;
  delete[] rel;
  delete[] branches;
}


void ParBranchAndBound::parsolve(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  double wallTimeStart = getWallTime();
  bool *should_dive = new bool[numThreads];
  bool *dived_prev = new bool[numThreads];
  bool *should_prune = new bool[numThreads];
  bool *initialized = new bool[numThreads];
  NodePtr *current_node = new NodePtr[numThreads]();
  NodePtr *new_node = new NodePtr[numThreads];
  Branches *branches = new Branches[numThreads];
  WarmStartPtr *ws = new WarmStartPtr[numThreads]();
  RelaxationPtr *rel = new RelaxationPtr[numThreads];
  UInt nodeCount;
  double treeLb, nodeLb, minNodeLb;
  double *treeLbTh = new double[numThreads];
  double *nodeLbTh = new double[numThreads];
  double *minNodeLbTh = new double[numThreads];
  bool *shouldRunTh = new bool[numThreads];
  UInt *nodeCountTh = new UInt[numThreads];
  UInt iterCount = 1;
  UInt numVars = 0;

  //Time taken and nodes solved by each thread
  double *sTimeTh = new double[numThreads];
  double *wTimeTh = new double[numThreads];
  double *reachTimeTh = new double[numThreads];
  double *idleTimeTh = new double[numThreads];
  UInt *nodesProcTh = new UInt[numThreads];

  omp_set_num_threads(numThreads);
//#pragma omp parallel for
  for (UInt i = 0; i < numThreads; ++i) {
    should_dive[i] = false;
    dived_prev[i] = false;
    should_prune[i] = false;
    initialized[i] = false;
    shouldRunTh[i] = true;
    nodeCountTh[i] = 1;
    nodesProcTh[i] = 0;
    wTimeTh[i] = 0;
    idleTimeTh[i] = 0;
  }

  // initialize timer
  timer_->start();

  logger_->msgStream(LogInfo) << me_ << "starting branch-and-bound ";
  if(numThreads > 1) {
    logger_->msgStream(LogInfo) << "using " << numThreads << " out of "
      << omp_get_num_procs() << " processors";
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

  rel[0] = parNodeRlxr[0]->getRelaxation();

  // call heuristics before the root, if needed 
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->solve(current_node[0], rel[0], solPool_);
  }
  tm_->setUb(solPool_->getBestSolutionValue());

  // do the root
  current_node[0] = (NodePtr) new Node ();
  processRoot_(&should_prune[0], &dived_prev[0], parNodeRlxr[0],
                  nodePrcssr[0], ws[0], current_node[0]);
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
  bool notRampedUp = true;

  // memory leak check: remove later
  if (numThreads > 1) {
    for (UInt i=1; i < numThreads; i++) {
      if (current_node[i]) {
        assert(!"Memory leak (fake node) with a non-master thread\n");
      }
    }
  }

  while(nodeCount > 0 && shouldRun) {

#pragma omp parallel 
    {
#pragma omp for
      for (UInt i = 0; i < numThreads; ++i) {
        sTimeTh[i] = omp_get_wtime();
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
        if (current_node[i]) {
//#if SPEW
//#pragma omp critical (logger)
          //logger_->msgStream(LogDebug1) << me_ << "process node "
            //<< current_node[0]->getId() << std::endl
            //<< me_ << "depth = " << current_node[0]->getDepth() << std::endl
            //<< me_ << "did we dive = " << dived_prev[0] << std::endl;
//#endif
#pragma omp critical (treeManager)
          {
            if (tm_->shouldPrune_(current_node[i])) {
              parNodeRlxr[i]->reset(current_node[i], false);
#if SPEW
              logger_->msgStream(LogInfo) << me_ << "prune node "
                << current_node[i]->getId() << " thread "
                << omp_get_thread_num() << std::endl;
#endif
              tm_->pruneNode(current_node[i]);
              current_node[i] = NodePtr();
            }
          }
        } else {
#pragma omp critical (treeManager)
          {
            current_node[i] = tm_->getCandidate();
            if(current_node[i]) {
              tm_->removeActiveNode(current_node[i]);
            }
          }
          dived_prev[i] = false;
        }
        if (current_node[i]) {
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogInfo) << me_ << "process node "
            << current_node[i]->getId() << " thread " << omp_get_thread_num() << std::endl
            << me_ << "depth = " << current_node[i]->getDepth() << std::endl
            << me_ << "did we dive = " << dived_prev[i] << std::endl;
#endif
          should_dive[i] = false;

          rel[i] = parNodeRlxr[i]->createNodeRelaxation(current_node[i],
                                                        dived_prev[i],
                                                        should_prune[i]);
          if (isParRel) {
            for (UInt j = 0; j < numThreads; ++j) {
              if (i!=j) {
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
          nodesProcTh[i] += 1;
#pragma omp critical (stats)
          {
            ++stats_->nodesProc;
          }

#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug1) << me_ << "node lower bound = " <<
            current_node[i]->getLb() << current_node[i]->getId() << " thread "
            << omp_get_thread_num()<< std::endl;
#endif
          if (nodePrcssr[i]->foundNewSolution()) {
#pragma omp critical (treeManager)
            {
              tm_->setUb(solPool_->getBestSolutionValue());
            }
          }
          should_prune[i] = shouldPrune_(current_node[i]);

          if (should_prune[i]) {
#if SPEW
#pragma omp critical (logger)
            logger_->msgStream(LogInfo) << me_ << "prune node "
              << current_node[i]->getId() << " thread "
              << omp_get_thread_num() << std::endl;
#endif
            parNodeRlxr[i]->reset(current_node[i], false);
#pragma omp critical (treeManager)
            {
              tm_->pruneNode(current_node[i]);
            }
            current_node[i] = NodePtr();
#pragma omp critical (treeManager)
            {
              new_node[i] = tm_->getCandidate();
              if (new_node[i]) {
#if SPEW
#pragma omp critical (logger)
                logger_->msgStream(LogDebug) << me_ << "get node "
                  << new_node[i]->getId() << " (prune) thread "
                  << omp_get_thread_num() << std::endl;
#endif
                //getting and removing node must be in the same critical
                //block otherwise some other thread might take the same node
                tm_->removeActiveNode(new_node[i]);
              }
            }
            dived_prev[i] = false;

          } else {
            initialized[i] = true;
#if SPEW
#pragma omp critical (logger)
            logger_->msgStream(LogDebug) << me_ << "branch at node "
              << current_node[i]->getId() << " thread "
              << omp_get_thread_num() << std::endl;
#endif
            branches[i] = nodePrcssr[i]->getBranches();

            ws[i] = nodePrcssr[i]->getWarmStart();

            should_dive[i] = tm_->shouldDive();
            if (!branches[i]) {
              logger_->msgStream(LogDebug) << " NO BRANCHES \n";
            }
#pragma omp critical (treeManager)
            {
              new_node[i] = tm_->branch(branches[i], current_node[i], ws[i]);
#if SPEW
#pragma omp critical (logger)
              logger_->msgStream(LogDebug) << me_ << "get node "
                << new_node[i]->getId() << " (branch) thread " << omp_get_thread_num()
                << std::endl;
#endif
            }
            assert((should_dive[i] && new_node[i])
                   || (!should_dive[i] && !new_node[i]));
            if (should_dive[i]) {
              dived_prev[i] = true;
            } else {
              parNodeRlxr[i]->reset(current_node[i], false);
#pragma omp critical (treeManager)
              {
                new_node[i] = tm_->getCandidate(); // Can be NULL. The
                // branches that were created could have large lb and tm
                // might have eliminated them.
                if (new_node[i]) {
                  tm_->removeActiveNode(new_node[i]);
#if SPEW
#pragma omp critical (logger)
                  logger_->msgStream(LogDebug) << me_ << "get/remove node "
                    << new_node[i]->getId() << " thread "
                    << omp_get_thread_num() << std::endl;
#endif
                }
                dived_prev[i] = false;
              }
            }
          }
          current_node[i] = new_node[i];
        } // if (current_node[i]) ends
        wTimeTh[i] += omp_get_wtime() - sTimeTh[i];
        reachTimeTh[i] = omp_get_wtime();
      } //parallel for end

#pragma omp for
      for (UInt i = 0; i < numThreads; ++i) {
        idleTimeTh[i] += omp_get_wtime() - reachTimeTh[i];
        sTimeTh[i] = omp_get_wtime();
        //stopping condition at each thread
        nodeCountTh[i] = 0;
#pragma omp critical (treeManager)
        {
          treeLbTh[i] = tm_->updateLb();
        }
        minNodeLbTh[i] = INFINITY;

        for (UInt j=0; j < numThreads; ++j) {
          if (current_node[j]) {
            nodeCountTh[i]++;
            nodeLbTh[i] = current_node[j]->getLb();
            if (nodeLbTh[i] < minNodeLbTh[i])
            {
              minNodeLbTh[i] = nodeLbTh[i];
            }
          }
        }

        if (minNodeLbTh[i] < treeLbTh[i]) {
          treeLbTh[i] = minNodeLbTh[i];
        }
#pragma omp critical (logger)
        {
          showParStatus_(nodeCountTh[i], treeLbTh[i], wallTimeStart, i);
        }
        if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
#pragma omp critical (treeManager)
          {
            tm_->updateLb();
          }
          shouldRunTh[i] = false;
        }
        wTimeTh[i] += omp_get_wtime() - sTimeTh[i];
        reachTimeTh[i] = omp_get_wtime();
      } //parallel for2 end
      for (UInt i = 0; i < numThreads; ++i) {
        idleTimeTh[i] += omp_get_wtime() - reachTimeTh[i];
      }
#pragma omp single
      {
        iterCount++;
        nodeCount = 0;
        treeLb = tm_->updateLb();
        minNodeLb = INFINITY;
        for (UInt j = 0; j < numThreads; ++j) {
          if (current_node[j]) {
            nodeCount++;
            nodeLb = current_node[j]->getLb();
            if (nodeLb < minNodeLb) {
              minNodeLb = nodeLb;
            }
          }
        }
        if (minNodeLb < treeLb) {
          treeLb = minNodeLb;
        }

        showParStatus_(nodeCount, treeLb, wallTimeStart, 0);

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
#pragma omp critical (logger)
          logger_->msgStream(LogDebug) << me_ << "all nodes have "
            << "been processed" << std::endl;
#endif
        } else if (shouldStopPar_(wallTimeStart, treeLb)) {
          tm_->updateLb();
          shouldRun = false;
        } else if (notRampedUp && (nodeCount == numThreads)) {
          tm_->updateLb();
          notRampedUp = false;
#pragma omp critical (logger)
          logger_->msgStream(LogExtraInfo) << me_
            << "ramp-up time = "
            << getWallTime() - wallTimeStart << std::endl;
          //shouldRun = false;
        } else {
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug) << std::setprecision(8)
            << me_ << "lb = " << tm_->updateLb() << std::endl 
            << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
        }
      } //omp master/single ended
    }   //parallel region ends
  }     //while ends
  logger_->msgStream(LogExtraInfo) << me_ << "stopping branch-and-bound"
    << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl;

  for (UInt i = 0; i < numThreads; ++i) {
    logger_->msgStream(LogExtraInfo) << me_ << "nodes processed by thread "
      << i << " = " << nodesProcTh[i] << std::endl;
  }
  for (UInt i = 0; i < numThreads; ++i) {
    logger_->msgStream(LogExtraInfo) << me_ << "time taken by thread "
      << i << " = " << wTimeTh[i] << std::endl;
  }
  for (UInt i = 0; i < numThreads; ++i) {
    logger_->msgStream(LogExtraInfo) << me_ << "idle time of thread "
      << i << " = " << idleTimeTh[i] << std::endl;
  }

  //if (iterMode) {
  logger_->msgStream(LogInfo)
        << "---------------------------------------------------------------------------------------" << std::endl;
  logger_->msgStream(LogInfo) << me_ << "iterations = " << iterCount
      << std::endl;
  //}
  solPool_->writeStats(logger_->msgStream(LogExtraInfo));

  stats_->timeUsed = timer_->query();
  timer_->stop();

  delete[] should_dive;
  delete[] dived_prev;
  delete[] should_prune;
  delete[] initialized;
  for (UInt i=0; i < numThreads; i++) {
    if (current_node[i]) {
      delete current_node[i]; current_node[i] = 0;
    }
    if (new_node[i]) {
      new_node[i] = 0;
    }
    if (ws[i]) {
      ws[i] = 0;
    }
    if (branches[i]) {
      branches[i] = 0;
    }
  }
  delete[] current_node;
  delete[] new_node;
  delete[] nodeCountTh;
  delete[] nodesProcTh;
  delete[] treeLbTh;
  delete[] nodeLbTh;
  delete[] minNodeLbTh;
  delete[] sTimeTh;
  delete[] wTimeTh;
  delete[] reachTimeTh;
  delete[] idleTimeTh;
  delete[] shouldRunTh;
  delete[] ws;
  delete[] rel;
  delete[] branches;
}


void ParBranchAndBound::parsolveSync(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  double wallTimeStart = getWallTime();
  bool *should_dive = new bool[numThreads];
  bool *dived_prev = new bool[numThreads];
  bool *should_prune = new bool[numThreads];
  bool *initialized = new bool[numThreads];
  NodePtr *current_node = new NodePtr[numThreads]();
  NodePtr *new_node = new NodePtr[numThreads];
  Branches *branches = new Branches[numThreads];
  WarmStartPtr *ws = new WarmStartPtr[numThreads]();
  RelaxationPtr *rel = new RelaxationPtr[numThreads];
  UInt nodeCount;
  double treeLb, nodeLb, minNodeLb;
  double *treeLbTh = new double[numThreads];
  double *nodeLbTh = new double[numThreads];
  double *minNodeLbTh = new double[numThreads];
  bool *shouldRunTh = new bool[numThreads];
  UInt *nodeCountTh = new UInt[numThreads];
  UInt iterCount = 1;
  //bool ubUpdated = false;

  omp_set_num_threads(numThreads);
//#pragma omp parallel for
  for (UInt i = 0; i < numThreads; ++i) {
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
  logger_->msgStream(LogInfo) << "using " << numThreads << " out of "
    << omp_get_num_procs() << " processors";
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

  rel[0] = parNodeRlxr[0]->getRelaxation();

  // call heuristics before the root, if needed
  for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
    (*it)->solve(current_node[0], rel[0], solPool_);
  }
  tm_->setUb(solPool_->getBestSolutionValue());

  // do the root
  current_node[0] = (NodePtr) new Node ();
  processRoot_(&should_prune[0], &dived_prev[0], parNodeRlxr[0],
                  nodePrcssr[0], ws[0], current_node[0]);
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

  // memory leak check: remove later
  if (numThreads > 1) {
    for (UInt i=1; i < numThreads; i++) {
      if (current_node[i]) {
        assert(!"Memory leak (fake node) with a non-master thread\n");
      }
    }
  }

  while ((nodeCount > 0 || tm_->anyActiveNodesLeft()) && shouldRun) {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << "processing node "
      << current_node[0]->getId() << std::endl
      << me_ << "depth = " << current_node[0]->getDepth() << std::endl
      << me_ << "did we dive = " << dived_prev[0] << std::endl;
#endif

//#pragma omp parallel
    //{}
    // NODE ASSIGNMENT
#pragma omp for
    for (UInt i = 0; i < numThreads; ++i) {
      if (current_node[i]) {
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug1) << me_ << "process node "
            << current_node[0]->getId() << std::endl
            << me_ << "depth = " << current_node[0]->getDepth() << std::endl
            << me_ << "did we dive = " << dived_prev[0] << std::endl;
#endif
      } else {
#pragma omp critical (treeManager)
        {
          current_node[i] = tm_->getCandidate();
          if (current_node[i]) {
#if SPEW
#pragma omp critical (logger)
            logger_->msgStream(LogDebug1) << "assign node " << current_node[i]->getId() << " score "
              << (int)current_node[i]->getTbScore() << " lb "
              << current_node[i]->getLb() << " thread " << omp_get_thread_num() << "\n";
#endif
            tm_->removeActiveNode(current_node[i]);
          }
        }
        dived_prev[i] = false;
      }
    } //for end

    // PARALLEL REGION STARTS
#pragma omp parallel
    {
      // NODE SOLVING
#pragma omp for
      for (UInt i = 0; i < numThreads; ++i) {
        UIntVector tmpTimesUp, tmpTimesDown, timesUp, timesDown, lastStrBranched;
        DoubleVector tmpPseudoUp, tmpPseudoDown, pseudoUp, pseudoDown;

        if (current_node[i]) {
          should_dive[i] = false;
          rel[i] = parNodeRlxr[i]->createNodeRelaxation(current_node[i],
                                                        dived_prev[i],
                                                        should_prune[i]);
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogInfo) << me_ << "process node "
            << current_node[i]->getId() << " score "
            << (int)current_node[i]->getTbScore() << " thread "
            << omp_get_thread_num() << std::endl;
#endif
          nodePrcssr[i]->process(current_node[i], rel[i], solPool_,
                                   initialized[i], timesUp, timesDown,
                                   pseudoUp, pseudoDown, stats_->nodesProc);
#pragma omp critical (stats)
          ++stats_->nodesProc;
        } //if current_node[i]
      } //for ends

      // UPPER BOUND UPDATE
#pragma omp single
      {
        for (UInt i = 0; i < numThreads; ++i) {
          if (current_node[i]) {
            //double oldUb = tm_->getUb();
            if (nodePrcssr[i]->foundNewSolution()) {
#if SPEW
              logger_->msgStream(LogInfo) << me_ << "found sol at "
                << current_node[i]->getId() << " score "
                << (int)current_node[i]->getTbScore() << " thread "
                << omp_get_thread_num() << std::endl;
#endif
              tm_->setUb(solPool_->getBestSolutionValue());
              //ubUpdated = (oldUb > tm_->getUb()) ? 1 : 0;
            }
            should_prune[i] = shouldPrune_(current_node[i]);
          } //if current_node[i]
        } //for ends
      }
      // BRANCHING SYNCHRONIZATION
#pragma omp for
      for (UInt i = 0; i < numThreads; ++i) {
        if (current_node[i]) {
          //if (!should_prune[i]) {
            //should_prune[i] = nodePrcssr[i]->shouldPrune_(current_node[i], current_node[i]->getLb(), solPool_);
          //}
          if (should_prune[i]) {
#if SPEW
#pragma omp critical (logger)
            logger_->msgStream(LogInfo) << me_ << "prune node "
              << current_node[i]->getId() << " score "
              << (int)current_node[i]->getTbScore() << " thread "
              << omp_get_thread_num() << std::endl;
#endif
            parNodeRlxr[i]->reset(current_node[i], false);
#pragma omp critical (treeManager)
            {
              tm_->pruneNode(current_node[i]);
              new_node[i] = NodePtr();
            }
            dived_prev[i] = false;
          } else {
            initialized[i] = true;
#if SPEW
#pragma omp critical (logger)
            logger_->msgStream(LogInfo) << me_ << "branch at node "
              << current_node[i]->getId() << " score "
              << (int)current_node[i]->getTbScore() << " thread "
              << omp_get_thread_num() << std::endl;

#endif
            branches[i] = nodePrcssr[i]->getBranches();
            //if (ubUpdated) {
//#pragma omp critical (logger)
              //std::cout << "ub updated\n";
              //BrCandPtr br_can = (*branches[i]->begin())->getBrCand();
              //const double* xsol = nodePrcssr[i]->getSolution()->getPrimal();
              //DoubleVector x;
              //x.resize(rel[i]->getNumVars());
              //std::copy(xsol, xsol+rel[i]->getNumVars(), x.begin());
              //branches[i] = br_can->getHandler()->getBranches(br_can, x, rel[i], solPool_);
              //for (BranchConstIterator br_iter=branches[i]->begin();
                  //br_iter!=branches[i]->end(); ++br_iter) {
                //(*br_iter)->setBrCand(br_can);
              //}
            //}
            ws[i] = nodePrcssr[i]->getWarmStart();
            should_dive[i] = tm_->shouldDive();
            assert(branches[i]);
#pragma omp critical (treeManager)
            {
              new_node[i] = tm_->branch(branches[i], current_node[i], ws[i]);
#if SPEW
#pragma omp critical (logger)
              logger_->msgStream(LogInfo) << me_ << "get node "
                << new_node[i]->getId() << " score "
                << (int)new_node[i]->getTbScore() << " (branch) thread "
                << omp_get_thread_num() << std::endl;
#endif
            }
            assert((should_dive[i] && new_node[i])
                   || (!should_dive[i] && !new_node[i]));
            if (should_dive[i]) {
              dived_prev[i] = true;
            } else {
              parNodeRlxr[i]->reset(current_node[i], false);
#pragma omp critical (treeManager)
              {
                new_node[i] = tm_->getCandidate(); // Can be NULL. The
                // branches that were created could have large lb and tm
                // might have eliminated them.
                if (new_node[i]) {
                  tm_->removeActiveNode(new_node[i]);
#if SPEW
#pragma omp critical (logger)
              logger_->msgStream(LogInfo) << me_ << "get node "
                << new_node[i]->getId() << " score "
                << (int)new_node[i]->getTbScore() << " (prune) thread "
                << omp_get_thread_num() << std::endl;
#endif
                }
              }
              dived_prev[i] = false;
            }
          }
          current_node[i] = new_node[i];
        } // if (current_node[i]) ends
      } //parallel for end
#pragma omp single
      {
        //iterCount++;
        nodeCount = 0;
        treeLb = tm_->updateLb();
        minNodeLb = INFINITY;
        for (UInt j = 0; j < numThreads; ++j) {
          if (current_node[j]) {
            nodeCount++;
            nodeLb = current_node[j]->getLb();
            if (nodeLb < minNodeLb) {
              minNodeLb = nodeLb;
            }
          }
        }
        if (minNodeLb < treeLb) {
          treeLb = minNodeLb;
        }
        showParStatus_(nodeCount, treeLb, wallTimeStart, 0);

        // update stopping conditions for par
        if (nodeCount == 0 && !(tm_->anyActiveNodesLeft())) {
          tm_->updateLb();
          if (tm_->getUb() <= -INFINITY) {
            status_ = SolvedUnbounded;
          } else if (tm_->getUb() < INFINITY) {
            status_ = SolvedOptimal; // TODO: get the right status
          } else {
            status_ = SolvedInfeasible; // TODO: get the right status
          }
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug) << me_ << "all nodes have "
            << "been processed" << std::endl;
#endif
        } else if (shouldStopPar_(wallTimeStart, treeLb)) {
          tm_->updateLb();
          shouldRun = false;
        } else {
#if SPEW
#pragma omp critical (logger)
          logger_->msgStream(LogDebug) << std::setprecision(8)
            << me_ << "lb = " << tm_->updateLb() << std::endl
            << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
          iterCount++;
        }
      } //omp master/single ended
    }   //parallel region ends
  }     //while ends
  logger_->msgStream(LogInfo)
        << "---------------------------------------------------------------------------------------" << std::endl;
  logger_->msgStream(LogInfo) << me_ << "stopping branch-and-bound"
    << std::endl
    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
    << me_ << "nodes created   = " << tm_->getSize() << std::endl
    << me_ << "iterations = " << iterCount << std::endl;

  stats_->timeUsed = timer_->query();
  timer_->stop();

  delete[] should_dive;
  delete[] dived_prev;
  delete[] should_prune;
  delete[] initialized;
  for (UInt i=0; i < numThreads; i++) {
    if (current_node[i]) {
      delete current_node[i]; current_node[i] = 0;
    }
    if (new_node[i]) {
      new_node[i] = 0;
    }
    if (ws[i]) {
      ws[i] = 0;
    }
    if (branches[i]) {
      branches[i] = 0;
    }
  }
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

void ParBranchAndBound::writeParStats(std::ostream &out, ParPCBProcessorPtr nodePrcssr[])
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
  nodeLimit(0),
  perGapLimit(0.),
  solLimit(0),
  timeLimit(0.)

{
}


ParBabOptions::ParBabOptions(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();

  logInterval = options->findDouble("log_interval")->getValue();
  nodeLimit   = options->findInt("node_limit")->getValue();
  perGapLimit = options->findDouble("obj_gap_percent")->getValue();
  solLimit    = options->findInt("sol_limit")->getValue();
  timeLimit   = options->findDouble("time_limit")->getValue();
  createRoot  = true;
}


