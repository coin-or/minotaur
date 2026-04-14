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
    sStats_(),
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
 

  double fval = log(x[cd.riv->getIndex()]);
  double fhat=x[cd.rov->getIndex()];
  double absViol = std::abs(fhat-fval); 
  double relViol = 0.0;

  
  relViol = absViol;
  if (std::fabs(fval) + absViol > 1.0) {
    relViol = absViol / (std::fabs(fval) + absViol);
  }
  // log_->msgStream(LogDebug1) << me_ << "absolute viol "<<absViol << std::endl;

  return relViol;
}

void LogHandler::addLin_(LogCons &cd,
                           RelaxationPtr rel,
                           DoubleVector &tmpX,
                           DoubleVector &grad,
                           ModVector &,
                           bool )
{
  int error = 0;
  ConstraintPtr cons;
  double xlb = std::max(cd.riv->getLb(), 1e-7);
  double xub = std::min(cd.riv->getUb(), 1e12);
  double fxlbval = 0.0, fxubval = 0.0, dfxlbval = 0.0, dfxubval = 0.0;
  double tmpxval = 0.0, fxval = 0.0, dfxval = 0.0;
  LinearFunctionPtr lf;
  FunctionPtr f;
  FunctionPtr fn = cd.con->getFunction();

  int npts = 3;
  double xvals[] = { xlb, xub, (xub - xlb) / 2.0 };


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


    dfxval = grad[cd.riv->getIndex()];
    if (i == 0) {
      fxlbval = fxval;
      dfxlbval = dfxval;
    } else if (i == 1) {
      fxubval = fxval;
      dfxubval = dfxval;
    }
    //Tangents are global
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(cd.rov, 1.0);
    lf->addTerm(cd.riv, -dfxval);

    f = (FunctionPtr) new Function(lf);
    cons = rel->newConstraint(f, -INFINITY ,fxval - dfxval * xvals[i]);
    cd.linCons.push_back(cons);
#if SPEW
   log_->msgStream(LogDebug1) << me_ << "tangent added to the node for " 
      << cd.rov->getName() << " = log(" << cd.riv->getName() << ")" << std::endl;
      cd.linCons[i]->write(log_->msgStream(LogDebug1)); 
#endif


      
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
  double xlb = std::max(cd.riv->getLb(), 1e-7);
  double xub = std::min(cd.riv->getUb(), 1e12);
  double fxlb, fxub, m, intercept;
  LinearFunctionPtr lf;
  FunctionPtr f;
  FunctionPtr fn = cd.con->getFunction();

  xlb = cd.riv->getLb();
  xub = cd.riv->getUb();

  // No secant if unbounded either way
  if (xlb <= -0.9 * INFINITY || xub >= 0.9 * INFINITY) {
    log_->msgStream(LogDebug1) << me_
                               << "Cannot add secant -- bound is infinite"
                               << std::endl;
    return;
  }



  tmpX[cd.riv->getIndex()] = xlb;
  fxlb = fn->eval(tmpX, &error);
  tmpX[cd.riv->getIndex()] = xub;
  fxub = fn->eval(tmpX, &error);
  tmpX[cd.riv->getIndex()] = 0.0;///reset tmpX

  if (xub - xlb > 10e-7) {
    m = (fxub - fxlb) / (xub - xlb);
  } else {
    m = 0.0;
  }

  intercept = fxlb - m * xlb;
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -m);

  if (init) {//if at the root node
    f = (FunctionPtr) new Function(lf);
    cd.secCon = rel->newConstraint(f, intercept ,INFINITY);
#if SPEW
   log_->msgStream(LogDebug1) << me_ << "added new secant at root node for " 
                               << cd.rov->getName() << " = log(" << cd.riv->getName() << ")" << std::endl;
    cd.secCon->write(log_->msgStream(LogDebug1));
#endif

  } else {
    rel->changeConstraint(cd.secCon, lf,intercept, INFINITY); /// if not at root node modify the constraint
#if SPEW
   log_->msgStream(LogDebug1) << me_ << "modified secant at a node for " 
                               << cd.rov->getName() << " = log(" << cd.riv->getName() << ")" << std::endl;
    cd.secCon->write(log_->msgStream(LogDebug1));
#endif

    LinConModPtr lcmod =
      (LinConModPtr) new LinConMod(cd.secCon, lf, intercept ,INFINITY);
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

void LogHandler::relaxNodeInc(NodePtr node  ,
                              RelaxationPtr rel ,
                              bool *is_infeasible)
{ 
  ModVector mods;

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    (*it)->riv = rel->getVariable((*it)->iv->getIndex());
    (*it)->rov = rel->getVariable((*it)->ov->getIndex());
#if SPEW 
    log_->msgStream(LogDebug1) << me_ << "Updating log relaxation for node " 
                               << node->getId() << std::endl;
# endif
    updateRelax_(**it, rel, tmpX_, grad_, mods);
  }
  *is_infeasible = false;
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


void LogHandler::addCut_(VariablePtr x, VariablePtr y, double xval, double yval, 
                         RelaxationPtr rel, bool& ifcuts) {
  ifcuts = false;
  double log_x = std::log(xval);
  double violation = yval - log_x;
  double deriv, rhs;

  // Calculate scaled tolerance
  double cutTol = aTol_ ;

  // Check if the violation is mathematically significant
  if (violation > cutTol) {
    // Tangent (Overestimator)
    deriv = 1.0 / xval;
    rhs = log_x - 1.0;

    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.0);
    lf->addTerm(x, -deriv);
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr cut = rel->newConstraint(f, -INFINITY, rhs);
    ifcuts = true;

#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Separation cut added for violation " << violation 
                               << " (Threshold was " << cutTol << ")" << std::endl;
    cut->write(log_->msgStream(LogDebug1));
#endif
  } 
}
bool LogHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, bool& isfeas,
                             double& inf_meas)
{
  return isFeasible_(sol, rel, isfeas, inf_meas);
}


bool LogHandler::isFeasible_ (ConstSolutionPtr sol,
                            RelaxationPtr,
                            bool &,
                            double &inf_meas)
{
  bool isfeas = true;
  int num_inf=0;
  inf_meas = 0.0;
  int num1=0;

   double viol=0;
  // Get the raw primal array from the solution
  const double *primal = sol->getPrimal();

  // Convert the solution pointer data into a DoubleVector (std::vector<double>)
  // using the pointer range constructor.
  DoubleVector x(primal, primal + p_->getNumVars());

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
   num1++;
    LogCons *cd = *it;
    
    // Call getViol_ using the converted DoubleVector
    viol = getViol_(*cd, x);

    if (viol > eTol_) {
      // Check if variable range is larger than tolerance before flagging infeasibility
      if (cd->riv->getUb() - cd->riv->getLb() > vTol_) {
        isfeas = false;
        inf_meas += viol;
        num_inf++;
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Log constraint violated: ";
        cd->con->write(log_->msgStream(LogDebug1));
        log_->msgStream(LogDebug1) << " | Violation = " << viol << std::endl;
#endif
      }

    }  
  }
#if SPEW
  log_->msgStream(LogDebug1) << me_  
    << " num infeas = " << num_inf << " inf measure = " << inf_meas 
    << " number of feasible log constraint= " << num1-num_inf << std::endl;
#endif





  return isfeas;
}
void LogHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                          CutManager* cutman, SolutionPoolPtr s_pool,
                          ModVector& p_mods, ModVector& r_mods, bool* sol_found,
                          SeparationStatus* status)
{
  (void)node; (void)cutman; (void)s_pool; (void)p_mods; (void)r_mods; (void) status;
  
  double xval, yval;
  const double* x = sol->getPrimal();
  bool ifcuts = false;
  ++sStats_.iters;
  *sol_found = false;
  
  for(LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    LogCons* cd = *it;
    
    // Get current relaxation variables
    VariablePtr x_var = rel->getRelaxationVar(cd->iv);
    VariablePtr y_var = rel->getRelaxationVar(cd->ov);
    
    xval = x[x_var->getIndex()];
    yval = x[y_var->getIndex()];

    if (xval <= 1e-9) continue; 

    bool added = false;
    if( yval - log(xval) > aTol_)
    {

#if SPEW
      log_->msgStream(LogDebug1) << me_ << "Separation Function called. Point above log(x);";
      log_->msgStream(LogDebug1) << " | x = " << xval <<" |log(x)"<<log(xval)<< " | y = " << yval << " | Violation = " <<yval - log(xval) << std::endl;
      log_->msgStream(LogDebug1) << "Adding tangent for "<<std::endl; 
      cd->con->write(log_->msgStream(LogDebug1));
         
#endif
        addCut_(x_var, y_var, xval, yval, rel, added);

    }

       if (added) {
      ifcuts = true;
       ++sStats_.tangentcuts;
    }
  }

  if (ifcuts) {
    // *status = SepaResolve; // Update status if cuts were added
  }
}
void LogHandler::getBranchingCandidates(RelaxationPtr,
                                        const DoubleVector &x,
                                        ModVector &,
                                        BrVarCandSet &cands,
                                        BrCandVector &,
                                        bool &is_inf)
{
  is_inf = false;
  std::map<ConstVariablePtr, double> allCands;
  std::map<ConstVariablePtr, double>::iterator curc_it;

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    LogCons &cd = **it;
    double curviol = getViol_(cd, x);
    if (curviol > eTol_) {


#if SPEW
      log_->msgStream(LogDebug1) 
          << "branching candidate for ln(x): " << cd.iv->getName()
          << " value = " << x[cd.riv->getIndex()] 
          << " aux var: " << cd.ov->getName()
          << " value = " << x[cd.rov->getIndex()] 
          << " violation = " << curviol << std::endl;
#endif

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
    VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    lmods->insert(bmod);
    //add linearization correspoding to new bound
  } else if (brdir == UpBranch) {
    VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    lmods->insert(bmod);
    //add linearization correspoding to new bound
  }

  return lmods;
}

Branches LogHandler::getBranches(BrCandPtr cand,
                                 DoubleVector &x,
                                 RelaxationPtr rel,
                                 SolutionPoolPtr)
{
 double minFromBds = 0.1;
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar(); // Relaxation variable
  VariablePtr v2 = 0;              // Original variable
  double xval = x[v->getIndex()];
  double value = xval;
  double len = v->getUb() - v->getLb();
  VarBoundModPtr mod;
  Branches branches = (Branches) new BranchPtrVector();

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

 
    // Down Branch
 
  BranchPtr branch = (BranchPtr) new Branch();
 
  // Problem Modification 
  if (modProb_) {
    v2 = rel->getOriginalVar(v);
    mod = (VarBoundModPtr) new VarBoundMod(v2, Upper, value);
    branch->addPMod(mod);
  }
  
  if(modRel_) 
  {
    mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    branch->addRMod(mod);
  }
  
  branch->setActivity(value);
  branches->push_back(branch);

    // Up Branch
   branch = (BranchPtr) new Branch();
  
  // Problem Modification
  if (modProb_) {
    mod = (VarBoundModPtr) new VarBoundMod(v2, Lower, value);
    branch->addPMod(mod);
  }
  
  if(modRel_)
  {
    mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    branch->addRMod(mod);
  }
  
  branch->setActivity(value);
  branches->push_back(branch);
  vcand->setNumBranches(2);
 #if SPEW
  log_->msgStream(LogDebug1)
      << me_ << "branching on " << v->getName() << " <= " << value << " or "
      << " >= " << value << std::endl;
#endif

  return branches;
}

BranchPtr LogHandler::doBranch_(BranchDirection UpOrDown,
                                ConstVariablePtr v,
                                double bvalue)
{
  BranchPtr branch;
  LinModsPtr linmods;

#if MDBUG
  log_->msgStream(LogDebug1)
    << me_ << "LogHandler, Branching: "
    << (UpOrDown == DownBranch ? "Down" : "Up")
    << " at value: " << bvalue << " on: " << std::endl;
  v->write(log_->msgStream(LogDebug1));
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

 // assert(!"add Mod correctly here.");
  branch->addPMod(linmods);
  return branch;
}


// Duplicate row detection and correction
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
  
// ---it checks if that number is extremely close (within 0.0001) to a whole number or not--
   if (std::abs(newlb - std::round(newlb)) < 1e-4) {
    newlb = std::round(newlb);
  }
  if (std::abs(newub - std::round(newub)) < 1e-4) {
    newub = std::round(newub);
  }
  
  
  if (newlb > p->getLb() + eTol_ || newub < p->getUb() - eTol_) {
  log_->msgStream(LogDebug2) << me_ <<" Tightening bounds for: " << p->getName() 
              << " (Index: " << p->getIndex() << ")" << std::endl;
  }

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
  if (newlb > lb ) {
    lb = newlb;
    *changed = true;
    ++pStats_.vBnd;
  }

  // Tighten upper bound if possible
  if (newub < ub ) {
    ub = newub;
    *changed = true;
    ++pStats_.vBnd;
  }


 // Infeasibility check
  if (lb > ub + eTol_ ) {
    log_->msgStream(LogDebug2) << me_
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


int LogHandler::updatePBnds_(VariablePtr v, double newlb, double newub,
                             RelaxationPtr rel, bool mod_rel, bool *changed,
                             ModVector &p_mods, ModVector &r_mods)
{
  VarBoundModPtr bmod;
  VarBoundMod2Ptr b2mod;
  double lb = v->getLb();
  double ub = v->getUb();

  // --- ADDED: Snapping logic for floating point noise ---
  if (std::abs(newlb - std::round(newlb)) < 1e-4) {
    newlb = std::round(newlb);
  }
  if (std::abs(newub - std::round(newub)) < 1e-4) {
    newub = std::round(newub);
  }
  // ----------------------------------------------------

  // Rounding logic for integer variables
  if (v->getType() == Binary || v->getType() == ImplBin ||
      v->getType() == Integer || v->getType() == ImplInt) {
    if (newlb - floor(newlb) < eTol_) newlb = floor(newlb);
    else newlb = ceil(newlb);

    if (ceil(newub) - newub < eTol_) newub = ceil(newub);
    else newub = floor(newub);
  }

  // Sanity check: if new bounds are completely outside existing bounds
  if (newlb > ub + eTol_ || newub < lb - eTol_) {
    return -1; // Infeasible
  }

  bool tighten_lb = false;
  bool tighten_ub = false;

  // Check Lower Bound tightening (REMOVED eTol_)
  if (newlb > lb) {
    lb = newlb;
    tighten_lb = true;
  }

  // Check Upper Bound tightening (REMOVED eTol_)
  if (newub < ub) {
    ub = newub;
    tighten_ub = true;
  }

  // Check infeasibility after tightening
  if (lb > ub + eTol_) {
    return -1;
  }

  // If no change, return success
  if (!tighten_lb && !tighten_ub) {
    return 0;
  }

  *changed = true;
  ++pStats_.vBnd;

  // Apply changes using Modifications
  if (tighten_lb && tighten_ub) {
    // Both bounds changed: Use VarBoundMod2
    b2mod = (VarBoundMod2Ptr) new VarBoundMod2(v, lb, ub);
    b2mod->applyToProblem(p_);
    p_mods.push_back(b2mod);

    if (mod_rel) {
      VariablePtr rv = rel->getVariable(v->getIndex());
      b2mod = (VarBoundMod2Ptr) new VarBoundMod2(rv, lb, ub);
      b2mod->applyToProblem(rel);
      r_mods.push_back(b2mod);
    }
  } 
  else if (tighten_lb) {
    // Lower bound changed: Use VarBoundMod
    bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, lb);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);

    if (mod_rel) {
      VariablePtr rv = rel->getVariable(v->getIndex());
      bmod = (VarBoundModPtr) new VarBoundMod(rv, Lower, lb);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  } 
  else if (tighten_ub) {
    // Upper bound changed: Use VarBoundMod
    bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, ub);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);

    if (mod_rel) {
      VariablePtr rv = rel->getVariable(v->getIndex());
      bmod = (VarBoundModPtr) new VarBoundMod(rv, Upper, ub);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
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


  out << me_ << "Statistics for separation by LogHandler:" << std::endl
      << me_ << "Number of calls to separate     = " << sStats_.iters
      << std::endl
      << me_ << "Number of tangent cuts added    = " << sStats_.tangentcuts
      << std::endl;

}

