////
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file AbsHandler.cpp
 * \brief Implement the handler for functions of type y = |x|.
 * \author Neeraj Kumar, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <algorithm>
#include <map>
#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrVarCand.h"
#include "Environment.h"
#include "AbsHandler.h"
#include "Function.h"
#include "LinMods.h"
#include "Logger.h"
#include "Node.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Variable.h"

using namespace Minotaur;

const std::string AbsHandler::me_ = "AbsHandler: ";

namespace {
const double kBig = 1e15;   // |bound| above this counts as infinite
}

AbsHandler::AbsHandler(EnvPtr env, ProblemPtr problem)
  : pStats_(),
    eTol_(1e-5),
    vTol_(1e-6),
    p_(problem),
    log_(env->getLogger())
{
}

AbsHandler::~AbsHandler()
{
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    delete *it;
  }
}

void AbsHandler::addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                               ConstVariablePtr ovar, char sense)
{
  Handler::addConstraint(newcon);
  consd_.push_back(new AbsCons(newcon, (VariablePtr) ivar, (VariablePtr) ovar,
                               sense));
}

void AbsHandler::addLin_(AbsCons &cd, RelaxationPtr rel)
{
  for (int i = 0; i < 2; ++i) {
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(cd.rov, 1.0);
    lf->addTerm(cd.riv, i == 0 ? -1.0 : 1.0);   // y - x >= 0, y + x >= 0
    rel->newConstraint((FunctionPtr) new Function(lf), 0.0, INFINITY);
  }
}

void AbsHandler::addSecant_(AbsCons &cd, RelaxationPtr rel)
{
  double a = cd.riv->getLb(), b = cd.riv->getUb();
  double m = 0.0, c = INFINITY;   // y - m*x <= c; c = inf: free row

  // Only the y <= |x| side (sense E/G) needs an upper bound.
  if (cd.sense != 'L') {
    if (a >= 0.0) {                         // |x| = x: y <= x, exact
      m = 1.0;
      c = 0.0;
    } else if (b <= 0.0) {                  // |x| = -x: y <= -x, exact
      m = -1.0;
      c = 0.0;
    } else if (a > -kBig && b < kBig) {     // straddles 0: chord
      m = (a + b) / (b - a);
      c = -2.0 * a * b / (b - a);
    } else if (a > -kBig) {                 // b = inf: slope-1 line through (a,|a|)
      m = 1.0;
      c = -2.0 * a;
    } else if (b < kBig) {                  // a = -inf: slope -1 line through (b,b)
      m = -1.0;
      c = 2.0 * b;
    }
  }

  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  if (m != 0.0) {
    lf->addTerm(cd.riv, -m);
  }
  if (cd.secCon) {
    rel->changeConstraint(cd.secCon, lf, -INFINITY, c);
  } else {
    cd.secCon = rel->newConstraint((FunctionPtr) new Function(lf), -INFINITY, c);
  }
}

void AbsHandler::relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf)
{
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    AbsCons &cd = **it;
    cd.riv = rel->getVariable(cd.iv->getIndex());
    cd.rov = rel->getVariable(cd.ov->getIndex());
    if (cd.sense != 'G') {   // y >= |x| is needed for E/L only
      addLin_(cd, rel);
    }
    addSecant_(cd, rel);
  }
  *is_inf = false;
}

void AbsHandler::relaxNodeInc(NodePtr, RelaxationPtr rel, bool *is_infeasible)
{
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    addSecant_(**it, rel);
  }
  *is_infeasible = false;
}

double AbsHandler::getViol_(const AbsCons &cd, const DoubleVector &x) const
{
  // x on one side of 0: |x| is linear, relaxation is exact, nothing to branch on
  if (cd.riv->getLb() >= 0.0 || cd.riv->getUb() <= 0.0) {
    return 0.0;
  }

  double fval = std::fabs(x[cd.riv->getIndex()]);
  double absViol = std::fabs(x[cd.rov->getIndex()] - fval);
  double relViol = absViol;

  if (fval + absViol > 1.0) {
    relViol = absViol / (fval + absViol);
  }
  return relViol;
}

bool AbsHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                            double &inf_meas)
{
  bool isfeas = true;
  const double *primal = sol->getPrimal();
  DoubleVector x(primal, primal + p_->getNumVars());

  inf_meas = 0.0;
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    double viol = getViol_(**it, x);
    if (viol > eTol_) {
      isfeas = false;
      inf_meas += viol;
    }
  }
  return isfeas;
}

void AbsHandler::separate(ConstSolutionPtr, NodePtr, RelaxationPtr,
                          CutManager *, SolutionPoolPtr, ModVector &,
                          ModVector &, bool *sol_found, SeparationStatus *)
{
  *sol_found = false;
}

void AbsHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector &x,
                                        ModVector &, BrVarCandSet &cands,
                                        BrCandVector &, bool &is_inf)
{
  std::map<ConstVariablePtr, double> allCands;
  std::map<ConstVariablePtr, double>::iterator curc_it;

  is_inf = false;
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    double curviol = getViol_(**it, x);   // > 0 only if x straddles 0
    if (curviol > eTol_) {
      ConstVariablePtr v = (*it)->riv;
      curc_it = allCands.find(v);
      if (curc_it == allCands.end()) {
        allCands[v] = curviol;
      } else {
        curc_it->second += curviol;
      }
    }
  }

  for (curc_it = allCands.begin(); curc_it != allCands.end(); ++curc_it) {
    BrVarCandPtr br_can = (BrVarCandPtr) new BrVarCand(
        curc_it->first, curc_it->first->getIndex(), curc_it->second,
        curc_it->second);
    cands.insert(br_can);
  }
}

// Always branch exactly at x = 0: both children are one-sided, so the
// relaxation is exact there and x is never branched on again.
ModificationPtr AbsHandler::getBrMod(BrCandPtr cand, DoubleVector &,
                                     RelaxationPtr, BranchDirection brdir)
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  VariablePtr v = dynamic_cast<BrVarCand *>(cand)->getVar();

  VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(
      v, (brdir == DownBranch) ? Upper : Lower, 0.0);
  lmods->insert(bmod);
  return lmods;
}

Branches AbsHandler::getBranches(BrCandPtr cand, DoubleVector &,
                                 RelaxationPtr rel, SolutionPoolPtr)
{
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();   // relaxation variable
  VariablePtr v2 = 0;                // original variable
  VarBoundModPtr mod;
  Branches branches = (Branches) new BranchPtrVector();

  // Down branch: x <= 0
  BranchPtr branch = (BranchPtr) new Branch();
  if (modProb_) {
    v2 = rel->getOriginalVar(v);
    mod = (VarBoundModPtr) new VarBoundMod(v2, Upper, 0.0);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod = (VarBoundModPtr) new VarBoundMod(v, Upper, 0.0);
    branch->addRMod(mod);
  }
  branch->setActivity(0.0);
  branches->push_back(branch);

  // Up branch: x >= 0
  branch = (BranchPtr) new Branch();
  if (modProb_) {
    mod = (VarBoundModPtr) new VarBoundMod(v2, Lower, 0.0);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod = (VarBoundModPtr) new VarBoundMod(v, Lower, 0.0);
    branch->addRMod(mod);
  }
  branch->setActivity(0.0);
  branches->push_back(branch);

  vcand->setNumBranches(2);
#if SPEW
  log_->msgStream(LogDebug1) << me_ << "branching on " << v->getName()
                             << " <= 0 or >= 0" << std::endl;
#endif
  return branches;
}

bool AbsHandler::propAbsBnds_(AbsConsPtr cd, bool *changed)
{
  VariablePtr x = cd->iv, y = cd->ov;

  // bounds can be propagated only through the equality
  if (cd->sense != 'E') {
    return false;
  }

  // x -> y: y in [dist(0, [xlb,xub]), max(|xlb|,|xub|)]
  double xlb = x->getLb(), xub = x->getUb();
  double lo = (xlb > 0.0) ? xlb : (xub < 0.0) ? -xub : 0.0;
  double hi = std::max(std::fabs(xlb), std::fabs(xub));
  if (updatePBnds_(y, lo, hi, changed) < 0) {
    return true;
  }

  // y -> x: |x| <= yub, and x cannot lie in (-ylb, ylb)
  double ylb = y->getLb(), yub = y->getUb();   // re-read after tightening
  lo = std::max(xlb, -yub);
  hi = std::min(xub, yub);
  if (xlb > -ylb) {
    lo = std::max(lo, ylb);
  }
  if (xub < ylb) {
    hi = std::min(hi, -ylb);
  }
  return updatePBnds_(x, lo, hi, changed) < 0;
}

int AbsHandler::updatePBnds_(VariablePtr p, double newlb, double newub,
                             bool *changed)
{
  double lb, ub;

  if (p->getType() == Binary || p->getType() == ImplBin ||
      p->getType() == Integer || p->getType() == ImplInt) {
    newlb = std::ceil(newlb - eTol_);
    newub = std::floor(newub + eTol_);
  }

  lb = std::max(p->getLb(), newlb);
  ub = std::min(p->getUb(), newub);
  if (lb > ub + eTol_) {
    return -1;
  }
  if (ub - lb <= vTol_) {   // treat nearly equal bounds as fixed
    lb = ub = 0.5 * (lb + ub);
  }

  if (lb != p->getLb() || ub != p->getUb()) {
    p->setLb_(lb);
    p->setUb_(ub);
    *changed = true;
    ++pStats_.vBnd;
  }
  return 0;
}

SolveStatus AbsHandler::presolve(PreModQ *, bool *changed, Solution **)
{
  ++pStats_.iters;
  *changed = false;
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    if (propAbsBnds_(*it, changed)) {
      return SolvedInfeasible;
    }
  }
  return Finished;
}

bool AbsHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr,
                              ModVector &, ModVector &)
{
  return false;
}

std::string AbsHandler::getName() const
{
  return "AbsHandler (Handling y=|x| terms).";
}

void AbsHandler::writeStats(std::ostream &out) const
{
  out << me_ << "Statistics for presolve by AbsHandler:" << std::endl
      << me_ << "Number of iterations           = " << pStats_.iters << std::endl
      << me_ << "Number of abs constraints      = " << consd_.size() << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd << std::endl;
}
