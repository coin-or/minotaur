//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
//

/**
 * \file IntVarHandler.cpp
 * \brief Define the IntVarHandler class for handling integer constrained
 * variables. It checks integrality and provides branching candidates. Does
 * not do any presolving and cut-generation.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>

#include "MinotaurConfig.h"
#include "BrCand.h"
#include "Branch.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "Logger.h"
#include "Option.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"

//#define SPEW 1

using namespace Minotaur;
const std::string IntVarHandler::me_ = "IntVarHandler: ";

IntVarHandler::IntVarHandler(EnvPtr env, ProblemPtr problem)
  : env_(env)
{
  logger_   = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
                                     findInt("intvar_h_log_level")->getValue());
  intTol_   = env_->getOptions()->findDouble("int_tol")->getValue();
  gDive_    = env_->getOptions()->findBool("guided_dive")->getValue();
  mRelOnly_ = env_->getOptions()->findBool("modify_rel_only")->getValue();
  problem_  = problem;
}


IntVarHandler::~IntVarHandler()
{
  problem_.reset();
  env_.reset();
  logger_.reset();
}


Bool IntVarHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                               Bool &)
{
  VariableConstIterator v_iter, v_iter2;
  VariableType v_type;
  Double value;
  const Double *x = sol->getPrimal();
  Bool is_feas = true;

  for (v_iter=relaxation->varsBegin(); v_iter!=relaxation->varsEnd(); 
       ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type==Binary || v_type==Integer) {
      value = x[(*v_iter)->getIndex()];
      if (fabs(value - floor(value+0.5)) > intTol_) {
        is_feas = false;
#if SPEW
        logger_->MsgStream(LogDebug) << me_ << "variable " <<
          (*v_iter)->getName() << " has fractional value = " << value <<
          std::endl;
#endif
        break;
      }
    }
  }
#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "is_feas = " << is_feas << std::endl;
#endif
  return is_feas;
}


void IntVarHandler::getBranchingCandidates(RelaxationPtr rel, 
                                           const std::vector< Double > &x,
                                           ModVector &, BrCandSet & cands,
                                           Bool & is_inf)
{
  VariablePtr v;
  VariableType v_type;
  UInt index;
  BrVarCandPtr br_can;

  for (VariableConstIterator it=rel->varsBegin(); it!=rel->varsEnd(); ++it) {
    v = *it;
    v_type = v->getType();
    index = v->getIndex();
    if ((v_type==Binary || v_type==Integer) && 
        fabs(floor(x[index]+0.5) - x[index]) > intTol_) {
      // yes, it can be branched upon.
      br_can = (BrVarCandPtr) new BrVarCand(v, v->getIndex(), 
          x[index]-floor(x[index]));
      cands.insert(br_can);
    } 
  }
  is_inf = false;
}


ModificationPtr IntVarHandler::getBrMod(BrCandPtr cand, DoubleVector & x,
                                        RelaxationPtr , BranchDirection dir) 
{
  // TODO: fix this dynamic cast
  BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  VariablePtr v = vcand->getVar();
  VarBoundModPtr mod;
  Double bnd;

  if (dir==DownBranch) {
    bnd = floor(x[v->getIndex()]);
    mod = (VarBoundModPtr) new VarBoundMod(v, Upper, bnd);
  } else {
    bnd = ceil(x[v->getIndex()]);
    mod = (VarBoundModPtr) new VarBoundMod(v, Lower, bnd);
  }
  return mod;
}


Branches IntVarHandler::getBranches(BrCandPtr cand, DoubleVector & x, 
                                    RelaxationPtr rel, SolutionPoolPtr s_pool)
{
  BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  VariablePtr v = vcand->getVar();
  Double value = x[v->getIndex()];
  BranchPtr branch1, branch2;
  Branches branches = (Branches) new BranchPtrVector();
  VarBoundModPtr mod;
  SolutionPtr bestsol = s_pool->getBestSolution();

  if (!mRelOnly_) {
    v = rel->getOriginalVar(v);
  }
  mod = (VarBoundModPtr) new VarBoundMod(v, Upper, floor(value));
  branch1 = (BranchPtr) new Branch();
  branch1->addMod(mod);
  branch1->setActivity(value);

  mod = (VarBoundModPtr) new VarBoundMod(v, Lower, ceil(value));
  branch2 = (BranchPtr) new Branch();
  branch2->addMod(mod);
  branch2->setActivity(value);

  if (true==gDive_ && bestsol) {
    if (bestsol->getPrimal()[v->getIndex()] < x[v->getIndex()]) {
      branches->push_back(branch1);
      branches->push_back(branch2);
    } else {
      branches->push_back(branch2);
      branches->push_back(branch1);
    }
  } else {
    if (cand->getDir() == DownBranch) {
      branches->push_back(branch1);
      branches->push_back(branch2);
    } else {
      branches->push_back(branch2);
      branches->push_back(branch1);
    }
  }
  return branches;
}


Double IntVarHandler::getTol() const
{
  return intTol_;
}


Bool IntVarHandler::isNeeded()
{
  if (problem_) {
    problem_->calculateSize();
    if (problem_->getSize()->bins > 0 || problem_->getSize()->ints > 0) {
      return true;
    }
  }
  return false;
}


void IntVarHandler::relaxInitFull(RelaxationPtr, Bool *is_inf)
{
  *is_inf = false;
}


void IntVarHandler::relaxInitInc(RelaxationPtr , Bool *is_inf)
{
  *is_inf = false;
}


void IntVarHandler::relaxNodeFull(NodePtr , RelaxationPtr, Bool *is_inf)
{
  *is_inf = false;
}


void IntVarHandler::relaxNodeInc(NodePtr , RelaxationPtr , Bool *is_inf)
{
  *is_inf = false;
}


void IntVarHandler::setTol(Double tol)
{
  intTol_ = tol;
}


std::string IntVarHandler::getName() const
{
  return "IntVarHandler (Handling integrality of variables).";
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
