//
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
#include <iomanip>
#include <algorithm>
#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrVarCand.h"
#include "Constraint.h"
#include "Environment.h"
#include "RecipHandler.h"
#include "Function.h"
#include "LinMods.h"
#include "Logger.h"
#include "Node.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "QuadraticFunction.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Variable.h"

//#define SPEW 1

using namespace Minotaur;

const std::string RecipHandler::me_ = "RecipHandler: ";

RecipHandler::RecipHandler(EnvPtr env, ProblemPtr problem)
  : bStats_(),
    pStats_(),
    sStats_(),
    LBd_(-1e6),
    UBd_(1e6),
    bTol_(1e-6),
    env_(env),
    orig_(problem),
    optCuts_(),
    aTol_(1e-6),
    rTol_(1e-6),
    eTol_(1e-6),
    vTol_(1e-6),
    p_(problem),
    log_(env->getLogger()),
    tmpX_(),
    grad_()
{
  tmpX_.assign(problem->getNumVars(), 0.0);
  grad_.assign(problem->getNumVars(), 0.0);
}

RecipHandler::~RecipHandler()
{
  RecipConsIter it;
  for (it = consd_.begin(); it != consd_.end(); ++it) {
    delete *it;
  }
}

void RecipHandler::initRelax_(RecipCons &cd, RelaxationPtr rel, DoubleVector &tmpX, DoubleVector &grad)
{
  ModVector mods;
  cd.riv = rel->getVariable(cd.iv->getIndex());
  cd.rov = rel->getVariable(cd.ov->getIndex());

  if (cd.sense == 'E' || cd.sense == 'L') {
    addSecant_(cd, rel, tmpX, mods, true);
  }
  if (cd.sense == 'E' || cd.sense == 'G') {
    addLin_(cd, rel, tmpX, grad, mods, true);
  }
}

void RecipHandler::updateRelax_(RecipCons &cd, RelaxationPtr rel, DoubleVector &tmpX, DoubleVector &grad, ModVector &mods)
{
  if (cd.sense == 'E' || cd.sense == 'L') {
    addSecant_(cd, rel, tmpX, mods, false);
  }
  if (cd.sense == 'E' || cd.sense == 'G') {
    addLin_(cd, rel, tmpX, grad, mods, false);
  }
}

double RecipHandler::getViol_(const RecipCons &cd, const DoubleVector &x) const
{
  double xval, fval, fhat, absViol, relViol;
  
  xval = x[cd.riv->getIndex()];
  fhat = x[cd.rov->getIndex()];

  // Massive violation if evaluated directly at  0
  if (std::abs(xval) < 1e-9) return 1e9;

  fval = 1.0 / xval;
  absViol = std::abs(fhat - fval); 
  relViol = absViol;

  if (std::fabs(fval) + absViol > 1.0) {
    relViol = absViol / (std::fabs(fval) + absViol);
  }

  return relViol;
}

void RecipHandler::addLin_(RecipCons &cd, RelaxationPtr rel, DoubleVector &, DoubleVector &, ModVector &, bool)
{
  ConstraintPtr cons;
  double xlb, xub, margin, tmpxval;
  double fxlbval, fxubval, dfxlbval, dfxubval;
  double fxval, dfxval, rhs;
  LinearFunctionPtr lf;
  FunctionPtr f;
  double xvals[3];
  int i;

  xlb = cd.riv->getLb();
  xub = cd.riv->getUb();

  // Cannot draw tangents if domain spans 0. 
  // Node must be branched into strictly positive or negative domains first.
  if (xlb <= 0.0 && xub >= 0.0) return;

  if (xlb > 0.0) {
    xlb = std::max(xlb, 1e-5);
    xub = std::min(xub, 1e15);
  } else {
    xlb = std::max(xlb, -1e15);
    xub = std::min(xub, -1e-5);
  }

  if (xub - xlb < 1e-6) return;

  xvals[0] = xlb;
  xvals[1] = xub;
  xvals[2] = xlb + (xub - xlb) / 2.0;

  fxlbval = 1.0 / xlb;
  dfxlbval = -1.0 / (xlb * xlb);
  fxubval = 1.0 / xub;
  dfxubval = -1.0 / (xub * xub);

  if (std::abs(dfxubval - dfxlbval) > 1e-6) { 
    tmpxval = (dfxubval * xub - dfxlbval * xlb + fxlbval - fxubval) / (dfxubval - dfxlbval);
    margin  = (xub - xlb) * 0.05;
    xvals[2] = std::max(xlb + margin, std::min(xub - margin, tmpxval));
  }

  for (i = 0; i < 3; ++i) {
    fxval  = 1.0 / xvals[i];
    dfxval = -1.0 / (xvals[i] * xvals[i]); 
    rhs    = fxval - dfxval * xvals[i];

    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(cd.rov, 1.0);
    lf->addTerm(cd.riv, -dfxval);

    f = (FunctionPtr) new Function(lf);

    // Flip based on convexity (x>0) or concavity (x<0)
    if (xvals[i] > 0.0) {
      cons = rel->newConstraint(f, rhs, INFINITY); // Underestimator
    } else {
      cons = rel->newConstraint(f, -INFINITY, rhs); // Overestimator
    }
    cd.linCons.push_back(cons);

#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Tangent added for 1/x at x=" << xvals[i] << std::endl;
#endif
  }
}

void RecipHandler::addSecant_(RecipCons &cd, RelaxationPtr rel, DoubleVector &, ModVector &mods, bool init)
{
  double xlb, xub;
  double fxlb, fxub, m, intercept;
  LinearFunctionPtr lf;
  FunctionPtr f;
  LinConModPtr lcmod;

  xlb = cd.riv->getLb();
  xub = cd.riv->getUb();

  //  initialize the constraint at the root node 
  if (init) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(cd.rov, 1.0);
    f = (FunctionPtr) new Function(lf);
    // Start completely inactive [-inf, inf] so it doesn't restrict the root node
    cd.secCon = rel->newConstraint(f, -INFINITY, INFINITY); 
  }

  if (xlb <= -1e15 || xub >= 1e15) return;
  
  // If the domain strictly touches 0 due to branching, shift it minutely 
  // so we can draw a valid secant instead of abandoning it.
  if (xlb < 0.0 && xub >= 0.0 && xub < 1e-5) {
     xub = -1e-5;
  } else if (xub > 0.0 && xlb <= 0.0 && xlb > -1e-5) {
     xlb = 1e-5;
  }

  // Cannot draw secant if domain still has 0
  if (xlb <= 0.0 && xub >= 0.0) return;

  fxlb = 1.0 / xlb;
  fxub = 1.0 / xub;

  if (xub - xlb > 1e-6) {
    m = (fxub - fxlb) / (xub - xlb);
  } else {
    m = 0.0;
  }

  intercept = fxlb - m * xlb;

  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -m);

  //  Direction Flip for Convex/Concave domains
  double lb_val = (xlb > 0.0) ? -INFINITY : intercept;
  double ub_val = (xlb > 0.0) ? intercept : INFINITY;

  if (init) {
    rel->changeConstraint(cd.secCon, lf, lb_val, ub_val); 
  } else {
    rel->changeConstraint(cd.secCon, lf, lb_val, ub_val); 
    lcmod = (LinConModPtr) new LinConMod(cd.secCon, lf, lb_val, ub_val);
    mods.push_back(lcmod);
  }

#if SPEW
  log_->msgStream(LogDebug1) << me_ << (init ? "Added" : "Modified") 
       << " secant cut. Bounds: [" << lb_val << ", " << ub_val << "]" 
       << " | Eq: y - (" << m << ")x. Domain: [" << xlb << ", " << xub << "]" << std::endl;
#endif
}

void RecipHandler::relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf)
{
  RecipConsIter it;
  if (tmpX_.size() != p_->getNumVars()) tmpX_.assign(p_->getNumVars(), 0.0);
  if (grad_.size() != p_->getNumVars()) grad_.assign(p_->getNumVars(), 0.0);

  for (it = consd_.begin(); it != consd_.end(); ++it) {
    initRelax_(**it, rel, tmpX_, grad_);
  }
  *is_inf = false;
}

void RecipHandler::relaxNodeInc(NodePtr, RelaxationPtr rel, bool *is_infeasible)
{ 
  ModVector mods;
  RecipConsIter it;

  for (it = consd_.begin(); it != consd_.end(); ++it) {
    (*it)->riv = rel->getVariable((*it)->iv->getIndex());
    (*it)->rov = rel->getVariable((*it)->ov->getIndex());
    updateRelax_(**it, rel, tmpX_, grad_, mods);
  }
  *is_infeasible = false;
}

void RecipHandler::addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar, ConstVariablePtr ovar, char sense)
{
  VariablePtr vivar;
  VariablePtr vovar;
  
  Handler::addConstraint(newcon);
  vivar = (VariablePtr) ivar;
  vovar = (VariablePtr) ovar;
  consd_.push_back(new RecipCons(newcon, vivar, vovar, sense));
}

void RecipHandler::addCut_(VariablePtr x, VariablePtr y, double xval, double yval, RelaxationPtr rel, bool& ifcuts) 
{
  double fval, deriv, rhs, violation;
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr cut;
  
  ifcuts = false;
  if (std::abs(xval) < 1e-9) return;

  fval = 1.0 / xval;
  violation = std::abs(yval - fval);

  if (violation > aTol_) {
    deriv = -1.0 / (xval * xval);
    rhs = fval - deriv * xval;

    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.0);
    lf->addTerm(x, -deriv);
    f = (FunctionPtr) new Function(lf);
    
    if (xval > 0.0) {
      cut = rel->newConstraint(f, rhs, INFINITY);
    } else {
      cut = rel->newConstraint(f, -INFINITY, rhs);
    }
    (void)cut;
    ifcuts = true;
  } 
}

bool RecipHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, bool& isfeas, double& inf_meas)
{
  return isFeasible_(sol, rel, isfeas, inf_meas);
}

bool RecipHandler::isFeasible_(ConstSolutionPtr sol, RelaxationPtr, bool &isfeas, double &inf_meas)
{
  int num_inf, num1;
  double viol;
  const double *primal;
  RecipConsIter it;
  RecipCons *cd;

  isfeas = true;
  num_inf = 0;
  inf_meas = 0.0;
  num1 = 0;
  primal = sol->getPrimal();
  DoubleVector x(primal, primal + p_->getNumVars());

  for (it = consd_.begin(); it != consd_.end(); ++it) {
    num1++;
    cd = *it;
    viol = getViol_(*cd, x);

    if (viol > eTol_) {
      if (cd->riv->getUb() - cd->riv->getLb() > vTol_) {
        isfeas = false;
        inf_meas += viol;
        num_inf++;
      }
    }  
  }
  return isfeas;
}

void RecipHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, CutManager* cutman, SolutionPoolPtr s_pool, 
                            ModVector& p_mods, ModVector& r_mods, bool* sol_found, SeparationStatus* status)
{
  (void)node; (void)cutman; (void)s_pool; (void)p_mods; (void)r_mods; (void) status;
  
  double xval, yval;
  const double* x;
  bool added;
  RecipConsIter it;
  RecipCons* cd;
  VariablePtr x_var, y_var;

  x = sol->getPrimal();
  ++sStats_.iters;
  *sol_found = false;
  
  for(it = consd_.begin(); it != consd_.end(); ++it) {
    cd = *it;
    x_var = rel->getRelaxationVar(cd->iv);
    y_var = rel->getRelaxationVar(cd->ov);
    
    xval = x[x_var->getIndex()];
    yval = x[y_var->getIndex()];

    if (std::abs(xval) < 1e-9) continue; 

    added = false;
    addCut_(x_var, y_var, xval, yval, rel, added);

    if (added) {
      ++sStats_.tangentcuts;
    }
  }
}

void RecipHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector &x, ModVector &, BrVarCandSet &cands, BrCandVector &, bool &is_inf)
{
  std::map<ConstVariablePtr, double> allCands;
  std::map<ConstVariablePtr, double>::iterator curc_it;
  RecipConsIter it;
  ConstVariablePtr v;
  double curviol;
  BrVarCandPtr br_can;

  is_inf = false;

  for (it = consd_.begin(); it != consd_.end(); ++it) {
    curviol = getViol_(**it, x);
    if (curviol > eTol_) {
      v = (*it)->riv;
      curc_it = allCands.find(v);
      if (curc_it == allCands.end()) {
        allCands[v] = curviol;
      } else {
        curc_it->second += curviol;
      }
    }
  }

  for (curc_it = allCands.begin(); curc_it != allCands.end(); ++curc_it) {
    br_can = (BrVarCandPtr) new BrVarCand(curc_it->first, curc_it->first->getIndex(), curc_it->second, curc_it->second);
    cands.insert(br_can);
  }
}

ModificationPtr RecipHandler::getBrMod(BrCandPtr cand, DoubleVector &x, RelaxationPtr, BranchDirection brdir)
{
  LinModsPtr lmods;
  BrVarCandPtr vcand;
  VariablePtr v;
  double xval, value, len, minFromBds;
  VarBoundModPtr bmod;

  lmods = (LinModsPtr) new LinMods();
  minFromBds = 0.1;
  vcand = dynamic_cast<BrVarCand *>(cand);
  v = vcand->getVar();
  xval = x[v->getIndex()];
  value = xval;
  len = v->getUb() - v->getLb();

  // Zero Check
  if (v->getLb() < -1e-6 && v->getUb() > 1e-6) {
    value = 0.0; // Force spatial branch at 0
  } else {
    if (value < v->getLb() + minFromBds * len) value = v->getLb() + minFromBds * len;
    else if (value > v->getUb() - minFromBds * len) value = v->getUb() - minFromBds * len;
  }

  if (brdir == DownBranch) {
    bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
  } else {
    bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
  }
  
  lmods->insert(bmod);
  return lmods;
}

Branches RecipHandler::getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel, SolutionPoolPtr)
{
  double minFromBds, xval, value, len;
  BrVarCandPtr vcand;
  VariablePtr v, v2;
  VarBoundModPtr mod;
  Branches branches;
  BranchPtr branch;

  minFromBds = 0.1;
  vcand = dynamic_cast<BrVarCand *>(cand);
  v = vcand->getVar(); 
  v2 = rel->getOriginalVar(v);
  xval = x[v->getIndex()];
  value = xval;
  len = v->getUb() - v->getLb();
  branches = (Branches) new BranchPtrVector();

  //  ZERO  CHECK
  if (v->getLb() < -1e-6 && v->getUb() > 1e-6) {
    value = 0.0; // Split node exactly at 0 to stabilize children
  } else {
    if (value < v->getLb() + minFromBds * len) value = v->getLb() + minFromBds * len;
    else if (value > v->getUb() - minFromBds * len) value = v->getUb() - minFromBds * len;
  }

  // Down Branch
  branch = (BranchPtr) new Branch();
  mod = (VarBoundModPtr) new VarBoundMod(v2, Upper, value);
  branch->addPMod(mod);
  mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
  branch->addRMod(mod);
  branch->setActivity(value);
  branches->push_back(branch);

  // Up Branch
  branch = (BranchPtr) new Branch();
  mod = (VarBoundModPtr) new VarBoundMod(v2, Lower, value);
  branch->addPMod(mod);
  mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
  branch->addRMod(mod);
  branch->setActivity(value);
  branches->push_back(branch);
  
  vcand->setNumBranches(2);
  return branches;
}

BranchPtr RecipHandler::doBranch_(BranchDirection UpOrDown, ConstVariablePtr v, double bvalue)
{
  BranchPtr branch;
  LinModsPtr linmods;
  VarBoundModPtr mod;
  UInt j;

  for (j = 0; j < tmpX_.size(); ++j) {
    tmpX_[j] = 0.0;
    grad_[j] = 0.0;
  }

  branch  = (BranchPtr) new Branch();
  linmods = (LinModsPtr) new LinMods();

  if (UpOrDown == DownBranch) {
    mod = (VarBoundModPtr) new VarBoundMod(VariablePtr(const_cast<Variable *>(v)), Upper, bvalue);
  } else {
    mod = (VarBoundModPtr) new VarBoundMod(VariablePtr(const_cast<Variable *>(v)), Lower, bvalue);
  }
  
  linmods->insert(mod);
  branch->addPMod(linmods);
  return branch;
}

void RecipHandler::dupRows_(bool *changed)
{
  UInt n, m, i, j;
  int err;
  DoubleVector r1, r2, h1, h2;
  ConstraintPtr c1, c2;
  bool is_deleted;
  RecipConsPtr cdata;

  n = p_->getNumVars();
  m = consd_.size(); 
  err = 0;
  
  r1.reserve(n);
  r2.reserve(n);
  h1.reserve(m);
  h2.reserve(m);

  // Random test points strictly positive to avoid /0
  for (i = 0; i < n; ++i) {
    r1.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
    r2.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
  }

  for (i = 0; i < m; ++i) {
    cdata = consd_[i];
    c1 = cdata->con;
    h1.push_back(c1->getActivity(&(r1[0]), &err));
    h2.push_back(c1->getActivity(&(r2[0]), &err));
  }

  for (i = 0; i < m; ++i) {
    if (h1[i] >= 1e29) continue;
    for (j = i + 1; j < m; ++j) {
      if (h1[j] >= 1e29) continue;
      if (fabs(h1[i] - h1[j]) < 1e-10 && fabs(h2[i] - h2[j]) < 1e-10) {
        c1 = consd_[i]->con;
        c2 = consd_[j]->con;
        is_deleted = treatDupRows_(c1, c2, 1.0, changed);
        if (is_deleted) h1[j] = h2[j] = 1e30;
      }
    }
  }
}

bool RecipHandler::treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double, bool *changed)
{
  RecipConsPtr d1, d2;
  double lb, ub;

  d1 = nullptr;
  d2 = nullptr;

  for (auto &cd : consd_) {
    if (cd->con == c1) d1 = cd;
    if (cd->con == c2) d2 = cd;
  }

  if (!d1 || !d2) return false;
  if (d1->iv != d2->iv || d1->ov != d2->ov || d1->sense != d2->sense) return false;

  lb = std::max(c1->getLb(), c2->getLb());
  ub = std::min(c1->getUb(), c2->getUb());

  p_->changeBound(c1, lb, ub);
  c2->setBFlag(true);
  p_->markDelete(c2);
  ++pStats_.conDel;

  *changed = true;
  return true;
}

bool RecipHandler::propRecipBnds_(RecipConsPtr cdata, bool *changed)
{
  VariablePtr x, z;
  double xlb, xub, zlb, zub;
  double new_zlb, new_zub, new_xlb, new_xub;

  x = cdata->iv;
  z = cdata->ov; 
  
  xlb = x->getLb();
  xub = x->getUb();
  zlb = z->getLb();
  zub = z->getUb();

  if (xlb == 0.0 && xub == 0.0) return true; 

  // Forward (x -> z) 
  if (xlb >= 0.0) { 
    double safe_xlb = std::max(xlb, 1e-9); // Prevent 1/0
    new_zlb = (xub < 1e15) ? 1.0 / xub : 0.0;
    new_zub = 1.0 / safe_xlb;
  } else if (xub <= 0.0) { 
    double safe_xub = std::min(xub, -1e-9);
    new_zlb = 1.0 / safe_xub;
    new_zub = (xlb > -1e15) ? 1.0 / xlb : 0.0;
  } else {
    new_zlb = -INFINITY;
    new_zub = INFINITY;
  }
  if (updatePBnds_(z, new_zlb, new_zub, changed) < 0) return true;

  //  Backward (z -> x) 
  if (zlb >= 0.0) {
    double safe_zlb = std::max(zlb, 1e-9);
    new_xlb = (zub < 1e15) ? 1.0 / zub : 0.0;
    new_xub = 1.0 / safe_zlb;
  } else if (zub <= 0.0) {
    double safe_zub = std::min(zub, -1e-9);
    new_xlb = 1.0 / safe_zub;
    new_xub = (zlb > -1e15) ? 1.0 / zlb : 0.0;
  } else {
    new_xlb = -INFINITY;
    new_xub = INFINITY;
  }
  if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) return true;

  return false;
}
int RecipHandler::updatePBnds_(VariablePtr p, double newlb, double newub, bool *changed)
{
  double oldlb, oldub, lb, ub, mid;
  
  oldlb = p->getLb();
  oldub = p->getUb();
  lb = oldlb;
  ub = oldub;
  
  if (std::abs(newlb - std::round(newlb)) < 1e-4) newlb = std::round(newlb);
  if (std::abs(newub - std::round(newub)) < 1e-4) newub = std::round(newub);
  
  if (p->getType() == Binary || p->getType() == ImplBin || p->getType() == Integer || p->getType() == ImplInt) {
    if (newlb - floor(newlb) < eTol_) newlb = floor(newlb);
    else newlb = ceil(newlb);
    if (ceil(newub) - newub < eTol_) newub = ceil(newub);
    else newub = floor(newub);
  }

  if (newlb > lb) { lb = newlb; *changed = true; ++pStats_.vBnd; }
  if (newub < ub) { ub = newub; *changed = true; ++pStats_.vBnd; }

  if (lb > ub + eTol_) return -1;
 
  if (fabs(ub - lb) <= vTol_) {
    mid = 0.5 * (lb + ub);
    lb = mid;
    ub = mid;
  }
 
  if (*changed) { p->setLb_(lb); p->setUb_(ub); }
  return 0;
}

int RecipHandler::updatePBnds_(VariablePtr v, double newlb, double newub, RelaxationPtr rel, bool mod_rel, bool *changed, ModVector &p_mods, ModVector &r_mods)
{
  VarBoundModPtr bmod;
  VarBoundMod2Ptr b2mod;
  VariablePtr rv;
  double lb, ub;
  bool tighten_lb, tighten_ub;

  lb = v->getLb();
  ub = v->getUb();
  tighten_lb = false;
  tighten_ub = false;

  if (std::abs(newlb - std::round(newlb)) < 1e-4) newlb = std::round(newlb);
  if (std::abs(newub - std::round(newub)) < 1e-4) newub = std::round(newub);

  if (v->getType() == Binary || v->getType() == ImplBin || v->getType() == Integer || v->getType() == ImplInt) {
    if (newlb - floor(newlb) < eTol_) newlb = floor(newlb);
    else newlb = ceil(newlb);
    if (ceil(newub) - newub < eTol_) newub = ceil(newub);
    else newub = floor(newub);
  }

  if (newlb > ub + eTol_ || newub < lb - eTol_) return -1;

  if (newlb > lb) { lb = newlb; tighten_lb = true; }
  if (newub < ub) { ub = newub; tighten_ub = true; }

  if (lb > ub + eTol_) return -1;
  if (!tighten_lb && !tighten_ub) return 0;

  *changed = true;
  ++pStats_.vBnd;

  if (tighten_lb && tighten_ub) {
    b2mod = (VarBoundMod2Ptr) new VarBoundMod2(v, lb, ub);
    b2mod->applyToProblem(p_);
    p_mods.push_back(b2mod);
    if (mod_rel) {
      rv = rel->getVariable(v->getIndex());
      b2mod = (VarBoundMod2Ptr) new VarBoundMod2(rv, lb, ub);
      b2mod->applyToProblem(rel);
      r_mods.push_back(b2mod);
    }
  } else if (tighten_lb) {
    bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, lb);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
    if (mod_rel) {
      rv = rel->getVariable(v->getIndex());
      bmod = (VarBoundModPtr) new VarBoundMod(rv, Lower, lb);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  } else if (tighten_ub) {
    bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, ub);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
    if (mod_rel) {
      rv = rel->getVariable(v->getIndex());
      bmod = (VarBoundModPtr) new VarBoundMod(rv, Upper, ub);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  }
  return 0;
}

bool RecipHandler::varBndsFromCons_(bool *changed)
{
  bool is_inf;
  std::vector<RecipConsPtr>::const_iterator it;
  
  is_inf = false;
  for (it = consd_.begin(); it != consd_.end(); ++it) {
    is_inf = propRecipBnds_(*it, changed);
    if (is_inf) return true;
  }
  return false;
}

SolveStatus RecipHandler::presolve(PreModQ *, bool *changed, Solution **)
{
  bool is_inf;
  SolveStatus status;
  
  is_inf = false;
  status = Started;
  ++pStats_.iters;
  *changed = false;

  if (!consd_.empty()) {
    is_inf = varBndsFromCons_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
  }

  dupRows_(changed);
  p_->delMarkedCons();
  if (status == Started) status = Finished;

  return status;
}

bool RecipHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &, ModVector &)
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
      << me_ << "Number of constraints deleted  = " << pStats_.conDel << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd << std::endl;

  out << me_ << "Statistics for separation by RecipHandler:" << std::endl
      << me_ << "Number of calls to separate    = " << sStats_.iters << std::endl
      << me_ << "Number of tangent cuts added   = " << sStats_.tangentcuts << std::endl;
}


