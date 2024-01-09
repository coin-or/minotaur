// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

/**
 * \file MsProcessor.cpp
 * \brief Implement simple multi-start node-processor for branch-and-bound
 * \author Prashant Palkar, IIT Bombay
 */
#include <cmath> // for INFINITY
#include <ctime> // for Windows
#if USE_OPENMP
#include <omp.h>
#endif
#include <complex>
#include "MinotaurConfig.h"
#include "Brancher.h"
#include "Engine.h"
#include "Environment.h"
#include "Handler.h"
#include "MsProcessor.h"
#include "Logger.h"
#include "Node.h"
#include "Option.h"
#include "Modification.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Operations.h"

using namespace Minotaur;

//#define SPEW 1

const std::string MsProcessor::me_ = "MsProcessor: ";

MsProcessor::MsProcessor(EnvPtr env)
: contOnErr_(false),
  cutOff_(INFINITY),
  engine_(EnginePtr()),
  engineStatus_(EngineUnknownStatus),
  env_(env),
  numSolutions_(0),
  relaxation_(RelaxationPtr()),
  ws_(WarmStartPtr())
{
  handlers_.clear();
  logger_ = env_->getLogger();
  stats_.inf = 0;
  stats_.opt = 0;
  stats_.prob = 0;
  stats_.proc = 0;	
  stats_.ub = 0;
}


MsProcessor::MsProcessor (EnvPtr env, EnginePtr engine,
                          HandlerVector handlers)
: contOnErr_(false),						
  engine_(engine),
  engineStatus_(EngineUnknownStatus),
  env_(env),
  numSolutions_(0),
  relaxation_(RelaxationPtr()),
  ws_(WarmStartPtr())
{
  cutOff_ = env->getOptions()->findDouble("obj_cut_off")->getValue();
  handlers_ = handlers;
  logger_ = env->getLogger();
  numRestarts_ = env->getOptions()->findInt("msbnb_restarts")->getValue();
  numThreads_ = env->getOptions()->findInt("threads")->getValue();
  schemeId_ = env->getOptions()->findInt("msbnb_scheme_id")->getValue();
  stats_.bra = 0;
  stats_.inf = 0;
  stats_.opt = 0;
  stats_.prob = 0;
  stats_.proc = 0;
  stats_.ub = 0;
}


MsProcessor::~MsProcessor()
{
  handlers_.clear();
}


bool MsProcessor::foundNewSolution()
{
  return (numSolutions_ > 0);
}


Branches MsProcessor::getBranches()
{
  ++stats_.bra;
  return branches_;
}


WarmStartPtr MsProcessor::getWarmStart()
{
  return ws_;
}


bool MsProcessor::isFeasible_(NodePtr node, ConstSolutionPtr sol, 
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

  if (is_feas == true && h == handlers_.end()) {
    s_pool->addSolution(sol);
    ++numSolutions_;
    node->setStatus(NodeOptimal);
    ++stats_.opt;
    should_prune = true;
  }
  return is_feas;
}

double MsProcessor::InnerProduct(double b[], double c[], UInt n)
{
  double dotProd = 0;
  for(UInt i = 0; i < n; i++) {
    dotProd += b[i]*c[i];
  }
  return dotProd;
}

double EDist(double b[], double c[], UInt n)
{
  double dist = 0;
  for(UInt i = 0; i < n; i++) {
    dist += (b[i]-c[i])*(b[i]-c[i]);
  }
  return sqrt(dist);
}

double MsProcessor::ENorm(double b[], UInt n)
{
  double norm = 0;
  for(UInt i = 0; i < n; i++) {
    norm+= b[i]*b[i];        
  }
  return sqrt(norm);
}

double * MsProcessor::getBoxCorner(UInt n, RelaxationPtr rel1, int threadId,
                                   int K)
{
  double* cornerPoint = new double[n];
  srand(time(NULL));
  double lb, ub, lbdbVar, ubdbVar, largestInterval = -INFINITY;
  
  int dbVar = -1;     //index of a doubly bounded variable
  for(UInt k = 0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if (lb > -INFINITY) {
      if (ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if ((ub - lb) > largestInterval) {
          dbVar = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if (dbVar >= 0) {
    lbdbVar = rel1->getVariable(dbVar)->getLb();
    ubdbVar = rel1->getVariable(dbVar)->getUb();
    double width = (ubdbVar - lbdbVar)/numThreads_;

    for(UInt j = 0; j < n; j++) {
      if ((int)j != dbVar) {
        lb = rel1->getVariable(j)->getLb();
        ub = rel1->getVariable(j)->getUb();
        if (lb > -INFINITY) {
          if (ub < INFINITY) {
            if (rand()/2 == 0) {
              cornerPoint[j] = lb;
            } else {
              cornerPoint[j] = ub;
            }
          } else {
            cornerPoint[j] = lb + rand()%K;
          }
        } else {
          if (ub < INFINITY) {
            cornerPoint[j] = ub - rand()%K;
          } else {
            cornerPoint[j] = rand()%K;
          }
        }
      } else {
        lb = lbdbVar + (threadId)*width;
        ub = lb + width;
        if (rand()/2 == 0) {
          cornerPoint[dbVar] = lb;
        } else {
          cornerPoint[dbVar] = ub;
        }
      }
    }
  } else {
    cornerPoint = getStartPointScheme1(n, rel1);
  }
  return cornerPoint;
}


double * MsProcessor::getFarBoxCorner(UInt n, RelaxationPtr rel1,
                                      int threadId, double* prevOpt, int K)
{

  double* cornerPoint = new double[n];
  srand(time(NULL));
  double lb, ub, lbdbVar, ubdbVar, largestInterval = -INFINITY;
  int dbVar = -1;
  for(UInt k = 0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if (lb > -INFINITY) {
      if (ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if ((ub - lb) > largestInterval) {
          dbVar = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if (dbVar >= 0) {
    lbdbVar = rel1->getVariable(dbVar)->getLb();
    ubdbVar = rel1->getVariable(dbVar)->getUb();
    double width = (ubdbVar - lbdbVar)/numThreads_;

    for(UInt j = 0; j < n; j++) {
      if ((int)j!=dbVar) {
        lb = rel1->getVariable(j)->getLb();
        ub = rel1->getVariable(j)->getUb();
        if (lb > -INFINITY) {
          if (ub < INFINITY) {
            if (abs(ub - prevOpt[j]) > abs(prevOpt[j] - lb)) {
              cornerPoint[j] = ub;
            } else {
              cornerPoint[j] = lb;
            }
          } else {
            cornerPoint[j] = lb + rand()%K;
          }
        } else {
          if (ub < INFINITY) {
            cornerPoint[j] = ub - rand()%K;
          } else {
            cornerPoint[j] = rand()%K;
          }
        }
      } else {
        lb = lbdbVar + (threadId)*width;
        ub = lb + width;
        if (abs(ub - prevOpt[j]) > abs(prevOpt[j] - lb)) {
          cornerPoint[j] = ub;
        } else {
          cornerPoint[j] = lb;
        }
      }
    }
  } else {
    cornerPoint = getStartPointScheme1(n, rel1);
  }
  return cornerPoint;
}

double * MsProcessor::getStartPointScheme1(UInt n, RelaxationPtr rel1)
{
  double* initPoint = new double[n];
  srand(time(NULL));
  double lb, ub;
  for(UInt k = 0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();

    if (lb > -INFINITY) {
      if (ub < INFINITY) {
        initPoint[k] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
      } else {
        initPoint[k] = lb + rand()%1000;
      }	
    } else {
      if (ub < INFINITY) {
        initPoint[k] = ub - rand()%1000;
      } else {
        initPoint[k] = rand()%1000;
      }
    }
  }
  return initPoint;
}

double* MsProcessor::getStartPointScheme2(UInt n, RelaxationPtr rel1, 
                                        int threadId, double radius,
                                        int numSols,
                                        double* prevStartPoint)
{
  double* initPoint = new double[n];
  srand(time(NULL));
  double lb, ub, lbdbVar, ubdbVar, norm, largestInterval = -INFINITY;
  double *a = new double [n];
  int dbVar = -1;

  for(UInt k = 0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if (lb > -INFINITY) {
      if (ub < INFINITY) {
        //(first) doubly bdd variable found
        if ((ub - lb) > largestInterval) {
          dbVar = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if (dbVar >= 0) {
    lbdbVar = rel1->getVariable(dbVar)->getLb();
    ubdbVar = rel1->getVariable(dbVar)->getUb();
    double width = (ubdbVar - lbdbVar)/numThreads_;

    if (numSols == 0) {
      //finding the initial point
      for(UInt j = 0; j < n; j++) {
        if ((int)j != dbVar) {
          lb = rel1->getVariable(j)->getLb();
          ub = rel1->getVariable(j)->getUb();

          if (lb > -INFINITY) {
            if (ub < INFINITY) {
              initPoint[j] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
            } else {
              initPoint[j] = lb + rand()%1000;
            }	
          } else {
            if (ub < INFINITY) {
              initPoint[j] = ub - rand()%1000;
            } else {
              initPoint[j] = rand()%1000;
            }
          }
        }
      }
      //generating (dbVar)th coordinate within thread specific bound
      lb = lbdbVar + (threadId)*width;
      ub = lb + width;
      initPoint[dbVar] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
      return initPoint;
    } 
    //else if there exists a previous starting point:
    //generate a random point outside a ball
    else {
      for (UInt i = 0; i < n; ++i) {
        a[i] = ((double)rand())/((double)(RAND_MAX));
      }
      norm = ENorm(a,n);
      for (UInt i = 0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i = 0; i < n; i++) {
        if ((int)i!=dbVar) {
          lb = rel1->getVariable(i)->getLb();
          ub = rel1->getVariable(i)->getUb();
          initPoint[i] = std::max(std::min(prevStartPoint[i]
                                           + (a[i])*radius, ub), lb);
        }
      }
      lb = lbdbVar + (threadId)*width;
      ub = lb + width;
      initPoint[dbVar] = std::max(std::min(prevStartPoint[dbVar]
                                           + (a[dbVar])*radius, ub), lb);
    }
  } else {
    //for singly bounded and unbdd variables
    if (numSols == 0) {
      initPoint = getStartPointScheme1(n, rel1);
      delete[] a;
      return initPoint;
    } else {
      //generate a random point outside the radius
      for (UInt i = 0; i < n; ++i) {
        a[i] = ((double)rand())/((double)(RAND_MAX));        
      }
      norm = ENorm(a,n);
      for (UInt i = 0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i = 0; i < n; i++) {
        lb = rel1->getVariable(i)->getLb();
        ub = rel1->getVariable(i)->getUb();
        initPoint[i] = std::max( std::min(prevStartPoint[i] + (a[i])*radius,
                                           ub), lb);
      }
    }
  }
  delete[] a;
  return initPoint;
}


double * MsProcessor::getStartPointScheme4(UInt n, RelaxationPtr rel1,
                                           int threadId, double radius,
                                           int numSols,
                                           double* prevStartPoint,
                                           double* prevOpt,
                                           double cosThrshldAngle)
{
  double* initPoint = new double[n];
  srand(time(NULL));
  double lb, ub, lbdbVar, ubdbVar, norm, largestInterval = -INFINITY;
  double* a = new double [n];        //random direction
  double* prevDir = new double [n]; //prev iter. direction
  int dbVar = -1;                    //index of a doubly bounded variable
  double cosAngle = 1;               //cos of angle between 'prevDir' and 'a'

  for(UInt k = 0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if (lb > -INFINITY) {
      if (ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if ((ub - lb) > largestInterval) {
          dbVar = k;
          largestInterval = ub - lb;
        }
      }
    }
  }
  if (dbVar >= 0 ) {
    lbdbVar = rel1->getVariable(dbVar)->getLb();
    ubdbVar = rel1->getVariable(dbVar)->getUb();
    double width = (ubdbVar - lbdbVar)/numThreads_;
    if (numSols == 0) {
      //getting the initial point
      for(UInt j = 0; j < n; j++) {
        if ((int)j != dbVar) {
          lb = rel1->getVariable(j)->getLb();
          ub = rel1->getVariable(j)->getUb();

          if (lb > -INFINITY) {
            if (ub < INFINITY) {
              initPoint[j] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
            } else {
              initPoint[j] = lb + rand()%1000;
            }	
          } else {
            if (ub < INFINITY) {
              initPoint[j] = ub - rand()%1000;
            } else {
              initPoint[j] = rand()%1000;
            }
          }
        }
      }
      //generating (dbVar)th coordinate within thread specific bound
      lb = lbdbVar + (threadId)*width;
      ub = lb + width;
      initPoint[dbVar] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
      delete[] a;
      delete[] prevDir;
      return initPoint;
    } else {
    //else if there exists a previous starting point:
    //generate a random point outside the radius in conjugate direction
      while(cosAngle < cosThrshldAngle) {
        for (UInt i = 0; i < n; ++i) {
          a[i] = ((double)rand())/((double)(RAND_MAX));        
        }
        //find the angle between the two directions
        for (UInt i = 0; i < n; ++i) {
          prevDir[i] = prevOpt[i] - prevStartPoint[i];
        }
        cosAngle = InnerProduct(a, prevDir,n)/((ENorm(a,n))*(ENorm(prevDir,n)));
      }
      norm = ENorm(a,n);
      for (UInt i = 0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i = 0; i < n; i++) {
        if ((int)i != dbVar) {
          lb = rel1->getVariable(i)->getLb();
          ub = rel1->getVariable(i)->getUb();
          initPoint[i] = std::max(std::min(prevStartPoint[i]
                                            +(a[i])*radius, ub), lb);
        }
      }
      lb = lbdbVar + (threadId)*width;
      ub = lb + width;
      initPoint[dbVar] = std::max(std::min(prevStartPoint[dbVar]
                                              +(a[dbVar])*radius, ub),lb);
    }
  } else {
    //for singly bounded and unbdd variables
    if (numSols == 0) {
      initPoint = getStartPointScheme1(n, rel1);
      delete[] a;
      delete[] prevDir;
      return initPoint;
    } else {
      //generate a random point outside the radius
      while(cosAngle < cosThrshldAngle) {
        for (UInt i = 0; i < n; ++i) {
          a[i] = ((double)rand())/((double)(RAND_MAX));        
        }
        //find the angle between the two directions
        for (UInt i = 0; i < n; ++i) {
          prevDir[i] = prevOpt[i] - prevStartPoint[i];
        }
        cosAngle = InnerProduct(a, prevDir,n)/((ENorm(a,n))*(ENorm(prevDir,n)));
      }
      norm = ENorm(a,n);
      for (UInt i = 0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i = 0; i < n; i++) {
        lb = rel1->getVariable(i)->getLb();
        ub = rel1->getVariable(i)->getUb();
        initPoint[i] = std::max(std::min(prevStartPoint[i] + (a[i])*radius, ub)
                                , lb);
      }
    }
  }
  delete[] a;
  delete[] prevDir;
  return initPoint;
}

double * MsProcessor::getStartPointScheme5 (UInt n, RelaxationPtr rel1,
                                            int threadId, double radius,
                                            double* prevStartPoint,
                                            double* prevOpt, int K,
                                            double lambda)
{
  double* initPoint = new double[n];
  srand(time(NULL));
  double lb, ub, lbdbVar = -INFINITY, ubdbVar = INFINITY;
  double width = 0, norm, largestInterval = -INFINITY;
  double* newDir = new double [n];     //random direction
  double* tempCorner = new double [n];  //new iteration direction
  int dbVar = -1;                       //index of a doubly bounded variable
  double* farCorner;

  for(UInt k = 0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if (lb > -INFINITY) {
      if (ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if ((ub - lb) > largestInterval) {
          dbVar = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  //get distance r of prev start from previous optimal
  radius = getDistance(prevStartPoint, prevOpt, n);

  //get farthest box corner
  farCorner = getFarBoxCorner(n, rel1, threadId, prevOpt, K);

  //get the direction unit vector of prev opt from farboxcorner
  for (UInt i = 0; i < n; i++) {
    newDir[i] = farCorner[i] - prevOpt[i];
  }
  norm = ENorm(newDir, n);
  for (UInt i = 0; i < n; i++) {
    newDir[i] = newDir[i]/norm;
  }
  //find a point at a distance = radius in this direction
  for (UInt i = 0; i < n; i++) {
    if ((int)i == dbVar) {    //this considers dbVar >= 0 also
      lbdbVar = rel1->getVariable(dbVar)->getLb();
      ubdbVar = rel1->getVariable(dbVar)->getUb();
      width = (ubdbVar - lbdbVar)/numThreads_;
      lb = lbdbVar + (threadId)*width;
      ub = lb + width;
    } else {
      lb = rel1->getVariable(i)->getLb();
      ub = rel1->getVariable(i)->getUb();
    }
    tempCorner[i] = std::max(std::min(prevOpt[i] + radius*
                                      newDir[i], ub), lb);
  }

  //take a convex combination of this point and farboxcorner as the start
  //point and bring it within variable bounds
  if (lambda == -1) {
    lambda = ((double)rand())/((double)(RAND_MAX));
  }
  for (UInt i = 0; i < n; i++) {
    if ((int)i == dbVar) {
      lb = lbdbVar + (threadId)*width;
      ub = lb + width;
    } else {
      lb = rel1->getVariable(i)->getLb();
      ub = rel1->getVariable(i)->getUb();
    }
    initPoint[i] = std::max(std::min(tempCorner[i]+(1-lambda)*
                                      (farCorner[i]-tempCorner[i]), ub), lb);
  }
  delete[] newDir;
  delete[] tempCorner;
  delete[] farCorner;
  return initPoint;
}


double * MsProcessor::genInitialPoint(UInt n, RelaxationPtr rel1)
{
  double* initPoint = new double[n];
  EngineStatus status;
  RelaxationPtr relc = (RelaxationPtr) new Relaxation(rel1, env_);
  EnginePtr ec = engine_->emptyCopy();
  ec->clear();
  VariablePtr v;
  VarBoundMod2 *mod = 0;
  for (VariableConstIterator i = relc->varsBegin(); i != relc->varsEnd(); i++) {
    v = *i;
    if (v->getType() == Binary || v->getType() == Integer) {
      mod = new VarBoundMod2(v, v->getLb(), v->getLb());
      mod->applyToProblem(rel1);
      relMods_.push(mod);
    }
  }
  relc->prepareForSolve();
  ec->load(relc);

  solveRelaxation_(ec);
  status = ec->getStatus();
  if (ProvenInfeasible==status || ProvenLocalInfeasible==status
       || ProvenObjectiveCutOff==status || ProvenFailedCQInfeas==status
       || FailedInfeas==status || FailedFeas==status
       || ProvenFailedCQFeas==status || ProvenLocalOptimal==status) {
    initPoint = (double*)ec->getSolution()->getPrimal();
  }
  return initPoint;
}

void MsProcessor::process(NodePtr node, RelaxationPtr rel,
                          SolutionPoolPtr s_pool)
{
  bool should_prune = false;
  bool should_resolve;
  BrancherStatus br_status;
  ConstSolutionPtr sol;
  ModVector mods;
  int iter = 0;

  ++stats_.proc;
  relaxation_ = rel;
  UInt numVars = rel->getNumVars();     //number of variables in the problem
  RelaxationPtr *relCopy = new RelaxationPtr[numThreads_];
  EnginePtr* eCopy = new EnginePtr[numThreads_];
  EngineStatus* eStatus = new EngineStatus[numThreads_];
  int* solCount = new int[numThreads_]; //#solutions obtained at each thread

  ConstSolutionPtr bestsolthd = 0;             //thread-best solution
  ConstSolutionPtr bestsol = 0;            //best solution among all threads

  double threadBestVal = INFINITY, bestVal = INFINITY;
  double *startPoint = NULL;
  double *prevStartPoint = NULL;
  double *prevOpt = NULL;
  double radScal = 1.5;                 //factor for scaling radius
  double cosThrshldAngle = sqrt(3)/2;   //threshold angle set to 30 degree
  int K = 1000;                         //upper bound for rand. no. generated
  double lambda = -1;                   //value for taking convex combination
  double prevSolVal = INFINITY;         //previous solution value

#if USE_OPENMP
#pragma omp parallel for   
#endif
  for(UInt i = 0; i < numThreads_; i++) {
    relCopy[i] = (RelaxationPtr) new Relaxation(rel, env_);
    eCopy[i] = engine_->emptyCopy();
    eCopy[i]->clear();
    relCopy[i]->prepareForSolve();
    eCopy[i]->load(relCopy[i]);
    solCount[i] = 0;
  }

  // loop for branching and resolving if necessary.
  while (true) {			           
    ++iter;
    should_resolve = false;

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "iteration " << iter << std::endl;
#endif

    //Solving numThreads_ copies of relaxations (parallel region begin)
#if USE_OPENMP
#pragma omp parallel for 
#endif
    for(UInt i = 0; i < numThreads_; i++) {
      ConstSolutionPtr sol1 = 0;
      double radius = 0.8;                //initial radius of region

      for(UInt j = 0; j < 1 + numRestarts_; j++) {

        //Initial point generation schemes
        switch(schemeId_) {
        case 1:
          //Scheme 1: generate random points within variable bounds
          startPoint = getStartPointScheme1(numVars, relCopy[i]);
          break;

        case 2:
          //Scheme 2: generate a random start point outside a radius from prev
          //start point; enlarge the region radius if no improvement
          if (solCount[i] > 1) {
            if (sol1->getObjValue() == prevSolVal) {
              radius *= radScal;
            }
          }
          startPoint = getStartPointScheme2(numVars, relCopy[i], i, radius,
                                           solCount[i], prevStartPoint);
          break;

        case 3:
          //Scheme 2: generate a random start point outside a radius from prev
          //optimal point; take the region radius > dist(startpt, optimal) 
          if (solCount[i] > 1) {
            if (sol1->getObjValue() == prevSolVal) {
              radius = radScal*getDistance(prevStartPoint, prevOpt, numVars);
            } else {
              radius = getDistance(prevStartPoint, prevOpt, numVars);
            }
            startPoint = getStartPointScheme2(numVars, relCopy[i], i, radius,
                                             solCount[i], prevOpt);
          } else {//pass the previous start point
            startPoint = getStartPointScheme2(numVars, relCopy[i], i, radius,
                                             solCount[i], prevStartPoint);
          }
          break;

        case 4:
          //Scheme 4: pass previous optimal to get a possibly conjugate
          //direction from the previous starting point
          if (solCount[i] > 1) {
            if (sol1->getObjValue() == prevSolVal) {
              radius = radScal*getDistance(prevStartPoint, prevOpt, numVars);
            } else {
              radius = getDistance(prevStartPoint, prevOpt, numVars);
            }
          }
          startPoint = getStartPointScheme4(numVars, relCopy[i], i, radius,
                                             solCount[i], prevStartPoint,
                                             prevOpt, cosThrshldAngle);
          break;

        case 5:
          //Scheme 5: take convex combination of prev optimal and the farthest
          //box corner point from the optimal
          if (solCount[i] > 1) {
            if (sol1->getObjValue() == prevSolVal) {
              startPoint = getStartPointScheme1(numVars, relCopy[i]);
            } else {
              startPoint = getStartPointScheme5(numVars, relCopy[i], i,
                                                  radius, prevStartPoint,
                                                  prevOpt, K, lambda);
            }
          } else if (solCount[i] == 1) {
            startPoint = getStartPointScheme5(numVars, relCopy[i], i,
                                               radius, prevStartPoint,
                                               prevOpt, K, lambda);

          } else {
              startPoint = getBoxCorner(numVars, relCopy[i], i, K);
          }
          break;

        default:
          startPoint = getStartPointScheme1(numVars,relCopy[i]);
          break;
        }          

        relCopy[i]->setInitialPoint(startPoint);
        solveRelaxation_(eCopy[i]);
        eStatus[i] = eCopy[i]->getStatus();

        if (ProvenOptimal == eStatus[i] || ProvenLocalOptimal == eStatus[i]) {
          sol1 = eCopy[i]->getSolution();
          prevOpt = (double*)sol1->getPrimal();

          if (sol1->getObjValue() < threadBestVal) {
            bestsolthd = (SolutionPtr) new Solution(sol1);
            threadBestVal = sol1->getObjValue();
          }
          prevSolVal = sol1->getObjValue();
          solCount[i]++;
        } 
#if SPEW
        else if (ProvenInfeasible==eStatus[i]
                || ProvenLocalInfeasible==eStatus[i]
                || ProvenObjectiveCutOff==eStatus[i]
                || ProvenFailedCQInfeas==eStatus[i]
                || FailedInfeas==eStatus[i] || FailedFeas==eStatus[i]
                || ProvenFailedCQFeas==eStatus[i]) {
          logger_->msgStream(LogDebug2) << me_
            <<"Relaxation infeasible from this starting point!" << std::endl;
        } else if (ProvenUnbounded==eStatus[i]) {
          logger_->msgStream(LogDebug2) << me_
            <<"Relaxation unbounded from this starting point!" << std::endl;
        } else {
          logger_->msgStream(LogDebug2) << me_
            <<"Unknown status" << std::endl;
        }
#endif
        prevStartPoint = startPoint;
      }
#if USE_OPENMP
#pragma omp critical
#endif
      {
        if (solCount[i] > 0) {
#if SPEW
          logger_->msgStream(LogDebug2) << me_ << " best value at thread "
#if USE_OPENMP
            << omp_get_thread_num()
#endif
            << "= threadBestVal"<<std::endl;
#endif
          //5. Compare with/obtain the best solution (thread critical region)
          if (threadBestVal < bestVal) {
            if (bestsol) {
              delete bestsol; bestsol=0;
            }
            bestsol = (SolutionPtr) new Solution(bestsolthd);
            bestVal = threadBestVal;
          }
        } else {
          should_prune = shouldPrune_(node,
                                      eCopy[i]->getSolution()->getObjValue(),
                                      s_pool);
        }
      }
    }
    if (should_prune) {
      break;
    }
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " best overall value at node = "
      << bestVal << std::endl;
#endif
    //6. Update the best solution (if no bestsol is obtained, rel is inf)
    sol = (SolutionPtr) new Solution(bestsol);

    // check if the relaxation is infeasible or if the cost is too high.
    // In either case we can prune. Also set lb of node.
    should_prune = shouldPrune_(node, sol->getObjValue(), s_pool);
    if (should_prune) {
      break;
    }

    // update pseudo-cost from last branching.
    if (iter == 1) {
      brancher_->updateAfterSolve(node, sol);
    }

    // check feasibility. if it is feasible, we can still prune this node.
    isFeasible_(node, sol, s_pool, should_prune);
    if (should_prune) {
      break;
    }

    //save warm start information before branching. This step is expensive.
    ws_ = engine_->getWarmStartCopy();
    branches_ = brancher_->findBranches(relaxation_, node, sol, s_pool, 
                                        br_status, mods);
    if (br_status==PrunedByBrancher) {

      should_prune = true;
      node->setStatus(NodeInfeasible);
      stats_.inf++;
      break;
    } else if (br_status==ModifiedByBrancher) {
      for (ModificationConstIterator miter=mods.begin(); miter!=mods.end();
           ++miter) {
        node->addPMod(*miter);
        (*miter)->applyToProblem(relaxation_);
      }
      should_resolve = true;
    } 
    if (should_resolve == false) {
      break;
    }
  }

  for(UInt i = 0; i < numThreads_; ++i) {
    delete relCopy[i];
  }
  delete[] relCopy;
  for(UInt i = 0; i < numThreads_; ++i) {
    delete eCopy[i];
  }
  delete[] eCopy;
  delete[] eStatus;
  delete[] solCount;
  delete[] startPoint;
  //delete prevStartPoint;
  //delete prevOpt;
  return;
}


bool MsProcessor::shouldPrune_(NodePtr node, double solval, 
                               SolutionPoolPtr s_pool)
{
  bool should_prune = false;
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
    // This comment silences g++ warnings. Do not remove
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
    // This comment silences g++ warnings. Do not remove
    // fall through
  case (ProvenLocalOptimal):
  case (ProvenOptimal):
    node->setLb(solval);
    if (solval >= s_pool->getBestSolutionValue() || solval >= cutOff_) {
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

void MsProcessor::solveRelaxation_() 	
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

void MsProcessor::solveRelaxation_(EnginePtr e1)
{
  e1->solve();
  engineStatus_ = e1->getStatus();
#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "solving relaxation" << std::endl
    << me_ << "engine status = " 
    << e1->getStatusString() << std::endl;
#endif
}

void MsProcessor::writeStats(std::ostream &out) const
{
  out << me_ << "nodes processed     = " << stats_.proc << std::endl 
    << me_ << "nodes branched      = " << stats_.bra << std::endl 
    << me_ << "nodes infeasible    = " << stats_.inf << std::endl 
    << me_ << "nodes optimal       = " << stats_.opt << std::endl 
    << me_ << "nodes hit ub        = " << stats_.ub << std::endl 
    << me_ << "nodes with problems = " << stats_.prob << std::endl 
    ;
}


void MsProcessor::writeStats() const
{
  writeStats(logger_->msgStream(LogNone));
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
