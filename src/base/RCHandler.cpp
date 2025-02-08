//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file RCHandler.cpp
 * \author Vinay Chourasiya, IIT Bombay
 */

 
#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "VarBoundMod.h"
#include "Types.h"
#include "Node.h"
#include "RCHandler.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Logger.h"
#include "Relaxation.h"
#include "Timer.h"

using namespace Minotaur;
const std::string RCHandler::me_ = "RCHandler: ";

RCHandler::RCHandler(EnvPtr env)
{
  lastBest_ = INFINITY;
  logger_ = env->getLogger();
  rootDuals_ = 0;
  rootX_ = 0;
  rootObj_ = -INFINITY;
  timer_ = env->getTimer();

  stats_ = new RCStats();
  stats_->nlb = 0;
  stats_->nub = 0;
  stats_->time = 0;
}

RCHandler::~RCHandler()
{
  if (stats_) {
    delete stats_;
  }
  if (rootDuals_) {
    delete [] rootDuals_;
  }
  if (rootX_) {
    delete [] rootX_;
  }
}

void RCHandler::separate(ConstSolutionPtr sol, NodePtr node,
                         RelaxationPtr rel, CutManager *,
                         SolutionPoolPtr s_pool, ModVector &,
                         ModVector &r_mods, bool *,
                         SeparationStatus *)
{
  double start = timer_->query();
  double bestobj = INFINITY;
  const double* p = sol->getDualOfVars(); //reduced cost vector
  const double* x = sol->getPrimal(); 
  const double rel_obj = sol->getObjValue();
  double rnode, rroot; //Reduced costs
  double  xval;
  const double TOL=1e-3;
  VariablePtr v; 
  VariableConstIterator v_iter;
  
  if (node->getId() == 0){
    copyRootDetails_(sol, rel);
  }

  if (s_pool->getNumSols() > 0) {
    bestobj = s_pool->getBestSolutionValue();
  } else {
    stats_->time += (timer_->query() - start);
    return;
  }

  int i=0;
  for (v_iter = rel->varsBegin(); v_iter != rel->varsEnd(); ++v_iter,++i) {
    v = *v_iter;
    rnode = p[i];
    xval = x[i];
    rcfix_( rel, r_mods, bestobj, rel_obj, xval, rnode, v);
    
  }

  if (rootDuals_ && bestobj<lastBest_-TOL) {
    lastBest_=bestobj;
    for (v_iter = rel->varsBegin(); v_iter != rel->varsEnd(); ++v_iter){
      v = *v_iter;
      rroot = rootDuals_[v->getIndex()];
      xval = rootX_[v->getIndex()];
      rcfix_(rel, r_mods, bestobj, rootObj_, xval, rroot, v);
    } 
  }

  stats_->time += (timer_->query() - start);
  return;
}

void RCHandler::rcfix_(RelaxationPtr rel,
                       ModVector &r_mods, double bestobj,
                       const double rel_obj, double xval, 
                       double r, VariablePtr v)
{

  double ZTOL = 1e-6; // tolerance for checking if the variable is already fixed.
  double MINR = 1e-3; // if abs reduced cost is more than MINR then it is used
                      // for fixing, otherwise it may be error prone.
  double MINIMP = 1e-3; // bound changes less than MINIMP (before rounding) are
                        // ignored (integer variable)
  double MINREL = 1e-2; // bounds of continuous variable must improve
                        // by both MINIMP (abs) and MINREL (rel). bounds of 
                        // integer variable must improve by MINREL (rel) to
                        // be accepted.
  double new_ub, ub;
  double new_lb, lb;
  VarBoundModPtr m;
  VariableType v_type;
  lb = v->getLb();
  ub = v->getUb();

  if (ub - lb < ZTOL) {
    return;
  }

  v_type = v->getType();

  if (r > MINR){ //update upper bound
    new_ub = xval + (bestobj - rel_obj) / r;
    if (v_type == Binary || v_type == Integer || v_type == ImplBin ||
        v_type == ImplInt){
      new_ub = floor(new_ub + MINIMP);
      if (new_ub<ub-MINREL*abs(ub)) {
        m = (VarBoundModPtr) new VarBoundMod(v, Upper, new_ub);
        m->applyToProblem(rel);
        r_mods.push_back(m);
#if SPEW
        logger_->msgStream(LogDebug1) << me_ << "Variable name = "
          << v->getName() << " bound type = ub"
          << " old value = " << ub << " new value = "
          << new_ub << " (type = Int)" 
          << " xval = " << xval 
          << " red cost = " << r <<std::endl;
#endif
        ++(stats_->nub);
      } 
    } else { // a continuous variable
      if (new_ub<ub-MINIMP && new_ub<ub-MINREL*abs(ub)) {
        m = (VarBoundModPtr) new VarBoundMod(v, Upper, new_ub);
        m->applyToProblem(rel);
        r_mods.push_back(m);
#if SPEW
        logger_->msgStream(LogDebug1) << me_ << "Variable name = "
          << v->getName() << " bound type = ub"
          << " old value = "
          << ub << " new value = " << new_ub << std::endl;
#endif
        ++(stats_->nub);
      }
    }
  } else if (r < -MINR) { // update lower bound
    new_lb = xval + (bestobj - rel_obj) / r;
    if (v_type == Binary || v_type == Integer || v_type == ImplBin ||
        v_type == ImplInt){
      new_lb = ceil(new_lb - MINIMP);
      if (new_lb > lb+MINREL*abs(lb)) {
        m = (VarBoundModPtr) new VarBoundMod(v, Lower, new_lb);
        m->applyToProblem(rel);
        r_mods.push_back(m);
#if SPEW
        logger_->msgStream(LogDebug1) << me_ << "Variable name = "
          << (v)->getName()
          << " bound type = lb old value = "
          << lb << " new value = "
          << new_lb << " (type =Int)"
          << " xval = " << xval 
          << " red cost = " << r 
          << std::endl;
#endif
        ++(stats_->nlb);
      }
    } else { // continuous variable
      if (new_lb > lb+MINIMP && new_lb > lb+MINREL*abs(lb)) {
        m = (VarBoundModPtr) new VarBoundMod(v, Lower, new_lb);
        m->applyToProblem(rel);
        r_mods.push_back(m);
#if SPEW
        logger_->msgStream(LogDebug1) << me_ << "Variable name = "
          << (v)->getName()
          << " bound type = lb, old value = "
          << lb << " new value = "
          << new_lb << std::endl;
#endif
        ++(stats_->nlb);
      }
    }
  }
  return;
}

std::string RCHandler::getName() const
{
  return "RCHandler (Reduced Cost Strengthening)";
}

void RCHandler::copyRootDetails_(ConstSolutionPtr sol, RelaxationPtr rel)
{
  const double *p1 = sol->getDualOfVars();
  const double *x  = sol->getPrimal();
  int n = rel->getNumVars();
  int i;

  if (!rootDuals_){
    rootDuals_ = new double[n];
    rootX_     = new double[n];
    rootObj_   = sol->getObjValue();
  }

  i=0;
  for (i=0; i!=n; ++i){
    rootDuals_[i] = p1[i];
    rootX_[i] = x[i];
  }
  return;
}

void RCHandler::writeStats(std::ostream &out) const
{
  out << me_ << "Number of times lower bound changed = " << stats_->nlb
      << std::endl
      << me_ << "Number of times upper bound changed = " << stats_->nub
      << std::endl
      << me_ << "Time used = " << stats_->time << std::endl;
  return;
}
