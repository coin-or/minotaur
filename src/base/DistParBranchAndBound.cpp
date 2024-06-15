// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2024 The Minotaur Team.
// 

/**
 * \file DistParBranchAndBound.cpp
 * \brief Define DistParBranchAndBound class for a distributed memory parallel branch-and-bound.
 * \author Meenarli Sharma and Prashant Palkar
 */

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "mpi.h"
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
#include "DistParBranchAndBound.h"
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
//
#define TAG_Terminate 0
#define TAG_NodeFound 1
#define TAG_Wait 2
#define TAG_Ub 3
#define TAG_Lb 4

using namespace Minotaur;

const std::string DistParBranchAndBound::me_ = "DistParBranchAndBound: ";

DistParBranchAndBound::DistParBranchAndBound()
  : env_(0),
    nodePrcssr_(),
    nodeRlxr_(0),
    options_(0),
    problem_(0),
    solPool_(0),
    stats_(0),
    status_(NotStarted),
    timer_(0),
    tm_(0),
    proc_rank_(0),
    num_procs_(0)
{
}

DistParBranchAndBound::DistParBranchAndBound(EnvPtr env, ProblemPtr p, UInt proc_rank, UInt num_procs)
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
  options_ = (DistParBabOptionsPtr) new DistParBabOptions(env);
  logger_ = env->getLogger();
  proc_rank_ = proc_rank;
  num_procs_ = num_procs;
}


DistParBranchAndBound::~DistParBranchAndBound()
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


void DistParBranchAndBound::addPreRootHeur(HeurPtr h)
{
  preHeurs_.push_back(h);
}


double DistParBranchAndBound::getPerGap() 
{ 
  return tm_->getPerGap(); 
}


double DistParBranchAndBound::getLb() 
{
  return tm_->getLb();
}


//NodePtr DistParBranchAndBound::createNode_(std::vector<double> node_data)
//{
//  NodePtr new_node = NodePtr(); // NULL
//
//  tm_->insertRoot(new_node);
//  rel = parNodeRlxr0->createRootRelaxation(current_node, solPool_, prune);
//  return new_node;
//}


NodeProcessorPtr DistParBranchAndBound::getNodeProcessor()
{
  return nodePrcssr_;
}


NodeRelaxerPtr DistParBranchAndBound::getNodeRelaxer()
{
  return nodeRlxr_;
}


SolutionPtr DistParBranchAndBound::getSolution()
{
  return solPool_->getBestSolution();
}


SolveStatus DistParBranchAndBound::getStatus()
{
  return status_;
}


ParTreeManagerPtr DistParBranchAndBound::getTreeManager() 
{
  return tm_;
}


double DistParBranchAndBound::getUb() 
{
  return tm_->getUb();
}


std::vector<std::vector<double> > DistParBranchAndBound::mapSerialOutput (
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


UInt DistParBranchAndBound::numProcNodes()
{
  return stats_->nodesProc;
}


void DistParBranchAndBound::print2dvec(std::vector<std::vector<double> > output)
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


void DistParBranchAndBound::print2dvec(std::vector<std::vector<int> > output)
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


void DistParBranchAndBound::processRoot_(bool *should_prune, bool *should_dive,
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
}

std::vector<std::vector<double> > DistParBranchAndBound::
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


void DistParBranchAndBound::setLogLevel(LogLevel level) 
{
  logger_->setMaxLevel(level);
}


void DistParBranchAndBound::setNodeProcessor(NodeProcessorPtr p)
{
  nodePrcssr_ = p;
}


void DistParBranchAndBound::setNodeRelaxer(NodeRelaxerPtr nr)
{
  nodeRlxr_ = nr;
}


void DistParBranchAndBound::shouldCreateRoot(bool b)
{
  options_->createRoot = b;
}


bool DistParBranchAndBound::shouldPrune_(NodePtr node)
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

bool DistParBranchAndBound::shouldStopPar_(double WallTimeStart, double treeLb)
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

void DistParBranchAndBound::showStatus_(bool current_uncounted)
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



void DistParBranchAndBound::showParStatus_(UInt off, double treeLb,
                                       double WallTimeStart)
{
  if (timer_->query()-stats_->updateTime > options_->logInterval) {
    logger_->msgStream(LogInfo) 
      << me_ 
      << std::fixed
      << std::setprecision(1)  << "time = " << getWallTime() - WallTimeStart
      << std::setprecision(4)  << " lb = "  << treeLb
      << std::setprecision(4)  << " ub = "  << tm_->getUb()
      << std::setprecision(2)  << " gap% = " << tm_->getPerGapPar(treeLb)
      << " nodes processed = " << tm_->getSize()-tm_->getActiveNodes()-off 
      << " left = " << tm_->getActiveNodes()+off
      //<< " thread " << i
      << std::endl;
    stats_->updateTime = timer_->query();
  }
}


int DistParBranchAndBound::strToInt(std::string str)
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

//void DistParBranchAndBound::parsolveOppor(ParNodeIncRelaxerPtr parNodeRlxr[],
//                                 ParPCBProcessorPtr nodePrcssr[],
//                                 UInt numThreads)
//{
// //------------------ 
//  int trap_key = 0;
//  MPI_Status status;
//  if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
//        MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
//
//  //the critical section 
//  std::cout << "Process " << proc_rank_ << " entered parsolveOppor()." << std::endl;
//  
//  // this will allow only the last process to skip this
//  if(proc_rank_ != num_procs_ - 1) {
//    MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
//  }
// //------------------
//  UInt iter;
//  UInt index; 
//  bool shouldRun = true;
//  bool shouldWait = true;
//  std::vector<double> bound_changes;
//
//  //if (proc_rank_ > 0) {
//  //  while (shouldWait) {
//  //    if (shouldRun == false) {
//  //      return;
//  //    } else {
//  //      getStartingNode_(&shouldWait, bound_changes);
//  //      if (shouldWait == false) {
//  //        break;
//  //      }
//  //    }
//  //  }
//  //}
//
//  bool isParRel = false;
//  bool shouldDistribute = true;
//  double wallTimeStart = getWallTime();
//  double last_log_time = wallTimeStart;
//  double log_freq = 5; // create an options for this later
//  bool *should_dive = new bool[numThreads];
//  bool *dived_prev = new bool[numThreads];
//  bool *should_prune = new bool[numThreads];
//  bool *initialized = new bool[numThreads];
//  NodePtr *current_node = new NodePtr[numThreads]();
//  NodePtr *new_node = new NodePtr[numThreads]();
//  Branches *branches = new Branches[numThreads]();
//  WarmStartPtr *ws = new WarmStartPtr[numThreads]();
//  RelaxationPtr *rel = new RelaxationPtr[numThreads];
//  double *treeLbTh = new double[numThreads];
//  double *nodeLbTh = new double[numThreads];
//  double *minNodeLbTh = new double[numThreads];
//  UInt *nodeCountTh = new UInt[numThreads];
//  UInt *nodesProcTh = new UInt[numThreads];
//  UInt numVars = 0;
//  double procBestLB;
//  double globalBestLb;
//  MPI_Request mpi_request;
//  int flag;
//  bool request_in_progress;
//
//  //double new_obj_val = INFTY;
//  omp_set_num_threads(numThreads);
////#pragma omp parallel for
//  for (UInt i = 0; i < numThreads; ++i) {
//    should_dive[i] = false;
//    dived_prev[i] = false;
//    should_prune[i] = false;
//    initialized[i] = false;
//    nodeCountTh[i] = 1;
//    nodesProcTh[i] = 0;
//  }
//
//  // initialize timer
//  timer_->start();
//  if (proc_rank_ == 0) {
//    logger_->msgStream(LogInfo) << me_ << "starting branch-and-bound ";
//    if (numThreads > 1) {
//      logger_->msgStream(LogInfo) << "using " << numThreads << " out of "
//        << omp_get_num_procs() << " processors";
//    }
//    logger_->msgStream(LogInfo) << std::endl;
//    // get problem size and statistics to detect problem type.
//    problem_->calculateSize();
//  #if SPEW
//    problem_->writeSize(logger_->msgStream(LogExtraInfo));
//  #endif
//  
//    // initialize statistics
//    if (stats_) {
//      delete stats_;
//    }
//    stats_ = new DistParBabStats();
//  
//    // initialize solution pool
//    // TODO: use user options to set the pool size. For now it is 1.
//    solPool_ = (SolutionPoolPtr) new SolutionPool(env_, problem_, 1);
//  
//    rel[0] = parNodeRlxr[0]->getRelaxation();
//    // call heuristics before the root, if needed 
//    for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
//      (*it)->solve(current_node[0], rel[0], solPool_);
//    }
//    tm_->setUb(solPool_->getBestSolutionValue());
//  
//    // do the root
//    current_node[0] = (NodePtr) new Node ();
//    processRoot_(&should_prune[0], &dived_prev[0], parNodeRlxr[0],
//                    nodePrcssr[0], ws[0], current_node[0]);
//    // stop if done
//    if (!current_node[0]) {
//      tm_->updateLb();
//      if (tm_->getUb() <= -INFINITY) {
//        status_ = SolvedUnbounded;
//      } else  if (tm_->getUb() < INFINITY) {
//        status_ = SolvedOptimal; 
//      } else {
//        status_ = SolvedInfeasible; 
//      }
//  #if SPEW
//      logger_->msgStream(LogDebug) << me_ << "stopping after root node "
//        << std::endl;
//  #endif
//      shouldRun = false;
//    } else if (shouldStopPar_(wallTimeStart, tm_->getLb())) {
//      tm_->updateLb();
//    } else {
//  #if SPEW
//      logger_->msgStream(LogDebug) << std::setprecision(8)
//        << me_ << "lb = " << tm_->updateLb() << std::endl
//        << me_ << "ub = " << tm_->getUb() << std::endl;
//  #endif
//    }
//  }
//
// //------------------ 
//  trap_key = 0;
//  //MPI_Status status;
//  if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
//        MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
//
//  //the critical section 
//  std::cout << "Process " << proc_rank_ << " reached barrier." << std::endl;
//  
//  // this will allow only the last process to skip this
//  if(proc_rank_ != num_procs_ - 1) {
//    MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
//  }
// //------------------ 
//  MPI_Barrier(MPI_COMM_WORLD);
//  
//  MPI_Bcast(&shouldRun, 1, MPI_INT, 0, MPI_COMM_WORLD);
// //------------------ 
//  trap_key = 0;
//  //MPI_Status status;
//  if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
//        MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
//
//  //the critical section 
//  std::cout << "Process " << proc_rank_ << " " << " var shouldRun " << shouldRun << " " << std::endl;
//  
//  // this will allow only the last process to skip this
//  if(proc_rank_ != num_procs_ - 1) {
//    MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
//  }
// //------------------ 
//
//  if (proc_rank_ > 0) {
//    while (shouldWait) {
//      if (shouldRun == false) {
//        return;
//      } else {
//        getStartingNode_(&shouldWait, &shouldRun, bound_changes);
//        std::cout << bound_changes.size() << std::endl;
//        //exit(0);
//        if (shouldWait == false) {
//          break;
//        }
//      }
//    }
//
//    logger_->msgStream(LogInfo) << me_ << "starting branch-and-bound ";
//    if (numThreads > 1) {
//      logger_->msgStream(LogInfo) << "using " << numThreads << " out of "
//        << omp_get_num_procs() << " processors";
//    }
//    logger_->msgStream(LogInfo) << std::endl;
//    // get problem size and statistics to detect problem type.
//    problem_->calculateSize();
//  #if SPEW
//    problem_->writeSize(logger_->msgStream(LogExtraInfo));
//  #endif
//  
//    // initialize statistics
//    if (stats_) {
//      delete stats_;
//    }
//    stats_ = new DistParBabStats();
//  
//    // initialize solution pool
//    // TODO: use user options to set the pool size. For now it is 1.
//    solPool_ = (SolutionPoolPtr) new SolutionPool(env_, problem_, 1);
//  
//    rel[0] = parNodeRlxr[0]->getRelaxation();
//    // Apply bound_changes to rel[0]
//    iter = 0;
//    while(iter < bound_changes.size() - 2) {
//      index = static_cast<UInt>(bound_changes[iter]);
//      if (bound_changes[iter+1] == 0.0) {
//        rel[0]->changeBoundByInd(index, Lower, bound_changes[iter+2]);
//      } else {
//        rel[0]->changeBoundByInd(index, Upper, bound_changes[iter+2]);
//      }
//      iter = iter + 3;
//    } 
//  
//    // call heuristics before the root, if needed 
//    for (HeurVector::iterator it=preHeurs_.begin(); it!=preHeurs_.end(); ++it) {
//      (*it)->solve(current_node[0], rel[0], solPool_);
//    }
//    tm_->setUb(solPool_->getBestSolutionValue());
//  
//    // do the root
//    current_node[0] = (NodePtr) new Node ();
//    processRoot_(&should_prune[0], &dived_prev[0], parNodeRlxr[0],
//                    nodePrcssr[0], ws[0], current_node[0]);
//    // stop if done
//    if (!current_node[0]) {
//      tm_->updateLb();
//      if (tm_->getUb() <= -INFINITY) {
//        status_ = SolvedUnbounded;
//      } else  if (tm_->getUb() < INFINITY) {
//        status_ = SolvedOptimal; 
//      } else {
//        status_ = SolvedInfeasible; 
//      }
//  #if SPEW
//      logger_->msgStream(LogDebug) << me_ << "stopping after root node "
//        << std::endl;
//  #endif
//      shouldRun = false;
//    } else if (shouldStopPar_(wallTimeStart, tm_->getLb())) {
//      tm_->updateLb();
//    } else {
//  #if SPEW
//      logger_->msgStream(LogDebug) << std::setprecision(8)
//        << me_ << "lb = " << tm_->updateLb() << std::endl
//        << me_ << "ub = " << tm_->getUb() << std::endl;
//  #endif
//    }
//  }
//  
//
//  // solve root outside the loop. save the useful information.
//  initialized[0] = true; //pseudoCosts for thread0 initialized while doing root
//  numVars = rel[0]->getNumVars();
//  if (nodePrcssr[0]->getBrancher()->getName() == "ParReliabilityBrancher") {
//    isParRel = true;
//  }
//
//  //bool notRampedUp = true;
//  UInt i=0; // thread id
//#pragma omp parallel private(i)
//  {
//    i = omp_get_thread_num();
//    ParReliabilityBrancherPtr parRelBr;
//    UIntVector tmpTimesUp, tmpTimesDown, timesUp, timesDown, lastStrBranched;
//    DoubleVector tmpPseudoUp, tmpPseudoDown, pseudoUp, pseudoDown;
//    if (isParRel) {
//      timesUp.resize(numVars,0);
//      timesDown.resize(numVars,0);
//      pseudoUp.resize(numVars,0);
//      pseudoDown.resize(numVars,0);
//      lastStrBranched.resize(numVars,0);
//    }
// 
//    while (nodeCountTh[i] > 0 && shouldRun) {
//      checkBUpdatesFromOtherProcs_();
//      if (current_node[i]) {
//#pragma omp critical (treeManager)
//        {
//          if (tm_->shouldPrune_(current_node[i])) {
//            parNodeRlxr[i]->reset(current_node[i], false);
//#if SPEW
//            logger_->msgStream(LogInfo) << me_ << "prune node "
//              << current_node[i]->getId() << " thread "
//              << omp_get_thread_num() << std::endl;
//#endif
//            tm_->pruneNode(current_node[i]);
//#pragma omp critical (current_node)
//            current_node[i] = NodePtr();
//          }
//        }
//      } else {
//#pragma omp critical (treeManager)
//        {
//          current_node[i] = tm_->getCandidate();
//          if (current_node[i]) {
//            tm_->removeActiveNode(current_node[i]);
//          }
//        }
//        if (current_node[i]) {
//          nodesProcTh[i]++;
//#if SPEW
//#pragma omp critical (logger)
//          logger_->msgStream(LogDebug) << me_ << "get node "
//            << current_node[i]->getId() << " thread "
//            << omp_get_thread_num() << std::endl;
//#endif
//        }
//        dived_prev[i] = false;
//      }
//
//      // Checking tm_ size and distributing nodes to other processors
//      // // TODO: Check if i == 0 is neeeded for threads/procs interaction
//      if (proc_rank_ == 0 && i == 0 && shouldDistribute) {
//#pragma omp critical (treeManager)
//        {
//          std::cout << "tm size " << tm_->getSize() << "\n";
//          if (static_cast<int>(tm_->getSize()) >= num_procs_) {
//            distributeNodes_();
//            shouldDistribute = false;
//          }
//        } 
//      }
//
//      if (current_node[i]) {
//#if SPEW
//#pragma omp critical (logger)
//        logger_->msgStream(LogInfo) << me_ << "process node "
//          << current_node[i]->getId() << " thread " << omp_get_thread_num() << std::endl;
//          //<< me_ << "depth = " << current_node[i]->getDepth() << std::endl
//          //<< me_ << "did we dive = " << dived_prev[i] << std::endl;
//#endif
//        should_dive[i] = false;
//
//        rel[i] = parNodeRlxr[i]->createNodeRelaxation(current_node[i],
//                                                      dived_prev[i],
//                                                      should_prune[i]);
//        if (isParRel) {
//          for (UInt j = 0; j < numThreads; ++j) {
//            if (i!=j) {
//              parRelBr = dynamic_cast <ParReliabilityBrancher*> (nodePrcssr[j]->getBrancher());
//              tmpTimesUp = parRelBr->getTimesUp();
//              tmpTimesDown = parRelBr->getTimesDown();
//              tmpPseudoUp = parRelBr->getPCUp();
//              tmpPseudoDown = parRelBr->getPCDown();
//              for (UInt l=0; l < tmpTimesDown.size(); ++l) {
//                timesUp[l] += tmpTimesUp[l];
//                timesDown[l] += tmpTimesDown[l];
//                pseudoUp[l] += tmpTimesUp[l]*tmpPseudoUp[l];
//                pseudoDown[l] += tmpTimesDown[l]*tmpPseudoDown[l];
//              }
//            }
//          }
//        }
//        nodePrcssr[i]->process(current_node[i], rel[i], solPool_,
//                               initialized[i], timesUp, timesDown,
//                               pseudoUp, pseudoDown, stats_->nodesProc);
//#pragma omp critical (stats)
//        {
//          ++stats_->nodesProc;
//        }
//#if SPEW
//#pragma omp critical (logger)
//        logger_->msgStream(LogDebug1) << me_ << "node " 
//          << current_node[i]->getId() << " lower bound = "
//          << current_node[i]->getLb() << " thread " 
//          << omp_get_thread_num() << std::endl;
//#endif
//        if (nodePrcssr[i]->foundNewSolution()) {
//
//#pragma omp critical (treeManager)
//          {
//            // Broadcast updated ub to all
//            if (solPool_->getBestSolutionValue() < tm_->getUb()) {
//              sendValToOtherProcs_(solPool_->getBestSolutionValue(), UB);
//            }
//            tm_->setUb(solPool_->getBestSolutionValue());
//          }
//        }
//        should_prune[i] = shouldPrune_(current_node[i]);
//
//        if (should_prune[i]) {
//#if SPEW
//#pragma omp critical (logger)
//          logger_->msgStream(LogInfo) << me_ << "prune node "
//            << current_node[i]->getId() << " thread "
//            << omp_get_thread_num() << std::endl;
//#endif
//          parNodeRlxr[i]->reset(current_node[i], false);
//#pragma omp critical (treeManager)
//          {
//            tm_->pruneNode(current_node[i]);
//          }
//#pragma omp critical (current_node)
//          current_node[i] = NodePtr();
//#pragma omp critical (treeManager)
//          {
//            new_node[i] = tm_->getCandidate();
//            if (new_node[i]) {
//              //getting and removing node must be in the same critical
//              //block otherwise some other thread might take the same node
//              tm_->removeActiveNode(new_node[i]);
//              nodesProcTh[i]++;
//#if SPEW
//#pragma omp critical (logger)
//              logger_->msgStream(LogDebug) << me_ << "get node "
//                << new_node[i]->getId() << " (prune) thread "
//                << omp_get_thread_num() << std::endl;
//#endif
//            }
//          }
//          dived_prev[i] = false;
//        } else {
//          initialized[i] = true;
//#if SPEW
//#pragma omp critical (logger)
//          logger_->msgStream(LogDebug) << me_ << "branch at node "
//            << current_node[i]->getId() << " thread "
//            << omp_get_thread_num() << std::endl;
//#endif
//          branches[i] = nodePrcssr[i]->getBranches();
//
//          ws[i] = nodePrcssr[i]->getWarmStart();
//
//          should_dive[i] = tm_->shouldDive();
//          if (!branches[i]) {
//            logger_->msgStream(LogDebug) << " NO BRANCHES \n";
//          }
//#pragma omp critical (treeManager)
//          {
//#pragma omp critical (current_node)
//            new_node[i] = tm_->branch(branches[i], current_node[i], ws[i]);
//#if SPEW
//#pragma omp critical (logger)
//            logger_->msgStream(LogDebug) << me_ << "get node "
//              << new_node[i]->getId() << " (branch) thread " << omp_get_thread_num()
//              << std::endl;
//#endif
//          }
//          assert((should_dive[i] && new_node[i])
//                 || (!should_dive[i] && !new_node[i]));
//          if (should_dive[i]) {
//            dived_prev[i] = true;
//          } else {
//            parNodeRlxr[i]->reset(current_node[i], false);
//#pragma omp critical (treeManager)
//            {
//              new_node[i] = tm_->getCandidate(); // Can be NULL. The
//              // branches that were created could have large lb and tm
//              // might have eliminated them.
//              if (new_node[i]) {
//                tm_->removeActiveNode(new_node[i]);
//#if SPEW
//#pragma omp critical (logger)
//                logger_->msgStream(LogDebug) << me_ << "get/remove node "
//                  << new_node[i]->getId() << " thread "
//                  << omp_get_thread_num() << std::endl;
//#endif
//              }
//              dived_prev[i] = false;
//            }
//          }
//        }
//#pragma omp critical (current_node)
//        current_node[i] = new_node[i];
//      } // if (current_node[i]) ends
//      //update lower bound
//#pragma omp critical (treeManager)
//      {
//        treeLbTh[i] = tm_->updateLb();
//      }
//      minNodeLbTh[i] = INFINITY;
//      for (UInt j=0; j < numThreads; ++j) {
//#pragma omp critical (current_node)
//        {
//          if (current_node[j]) {
//            nodeLbTh[i] = current_node[j]->getLb();
//          }
//        }
//        if (current_node[j]) {
//          if (nodeLbTh[i] < minNodeLbTh[i]) {
//            minNodeLbTh[i] = nodeLbTh[i];
//          }
//        }
//      }
//      if (minNodeLbTh[i] < treeLbTh[i]) {
//        treeLbTh[i] = minNodeLbTh[i];
//      }
//      //stopping condition at each thread
//      nodeCountTh[i] = tm_->anyActiveNodesLeft();
//      if (nodeCountTh[i] == 0) {
//        for (UInt j=0; j < numThreads; ++j) {
//          if (current_node[j]) {
//#pragma omp atomic
//            nodeCountTh[i]++;
//            break;
//          }
//        }
//      }
//      // Regular tree status logs
//      if (getWallTime() - last_log_time > log_freq) {
//        procBestLB = tm_->getLb();
//        last_log_time = getWallTime();
//        if (!request_in_progress) {
//          MPI_Ireduce(&procBestLB, &globalBestLb, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD, &mpi_request);
//          request_in_progress = true;
//          std::cout << globalBestLb << "\n";
//        }
//      }
//      if (request_in_progress) {
//        MPI_Test(&mpi_request, &flag, MPI_STATUS_IGNORE);
//        //MPI_Wait(&mpi_request, MPI_STATUS_IGNORE);
//          if (proc_rank_ == 0 && flag && i == 0) {
//#pragma omp critical (logger)
//            {
//              showParStatus_(nodeCountTh[i], globalBestLb, wallTimeStart);
//              request_in_progress = false;
//            }
//        }
//      }
//      if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
//#pragma omp critical (treeManager)
//        {
//          tm_->updateLb();
//        }
//      }
//
//      // update stopping conditions
//      if (nodeCountTh[i] == 0) {
//        tm_->updateLb();
//        if (tm_->getUb() <= -INFINITY) {
//          status_ = SolvedUnbounded;
//        } else if (tm_->getUb() < INFINITY) {
//          status_ = SolvedOptimal; // TODO: get the right status
//        } else {
//          status_ = SolvedInfeasible; // TODO: get the right status
//        }
//#if SPEW
//#pragma omp single
//        logger_->msgStream(LogDebug) << me_ << "all nodes have "
//          << "been processed" << " thread " << i << std::endl;
//#endif
//      } else if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
//        tm_->updateLb();
//        shouldRun = false;
//      //} else if (notRampedUp && (nodeCount == numThreads)) {
//        //tm_->updateLb();
//        //notRampedUp = false;
////#pragma omp critical (logger)
//        //logger_->msgStream(LogExtraInfo) << me_
//          //<< "ramp-up time = "
//          //<< getWallTime() - wallTimeStart << std::endl;
//        ////shouldRun = false;
//      } else {
//#if SPEW
//#pragma omp critical (logger)
//        //logger_->msgStream(LogInfo) << "nodesCount " << nodeCountThread << " thread " << i << std::endl;
//        logger_->msgStream(LogDebug) << std::setprecision(8)
//          << me_ << "lb = " << tm_->updateLb() << std::endl
//          << me_ << "ub = " << tm_->getUb() << std::endl;
//#endif
//      }
//    } //while ends
//#if SPEW
//#pragma omp single
//    {
//      for (UInt j=0; j < numThreads; ++j) {
//        logger_->msgStream(LogInfo) << "nodesProc " << nodesProcTh[j] << " thread " << j << std::endl;
//      }
//    }
//#endif
//  }   //parallel region ends
//  logger_->msgStream(LogExtraInfo) << me_ << "stopping branch-and-bound"
//    << std::endl
//    << me_ << "nodes processed = " << stats_->nodesProc << std::endl
//    << me_ << "nodes created   = " << tm_->getSize() << std::endl;
//  solPool_->writeStats(logger_->msgStream(LogExtraInfo));
//
//  //if (proc_rank_ > 0) {
//  //  SEND LB to proc rank 0
//  //}
//
//  for (UInt k = 0; k < numThreads; ++k) {
//    logger_->msgStream(LogExtraInfo) << me_ << "nodes processed by thread "
//      << k << " = " << nodesProcTh[k] << std::endl;
//  }
//
//  if (proc_rank_ == 0 && i == 0) {
//    if (shouldDistribute) {
//      int proc;
//      double value = 0.0;
//      for (proc = 1; proc < num_procs_; ++proc) {
//        //TODO: Use enum for tag 0
//        MPI_Send(&value, 1, MPI_DOUBLE, proc, 0, MPI_COMM_WORLD);
//      }
//    }
//  }
//
//  stats_->timeUsed = timer_->query();
//  timer_->stop();
//
//  delete[] should_dive;
//  delete[] dived_prev;
//  delete[] should_prune;
//  delete[] initialized;
//  for (UInt j=0; j < numThreads; j++) {
//    if (current_node[j]) {
//      delete current_node[j]; current_node[j] = 0;
//    }
//    if (new_node[j]) {
//      new_node[j] = 0;
//    }
//    if (ws[j]) {
//      ws[j] = 0;
//    }
//    if (branches[j]) {
//      branches[j] = 0;
//    }
//  }
//  delete[] current_node;
//  delete[] new_node;
//  delete[] nodeCountTh;
//  delete[] nodesProcTh;
//  delete[] treeLbTh;
//  delete[] nodeLbTh;
//  delete[] minNodeLbTh;
//  delete[] ws;
//  delete[] rel;
//  delete[] branches;
// //------------------ 
//  trap_key = 0;
//  //MPI_Status status;
//  if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
//        MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
//
//  //the critical section 
//  std::cout << "Process " << proc_rank_ << " exiting parsolveOppor()." << std::endl;
//  
//  // this will allow only the last process to skip this
//  if(proc_rank_ != num_procs_ - 1) {
//    MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
//  }
// //------------------
//}
void DistParBranchAndBound::parsolveOppor(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  if (proc_rank_ == 0) {
    solveMasterProc_(parNodeRlxr, nodePrcssr, numThreads); 
  } else {
    solveWorkerProc_(parNodeRlxr, nodePrcssr, numThreads); 
  }

  MPI_Barrier(MPI_COMM_WORLD);
}


void DistParBranchAndBound::solveMasterProc_(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
  bool shouldRun = true;
  std::vector<double> bound_changes;

  bool isParRel = false;
  bool shouldDistribute = true;
  double wallTimeStart = getWallTime();
  double last_log_time_lb = wallTimeStart;
  double last_log_time_status = wallTimeStart;
  double last_log_time_ub = wallTimeStart;
  double log_freq_ub = 5; // create an options for this later
  double log_freq_lb = 5; // create an options for this later
  double log_freq_status = 5; // create an options for this later
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
  double globalBestLb = INFINITY;
  MPI_Request mpi_request_0;
  int shoulRun_int_val;
  bool solved_at_root = false;
  std::vector<int> proc_running;
  std::vector<double> proc_lb;

  int checkPrint = false;

  //double new_obj_val = INFTY;
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
  stats_ = new DistParBabStats();

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
            
  proc_lb.resize(1, INFINITY);
  // stop if done
  if (!current_node[0]) {
    //tm_->updateLb();
    if (tm_->getUb() <= -INFINITY) {
      status_ = SolvedUnbounded;
    } else if (tm_->getUb() < INFINITY) {
      // since there is no active node, setting lb to the ub obtained
      //globalBestLb = tm_->getUb();
      //tm_->setLb(globalBestLb);
      proc_lb[0] = tm_->getUb();
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
    //tm_->updateLb();
    proc_lb[0] = tm_->getLb();
    shouldRun = false;
  } else {
    globalBestLb = tm_->getLb();
    proc_lb[0] = tm_->getLb();
//#if SPEW
//    logger_->msgStream(LogDebug) << std::setprecision(8)
//      << me_ << "lb = " << tm_->updateLb() << std::endl
//      << me_ << "ub = " << tm_->getUb() << std::endl;
//#endif
  }
  
  if (shouldRun) {
    shoulRun_int_val = 1;
    std::cout <<"Start: globalBestLb "<< globalBestLb << "\n";
  } else {
    shoulRun_int_val = 0;
    solved_at_root = true;
  }

  for (int i = 1; i < num_procs_; ++i) {
    if (shouldRun) {
      proc_running.resize(num_procs_,0);
      MPI_Isend(&shoulRun_int_val, 1, MPI_INT, i, TAG_Wait, MPI_COMM_WORLD, &mpi_request_0);
    } else {
      proc_running.resize(num_procs_,0);
      MPI_Isend(&shoulRun_int_val, 1, MPI_INT, i, TAG_Terminate, MPI_COMM_WORLD, &mpi_request_0);
      shouldDistribute = false;
    }
  }

  // solve root outside the loop. save the useful information.
  initialized[0] = true; //pseudoCosts for thread0 initialized while doing root
  numVars = rel[0]->getNumVars();
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
      // check if always thread 0 is bound to MPI process, otherwise use
      // pragma single
      if (i == 0 && num_procs_ > 1) {
        if (getWallTime() - last_log_time_status > log_freq_status && (!shouldDistribute)) {
          updateProcsRunningStatus_(proc_running);
          last_log_time_status = getWallTime();
        }
        if (getWallTime() - last_log_time_ub > log_freq_ub && (!shouldDistribute)) {
          checkUbUpdates_(proc_running);
          last_log_time_ub = getWallTime();
        }
      } 
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

      // Checking tm_ size and distributing nodes to other processors
      // // TODO: Check if i == 0 is neeeded for threads/procs interaction
      if (num_procs_ > 1 && i == 0 && shouldDistribute) {
#pragma omp critical (treeManager)
        {
          if (static_cast<int>(tm_->getActiveNodes()) > num_procs_) {
            std::cout << " tm size " << tm_->getActiveNodes() << "\n";
            //std::cout << " Proc " << proc_rank_ << " sent node\n";
            distributeNodes_();
            shouldDistribute = false;
            proc_running.resize(num_procs_, 1);
            proc_lb.resize(num_procs_, INFINITY);
          }
        } 
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
            std::cout << "shouldDistribute "<< shouldDistribute << "\n";
            // Send updated ub to all
            if (num_procs_ > 1 && solPool_->getBestSolutionValue() < tm_->getUb()) {
              std::cout << " Found a solution, value " << tm_->getUb() << "\n"; 
              sendUbToOtherProcs_(solPool_->getBestSolutionValue(), proc_running);
            }
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
      // Regular tree status logs (check MPI-thread binding and remove
      // critical accordingly)
      if (num_procs_ > 1 && i==0 && getWallTime() - last_log_time_lb > log_freq_lb && (!shouldDistribute)) {
        //checkLbUpdatesFromOtherProcs_(globalBestLb, proc_running);
        checkLbUpdatesFromOtherProcs_(globalBestLb, proc_lb);
        last_log_time_lb = getWallTime();
        //MPI_Wait(&mpi_request, MPI_STATUS_IGNORE);
//#pragma omp critical (logger)
//        {
        std::cout << "globalBestLb " << globalBestLb << "\n";
        showParStatus_(nodeCountTh[i], globalBestLb, wallTimeStart);
//        }
      }
//      if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
//#pragma omp critical (treeManager)
//        {
//          tm_->updateLb();
//        }
//      }

      // update stopping conditions
      if (nodeCountTh[i] == 0) {
#pragma omp critical (treeManager)
        {
          tm_->updateLb();
        }
        if (tm_->getUb() <= -INFINITY) {
          status_ = SolvedUnbounded;
        } else if (tm_->getUb() < INFINITY) {
          status_ = SolvedOptimal; // TODO: get the right status
//#pragma omp single
//          {
//            globalBestLb = std::min(globalBestLb, tm_->getLb());
//            //std::cout <<" Inside: globalBestLb "<< globalBestLb << "\n";
//          }
        } else {
          status_ = SolvedInfeasible; // TODO: get the right status
        }
#if SPEW
#pragma omp single
        logger_->msgStream(LogDebug) << me_ << "all nodes have "
          << "been processed" << " thread " << i << std::endl;
#endif
      } else if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
#pragma omp critical (treeManager)
        {
          tm_->updateLb();
        }
//#pragma omp single
//        {
//          globalBestLb = std::min(globalBestLb, tm_->getLb());
//          std::cout <<"Inside: globalBestLb "<< globalBestLb << "\n";
//        }
        shouldRun = false;
      } else {
//#pragma omp single
//        {
//          globalBestLb = std::min(globalBestLb, tm_->getLb());
//          //std::cout <<"Inside: globalBestLb "<< globalBestLb << "\n";
//        }
//#if SPEW
//#pragma omp critical (logger)
//        //logger_->msgStream(LogInfo) << "nodesCount " << nodeCountThread << " thread " << i << std::endl;
//        logger_->msgStream(LogDebug) << std::setprecision(8)
//          << me_ << "lb = " << tm_->updateLb() << std::endl
//          << me_ << "ub = " << tm_->getUb() << std::endl;
//#endif
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


  if (status_ == SolvedOptimal) {
    tm_->setLb(tm_->getUb());
    proc_lb[0] = tm_->getUb();
    //globalBestLb = std::max(globalBestLb, tm_->getLb());
  } else {
    proc_lb[0] = tm_->getLb();
    //globalBestLb = std::min(globalBestLb, tm_->getLb());
  }

  std::cout << " Parallel region exit for proc " << proc_rank_ << "\n";
  if (shouldDistribute) {
    int proc;
    double value = 0.0;
    for (proc = 1; proc < num_procs_; ++proc) {
      MPI_Send(&value, 1, MPI_DOUBLE, proc, TAG_Terminate, MPI_COMM_WORLD);
      proc_running[i] = 0;
    }
  }

  std::cout << " After parallel region exit for proc " << proc_rank_ << " solved_at_root " << solved_at_root << "\n";
  shouldRun = false;
  std::cout << "Came here\n";
  std::cout <<"TM LB " << tm_->getLb() << "\n";
  while (!solved_at_root && num_procs_ > 1) {
    //std::cout << "Came here\n";
    //std::cout <<"End: globalBestLb "<< globalBestLb << "\n";
    for (int j = 1; j < num_procs_; ++j) {
      if (proc_running[j]) {
        shouldRun = true;
        if (!checkPrint && shouldRun) {
          std::cout << " Proc " << proc_running[j] << " running\n";
          checkPrint = true;
        }
        break; 
      }
    }

    if (shouldRun) {
      if (getWallTime() - last_log_time_status > log_freq_status) {
        updateProcsRunningStatus_(proc_running);
        last_log_time_status = getWallTime();
      }
      if (getWallTime() - last_log_time_ub > log_freq_ub) {
        checkUbUpdates_(proc_running);
        last_log_time_ub = getWallTime();
      }
      if (getWallTime() - last_log_time_lb > log_freq_lb) {
        //checkLbUpdatesFromOtherProcs_(globalBestLb, proc_running);
        checkLbUpdatesFromOtherProcs_(globalBestLb, proc_lb);
        last_log_time_lb = getWallTime();
        //std::cout << "End: globalBestLb " << globalBestLb << "\n";
        showParStatus_(0, globalBestLb, wallTimeStart);
      }
    } else {
      checkUbUpdates_(proc_running);
      //checkLbUpdatesFromOtherProcs_(globalBestLb, proc_running);
      checkLbUpdatesFromOtherProcs_(globalBestLb, proc_lb);
      tm_->setLb(globalBestLb);
      std::cout <<"Final lower bound "<< globalBestLb << " " << tm_->getLb() << "\n";
      std::cout <<"Final upper bound "<< tm_->getUb() << "\n";
      break;
    }
    shouldRun = false;
  }
  
  if (!solved_at_root && num_procs_ > 1) {
    if (tm_->getPerGapPar(globalBestLb) <= 0.0) {
      status_ = SolvedOptimal;
    } else if (globalBestLb < -INFINITY) {
      status_ = SolvedUnbounded;
    } else if ((getWallTime() - wallTimeStart) > options_->timeLimit) {
      status_ = TimeLimitReached;
    } else if (tm_->getUb() == INFINITY) {
      status_ = SolvedInfeasible;
    }
    //TODO: More yet to be added
  }

  std::cout << "Stopping branch-and-bound for proc " << proc_rank_ << "\n";

  //logger_->msgStream(LogExtraInfo) << me_ << "stopping branch-and-bound"
  //  << std::endl
  //  << me_ << "nodes processed = " << stats_->nodesProc << std::endl
  //  << me_ << "nodes created   = " << tm_->getSize() << std::endl;
  //solPool_->writeStats(logger_->msgStream(LogExtraInfo));

  //for (UInt k = 0; k < numThreads; ++k) {
  //  logger_->msgStream(LogExtraInfo) << me_ << "nodes processed by thread "
  //    << k << " = " << nodesProcTh[k] << std::endl;
  //}
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


void DistParBranchAndBound::solveWorkerProc_(ParNodeIncRelaxerPtr parNodeRlxr[],
                                 ParPCBProcessorPtr nodePrcssr[],
                                 UInt numThreads)
{
 //------------------ 
  //int trap_key = 0;
  //MPI_Status status;
  //if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
  //      MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
  ////the critical section 
  //std::cout << "Process " << proc_rank_ << " entered solveWorkerProc_()." << std::endl;
  //// this will allow only the last process to skip this
  //if(proc_rank_ != num_procs_ - 1) {
  //  MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
  //}
 //------------------
  UInt iter;
  UInt index; 
  bool shouldRun = true;
  bool shouldWait = true;
  std::vector<double> bound_changes;

  bool isParRel = false;
  double wallTimeStart = getWallTime();
  double last_log_time_ub = wallTimeStart;
  double last_log_time_lb = wallTimeStart;
  //double last_log_time_status = wallTimeStart;
  double log_freq_ub = 5; // create an options for this later
  double log_freq_lb = 5; // create an options for this later
  //double log_freq_status = 5; // create an options for this later
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
  MPI_Request termination_req, lb_req, ub_req;
  int termination_status = 0;
  double tree_lb, tree_ub;
  std::vector<int> proc_running;
  
  int checkPrint = true;

  //double new_obj_val = INFTY;
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
 //------------------ 
  //trap_key = 0;
  ////MPI_Status status;
  //if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
  //      MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
  ////the critical section 
  //std::cout << "Process " << proc_rank_ << " reached barrier." << std::endl;
  //// this will allow only the last process to skip this
  //if(proc_rank_ != num_procs_ - 1) {
  //  MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
  //}
 ////------------------ 
  //
 ////------------------ 
  //trap_key = 0;
  ////MPI_Status status;
  //if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
  //      MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
  ////the critical section 
  //std::cout << "Process " << proc_rank_ << " " << " var shouldRun " << shouldRun << " " << std::endl;
  //// this will allow only the last process to skip this
  //if(proc_rank_ != num_procs_ - 1) {
  //  MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
  //}
 //------------------ 


  while (shouldWait) {
    if (shouldRun == false) {
      //MPI_Isend(&termination_status, 1, MPI_INT, 0, TAG_Terminate, MPI_COMM_WORLD, &termination_req);
      return;
    } else {
      //std::cout << " Proc " << proc_rank_ << " entering getStartingNode_" << "\n";
      getStartingNode_(&shouldWait, &shouldRun, bound_changes);
      if (int(getWallTime()) % 100 == 500) {
        //std::cout << " Proc " << proc_rank_ << " shouldWait " << shouldWait << "\n";
      }
      //if (shouldWait == false) {
      //  break;
      //}
    }
    //std::cout << "HEREE \n";
  }

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
  stats_ = new DistParBabStats();

  // initialize solution pool
  // TODO: use user options to set the pool size. For now it is 1.
  solPool_ = (SolutionPoolPtr) new SolutionPool(env_, problem_, 1);

  rel[0] = parNodeRlxr[0]->getRelaxation();
  // Apply bound_changes to rel[0]
  iter = 0;
  while(iter < bound_changes.size() - 2) {
    index = static_cast<UInt>(bound_changes[iter]);
    if (bound_changes[iter+1] == 0.0) {
      rel[0]->changeBoundByInd(index, Lower, bound_changes[iter+2]);
    } else {
      rel[0]->changeBoundByInd(index, Upper, bound_changes[iter+2]);
    }
    iter = iter + 3;
  } 

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
    //tm_->updateLb();
    if (tm_->getUb() <= -INFINITY) {
      status_ = SolvedUnbounded;
    } else if (tm_->getUb() < INFINITY) {
      tree_lb = tm_->getLb(); 
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
    //tm_->updateLb();
    tree_lb = tm_->getLb(); 
    shouldRun = false;
  } else {
    tree_lb = tm_->getLb(); 
#if SPEW
    logger_->msgStream(LogDebug) << std::setprecision(8)
      << me_ << "lb = " << tm_->updateLb() << std::endl
      << me_ << "ub = " << tm_->getUb() << std::endl;
#endif
  }
  

  // solve root outside the loop. save the useful information.
  initialized[0] = true; //pseudoCosts for thread0 initialized while doing root
  numVars = rel[0]->getNumVars();
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
 
    std::cout << "Proc " << proc_rank_ << " entering while." << "\n";
    while (nodeCountTh[i] > 0 && shouldRun) {
      if (checkPrint) {
        std::cout << "Proc " << proc_rank_ << " entered while." << "\n";
        checkPrint = false;
      }
      // check MPI-thread binding
      //if (i == 0) {
        //if (getWallTime() - last_log_time_status > log_freq_status) {
        //  updateProcsRunningStatus_();
        //  last_log_time_status = getWallTime();
        //}
        if (getWallTime() - last_log_time_ub > log_freq_ub) {
          checkUbUpdates_(proc_running);
          last_log_time_ub = getWallTime();
        }
        if (getWallTime() - last_log_time_lb > log_freq_lb) {
          tree_lb = tm_->getLb();
          std::cout <<"From worker sent lb: treeLb "<< tree_lb << "\n";
          MPI_Isend(&tree_lb, 1, MPI_DOUBLE, 0, TAG_Lb, MPI_COMM_WORLD, &lb_req);
          last_log_time_lb = getWallTime();
        }
      //}
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
            // Send updated ub to all
            if (solPool_->getBestSolutionValue() < tm_->getUb()) {
              tree_ub = solPool_->getBestSolutionValue();
              MPI_Isend(&tree_ub, 1, MPI_DOUBLE, 0, TAG_Ub, MPI_COMM_WORLD, &ub_req);
              std::cout << "\n\nUB update: Proc " << proc_rank_ << "sent ub to Proc " << 0 << "\n\n";
              //sendUbToMaster_(solPool_->getBestSolutionValue());
            }
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

//      if (shouldStopPar_(wallTimeStart, treeLbTh[i])) {
//#pragma omp critical (treeManager)
//        {
//          tm_->updateLb();
//        }
//      }

      // update stopping conditions
      if (nodeCountTh[i] == 0) {
#pragma omp critical (treeManager)
        {
          tm_->updateLb();
        }
        //globalBestLb = tm_->getLb(); 
        if (tm_->getUb() <= -INFINITY) {
          status_ = SolvedUnbounded;
        } else if (tm_->getUb() < INFINITY) {
          tree_lb = tm_->getLb(); 
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
#pragma omp critical (treeManager)
        {
          tm_->updateLb();
        }
        tree_lb = tm_->getLb(); 
        shouldRun = false;
      } else {
        tree_lb = tm_->getLb(); 
//#if SPEW
//#pragma omp critical (logger)
//        //logger_->msgStream(LogInfo) << "nodesCount " << nodeCountThread << " thread " << i << std::endl;
//        logger_->msgStream(LogDebug) << std::setprecision(8)
//          << me_ << "lb = " << tm_->updateLb() << std::endl
//          << me_ << "ub = " << tm_->getUb() << std::endl;
//#endif
      }

    } //while ends
    std::cout << "Proc " << proc_rank_ << " exited while." << "\n";
#if SPEW
#pragma omp single
    {
      for (UInt j=0; j < numThreads; ++j) {
        logger_->msgStream(LogInfo) << "nodesProc " << nodesProcTh[j] << " thread " << j << std::endl;
      }
    }
#endif
  }   //parallel region ends
  
  std::cout << "Parallel region exit for proc " << proc_rank_ << "\n";
  // Termination message
  termination_status = 1;
  //for (int j = 0; j < num_procs_; ++j) {
  //  if (j != proc_rank_) {
  //    MPI_Isend(&termination_status, 1, MPI_INT, j, TAG_Terminate, MPI_COMM_WORLD, &termination_req);
  //  }
  //}
  //std::cout << "Proc " << proc_rank_ << " sent termination." << "\n";
  //exit(0);
  if (status_ == SolvedOptimal) {
    tm_->setLb(tm_->getUb());
  }
  //if (tree_lb < INFINITY) {
  //  MPI_Isend(&tree_lb, 1, MPI_DOUBLE, 0, TAG_Lb, MPI_COMM_WORLD, &lb_req);
  //}
  tree_lb = tm_->getLb();
  MPI_Isend(&tree_lb, 1, MPI_DOUBLE, 0, TAG_Lb, MPI_COMM_WORLD, &lb_req);
  MPI_Isend(&termination_status, 1, MPI_INT, 0, TAG_Terminate, MPI_COMM_WORLD, &termination_req);

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
 //------------------ 
  //trap_key = 0;
  ////MPI_Status status;
  //if (proc_rank_ != trap_key)  //this will allow only process 0 to skip this stmt
  //      MPI_Recv(&trap_key, 1, MPI_INT, proc_rank_ - 1, 0, MPI_COMM_WORLD, &status);
  ////the critical section 
  //std::cout << "Process " << proc_rank_ << " exiting parsolveOppor()." << std::endl;
  //// this will allow only the last process to skip this
  //if(proc_rank_ != num_procs_ - 1) {
  //  MPI_Send(&trap_key, 1, MPI_INT, proc_rank_ + 1, 0, MPI_COMM_WORLD);
  //}
 //------------------
}

void DistParBranchAndBound::getBoundChanges_(NodePtr node, std::vector<double>& bound_changes)
{
  bound_changes.resize(0);
  BranchPtr branch;
  std::stack<NodePtr> predecessors;
  NodePtr t_node = node->getParent();
  //std::cout << " t_node (parent) " << t_node->getId() << std::endl;
  VarBoundModPtr mod;
  ModificationConstIterator mod_iter;

  // including the node itself in the predecessors stack
  predecessors.push(node);
  while (t_node) {
    predecessors.push(t_node);
    t_node = t_node->getParent();
  }

  //std::cout << " predecessors " << predecessors.size() << std::endl;
  while (!predecessors.empty()) {
    t_node = predecessors.top();
    //std::cout << " t_node " << t_node->getId() << std::endl;
    branch = t_node->getBranch();
    //std::cout << " branch " << branch << std::endl;
    if (branch) {
      for (mod_iter=branch->rModsBegin(); mod_iter!=branch->rModsEnd(); 
          ++mod_iter) {
        mod = dynamic_cast <VarBoundMod *> (*mod_iter);
        bound_changes.push_back(mod->getVar()->getIndex());
        if (mod->getLU() == Lower) {
          bound_changes.push_back(0);
        } else {
          bound_changes.push_back(1);
        }
        bound_changes.push_back(mod->getNewVal());
      }
    }
    predecessors.pop();
  }

  //std::cout << " bound_changes " << bound_changes.size() << std::endl;
  //exit(0);
  return; 
}

void DistParBranchAndBound::updateProcsRunningStatus_(std::vector<int>& proc_running)
{
  double value;
  int flag;
  MPI_Status mpi_status;
  //MPI_Request request;

  for (int i = 1; i < num_procs_; ++i) {
    if (i != proc_rank_) {
      if (proc_running[i]) {
        MPI_Iprobe(i, TAG_Terminate, MPI_COMM_WORLD, &flag, &mpi_status);
        if (flag) {
          //MPI_Irecv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &request);
          MPI_Recv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          std::cout << "\n\nProc " << proc_rank_ << " terminating\n\n";
          proc_running[i] = 0; 
        }
      }
    } 
  }
//  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &is_msg, &mpi_status);
//  if (is_msg == false) {
//    return;
//  }
//  MpiMessageTag tag = static_cast<MpiMessageTag>(mpi_status.MPI_TAG);
//  if (tag == UB) {
//    MPI_Recv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//    if (value < tm_->getUb())
//      tm_->setUb(value);
//  } 
//  //else if (tag == LB) {
//  //
//  //} 
  return;
}


//void DistParBranchAndBound::checkLbUpdatesFromOtherProcs_(double& globalBestLb, const std::vector<int>& proc_running)
void DistParBranchAndBound::checkLbUpdatesFromOtherProcs_(double& globalBestLb, std::vector<double>& proc_lb)
{
  double value;
  int flag;
  MPI_Status mpi_status;
  //MPI_Request request;

  //globalBestLb = tm_->getLb();
  for (int i = 1; i < num_procs_; ++i) {
    //std::cout << " Proc " << proc_rank_ << " " << " running " << proc_running[i] << "\n";
    //while (true && proc_running[i]) {
    while (true) {
      MPI_Iprobe(i, TAG_Lb, MPI_COMM_WORLD, &flag, &mpi_status);
      if (flag) {
        //MPI_Irecv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &request);
        //std::cout << " Proc " << proc_rank_  << " about to receive in checkLbUpdatesFromOtherProcs_ " << "\n";
        MPI_Recv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << " Proc " << proc_rank_  << " received " << value << " in checkLbUpdatesFromOtherProcs_ " << "\n";
        proc_lb[i] = value;
        //globalBestLb = std::min(globalBestLb, value);
        std::cout <<"From checkLbUpdates_: globalBestLb and value "<< globalBestLb << " " << value << "\n";
      } else {
        break;
      }
    }
  }
  proc_lb[0] = tm_->getLb();
  globalBestLb = *(std::min_element(proc_lb.begin(), proc_lb.end()));
  return; 
}


void DistParBranchAndBound::checkUbUpdates_(const std::vector<int>& proc_running)
{
  double value;
  int flag;
  MPI_Status mpi_status;
  MPI_Request req;
  //MPI_Request request;

  if (proc_rank_ == 0) {
    for (int i = 1; i < num_procs_; ++i) {
      MPI_Iprobe(i, TAG_Ub, MPI_COMM_WORLD, &flag, &mpi_status);
      if (flag) {
        //MPI_Irecv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &request);
        MPI_Recv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "\n\nUB update: Proc " << proc_rank_ << " received ub from Proc " << i << "\n\n";
        if (value < tm_->getUb()) {
          for (int j = 1; j < num_procs_; ++j) {
            if (j != i && proc_running[j]) {
              MPI_Isend(&value, 1, MPI_DOUBLE, j, TAG_Ub, MPI_COMM_WORLD, &req);
              std::cout << "\n\nUB update: Proc " << proc_rank_ << "sent ub to Proc " << j << "\n\n";
            }
          }
#pragma omp critical (treeManager)
          {
            tm_->setUb(value); 
          }      
        }
      } else {
        break;
      } 
    }
  } else {
    while (true) {
      MPI_Iprobe(0, TAG_Ub, MPI_COMM_WORLD, &flag, &mpi_status);
      if (flag) {
        //MPI_Irecv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &request);
        MPI_Recv(&value, 1, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "\n\nUB update: Proc " << proc_rank_ << "received ub from Proc " << 0 << "\n\n";
        if (value < tm_->getUb()) {
#pragma omp critical (treeManager)
          {
            tm_->setUb(value);       
          }
        }
      } else {
        break;
      } 

    }
  }
  return;
}


void DistParBranchAndBound::sendUbToOtherProcs_(double val, const std::vector<int>& proc_running)
{
  MPI_Request req;
  for (int i = 1; i < num_procs_; ++i) {
    std::cout << " inside sendUb: proc running " << i << " " << proc_running[i] << "\n";
    if (proc_running[i]) {
      std::cout << "\n\nUB update: Proc " << proc_rank_ << "sent ub to Proc " << i << "\n\n";
      MPI_Isend(&val, 1, MPI_DOUBLE, i, TAG_Ub, MPI_COMM_WORLD, &req);
    }
  }
  return;
}


void DistParBranchAndBound::distributeNodes_() 
{
  int count;
  NodePtr node = 0;
  
  std::vector<double> bound_changes;
  //std::cout << "In distribute() tm size " << tm_->getSize() << "\n";
  std::cout << " Num procs " << num_procs_ << "\n";
  for (int i = 1; i < num_procs_; ++i) {
    //std::cout << "Inside\n";
    node = tm_->getCandidate();
    std::cout <<"Node " << node << "\n";
    if (node) {
      std::cout << " Proc " << proc_rank_ << " about to send node " << node->getId() << " with tag " << TAG_NodeFound << " to proc " << i << "\n";
      getBoundChanges_(node, bound_changes);
      count = bound_changes.size();
      //std::cout << "Before Send\n";
      //std::cout << "Count " << count << std::endl;
      MPI_Send(&bound_changes, count, MPI_DOUBLE, i, TAG_NodeFound, MPI_COMM_WORLD);
      tm_->removeActiveNode(node);
      std::cout << " Proc " << proc_rank_ << " sent node, tm size " << tm_->getActiveNodes() << "\n";
    }
  }
 return; 
}


void DistParBranchAndBound::getStartingNode_(bool *shouldWait, bool *shouldRun, std::vector<double>& node_data)
{
  int count;
  int flag;
  MPI_Status mpi_status;
  //bool checkProbe = true;
  //bool checkRecv = true;

  MPI_Iprobe(0, TAG_Terminate, MPI_COMM_WORLD, &flag, &mpi_status);
  //printf(" iprobe MPI process %d from rank %d, with tag %d and error code %d.\n", proc_rank_, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, mpi_status.MPI_ERROR);
  if (flag) {
    //std::cout << " iprobe Proc " << proc_rank_ << " flag received\n";
    *shouldRun = false;
  } else {
    MPI_Iprobe(0, TAG_NodeFound, MPI_COMM_WORLD, &flag, &mpi_status);
    //return;
    if (flag) {
      //std::cout << " iprobe2 Proc " << proc_rank_ << " flag received\n";
      MPI_Get_count(&mpi_status, MPI_DOUBLE, &count);
      node_data.resize(count);
      //std::cout << " Before Recv\n";
      //std::cout << " Count " << count << std::endl;
      MPI_Recv(node_data.data(), count, MPI_DOUBLE, 0, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
      std::cout << " Proc " << proc_rank_ << " received node\n";
      *shouldWait = false;
    }
  }
  
  return;
  //if (checkProbe) {
    //std::cout << " iprobe Proc " << proc_rank_ << " waiting for flag " << TAG_NodeFound << "\n";
    //checkProbe = false;
  //}
  //MPI_Iprobe(0, TAG_NodeFound, MPI_COMM_WORLD, &flag, &mpi_status);
  ////MPI_Probe(0, TAG_NodeFound, MPI_COMM_WORLD, &mpi_status);
  ////std::cout << " Proc " << proc_rank_ << " iprobe2 status " << mpi_status << "\n";
  ////printf(" iprobe2 MPI process %d from rank %d, with tag %d and error code %d.\n", proc_rank_, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, mpi_status.MPI_ERROR);
 //else {
  //  return;
  //}
  
  //MPI_Probe(0, TAG_Wait, MPI_COMM_WORLD, &flag, &mpi_status);
  //if (flag) {
  //
  //}

  
 
  //std::cout <<"TAG " << tag << std::endl;
  //if (tag == Terminate) {
  //  *shouldRun = false;
  //  *shouldWait = false;
  //} else if (tag == NodeFound) {
  //  MPI_Get_count(&mpi_status, MPI_CHAR, &count);
  //  node_data.resize(count);
  //  std::cout << "Before Recv\n";
  //  std::cout << "Count " << count << std::endl;
  //  MPI_Recv(node_data.data(), count, MPI_DOUBLE, 0, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
  //  std::cout << "After Recv\n";
  //  *shouldWait = false;
  //} else if (tag == Wait) {
  //  //logger_->msgStream(LogInfo) << me_ << "Continue wait."
  //            //<< std::endl; 
  //}
  //return;
}

void DistParBranchAndBound::writeStats(std::ostream &out)
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

void DistParBranchAndBound::writeParStats(std::ostream &out, ParPCBProcessorPtr nodePrcssr[])
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

double DistParBranchAndBound::totalTime()
{
  return stats_->timeUsed;
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

  DistParBabStats::DistParBabStats()
:nodesProc(0),
  timeUsed(0),
  updateTime(0)
{
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
  DistParBabOptions::DistParBabOptions()
: createRoot(true),
  nodeLimit(0),
  perGapLimit(0.),
  solLimit(0),
  timeLimit(0.)

{
}


DistParBabOptions::DistParBabOptions(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();

  logInterval = options->findDouble("log_interval")->getValue();
  nodeLimit   = options->findInt("node_limit")->getValue();
  perGapLimit = options->findDouble("obj_gap_percent")->getValue();
  solLimit    = options->findInt("sol_limit")->getValue();
  timeLimit   = options->findDouble("time_limit")->getValue();
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
