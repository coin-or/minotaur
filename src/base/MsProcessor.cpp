// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
// 

/**
 * \file MsProcessor.cpp
 * \brief Implement simple multi-start node-processor for branch-and-bound
 * \author Ashutosh Mahajan, IIT Bombay
 */
#include <cmath> // for INFINITY
#include <omp.h>
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

  MsProcessor::MsProcessor()
: contOnErr_(false),
  cutOff_(INFINITY),
  engine_(EnginePtr()),
  engineStatus_(EngineUnknownStatus),
  numSolutions_(0),
  relaxation_(RelaxationPtr()),
  ws_(WarmStartPtr())
{
  handlers_.clear();
  logger_ = (LoggerPtr) new Logger(LogInfo);
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
  numSolutions_(0),
  relaxation_(RelaxationPtr()),
  ws_(WarmStartPtr())
{
  cutOff_ = env->getOptions()->findDouble("obj_cut_off")->getValue();
  numThreads_ = env->getOptions()->findInt("threads")->getValue();
  schemeId_ = env->getOptions()->findInt("msbnb_scheme_id")->getValue();
  handlers_ = handlers;
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("node_processor_log_level")->
                                   getValue());
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
  logger_.reset();
  engine_.reset();
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

  if (is_feas == true && h==handlers_.end()) {
    s_pool->addSolution(sol);
    ++numSolutions_;
    node->setStatus(NodeOptimal);
    ++stats_.opt;
    should_prune = true;
  }
  return is_feas;
}

void MsProcessor::par()
{
#pragma omp parallel
#pragma omp critical
  std::cout<<"Hello from thread "<<omp_get_thread_num()<<", total threads = "<<omp_get_num_threads()<<std::endl;
}

double MsProcessor::InnerProduct(double b[], double c[], UInt n)
{
  double dotprod = 0;
  for(UInt i=0; i < n; i++) {
    dotprod += b[i]*c[i];        
  }
  return dotprod;
}

double EDist(double b[], double c[], UInt n)
{
  double dist = 0;
  for(UInt i=0; i < n; i++) {
    dist += (b[i]-c[i])*(b[i]-c[i]);
  }
  return sqrt(dist);
}

double MsProcessor::ENorm(double b[], UInt n)
{
  double norm = 0;
  for(UInt i=0;i < n;i++) {
    norm+= b[i]*b[i];        
  }
  return sqrt(norm);
}

double * MsProcessor::getBoxCorner(UInt n, RelaxationPtr rel1, int threadid,
                                   int K)
{
  double* corner_point = new double[n];
  srand(time(NULL));
  double lb,ub, lbdbindex,ubdbindex, largestInterval = -INFINITY;
  
  int dbindex = -1;     //index of a doubly bounded variable
  for(UInt k=0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if(lb  > -INFINITY) {
      if(ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if( (ub - lb) > largestInterval) {
          dbindex = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if(dbindex >= 0) {
    lbdbindex = rel1->getVariable(dbindex)->getLb();
    ubdbindex = rel1->getVariable(dbindex)->getUb();
    double width = (ubdbindex - lbdbindex)/numThreads_;

    for(UInt j=0; j < n; j++) {
      if((int)j!=dbindex) {
        lb = rel1->getVariable(j)->getLb();
        ub = rel1->getVariable(j)->getUb();
        if(lb  > -INFINITY) {
          if(ub < INFINITY) {
            if (rand()/2 ==0) {
              corner_point[j] = lb;
            }
            else {
              corner_point[j] = ub;
            }
          }
          else {
            corner_point[j] = lb + rand()%K;
          }
        }
        else {
          if(ub < INFINITY) {
            corner_point[j] = ub - rand()%K;
          }
          else {
            corner_point[j] = rand()%K;
          }
        }
      }
      else {
        lb = lbdbindex + (threadid)*width;
        ub = lb + width;
        if (rand()/2 ==0) {
          corner_point[dbindex] = lb;
        }
        else {
          corner_point[dbindex] = ub;
        }
      }
    }
  }
  else {
    corner_point = getStartPointScheme1(n, rel1);
  }
  return corner_point;
}


double * MsProcessor::getFarBoxCorner(UInt n, RelaxationPtr rel1,
                                      int threadid, double* prev_opt, int K)
{

  double* corner_point = new double[n];
  srand(time(NULL));
  double lb,ub, lbdbindex,ubdbindex, largestInterval = -INFINITY;
  int dbindex = -1;
  for(UInt k=0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if(lb  > -INFINITY) {
      if(ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if( (ub - lb) > largestInterval) {
          dbindex = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if(dbindex >= 0) {
    lbdbindex = rel1->getVariable(dbindex)->getLb();
    ubdbindex = rel1->getVariable(dbindex)->getUb();
    double width = (ubdbindex - lbdbindex)/numThreads_;

    for(UInt j=0; j < n; j++) {
      if((int)j!=dbindex) {
        lb = rel1->getVariable(j)->getLb();
        ub = rel1->getVariable(j)->getUb();
        if(lb  > -INFINITY) {
          if(ub < INFINITY) {
            if(abs(ub - prev_opt[j]) > abs(prev_opt[j] - lb)) {
              corner_point[j] = ub;
            }
            else {
              corner_point[j] = lb;
            }
          }
          else {
            corner_point[j] = lb + rand()%K;
          }
        }
        else {
          if(ub < INFINITY) {
            corner_point[j] = ub - rand()%K;
          }
          else {
            corner_point[j] = rand()%K;
          }
        }
      }
      else {
        lb = lbdbindex + (threadid)*width;
        ub = lb + width;
        if(abs(ub - prev_opt[j]) > abs(prev_opt[j] - lb)) {
          corner_point[j] = ub;
        }
        else {
          corner_point[j] = lb;
        }
      }
    }
  }
  else {
    corner_point = getStartPointScheme1(n, rel1);
  }
  return corner_point;
}

double * MsProcessor::getStartPointScheme1(UInt n, RelaxationPtr rel1)
{
  double* init_point = new double[n];
  srand(time(NULL));
  double lb,ub;
  for(UInt k=0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();

    if(lb  > -INFINITY) {
      if(ub < INFINITY) { 
        init_point[k] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
      } else {
        init_point[k] = lb + rand()%1000;
      }	
    } else {
      if(ub < INFINITY) {
        init_point[k] = ub - rand()%1000;
      } else {
        init_point[k] = rand()%1000;
      }
    }
  }
  return init_point;
}

double* MsProcessor::getStartPointScheme2(UInt n, RelaxationPtr rel1, 
                                        int threadid, double radius, int numsols,
                                        double* prev_start_point)
{
  double* init_point = new double[n];
  srand(time(NULL));
  double lb,ub, lbdbindex,ubdbindex, norm, largestInterval = -INFINITY;
  double a[n];
  int dbindex = -1; 

  for(UInt k=0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if(lb  > -INFINITY) {
      if(ub < INFINITY) {
        //(first) doubly bdd variable found
        if( (ub - lb) > largestInterval) {
          dbindex = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if(dbindex >= 0) {
    lbdbindex = rel1->getVariable(dbindex)->getLb();
    ubdbindex = rel1->getVariable(dbindex)->getUb();
    double width = (ubdbindex - lbdbindex)/numThreads_;

    if(numsols==0) {
      //finding the initial point
      for(UInt j=0; j < n; j++) {
        if((int)j!=dbindex) {
          lb = rel1->getVariable(j)->getLb();
          ub = rel1->getVariable(j)->getUb();

          if(lb > -INFINITY) {
            if(ub < INFINITY) {
              init_point[j] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
            } else {
              init_point[j] = lb + rand()%1000;
            }	
          } else {
            if(ub < INFINITY) {
              init_point[j] = ub - rand()%1000;
            } else {
              init_point[j] = rand()%1000;
            }
          }
        }
      }
      //generating (dbindex)th coordinate within thread specific bound
      lb = lbdbindex + (threadid)*width;
      ub = lb + width;
      init_point[dbindex] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
      return init_point;
    } 
    //else if there exists a previous starting point:
    //generate a random point outside a ball
    else {
      for (UInt i=0; i < n; ++i) {
        a[i] = ((double)rand())/((double)(RAND_MAX));        
      }
      norm = ENorm(a,n);
      for (UInt i=0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i=0; i < n; i++) {
        if((int)i!=dbindex) {
          lb = rel1->getVariable(i)->getLb();
          ub = rel1->getVariable(i)->getUb();
          init_point[i] = std::max( std::min(prev_start_point[i] + (a[i])*radius, ub), lb );  
        }
      }
      lb = lbdbindex + (threadid)*width;
      ub = lb + width;
      init_point[dbindex] = std::max( std::min(prev_start_point[dbindex] + (a[dbindex])*radius, ub), lb );
    }
  }
  else {
    //for singly bounded and unbdd variables
    if(numsols==0) {
      init_point = getStartPointScheme1(n, rel1);
      return init_point;
    }
    else {
      //generate a random point outside the radius
      for (UInt i=0; i < n; ++i) {
        a[i] = ((double)rand())/((double)(RAND_MAX));        
      }
      norm = ENorm(a,n);
      for (UInt i=0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i=0; i < n; i++) {
        lb = rel1->getVariable(i)->getLb();
        ub = rel1->getVariable(i)->getUb();
        init_point[i] = std::max( std::min(prev_start_point[i] + (a[i])*radius,
                                           ub), lb );
      }
    }
  }
  return init_point;
}


double * MsProcessor::getStartPointScheme4(UInt n, RelaxationPtr rel1,
                                           int threadid, double radius,
                                           int numsols,
                                           double* prev_start_point,
                                           double* prev_opt, 
                                           double costhetalim)
{
  double* init_point = new double[n];
  srand(time(NULL));
  double lb,ub, lbdbindex,ubdbindex, norm, largestInterval = -INFINITY;
  double a[n], prev_dir[n];      //random direction and prev iter. direction
  int dbindex = -1;              //index of a doubly bounded variable
  double costheta = 1;           //cos of angle between 'prev_dir' and 'a'

  for(UInt k=0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if(lb  > -INFINITY) {
      if(ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if( (ub - lb) > largestInterval) {
          dbindex = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  if(dbindex >= 0 ) {
    lbdbindex = rel1->getVariable(dbindex)->getLb();
    ubdbindex = rel1->getVariable(dbindex)->getUb();
    double width = (ubdbindex - lbdbindex)/numThreads_;

    if(numsols==0) {
      //getting the initial point
      for(UInt j=0; j < n; j++) {
        if((int)j!=dbindex) {
          lb = rel1->getVariable(j)->getLb();
          ub = rel1->getVariable(j)->getUb();

          if(lb > -INFINITY) {
            if(ub < INFINITY) {
              init_point[j] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
            } else {
              init_point[j] = lb + rand()%1000;
            }	
          } else {
            if(ub < INFINITY) {
              init_point[j] = ub - rand()%1000;
            } else {
              init_point[j] = rand()%1000;
            }
          }
        }
      }
      //generating (dbindex)th coordinate within thread specific bound
      lb = lbdbindex + (threadid)*width;
      ub = lb + width;
      init_point[dbindex] = lb + ((double)(rand())/(double)(RAND_MAX))*(ub-lb);
      return init_point;
    } 
    //else if there exists a previous starting point:
    //generate a random point outside the radius in conjugate direction
    else {
      while(costheta < costhetalim) {
        for (UInt i=0; i < n; ++i) {
          a[i] = ((double)rand())/((double)(RAND_MAX));        
        }
        //find the angle between the two directions
        for (UInt i=0; i < n; ++i) {
          prev_dir[i] = prev_opt[i] - prev_start_point[i];
        }
        costheta = InnerProduct(a, prev_dir, n)/((ENorm(a,n))*(ENorm(prev_dir,n)));
      }
      norm = ENorm(a,n);
      for (UInt i=0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i=0; i < n; i++) {
        if((int)i!=dbindex) {
          lb = rel1->getVariable(i)->getLb();
          ub = rel1->getVariable(i)->getUb();
          init_point[i] = std::max(std::min(prev_start_point[i]
                                            +(a[i])*radius, ub), lb); //check pointers
        }
      }
      lb = lbdbindex + (threadid)*width;
      ub = lb + width;
      init_point[dbindex] = std::max(std::min(prev_start_point[dbindex]
                                              +(a[dbindex])*radius, ub),lb);
    }
  }
  else {
    //for singly bounded and unbdd variables
    if(numsols==0) {
      init_point = getStartPointScheme1(n, rel1);
      return init_point;
    }
    else {
      //generate a random point outside the radius
      while(costheta < costhetalim) {
        for (UInt i=0; i < n; ++i) {
          a[i] = ((double)rand())/((double)(RAND_MAX));        
        }
        //find the angle between the two directions
        for (UInt i=0; i < n; ++i) {
          prev_dir[i] = prev_opt[i] - prev_start_point[i];
        }
        costheta = InnerProduct(a, prev_dir, n)/((ENorm(a,n))*(ENorm(prev_dir,n)));
      }
      norm = ENorm(a,n);
      for (UInt i=0; i < n; ++i) {
        a[i] /= norm;
      }
      for (UInt i=0; i < n; i++) {
        lb = rel1->getVariable(i)->getLb();
        ub = rel1->getVariable(i)->getUb();
        init_point[i] = std::max(std::min(prev_start_point[i]+(a[i])*radius,ub),lb);
      }
    }
  }
  return init_point;
}

double * MsProcessor::getStartPointScheme5 (UInt n, RelaxationPtr rel1,
                                            int threadid, double radius,
                                            double* prev_start_point,
                                            double* prev_opt, int K, 
                                            double lambda)
{
  double* init_point = new double[n];
  srand(time(NULL));
  double lb,ub, lbdbindex,ubdbindex, width, norm, largestInterval = -INFINITY;
  double new_dir[n], tempcorner[n]; //random direction and new iter. direction
  int dbindex = -1;                 //index of a doubly bounded variable
  double* farcorner;

  for(UInt k=0; k < n; k++) {
    lb = rel1->getVariable(k)->getLb();
    ub = rel1->getVariable(k)->getUb();
    if(lb  > -INFINITY) {
      if(ub < INFINITY) {
        // doubly bdd variable with largest bound width
        if( (ub - lb) > largestInterval) {
          dbindex = k;
          largestInterval = ub - lb;
        }
      }
    }
  }

  //get distance r of prev start from previous optimal
  radius = getDistance(prev_start_point, prev_opt, n);

  //get farthest box corner
  farcorner = getFarBoxCorner(n, rel1, threadid, prev_opt, K);

  //get the direction unit vector of prev opt from farboxcorner
  for (UInt i=0; i < n; i++) {
    new_dir[i] = farcorner[i] - prev_opt[i];
  }
  norm = ENorm(new_dir, n);
  for (UInt i=0; i < n; i++) {
    new_dir[i] = new_dir[i]/norm;
  }
  //find a point at a distance = radius in this direction
  for (UInt i=0; i < n; i++) {
    if((int)i==dbindex)      //this considers dbindex>=0 also
    {
      lbdbindex = rel1->getVariable(dbindex)->getLb(); 
      ubdbindex = rel1->getVariable(dbindex)->getUb();
      width = (ubdbindex - lbdbindex)/numThreads_;
      lb = lbdbindex + (threadid)*width;
      ub = lb + width;
    }
    else {
      lb = rel1->getVariable(i)->getLb();
      ub = rel1->getVariable(i)->getUb();
    }
    tempcorner[i] = std::max(std::min(prev_opt[i] + radius*
                                      new_dir[i], ub), lb);
  }

  //take a convex combination of this point and farboxcorner as the start
  //point and bring it within variable bounds
  if(lambda==-1) {
    lambda = ((double)rand())/((double)(RAND_MAX));
  }
  for (UInt i=0; i < n; i++) {
    if((int)i==dbindex)     
    {
      //lbdbindex = rel1->getVariable(dbindex)->getLb(); 
      //ubdbindex = rel1->getVariable(dbindex)->getUb();
      //double width = (ubdbindex - lbdbindex)/numThreads_;
      lb = lbdbindex + (threadid)*width;
      ub = lb + width;
    }
    else {
      lb = rel1->getVariable(i)->getLb();
      ub = rel1->getVariable(i)->getUb();
    }
    init_point[i] = std::max(std::min(tempcorner[i]+(1-lambda)*
                                      (farcorner[i]-tempcorner[i]), ub), lb);
  }
  return init_point;
}

//double * MsProcessor::getStartPointScheme6(UInt n, RelaxationPtr rel1, 
                                           //int threadid, double radius,
                                           //int numsols, 
                                           //double* prev_start_point,
                                           //double* prev_opt, int K, 
                                           //double lambda)
//{
  //double* init_point = new double[n];
  //srand(time(NULL));
  //double lb,ub, lbdbindex,ubdbindex, width, norm, largestInterval = -INFINITY;
  //double new_dir[n], tempcorner[n];      //random direction and new iter. direction
  //int dbindex = -1;                      //index of maxspan doubly bounded variable

//find active constraints at the prev optimal

//take a random linear combination of the gradients of active constraints by
//using appropriate signs depending upon <= or >=

//while
//the objective function is decreasing
//
//the average constraint violation
//
//increase the step length
//


//}

double * MsProcessor::genInitialPoint(UInt n, RelaxationPtr rel1)
{
  double* init_point = new double[n];
  EngineStatus status;
  RelaxationPtr relc = (RelaxationPtr) new Relaxation(rel1);
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
  if ( (ProvenInfeasible==status || ProvenLocalInfeasible==status ||
        ProvenObjectiveCutOff==status || ProvenFailedCQInfeas==status || 
        FailedInfeas==status) || ((FailedFeas==status || 
                                   ProvenFailedCQFeas==status || ProvenLocalOptimal==status) )) 
    init_point = (double*)ec->getSolution()->getPrimal();
  return init_point;

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
  //int numThreads_ = omp_get_num_procs()-5;	  //number of threads to be created
  UInt numStarts = 1;      	                  //number of restarts per thread
  UInt numvars = rel->getNumVars();               //number of variables in the problem
  RelaxationPtr relcopy[numThreads_];		  //array of ptrs of copies of relaxation

  EnginePtr ecopy[numThreads_];                    //array of ptrs of copies of engine
  EngineStatus estatus[numThreads_];               //array of statuses of copies of engine
  int solcnt[numThreads_];                         //array of count of solutions obtained

  ConstSolutionPtr bestsolthd;                    //pointer for the thread-best solution
  ConstSolutionPtr bestsol ;                      //pointer for best solution among all threads

  double threadbestval = INFINITY, bestval = INFINITY;
  double *start_point, *prev_start_point, *prev_opt; 
  double radscal = 1.5;                           //scaling factor for radius      
  double costhetalim = sqrt(3)/2;                 //angle more than 30 degree
  int K = 1000;                                   //scale of number for generating a random number
  double lambda = -1;                             //fixed value for taking convex combination
  double prevsolval;                              //previous solution value
  //UInt schemeId_ = 5;                            //scheme id for generating starting point

#pragma omp parallel for   
  for(UInt i=0; i < numThreads_; i++) {
#pragma omp critical
    {
      relcopy[i] = (RelaxationPtr) new Relaxation(rel); 
      ecopy[i] = engine_->emptyCopy();
      ecopy[i]->clear();
      relcopy[i]->prepareForSolve();
      ecopy[i]->load(relcopy[i]);
    }
    solcnt[i] = 0;
  }

  // loop for branching and resolving if necessary.
  while (true) {			           
    ++iter;
    should_resolve = false;

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "iteration " << iter << std::endl;
#endif

    //Solving multiple copies of relaxations (parallel region begin)
#pragma omp parallel for 
    for(UInt i=0; i < numThreads_; i++) {
      ConstSolutionPtr sol1;
      double radius = 0.8;                //initial radius of region

      for(UInt j=0; j < numStarts; j++) {  

        //Initial point generation schemes
        switch(schemeId_) {
        case 1:
          //Scheme 1: generate random points within variable bounds
          start_point = getStartPointScheme1(numvars, relcopy[i]);
          break;

        case 2:
          //Scheme 2: generate a random start point outside a radius from prev
          //start point; enlarge the region radius if no improvement
          if(solcnt[i] > 1) {
            if(sol1->getObjValue() == prevsolval) {
              radius *= radscal;  //tbd: take the distance between start pt and optimal
            }
          }
          start_point = getStartPointScheme2(numvars, relcopy[i], i, radius,
                                           solcnt[i], prev_start_point);
          break;

        case 3:
          //Scheme 2: generate a random start point outside a radius from prev
          //optimal point; take the region radius > dist(startpt, optimal) 
          if(solcnt[i] > 1) {
            if(sol1->getObjValue() == prevsolval) {
              radius = radscal*getDistance(prev_start_point, prev_opt, numvars);
            }
            else {
              radius = getDistance(prev_start_point, prev_opt, numvars);
            }
            start_point = getStartPointScheme2(numvars, relcopy[i], i, radius,
                                             solcnt[i], prev_opt);
          }
          else {//pass the previous start point
            start_point = getStartPointScheme2(numvars, relcopy[i], i, radius,
                                             solcnt[i], prev_start_point);
          }
          break;

        case 4:
          //Scheme 4: passing previous optimal to get a possibly conjugate direction
          //from the previous starting point
          if (solcnt[i] > 1) {
            if(sol1->getObjValue() == prevsolval) {
              radius = radscal*getDistance(prev_start_point, prev_opt, numvars);
            }
            else {
              radius = getDistance(prev_start_point, prev_opt, numvars);
            }
          }
          start_point = getStartPointScheme4(numvars, relcopy[i], i, radius,
                                             solcnt[i], prev_start_point,
                                             prev_opt, costhetalim);
          break;

        case 5:
          //Scheme 5: take convex combination of prev optimal and the farthest
          //box corner point from the optimal
          if(solcnt[i] > 1) {
            if(sol1->getObjValue() == prevsolval) {
              start_point = getStartPointScheme1(numvars, relcopy[i]);
            }
            else {
              start_point = getStartPointScheme5(numvars, relcopy[i], i,
                                                  radius, prev_start_point,
                                                  prev_opt, K, lambda);
            }
          }
          else if (solcnt[i]==1) {
            start_point = getStartPointScheme5(numvars, relcopy[i], i,
                                               radius, prev_start_point,
                                               prev_opt, K, lambda);

          }
          else {
              start_point = getBoxCorner(numvars, relcopy[i], i, K);
          }
          break;

        //case 6:
          //Scheme 6: take random linear combination of active constraint
          //gradients at the previous optimal point
          //if(solcnt[i] > 1) {
            //start_point = getStartPointScheme6(numvars, relcopy[i], i,
                                               //radius, solcnt[i],
                                               //prev_start_point, 
                                               //prev_opt, K, lambda);
          //}
          //else {
            //start_point = getStartPointScheme1(numvars,relcopy[i]);
          //}
        //case 7:
          //Scheme 7: randomly fix k vars and solve the NLP; take sol as start point
          //start_point = genInitialPoint(numvars, relcopy[i]);
          //break;

          //case 8:
          //Scheme 8: consider only linear constraints, take the soln as start pt.
          //break;

        default:
          start_point = getStartPointScheme1(numvars,relcopy[i]);
          break;

        }          

        relcopy[i]->setInitialPoint(start_point);
        solveRelaxation_(ecopy[i]);      
        estatus[i] = ecopy[i]->getStatus();      

        if (ProvenOptimal==estatus[i] || ProvenLocalOptimal==estatus[i]) {
          sol1 = ecopy[i]->getSolution();
          prev_opt = (double*)sol1->getPrimal();

          if(sol1->getObjValue() < threadbestval) {
            bestsolthd = (SolutionPtr) new Solution(sol1);
            threadbestval = sol1->getObjValue();
          }
          prevsolval = sol1->getObjValue();
          solcnt[i]++;
        } 
        else if(ProvenInfeasible==estatus[i] || ProvenLocalInfeasible==estatus[i] ||
                ProvenObjectiveCutOff==estatus[i] || ProvenFailedCQInfeas==estatus[i] ||
                FailedInfeas==estatus[i] || FailedFeas==estatus[i] ||
                ProvenFailedCQFeas==estatus[i]) {
          //std::cout<<"Relaxation infeasible from this starting point!"<<std::endl;
        }
        else if(ProvenUnbounded==estatus[i]) {
          //std::cout<<"Relaxation unbounded from this starting point!"<<std::endl;
        }
        else {
        }

        prev_start_point = start_point;
      }
#pragma omp critical
      {
        if(solcnt[i] > 0) {
          //std::cout<<"Best value at thread "<<omp_get_thread_num()<<": "<<threadbestval<<std::endl;

          //5. Compare with/obtain the best solution (thread critical region)
          if(threadbestval < bestval) {
            bestsol = (SolutionPtr) new Solution(bestsolthd);
            bestval = threadbestval;
          }
        }
        else {
          //should_prune = true;
          should_prune = shouldPrune_(node, ecopy[i]->getSolution()->getObjValue(), s_pool);  // (Prune infeas.)
        }
      }
    }

    if (should_prune) {
      break;
    }
    //std::cout<<std::endl<<"Best overall value at node: "<<bestval<<std::endl<<std::endl;

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
      brancher_->updateAfterLP(node, sol);
    }

    // check feasibility. if it is feasible, we can still prune this node.
    isFeasible_(node, sol, s_pool, should_prune);
    if (should_prune) {
      break;
    }

    //relaxation_->write(std::cout);

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
#if 0										//for debugging
  if ((true==should_prune || node->getLb() >-4150) && true==xfeas) {
    std::cout << "problem here!\n";
    std::cout << node->getStatus() << "\n";
    rel->write(std::cout);
    exit(0);
  }
#endif

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
