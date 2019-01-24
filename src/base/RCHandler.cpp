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



void RCHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr                            rel, CutManager *cutman, SolutionPoolPtr s_pool,
                         bool *sol_found, SeparationStatus *status)
{
  //std::cout<< "RCHandler: separate"<<std::endl;
  //double xval;
  VariableConstIterator v_iter;
  VariableType v_type;
  VarBoundMod *m = 0;
  const double *x = sol->getPrimal();
  const double *p = sol->getDualOfVars();  //reduced cost
  // Current LP relaxation objective value (in my view )   
  const double current_objval = sol->getObjValue();  
  // Best known solution
  double bestFeasible_objval = INFINITY;
  //std::cout<<"Number of solutions: "<<s_pool->getNumSols()<<std::endl;
  if (s_pool->getNumSols() > 0) {
      //double LastBest = bestFeasible_objval; 
      bestFeasible_objval = s_pool->getBestSolutionValue();
      //std::cout<<"curernt Best Feasible: "<< bestFeasible_objval << "last Best"<< LastBest <<std::endl;
  } else {
      return;
  } 
  double tolerance = (std::pow(10.0,-6));
  //std::cout<<"Tolerance"<<tolerance<<std::endl;
  for (v_iter=rel->varsBegin(); v_iter!=rel->varsEnd(); ++v_iter) {
     //xval = x[(*v_iter)->getIndex()];
     double r = p[(*v_iter)->getIndex()];
     //std::cout<<"inside the loop"<< r <<std::endl;
     if (r > tolerance){
       double lb = (*v_iter)->getLb();
       double new_ub  = lb +  (bestFeasible_objval-current_objval)/ r;
       //std::cout<<"old Bound UB for x" << (*v_iter)->getIndex()<<": " <<(*v_iter)->getUb()<< " new bound: "<< new_ub<<std::endl;
       if (new_ub < (*v_iter)->getUb() - tolerance*10 ){
        m = new VarBoundMod(*v_iter, Upper, new_ub);
        m->applyToProblem(rel); 
        //std::cout<<"Changed Bound UB for x" << (*v_iter)->getIndex()<< ": " <<(*v_iter)->getUb()<<std::endl;
       } 
       
     }
     if (r < -tolerance ){
       double ub = (*v_iter)->getUb();
       double new_lb  = ub +  (bestFeasible_objval-current_objval) / r;
       //std::cout<<"old Bound LB for x" << (*v_iter)->getIndex()<<":  "<<(*v_iter)->getLb()<< " new bound: "<< new_lb<<std::endl;
       if (new_lb > (*v_iter)->getLb() + tolerance*10 ){
       m = new VarBoundMod(*v_iter, Lower, new_lb);
       m->applyToProblem(rel);
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
void RCHandler::relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf){}
// Base class method.
void RCHandler::relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf){}
// Base class method. calls relax_().
void RCHandler::relaxInitInc(RelaxationPtr rel, bool *is_inf){}
// Base class method. 
//void relaxInitFull(RelaxationPtr rel, bool *is_inf){}

bool RCHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool & should_prune, double &inf_meas){}
/// Base class method
SolveStatus RCHandler::presolve(PreModQ *, bool *) {return Finished;}

/*
bool RCHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
{return false;}
*/
/// Base class method.
//void RCHandler::postsolveGetX(const double *, UInt, DoubleVector *) {}
// Base class method 
void RCHandler::relaxInitFull(RelaxationPtr rel, bool *is_inf){}

