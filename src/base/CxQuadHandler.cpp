//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2021 The MINOTAUR Team.
//

/**
 * \file CxQuadHandler.cpp
 * \brief Implement the handler for functions of the general quadratic form 
 * \f$ x^TAx \leq b \f$, where \f$A\f$ is psd,
 * or \f$ x^TAx \geq b \f$, where \f$A\f$ is nsd.
 * \author Mustafa Vora, IIT Bombay
 */

#include <algorithm>
#include <cmath>

#include "Constraint.h"
#include "CxQuadHandler.h"
#include "Environment.h"
#include "Function.h"
#include "MinotaurConfig.h"
#include "Operations.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Types.h"

using namespace Minotaur;

const std::string CxQuadHandler::me_ = "CxQuadHandler: ";

CxQuadHandler::CxQuadHandler(EnvPtr env)
  : aTol_(1e-6),
    rTol_(1e-7)
{
  logger_ = env->getLogger();
  ncuts_ = 0;
}

CxQuadHandler::~CxQuadHandler() { }

void CxQuadHandler::relaxInitFull(RelaxationPtr, bool*)
{
  // Do Nothing
}

void CxQuadHandler::relaxInitInc(RelaxationPtr, bool*)
{
  // Do Nothing
}

void CxQuadHandler::relaxNodeFull(NodePtr, RelaxationPtr, bool*)
{
  // Do Nothing
}

void CxQuadHandler::relaxNodeInc(NodePtr, RelaxationPtr, bool*)
{
  assert(!"CxQuadHandler::relaxNodeInc not implemented!");
}

bool CxQuadHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool&,
                               double&)
{
  ConstraintPtr c;
  double act, clb, cub;
  const double* x = sol->getPrimal();
  int error = 0;
  bool is_feas = true;

  for(ConstraintConstIterator cit = consBegin(); cit != consEnd(); ++cit) {
    c = *cit;
    act = c->getActivity(x, &error);
    if(error == 0) {
      cub = c->getUb();
      clb = c->getLb();
      if(((act > cub + aTol_) && (cub == 0 || act > cub + fabs(cub) * rTol_)) ||
         ((act < clb - aTol_) && (clb == 0 || act < clb - fabs(clb) * rTol_))) {
        is_feas = false;
      }
    } else {
      logger_->msgStream(LogError)
          << me_ << c->getName() << " Constraint not defined at this point."
          << std::endl;
      is_feas = false;
    }
  }

  return is_feas;
}

void CxQuadHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector&,
                                           ModVector&, BrVarCandSet&,
                                           BrCandVector&, bool&)
{
  // No branching candidates will be returned.
}

ModificationPtr CxQuadHandler::getBrMod(BrCandPtr, DoubleVector&, RelaxationPtr,
                                        BranchDirection)
{
  return ModificationPtr();
}

Branches CxQuadHandler::getBranches(BrCandPtr, DoubleVector&, RelaxationPtr,
                                    SolutionPoolPtr)
{
  return Branches();
}

void CxQuadHandler::separate(ConstSolutionPtr sol, NodePtr, RelaxationPtr rel,
                             CutManager*, SolutionPoolPtr, ModVector&,
                             ModVector&, bool*, SeparationStatus*)
{
  const double* x = sol->getPrimal();
  ConstraintPtr c, cut;
  double fval, cub, clb;
  double* grad = new double[rel->getNumVars()];
  double rhs;
  FunctionPtr f, fcut;
  LinearFunctionPtr lfcut;
  int errorf = 0, errorg = 0, i;

  for(ConstraintConstIterator cit = consBegin(); cit != consEnd(); ++cit) {
    c = *cit;
    f = c->getFunction();
    fval = f->eval(x, &errorf);
    f->evalGradient(x, grad, &errorg);
    if(errorf == 0 && errorg == 0) {
      clb = c->getLb();
      cub = c->getUb();
      if((fval > cub + aTol_) && (cub == 0 || fval > cub + fabs(cub) * rTol_)) {
        lfcut = (LinearFunctionPtr) new LinearFunction();
        i = 0;
        for(VariableConstIterator it = rel->varsBegin(); it != rel->varsEnd();
            ++it) {
          lfcut->addTerm((*it), x[i]);
          ++i;
        }
        fcut = (FunctionPtr) new Function(lfcut);
        rhs = cub;
        rhs += InnerProduct(x, grad, rel->getNumVars());
        rhs -= fval;
        cut = rel->newConstraint(fcut, -INFINITY, rhs);
#if SPEW
        logger_->msgStream(LogDebug)
            << me_ << "added new constraint" << std::endl;
        cut->write(logger_->msgStream(LogDebug));
#endif
      }
      if((fval < clb - aTol_) && (clb == 0 || fval < clb - fabs(clb) * rTol_)) {
        lfcut = (LinearFunctionPtr) new LinearFunction();
        i = 0;
        for(VariableConstIterator it = rel->varsBegin(); it != rel->varsEnd();
            ++it) {
          lfcut->addTerm((*it), x[i]);
          ++i;
        }
        fcut = (FunctionPtr) new Function(lfcut);
        rhs = clb;
        rhs += InnerProduct(x, grad, rel->getNumVars());
        rhs -= fval;
        cut = rel->newConstraint(fcut, rhs, INFINITY);
#if SPEW
        logger_->msgStream(LogDebug)
            << me_ << "added new constraint" << std::endl;
        cut->write(logger_->msgStream(LogDebug));
#endif
      }
    }
  }
}

bool CxQuadHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr,
                                 ModVector&, ModVector&)
{
  // TO IMPLEMENT
  return false;
}

SolveStatus CxQuadHandler::presolve(PreModQ*, bool*, Solution**)
{
  // TO IMPLEMENT
  return Finished;
}

std::string CxQuadHandler::getName() const
{
  return "CxQuadHandler (Handling convex quadratic constraints).";
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
