//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
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

// #define SPEW 1

const std::string SppHeur::me_ = "SppHeur: ";

SppHeur::SppHeur(EnvPtr env, ProblemPtr p)
  : env_(env),
    p_(p)
{
  logger_ = env_->getLogger();
  stats_.calls = 0;
  stats_.runs = 0;
  stats_.sols = 0;
  stats_.time = 0.0;
}

double SppHeur::addSol_(SolutionPool *spool) 
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
    logger_->msgStream(LogDebug1) << me_ << "Found a feasible point with "
      << " objective value " << objval << std::endl;
    spool->addSolution(x, objval);
    stats_.sols += 1;
    // solveLP_();
  } else {
    logger_->msgStream(LogExtraInfo) << me_ << "A solution we thought was "
      << "feasible is not really feasible " << std::endl;
  } 
  delete[] x;
  return objval;
}

void SppHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  double start = env_->getTime();

  stats_.calls += 1;
  if (!isSpp_()) {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << "not a set partitioning model" 
      << std::endl;
#endif
    return;
  }

  cv2_.clear();

  for (int i=0; i<5; ++i) {
    conWise_(NULL, NULL, spool);
  }
  for (int i=0; i<15; ++i) {
     conWise2_(NULL, NULL, spool);
  }
  // for (int i=0; i<1; ++i) {
  //   conWise_(NULL, NULL, spool);
  // }
  // enum_(NULL, NULL, spool);

  stats_.time += env_->getTime()-start;

}


void SppHeur::solveNode(ConstSolutionPtr sol, NodePtr , RelaxationPtr , 
           SolutionPoolPtr spool)
{
  const double *x = sol->getPrimal();
  double start = env_->getTime();
  stats_.calls += 1;
  if (!x || spool->getNumSols()==0|| !isSpp_()) {
    return;
  }

  cv2_.clear();
  for (int i=0; i<10; ++i) {
    conWiseNb_(x, NULL, NULL, spool);
  }

  stats_.time += env_->getTime()-start;

}

  

void SppHeur::solveLP_(SolutionPool *spool)
{
  OsiLPEngine lpe(env_);
  int j = 0;
  double maxvar = 0.0;
  const double *x = 0;
  int fixed = 0;
  
  if (stats_.sols==0) {
    return;
  }

  x = spool->getBestSolution()->getPrimal();
  lpe.load(p_);
  //lpe.writeLP("presolved-model.lp");


  // Fix 80% of variables in solution at 1 to get a much smaller LP.
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
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit,++j) {
      p_->changeBound((*vit),0.0,1.0);
  }
  lpe.solve();
}


void SppHeur::conWiseNb_(const double *x, NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  ConstraintPtr c;
  VariablePtr v;
  ConstraintVector cv;
  FunctionPtr f;
  VarSetConstIter vsit;
  int val, j, num1, num0, kk;
  bool cfixed, feas;
  const double tol=1e-6;

  stats_.runs += 1;
  logger_->msgStream(LogDebug1) << me_ << " running in neighborhood mode" 
    << " round " << stats_.runs << std::endl;

  num0 = 0;
  num1 = 0;
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    c->setITemp(0);
    c->setBFlag(false);
    cv.push_back(c);
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
      << " has " << c->getFunction()->getNumVars() << " variables" << std::endl;
#endif
  }

  // shuffle the constraints
  for (int i=0; i<5; ++i) {
    j = rand()%cv.size();
    c = cv[i];
    cv[i] = cv[j];
    cv[j] = c;
  }

  std::sort(cv.begin(), cv.end(), CompareLen);
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit) {
    (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
  }

  feas=true;

  // start fixing vars in constraints one by one
  j=0;
  for (ConstraintConstIterator cit = cv.begin(); cit != cv.end(); ++cit) {
    ++j;
    c = *cit;
    f = c->getFunction();
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " trying constraint number " << j
      << " out of " << cv.size() << " with " << f->getNumVars() << " vars. "
      << c->getName() << " "
      << num1 << " vars at 1 and " << num0 << " vars at 0 " << std::endl;
#endif

    // check if already fixed
    if (true==c->getBFlag()) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " is feasible already. Skipping " << std::endl;
#endif
      continue;
    }

    // pick one variable to set to 1. Go left to right and pick the one that
    // is nonzero in x 
    UInt i=0;
    cfixed=false;
    for (vsit = f->varsBegin(); vsit != f->varsEnd(); ++vsit,++i) {
      v = *vsit;
      if (10==v->getItmp() && x[v->getIndex()]>tol && rand()%100<10) {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " setting variable " << v->getName() << " to 1, i =  " << i <<  std::endl;
#endif
        if (false==propVal_(v, num0, kk)) {
          v->setItmp(0);
#if SPEW
          logger_->msgStream(LogDebug2) << me_ << " propagation found that var "
           << "cannot be 1 " << v->getName() <<  ". Should be caught earlier"
           <<  std::endl;
#endif
        } else {
          cfixed = true;
          ++num1;
          break;
        }
      }
    }
    if (cfixed) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " feasible from nz x." << std::endl;
#endif
      continue;
    }

    // tried the nbhd, but didn't work. Try any variable from left to right
    // that works
    i = 0;
    for (vsit = f->varsBegin(); vsit != f->varsEnd(); ++vsit,++i) {
      cfixed=false;
      v = *vsit;
      val = v->getItmp();
      if (10==val) {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " setting variable " 
          << v->getName() << " to 1, i =  " << i <<  std::endl;
#endif
        if (false==propVal_(v, num0, kk)) {
          v->setItmp(0);
#if SPEW
          logger_->msgStream(LogDebug2) << me_ << " propagation found that var "
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
        logger_->msgStream(LogDebug2) << me_ << " variable " << v->getName()
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
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " feasible." << std::endl;
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << "has all zeros. Infeasible." << std::endl;
#endif
      cv.insert(cv.begin(),c);
      feas=false;
      break;
    }
  }

#if SPEW
  logger_->msgStream(LogDebug2) << me_ << " cv size = " << cv.size() << std::endl;
#endif

  if (feas) {
    addSol_(spool);
  } else {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << " Failed to find a feasible solution. "
      << std::endl;
#endif
  }
}

void SppHeur::varWise_(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  VariablePtr v;
  VarVector vv;
  VarSetConstIter vsit;
  int num1, num0;
  bool feas;
  const double tol=1e-6;
  int consfixed, i;
  const double *xx=0;

  stats_.runs += 1;
  logger_->msgStream(LogDebug1) << me_ << " running in Variable Wise mode" 
    << " round " << stats_.runs << " Time = " << env_->getTime() << std::endl;

  if (spool->getNumSols()>0) {
    xx = spool->getBestSolution()->getPrimal();
  }

  num0 = 0;
  num1 = 0;

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd(); ++cit) {
    (*cit)->setITemp((*cit)->getFunction()->getNumVars());
    (*cit)->setBFlag(false);
  }

  i=0;
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit,++i) {
    v = *vit;
    v->setItmp(10);
    vv.push_back(v);
    if (xx && xx[i]>tol && rand()%100<40 && v->getNumCons()>3) {
      v->setItmp(0);
      for (ConstrSet::iterator cit2=(*vit)->consBegin(); cit2!=(*vit)->consEnd(); ++cit2) {
        (*cit2)->setITemp((*cit2)->getITemp()-1);
      }
    }
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " variable " << v->getName()
      << " appears in " << v->getNumCons() << " constraints" << std::endl;
#endif
  }

  struct
  {
    bool operator()(Variable *v1, Variable *v2) const
    {
      return (v1->getNumCons() > v2->getNumCons());
    }
  } compvars;

  std::sort(vv.begin(), vv.end(), compvars);
  feas = true;
  consfixed = 0;

  for (VariableConstIterator vit = vv.begin(); vit!=vv.end(); ++vit) {
    v = *vit;
    if (v->getItmp()==10) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_  << "trying variable " << v->getName() << " appearing in " << v->getNumCons() << std::endl;
#endif
      if (true==propVal_(v, num0, consfixed)) {
        ++num1;
#if SPEW
        logger_->msgStream(LogDebug2) << me_  << "variable " << v->getName() << " set to 1" << std::endl;
#endif
        for (ConstraintConstIterator cit = p_->consBegin(); 
             cit != p_->consEnd(); ++cit) {
          if (0==(*cit)->getITemp() && false==(*cit)->getBFlag()) {
#if SPEW
            logger_->msgStream(LogDebug2) << me_  << "constraint " 
              << (*cit)->getName() << " has all zeros. stopping. " << std::endl;
#endif
            feas = false;
            break;
          }
        }
        if (false==feas || consfixed== (int) p_->getNumCons()) {
          break;
        }
      }
    }
  }
  if (true==feas) {
    addSol_(spool);
    logger_->msgStream(LogDebug1) << me_  << "feasible sol found with " 
      << num1 << " variables" << std::endl;
  } else {
    logger_->msgStream(LogDebug1) << me_  << "failed to find a feasible sol"
      << std::endl;
  }
}

void SppHeur::enum_(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  ConstraintPtr c;
  VariablePtr v;
  ConstraintVector cv;
  Function *f;
  bool cfixed;
  VarSetConstIter vsit;
  ConstraintConstIterator cit;
  SppHeurMods *m = 0;
  UInt minsize = 100000;
  int iter=0;

  for (cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    c->setBFlag(false);
    cv.push_back(c);
  }
  std::sort(cv.begin(), cv.end(), CompareLen);

  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit) {
    (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
  }

  // start fixing vars one by one
  mstack_.clear();

  cit = cv.begin();
  f = (*cit)->getFunction();
  for (vsit = f->varsBegin(); vsit != f->varsEnd(); ++vsit) {
    v = *vsit;
    if (10==v->getItmp()) {
      if (true==propEnum_(v, cit, vsit)) {
        break;
      }
    }
  }

  while (true) {
    ++iter;
    c = *cit;
    cfixed = false;

#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " trying constraint " 
      << c->getName() << " " << std::endl;
#endif

    // check if already fixed
    if (true==c->getBFlag()) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " is feasible already. Skipping " << std::endl;
#endif
      cfixed = true;
    } else {
      f = c->getFunction();
      for (; vsit != f->varsEnd(); ++vsit) {
        v = *vsit;
        if (10==v->getItmp()) {
          if (false==propEnum_(v, cit, vsit)) {
            continue;
          } else {
            cfixed=true;
            break;
          }
        }
      }
    }

    if (cfixed) {
      ++cit;
    }

    if (cit==cv.end() || false==cfixed || mstack_.size()>=minsize) {

      // logger_->msgStream(LogDebug2) << me_ << " reached the end\n" << std::endl
      //   << me_ << "stack size = " << mstack_.size() << std::endl;
      if (true==cfixed && cit==cv.end()) {
        minsize = mstack_.size();
        addSol_(spool);
      }

      if (false==cfixed) {
        // logger_->msgStream(LogInfo) << me_ << (*cit)->getName() << " exhausted " << std::endl;
        if (cit == cv.begin()) {
          // logger_->msgStream(LogInfo) << me_ << " all done " << std::endl;
          break;
        }
      }


      m = mstack_.back();

      vsit = m->vit;
      ++vsit;

      cit = m->cit;

      if (mstack_.size()<100 && cit!=cv.end()) {
        logger_->msgStream(LogDebug2) << me_ << " depth = " << mstack_.size() 
          << " " << (*cit)->getIndex() << " " << env_->getTime() << std::endl;
      }

      for (std::deque<Constraint *>::iterator it=m->cq.begin(); it!=m->cq.end(); ++it) {
        (*it)->setBFlag(false);
      }
      for (std::deque<Variable *>::iterator it=m->vq0.begin(); it!=m->vq0.end(); ++it) {
        (*it)->setItmp(10);
      }
      mstack_.pop_back();
      delete m;
    } else {
      vsit=(*cit)->getFunction()->varsBegin();
    }
  }
  exit(0);
}


void SppHeur::conWise_(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  ConstraintPtr c;
  VariablePtr v;
  ConstraintVector cv;
  FunctionPtr f;
  VarSetConstIter vsit;
  int val, j, num1, num0, kk;
  bool cfixed, feas;
  const double *xx;

  stats_.runs += 1;
  logger_->msgStream(LogDebug1) << me_ << " initial mode" 
    << " round " << stats_.runs << std::endl;

  num0 = 0;
  num1 = 0;
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    c->setITemp(0);
    c->setBFlag(false);
    cv.push_back(c);
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
      << " has " << c->getFunction()->getNumVars() << " variables"
      << std::endl;
#endif
  }

  std::sort(cv.begin(), cv.end(), CompareLen);
  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit) {
    (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
  }

  cv2_.insert(cv2_.end(), cv.begin(), cv.end());
  feas=true;
  cv.clear();

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
      }
    }
    logger_->msgStream(LogDebug2) << me_ << num0 << 
      " variables at 0. Reference solution value =  " 
      << spool->getBestSolution()->getObjValue() << std::endl;
  }


  // start fixing vars one by one
  j=0;
  for (ConstraintConstIterator cit = cv2_.begin(); cit != cv2_.end(); ++cit) {
    ++j;
    c = *cit;
    f = c->getFunction();
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " trying constraint number " << j
      << " out of " << cv2_.size() << " with " << f->getNumVars() << " vars. "
      << c->getName() << " "
      << num1 << " vars at 1 and " << num0 << " vars at 0 " << std::endl;
#endif

    // check if already fixed
    if (true==c->getBFlag()) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
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
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " setting variable " 
          << v->getName() << " to 1, i =  " << i <<  std::endl;
#endif
        if (false==propVal_(v, num0, kk)) {
          v->setItmp(0);
#if SPEW
          logger_->msgStream(LogDebug2) << me_ << " propagation found that var "
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
        logger_->msgStream(LogDebug2) << me_ << " variable " << v->getName()
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
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " feasible." << std::endl;
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
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
  logger_->msgStream(LogDebug2) << me_ << "cv size = " << cv.size() 
    << std::endl;
#endif

  if (feas) {
    addSol_(spool);
  } else {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << "Failed to find a feasible solution."
      << std::endl;
#endif
  }
}

void SppHeur::conWise2_(NodePtr, RelaxationPtr, SolutionPoolPtr spool)
{
  ConstraintPtr c;
  VariablePtr v;
  ConstraintVector cv;
  FunctionPtr f;
  VarSetConstIter vsit;
  int j, num1, num0, nz, kk;
  bool cfixed, feas;
  const double *xx;
  VarVector vvec;
  int consfixed = 0;

  stats_.runs += 1;
#if SPEW
  logger_->msgStream(LogDebug1) << me_ << " initial mode" 
    << " round " << stats_.runs << std::endl;
#endif

  num0 = 0;
  num1 = 0;
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    c->setITemp(c->getFunction()->getNumVars());
    c->setBFlag(false);
    cv.push_back(c);
  }

  std::sort(cv.begin(), cv.end(), CompareLen);
  feas=true;

  if (0<spool->getNumSols()) {
    // Fix some of the variables to zero. Choose some randomly from the
    // variables that have value one in an existing solution.
    j=0;
    xx=spool->getBestSolution()->getPrimal();
    for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
         ++vit,++j) {
      (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
      if (fabs(xx[j]-1.0)<1e-6 && (*vit)->getNumCons()>3 && rand()%100<30) {
        (*vit)->setItmp(0);
        ++num0;
      }
    }
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << num0 << " variables at 0."
     " Reference solution value =  " << spool->getBestSolution()->getObjValue()
     << std::endl;
#endif
  } else {
    for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
         ++vit) {
      (*vit)->setItmp(10); // 10 for FREE, 0 for FIXED at 0, 1 for FIXED at 1
    }
  }

  j=0;
  while (true) {
    int i=0;
    ++j;
    // pick a constraint with the fewest free vars
    c = NULL;
    nz = p_->getNumVars()+1;
    for (ConstraintConstIterator cit = cv.begin(); cit != cv.end(); ++cit) {
      if (false==(*cit)->getBFlag()) {
        ++i;
        if ((*cit)->getITemp()<nz) {
          c = (*cit);
          nz = c->getITemp();
        }
      }
    }

    if (0==i) {
#if SPEW
      logger_->msgStream(LogDebug1) << me_ << " all constraints done "
        << std::endl;
#endif
      break;
    }

    if (c==NULL) {
      logger_->errStream() << me_ << "Err: Could not find a constraint "
       << "to fix next. " << std::endl;
      break;
    }

    f = c->getFunction();
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << " trying constraint number " << j
      << " out of " << cv.size() << " with " << f->getNumVars() << " vars. "
      << c->getName() << " " << c->getITemp() << " unfixed vars. " 
      << num1 << " vars at 1 and " << num0 << " vars at 0 in model " 
      << std::endl;
#endif

    // sort vars first and put in a vec
    vvec.clear();
    for (vsit = f->varsBegin(); vsit != f->varsEnd(); ++vsit) {
      v = *vsit;
      if (v->getItmp()==10) {
        vvec.push_back(v);
      }
    }
    sortVars_(vvec);

    // pick one variable to set to 1
    cfixed=false;
    i=0;
    // std::cout << c->getITemp() << " " ;
    for (VariableConstIterator vit = vvec.begin(); vit != vvec.end(); ++vit,++i) {
      if (vvec.size()>500 && rand()%100<60) {
        continue;
      }
      v = *vit;
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " setting variable " 
        << v->getName() << " to 1, i =  " << i << " of " << vvec.size()
        << ". Con count = " << v->getNumCons() <<  std::endl;
#endif
      if (false==propVal_(v, num0, kk)) {
        v->setItmp(0);
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " propagation found that var "
          << "cannot be 1 " << v->getName() <<  ". Should be caught earlier"
          <<  std::endl;
#endif
      } else {
        // std::cout << i << " " << vvec.size() << " " << v->getNumCons() << std::endl;
        cfixed = true;
        ++consfixed;
        ++num1;
        break;
      }
    }

    if (cfixed) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " feasible." << std::endl;
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << " constraint " << c->getName()
        << " has all zeros. Infeasible." << std::endl;
#endif
      feas=false;
      break;
    }
  }

  if (feas) {
    addSol_(spool);
    // exit(0);
  } else {
#if SPEW
    logger_->msgStream(LogDebug1) << me_ << "Failed to find a feasible solution."
      << std::endl;
#endif
  }
}


void SppHeur::sortVars_(VarVector &vvec)
{
  struct
  {
    bool operator()(Variable *v1, Variable *v2) const
    {
      return (v1->getNumCons() > v2->getNumCons());
    }
  } compvars;
  std::sort(vvec.begin(), vvec.end(), compvars);
}

bool SppHeur::propEnum_(Variable* v, ConstraintConstIterator cit2, VarSetConstIter vit2)
{
  SppHeurMods *m = 0;
  Function* f = 0;

  for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
    if (true==(*cit)->getBFlag()) {
      return false;
    } 
  }

  m = new SppHeurMods();
  for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
    if ((*cit)->getBFlag() == false) {
      (*cit)->setBFlag(true);
      m->cq.push_back(*cit);
      f = (*cit)->getFunction();
      for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd(); ++vit) {
        if ((*vit)->getItmp()==10) {
          (*vit)->setItmp(0);
	  m->vq0.push_back(*vit);
        }
      }
    }
  }
  v->setItmp(1);
  m->cit = cit2;
  m->vit = vit2;
  mstack_.push_back(m);
  return true;
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


bool SppHeur::propVal_(VariablePtr v, int &num0, int &consfixed)
{
  FunctionPtr f;
  consfixed = 0;

  for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
    if (true==(*cit)->getBFlag()) {
      return false;
    }
  }

  for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
    if (false==(*cit)->getBFlag()) {
      (*cit)->setBFlag(true);
      f = (*cit)->getFunction();
      for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd(); ++vit) {
        if ((*vit)->getItmp()==10) {
          (*vit)->setItmp(0);
          ++num0;
          for (ConstrSet::iterator cit2=(*vit)->consBegin(); cit2!=(*vit)->consEnd(); ++cit2) {
            (*cit2)->setITemp((*cit2)->getITemp()-1);
          }
        }
      }
    } else {
      ++consfixed;
    }
  }
  v->setItmp(1);
  return true;
}

/// writing the statistics to the logger
void SppHeur::writeStats(std::ostream& out) const
{
  out << me_ << "Number of times called: " << stats_.calls << std::endl <<
  me_ << "Number of runs " << stats_.runs << std::endl <<
  me_ << "Number of solutions found: " << stats_.sols << std::endl <<
  me_ << "Time taken: " << stats_.time << std::endl;
}


bool SppHeur::isSpp_() 
{
  ConstraintPtr c;
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    if (c->getType() != SetPartition && c->getType()!=Aggregation) {
      logger_->msgStream(LogDebug1) << me_ << " model is not set partitioning" 
        << std::endl;
      return false;
    }
  }
  logger_->msgStream(LogDebug1) << me_ << " model is set partitioning" 
    << std::endl;
  return true;
}


