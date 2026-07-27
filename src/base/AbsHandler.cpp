///
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file AbsHandler.cpp
 * \brief Implement the handler for functions of type y = |x|.
 * \author Neeraj Kumar, IIT Bombay
 */
#include "Branch.h"
#include "BrVarCand.h"
#include <cmath>
#include <iostream>
#include "MinotaurConfig.h"
#include "Constraint.h"
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

AbsHandler::AbsHandler(EnvPtr env, ProblemPtr problem)
  : bTol_(1e-6),
    env_(env),
    aTol_(1e-6),
    rTol_(1e-6),
    eTol_(1e-6),
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

// -----------------------------------------------------------------------
// y >= x and y >= -x. Both are GLOBAL, exact, and never change with
// bounds -- so they are added exactly once, at init, and never touched
// again (same as PowHandler's tangent cuts for a convex function).
// -----------------------------------------------------------------------
void AbsHandler::addLin_(AbsCons &cd, RelaxationPtr rel)
{
  LinearFunctionPtr lf1 = (LinearFunctionPtr) new LinearFunction();
  lf1->addTerm(cd.rov, 1.0);
  lf1->addTerm(cd.riv, -1.0);
  FunctionPtr f1 = (FunctionPtr) new Function(lf1);
  cd.linCons.push_back(rel->newConstraint(f1, 0.0, INFINITY));   // y - x >= 0

  LinearFunctionPtr lf2 = (LinearFunctionPtr) new LinearFunction();
  lf2->addTerm(cd.rov, 1.0);
  lf2->addTerm(cd.riv, 1.0);
  FunctionPtr f2 = (FunctionPtr) new Function(lf2);
  cd.linCons.push_back(rel->newConstraint(f2, 0.0, INFINITY));   // y + x >= 0
}

// -----------------------------------------------------------------------
// Upper-bounding secant: y <= m*x + c on [xlb, xub].
// Only needed while the domain straddles zero -- once branching has
// pushed the domain to one side of zero (xlb>=0 or xub<=0), |x| is
// exactly linear on that side and this secant coincides exactly with
// one of the two lines from addLin_. The relaxation is then EXACT.
// -----------------------------------------------------------------------
void AbsHandler::addSecant_(AbsCons &cd, RelaxationPtr rel, ModVector &mods,
                            bool init)
{
  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();

  if (xlb <= -1e15 || xub >= 1e15) {
    return;
  }

  double fxlb = std::fabs(xlb);
  double fxub = std::fabs(xub);
  double m = 0.0;
  if (xub - xlb > 1e-9) {
    m = (fxub - fxlb) / (xub - xlb);
  }
  double intercept = fxlb - m * xlb;

  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -m);

  if (!cd.secCon) {
    FunctionPtr f = (FunctionPtr) new Function(lf);
    cd.secCon = rel->newConstraint(f, -INFINITY, intercept);
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "added secant at root for "
                               << cd.rov->getName() << " = |" << cd.riv->getName()
                               << "|" << std::endl;
#endif
  } else {
    rel->changeConstraint(cd.secCon, lf, -INFINITY, intercept);
    if (!init) {
      LinConModPtr lcmod =
        (LinConModPtr) new LinConMod(cd.secCon, lf, -INFINITY, intercept);
      mods.push_back(lcmod);
    }
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "modified secant for "
                               << cd.rov->getName() << " = |" << cd.riv->getName()
                               << "|" << std::endl;
#endif
  }
}

void AbsHandler::initRelax_(AbsCons &cd, RelaxationPtr rel)
{
  cd.riv = rel->getVariable(cd.iv->getIndex());
  cd.rov = rel->getVariable(cd.ov->getIndex());
  ModVector dummy;
  addLin_(cd, rel);
  addSecant_(cd, rel, dummy, true);
}

void AbsHandler::updateRelax_(AbsCons &cd, RelaxationPtr rel, ModVector &mods)
{
  cd.riv = rel->getVariable(cd.iv->getIndex());
  cd.rov = rel->getVariable(cd.ov->getIndex());
  addSecant_(cd, rel, mods, false);
}

void AbsHandler::relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf)
{
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    initRelax_(**it, rel);
  }
  *is_inf = false;
}

void AbsHandler::relaxNodeInc(NodePtr, RelaxationPtr rel, bool *is_infeasible)
{
  ModVector mods;
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    updateRelax_(**it, rel, mods);
  }
  *is_infeasible = false;
}

void AbsHandler::addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                               ConstVariablePtr ovar, char sense)
{
  Handler::addConstraint(newcon);
  VariablePtr vivar = (VariablePtr) ivar;
  VariablePtr vovar = (VariablePtr) ovar;
  consd_.push_back(new AbsCons(newcon, vivar, vovar, sense));
}

double AbsHandler::getViol_(const AbsCons &cd, const DoubleVector &x) const
{
  double xval = x[cd.riv->getIndex()];
  double fhat = x[cd.rov->getIndex()];
  double fval = std::fabs(xval);
  double absViol = std::fabs(fhat - fval);
  double relViol = absViol;
  if (std::fabs(fval) + absViol > 1.0) {
    relViol = absViol / (std::fabs(fval) + absViol);
  }
  return relViol;
}

bool AbsHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &isfeas,
                            double &inf_meas)
{
  isfeas = true;
  inf_meas = 0.0;
  const double *primal = sol->getPrimal();
  DoubleVector x(primal, primal + p_->getNumVars());

  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    double viol = getViol_(**it, x);
    if (viol > eTol_) {
      isfeas = false;
      inf_meas += viol;
    }
  }
  return isfeas;
}

// -----------------------------------------------------------------------
// Branching: always split exactly at x=0
// further branching on this variable is ever needed.
// -----------------------------------------------------------------------
//
void AbsHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                          CutManager *cutman, SolutionPoolPtr s_pool,
                          ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                          SeparationStatus *status)
{
  (void)sol; (void)node; (void)rel; (void)cutman; (void)s_pool;
  (void)p_mods; (void)r_mods; (void)status;
  *sol_found = false;
  // No separation needed on the underestimator side (t>=x, t>=-x are exact
  // and global). Overestimator (secant) side is refreshed via updateRelax_
  // at every node already; nothing additional to cut here for now.
}

ModificationPtr AbsHandler::getBrMod(BrCandPtr cand, DoubleVector &x,
                                     RelaxationPtr rel, BranchDirection brdir)
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  double minFromBds = 0.1;
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  (void)rel;

  double xval = x[v->getIndex()];
  double value = xval;
  double len = v->getUb() - v->getLb();
  if (value < v->getLb() + minFromBds * len) {
    value = v->getLb() + minFromBds * len;
  } else if (value > v->getUb() - minFromBds * len) {
    value = v->getUb() - minFromBds * len;
  }

  if (brdir == DownBranch) {
    VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    lmods->insert(bmod);
  } else if (brdir == UpBranch) {
    VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    lmods->insert(bmod);
  }
  return lmods;
}
void AbsHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector &x,
                                        ModVector &, BrVarCandSet &cands,
                                        BrCandVector &, bool &is_inf)
{
  is_inf = false;
  for (AbsConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    AbsCons &cd = **it;
    double curviol = getViol_(cd, x);
    if (curviol > eTol_ && cd.riv->getLb() < -bTol_ && cd.riv->getUb() > bTol_) {
      BrVarCandPtr br_can = (BrVarCandPtr) new BrVarCand(
        cd.riv, cd.riv->getIndex(), curviol, curviol);
      cands.insert(br_can);
    }
  }
}

Branches AbsHandler::getBranches(BrCandPtr cand, DoubleVector &, RelaxationPtr rel,
                                 SolutionPoolPtr)
{
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  VariablePtr v2 = rel->getOriginalVar(v);
  Branches branches = (Branches) new BranchPtrVector();

  // Down branch: x <= 0
  BranchPtr branchDown = (BranchPtr) new Branch();
  branchDown->addPMod((VarBoundModPtr) new VarBoundMod(v2, Upper, 0.0));
  branchDown->addRMod((VarBoundModPtr) new VarBoundMod(v, Upper, 0.0));
  branchDown->setActivity(0.0);
  branches->push_back(branchDown);

  // Up branch: x >= 0
  BranchPtr branchUp = (BranchPtr) new Branch();
  branchUp->addPMod((VarBoundModPtr) new VarBoundMod(v2, Lower, 0.0));
  branchUp->addRMod((VarBoundModPtr) new VarBoundMod(v, Lower, 0.0));
  branchUp->setActivity(0.0);
  branches->push_back(branchUp);

  vcand->setNumBranches(2);
  return branches;
}

int AbsHandler::updatePBnds_(VariablePtr p, double newlb, double newub, bool *changed)
{
  double lb = p->getLb();
  double ub = p->getUb();

  if (newlb > lb) { lb = newlb; *changed = true; }
  if (newub < ub) { ub = newub; *changed = true; }

  if (lb > ub + eTol_) {
    return -1;
  }
  if (fabs(ub - lb) <= vTol_) {
    double mid = 0.5 * (lb + ub);
    lb = mid;
    ub = mid;
  }
  if (*changed) {
    p->setLb_(lb);   // fine: called from within a Handler-derived class
    p->setUb_(ub);   // that Variable presumably grants access to
  }
  return 0;
}

bool AbsHandler::propAbsBnds_(AbsConsPtr cdata, bool *changed)
{
  VariablePtr x = cdata->iv, y = cdata->ov;
  double xlb = x->getLb(), xub = x->getUb();

  double new_ylb;
  if (xlb > 0.0) {
    new_ylb = xlb;
  } else if (xub < 0.0) {
    new_ylb = -xub;
  } else {
    new_ylb = 0.0;
  }
  double new_yub = std::max(std::fabs(xlb), std::fabs(xub));

  if (updatePBnds_(y, new_ylb, new_yub, changed) < 0) {
    return true;
  }

  double new_xlb = std::max(xlb, -y->getUb());
  double new_xub = std::min(xub, y->getUb());
  if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) {
    return true;
  }

  return false;
}
SolveStatus AbsHandler::presolve(PreModQ *, bool *changed, Solution **)
{
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
void AbsHandler::relaxInitFull(RelaxationPtr rel, SolutionPool *sp, bool *is_inf) {
  // Usually, you just route this to your incremental init function, 
  // or your shared relaxation builder.
  relaxInitInc(rel, sp, is_inf);
}

void AbsHandler::relaxNodeFull(NodePtr node, RelaxationPtr rel, bool* is_inf) {
  // If MINOTAUR is set up to use incremental node relaxations (relaxNodeInc),
  // this is rarely called.
  assert(!"AbsHandler::relaxNodeFull not implemented!");
  *is_inf = false;
}
std::string AbsHandler::getName() const
{
  return "AbsHandler (Handling y=|x| terms).";
}

void AbsHandler::writeStats(std::ostream &out) const
{
  out << me_ << "Number of abs constraints = " << consd_.size() << std::endl;
}
