// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2014 The MINOTAUR Team.
// 

/**
 * \file QuadHandler.cpp
 * \brief Implement a handler for simple quadratic constraints of the form
 * \f$ y = x_1x_2 \f$,
 * and
 * \f$ y = x_1^2 \f$,
 * \author Ashutosh Mahajan, IIT Bombay
 */


#include <cmath>
#include <iostream>
#include <iomanip>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrCand.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "LinMods.h"
#include "Logger.h"
#include "SecantMod.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "QuadHandler.h"
#include "QuadraticFunction.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Variable.h"

//#define SPEW 1
// TODO: arrange alphabetically

using namespace Minotaur;

const std::string QuadHandler::me_ = "QuadHandler: ";

QuadHandler::QuadHandler(EnvPtr env, ProblemPtr problem)
  : eTol_(1e-6)
{
  p_ = problem; 
  modProb_ = true;
  modRel_ = true;
  logger_  = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("handler_log_level")->getValue());
}


QuadHandler::~QuadHandler()
{
  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    delete it->second;
  }
  x2Funs_.clear();
  x0x1Funs_.clear();
}


void QuadHandler::addLinSqr_(VariablePtr x, VariablePtr y, RelaxationPtr rel)
{
  LinearFunctionPtr lf;
  LinSqrPtr lx2;
  ConstraintPtr newcon;
  FunctionPtr f;

  double rhs;
  //bounds on y
  double lb = -1e21;
  double ub =  1e21;

  BoundsOnSquare(x, lb, ub);
  lb = std::max(lb, y->getLb());
  ub = std::min(ub, y->getUb());
  lb = std::max(lb, -1e20);
  ub = std::min(ub,  1e20);

  lf = getNewSqLf_(rel->getRelaxationVar(x), rel->getRelaxationVar(y), lb, ub,
                   rhs);

  f = (FunctionPtr) new Function(lf);
  newcon = rel->newConstraint(f, -INFINITY, rhs);

  lx2        = new LinSqr();
  lx2->y     = y;
  lx2->x     = x;
  lx2->OECon = newcon;

  x2Funs_.insert(std::pair<VariablePtr, LinSqrPtr>(x, lx2));
}


void QuadHandler::addLinBil_(VariablePtr x0, VariablePtr x1, VariablePtr y,
                             RelaxationPtr rel)
{
  // add linear inequalities for \f$  y \leq x0.x1 \f$.
  LinBilPtr linbil = new LinBil(x0, x1, y);
  LinBilSetIter biter;
  double lb0, lb1, ub0, ub1;
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr cons;
  double rhs = 0;
  ConstraintVector newcons(4);

  biter = x0x1Funs_.find(linbil);
  if (biter==x0x1Funs_.end()) {

    // add a new bilinear term in our list
    lb0 = std::max(x0->getLb(), -1e20);
    ub0 = std::min(x0->getUb(),  1e20);
    lb1 = std::max(x1->getLb(), -1e20);
    ub1 = std::min(x1->getUb(),  1e20);

    for (UInt i=0; i<4; ++i) {
      lf = getNewBilLf_(rel->getRelaxationVar(x0), lb0, ub0,
                        rel->getRelaxationVar(x1), lb1, ub1,
                        rel->getRelaxationVar(y), i, rhs);
      f = (FunctionPtr) new Function(lf);

      newcons[i] = rel->newConstraint(f, -INFINITY, rhs);
    }
    linbil->setCons(newcons[0], newcons[1], newcons[2], newcons[3]);
  } 
}


void QuadHandler::relax_(RelaxationPtr rel, bool *)
{
  ConstraintConstIterator c_iter;
  ConstraintPtr c;

  LinearFunctionPtr lf0, lf1, lf;
  QuadraticFunctionPtr qf; 
  NonlinearFunctionPtr nlf;
  VariablePtr y, x0, x1;
  FunctionPtr f;

  std::vector< VariablePtr > psqVars, nsqVars;
  VariablePtr v0, v1, v;

  for (c_iter=cons_.begin(); c_iter!=cons_.end(); ++c_iter) {
    c = (*c_iter);
    qf = c->getQuadraticFunction();
    if (qf) {
      assert(!"cannot yet handle qf in QuadHandler.");
    } else {
      nlf = c->getNonlinearFunction();
      lf  = c->getLinearFunction();
      assert(lf && nlf);
      assert(1==lf->getNumTerms());
      assert(nlf->numVars()<3);
      y = lf->termsBegin()->first;
      if (nlf->numVars()==1) {
        x0 = *(nlf->varsBegin());
        addLinSqr_(x0, y, rel);
      } else {
        VariableSet::iterator it = nlf->varsBegin();
        x0 = *it;
        ++it;
        x1 = *it;
        addLinBil_(x0, x1, y, rel);
      }
    }
  }
  std::cout << "Writing the Tranformed Problem:\n";
  p_->write(std::cout);
  std::cout << "Writing the relaxation after quadratic relaxation:\n";
  rel->write(std::cout);
  assert(0 == rel->checkConVars());
  return;
}


void QuadHandler::relaxInitFull(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}


void QuadHandler::relaxInitInc(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}


void QuadHandler::relaxNodeFull(NodePtr, RelaxationPtr, bool *)
{
  assert(!"QuadHandler::relaxNodeFull not implemented!");
}


void QuadHandler::relaxNodeInc(NodePtr, RelaxationPtr, bool *)
{
  assert(!"QuadHandler::relaxNodeInc not implemented!");
}

#if 0
void QuadHandler::relaxTwoSided_(QuadraticFunctionPtr qf, 
    ConstraintPtr cons, RelaxationPtr rel)
{
  VariablePtr v0, v1, v;
  Double vlb, vub;
  FunctionPtr f;

  QuadraticFunctionPtr cx_qf0 = (QuadraticFunctionPtr) new QuadraticFunction();
  QuadraticFunctionPtr cx_qf1 = (QuadraticFunctionPtr) new QuadraticFunction();
  LinearFunctionPtr lf0 = (LinearFunctionPtr) new LinearFunction();
  LinearFunctionPtr lf1 = (LinearFunctionPtr) new LinearFunction();

  for (VariablePairGroupConstIterator it=qf->begin(); it!=qf->end(); 
      ++it) {
    v0 = rel->getVariable(it->first.first->getIndex());
    v1 = rel->getVariable(it->first.second->getIndex());
    if (v0==v1) {
      BoundsOnSquare(v0, vlb, vub);
      // convex and secant.
      if (it->second > 0.) {
        // convex part
        cx_qf0->incTerm(std::make_pair(v0, v1), it->second);
        // secant part
        v = rel->newVariable(vlb, vub, Continuous);
        lf1->incTerm(v, -1.0*it->second);
        addSecant_(v0, v, rel);
      } else {
        // convex part
        cx_qf1->incTerm(std::make_pair(v0, v1), -1.*it->second);
        // secant part
        v = rel->newVariable(vlb, vub, Continuous);
        lf0->incTerm(v, it->second);
        addSecant_(v0, v, rel);
      }
    } else {
      // LinBil
      v = addLinBil_(v0, v1, rel);
      lf0->incTerm(v, it->second);
      lf1->incTerm(v, -it->second);
    }
  }

  if (cx_qf0->getNumTerms()==0) {
    cx_qf0.reset();
  }
  if (cx_qf1->getNumTerms()==0) {
    cx_qf1.reset();
  }

  (*lf0) += cons->getLinearFunction();
  (*lf1) -= cons->getLinearFunction();

  if (!cx_qf0 && !cx_qf1) {
    // If there are no quadratic terms left, we need to add only one linear
    // constraint.
    f = (FunctionPtr) new Function(lf0);
    rel->newConstraint(f, cons->getLb(), cons->getUb());
  } else if (!cx_qf0 && cx_qf1) {
    f = (FunctionPtr) new Function(lf0);
    rel->newConstraint(f, cons->getLb(), cons->getUb());

    f = (FunctionPtr) new Function(lf1, cx_qf1);
    rel->newConstraint(f, -INFINITY, -1.*cons->getLb());
  } else {
    if (cx_qf0) {
      f = (FunctionPtr) new Function(lf0, cx_qf0);
      rel->newConstraint(f, -INFINITY, cons->getUb());
    } else {
      f = (FunctionPtr) new Function(lf1);
      rel->newConstraint(f, -1.*cons->getUb(), -1.*cons->getLb());
    }

    if (cx_qf1) {
      f = (FunctionPtr) new Function(lf1, cx_qf1);
      rel->newConstraint(f, -INFINITY, -1.*cons->getLb());
    } else {
      f = (FunctionPtr) new Function(lf1);
      rel->newConstraint(f, -cons->getUb(), -cons->getLb());
    }
  }
}
#endif 

#if 0
void QuadHandler::relaxOneSided_(QuadraticFunctionPtr qf, 
    ConstraintPtr cons, RelaxationPtr rel)
{
  VariablePtr v0, v1, v;
  Double vlb, vub;
  FunctionPtr f;

  QuadraticFunctionPtr cx_qf0 = (QuadraticFunctionPtr) new QuadraticFunction();
  LinearFunctionPtr lf0 = (LinearFunctionPtr) new LinearFunction();


  for (VariablePairGroupConstIterator it=qf->begin(); it!=qf->end(); 
      ++it) {
    v0 = rel->getVariable(it->first.first->getIndex());
    v1 = rel->getVariable(it->first.second->getIndex());
    if (v0==v1) {
      if (it->second > 0.) {
        // convex only 
        cx_qf0->incTerm(std::make_pair(v0, v0), it->second);
      } else {
        // secant only 
        BoundsOnSquare(v0, vlb, vub);
        v = rel->newVariable(vlb, vub, Continuous);
        lf0->incTerm(v, it->second);
        addSecant_(v0, v, rel);
      }
    } else {
      // LinBil
      if (it->second > 0.) {
        // we only need lower-LinBil for y >= x1x2
        v = addLinBilLower_(v0, v1, rel);
      } else {
        // we only need upper-LinBil for y <= x1x2
        v = addLinBilUpper_(v0, v1, rel);
      }
      assert(v); // v should not be NULL
      lf0->incTerm(v, it->second);
    }
  }

  if (cx_qf0->getNumTerms()==0) {
    cx_qf0.reset();
  }
  (*lf0) += cons->getLinearFunction();
  f = (FunctionPtr) new Function(lf0, cx_qf0);
  rel->newConstraint(f, -INFINITY, cons->getUb());
}
#endif


#if 0
void QuadHandler::addSecant_(VariablePtr x, VariablePtr y, RelaxationPtr rel)
{
  LinearFunctionPtr lf;
  LinSqrPtr sec;
  ConstraintPtr cons;
  FunctionPtr f;

  Double lb = x->getLb();
  Double ub = x->getUb();
  Double r;

  lf = getNewSecantLf_(x, y, lb, ub, r);

  f = (FunctionPtr) new Function(lf);
  cons = rel->newConstraint(f, -INFINITY, r);

  sec         = new Secant();
  sec->y = y;
  sec->sqVar  = x;
  sec->cons   = cons;

  x2Funs_.insert(std::pair<VariablePtr, LinSqrPtr>(x, sec));
}
#endif



LinearFunctionPtr QuadHandler::getNewSqLf_(VariablePtr x, VariablePtr y,
                                           double lb, double ub, double & r)
{
  LinearFunctionPtr lf = LinearFunctionPtr(); // NULL
  r = -ub*lb;
  assert (lb > -1e21 && ub < 1e21); // Can't approximate when unbounded
  if (fabs(ub+lb) > eTol_) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.);
    lf->addTerm(x, -1.*(ub+lb));
  } else {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.);
#if SPEW
    logger_->MsgStream(LogDebug) << me_ << 
      "warning: generating a bound as a secant constraint." << std::endl;
#endif
  }
  return lf;
}



LinearFunctionPtr QuadHandler::getNewBilLf_(VariablePtr x0, double lb0,
                                            double ub0, VariablePtr x1,
                                            double lb1, double ub1,
                                            VariablePtr y, int type,
                                            double &rhs)
{
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

  switch (type) {
  case(0):
    // y >= l0x1 + l1x0 - l1l0
    lf->addTerm(x1, lb0);
    lf->addTerm(x0, lb1);
    lf->addTerm(y, -1.);
    rhs = lb0*lb1;
  case(1):
     // y >= u0x1 + u1x0 - u1u0
     lf->addTerm(x1, ub0);
     lf->addTerm(x0, ub1);
     lf->addTerm(y, -1.);
     rhs = ub0*ub1;
     break;
  case(2):
     // y <= u1x0 + l0x1 - l0u1
     lf->addTerm(x0, -1.0*ub1);
     lf->addTerm(x1, -1.0*lb0);
     lf->addTerm(y, 1.);
     rhs = -lb0*ub1;
     break;
  case(3):
     // y <= l1x0 + u0x1 - u0l1
     lf->addTerm(x0, -1.0*lb1);
     lf->addTerm(x1, -1.0*ub0);
     lf->addTerm(y, 1.);
     rhs = -ub0*lb1;
     break;

  default:
     assert(!"getNewBilLf_ called with wrong value of i");
     break;
  }
  return lf;
}


bool QuadHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr , bool & )
{
  Double yval, xval;
  const Double *x = sol->getPrimal();

  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    // check if y <= x^2
    xval  = x[it->first->getIndex()];
    yval = x[it->second->y->getIndex()];
    if (yval > xval*xval + eTol_) {
      return false;
    }
  }
#if SPEW
  logger_->MsgStream(LogDebug2) << me_ << "relaxation is secant feasible." 
    << std::endl;
#endif

  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    if ((*it)->isViolated(x, eTol_)) {
      return false;
    }
  }

#if SPEW
  logger_->MsgStream(LogDebug2) << me_ << "relaxation is LinBil feasible." 
    << std::endl;
#endif
  return true;
}


void QuadHandler::getBranchingCandidates(RelaxationPtr, 
    const DoubleVector &x, ModVector & , BrCandSet & cands, 
    bool & is_inf)
{
  Double yval, x0val, x1val;
  BrVarCandPtr br_can;
  VariablePtr x0, x1;
  UIntSet cand_inds;
  std::pair<UIntSet::iterator, bool> ret;
#if DEBUG
  bool check;
#endif

  is_inf = false;

  // First check if there is a candidate x0 that violates y <= x0^2.
  for (LinSqrMapIter s_it=x2Funs_.begin(); s_it!=x2Funs_.end(); ++s_it) {
    x0val = x[s_it->first->getIndex()];
    yval = x[s_it->second->y->getIndex()];
    if (yval > x0val*x0val+eTol_) {
#if SPEW
      logger_->MsgStream(LogDebug2) << std::setprecision(9) << me_ 
        << "branching candidate for secant: " << s_it->first->getName()
        << " value = " << x0val << " aux var: " 
        << s_it->second->y->getName() 
        << " value = " << yval << std::endl;
      //s_it->first->write(std::cout);
      //s_it->second->y->write(std::cout);
#endif
      ret = cand_inds.insert(s_it->first->getIndex());
      if (true == ret.second) { // does not already exist.
        br_can = (BrVarCandPtr) new BrVarCand(s_it->first, 
            s_it->first->getIndex(), 0.5);
        cands.insert(br_can);
      }
    }
  }

  // Now check if there is a violated constraint of the form y <=/=/=> x0.x1.
  // If so, add both x0 and x1 to the candidate set.
  for (LinBilSetIter s_it=x0x1Funs_.begin(); s_it!=x0x1Funs_.end(); ++s_it) {
    LinBilPtr bil = (*s_it);
    x0 = bil->getX0();
    x1 = bil->getX1();
    x0val = x[x0->getIndex()];
    x1val = x[x1->getIndex()];
    yval  = x[bil->getY()->getIndex()];
    if (bil->isViolated(x0val, x1val, yval, eTol_)) {
#if DEBUG
      check = false;
#endif
      // If a variable is at bounds, then it is not a candidate.
      if (x0val < x0->getUb() - eTol_ && x0val > x0->getLb() + eTol_) {
#if DEBUG
        check = true;
#endif
        ret = cand_inds.insert(x0->getIndex());
        if (true == ret.second) { // does not already exist.
          br_can = (BrVarCandPtr) new BrVarCand(x0, x0->getIndex(), 0.5); 
          cands.insert(br_can);
        }
      }

      if (x1val < x1->getUb() - eTol_ && x1val > x1->getLb() + eTol_) {
#if DEBUG
        check = true;
#endif
        ret = cand_inds.insert(x1->getIndex());
        if (true == ret.second) { // does not already exist.
          br_can = (BrVarCandPtr) new BrVarCand(x1, x1->getIndex(), 0.5); 
          cands.insert(br_can);
        }
      }
      //if (false==check) {
      //  std::cout << std::setprecision(9) << x0->getName() 
      //   << " = " << x0val << " " << x1->getName() 
      //   << " = " << x1val << " " << bil->getAux()->getName() 
      //   << " = " << yval << std::endl;
      //  rel->write(std::cout);
      //}
#if DEBUG
      assert (check); // If both variables are at bounds, the LinBil 
                      // inequalities can not be violated.
#endif
    }
  }
}


ModificationPtr QuadHandler::getBrMod(BrCandPtr cand, DoubleVector &xval, 
    RelaxationPtr , BranchDirection dir)
{
  Double            lb, ub, lb1, ub1, b2, rhs=0;
  BoundType         lu;
  ConstraintPtr     cons;
  BrVarCandPtr      vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  VariablePtr       x0, x1, v, y;
  LinSqrMapIter  s_it;
  LinearFunctionPtr lf;
  LinBilPtr      mcc;
  SecantModPtr      smod;
  LinModsPtr        lmods;
  LinConModPtr      lmod;
  VarBoundModPtr    bmod;
  VarBoundMod2Ptr   b2mod;

  x0 = vcand->getVar();

  if (dir == DownBranch ) {
    lb    = x0->getLb();
    ub    = xval[x0->getIndex()];
    b2    = ub;
    lu    = Upper;
  } else {
    lb    = xval[x0->getIndex()];
    ub    = x0->getUb();
    b2    = lb;
    lu    = Lower;
  }

  // first find if we have secants associated with x0
  s_it = x2Funs_.find(x0);
  if (s_it!=x2Funs_.end()) {
    y = s_it->second->y;
    cons = s_it->second->OECon;
    //lf    = getNewSecantLf_(x0, y, lb, ub, rhs);
    smod = (SecantModPtr) new SecantMod(cons, lf, rhs, x0, lu, b2, y);
    return smod;
  } 

  // also try to find any LinBil inequalities associated with x0
  lmods = (LinModsPtr) new LinMods();
  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    mcc = (*it);
    x1 = mcc->getOtherX(x0);
    if (x1) {
      // This term contains x0 and x1. 
      y = mcc->getY();
      lmods = (LinModsPtr) new LinMods();
      //if (mcc->getSense() == LinBil::LT || 
      //    mcc->getSense() == LinBil::EQ) {
      //  // y >= l0x1 + l1x0 - l1l0
      //  lf = getMcLf_(x0, lb, ub, x1, x1->getLb(), x1->getUb(), y, rhs, 0);
      //  lmod = (LinConModPtr) new LinConMod(mcc->getC0(), lf, -INFINITY, rhs);
      //  lmods->insert(lmod);

      //  // y >= u0x1 + u1x0 - u1u0
      //  lf = getMcLf_(x0, lb, ub, x1, x1->getLb(), x1->getUb(), y, rhs, 1);
      //  lmod = (LinConModPtr) new LinConMod(mcc->getC1(), lf, -INFINITY, rhs);
      //  lmods->insert(lmod);
      //}

      //if (mcc->getSense() == LinBil::GT || 
      //    mcc->getSense() == LinBil::EQ) {
      //  // y <= u1x0 + l0x1 - l0u1
      //  lf = getMcLf_(x0, lb, ub, x1, x1->getLb(), x1->getUb(), y, rhs, 2);
      //  lmod = (LinConModPtr) new LinConMod(mcc->getC2(), lf, -INFINITY, rhs);
      //  lmods->insert(lmod);

      //  // y <= l1x0 + u0x1 - u0l1
      //  lf = getMcLf_(x0, lb, ub, x1, x1->getLb(), x1->getUb(), y, rhs, 3);
      //  lmod = (LinConModPtr) new LinConMod(mcc->getC3(), lf, -INFINITY, rhs);
      //  lmods->insert(lmod);
      //}
      BoundsOnProduct(lb, ub, x1->getLb(), x1->getUb(), lb1, ub1);
      b2mod  = (VarBoundMod2Ptr) new VarBoundMod2(y, lb1, ub1);
      lmods->insert(b2mod);
    }
  }
  bmod = (VarBoundModPtr) new VarBoundMod(x0, lu, b2);
  lmods->insert(bmod);
  return lmods;
}


Branches QuadHandler::getBranches(BrCandPtr cand, DoubleVector & x,
                                  RelaxationPtr rel, SolutionPoolPtr)
{
  BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  VariablePtr v = vcand->getVar();
  VariablePtr v2;
  Double value = x[v->getIndex()];
  BranchPtr branch;
  VarBoundModPtr mod;
  Branches branches = (Branches) new BranchPtrVector();

  // can't branch on something that is at its bounds.
  assert(value > v->getLb()+1e-8 && value < v->getUb()-1e-8);

  // down branch
  branch = (BranchPtr) new Branch();
  if (modProb_) {
    v2 = rel->getOriginalVar(v);
    mod = (VarBoundModPtr) new VarBoundMod(v2, Upper, value);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod = (VarBoundModPtr) new VarBoundMod(v, Upper, value);
    branch->addRMod(mod);
  }
  branch->setActivity(0.5);// TODO: set this correctly
  branches->push_back(branch);

  // up branch
  branch = (BranchPtr) new Branch();
  if (modProb_) {
    mod    = (VarBoundModPtr) new VarBoundMod(v2, Lower, value);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod    = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    branch->addRMod(mod);
  }
  branch->setActivity(0.5); // TODO: set this correctly
  branches->push_back(branch);

#if SPEW
  logger_->MsgStream(LogDebug2) << me_ << "branching on " << v->getName()
                                       << " <= " << value << " or " 
                                       << " >= " << value << std::endl;
#endif
  return branches;
}


void QuadHandler::separate(ConstSolutionPtr, NodePtr , RelaxationPtr ,
                             CutManager *, SolutionPoolPtr , bool *,
                             SeparationStatus *)
{

}



bool QuadHandler::presolveNode(RelaxationPtr rel, NodePtr, SolutionPoolPtr,
                               ModVector &p_mods, ModVector &r_mods)
{
  // visit each quadratic constraint and see if bounds can be improved.

  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    propSqrBnds_(it, rel, p_mods, r_mods);
  }
  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    propBilBnds_(it, rel, p_mods, r_mods);
  }

  return false;
}


bool QuadHandler::propSqrBnds_(LinSqrMapIter lx2, RelaxationPtr rel, 
                               ModVector &p_mods, ModVector &r_mods)
{
  double lb1, ub1, r;
  VarBoundMod2Ptr b2mod;
  VarBoundModPtr bmod;
  LinearFunctionPtr lf;
  LinConModPtr lmod;

  VariablePtr x0   = lx2->first;      // x0 and y are variables in p_
  VariablePtr y    = lx2->second->y;
  double lb = x0->getLb();
  double ub = x0->getUb();
  ConstraintPtr cons = lx2->second->OECon;

  bool uprel = true;

  // If rel exists
  // bound changes go into p_ and rel
  // Linearization changes go into rel
  // otherwise
  // only bound changes go into p_ 

  BoundsOnSquare(x0, lb1, ub1);
  if (lb1>y->getLb()+eTol_ && ub1<y->getUb()-eTol_) {
    // bounds on y and also the secant approximation can be updated.
    b2mod  = (VarBoundMod2Ptr) new VarBoundMod2(y, lb1, ub1);
    b2mod->applyToProblem(p_);
    p_mods.push_back(b2mod);

    if (uprel) {
      lf = getNewSqLf_(x0, y, lb, ub, r);
      lmod = (LinConModPtr) new LinConMod(cons, lf, -INFINITY, r);
      lmod->applyToProblem(rel);
    }
  } else if (lb1>y->getLb()+eTol_) {
    bmod  = (VarBoundModPtr) new VarBoundMod(y, Lower, lb1);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
    //lf   = getNewSecantLf_(x0, y, lb, ub, r);
    lmod = (LinConModPtr) new LinConMod(cons, lf, -INFINITY, r);
    lmod->applyToProblem(rel);
  } else if (ub1<y->getUb()-eTol_) {
    bmod  = (VarBoundModPtr) new VarBoundMod(y, Upper, ub1);
    bmod->applyToProblem(rel);
    r_mods.push_back(bmod);
    //lf   = getNewSecantLf_(x0, y, lb, ub, r);
    lmod = (LinConModPtr) new LinConMod(cons, lf, -INFINITY, r);
    lmod->applyToProblem(rel);
  } else {
    lf = cons->getLinearFunction();
    if ((cons->getUb()+lb*ub)>1e-8 || 
        (fabs(lf->getWeight(x0)+(lb+ub)) > 1e-8)) {
      // bounds are up to date but the constraint needs update.
      //lf   = getNewSecantLf_(x0, y, lb, ub, r);
      lmod = (LinConModPtr) new LinConMod(cons, lf, -INFINITY, r);
      lmod->applyToProblem(rel);
    }
  }
  return false;
}


bool QuadHandler::propBilBnds_(LinBilSetIter lx0x1, RelaxationPtr , 
                               ModVector &p_mods, ModVector &)
{
  VariablePtr x0 = (*lx0x1)->getX0();
  VariablePtr x1 = (*lx0x1)->getX1();
  VariablePtr  y = (*lx0x1)->getY();
  double lb, ub;
  VarBoundMod2Ptr b2mod;
  VarBoundModPtr bmod;

  BoundsOnProduct(x0, x1, lb, ub);
  assert(!"modify relaxation also");
  if (lb>y->getLb()+eTol_ && ub<y->getUb()-eTol_) {
    b2mod  = (VarBoundMod2Ptr) new VarBoundMod2(y, lb, ub);
    p_mods.push_back(b2mod);
    b2mod->applyToProblem(p_);

  } else if (lb>y->getLb()+eTol_) {
    bmod  = (VarBoundModPtr) new VarBoundMod(y, Lower, lb);
    p_mods.push_back(bmod);
    bmod->applyToProblem(p_);

  } else if (ub<y->getUb()-eTol_) {
    bmod  = (VarBoundModPtr) new VarBoundMod(y, Upper, ub);
    p_mods.push_back(bmod);
    bmod->applyToProblem(p_);

  } 
  return false;
}


SolveStatus QuadHandler::presolve(PreModQ *, bool *)
{
  return Finished; // disabled for now.
  removeFixed_();
  binToLin_();
  return Finished;
}


void QuadHandler::removeFixed_()
{
  ObjectivePtr oPtr = p_->getObjective();
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;
  ConstraintConstIterator c_iter;
  Double c;

  if (oPtr && oPtr->getFunctionType() == Quadratic) {
    c = 0.;
    f = oPtr->getFunction();
    removeFixedFun_(f, lf, &c);
    if (lf->getNumTerms()>0) {
      lf = (LinearFunctionPtr) new LinearFunction();
    }
    if (fabs(c)>eTol_) {
      p_->addToObj(c);
    }
  }

  for (c_iter=p_->consBegin(); c_iter!=p_->consEnd(); ++c_iter) {
    f = (*c_iter)->getFunction();
    c = 0.;
    if (f->getType()==Quadratic) {
      removeFixedFun_(f, lf, &c);
      if (lf->getNumTerms()>0) {
        lf = (LinearFunctionPtr) new LinearFunction();
      }
      if (fabs(c)>eTol_) {
        p_->addToCons(*c_iter, c);
      }
    }
  }
}


void QuadHandler::removeFixedFun_(FunctionPtr f, LinearFunctionPtr lf2, 
    Double *c)
{
  QuadraticFunctionPtr qf,qf2;
  LinearFunctionPtr lf;
  bool new_lf = false;
  ConstVariablePtr v1, v2;
  Double l1, l2, u1, u2;

  qf = f->getQuadraticFunction();
  lf = f->getLinearFunction();
  if (!lf) {
    new_lf = true;
    lf = lf2;
  }
  qf2 = (QuadraticFunctionPtr) new QuadraticFunction();
  *c  = 0;
  for (VariablePairGroupConstIterator it=qf->begin(); it!=qf->end(); 
      ++it) {
    v1 = it->first.first;
    v2 = it->first.second;
    l1 = v1->getLb();
    l2 = v2->getLb();
    u1 = v1->getUb();
    u2 = v2->getUb();
    if (u1-l1 < eTol_ && u2-l2 < eTol_) {
      // remove
      *c += l1*l2*it->second;
      qf2->addTerm(v1, v2, -1.*(it->second));
    } else if (u1-l1 < eTol_) {
      // remove and add linear term
      lf->incTerm(v2, l1*(it->second));
      qf2->addTerm(v1, v2, -1.*(it->second));
    } else if (u2-l2 < eTol_) {
      // remove and add linear term
      lf->incTerm(v1, l2*(it->second));
      qf2->addTerm(v1, v2, -1.*(it->second));
    }
  }
  (*qf) += qf2;
  if (new_lf==true && lf->getNumTerms()>0) {
    f->add(lf); // lf is cloned and added.
  }
}


void QuadHandler::binToLin_()
{
  if (p_->getSize()->bins > 0) {
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    FunctionPtr f;
    ConstraintConstIterator c_iter;
    ObjectivePtr oPtr = p_->getObjective();

    if (oPtr && oPtr->getFunctionType() == Quadratic) {
      f = oPtr->getFunction();
      if (f->getType()==Quadratic) {
        binToLinFun_(f, lf);
        if (lf->getNumTerms()>0) {
          lf = (LinearFunctionPtr) new LinearFunction();
        }
      }
      if (lf->getNumTerms()>0) {
        lf = (LinearFunctionPtr) new LinearFunction();
      }
    }
    for (c_iter=p_->consBegin(); c_iter!=p_->consEnd(); ++c_iter) {
      f = (*c_iter)->getFunction();
      if (f->getType()==Quadratic) {
        binToLinFun_(f, lf);
        if (lf->getNumTerms()>0) {
          lf = (LinearFunctionPtr) new LinearFunction();
        }
      }
    }
  }
}


void QuadHandler::binToLinFun_(FunctionPtr f, LinearFunctionPtr lf2)
{
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;
  bool new_lf = false;

  qf = f->getQuadraticFunction();
  lf = f->getLinearFunction();
  if (!lf) {
    new_lf = true;
    lf = lf2;
  }
  for (VariablePairGroupConstIterator it=qf->begin(); it!=qf->end(); 
      ++it) {
    if (it->first.first == it->first.second && it->first.first->getType() == 
        Binary) {
      lf->incTerm(it->first.first, 1*(it->second));
      qf->incTerm(it->first.first, it->first.first, -1.*(it->second));
      assert(!"bug above!");
    }
  }
  if (new_lf==true && lf->getNumTerms()>0) {
    f->add(lf); // lf is cloned and added.
  }
}


std::string QuadHandler::getName() const
{
   return "QuadHandler (Handling quadratic terms of the form y=x1*x2).";
}



#if 0
VariablePtr QuadHandler::addLinBil_(VariablePtr x0, VariablePtr x1,  
    RelaxationPtr rel)
{
  // add all four LinBil inequalities
  VariablePtr y = addLinBilLower_(x0, x1, rel);
  y = addLinBilUpper_(x0, x1, rel);
  return y;
}


VariablePtr QuadHandler::addLinBilUpper_(VariablePtr x0, VariablePtr x1,
    RelaxationPtr rel)
{
  // add LinBilmick inequalities for \f$  y \leq x0.x1 \f$.
  LinBilPtr mcc = (LinBilPtr) new LinBil(x0, x1, LinBil::GT);
  LinBilSetIter biter = x0x1Funs_.find(mcc);
  Double lb = 0, ub = 0;
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr cons;
  VariablePtr y = VariablePtr();   // NULL
  bool exists = true;

  if (biter==x0x1Funs_.end()) {
    exists = false;
  }

  if (exists) {
    mcc = (*biter);
    y = mcc->getAux();
  } else {
    // add a new bilinear term in our list
    BoundsOnProduct(x0, x1, lb, ub);
    y = rel->newVariable(lb, ub, Continuous);
  }

  assert(y);
  if (!exists || mcc->getSense() == LinBil::LT) {
    // we don't already have constraints for x0x1 <= y. Add them
    // y <= u1x0 + l0x1 - l0u1
    lf = getMcLf_(x0, x0->getLb(), x0->getUb(), x1, x1->getLb(), 
        x1->getUb(), y, ub, 2);
    f = (FunctionPtr) new Function(lf);
    cons = rel->newConstraint(f, -INFINITY, ub);
    mcc->setC2(cons);

    // y <= l1x0 + u0x1 - u0l1
    lf = getMcLf_(x0, x0->getLb(), x0->getUb(), x1, x1->getLb(), 
        x1->getUb(), y, ub, 3);
    f = (FunctionPtr) new Function(lf);
    cons = rel->newConstraint(f, -INFINITY, ub);
    mcc->setC3(cons);

    if (exists) {
      // we have both kinds of inequalities, so change the sense.
      mcc->setSense(LinBil::EQ);
    } else {
      mcc->setAux(y);
      mcc->setSense(LinBil::GT);
      // add it to the set.
      x0x1Funs_.insert(mcc);
    }
  } else {
    // no need to add any new variables or new constraints.
    y = mcc->getAux();
  }
  return y;
}


VariablePtr QuadHandler::addLinBilLower_(VariablePtr x0, VariablePtr x1, 
    RelaxationPtr rel)
{
  // add two LinBil inequalities for \f$ y \geq x0.x1 \f$.
  LinBilPtr mcc = (LinBilPtr) new LinBil(x0, x1, LinBil::LT);
  LinBilSetIter biter = x0x1Funs_.find(mcc);
  Double lb = 0, ub = 0;
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr cons;
  VariablePtr y = VariablePtr();   // NULL
  bool exists = true;

  if (biter==x0x1Funs_.end()) {
    exists = false;
  }

  if (exists) {
    mcc = *(biter);
    y = mcc->getAux();
  } else {
    // add a new bilinear term in our list
    BoundsOnProduct(x0, x1, lb, ub);
    y = rel->newVariable(lb, ub, Continuous);
  }

  assert(y);
  if (!exists || mcc->getSense() == LinBil::GT) {
    // we don't already have constraints for x0x1 <= y. Add them
    // y >= l0x1 + l1x0 - l1l0
    lf = getMcLf_(x0, x0->getLb(), x0->getUb(), x1, x1->getLb(), x1->getUb(), 
        y, ub, 0);
    f = (FunctionPtr) new Function(lf);
    cons = rel->newConstraint(f, -INFINITY, ub);
    mcc->setC0(cons);

    // y >= u0x1 + u1x0 - u1u0
    lf = getMcLf_(x0, x0->getLb(), x0->getUb(), x1, x1->getLb(), x1->getUb(), 
        y, ub, 1);
    f = (FunctionPtr) new Function(lf);
    cons = rel->newConstraint(f, -INFINITY, ub);
    mcc->setC1(cons);

    if (exists) {
      // we have both kinds of inequalities, so change the sense.
      mcc->setSense(LinBil::EQ);
    } else {
      mcc->setAux(y);
      mcc->setSense(LinBil::LT);
      // add it to the set.
      x0x1Funs_.insert(mcc);
    }
  } else {
    // no need to add any new variables or new constraints.
    y = mcc->getAux();
  }
  return y;
}
#endif




// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

LinBil::LinBil(VariablePtr x0, VariablePtr x1, VariablePtr y)
 : y_(y)
{
  if (x0->getIndex()>x1->getIndex()) {
    x0_ = x1;
    x1_ = x0;
  } else {
    x0_ = x0;
    x1_ = x1;
  }
  c0_ = c1_ = c2_ = c3_ = ConstraintPtr(); // NULL
}


LinBil::~LinBil() 
{
  x0_.reset();
  x1_.reset();
  y_.reset();
  c0_.reset();
  c1_.reset();
  c2_.reset();
  c3_.reset();
}


bool Minotaur::CompareLinBil::operator()(LinBilPtr b0, LinBilPtr b1) const
{
  UInt b0x0 = b0->getX0()->getId();
  UInt b0x1 = b0->getX1()->getId();

  UInt b1x0 = b1->getX0()->getId();
  UInt b1x1 = b1->getX1()->getId();

  if (b0x0 == b1x0) {
    return (b0x1 < b1x1);
  }
  return (b0x0 < b1x0);
}


bool LinBil::isViolated(const double *x, const double &tol) const
{

  double xval = x[x0_->getIndex()] * x[x1_->getIndex()];
  double yval = x[y_->getIndex()];
  if (fabs(xval - yval) > tol) {
    return true;
  }
  return false;
}


bool LinBil::isViolated(const double x0val, const double x1val, 
                        const double yval, const double tol) const
{
  double xval = x1val*x0val;
  if (fabs(xval - yval) > tol) {
    return true;
  }
  return false;
}


VariablePtr LinBil::getOtherX(ConstVariablePtr x) const
{
  if (x0_==x) {
   return x1_;
  } else if (x1_==x) {
    return x0_;
  } else {
    VariablePtr v = VariablePtr(); // NULL
    return v;
  }
}


void LinBil::setCons(ConstraintPtr c0, ConstraintPtr c1, ConstraintPtr c2,
                     ConstraintPtr c3)
{
  c0_ = c0;
  c1_ = c1;
  c2_ = c2;
  c3_ = c3;
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
