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
  stats_   = new RCStats();
  stats_->nlb = 0;
  stats_->nub = 0;
  stats_->time = 0;
  logger_ = env->getLogger();
  timer_ = env->getTimer();
}

RCHandler::~RCHandler()
{
  if (stats_) {
  delete stats_;
  }
}

void RCHandler::separate(ConstSolutionPtr sol, NodePtr,
                         RelaxationPtr rel, CutManager *,
                         SolutionPoolPtr s_pool, ModVector &, 
                         ModVector &r_mods, bool *,
                         SeparationStatus *)
{
  VariableConstIterator v_iter;
  VariableType v_type;
  VarBoundModPtr m;
  const double *p = sol->getDualOfVars();   
  const double current_objval = sol->getObjValue();  
  double bestFeasible_objval = INFINITY;
  double start = timer_->query();
  double tolerance = (std::pow(10.0,-6));
  double r; //Reduced cost
  double ub,new_ub,current_ub;
  double lb,new_lb,current_lb;

  if (s_pool->getNumSols() > 0) {
      bestFeasible_objval = s_pool->getBestSolutionValue();
  } 
  else {
      stats_->time += (timer_->query()-start);
      return;
  } 
  
  for (v_iter=rel->varsBegin(); v_iter!=rel->varsEnd(); ++v_iter) 
  {
     r = p[(*v_iter)->getIndex()];
     lb = (*v_iter)->getLb();
     ub = (*v_iter)->getUb();
     if (ub - lb < tolerance) {
         continue;
     }

     if (r > tolerance)
     {
       v_type = (*v_iter)->getType();
       new_ub  = lb +  (bestFeasible_objval-current_objval)/ r;
       current_ub = (*v_iter)->getUb();
       
       if (v_type==Binary || v_type==Integer || v_type==ImplBin ||
           v_type==ImplInt)
       {
         //std::cout << (*v_iter)->getName() << " bound type = ub new ub = " << new_ub << " old ub = " << current_ub << " lb = " << lb << " best-current " << bestFeasible_objval-current_objval << " " << bestFeasible_objval << " " << current_objval << " r = " << r << std::endl;
         new_ub = floor((new_ub+tolerance*100));
         if (new_ub < current_ub)
         {
           m = (VarBoundModPtr) new VarBoundMod(*v_iter, Upper, new_ub);
           m->applyToProblem(rel); 
           r_mods.push_back(m);
#if SPEW
           logger_->msgStream(LogDebug) << me_ << "Variable name = " 
               << (*v_iter)->getName() << " bound type = ub"
               << " old value = "<< current_ub  << " new value = "
               << new_ub << " (type = Int)" << std::endl;
#endif   
           ++(stats_->nub);
         }
       }
       else{
         if (new_ub < current_ub*0.9)
         {
           m = (VarBoundModPtr) new VarBoundMod(*v_iter, Upper, new_ub);
           m->applyToProblem(rel); 
           r_mods.push_back(m);
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "Variable name = "
              << (*v_iter)->getName()<< " bound type =ub"<< " old value = "
              << current_ub << " new value = " << new_ub<< std::endl;
#endif   
           ++(stats_->nub); 
         } 
       }
     } else if (r < -tolerance ) {
       v_type = (*v_iter)->getType();
       new_lb  = ub +  (bestFeasible_objval-current_objval) / r;
       current_lb = (*v_iter)->getLb();
       if (v_type==Binary || v_type==Integer || v_type==ImplBin ||
           v_type==ImplInt)
         {
           new_lb =  ceil(new_lb - tolerance*100);
           if (new_lb < current_lb){
             m = (VarBoundModPtr) new VarBoundMod(*v_iter, Lower, new_lb);
             m->applyToProblem(rel);
             r_mods.push_back(m);

#if SPEW
             logger_->msgStream(LogDebug) << me_ << "Variable name = " 
                 << (*v_iter)->getName()<< " bound type = lb"
                 << " old value = " << current_lb << " new value = " 
                 << new_lb << " (type =Int)" <<std::endl;
#endif
            ++(stats_->nlb);
           }
         }
        else{
          if (new_lb > current_lb*1.1){
            m = (VarBoundModPtr) new VarBoundMod(*v_iter, Lower, new_lb);
            m->applyToProblem(rel);
            r_mods.push_back(m); 
#if SPEW
            logger_->msgStream(LogDebug) << me_ << "Variable name = "
                << (*v_iter)->getName()<< " bound type = lb"<< " old value = "
                << current_lb << " new value = " << new_lb<< std::endl;
#endif
            ++(stats_->nlb);
          }
        }
     }
  }
   
  stats_->time += (timer_->query()-start);
  return;
  }


// base class methods
ModificationPtr RCHandler::getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                                    BranchDirection)
  {return ModificationPtr();}

// base class methods
std::string RCHandler::getName() const
{
  return "RCHandler (Reduced Cost Strengthening)";
}                    

// Base class method.
void RCHandler::relaxNodeInc(NodePtr , RelaxationPtr , bool *){}
// Base class method.
void RCHandler::relaxNodeFull(NodePtr , RelaxationPtr , bool *){}
// Base class method. calls relax_().
void RCHandler::relaxInitInc(RelaxationPtr , bool *){}
// Base class method. 
//void relaxInitFull(RelaxationPtr rel, bool *is_inf){}

bool RCHandler::isFeasible(ConstSolutionPtr , RelaxationPtr , 
                  bool & , double &){return true;}
/// Base class method
SolveStatus RCHandler::presolve(PreModQ *, bool *) {return Finished;}
/// Base class method
void RCHandler::relaxInitFull(RelaxationPtr , bool *){}

void RCHandler::writeStats(std::ostream &out) const
{
  out
    << me_ << "Number of times lower bound changed = " << stats_->nlb << std::endl
    << me_ << "Number of times upper bound changed = " << stats_->nub << std::endl
    << me_ << "Time used = " << stats_->time << std::endl;
  return;
}

