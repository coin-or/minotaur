//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file LogHandler.cpp
 * \brief Implement the handler for functions of the general univariate convex
 * form y=log(x).
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
const std::string LogConstraintData::me_ = "LogHandler: ";

LogConstraintData::LogConstraintData(double eTol, double vTol,
                                     ConstraintPtr newcon,
                                     ConstVariablePtr ivar,
                                     ConstVariablePtr ovar, char sense)
  : eTol_(eTol),
    vTol_(vTol),
    con_(newcon),
    iv_(ivar),
    riv_(NULL),
    ov_(ovar),
    rov_(NULL),
    sense_(sense),
    secCon_(NULL),
    linCons_(ConstraintVector())
{
}


/// Creates initial relaxations
void LogConstraintData::initRelax(RelaxationPtr rel, DoubleVector &tmpX,
                                  DoubleVector &grad)
{
  // get and set the relaxation relaxation variables that match the original
  // problem variables
  riv_ = rel->getVariable(iv_->getIndex());
  rov_ = rel->getVariable(ov_->getIndex());

  // This isn't used
  ModVector mods;

  // Add secant
  if (sense_ == 'E' || sense_ == 'L') {
    addSecant(rel, riv_, rov_, con_->getFunction(), tmpX, true, mods);
  }

  // Add linearizations
  // TODO: Make strategy a parameter
  if (sense_ == 'E' || sense_ == 'G') {
    addLin(rel, iv_, ov_, con_->getFunction(), tmpX, grad, true, mods);
  }
}

/// Update the current relaxation based on current variable bounds
void LogConstraintData::updateRelax(RelaxationPtr rel, DoubleVector &tmpX,
                                    DoubleVector &grad, ModVector &mods)
{
  // Add secant
  if (sense_ == 'E' || sense_ == 'L') {
    addSecant(rel, riv_, rov_, con_->getFunction(), tmpX, false, mods);
  }

  // Add linearizations
  // TODO: Make strategy a parameter
  if (sense_ == 'E' || sense_ == 'G') {
    addLin(rel, riv_, rov_, con_->getFunction(), tmpX, grad, false, mods);
  }
}

double LogConstraintData::getViol(const DoubleVector &x)
{
  int error;
  double fval = con_->getFunction()->eval(x, &error);

  // TODO: Put in a better (scaled) feasibility check here
  double absViol = 0.0;
  double relViol = 0.0;
  if (fval < con_->getLb() - eTol_) {
    absViol = con_->getLb() - fval;
  }
  if (fval > con_->getUb() + eTol_ && fval - con_->getUb() > absViol) {
    absViol = fval - con_->getUb();
  }
  relViol = absViol;
  if (fabs(fval) + absViol > 1.0) {
    relViol = absViol / (fabs(fval) + absViol);
  }

  return relViol;
}

LogHandler::LogHandler(EnvPtr env, ProblemPtr problem)
  : eTol_(1e-6),
    vTol_(1e-5)
{
  p_ = problem;
  log_ = env->getLogger();

  tmpX_.resize(problem->getNumVars(), 0.0);
  grad_.resize(problem->getNumVars(), 0.0);
}


LogHandler::~LogHandler()
{
}

void LogHandler::relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *)
{

  if (tmpX_.size() != p_->getNumVars()) {
    tmpX_.resize(p_->getNumVars(), 0.0);
  }
  if (grad_.size() != p_->getNumVars()) {
    grad_.resize(p_->getNumVars(), 0.0);
  }

  for (LogConsIter dit = consd_.begin(); dit != consd_.end(); ++dit) {
    (*dit)->initRelax(rel, tmpX_, grad_);
  }
}

void LogHandler::relaxNodeInc(NodePtr, RelaxationPtr rel, bool *is_inf)
{

#if 0
  std::cout << "LogHandler::relaxNodeInc.  Current relaxation: "
            << std::endl;
  rel->write(std::cout);
  std::cout << "Current node: " << std::endl;
  node->write(std::cout);
#endif

  // You must apply all modifications to the nodes

  // I think this is all done by branches now.
  //  FALSE -- If you don't branch on a variable, you still need to update the
  //  relaxation from this node

  ModVector mods;
  LogConsIter dit;
  for (dit = consd_.begin(); dit != consd_.end(); ++dit) {
    (*dit)->updateRelax(rel, tmpX_, grad_, mods);
  }

  ModificationConstIterator it;
  for (it = mods.begin(); it != mods.end(); ++it) {
    assert(!"add Mod correctly here.");
    // node->addPMod(*it);
  }

  *is_inf = false;
}

void LogConstraintData::addLin(RelaxationPtr rel, ConstVariablePtr riv,
                               ConstVariablePtr rov, FunctionPtr fn,
                               DoubleVector &tmpX, DoubleVector &grad,
                               bool init, ModVector &mods)
{

  int error;
  ConstraintPtr cons;
  double xlb = riv->getLb();
  double xub = riv->getUb();
  double fxlbval = 0, fxubval = 0, dfxlbval = 0, dfxubval = 0;
  double tmpxval, fxval, dfxval;
  LinearFunctionPtr lf;
  FunctionPtr f;

  // More sophisticated strategies hopefully could be obtained by simply
  // changing this array
  int npts = 3;
  double xvals[] = {xlb, xub, (xub - xlb) / 2.0};

#if MDBUG
  log_->msgStream(LogDebug2) << me_ 
	    << "Adding linearizations.  rix id: " << riv->getId()
            << " rix index: " << riv->getIndex()
            << " rov id: " << rov->getId()
            << " rov index: " << rov->getIndex() << " xlb: " << xlb
            << " xub: " << xub << std::endl;
#endif

  for (int i = 0; i < npts; i++) {

    // Zero out tmpX and grad each time, or else bad things happen
    for (UInt j = 0; j < tmpX.size(); ++j) {
      tmpX[j] = 0.0;
      grad[j] = 0.0;
    }

    if (i == 2) {
      // Third linearization point taken to be where first two intersect:
      // x3 = (f'(xub)*xub - f'(xlb)*xlb + f(xlb) - f(xub))/(f'(xub) -
      // f'(xlb)) Unless this would put it too close to one of the end points
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
    tmpX[riv->getIndex()] = xvals[i];
    error = 0;
    fxval = fn->eval(tmpX, &error);
    fn->evalGradient(&tmpX[0], &grad[0], &error);
#if MDBUG
    for (UInt j = 0; j < tmpX.size(); ++j) {
      log_->msgStream(LogDebug2) << me_
	        << "x[" << j << "] = " << tmpX[j] << " dfdx[" << j
                << "] = " << grad[j] << std::endl;
    }
#endif
    dfxval = grad[riv->getIndex()];
    if (i == 0) {
      fxlbval = fxval;
      dfxlbval = dfxval;
    } else if (i == 1) {
      fxubval = fxval;
      dfxubval = dfxval;
    }
    // linearization:  rov >= f(xval) + f'(xval)(riv - xval)
    //                 rov - f'(xval)*riv >= f(xval) - f'(xval)*xval
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(rov, 1.0);
    lf->addTerm(riv, -dfxval);
    if (init) {
      f = (FunctionPtr) new Function(lf);
      cons = rel->newConstraint(f, fxval - dfxval * xvals[i], INFINITY);
      linCons_.push_back(cons);
    } else {
#if MDBUG
      log_->msgStream(LogDebug2) << me_ 
	        << "Will change 'linearization  ' constraint to have "
                << "linear function: ";
      lf->write(log_->msgStream(LogDebug2));
      log_->msgStream(LogDebug2) << std::endl;
#endif

      rel->changeConstraint(linCons_[i], lf, fxval - dfxval * xvals[i],
                            INFINITY);
      LinConModPtr lcmod = (LinConModPtr) new LinConMod(
          linCons_[i], lf, fxval - dfxval * xvals[i], INFINITY);
      mods.push_back(lcmod);
    }
  }
  tmpX[riv->getIndex()] = 0.0;
  grad[riv->getIndex()] = 0.0;
}


void LogConstraintData::addSecant(RelaxationPtr rel, ConstVariablePtr riv,
                                  ConstVariablePtr rov, FunctionPtr fn,
                                  DoubleVector &tmpX, bool init,
                                  ModVector &mods)
{

  int error;
  double xlb, xub, fxlb, fxub, m, intercept;
  LinearFunctionPtr lf;
  FunctionPtr f;

  // First add the secant inequalities based on variable bounds
  xlb = riv->getLb();
  xub = riv->getUb();

#if MDBUG
  log_->msgStream(LogDebug2) << me_
	    << "Adding secant on variable rix index: " << riv->getIndex()
            << " rov index: " << rov->getIndex() << " xlb: " << xlb
            << " xub: " << xub << std::endl;
#endif
  // no secant if unbounded either way
  if (xlb <= -0.9 * INFINITY || xub >= 0.9 * INFINITY) {
    log_->msgStream(LogDebug2) << me_
	    << "Cannot add secant -- bound is infinite" << std::endl;
    return;
  }

  // TODO: Check the error value!
  tmpX[riv->getIndex()] = xlb;
  fxlb = fn->eval(tmpX, &error);
  tmpX[riv->getIndex()] = xub;
  fxub = fn->eval(tmpX, &error);
  tmpX[riv->getIndex()] = 0.0;

  // TODO: check/remedy numerical issues in this division
  if (xub - xlb > 10e-7) {
    m = (fxub - fxlb) / (xub - xlb);
  } else {
    m = 0.0;
  }

  intercept = fxlb - m * xlb;
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(rov, 1.0);
  lf->addTerm(riv, -m);

  // rovar <= m*rivar + intercept
  if (init) {
    f = (FunctionPtr) new Function(lf);
    secCon_ = rel->newConstraint(f, -INFINITY, intercept);
  } else {
    rel->changeConstraint(secCon_, lf, -INFINITY, intercept);
    LinConModPtr lcmod =
        (LinConModPtr) new LinConMod(secCon_, lf, -INFINITY, intercept);
    mods.push_back(lcmod);
  }
}


void LogHandler::addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                               ConstVariablePtr ovar, char sense)
{

  Handler::addConstraint(newcon);
  consd_.push_back(
      new LogConstraintData(eTol_, vTol_, newcon, ivar, ovar, sense));
}


bool LogHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                            double &)
{
  bool isfeas = true;
  LogConsIter dit;
  int error = 0;
  double fval;
  Constraint* c;
  const double* x = sol->getPrimal();

  for (dit = consd_.begin(); dit != consd_.end(); ++dit) {
    c = (*dit)->con_;
    fval = c->getFunction()->eval(x, &error);
    // TODO: Put in a better (scaled) feasibility check here
    if ((fval < c->getLb() - eTol_ || fval > c->getUb() + eTol_) &&
         (*dit)->riv_->getUb() - (*dit)->riv_->getLb() > vTol_) {
      isfeas = false;
      break;
    }
  }
  return isfeas;
}

// Eventually, could add additional linearization inequalities for the convex
// side here... but not absolutely necessary
void LogHandler::separate(ConstSolutionPtr, NodePtr, RelaxationPtr,
                          CutManager *, SolutionPoolPtr, ModVector &,
                          ModVector &, bool *, SeparationStatus *)
{
}


void LogHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector &x,
                                        ModVector &, BrVarCandSet &cands,
                                        BrCandVector &, bool &is_inf)
{

  is_inf = true;
  LogConsIter dit;
  // Create a map of variables to their weights
  // Weights will be sum of scaled violation of constraints they are argument
  // for
  std::map<ConstVariablePtr, double> allCands;
  std::map<ConstVariablePtr, double>::iterator curc_it;
  double curviol = 0.0;

  for (dit = consd_.begin(); dit != consd_.end(); ++dit) {
    curviol = (*dit)->getViol(x);
    if (curviol > eTol_) {
      is_inf = false;
      curc_it = allCands.find((*dit)->getRInputVar());
      if (curc_it == allCands.end()) {
        allCands[(*dit)->getRInputVar()] = curviol;
      } else {
        (*curc_it).second = (*curc_it).second + curviol;
      }
    }
  }

  // TODO: For now putting in all candidates, eventually probably want to
  // choose a reasonable subset
  for (curc_it = allCands.begin(); curc_it != allCands.end(); ++curc_it) {
    BrVarCandPtr br_can = (BrVarCandPtr) new BrVarCand(
        (*curc_it).first, (*curc_it).first->getIndex(), (*curc_it).second,
        (*curc_it).second);
    cands.insert(br_can);
  }
}


// Implement Handler::getBrMod().
ModificationPtr LogHandler::getBrMod(BrCandPtr cand, DoubleVector &x,
                                     RelaxationPtr, BranchDirection brdir)
{

  // This method is used in Reliability branching.
  //XXX If you want it to be more accurate, you should add the new secant and
  //linearizations
  //   into lmods.

  LinModsPtr lmods;
  lmods = (LinModsPtr) new LinMods();

  double minFromBds = 0.1;
  //BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();

#if 0
  std::cout << "Branching mod candidate (working problem) ID: " << v->getId() 
            << " address: " << (  v.get() ) << std::endl;
#endif

  // x is a *relaxation* solution, while we have put the *original* (or
  // working) problem variables into the BrCandPtr, so we need to
  // update our value appropriately...

  double xval = x[v->getIndex()];
  double value =
      xval;  // Make sure branch value is not too close to an end point
  double len = v->getUb() - v->getLb();
  if (value < v->getLb() + minFromBds * len) {
    value = v->getLb() + minFromBds * len;
  } else if (value > v->getUb() - minFromBds * len) {
    value = v->getUb() - minFromBds * len;
  }

  // can't branch on something that is at its bounds.
  if (!(value > v->getLb() + 1e-8 && value < v->getUb() - 1e-8)) {
    std::cerr << "Warning!  Branching on variable with bounds/value: ["
              << v->getLb() << " , " << value << "  " << v->getUb() << " ]"
              << std::endl;
    //assert(value > v->getLb()+1e-8 && value < v->getUb()-1e-8);
  }

  if (brdir == DownBranch) {
    // down branch
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    lmods->insert(mod);
  } else if (brdir == UpBranch) {
    // up branch
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    lmods->insert(mod);
  }

  return lmods;
}


// Implement Handler::getBranches().
Branches LogHandler::getBranches(BrCandPtr cand, DoubleVector &x,
                                 RelaxationPtr, SolutionPoolPtr)
{
  double minFromBds = 0.1;
  //BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();

  double xval = x[v->getIndex()];
  double value =
      xval;  // Make sure branch value is not too close to an end point
  double len = v->getUb() - v->getLb();
  if (value < v->getLb() + minFromBds * len) {
    value = v->getLb() + minFromBds * len;
  } else if (value > v->getUb() - minFromBds * len) {
    value = v->getUb() - minFromBds * len;
  }

  // can't branch on something that is at its bounds.
  if (!(value > v->getLb() + 1e-8 && value < v->getUb() - 1e-8)) {
    std::cerr << "Warning!  Branching on variable with bounds/value: ["
              << v->getLb() << " , " << value << "  " << v->getUb() << " ]"
              << std::endl;
    //assert(value > v->getLb()+1e-8 && value < v->getUb()-1e-8);
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

#if MDBUG

  log_->msgStream(LogDebug2) << me_ << "branching on" << v->getName();
  log_->msgStream(LogDebug2)
      << " <= " << value << " or " << " >= " << value << std::endl;
#endif

  return branches;
}


BranchPtr LogHandler::doBranch_(BranchDirection UpOrDown, ConstVariablePtr v,
                                double bvalue)
{
  BranchPtr branch;
  LinModsPtr linmods;

#if MDBUG
  log_->msgStream(LogDebug2)
      << "LogHandler, Branching: " << (UpOrDown == DownBranch ? "Down" : "Up")
      << " at value: " << bvalue << " on: " << std::endl;
  v->write(log_->msgStream(LogDebug2));
#endif

  // Zero out tmpX and grad each time, or else bad things happen
  for (UInt j = 0; j < tmpX_.size(); ++j) {
    tmpX_[j] = 0.0;
    grad_[j] = 0.0;
  }

  branch = (BranchPtr) new Branch();
  linmods = (LinModsPtr) new LinMods();

  // Change bounds on the x var (called v here)
  if (UpOrDown == DownBranch) {

    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Upper, bvalue);
    linmods->insert(mod);

    // Find *all* cons_data that has v as an input variable.
    LogConsIter dit;

    for (dit = consd_.begin(); dit != consd_.end(); ++dit) {
      if ((*dit)->getRInputVar() == v) {

        ConstVariablePtr rov = (*dit)->getROutVar();
        FunctionPtr fn = (*dit)->getOriginalCon()->getFunction();
        int error;

        // Change the secant constraint
        ConstraintPtr secCon = (*dit)->getSecantCon();

        LinearFunctionPtr lf;

        double xlb = v->getLb();
        double xub = bvalue;

        // TODO: Check the error value!
        tmpX_[v->getIndex()] = xlb;
        double fxlb = fn->eval(tmpX_, &error);
        tmpX_[v->getIndex()] = xub;
        double fxub = fn->eval(tmpX_, &error);
        tmpX_[v->getIndex()] = 0.0;

        // TODO: check/remedy numerical issues in this division
        double m = 0.0;
        if (xub - xlb > 10e-7) {
          m = (fxub - fxlb) / (xub - xlb);
        }
        double intercept = fxlb - m * xlb;
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(rov, 1.0);
        lf->addTerm(v, -m);

        LinConModPtr lcmod =
            (LinConModPtr) new LinConMod(secCon, lf, -INFINITY, intercept);
        linmods->insert(lcmod);

        // Change all linearization constraints
        ConstraintVector::iterator lin_it;
        //for(lin_it = (*dit)->linConsBegin(); lin_it != (*dit)->linConsEnd();
        //    ++lin_it) {
        //  ConstraintPtr c = *lin_it;
        //}
      }
    }
  } else {
    VarBoundModPtr mod = (VarBoundModPtr) new VarBoundMod(v, Lower, bvalue);
    linmods->insert(mod);
  }

  assert(!"add Mod correctly here.");
  branch->addPMod(linmods);
  return branch;
}

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
    r1.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);  // ensure x > 0
    r2.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
  }

  // Evaluate all log constraints
  for (i = 0; i < m; ++i) {
    LogConsPtr cdata = consd_[i];
    c1 = cdata->con_;
    h1.push_back(c1->getActivity(&(r1[0]), &err));
    h2.push_back(c1->getActivity(&(r2[0]), &err));
  }

  // Compare pairwise for duplicates
  for (i = 0; i < m; ++i) {
    if (h1[i] >= 1e29)
      continue;

    for (j = i + 1; j < m; ++j) {
      if (h1[j] >= 1e29)
        continue;

      if (fabs(h1[i] - h1[j]) < 1e-10 && fabs(h2[i] - h2[j]) < 1e-10) {
        c1 = consd_[i]->con_;
        c2 = consd_[j]->con_;
        is_deleted = treatDupRows_(c1, c2, 1.0, changed);
        if (is_deleted) {
          h1[j] = h2[j] = 1e30;
        }
      }
    }
  }
}


// Write name
std::string LogHandler::getName() const
{
  return "LogHandler (Handling log terms).";
}


// presolve.
SolveStatus LogHandler::presolve(PreModQ *, bool *changed, Solution **)
{
  bool is_inf = false;
  SolveStatus status = Started;
  ++pStats_.iters;
  *changed = false;
  // // Start timer
  //   double t_start = env_->getTime();
  // log constraints, do a cheap first pass.
  if (!consd_.empty()) {
    is_inf = varBndsFromCons_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
  }

  // Remove duplicates (y = log(x) appearing multiple times)
  dupRows_(changed);
  p_->delMarkedCons();

  // Stronger tightening: multiple passes of log bound propagation
  is_inf = tightenSimple_(changed);
  if (is_inf) {
    status = SolvedInfeasible;
    return status;
  }
  // tightenlog function not implemented similar to loghandler
  //p_->delMarkedCons();

  if (status == Started)
    status = Finished;


  return status;
}

// Implement Handler::presolveNode().
bool LogHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr,
                              ModVector &, ModVector &)
{
  return false;
}
//1st presolve bound propagation
bool LogHandler::propLogBnds_(LogConsPtr cdata, bool *changed)
{
  VariablePtr x = cdata->iv_;  // input variable
  VariablePtr y = cdata->ov_;  // output variable (y = log(x))
  double xlb = x->getLb();
  double xub = x->getUb();
  double ylb = y->getLb();
  double yub = y->getUb();

  // --- Forward propagation: x → y ---
  if (xub < 0)
    return true;
  if (xlb < 0)
    xlb = 0;  //negative is not allowed in log
  if (xlb > 0.0) {
    double new_ylb = std::log(xlb);
    double new_yub = std::log(xub);
    if (updatePBnds_(y, new_ylb, new_yub, changed) < 0) {
      return true;
    }
  }

  // --- Backward propagation: y → x ---
  if (yub < INFINITY && ylb > -INFINITY) {
    double new_xlb = std::exp(ylb);
    double new_xub = std::exp(yub);
    if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) {
      return true;
    }
  }

  return false;
}

bool LogHandler::treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double,
                               bool *changed)
{
  LogConsPtr d1 = nullptr;
  LogConsPtr d2 = nullptr;

  for (auto &cd : consd_) {
    if (cd->con_ == c1)
      d1 = cd;
    if (cd->con_ == c2)
      d2 = cd;
  }

  if (!d1 || !d2)
    return false;

  if (d1->iv_ != d2->iv_ || d1->ov_ != d2->ov_ || d1->sense_ != d2->sense_) {
    return false;
  }

  double lb = std::max(c1->getLb(), c2->getLb());
  double ub = std::min(c1->getUb(), c2->getUb());

  p_->changeBound(c1, lb, ub);
  c2->setBFlag(true);
  p_->markDelete(c2);
  ++pStats_.conDel;  //stats to update deletion

  *changed = true;

  return true;
}

bool LogHandler::tightenSimple_(bool *changed)
{
  
  bool is_inf = false;
  bool local_changed = false;

  if (changed) {
    *changed = false;
  }

  if (consd_.empty()) {
    return false;
  }

  // Do a few passes until no more tightening
  const UInt max_passes = 5;  // like 5
  for (UInt pass = 0; pass < max_passes; ++pass) {
    local_changed = false;

    for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
      LogConsPtr cdata = *it;

      // propagate bounds across y = log(x) (or inequality variant)
      is_inf = propLogBnds_(cdata, &local_changed);
      if (is_inf) {
        // Some constraint concluded infeasibility
        return true;
      }
    }

    if (!local_changed) {
      // No further tightening this pass
      break;
    }

    if (changed) {
      *changed = true;
    }
  }

  return false;
}
int LogHandler::updatePBnds_(VariablePtr p, double newlb, double newub,
                             bool *changed)
{
  double oldlb = p->getLb();
  double oldub = p->getUb();
  double lb = oldlb;
  double ub = oldub;

  // Tighten lower bound if possible
  if (newlb > lb + eTol_) {
    lb = newlb;
    *changed = true;
    if (*changed) {
      ++pStats_.vBnd;
    }
  }

  // Tighten upper bound if possible
  if (newub < ub - eTol_) {
    ub = newub;
    *changed = true;
    if (*changed) {
      ++pStats_.vBnd;
    }
  }

  // Infeasibility check
  if (lb > ub + eTol_) {
    log_->msgStream(LogDebug) << me_
	      << "Infeasible bounds for variable " << p->getName() << ": ["
              << lb << ", " << ub << "]" << std::endl;
    return -1;
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

  // Loop over all log constraints stored in your handler
  for (std::vector<LogConsPtr>::const_iterator it = consd_.begin();
       it != consd_.end(); ++it) {
    is_inf = propLogBnds_(*it, changed);
    if (is_inf) {
      return true;  // infeasibility detected
    }
  }

  return false;  // everything consistent
}


void LogHandler::writeStats(std::ostream &out) const
{
  // Presolve statistics for y = log(x) constraints
  out << me_ << "Statistics for presolve by LogHandler:" << std::endl
      << me_ << "Number of iterations           = " << pStats_.iters
      << std::endl
      << me_ << "Number of log constraints      = " << consd_.size()
      << std::endl
      //<< me_ << "Time taken in presolve         = " << pStats_.time
      << std::endl
      << me_ << "Number of constraints deleted  = " << pStats_.conDel
      << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd;
}
