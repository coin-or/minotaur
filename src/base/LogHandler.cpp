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
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Variable.h"


using namespace Minotaur;

const std::string LogHandler::me_ = "LogHandler: ";
LogHandler::LogHandler(EnvPtr env, ProblemPtr problem)
  : bStats_(),
    pStats_(),
    sStats_(),
    LBd_(1e-6),
    UBd_(1e50),
    bTol_(1e-6),
    env_(env),
    orig_(problem),
    optCuts_(),
    aTol_(1e-6),
    rTol_(1e-6),
    eTol_(1e-5),
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

void LogHandler::relaxLog_(LogCons &cd, RelaxationPtr rel, bool init)
{
  ModVector mods; // only needed to match addSecant_'s signature

  if (init) {
    cd.riv = rel->getVariable(cd.iv->getIndex());
    cd.rov = rel->getVariable(cd.ov->getIndex());
  }

  // secant: created at the root (init), overwritten at every later node
  if (cd.sense == 'E' || cd.sense == 'L') {
    addSecant_(cd, rel);
  }

  // tangents are added once at the root and then later using separation 
  if (init && (cd.sense == 'E' || cd.sense == 'G')) {
    addLin_(cd, rel);
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
    relaxLog_(**it, rel, true);
  }

  *is_inf = false;
}

void LogHandler::relaxNodeInc(NodePtr node ,
                              RelaxationPtr rel,
                              bool *is_infeasible)
{
#if SPEW
  log_->msgStream(LogDebug1) << me_ << "Updating log relaxation for node "
                             << node->getId() << std::endl;
#endif

  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    relaxLog_(**it, rel, false);
  }

  *is_infeasible = false;
}

double LogHandler::getViol_(const LogCons &cd,
                              const DoubleVector &x) const
{
 
 double xval, fval, fhat, absViol, relViol;

  xval = x[cd.riv->getIndex()];
  fhat = x[cd.rov->getIndex()];


  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();
if (xlb > 0.0 && std::log(xub / xlb) < vTol_) return 0.0;

if (xval < 1e-9) return 1e9;

  fval = std::log(xval);
  absViol = std::abs(fhat - fval);
  relViol = absViol;

  if (std::fabs(fval) + absViol > 1.0) {
    relViol = absViol / (std::fabs(fval) + absViol);
  }

  return relViol;
}

ConstraintPtr LogHandler::addTangent_(VariablePtr x, VariablePtr y, double x0,
                                      RelaxationPtr rel)
{
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(y, 1.0);
  lf->addTerm(x, -1.0 / x0);
  FunctionPtr f = (FunctionPtr) new Function(lf);
#if SPEW
  log_->msgStream(LogDebug1) << me_ << "tangent for " << y->getName()
                             << " = log(" << x->getName() << ") at " << x0
                             << std::endl;
#endif
  return rel->newConstraint(f, -INFINITY, std::log(x0) - 1.0);
}

void LogHandler::addLin_(LogCons &cd, RelaxationPtr rel)
{
  double a = std::max(cd.riv->getLb(), LBd_);
  double b = std::min(cd.riv->getUb(), UBd_);
  double pts[3] = {a, b, 0.5 * (a + b)};
  int n = 1;                       // fixed variable: one tangent is enough

  if (b - a > 1e-5) {
    // third point: intersection of the tangents at a and b, kept 5% inside
    double xi  = a * b * std::log1p((b - a) / a) / (b - a);
    double pad = 0.05 * (b - a);
    pts[2] = std::min(std::max(xi, a + pad), b - pad);
    n = 3;
  }

  for (int i = 0; i < n; ++i) {
    cd.linCons.push_back(addTangent_(cd.riv, cd.rov, pts[i], rel));
  }
}

void LogHandler::addCut_(VariablePtr x, VariablePtr y, double xval,
                         double yval, RelaxationPtr rel, bool &ifcuts)
{
  ifcuts = (yval - std::log(xval) > aTol_);
  if (ifcuts) {
    addTangent_(x, y, xval, rel);
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Separation cut added for violation " <<yval - std::log(xval) 
                               << " (Threshold was " << aTol_ << ")" << std::endl;
#endif

  }
}


void LogHandler::addSecant_(LogCons &cd, RelaxationPtr rel)
{
  double xlb = std::max(cd.riv->getLb(), LBd_);
  double xub = std::min(cd.riv->getUb(), UBd_);
  double fxlb = std::log(xlb);

  // slope 0 (flat row y >= log(xlb)) if x is unbounded above or nearly fixed
  double m = 0.0;
  if (cd.riv->getUb() < 1e20 && std::log(xub / xlb) > 1e-6) {
    m = (std::log(xub) - fxlb) / (xub - xlb);
  }

  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -m);
  double lb = fxlb - m * xlb;

  if (cd.secCon) {
    rel->changeConstraint(cd.secCon, lf, lb, INFINITY);
  } else {
    cd.secCon = rel->newConstraint((FunctionPtr) new Function(lf), lb, INFINITY);
  }
#if SPEW
  log_->msgStream(LogDebug1) << me_ << "secant for " << cd.rov->getName()
                             << " = log(" << cd.riv->getName() << ") on ["
                             << xlb << ", " << xub << "]" << std::endl;
#endif
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
    if( yval - log(xval) > 1e-5) //thsi violation should be some scaled one 
    {

#if SPEW
      log_->msgStream(LogDebug1) << me_ << "Separation Function called. Point above log(x);";
      log_->msgStream(LogDebug1) << " | x = " << xval <<" |log(x)"<<log(xval)<< " | y = " << yval << " | Violation = " <<yval - log(xval) << std::endl;
      log_->msgStream(LogDebug1) << "Adding tangent for "<<std::endl; 
      cd->con->write(log_->msgStream(LogDebug2));
         
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

  double lb  = std::max(v->getLb(), LBd_);
  double ub  = std::min(v->getUb(), UBd_);
  double len = ub - lb;

  double xval  = x[v->getIndex()];
  double value = xval;

  if (value < lb + minFromBds * len) {
  value = lb + minFromBds * len;
  } else if (value > ub - minFromBds * len) {
  value = ub - minFromBds * len;
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
  double lb  = std::max(v->getLb(), LBd_);
  double ub  = std::min(v->getUb(), UBd_);
  double len = ub - lb;
  VarBoundModPtr mod;
  Branches branches = (Branches) new BranchPtrVector();

if (value < lb + minFromBds * len) {
  value = lb + minFromBds * len;
} else if (value > ub - minFromBds * len) {
  value = ub - minFromBds * len;
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




bool LogHandler::propLogBnds_(LogConsPtr cdata, bool *changed)
{
  VariablePtr x, y;
  double xlb, xub, ylb, yub;
  double new_ylb, new_yub, new_xlb, new_xub;
  const double logEps = 1e-7;  // x must stay strictly positive for log(x) to be defined
  x = cdata->iv;  // input variable x
  y = cdata->ov;  // output variable y = log(x)

  xlb = x->getLb();
  xub = x->getUb();
  ylb = y->getLb();
  yub = y->getUb();

  // Infeasibility Check 
  // strictly undefined for x <= 0
  if (xub <= 0.0) {
    return true; 
  }

  //   x -> y 

  if (xlb > 0.0) {
    new_ylb = std::log(xlb);
  } else {
    // If xlb <= 0, the valid domain of log(x) has negative infinity.
    // We MUST allow y to go to -INFINITY to support fractional x values.
    new_ylb = std::log(logEps); 
  }

  // Map xub to new_yub
  if (xub < INFINITY) {
    new_yub = std::log(xub);
  } else {
    new_yub = INFINITY;
  }

  if (updatePBnds_(y, new_ylb, new_yub, changed) < 0) {
    return true;
  }

  //y -> x 
  // The inverse:w of y = log(x) is x = e^y.
  // e^y maps (-inf, inf) to (0, inf).
  
  // Lower bound 
  if (ylb > -1e15) {
    new_xlb = std::exp(ylb);
  } else {
    new_xlb = 0.0; // e^(-inf) approaches 0
  }

  // Upper bound deduction (with overflow protection)
  if (yub < 700.0) {
    new_xub = std::exp(yub);
  } else {
    new_xub = INFINITY;
  }

  if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) {
    return true;  }

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
  
  
  if (newlb > p->getLb() + eTol_ || newub < p->getUb() - eTol_) {
  log_->msgStream(LogDebug2) << me_ <<" Tightening bounds for: " << p->getName() 
              << " (Index: " << p->getIndex() << ")" << std::endl;
  }

  if (p->getType() == Binary || p->getType() == ImplBin ||
        p->getType() == Integer || p->getType() == ImplInt) {
      // it checks if that number is extremely close (within 0.0001) to a whole number or not
    if (std::abs(newlb - std::round(newlb)) < 1e-4) {
    newlb = std::round(newlb);
    }
    if (std::abs(newub - std::round(newub)) < 1e-4) {
    newub = std::round(newub);
    }
    // Lower bound: ceil only if newlb is significantly greater,
    // else floor 
    if (newlb - floor(newlb)  < eTol_) {
      newlb = floor(newlb);
    } else {
      newlb = ceil(newlb);
    }

    // Upper bound: floor only if newub is siginficantly smaller
    // else ceil 

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

SolveStatus LogHandler::presolve(PreModQ *, bool *changed, Solution **)
{
  ++pStats_.iters;
  *changed = false;
  for (LogConsIter it = consd_.begin(); it != consd_.end(); ++it) {
    if (propLogBnds_(*it, changed)) return SolvedInfeasible;
  }
  return Finished;
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

