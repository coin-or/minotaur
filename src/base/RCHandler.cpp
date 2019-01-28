//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file RCHandler.cpp
 * \author Vinay Chourasiya, IIT Bombay
 */
 
 // need to import .h file

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "VarBoundMod.h"
#include "Types.h"
#include "RCHandler.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Relaxation.h"
 
using namespace Minotaur;
const std::string RCHandler::me_ = "RCHandler: ";

RCHandler::RCHandler()
{
  
}

RCHandler::~RCHandler()
{

}



void RCHandler::separate(ConstSolutionPtr sol, NodePtr,
                         RelaxationPtr rel, CutManager *,
                         SolutionPoolPtr s_pool, ModVector &, 
                         ModVector &r_mods, bool *,
                         SeparationStatus *)
{
  VariableConstIterator v_iter;
  //VariableType v_type;
  VarBoundModPtr m;
  const double *p = sol->getDualOfVars();   
  const double current_objval = sol->getObjValue();  
  double bestFeasible_objval = INFINITY;
  if (s_pool->getNumSols() > 0) {
      bestFeasible_objval = s_pool->getBestSolutionValue();
  } 
  else {
      return;
  } 
  double tolerance = (std::pow(10.0,-6));
  
  for (v_iter=rel->varsBegin(); v_iter!=rel->varsEnd(); ++v_iter) 
  {
     double r = p[(*v_iter)->getIndex()];
     if (r > tolerance){
       double lb = (*v_iter)->getLb();
       double new_ub  = lb +  (bestFeasible_objval-current_objval)/ r;
       //std::cout<<"old Bound UB for x" << (*v_iter)->getIndex()<<": " <<(*v_iter)->getUb()<< " new bound: "<< new_ub<<std::endl;
       //round 
       if (new_ub < (*v_iter)->getUb() - tolerance*10 ){
        m = (VarBoundModPtr) new VarBoundMod(*v_iter, Upper, new_ub);
        m->applyToProblem(rel); 
        r_mods.push_back(m);
        //std::cout<<"Changed Bound UB for x" << (*v_iter)->getIndex()<< ": " <<(*v_iter)->getUb()<<std::endl;
       } 
       
     }
     if (r < -tolerance ){
       double ub = (*v_iter)->getUb();
       double new_lb  = ub +  (bestFeasible_objval-current_objval) / r;
       //std::cout<<"old Bound LB for x" << (*v_iter)->getIndex()<<":  "<<(*v_iter)->getLb()<< " new bound: "<< new_lb<<std::endl;
       if (new_lb > (*v_iter)->getLb() + tolerance*10 ){
       m = (VarBoundModPtr) new VarBoundMod(*v_iter, Lower, new_lb);
       m->applyToProblem(rel);
       r_mods.push_back(m);
       //std::cout<<"Changed Bound LB for x" << (*v_iter)->getIndex() <<": " <<(*v_iter)->getLb()<<std::endl;
       }
     }
     }
  }


// base class methods
ModificationPtr RCHandler::getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                           BranchDirection)
  {return ModificationPtr();}

// base class methods
std::string RCHandler::getName() const
{
  return "RCHandler";
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