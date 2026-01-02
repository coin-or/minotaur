//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file LogHandler.cpp
 * \brief Implement the handler for functions of type
 * y=log(x).
 * \author Neeraj Kumar, IIT Bombay
 */


#include <cmath>
#include <iostream>
#include <iomanip>
#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrVarCand.h"
#include "Constraint.h"
#include "Environment.h"
#include "LogHandler.h"
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
//#define SPEW 1:

using namespace Minotaur;

const std::string LogHandler::me_ = "LogHandler: ";
LogHandler::LogHandler(EnvPtr env, ProblemPtr problem)
  : bStats_(),
    pStats_(),
    LBd_(-1e-6),
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
LogHandler::~LogHandler()
{
  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    delete *it;
  }
}

//-----------------------------------------------------------------------------
// Per-constraint helper methods
//-----------------------------------------------------------------------------

void LogHandler::initRelax_(LogCons &cd,
                              RelaxationPtr rel,
                              DoubleVector &tmpX,
                              DoubleVector &grad)
{
  // Get relaxation variables corresponding to original vars
  cd.riv = rel->getVariable(cd.iv->getIndex());
  cd.rov = rel->getVariable(cd.ov->getIndex());

  ModVector mods; // not used, but kept for uniform interface

  // Add secant
  if (cd.sense == 'E' || cd.sense == 'L') {
    addSecant_(cd, rel, tmpX, mods, /*init=*/true);
  }

  // Add linearizations
  if (cd.sense == 'E' || cd.sense == 'G') {
    addLin_(cd, rel, tmpX, grad, mods, /*init=*/true);
  }
}

void LogHandler::updateRelax_(LogCons &cd,
                                RelaxationPtr rel,
                                DoubleVector &tmpX,
                                DoubleVector &grad,
                                ModVector &mods)
{
  // Add secant
  if (cd.sense == 'E' || cd.sense == 'L') {
    addSecant_(cd, rel, tmpX, mods, /*init=*/false);
  }

  // Add linearizations
  if (cd.sense == 'E' || cd.sense == 'G') {
    addLin_(cd, rel, tmpX, grad, mods, /*init=*/false);
  }
}

double LogHandler::getViol_(const LogCons &cd,
                              const DoubleVector &x) const
{
  int error = 0;
  double fval = cd.con->getFunction()->eval(x, &error);

  // TODO: Put in a better (scaled) feasibility check here
  double absViol = 0.0;
  double relViol = 0.0;

  if (fval < cd.con->getLb() - eTol_) {
    absViol = cd.con->getLb() - fval;
  }
  if (fval > cd.con->getUb() + eTol_ && fval - cd.con->getUb() > absViol) {
    absViol = fval - cd.con->getUb();
  }

  relViol = absViol;
  if (std::fabs(fval) + absViol > 1.0) {
    relViol = absViol / (std::fabs(fval) + absViol);
  }

  return relViol;
}

void LogHandler::addLin_(LogCons &cd,
                           RelaxationPtr rel,
                           DoubleVector &tmpX,
                           DoubleVector &grad,
                           ModVector &mods,
                           bool init)
{
  int error = 0;
  ConstraintPtr cons;
  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();
  double fxlbval = 0.0, fxubval = 0.0, dfxlbval = 0.0, dfxubval = 0.0;
  double tmpxval = 0.0, fxval = 0.0, dfxval = 0.0;
  LinearFunctionPtr lf;
  FunctionPtr f;
  FunctionPtr fn = cd.con->getFunction();

  int npts = 3;
  double xvals[] = { xlb, xub, (xub - xlb) / 2.0 };

#if MDBUG
  log_->msgStream(LogDebug2) << me_
                             << "Adding linearizations.  rix id: "
                             << cd.riv->getId()
                             << " rix index: " << cd.riv->getIndex()
                             << " rov id: " << cd.rov->getId()
                             << " rov index: " << cd.rov->getIndex()
                             << " xlb: " << xlb
                             << " xub: " << xub << std::endl;
#endif

  for (int i = 0; i < npts; ++i) {
    // Zero out tmpX and grad each time
    for (UInt j = 0; j < tmpX.size(); ++j) {
      tmpX[j] = 0.0;
      grad[j] = 0.0;
    }

    if (i == 2) {
      // Third linearization point: where first two lines intersect
      if (dfxubval - dfxlbval > 0.0001 || dfxubval - dfxlbval < -0.0001) {
        tmpxval = (dfxubval * xub - dfxlbval * xlb + fxlbval - fxubval) /
                  (dfxubval - dfxlbval);
        if (tmpxval < xlb + (xub - xlb) * 0.05) {
          xvals[2] = xlb + (xub - xlb) * 0.05;
        } else if (tmpxval > xub - (xub - xlb) * 0.05) {
          xvals[2] = xub - (xub - xlb) * 0.05;
        } else {
          xvals[2] = tmpxval;
        }
      }
    }

    tmpX[cd.riv->getIndex()] = xvals[i];
    error = 0;
    fxval = fn->eval(tmpX, &error);
    fn->evalGradient(&tmpX[0], &grad[0], &error);

#if MDBUG
    for (UInt j = 0; j < tmpX.size(); ++j) {
      log_->msgStream(LogDebug2) << me_
                                 << "x[" << j << "] = " << tmpX[j]
                                 << " dfdx[" << j << "] = " << grad[j]
                                 << std::endl;
    }
#endif

    dfxval = grad[cd.riv->getIndex()];
    if (i == 0) {
      fxlbval = fxval;
      dfxlbval = dfxval;
    } else if (i == 1) {
      fxubval = fxval;
      dfxubval = dfxval;
    }

    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(cd.rov, 1.0);
    lf->addTerm(cd.riv, -dfxval);

    if (init) {
      f = (FunctionPtr) new Function(lf);
      cons = rel->newConstraint(f, fxval - dfxval * xvals[i], INFINITY);
      cd.linCons.push_back(cons);
    } else {
      rel->changeConstraint(cd.linCons[i], lf,
                            fxval - dfxval * xvals[i], INFINITY);
      LinConModPtr lcmod = (LinConModPtr) new LinConMod(
        cd.linCons[i], lf, fxval - dfxval * xvals[i], INFINITY);
      mods.push_back(lcmod);
    }
  }

  tmpX[cd.riv->getIndex()] = 0.0;
  grad[cd.riv->getIndex()] = 0.0;
}

void LogHandler::addSecant_(LogCons &cd,
                              RelaxationPtr rel,
                              DoubleVector &tmpX,
                              ModVector &mods,
                              bool init)
{
  int error = 0;
  double xlb, xub, fxlb, fxub, m, intercept;
  LinearFunctionPtr lf;
  FunctionPtr f;
  FunctionPtr fn = cd.con->getFunction();

  xlb = cd.riv->getLb();
  xub = cd.riv->getUb();

#if MDBUG
  log_->msgStream(LogDebug2) << me_
                             << "Adding secant on variable rix index: "
                             << cd.riv->getIndex()
                             << " rov index: " << cd.rov->getIndex()
                             << " xlb: " << xlb
                             << " xub: " << xub << std::endl;
#endif

  // No secant if unbounded either way
  if (xlb <= -0.9 * INFINITY || xub >= 0.9 * INFINITY) {
    log_->msgStream(LogDebug2) << me_
                               << "Cannot add secant -- bound is infinite"
                               << std::endl;
    return;
  }

  tmpX[cd.riv->getIndex()] = xlb;
  fxlb = fn->eval(tmpX, &error);
  tmpX[cd.riv->getIndex()] = xub;
  fxub = fn->eval(tmpX, &error);
  tmpX[cd.riv->getIndex()] = 0.0;

  if (xub - xlb > 10e-7) {
    m = (fxub - fxlb) / (xub - xlb);
  } else {
    m = 0.0;
  }

  intercept = fxlb - m * xlb;
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -m);

  if (init) {
    f = (FunctionPtr) new Function(lf);
    cd.secCon = rel->newConstraint(f, -INFINITY, intercept);
  } else {
    rel->changeConstraint(cd.secCon, lf, -INFINITY, intercept);
    LinConModPtr lcmod =
      (LinConModPtr) new LinConMod(cd.secCon, lf, -INFINITY, intercept);
    mods.push_back(lcmod);
  }
}

void LogHandler::relaxInitInc(RelaxationPtr rel,
                              SolutionPool *,
                              bool *is_inf)
{
  if (tmpX_.size() != p_->getNumVars()) {
    tmpX_.assign(p_->getNumVars(), 0.0);
  }
  if (grad_.size() != p_->getNumVars()) {
    grad_.assign(p_->getNumVars(), 0.0);
  }

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    initRelax_(**it, rel, tmpX_, grad_);
  }

  *is_inf = false;
}

void LogHandler::relaxNodeInc(NodePtr,
                              RelaxationPtr rel,
                              bool *is_inf)
{
  ModVector mods;

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    updateRelax_(**it, rel, tmpX_, grad_, mods);
  }

  for (ModificationConstIterator mit = mods.begin(); mit != mods.end(); ++mit) {
    assert(!"add Mod correctly here.");
    // node->addPMod(*mit);
  }

  *is_inf = false;
}



void LogHandler::addConstraint(ConstraintPtr newcon,
                               ConstVariablePtr ivar,
                               ConstVariablePtr ovar,
                               char sense)
{
  Handler::addConstraint(newcon);

  // Drop constness internally to allow bound changes in presolve
  VariablePtr vivar = (VariablePtr) ivar;
  VariablePtr vovar = (VariablePtr) ovar;

  consd_.push_back(new LogCons(newcon, vivar, vovar, sense));
}

bool LogHandler::isFeasible(ConstSolutionPtr sol,
                            RelaxationPtr,
                            bool &,
                            double &)
{
  bool isfeas = true;
  int error = 0;
  const double *x = sol->getPrimal();

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    LogCons *cd = *it;
    Constraint *c = cd->con;
    double fval = c->getFunction()->eval(x, &error);

    if ((fval < c->getLb() - eTol_ || fval > c->getUb() + eTol_) &&
        cd->riv->getUb() - cd->riv->getLb() > vTol_) {
      isfeas = false;
      break;
    }
  }
  return isfeas;
}

void LogHandler::separate(ConstSolutionPtr,
                          NodePtr,
                          RelaxationPtr,
                          CutManager *,
                          SolutionPoolPtr,
                          ModVector &,
                          ModVector &,
                          bool *,
                          SeparationStatus *)
{
  // Not implemented yet. Could add violated linearization cuts here.
}

void LogHandler::getBranchingCandidates(RelaxationPtr,
                                        const DoubleVector &x,
                                        ModVector &,
                                        BrVarCandSet &cands,
                                        BrCandVector &,
                                        bool &is_inf)
{
  is_inf = true;
  std::map<ConstVariablePtr, double> allCands;
  std::map<ConstVariablePtr, double>::iterator curc_it;

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    LogCons &cd = **it;
    double curviol = getViol_(cd, x);
    if (curviol > eTol_) {
      is_inf = false;
      ConstVariablePtr v = cd.riv;

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
      curc_it->first,
      curc_it->first->getIndex(),
      curc_it->second,
      curc_it->second);
    cands.insert(br_can);
  }
}

ModificationPtr LogHandler::getBrMod(BrCandPtr cand,
                                     DoubleVector &x,
                                     RelaxationPtr,
                                     BranchDirection brdir)
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();

  double minFromBds = 0.1;
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();

  double xval = x[v->getIndex()];
  double value = xval;
  double len = v->getUb() - v->getLb();
  if (value < v->getLb() + minFromBds * len) {
    value = v->getLb() + minFromBds * len;
  } else if (value > v->getUb() - minFromBds * len) {
    value = v->getUb() - minFromBds * len;
  }

  if (!(value > v->getLb() + 1e-8 && value < v->getUb() - 1e-8)) {
    std::cerr << "Warning!  Branching on variable with bounds/value: ["
              << v->getLb() << " , " << value << "  " << v->getUb() << " ]"
              << std::endl;
  }

  if (brdir == DownBranch) {
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    lmods->insert(mod);
  } else if (brdir == UpBranch) {
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    lmods->insert(mod);
  }

  return lmods;
}

Branches LogHandler::getBranches(BrCandPtr cand,
                                 DoubleVector &x,
                                 RelaxationPtr,
                                 SolutionPoolPtr)
{
  double minFromBds = 0.1;
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();

  double xval = x[v->getIndex()];
  double value = xval;
  double len = v->getUb() - v->getLb();
  if (value < v->getLb() + minFromBds * len) {
    value = v->getLb() + minFromBds * len;
  } else if (value > v->getUb() - minFromBds * len) {
    value = v->getUb() - minFromBds * len;
  }

  if (!(value > v->getLb() + 1e-8 && value < v->getUb() - 1e-8)) {
    std::cerr << "Warning!  Branching on variable with bounds/value: ["
              << v->getLb() << " , " << value << "  " << v->getUb() << " ]"
              << std::endl;
  }

  Branches branches = (Branches) new BranchPtrVector();

  BranchPtr branch = (BranchPtr) new Branch();
  VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
  assert(!"add Mod correctly here.");
  branch->addPMod(mod);
  branch->setActivity((v->getUb() - value) / len);
  branches->push_back(branch);

  branch = (BranchPtr) new Branch();
  mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
  assert(!"add Mod correctly here.");
  branch->addPMod(mod);
  branch->setActivity((value - v->getLb()) / len);
  branches->push_back(branch);

  log_->msgStream(LogDebug2) << me_ << "branching on " << v->getName();
  log_->msgStream(LogDebug2) << " <= " << value << " or "
                             << " >= " << value << std::endl;

  return branches;
}

BranchPtr LogHandler::doBranch_(BranchDirection UpOrDown,
                                ConstVariablePtr v,
                                double bvalue)
{
  BranchPtr branch;
  LinModsPtr linmods;

#if MDBUG
  log_->msgStream(LogDebug2)
    << me_ << "LogHandler, Branching: "
    << (UpOrDown == DownBranch ? "Down" : "Up")
    << " at value: " << bvalue << " on: " << std::endl;
  v->write(log_->msgStream(LogDebug2));
#endif

  for (UInt j = 0; j < tmpX_.size(); ++j) {
    tmpX_[j] = 0.0;
    grad_[j] = 0.0;
  }

  branch  = (BranchPtr) new Branch();
  linmods = (LinModsPtr) new LinMods();

  if (UpOrDown == DownBranch) {
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(
      VariablePtr(const_cast<Variable *>(v)), Upper, bvalue);
    linmods->insert(mod);

    // TODO: Update secants/linearizations if needed.
  } else {
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(
      VariablePtr(const_cast<Variable *>(v)), Lower, bvalue);
    linmods->insert(mod);
  }

  assert(!"add Mod correctly here.");
  branch->addPMod(linmods);
  return branch;
}

//-----------------------------------------------------------------------------
// Duplicate row detection and correction
//-----------------------------------------------------------------------------

void LogHandler::dupRows_(bool *changed)
{
  const UInt n = p_->getNumVars();
  const UInt m = consd_.size();  // only log constraints
  UInt i, j;
  int err = 0;
  DoubleVector r1, r2, h1, h2;
  ConstraintPtr c1, c2;
  bool is_deleted;

  r1.reserve(n);
  r2.reserve(n);
  h1.reserve(m);
  h2.reserve(m);

  // Random positive test points for log(x)
  for (i = 0; i < n; ++i) {
    r1.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
    r2.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
  }

  // Evaluate all log constraints
  for (i = 0; i < m; ++i) {
    LogConsPtr cdata = consd_[i];
    c1 = cdata->con;
    h1.push_back(c1->getActivity(&(r1[0]), &err));
    h2.push_back(c1->getActivity(&(r2[0]), &err));
  }

  // Compare pairwise for duplicates
  for (i = 0; i < m; ++i) {
    if (h1[i] >= 1e29) {
      continue;
    }

    for (j = i + 1; j < m; ++j) {
      if (h1[j] >= 1e29) {
        continue;
      }

      if (fabs(h1[i] - h1[j]) < 1e-10 && fabs(h2[i] - h2[j]) < 1e-10) {
        c1 = consd_[i]->con;
        c2 = consd_[j]->con;
        is_deleted = treatDupRows_(c1, c2, 1.0, changed);
        if (is_deleted) {
          h1[j] = h2[j] = 1e30;
        }
      }
    }
  }
}

bool LogHandler::treatDupRows_(ConstraintPtr c1,
                               ConstraintPtr c2,
                               double,
                               bool *changed)
{
  LogConsPtr d1 = nullptr;
  LogConsPtr d2 = nullptr;

  for (auto &cd : consd_) {
    if (cd->con == c1) {
      d1 = cd;
    }
    if (cd->con == c2) {
      d2 = cd;
    }
  }

  if (!d1 || !d2) {
    return false;
  }

  if (d1->iv != d2->iv || d1->ov != d2->ov || d1->sense != d2->sense) {
    return false;
  }

  double lb = std::max(c1->getLb(), c2->getLb());
  double ub = std::min(c1->getUb(), c2->getUb());

  p_->changeBound(c1, lb, ub);
  c2->setBFlag(true);
  p_->markDelete(c2);
  ++pStats_.conDel;

  *changed = true;
  return true;
}

//-----------------------------------------------------------------------------
// Presolve 
//-----------------------------------------------------------------------------

bool LogHandler::propLogBnds_(LogConsPtr cdata, bool *changed)
{
  VariablePtr x = cdata->iv;  // input variable
  VariablePtr y = cdata->ov;  // output variable (y = log(x))

  double xlb = x->getLb();
  double xub = x->getUb();
  double ylb = y->getLb();
  double yub = y->getUb();

  double newlb, newub;

  // --- Forward propagation: x -> y ---
  if (xub < 0.0) {
    return true; // infeasible
  }

  if (xlb < 0.0) {
    xlb = 0.0;  // negative is not allowed in log
  }

  if (xlb > 0.0) {
    newlb = std::log(xlb);
    newub = std::log(xub);
    if (updatePBnds_(y, newlb, newub, changed) < 0) {
      return true;
    }
  }

  if (xlb==0 && xub>0){
    newub = std::log(xub);
    newlb = -1e-6;      // only to update upper bound of y
    if (updatePBnds_(y, newlb,  newub, changed)<0){
      return true;
    }
  }

  // --- Backward propagation: y -> x ---
  if (yub < INFINITY && ylb > -INFINITY) {
    double new_xlb = std::exp(ylb);
    double new_xub = std::exp(yub);
    if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) {
      return true;
    }
  }

  return false;
}


int LogHandler::updatePBnds_(VariablePtr p,
                             double newlb,
                             double newub,
                             bool *changed)
{
  double oldlb = p->getLb();
  double oldub = p->getUb();
  double lb = oldlb;
  double ub = oldub;


  if (p->getType() == Binary || p->getType() == ImplBin ||
      p->getType() == Integer || p->getType() == ImplInt) {
    // ---- Lower bound: ceil only if newlb is significantly greater,
    // else floor ----
    if (newlb - floor(newlb)  < eTol_) {
      newlb = floor(newlb);
    } else {
      newlb = ceil(newlb);
    }

    // ---- Upper bound: floor only if newub is siginficantly smaller
    // else ceil ----

    if (ceil(newub) - newub < eTol_) {
      newub = ceil(newub);
    } else {
      newub = floor(newub);
    }
  }

  // Tighten lower bound if possible
  if (newlb > lb + eTol_) {
    lb = newlb;
    *changed = true;
    ++pStats_.vBnd;
  }

  // Tighten upper bound if possible
  if (newub < ub - eTol_) {
    ub = newub;
    *changed = true;
    ++pStats_.vBnd;
  }


 // Infeasibility check
  if (lb > ub + eTol_ ) {
    log_->msgStream(LogDebug) << me_
      << "Infeasible bounds for variable " << p->getName()
      << ": [" << lb << ", " << ub << "]" << std::endl;
    return -1;
  }
 
   //Treat nearly-equal boundS
  if (fabs(ub - lb) <= vTol_) {
    double mid = 0.5 * (lb + ub);
    lb = mid;
    ub = mid;
  }
 
   // Apply changes
  if (*changed) {
    p->setLb_(lb);
    p->setUb_(ub);
  }

  return 0;
}

bool LogHandler::varBndsFromCons_(bool *changed)
{
  bool is_inf = false;

  for (std::vector<LogConsPtr>::const_iterator it = consd_.begin();
       it != consd_.end(); ++it) {
    is_inf = propLogBnds_(*it, changed);
    if (is_inf) {
      return true;
    }
  }

  return false;
}

SolveStatus LogHandler::presolve(PreModQ *,
                                 bool *changed,
                                 Solution **)
{
  bool is_inf = false;
  SolveStatus status = Started;
  ++pStats_.iters;
  *changed = false;

  if (!consd_.empty()) {
    is_inf = varBndsFromCons_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
  }

  // Remove duplicates
  dupRows_(changed);
  p_->delMarkedCons();
   if (status == Started) {
    status = Finished;
  }

  return status;
}

bool LogHandler::presolveNode(RelaxationPtr,
                              NodePtr,
                              SolutionPoolPtr,
                              ModVector &,
                              ModVector &)
{
  return false;
}

//-----------------------------------------------------------------------------
// Misc
//-----------------------------------------------------------------------------

std::string LogHandler::getName() const
{
  return "LogHandler (Handling log terms).";
}

void LogHandler::writeStats(std::ostream &out) const
{
  out << me_ << "Statistics for presolve by LogHandler:" << std::endl
      << me_ << "Number of iterations           = " << pStats_.iters
      << std::endl
      << me_ << "Number of log constraints      = " << consd_.size()
      << std::endl
      << me_ << "Number of constraints deleted  = " << pStats_.conDel
      << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd<<std::endl;
}

