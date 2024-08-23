//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file SppHeur.cpp
 * \brief Define the SppHeur class for heuristics for set partitioning
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 */

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "Environment.h"
#include "MinotaurConfig.h"
#include "Operations.h"
#include "Problem.h"
#include "SppHeur.h"
#include "SolutionPool.h"
#include "Types.h"
#include "Variable.h"

#include "OsiLPEngine.h"
using namespace Minotaur;

#define SPEW 0

const std::string SppHeur::me_ = "SetPart Heuristic: ";

SppHeur::SppHeur(EnvPtr env, ProblemPtr p)
  : env_(env),
    p_(p)
{
  maxRand_ = 100;
  logger_ = env_->getLogger();
  stats_ = new SppHeurStats();
  stats_->calls = 0;
  stats_->runs = 0;
  stats_->sols = 0;
  stats_->time = 0.0;
}

void SppHeur::addSol_(SolutionPool *spool) 
{
  double objval=0.0;
  LinearFunction* lf=p_->getObjective()->getFunction()->getLinearFunction();
  double *x = new double[p_->getNumVars()];
  Variable *v=0;
  int j = 0;

  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit, ++j) {
    v = *vit;
    x[j]=0.0;
    if (1==v->getItmp()) {
      x[j]=1.0;
      objval+=lf->getWeight(v);
    }
  }
  if (isFeasible_(x)) {
    logger_->msgStream(LogExtraInfo) << me_ << "Found a feasible point with "
      << " objective value " << objval << std::endl;
    spool->addSolution(x, objval);
    stats_->sols += 1;
    // solveLP_();
  } else {
    logger_->msgStream(LogInfo) << me_ << "A solution we thought was "
      << "feasible is not really feasible " << std::endl;
  } 
  delete[] x;
  return;
}

void SppHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  double start = env_->getTime();

  stats_->calls += 1;
  if (!isSpp_()) {
    return;
  }

  srand(time(NULL));
  for (int i=0; i<30; ++i) {
    conWise_(NULL, NULL, spool);
  }

  stats_->time += env_->getTime()-start;

  writeStats(logger_->msgStream(LogExtraInfo));
  // solveLP_(spool);
}

void SppHeur::solveLP_(SolutionPool *spool)
{
  OsiLPEngine lpe(env_);
  int j = 0;
  double maxvar = 0.0;
  const double *x = 0;
  int fixed = 0;
  
  if (stats_->sols==0) {
    return;
  }

  x = spool->getBestSolution()->getPrimal();
  lpe.load(p_);
  //lpe.writeLP("presolved-model.lp");
  exit(10); 


  // Fix 80% of variables in solution at 1 to get a much smaller LP.
  std::cout << "FIXING VARS BEFORE LP\n";
  j=0;
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit,++j) {
    if (fabs(1.0-x[j])<1e-6) { 
      ++maxvar;
    }
  }

  j=0;
  maxvar = maxvar*0.8;
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit,++j) {
    if (fabs(1.0-x[j])<1e-6 && fixed<maxvar) { 
      p_->changeBound((*vit),1.0,1.0);
      ++fixed;
    } 
  }

  lpe.solve();
  std::cout << "SOLVING FULL LP\n";
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit,++j) {
      p_->changeBound((*vit),0.0,1.0);
  }
  lpe.solve();
  std::cout << "Time: " << env_->getTime() << std::endl;
}


void SppHeur::solveRand(NodePtr, RelaxationPtr, SolutionPoolPtr)
{
  ConstraintPtr c;
  VariablePtr v;
  ConstraintVector cv;
  FunctionPtr f;
  UInt k;
  VarSetConstIterator vsit;
  int val, j, rnd, num0;
  bool cfixed, feas;
  std::deque<Variable *>varq;

  if (!isSpp_()) {
    return;
  }
  logger_->msgStream(LogDebug) << me_ << " starting the set partitioning" 
    << " heuristic" << std::endl;

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    c->setBFlag(false);
    cv.push_back(c);
    logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
      << " has " << c->getFunction()->getNumVars() << " variables" << std::endl;
  }

  std::sort(cv.begin(), cv.end(), CompareLen);

  feas=false;
  rnd = 0;
  num0 = 0;
  while (feas==false) {
    ++rnd;
    logger_->msgStream(LogDebug) << me_ << " starting round " << rnd << std::endl;
    feas = true;

    // reset all flags
    for (ConstraintConstIterator cit = cv.begin(); cit != cv.end();
         ++cit) {
      (*cit)->setBFlag(false);
    }
    for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
         ++vit) {
      (*vit)->setItmp(10);
    }


    j=0;
    int num1=0;
    for (ConstraintConstIterator cit = cv.begin(); cit != cv.end(); ++cit) {
      ++j;
      c = *cit;
      f = c->getFunction();
      k = f->getNumVars();
      logger_->msgStream(LogDebug) << me_ << " trying constraint number " << j
        << " out of " << cv.size() << " with " << k << " variables. Total " << num1 << " vars at 1" <<  std::endl;


      // check if already fixed
      if (true==c->getBFlag()) {
        logger_->msgStream(LogDebug) << me_ << " constraint " << c->getName()
          << " is feasible already. Skipping " << std::endl;
        continue;
      }

      // pick one variable to set to 1
      for (UInt i=0; i<k; ++i) {
        cfixed=false;
        vsit = f->varsBegin();
        std::advance(vsit,rand()%k); // this advance is too expensive, computationally
        v = *vsit;
        val = v->getItmp();
#if SPEW
        logger_->msgStream(LogInfo) << me_ << " variable " << v->getName() << " has value " << v->getItmp() << std::endl;
#endif
        if (10==val) {
          v->setItmp(1);
#if SPEW
          logger_->msgStream(LogDebug2) << me_ << " setting variable " << v->getName() << " to 1 " << std::endl;
#endif
          if (false==propVal_(v, num0)) {
            logger_->msgStream(LogDebug) << me_ << " SHOULD NEVER BE resetting variable " << v->getName() << " to 10 " << std::endl;
            v->setItmp(10);
          } else {
            cfixed = true;
            ++num1;
            break;
          }
        }
      }

      if (cfixed) {
        logger_->msgStream(LogDebug) << me_ << " constraint " << c->getName() << " feasible." << std::endl;
      } else {
        logger_->msgStream(LogDebug) << me_ << " constraint " << c->getName() << "has all zeros. Infeasible." << std::endl;
        // c->write(logger_->msgStream(LogInfo));
        feas=false;
        break;
      }
    }
  }
}

void SppHeur::conWise_(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  ConstraintPtr c;
  VariablePtr v;
  ConstraintVector cv;
  FunctionPtr f;
  VarSetConstIter vsit;
  int val, j, num1, num0;
  bool cfixed, feas;
  const double *xx;

  stats_->runs += 1;
  logger_->msgStream(LogDebug) << me_ << " running in constraint-wise mode" 
    << " round " << stats_->runs << std::endl;

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    cv.push_back(c);
    logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
      << " has " << c->getFunction()->getNumVars() << " variables" << std::endl;
  }

  std::sort(cv.begin(), cv.end(), CompareLen);
  cv2_.insert(cv2_.end(), cv.begin(), cv.end());

  feas=true;
  num0 = 0;
  num1 = 0;

  // reset all flags
  for (ConstraintConstIterator cit = cv.begin(); cit != cv.end();
       ++cit) {
    (*cit)->setBFlag(false);
  }
  cv.clear();
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit) {
    (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
  }

  if (0<spool->getNumSols()) {
    // Fix some of the variables to zero. Choose some randomly from the
    // variables that have value one in an existing solution.
    j=0;
    xx=spool->getBestSolution()->getPrimal();
    for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
         ++vit,++j) {
      (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
      if (fabs(xx[j]-1.0)<1e-6 && (*vit)->getNumCons()>3 && rand()%100<10) {
        (*vit)->setItmp(0);
        ++num0;
        // std::cout << "Set " << (*vit)->getName() << " to zero\n";
      }
    }
    logger_->msgStream(LogDebug) << me_ << num0 << " variables at 0. Reference solution value =  " << spool->getBestSolution()->getObjValue() << std::endl;
  }


  // start fixing vars one by one
  j=0;
  for (ConstraintConstIterator cit = cv2_.begin(); cit != cv2_.end(); ++cit) {
    ++j;
    c = *cit;
    f = c->getFunction();
#if SPEW
    logger_->msgStream(LogDebug) << me_ << " trying constraint number " << j
      << " out of " << cv2_.size() << " with " << f->getNumVars() << " vars. "
      << c->getName() << " "
      << num1 << " vars at 1 and " << num0 << " vars at 0 " << std::endl;
#endif

    // check if already fixed
    if (true==c->getBFlag()) {
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << c->getName()
        << " is feasible already. Skipping " << std::endl;
#endif
      continue;
    }

    // pick one variable to set to 1
    UInt i=0;
    for (vsit = f->varsBegin(); vsit != f->varsEnd(); ++vsit,++i) {
      cfixed=false;
      v = *vsit;
      val = v->getItmp();
      if (10==val) {
        v->setItmp(1);
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " setting variable " << v->getName() << " to 1, i =  " << i <<  std::endl;
#endif
        if (false==propVal_(v, num0)) {
#if SPEW
          logger_->msgStream(LogDebug) << me_ << " propagation found that var "
           << "cannot be 1 " << v->getName() <<  ". Should be caught earlier"
           <<  std::endl;
#endif
        } else {
          cfixed = true;
          ++num1;
          if (i==f->getNumVars()-1) {
            cv.push_back(c);
          }
          break;
        }
      } else if(1==val) {
        logger_->msgStream(LogDebug) << me_ << " variable " << v->getName()
          << " already at 1. Cant happen " << std::endl;
      } else {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " variable "
          << v->getName() << " already at 0, trying next" << std::endl;
#endif
      }
    }

    if (cfixed) {
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << c->getName()
        << " feasible." << std::endl;
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << c->getName()
        << "has all zeros. Infeasible." << std::endl;
#endif
      cv.insert(cv.begin(),c);
      feas=false;
      break;
    }
  }

  cv2_.clear();
  cv2_.insert(cv2_.begin(), cv.begin(), cv.end());
#if SPEW
  logger_->msgStream(LogDebug) << me_ << " cv size = " << cv.size() << std::endl;
#endif

  if (feas) {
    addSol_(spool);
  } else {
#if SPEW
    logger_->msgStream(LogDebug) << me_ << " Failed to find a feasible solution. "
      << std::endl;
#endif
  }
}

bool SppHeur::isFeasible_(const double* x)
{
  ConstraintPtr c;
  double act, clb, cub;
  int error = 0;
  double aTol = 1e-6, rTol = 1e-7;

  for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    act = c->getActivity(x, &error);
    if(error == 0) {
      cub = c->getUb();
      clb = c->getLb();
      if((act > cub + aTol) && (cub == 0 || act > cub + fabs(cub) * rTol)) {
        return false;
      }
      if((act < clb - aTol) && (clb == 0 || act < clb - fabs(clb) * rTol)) {
        return false;
      }
    } else {
      logger_->msgStream(LogError) << me_ << c->getName() 
        << " Constraint function could not be evaluated at give point." 
        << std::endl;
      return false;
    }
  }
  return true;
}


bool SppHeur::propVal_(VariablePtr v, int &num0)
{
  FunctionPtr f;

  for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
    if (true==(*cit)->getBFlag()) {
      // logger_->msgStream(LogDebug) << me_ << " cant set variable " << v->getName() << " as it appears in " << (*cit)->getName() << std::endl;
      return false;
    }
  }

  for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
    if (false==(*cit)->getBFlag()) {
      (*cit)->setBFlag(true);
      f = (*cit)->getFunction();
      // logger_->msgStream(LogDebug) << me_ << " setting variables in constraint " << (*cit)->getName() << std::endl;
      for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd(); ++vit) {
        if ((*vit)->getItmp()==10) {
          (*vit)->setItmp(0);
          // logger_->msgStream(LogDebug) << me_ << " setting variable " << (*vit)->getName() << " to zero" << std::endl;
          ++num0;
        }
        // logger_->msgStream(LogDebug) << me_ << " set variable " << (*vit)->getName() << " to 0" << std::endl;
      }
    }
  }
  v->setItmp(1);
  return true;
}

/// writing the statistics to the logger
void SppHeur::writeStats(std::ostream& out) const
{
  out << me_ << "Number of times called: " << stats_->calls << std::endl <<
  me_ << "Number of runs " << stats_->runs << std::endl <<
  me_ << "Number of solutions found: " << stats_->sols << std::endl <<
  me_ << "Time taken: " << stats_->time << std::endl;
}


bool SppHeur::isSpp_() 
{
  ConstraintPtr c;
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    if (c->getType() != SetPartition && c->getType()!=Aggregation) {
      logger_->msgStream(LogDebug) << me_ << " model is not set partitioning" 
        << std::endl;
      return false;
    }
  }
  logger_->msgStream(LogDebug) << me_ << " model is set partitioning" 
    << std::endl;
  return true;
}


