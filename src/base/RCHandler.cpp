//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
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
  stats_ = new RCStats();
  stats_->nlb = 0;
  stats_->nub = 0;
  stats_->time = 0;
  rootDuals_ = 0;
  logger_ = env->getLogger();
  timer_ = env->getTimer();
}

RCHandler::~RCHandler()
{
  if (stats_) {
    delete stats_;
  }
  if (rootDuals_) {
    delete [] rootDuals_;
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
  VariablePtr v; 
  VariableConstIterator v_iter;
  
  if (node->getId() == 0){
    copyRootDetails_(sol, rel);
  }

  if (s_pool->getNumSols() > 0)
  {
    bestobj = s_pool->getBestSolutionValue();
  } else{
    stats_->time += (timer_->query() - start);
    return;
  }

  for (v_iter = rel->varsBegin(); v_iter != rel->varsEnd(); ++v_iter){
    v = *v_iter;
    rnode = p[v->getIndex()];
    xval = x[v->getIndex()];
    rcfix_( rel, r_mods, bestobj, rel_obj, xval, rnode, v);
    if (rootDuals_) {
      rroot = rootDuals_[v->getIndex()];
    }
    rcfix_( rel, r_mods, bestobj, rel_obj, xval, rroot, v);
  }

  stats_->time += (timer_->query() - start);
  return;
}

void RCHandler::rcfix_(RelaxationPtr,
                       ModVector &r_mods, double bestobj,
                       const double rel_obj, double xval, 
                       double r, VariablePtr v)
{

  double tolerance = (std::pow(10.0, -6));
  double new_ub, ub;
  double new_lb, lb;
  VarBoundModPtr m;
  VariableType v_type;
  lb = v->getLb();
  ub = v->getUb();

  if (ub - lb < tolerance) {
    return;
  }

  v_type = v->getType();

  if (r > tolerance){
    new_ub = xval + (bestobj - rel_obj) / r;
    if (v_type == Binary || v_type == Integer || v_type == ImplBin ||
        v_type == ImplInt){
          new_ub = floor(new_ub + tolerance*100);
          if (new_ub < ub){
            m = (VarBoundModPtr) new VarBoundMod(v, Upper, new_ub);
            // m->applyToProblem(rel);
            r_mods.push_back(m);
#if SPEW
            logger_->msgStream(LogDebug) << me_ << "Variable name = "
                                         << v->getName() << " bound type = ub"
                                         << " old value = " << ub << " new value = "
                                         << new_ub << " (type = Int)" << std::endl;
#endif
            ++(stats_->nub);
          } else{
            if (new_ub < ub*0.9){
              m = (VarBoundModPtr) new VarBoundMod(v, Upper, new_ub);
              // m->applyToProblem(rel);
              r_mods.push_back(m);
#if SPEW
              logger_->msgStream(LogDebug) << me_ << "Variable name = "
                                           << v->getName() << " bound type = ub"
                                           << " old value = "
                                           << ub << " new value = " << new_ub << std::endl;
#endif
              ++(stats_->nub);
            }
          }

    }
  } else if (r < -tolerance){
    new_lb = xval + (bestobj - rel_obj) / r;
    if (v_type == Binary || v_type == Integer || v_type == ImplBin ||
        v_type == ImplInt){
          new_lb = ceil(new_lb - tolerance * 100);
          if (new_lb > lb){
            m = (VarBoundModPtr) new VarBoundMod(v, Lower, new_lb);
            //m->applyToProblem(rel);
            r_mods.push_back(m);
#if SPEW
            logger_->msgStream(LogDebug) << me_ << "Variable name = "
                                         << (v)->getName()
                                         << " bound type = lb old value = "
                                         << lb << " new value = "
                                         << new_lb << " (type =Int)"
                                         << std::endl;
#endif
            ++(stats_->nlb);
          }
        }else{
          if (new_lb > lb * 1.1){
            m = (VarBoundModPtr) new VarBoundMod(v, Lower, new_lb);
            //m->applyToProblem(rel);
            r_mods.push_back(m);
#if SPEW
            logger_->msgStream(LogDebug) << me_ << "Variable name = "
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
  rootValue_ = sol->getObjValue();
  VariableConstIterator vIter;
  const double *p1 = sol->getDualOfVars();
  int n = rel->getNumVars();

  if (!rootDuals_){
    rootDuals_ = new double[n];
  }

  for (vIter = rel->varsBegin(); vIter != rel->varsEnd(); ++vIter){
    rootDuals_[(*vIter)->getIndex()] = p1[(*vIter)->getIndex()];
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
