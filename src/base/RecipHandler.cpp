///
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file RecipHandler.cpp
 * \brief Implement the handler for functions of type
 * y = 1/x.
 * \author Neeraj Kumar, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <algorithm>
#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrVarCand.h"
#include "Environment.h"
#include "RecipHandler.h"
#include "Function.h"
#include "LinMods.h"
#include "Logger.h"
#include "Node.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Variable.h"

using namespace Minotaur;

const std::string RecipHandler::me_ = "RecipHandler: ";

namespace {
const double kPole = 1e-6;   // |x| below this is "at the pole" (assumes |y| <= 1/kPole)
const double kBig  = 1e15;   // |bound| above this counts as infinite
const double kTmin = 1e-3;   // tangent points are kept in [kTmin, kTmax] (numerics)
const double kTmax = 1e6;

// Range [lo, hi] of 1/t over t in [l, u]. Returns false if t = 0 is the
// only admissible value (empty image).
bool recipRange(double l, double u, double &lo, double &hi)
{
  lo = -INFINITY;
  hi = INFINITY;
  if (l >= 0.0 && u <= 0.0) {
    return false;
  }
  if (l >= 0.0) {            // t > 0
    lo = (u < kBig) ? 1.0 / u : 0.0;
    hi = (l > kPole) ? 1.0 / l : INFINITY;
  } else if (u <= 0.0) {     // t < 0
    lo = (u < -kPole) ? 1.0 / u : -INFINITY;
    hi = (l > -kBig) ? 1.0 / l : 0.0;
  }
  return true;
}
}  // namespace

RecipHandler::RecipHandler(EnvPtr env, ProblemPtr problem)
  : pStats_(),
    sStats_(),
    eTol_(1e-5),
    vTol_(1e-6),
    p_(problem),
    log_(env->getLogger())
{
}

RecipHandler::~RecipHandler()
{
  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    delete *it;
  }
}

void RecipHandler::addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                                 ConstVariablePtr ovar, char sense)
{
  Handler::addConstraint(newcon);
  consd_.push_back(new RecipCons(newcon, (VariablePtr) ivar,
                                 (VariablePtr) ovar, sense));
}

// Tangent at x0: y + x/x0^2 >= 2/x0 if x0 > 0 (convex side),
//                y + x/x0^2 <= 2/x0 if x0 < 0 (concave side).
ConstraintPtr RecipHandler::addTangent_(VariablePtr x, VariablePtr y,
                                        double x0, RelaxationPtr rel,
                                        ConstraintPtr row)
{
  double lb = -INFINITY, ub = INFINITY;
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(y, 1.0);
  if (x0 != 0.0) {
    lf->addTerm(x, 1.0 / (x0 * x0));
    (x0 > 0.0 ? lb : ub) = 2.0 / x0;
  }
  if (row) {
    rel->changeConstraint(row, lf, lb, ub);
    return row;
  }
  return rel->newConstraint((FunctionPtr) new Function(lf), lb, ub);
}

void RecipHandler::addLin_(RecipCons &cd, RelaxationPtr rel)
{
  double a = cd.riv->getLb(), b = cd.riv->getUb();
  int s = (a >= 0.0) ? 1 : (b <= 0.0) ? -1 : 0;
  double p[3] = {1.0, 1.0, 1.0};   // tangent points, as |x|

  // x > 0: y >= tangent (sense E/L); x < 0: y <= tangent (sense E/G).
  // Otherwise no tangent is valid and the rows stay free.
  bool use = (s > 0 && cd.sense != 'G') || (s < 0 && cd.sense != 'L');

  if (use) {
    double lo = std::min(std::max(s > 0 ? a : -b, kTmin), kTmax);
    double hi = std::min(std::max(s > 0 ? b : -a, lo), kTmax);
    double pad = 0.05 * (hi - lo);
    p[0] = lo;
    p[1] = hi;
    // tangents at lo and hi meet at the harmonic mean, kept 5% inside
    p[2] = std::min(std::max(2.0 * lo * hi / (lo + hi), lo + pad), hi - pad);
  }
  for (int i = 0; i < 3; ++i) {
    cd.linCons[i] = addTangent_(cd.riv, cd.rov, use ? s * p[i] : 0.0, rel,
                                cd.linCons[i]);
  }
}

void RecipHandler::addSecant_(RecipCons &cd, RelaxationPtr rel)
{
  double a = cd.riv->getLb(), b = cd.riv->getUb();
  double m = 0.0, lb = -INFINITY, ub = INFINITY;
  bool pos = (a >= kPole), neg = (b <= -kPole);

  // x > 0: y <= chord (sense E/G); x < 0: y >= chord (sense E/L)
  if ((pos && cd.sense != 'L') || (neg && cd.sense != 'G')) {
    double c;
    if (std::fabs(pos ? b : a) < kBig) {   // y = (a + b - x)/(ab)
      m = -1.0 / (a * b);
      c = (a + b) / (a * b);
    } else {                               // unbounded end: flat row
      c = 1.0 / (pos ? a : b);
    }
    (pos ? ub : lb) = c;
  }

  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  if (m != 0.0) {
    lf->addTerm(cd.riv, -m);
  }
  if (cd.secCon) {
    rel->changeConstraint(cd.secCon, lf, lb, ub);
  } else {
    cd.secCon = rel->newConstraint((FunctionPtr) new Function(lf), lb, ub);
  }
}

bool RecipHandler::relaxRecip_(RecipCons &cd, RelaxationPtr rel, bool init)
{
  if (init) {
    cd.riv = rel->getVariable(cd.iv->getIndex());
    cd.rov = rel->getVariable(cd.ov->getIndex());
  }

  // x confined to the pole neighbourhood forces |y| >= 1/kPole
  if (cd.riv->getLb() >= -kPole && cd.riv->getUb() <= kPole) {
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "x = " << cd.riv->getName()
                               << " confined to the pole; node infeasible"
                               << std::endl;
#endif
    return true;
  }

  // Row validity depends on the sign of x, so both are rewritten at every
  // node (rows are created at the root).
  addSecant_(cd, rel);
  addLin_(cd, rel);
  return false;
}

void RecipHandler::relaxInitInc(RelaxationPtr rel, SolutionPool *,
                                bool *is_inf)
{
  *is_inf = false;
  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    if (relaxRecip_(**it, rel, true)) {
      *is_inf = true;
      return;
    }
  }
}

void RecipHandler::relaxNodeInc(NodePtr, RelaxationPtr rel, bool *is_infeasible)
{
  *is_infeasible = false;
  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    if (relaxRecip_(**it, rel, false)) {
      *is_infeasible = true;
      return;
    }
  }
}

double RecipHandler::getViol_(const RecipCons &cd, const DoubleVector &x) const
{
  double xval = x[cd.riv->getIndex()];
  double fhat = x[cd.rov->getIndex()];
  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();

  // x fixed away from the pole: relaxation is exact, nothing to branch on
  if (xub - xlb < vTol_ && std::fabs(xlb) >= kPole) {
    return 0.0;
  }
  // at the pole: maximal violation; getBrPoint_ guarantees this shrinks
  if (std::fabs(xval) < kPole) {
    return 1e6;
  }

  double fval = 1.0 / xval;
  double absViol = std::fabs(fhat - fval);
  double relViol = absViol;

  if (std::fabs(fval) + absViol > 1.0) {
    relViol = absViol / (std::fabs(fval) + absViol);
  }
  return relViol;
}

bool RecipHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel,
                              bool &should_prune, double &inf_meas)
{
  return isFeasible_(sol, rel, should_prune, inf_meas);
}

bool RecipHandler::isFeasible_(ConstSolutionPtr sol, RelaxationPtr, bool &,
                               double &inf_meas)
{
  bool isfeas = true;
  const double *primal = sol->getPrimal();
  DoubleVector x(primal, primal + p_->getNumVars());

  inf_meas = 0.0;
  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    double viol = getViol_(**it, x);
    if (viol > eTol_) {
      isfeas = false;
      inf_meas += viol;
    }
  }
  return isfeas;
}

void RecipHandler::addCut_(VariablePtr x, VariablePtr y, double xval,
                           double yval, RelaxationPtr rel, bool &ifcuts)
{
  double f = 1.0 / xval;
  // d > 0: y is on the side of 1/x that the tangent at xval cuts off
  double d = (xval > 0.0) ? f - yval : yval - f;

  ifcuts = (d > eTol_ * std::max(1.0, std::fabs(f)));
  if (ifcuts) {
    addTangent_(x, y, xval, rel, 0);
  }
}

void RecipHandler::separate(ConstSolutionPtr sol, NodePtr, RelaxationPtr rel,
                            CutManager *, SolutionPoolPtr, ModVector &,
                            ModVector &, bool *sol_found, SeparationStatus *)
{
  const double *x = sol->getPrimal();
  ++sStats_.iters;
  *sol_found = false;

  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    RecipCons *cd = *it;
    VariablePtr x_var = rel->getRelaxationVar(cd->iv);
    VariablePtr y_var = rel->getRelaxationVar(cd->ov);
    double xval = x[x_var->getIndex()];
    double yval = x[y_var->getIndex()];
    bool added = false;

    // near the pole the tangent is badly scaled: leave it to branching
    if (std::fabs(xval) < kTmin) {
      continue;
    }
    // The tangent at xval is valid only if the whole node domain lies on
    // xval's side of the pole, and only for the sense that needs it
    // (x > 0: y >= f, E/L; x < 0: y <= f, E/G).
    if (xval > 0.0 ? (x_var->getLb() < 0.0 || cd->sense == 'G')
                   : (x_var->getUb() > 0.0 || cd->sense == 'L')) {
      continue;
    }
    addCut_(x_var, y_var, xval, yval, rel, added);
    if (added) {
      ++sStats_.tangentcuts;
    }
  }
}

void RecipHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector &x,
                                          ModVector &, BrVarCandSet &cands,
                                          BrCandVector &, bool &is_inf)
{
  std::map<ConstVariablePtr, double> allCands;
  std::map<ConstVariablePtr, double>::iterator curc_it;

  is_inf = false;
  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    double curviol = getViol_(**it, x);
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

// x straddles 0: branch at 0. x touches 0: branch at +-kPole, so that the
// child inside the pole neighbourhood is pruned in relaxRecip_ and the other
// one is pole-free. Otherwise: LP value pushed 10% inside the bounds.
double RecipHandler::getBrPoint_(VariablePtr v, double xval) const
{
  double lb = v->getLb(), ub = v->getUb();

  if (lb < -kPole && ub > kPole) {
    return 0.0;
  }
  if (lb >= -kPole && lb < kPole) {
    return kPole;
  }
  if (ub > -kPole && ub <= kPole) {
    return -kPole;
  }
  lb = std::max(lb, -kTmax);   // cap unbounded domains
  ub = std::min(ub, kTmax);
  double margin = 0.1 * (ub - lb);
  return std::min(std::max(xval, lb + margin), ub - margin);
}

ModificationPtr RecipHandler::getBrMod(BrCandPtr cand, DoubleVector &x,
                                       RelaxationPtr, BranchDirection brdir)
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  double value = getBrPoint_(v, x[v->getIndex()]);

  VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(
      v, (brdir == DownBranch) ? Upper : Lower, value);
  lmods->insert(bmod);
  return lmods;
}

Branches RecipHandler::getBranches(BrCandPtr cand, DoubleVector &x,
                                   RelaxationPtr rel, SolutionPoolPtr)
{
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();   // relaxation variable
  VariablePtr v2 = 0;                // original variable
  double value = getBrPoint_(v, x[v->getIndex()]);
  VarBoundModPtr mod;
  Branches branches = (Branches) new BranchPtrVector();

  // Down branch
  BranchPtr branch = (BranchPtr) new Branch();
  if (modProb_) {
    v2 = rel->getOriginalVar(v);
    mod = (VarBoundModPtr) new VarBoundMod(v2, Upper, value);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    branch->addRMod(mod);
  }
  branch->setActivity(value);
  branches->push_back(branch);

  // Up branch
  branch = (BranchPtr) new Branch();
  if (modProb_) {
    mod = (VarBoundModPtr) new VarBoundMod(v2, Lower, value);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    branch->addRMod(mod);
  }
  branch->setActivity(value);
  branches->push_back(branch);

  vcand->setNumBranches(2);
#if SPEW
  log_->msgStream(LogDebug1) << me_ << "branching on " << v->getName()
                             << " <= " << value << " or >= " << value
                             << std::endl;
#endif
  return branches;
}

bool RecipHandler::propRecipBnds_(RecipConsPtr cd, bool *changed)
{
  double lo, hi;

  // bounds can be propagated only through the equality
  if (cd->sense != 'E') {
    return false;
  }
  // x -> y
  if (!recipRange(cd->iv->getLb(), cd->iv->getUb(), lo, hi)) {
    return true;
  }
  if (updatePBnds_((VariablePtr) cd->ov, lo, hi, changed) < 0) {
    return true;
  }
  // y -> x (y bounds re-read: they may just have been tightened)
  if (!recipRange(cd->ov->getLb(), cd->ov->getUb(), lo, hi)) {
    return true;
  }
  if (updatePBnds_((VariablePtr) cd->iv, lo, hi, changed) < 0) {
    return true;
  }
  return false;
}

int RecipHandler::updatePBnds_(VariablePtr p, double newlb, double newub,
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

bool RecipHandler::varBndsFromCons_(bool *changed)
{
  for (RecipConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    if (propRecipBnds_(*it, changed)) {
      return true;
    }
  }
  return false;
}

SolveStatus RecipHandler::presolve(PreModQ *, bool *changed, Solution **)
{
  ++pStats_.iters;
  *changed = false;
  if (varBndsFromCons_(changed)) {
    return SolvedInfeasible;
  }
  return Finished;
}

bool RecipHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr,
                                ModVector &, ModVector &)
{
  return false;
}

std::string RecipHandler::getName() const
{
  return "RecipHandler (Handling 1/x terms).";
}

void RecipHandler::writeStats(std::ostream &out) const
{
  out << me_ << "Statistics for presolve by RecipHandler:" << std::endl
      << me_ << "Number of iterations           = " << pStats_.iters << std::endl
      << me_ << "Number of recip constraints    = " << consd_.size() << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd << std::endl;

  out << me_ << "Statistics for separation by RecipHandler:" << std::endl
      << me_ << "Number of calls to separate    = " << sStats_.iters << std::endl
      << me_ << "Number of tangent cuts added   = " << sStats_.tangentcuts
      << std::endl;
}
