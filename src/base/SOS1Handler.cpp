//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2013 The MINOTAUR Team.
//

/**
 * \file SOS1Handler.cpp
 * \brief Declare the SOS1Handler class for handling SOS type I constraints.
 * It checks integrality and provides branching candidates. Does
 * not do any presolving and cut-generation.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>

#include "MinotaurConfig.h"
#include "BrCand.h"
#include "Branch.h"
#include "Environment.h"
#include "Logger.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "SOS.h"
#include "SOS1Handler.h"
#include "VarBoundMod.h"
#include "Variable.h"

#define SPEW 1

using namespace Minotaur;
const std::string SOS1Handler::me_ = "SOS1Handler: ";

SOS1Handler::SOS1Handler(EnvPtr env, ProblemPtr problem)
  : env_(env)
{
  //logger_   = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
  //                                 findInt("intvar_h_log_level")->getValue());
  logger_   = (LoggerPtr) new Logger(LogDebug2);
  intTol_   = env_->getOptions()->findDouble("int_tol")->getValue();
  problem_  = problem;
}


SOS1Handler::~SOS1Handler()
{
  problem_.reset();
  env_.reset();
  logger_.reset();
}


Bool SOS1Handler::isFeasible(ConstSolutionPtr , RelaxationPtr relaxation, 
                             Bool &)
{
  return true;
}


void SOS1Handler::getBranchingCandidates(RelaxationPtr rel, 
                                           const std::vector< Double > &x,
                                           ModVector &, BrCandSet & cands,
                                           Bool & is_inf)
{
  SOSConstIterator siter, siter2;
  VariableConstIterator viter;
  SOSPtr sos;
  Double solval;
  Bool is_feas = true;
  Int nfracs = 0;
  VarVector frac_vars;

  for (siter=rel->sos1Begin(); siter!=rel->sos1End(); ++siter) {
    sos = *siter;
    getFracsOrd_(sos, x, frac_vars);
  }

  if (frac_vars.size()>3) {
    // TODO here.
  }

  is_inf = false;
}


void SOS1Handler::getFracsOrd_(SOSPtr sos, const std::vector< Double > &x,
                               VarVector &frac_vars)
{
  VariableConstIterator viter;
  Int numfixed = 0;
  Int index;
  ConstVariablePtr v;
  DoubleVector frac_vals;

  for (viter=sos->varsBegin(); viter!=sos->varsEnd(); ++viter) {
    v = *viter;
    index = v->getIndex();
    if (fabs(floor(x[index]+0.5) - x[index]) > intTol_) {
      frac_vars.push_back(v);
      frac_vals.push_back(x[v->getIndex()]-floor(x[v->getIndex()]));
    }
  }

  sort(frac_vars, &(frac_vals[0]));
  /*
    for (viter=sos->varsBegin(); viter!=sos->varsEnd(); ++viter) {
      nfixed = 0;
      v = *viter;
      solval = x[v->getIndex()];
      index = v->getIndex();
        // check if number of fixed variables is large
        nfixed = getNumFixed_(sos);
        if (sos->getNz - nfixed > 3) {
        } 
        break;
      }
  */
}


#if 0
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
#endif
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
