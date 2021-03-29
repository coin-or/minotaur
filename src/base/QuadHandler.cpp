// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2017 The MINOTAUR Team.
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
#include <algorithm>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrVarCand.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "LinBil.h"
#include "LinearFunction.h"
#include "LinMods.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "QuadHandler.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"

// #define SPEW 1
// TODO:
// * arrange alphabetically
// * Move other classes to other files
// * remove rel->getRelaxationVar(). By default, these variables must be saved
//   in the data-structures.

using namespace Minotaur;

const std::string QuadHandler::me_ = "QuadHandler: ";

QuadHandler::QuadHandler(EnvPtr env, ProblemPtr problem)
: aTol_(1e-5),
  bTol_(1e-8),
  env_(env),
  lpe_(0),
  rTol_(1e-4)
{
  p_ = problem; 
  modProb_ = false;
  modRel_  = true;
  logger_  = env->getLogger();
  resetStats_();
  timer_ = env->getTimer();
  defaultLb_ = 1e12;
  defaultUb_ = -1e12;
}

QuadHandler::QuadHandler(EnvPtr env, ProblemPtr problem, EnginePtr lpe)
: aTol_(1e-5),
  bTol_(1e-8),
  env_(env),
  lpe_(lpe),
  rTol_(1e-4)
{
  p_ = problem; 
  modProb_ = false;
  modRel_  = true;
  logger_  = env->getLogger();
  resetStats_();
  timer_ = env->getTimer();
  defaultLb_ = 1e12;
  defaultUb_ = -1e12;
}

QuadHandler::~QuadHandler()
{
  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    delete it->second;
  }
  x2Funs_.clear();

  for (LinBilSetIter it=x0x1Funs_.begin(); it!=x0x1Funs_.end(); ++it) {
    delete *it;
  }
  x0x1Funs_.clear();
  bStats_.qvars.clear();
}


void QuadHandler::addConstraint(ConstraintPtr newcon)
{
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf; 
  NonlinearFunctionPtr nlf;
  VariablePtr y, x0, x1;
  LinSqrPtr lx2;
  LinBil* linbil;

  cons_.push_back(newcon);
  qf = newcon->getQuadraticFunction();

  if (qf) {
    lf = newcon->getLinearFunction();
    
    assert(lf && qf);
    assert(1==lf->getNumTerms());
    assert(1==qf->getNumTerms());
    y = lf->termsBegin()->first;
    if (qf->begin()->first.first->getId()==qf->begin()->first.second->getId())
    {
      x0 = qf->begin()->first.first;
      lx2 = new LinSqr();
      lx2->y = y;
      lx2->x = x0;
      lx2->oeCon = ConstraintPtr();
      x2Funs_.insert(std::pair<VariablePtr, LinSqrPtr>(x0, lx2));
    } else {
      x0 = qf->begin()->first.first;
      x1 = qf->begin()->first.second;
      linbil = new LinBil(x0, x1, y);
      x0x1Funs_.insert(linbil);
    }
  } else {
    nlf = newcon->getNonlinearFunction();
    lf  = newcon->getLinearFunction();

    assert(lf && nlf);
    assert(1==lf->getNumTerms());
    assert(nlf->numVars()<3);

    y = lf->termsBegin()->first;
    if (nlf->numVars()==1) {
      x0 = *(nlf->varsBegin());
      lx2        = new LinSqr();
      lx2->y     = y;
      lx2->x     = x0;
      lx2->oeCon = ConstraintPtr(); // NULL
      x2Funs_.insert(std::pair<VariablePtr, LinSqrPtr>(x0, lx2));
    } else {
      VariableSet::iterator it = nlf->varsBegin();
      x0 = *it;
      ++it;
      x1 = *it;
      linbil = new LinBil(x0, x1, y);
      x0x1Funs_.insert(linbil);
    }
  }
}

double QuadHandler::addDefaultBounds_(VariablePtr v, BoundType lu) {
  if (lu == Lower) {
    if (defaultLb_ < 1e12) {
      v->setLb_(defaultLb_);
    } else {
      for (VariableConstIterator it=p_->varsBegin(); it!=p_->varsEnd(); ++it) {
        if ((*it)->getLb() < defaultLb_ && (*it)->getLb() > -1e12) {
          defaultLb_ = (*it)->getLb();
        }
      }
      if (defaultLb_ < -aTol_ && defaultLb_ > -1e12) {
        defaultLb_ = 100*defaultLb_;
      } else if (defaultLb_ > aTol_) {
        defaultLb_ = -100*defaultLb_;
      } else {
        defaultLb_ = -1000;
      }
      v->setLb_(defaultLb_);
    }
    logger_->msgStream(LogDebug) << me_
                                 << "WARNING: Adding Default lower bound for "
                                 << v->getName() << "Lower bound value = "
                                 << defaultLb_ << std::endl;
    ++bStats_.dlb;
    return defaultLb_;
  } else {
    if (defaultUb_ > -1e12) {
      v->setUb_(defaultUb_);
    } else {
      for (VariableConstIterator it=p_->varsBegin(); it!=p_->varsEnd(); ++it) {
        if ((*it)->getUb() > defaultUb_ && (*it)->getUb() < 1e12) {
          defaultUb_ = (*it)->getUb();
        }
      }
      if (defaultUb_ < -aTol_) {
        defaultUb_ = -100*defaultUb_;
      } else if (defaultUb_ > aTol_ && defaultUb_ < 1e12) {
        defaultUb_ = 100*defaultUb_;
      } else {
        defaultUb_ = 1000;
      }
      v->setUb_(defaultUb_);
    }
    logger_->msgStream(LogDebug) << me_
                                 << "WARNING: Adding Default upper bound for "
                                 << v->getName() << "Upper bound value = "
                                 << defaultUb_ << std::endl;
    ++bStats_.dub;
    return defaultUb_;
  }
}

void QuadHandler::findLinPt_(double xval, double yval, double &xl,
                             double &yl)
{
  // The point (xval, yval) satisfies yval < xval^2.
  // We want to find a new point (xl, yl) on the curve y=x^2 so that 
  // the gradient inequality at (xl, yl) cuts off (xval, yval).
  //
  // We will find a point (xl, yl) that is on the parabola y=x^2 and is
  // nearest to (xval, yval), i.e., xl solves
  // min_x (x - xval)^2  + (x^2 - yval)^2.
  // Solving it analytically is tedious as it involves solving a depressed
  // cubic equation. It may require using complex numbers.
  //
  // We merely find the solution using golden section search.
  
  double a, b, mu, la;
  double mu_val, la_val;
  const double alfa = 0.618;
  const double errlim = 1e-4; // don't want too much accuracy.

  if (xval>0) {
    a = sqrt(yval);
    b = xval;
  } else {
    a = xval;
    b = -sqrt(yval);
  }

  mu = a + alfa*(b-a);
  la = b - alfa*(b-a);
  mu_val = (mu-xval)*(mu-xval) + (mu*mu-yval)*(mu*mu-yval);
  la_val = (la-xval)*(la-xval) + (la*la-yval)*(la*la-yval);
  while ( (b-a) > errlim) {
    if (mu_val < la_val) {
      a = la;
      la = mu;
      la_val = mu_val;
      mu = a + alfa*(b-a);
      mu_val = (mu-xval)*(mu-xval) + (mu*mu-yval)*(mu*mu-yval);
    } else {
      b = mu;
      mu = la;
      mu_val = la_val;
      la = b - alfa*(b-a);
      la_val = (la-xval)*(la-xval) + (la*la-yval)*(la*la-yval);
    }
  }
  xl = la;
  yl = la*la;
}


Branches QuadHandler::getBranches(BrCandPtr cand, DoubleVector &x,
                                  RelaxationPtr rel, SolutionPoolPtr)
{
  //BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  BrVarCandPtr vcand =dynamic_cast <BrVarCand*> (cand);
  VariablePtr v = vcand->getVar();
  VariablePtr v2 = 0;
  double value = x[v->getIndex()];
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
  vcand->setNumBranches(2);

#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "branching on " << v->getName()
                                       << " <= " << value << " or " 
                                       << " >= " << value << std::endl;
#endif
  return branches;
}


void QuadHandler::getBranchingCandidates(RelaxationPtr rel,
                                         const DoubleVector &x, ModVector &,
                                         BrVarCandSet &cands,
                                         BrCandVector &, bool &is_inf)
{
  double yval, x0val, x1val, udist, ddist;
  BrVarCandPtr br_can;
  VariablePtr x0, x1;
  std::pair<BrVarCandIter, bool> ret;
  bool check;

  is_inf = false;

  // First check if there is a candidate x0 that violates y <= x0^2.
  for (LinSqrMapIter s_it=x2Funs_.begin(); s_it!=x2Funs_.end(); ++s_it) {
    x0 = rel->getRelaxationVar(s_it->first);
    x0val = x[x0->getIndex()];
    x1 = rel->getRelaxationVar(s_it->second->y);
    yval = x[x1->getIndex()];
    if (yval - x0val*x0val > fabs(yval)*rTol_ && yval - x0val*x0val > aTol_) {
#if SPEW
      logger_->msgStream(LogDebug2) << std::setprecision(9) << me_ 
        << "branching candidate for x^2: " << s_it->first->getName()
        << " value = " << x0val << " aux var: " 
        << s_it->second->y->getName() 
        << " value = " << yval << std::endl;
#endif
      ddist = (yval - x0val*x0val)/
             sqrt(1.0+(x0->getLb()+x0val)*(x0->getLb()+x0val));
      udist = (yval - x0val*x0val)/
             sqrt(1.0+(x0->getUb()+x0val)*(x0->getUb()+x0val));
      br_can = (BrVarCandPtr) new BrVarCand(x0, x0->getIndex(), ddist,
                                            udist);
      ret = cands.insert(br_can);
      if (false == ret.second) { // already exists.
        br_can = *(ret.first);
        br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getDDist());
      }
    }
  }

  // Now check if there is a violated constraint of the form y = x0.x1.
  // If so, add both x0 and x1 to the candidate set.
  for (LinBilSetIter s_it=x0x1Funs_.begin(); s_it!=x0x1Funs_.end(); ++s_it) {
    LinBil* bil = (*s_it);
    x0 = rel->getRelaxationVar(bil->getX0());
    x1 = rel->getRelaxationVar(bil->getX1());
    x0val = x[x0->getIndex()];
    x1val = x[x1->getIndex()];
    yval  = x[bil->getY()->getIndex()];
    if (bil->isViolated(x0val, x1val, yval)) {
      check = false;
      // If a variable is at bounds, then it is not a candidate.
      if (false==isAtBnds_(x0, x0val)) {
        check = true;
        if (x0val*x1val > yval) {
          ddist = (-yval + x0val*x1val)/sqrt(1.0+x0val*x0val+
                                            x1->getUb()*x1->getUb());
          udist = (-yval + x0val*x1val)/sqrt(1.0+x0val*x0val+
                                            x1->getLb()*x1->getLb());
        } else {
          ddist = (yval - x0val*x1val)/sqrt(1.0+x0val*x0val+
                                            x1->getLb()*x1->getLb());
          udist = (yval - x0val*x1val)/sqrt(1.0+x0val*x0val+
                                            x1->getUb()*x1->getUb());
        }
        br_can = (BrVarCandPtr) new BrVarCand(x0, x0->getIndex(), ddist,
                                              udist); 
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getUDist());
        }
      }

      if (false==isAtBnds_(x1, x1val)) {
        check = true;
        if (x0val*x1val > yval) {
          ddist = (-yval + x1val*x0val)/sqrt(1.0+x1val*x1val+
                                            x0->getUb()*x0->getUb());
          udist = (-yval + x1val*x0val)/sqrt(1.0+x1val*x1val+
                                            x0->getLb()*x0->getLb());
        } else {
          ddist = (yval - x1val*x0val)/sqrt(1.0+x1val*x1val+
                                            x0->getLb()*x0->getLb());
          udist = (yval - x1val*x0val)/sqrt(1.0+x1val*x1val+
                                            x0->getUb()*x0->getUb());
        }
        br_can = (BrVarCandPtr) new BrVarCand(x1, x1->getIndex(), ddist,
                                              udist); 
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getUDist());
        }
#if SPEW
      logger_->msgStream(LogDebug2) << std::setprecision(9) << me_ 
        << "branching candidate for x0x1: " << x0->getName()
        << " = " << x0val << " " << x1->getName() << " = " << x1val 
        << " " << bil->getY()->getName() << " = " << yval << " vio = " 
        << fabs(x0val*x1val-yval) << std::endl;
#endif
      }
      if (false==check) {
        int err;
        logger_->msgStream(LogError) << std::setprecision(15) << me_ 
         << "both variables are at bounds, but we still want to branch on "
         << "a bilinear constraint. " << x0->getName() 
         << " = " << x0val << " " << x1->getName() 
         << " = " << x1val << " " << bil->getY()->getName() 
         << " = " << yval  << " product = " << x0val*x1val << std::endl;
        logger_->msgStream(LogDebug) << std::endl
                                     << bil->getC0()->getActivity(&(x[0]),
                                                                  &err) << " ";
        bil->getC0()->write(logger_->msgStream(LogDebug)); 

        logger_->msgStream(LogDebug) << std::endl
                                     << bil->getC1()->getActivity(&(x[0]),
                                                                  &err) << " " ;
        bil->getC1()->write(logger_->msgStream(LogDebug));
        logger_->msgStream(LogDebug) << std::endl
                                     << bil->getC2()->getActivity(&(x[0]),
                                                                  &err) << " " ;
        bil->getC2()->write(logger_->msgStream(LogDebug));
        logger_->msgStream(LogDebug) << std::endl
                                     << bil->getC3()->getActivity(&(x[0]),
                                                                  &err) << " " ;
        bil->getC3()->write(logger_->msgStream(LogDebug));
        logger_->msgStream(LogDebug) << std::endl;
        x0->write(logger_->msgStream(LogDebug));
        x1->write(logger_->msgStream(LogDebug));
        bil->getY()->write(logger_->msgStream(LogDebug));
      }
    }
  }
}


ModificationPtr QuadHandler::getBrMod(BrCandPtr cand, DoubleVector &x, 
                                      RelaxationPtr rel, BranchDirection dir)
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  LinearFunctionPtr lf;
  LinConModPtr lmod;
  //BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  BrVarCandPtr vcand = dynamic_cast <BrVarCand*> (cand);
  VariablePtr v = vcand->getVar();
  VariablePtr x0, x1, y;
  double x0val, yval, vio, rhs;
  UInt vind = v->getIndex();
  bool found = false;
 
  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    if (vind==it->first->getIndex()) {
      x0val  = x[it->first->getIndex()];
      yval = x[it->second->y->getIndex()];
      vio = x0val*x0val-yval;
      if (vio > aTol_ && vio > fabs(yval)*rTol_) {
        x0 = rel->getRelaxationVar(it->first);
        y = rel->getRelaxationVar(it->second->y);
        if (dir==DownBranch) {
          lf = getNewSqLf_(x0, y, x0->getLb(), x0val, rhs);
        } else {
          lf = getNewSqLf_(x0, y, x0val, x0->getUb(), rhs);
        }
        lmod = (LinConModPtr) new LinConMod(it->second->oeCon, lf,
                                            -INFINITY, rhs);
        lmods->insert(lmod);
      }
    }
  }
  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    if (vind==(*it)->getX0()->getIndex()) {
      if ((*it)->isViolated(&(x[0]), vio)) {
        x0 = rel->getRelaxationVar((*it)->getX0());
        x1 = rel->getRelaxationVar((*it)->getX1());
        found = true;
      }
    } else if (vind==(*it)->getX1()->getIndex()) {
      if ((*it)->isViolated(&(x[0]), vio)) {
        x0 = rel->getRelaxationVar((*it)->getX1());
        x1 = rel->getRelaxationVar((*it)->getX0());
        found = true;
      }
    }
    if (true==found) {
      y = rel->getRelaxationVar((*it)->getY());
      x0val  = x[x0->getIndex()];
      yval   = x[y->getIndex()];
      if (dir==DownBranch) {
        lf = getNewBilLf_(x0, x0->getLb(), x0val,
                          x1, x1->getLb(), x1->getUb(), y, 1, rhs);
        lmod = (LinConModPtr) new LinConMod((*it)->getC1(), lf, -INFINITY, rhs);
        lmods->insert(lmod);
        lf = getNewBilLf_(x0, x0->getLb(), x0val,
                          x1, x1->getLb(), x1->getUb(), y, 3, rhs);
        lmod = (LinConModPtr) new LinConMod((*it)->getC3(), lf, -INFINITY, rhs);
        lmods->insert(lmod);
      } else {
        lf = getNewBilLf_(x0, x0val, x0->getUb(),
                          x1, x1->getLb(), x1->getUb(), y, 0, rhs);
        lmod = (LinConModPtr) new LinConMod((*it)->getC0(), lf, -INFINITY, rhs);
        lmods->insert(lmod);
        lf = getNewBilLf_(x0, x0val, x0->getUb(),
                          x1, x1->getLb(), x1->getUb(), y, 2, rhs);
        lmod = (LinConModPtr) new LinConMod((*it)->getC2(), lf, -INFINITY, rhs);
        lmods->insert(lmod);
      }
      found = false;
    }
  }
  return lmods;
}


std::string QuadHandler::getName() const
{
  return "QuadHandler (Handling quadratic terms of the form y=x1*x2).";
}


LinearFunctionPtr QuadHandler::getNewBilLf_(VariablePtr x0, double lb0,
                                            double ub0, VariablePtr x1,
                                            double lb1, double ub1,
                                            VariablePtr y, int type,
                                            double &rhs)
{
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

  if (lb0 < -1e12 || lb1 < -1e12 || ub0 > 1e12 || ub1 > 1e12) {
    if (lb0 < -1e12) {
      lb0 = addDefaultBounds_(x0, Lower);
    }
    if (lb1 < -1e12) {
      lb1 = addDefaultBounds_(x1, Lower);
    }
    if (ub0 > 1e12) {
      ub0 = addDefaultBounds_(x0, Upper);
    }
    if (ub1 > 1e12) {
      ub1 = addDefaultBounds_(x1, Upper);
    }
//    logger_->errStream() << "can not relax " << x0->getName() << "*"
//                         << x1->getName() << " = " << y->getName() 
//                         << "because the bounds on variables are too weak"
//                         << std::endl;
//    exit(500);
  }

  switch (type) {
  case(0):
    // y >= l0x1 + l1x0 - l1l0
    assert(lb0 > -1e21 && lb1 > -1e21);
    lf->addTerm(x1, lb0);
    lf->addTerm(x0, lb1);
    lf->addTerm(y, -1.);
    rhs = lb0*lb1;
    break;
  case(1):
    // y >= u0x1 + u1x0 - u1u0
    assert(ub0 < 1e21 && ub1 < 1e21);
    lf->addTerm(x1, ub0);
    lf->addTerm(x0, ub1);
    lf->addTerm(y, -1.);
    rhs = ub0*ub1;
    break;
  case(2):
    // y <= u1x0 + l0x1 - l0u1
    assert(lb0 > -1e21 && ub1 < 1e21);
    lf->addTerm(x0, -1.0*ub1);
    lf->addTerm(x1, -1.0*lb0);
    lf->addTerm(y, 1.);
    rhs = -lb0*ub1;
    break;
  case(3):
    // y <= l1x0 + u0x1 - u0l1
    assert(ub0 < 1e21 && lb1 > -1e21);
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


LinearFunctionPtr QuadHandler::getNewSqLf_(VariablePtr x, VariablePtr y,
                                           double lb, double ub, double & r)
{
  LinearFunctionPtr lf = LinearFunctionPtr(); // NULL
  double new_lb = lb;
  double new_ub = ub;
  double eps = 1e-5; // we do not want a coefficient smaller than this in our LP.

  if (lb < -1e12 || ub > 1e12) {
    if (lb < -1e12) {
      new_lb = addDefaultBounds_(x, Lower);
    }
    if (ub > 1e12) {
      new_ub = addDefaultBounds_(x, Upper);
    }
//    logger_->errStream() << "can not relax " << y->getName() << "="
//                         << x->getName() << "^2, "
//                         << "because the bounds on latter are too weak"
//                         << std::endl;
//    exit(500);
  }
  r = -new_ub*new_lb;
  if (fabs(new_ub+new_lb) > eps) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.);
    lf->addTerm(x, -1.*(new_ub+new_lb));
  } else {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.);
#if SPEW
    logger_->msgStream(LogDebug) << me_ << 
      "warning: generating a bound as a secant constraint." << std::endl;
#endif
  }
  return lf;
}


void QuadHandler::addCut_(VariablePtr x, VariablePtr y, 
                          double xl, double yl, double xval, double yval,
                          RelaxationPtr rel, bool &ifcuts)
{
  // add the cut 2*xl*x - y - yl <= 0
  ifcuts = false;
  if (2*xl*xval - yval - yl > 1e-5 &&
      2*xl*xval - yval > yl*(1+1e-4)) {
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    FunctionPtr f;

    lf->addTerm(x, 2*xl);
    lf->addTerm(y, -1.0);
    f = (FunctionPtr) new Function(lf);
    ifcuts = true;
    ++sStats_.cuts;
#if SPEW
    {
    ConstraintPtr c = rel->newConstraint(f, -INFINITY, xl*xl);
    logger_->msgStream(LogDebug2) << me_ << "new cut added" << std::endl;
    c->write(logger_->msgStream(LogDebug2));
    }
#else
    rel->newConstraint(f, -INFINITY, xl*xl);
#endif
  } else {
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "Not adding cut because of "
                                  << "insufficient violation "
                                  << 2*xl*xval - yval - xl*xl << std::endl;
#endif
  }
}

void QuadHandler::coeffImprov_() {
  ConstraintPtr c;
  double min_coeff = INFINITY;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  double eps = 1e-3; //don't want coefficient smaller than this

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                               ++cit) {
    c = *cit;
    lf = c->getFunction()->getLinearFunction();
    qf = c->getFunction()->getQuadraticFunction();
    if (lf) {
      for (VariableGroupConstIterator lit = lf->termsBegin();
           lit != lf->termsEnd(); ++lit) {
        if (fabs(lit->second) < min_coeff) {
          min_coeff = fabs(lit->second);
        }
      }
    }

    if (qf) {
      for (VariablePairGroupConstIterator qit = qf->begin();
           qit != qf->end(); ++qit) {
        if (fabs(qit->second) < min_coeff) {
          min_coeff = fabs(qit->second);
        }
      } 
    }

    if (min_coeff < eps) {
      if (lf) {
        lf->multiply(1/min_coeff);
      }

      if (qf) {
        qf->multiply(1/min_coeff);
      }

      if (c->getLb() > -INFINITY) {
        p_->changeBound(c, Lower, c->getLb()/min_coeff);
      }
      if (c->getUb() < INFINITY) {
        p_->changeBound(c, Upper, c->getUb()/min_coeff);
      }
    }
  }
}

void QuadHandler::calcRangeOfQuadVars_() {
  double tot_range = 0, tot_sqr_range = 0, range;
  for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
    range = (*vit)->getUb() - (*vit)->getLb();
    tot_range += range;
    tot_sqr_range += range*range;
  }

  bStats_.avg_range = tot_range/bStats_.qvars.size();
  bStats_.body_diag = sqrt(tot_sqr_range);

  tot_sqr_range = 0;
  for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
    range = (*vit)->getUb() - (*vit)->getLb();
    tot_sqr_range += (range - bStats_.avg_range)*(range - bStats_.avg_range);
  }
  bStats_.sd_range = sqrt(tot_sqr_range/bStats_.qvars.size());
}

bool QuadHandler::isAtBnds_(ConstVariablePtr x, double xval)
{
  double lb = x->getLb();
  double ub = x->getUb();
  return(fabs(xval-lb) < bTol_ || fabs(xval-ub) < bTol_);
}


bool QuadHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr , bool &,
                             double &inf_meas)
{
  double yval, xval, vio;
  const double *x = sol->getPrimal();
  bool is_feas = true;

  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    // check if y <= x^2
    xval  = x[it->first->getIndex()];
    yval = x[it->second->y->getIndex()];
    vio = fabs(yval - xval*xval);
    if (vio > fabs(yval)*rTol_ && vio > aTol_) {
      is_feas = false;
      inf_meas += vio;
    }
  }
#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "no branching candidates for y=x^2" 
                                << std::endl;
#endif

  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    if ((*it)->isViolated(x, vio)) {
      is_feas = false;
      inf_meas += vio;
    }
  }

#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "no branching candidates for y=x1x2" 
                                << std::endl;
#endif
  return is_feas;
}


SolveStatus QuadHandler::presolve(PreModQ *, bool *changed)
{

  bool is_inf = false;
  SolveStatus status = Started;
  QuadraticFunctionPtr qf;
  ObjectivePtr obj;
  double stime = timer_->query();

  *changed = false;

  if (cons_.size() > 0) {
    is_inf = varBndsFromCons_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }

  } else {
    //if (bStats_.niters >= 1) {
    //  return Finished;
    //}
    ++bStats_.niters;
    coeffImprov_();
    for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                                 ++cit) {
      if ((*cit)->getFunctionType() == Quadratic ||
          (*cit)->getFunctionType() == Bilinear) {
        qf = (*cit)->getFunction()->getQuadraticFunction();
        if (qf) {
          for (VarIntMapConstIterator qit = qf->varsBegin();
               qit != qf->varsEnd(); ++qit) {
            bStats_.qvars.insert(qit->first);
          }
        }
      }
    }
    obj = p_->getObjective();
    if (obj->getFunctionType() == Quadratic ||
        obj->getFunctionType() == Bilinear) {
      qf = obj->getFunction()->getQuadraticFunction();
      if (qf) {
        for (VarIntMapConstIterator qit = qf->varsBegin();
             qit != qf->varsEnd(); ++qit) {
          bStats_.qvars.insert(qit->first);
        }
      }
    }
    for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
      if ((*vit)->getLb() > -INFINITY) {
        ++bStats_.nqlb;
      }
      if ((*vit)->getUb() < INFINITY) {
        ++bStats_.nqub;
      }
    } 
    //is_inf = tightenSimple_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
    for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
      if ((*vit)->getLb() > -INFINITY) {
        ++bStats_.nqlbs;
      }
      if ((*vit)->getUb() < INFINITY) {
        ++bStats_.nqubs;
      }
    }
    bStats_.nqlbs -= bStats_.nqlb;
    bStats_.nqubs -= bStats_.nqub;
    p_->delMarkedCons();

    //is_inf = tightenQuad_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
    for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
      if ((*vit)->getLb() > -INFINITY) {
        ++bStats_.nqlbq;
      }
      if ((*vit)->getUb() < INFINITY) {
        ++bStats_.nqubq;
      }
    }
    bStats_.nqlbq -= bStats_.nqlb + bStats_.nqlbs;
    bStats_.nqubq -= bStats_.nqub + bStats_.nqubs;
    p_->delMarkedCons();
    bStats_.time = timer_->query()-stime;
  }

  if (Started==status) {
    status = Finished;
  }
  return status;
}


bool QuadHandler::presolveNode(RelaxationPtr rel, NodePtr, SolutionPoolPtr,
                               ModVector &p_mods, ModVector &r_mods)
{
  bool lchanged = true;
  bool changed = true;
  bool is_inf = false;
  double stime = timer_->query();
  QuadraticFunctionPtr qf;
  ObjectivePtr obj;
  bool lpchanged = false;

  // visit each quadratic constraint and see if bounds can be improved.
  if (bStats_.niters < 1) {
    ++bStats_.niters;
    for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                                 ++cit) {
      if ((*cit)->getFunctionType() == Quadratic ||
          (*cit)->getFunctionType() == Bilinear) {
        qf = (*cit)->getFunction()->getQuadraticFunction();
        if (qf) {
          for (VarIntMapConstIterator qit = qf->varsBegin();
               qit != qf->varsEnd(); ++qit) {
            bStats_.qvars.insert(qit->first);
          }
        }
      }
    }
    obj = p_->getObjective();
    if (obj->getFunctionType() == Quadratic ||
        obj->getFunctionType() == Bilinear) {
      qf = obj->getFunction()->getQuadraticFunction();
      if (qf) {
        for (VarIntMapConstIterator qit = qf->varsBegin();
             qit != qf->varsEnd(); ++qit) {
          bStats_.qvars.insert(qit->first);
        }
      }
    }
    for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
      if ((*vit)->getLb() > -INFINITY) {
        ++bStats_.nqlb;
      }
      if ((*vit)->getUb() < INFINITY) {
        ++bStats_.nqub;
      }
    } 
    calcRangeOfQuadVars_();
    writeBTStats_(std::cout, true);
    //is_inf = tightenLP_(&lpchanged);
    if (is_inf) {
      return true;
    }

    if (true==lpchanged) {
      changed = true;
    }

    for (VariableSet::iterator vit = bStats_.qvars.begin();
         vit != bStats_.qvars.end(); ++vit) {
      if ((*vit)->getLb() > -INFINITY) {
        ++bStats_.nqlbl;
      }
      if ((*vit)->getUb() < INFINITY) {
        ++bStats_.nqubl;
      }
    }
    bStats_.nqlbl -= bStats_.nqlb;
    bStats_.nqubl -= bStats_.nqub;
    bStats_.timeLP = timer_->query()-stime;
    stime = timer_->query();
    calcRangeOfQuadVars_();
    writeBTStats_(std::cout, false);
  }
  while (true==changed) {
    ++pStats_.iters;
    changed = false;
    for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
      is_inf = propSqrBnds_(it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true==lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
    for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
      is_inf = propBilBnds_(*it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true==lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
  }

  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    upSqCon_(it->second->oeCon, rel->getRelaxationVar(it->first),
             rel->getRelaxationVar(it->second->y), rel, r_mods);
  }
  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    upBilCon_(*it, rel, r_mods);
  }

  if (modRel_) {
    pStats_.nMods += r_mods.size();
  } else {
    pStats_.nMods += p_mods.size();
  }
  //rel->write(std::cout);

  pStats_.timeN += timer_->query()-stime;
  return false;
}


bool QuadHandler::propBilBnds_(LinBil* lx0x1, RelaxationPtr rel,
                               bool mod_rel, bool *changed, ModVector &p_mods,
                               ModVector &r_mods)
{
  VariablePtr x0 = lx0x1->getX0();
  VariablePtr x1 = lx0x1->getX1();
  VariablePtr  y = lx0x1->getY();
  double lb, ub;

  BoundsOnProduct(true, x0, x1, lb, ub);
  //x0->write(std::cout);
  //x1->write(std::cout);
  // y->write(std::cout);
  //std::cout << lb << " " << ub << std::endl;
  if (updatePBounds_(y, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
    return true; // infeasible
  }

  // other direction
  BoundsOnDiv(y->getLb(), y->getUb(), x0->getLb(), x0->getUb(), lb, ub);
  if (updatePBounds_(x1, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
    return true; // infeasible
  }

  BoundsOnDiv(y->getLb(), y->getUb(), x1->getLb(), x1->getUb(), lb, ub);
  if (updatePBounds_(x0, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
    return true; // infeasible
  }

  return false;
}

bool QuadHandler::propBilBnds_(LinBil* lx0x1, bool *changed)
{
  double lb, ub;
  VariablePtr x0   = lx0x1->getX0();     // x0 and x1 are variables in p_
  VariablePtr x1   = lx0x1->getX1();
  VariablePtr y    = lx0x1->getY();

  BoundsOnProduct(true, x0, x1, lb, ub);
  if (updatePBounds_(y, lb, ub, changed) < 0) {
    return true;
  } 

  // reverse
  BoundsOnDiv(y->getLb(), y->getUb(), x0->getLb(), x0->getUb(), lb, ub);
  if (updatePBounds_(x1, lb, ub, changed) < 0) {
    return true;
  } 

  BoundsOnDiv(y->getLb(), y->getUb(), x1->getLb(), x1->getUb(), lb, ub);
  if (updatePBounds_(x0, lb, ub, changed) < 0) {
    return true;
  } 

  return false;
}


bool QuadHandler::propSqrBnds_(LinSqrMapIter lx2, bool *changed)
{
  VariablePtr x   = lx2->first;      // x0 and y are variables in p_
  VariablePtr y   = lx2->second->y;
  double lb, ub;

  BoundsOnSquare(x, lb, ub);
  if (updatePBounds_(y, lb, ub, changed) < 0) {
    return true;
  } 

  // other direction.
  if (y->getUb()>bTol_) {
    ub = sqrt(y->getUb());
    lb = -ub;
    assert(y->getLb()>=0.0); // square of a number.
    if (x->getLb() > -sqrt(y->getLb())+bTol_) {
      lb = sqrt(y->getLb());
    }
    if (updatePBounds_(x, lb, ub, changed) < 0) {
      return true;
    } 
  } else if (y->getUb()<-bTol_) {
    return true;
  } else {
    if (updatePBounds_(x, 0.0, 0.0, changed) < 0) {
      return true;
    } 
  }
  return false;
}


bool QuadHandler::propSqrBnds_(LinSqrMapIter lx2, RelaxationPtr rel,
                               bool mod_rel, bool *changed, ModVector &p_mods,
                               ModVector &r_mods)
{
  double lb, ub;

  VariablePtr x = lx2->first;      // x and y are variables in p_
  VariablePtr y = lx2->second->y;

  BoundsOnSquare(x, lb, ub);
  if (updatePBounds_(y, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
    return true; // infeasible
  }

  // other direction.
  if (y->getUb()>bTol_) {
    ub = sqrt(y->getUb());
    lb = -ub;
    assert(y->getLb()>=0.0);
    if (x->getLb() > -sqrt(y->getLb())+bTol_) {
      lb = sqrt(y->getLb());
    }
    if (updatePBounds_(x, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
      return true; // infeasible
    }
  } else if (y->getUb()<-bTol_) {
    return true; // infeasible
  } else {
    if (updatePBounds_(x, 0.0, 0.0, rel, mod_rel, changed, p_mods, r_mods)<0) {
      return true; // infeasible
    }
  }

  return false;
}

void QuadHandler::relax_(RelaxationPtr rel, bool *)
{
  double rhs;
  LinearFunctionPtr lf;
  VariablePtr y, x0, x1;
  FunctionPtr f;
  ConstraintVector cons(4);


  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    x0 = rel->getRelaxationVar(it->second->x);
    y  = rel->getRelaxationVar(it->second->y);
    lf = getNewSqLf_(x0, y, x0->getLb(), x0->getUb(), rhs);

    f = (FunctionPtr) new Function(lf);
    it->second->oeCon = rel->newConstraint(f, -INFINITY, rhs);
  }

  for (LinBilSetIter it=x0x1Funs_.begin(); it!=x0x1Funs_.end(); ++it) {
    x0 = rel->getRelaxationVar((*it)->getX0());
    x1 = rel->getRelaxationVar((*it)->getX1());
    y  = rel->getRelaxationVar((*it)->getY());
    for (int i=0; i<4; ++i) {
      lf = getNewBilLf_(x0, x0->getLb(), x0->getUb(),
                        x1, x1->getLb(), x1->getUb(), y, i, rhs);
      f = (FunctionPtr) new Function(lf);
      cons[i] = rel->newConstraint(f, -INFINITY, rhs);
    }
    (*it)->setCons(cons[0], cons[1], cons[2], cons[3]);
  }

  assert(0 == rel->checkConVars());

  //writeBTStats_(std::cout, true);
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
  // do nothing. Presolve will take care of tightening bounds
}


void QuadHandler::resetStats_()
{
  pStats_.iters  = 0;
  pStats_.time   = 0.0;
  pStats_.timeN  = 0.0;
  pStats_.vBnd   = 0;
  pStats_.nMods  = 0;
  
  sStats_.iters  = 0;
  sStats_.cuts   = 0;
  sStats_.time   = 0.0;

  bStats_.niters = 0;
  bStats_.qvars.clear();
  bStats_.nqlb = 0;
  bStats_.nqub = 0;
  bStats_.nqlbs = 0;
  bStats_.nqubs = 0;
  bStats_.vBnds = 0;
  bStats_.cBnds = 0;
  bStats_.nqlbq = 0;
  bStats_.nqubq = 0;
  bStats_.vBndq = 0;
  bStats_.cBndq = 0;
  bStats_.nqlbl = 0;
  bStats_.nqubl = 0;
  bStats_.vBndl = 0;
  bStats_.nLP = 0;
  bStats_.dlb = 0;
  bStats_.dub = 0;
  bStats_.time = 0;
  bStats_.timeLP = 0;
  bStats_.avg_range = 0;
  bStats_.sd_range = 0;
  bStats_.body_diag = 0;
}


void QuadHandler::separate(ConstSolutionPtr sol, NodePtr , RelaxationPtr rel,
                           CutManager *, SolutionPoolPtr , ModVector &,
                           ModVector &,  bool *, SeparationStatus *status)
{
  double yval, xval;
  double yl, xl;
  const double *x = sol->getPrimal();
  bool ifcuts;

  ++sStats_.iters;
  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    xval = x[it->first->getIndex()];
    yval = x[it->second->y->getIndex()];
    if (xval*xval - yval > rTol_*fabs(yval) &&
       fabs(xval*xval-yval) > aTol_) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "xval = " << xval
                                    << " yval = " << yval << " violation = "
                                    << xval*xval-yval << std::endl;
#endif
      findLinPt_(xval, yval, xl, yl);
      addCut_(rel->getRelaxationVar(it->first), 
              rel->getRelaxationVar(it->second->y), xl, yl, xval, yval, 
              rel, ifcuts);
      if (true==ifcuts) {
        *status = SepaResolve;
      }
    }
  }
}

double QuadHandler::calcUpperUnivar_(double a, double b, double lx,
                                     double ux) {
  double u,s,t,r;

  u = std::max(lx*(a*lx + b), ux*(a*ux + b));
  s = b/2.0;
  t = s/(-a);
  if (t > lx) {
    r = (-2.0*a)*ux;
    if (r > b) {
      u = std::max(u, s*t);
    }
  }
  return u;
}

void QuadHandler::getTermBnds_(VariablePtr v, double coef, double &lb,
                                  double &ub) {
  if (coef > 0) {
    lb = v->getLb() > -INFINITY ? coef*v->getLb() : -INFINITY;
    ub = v->getUb() < INFINITY ? coef*v->getUb() : INFINITY;
  } else {
    lb = v->getUb() < INFINITY ? coef*v->getUb() : -INFINITY;
    ub = v->getLb() > -INFINITY ? coef*v->getLb() : INFINITY;
  }
}

void QuadHandler::getTermBnds_(VariablePtr v1, VariablePtr v2, double coef,
                                   double &lb, double &ub) {
  double qlb, qub;
  if (v1->getIndex() == v2->getIndex()) {
    BoundsOnSquare(v1, qlb, qub);
  } else {
    BoundsOnProduct(true, v1, v2, qlb, qub);
  }
  if (coef > 0) {
    lb = qlb > -INFINITY ? coef*qlb : -INFINITY;
    ub = qub < INFINITY ? coef*qub : INFINITY;
  } else {
    lb = qub < INFINITY ? coef*qub : -INFINITY;
    ub = qlb > -INFINITY ? coef*qlb : INFINITY;
  }
}

void QuadHandler::getTermBnds_(VariablePtr v, double a, double b,
                               double &lb, double &ub) {
  double lx = v->getLb();
  double ux = v->getUb();

  if (lx > -aTol_) {
    ub = calcUpperUnivar_(a, b, lx, ux);
    lb = -calcUpperUnivar_(-a, -b, lx, ux);
  } else if (ux < aTol_) {
    ub = calcUpperUnivar_(a, -b, -ux, -lx);
    lb = -calcUpperUnivar_(-a, b, -ux, -lx);
  } else {
    ub = calcUpperUnivar_(a, b, 0.0, ux);
    ub = std::max(ub, calcUpperUnivar_(a, -b, 0.0, -lx));
    lb = -calcUpperUnivar_(-a, -b, 0.0, ux);
    lb = std::min(lb, -calcUpperUnivar_(-a, b, 0.0, -lx));
  }
}

bool QuadHandler::calcVarBnd_(VariablePtr v, double coef, double lb,
                              double ub, bool *c1) {
  double vlb, vub;
  vlb = coef > 0 ? lb/coef : ub/coef;
  vub = coef > 0 ? ub/coef : lb/coef;

  if (updatePBounds_(v, vlb, vub, c1) < 0) {
    return true;
  }
  return false;
}

bool QuadHandler::calcVarBnd_(VariablePtr v1, VariablePtr v2, double coef,
                              double lb, double ub, bool *c1, bool *c2) {
  double qlb, qub, vlb, vub;
  qlb = coef > 0 ? lb/coef : ub/coef;
  qub = coef > 0 ? ub/coef : lb/coef;
  qlb = qlb >= 0 ? qlb : 0;
  if (v1->getIndex() == v2->getIndex()) {
    if (qub>bTol_) {
      vub = sqrt(qub);
      vlb = -ub;
      assert(qlb>=0.0); // square of a number.
      if (v1->getLb() > -sqrt(qlb)+bTol_) {
        vlb = sqrt(qlb);
      }
      if (updatePBounds_(v1, vlb, vub, c1) < 0) {
        return true;
      } 
    } else if (qub<-bTol_) {
      return true;
    } else {
      if (updatePBounds_(v1, 0.0, 0.0, c1) < 0) {
        return true;
      }
    }
    return false;
  } else {
    BoundsOnDiv(qlb, qub, v1->getLb(), v1->getUb(), vlb, vub);
    if (updatePBounds_(v2, vlb, vub, c2) < 0) {
      return true;
    } 

    BoundsOnDiv(qlb, qub, v2->getLb(), v2->getUb(), vlb, vub);
    if (updatePBounds_(v1, vlb, vub, c1) < 0) {
      return true;
    }
    return false;
  }
}

bool QuadHandler::calcVarBnd_(VariablePtr v, double a, double b, double ly,
                              double uy, bool *c1) {
  double lx = v->getLb();
  double ux = v->getUb();
  double lb = -INFINITY, ub = INFINITY, delta, lb2, ub2;

  if (fabs(a) <= aTol_) {
    assert(fabs(b) > aTol_);
    lb = ly/b;
    ub = uy/b;
  } else if (a > aTol_) { /// the term is convex
    if (uy < INFINITY) { /// finite upper bound
      delta = b*b + 4.0*a*uy;
      if (delta < -aTol_) { /// upper bound is less than minima
        return true;
      } else if (fabs(delta) <= aTol_) { /// upper bound is at minima
        lb = -b/(2.0*a);
        ub = lb;
      } else { /// upper bound is greater than minima
        lb = (-b - sqrt(delta))/(2.0*a);
        ub = (-b + sqrt(delta))/(2.0*a);
        delta = b*b + 4.0*a*ly;
        if (delta > aTol_) { /// lower bound is greater than minima
          /// The bound on the variable now are
          /// \mathbb{R} \setminus (lb2, ub2) intersection [lb, ub]
          lb2 = (-b - sqrt(delta))/(2.0*a);
          ub2 = (-b + sqrt(delta))/(2.0*a);
          if (lx > lb2 + bTol_) {
            lb = ub2;
          }
          if (ux < ub2 - bTol_) {
            ub = lb2;
          }
        }
      }
    } else { /// infinite upper bound
      delta = b*b + 4.0*a*ly;
      if (delta > aTol_) {
        lb2 = (-b - sqrt(delta))/(2.0*a);
        ub2 = (-b + sqrt(delta))/(2.0*a);
        if (lx > lb2 + bTol_ && lx < ub2 - bTol_) {
          lb = ub2;
        }
        if (ux > lb2 + bTol_ && ux < ub2 - bTol_) {
          ub = lb2;
        }
      }
    }
  } else { /// concave term
    if (ly > -INFINITY) { /// finite lower bound
      delta = b*b + 4.0*a*ly;
      if (delta < -aTol_) { /// lower bound is greater than maxima
        return true;
      } else if (fabs(delta) <= aTol_) { /// lower bound is at maxima
        lb = -b/(2.0*a);
        ub = lb;
      } else { /// lower bound is less than maxima
        lb = (-b + sqrt(delta))/(2.0*a); // since a is -ve this term will be lb
        ub = (-b - sqrt(delta))/(2.0*a); // since a is -ve this term will be ub
        delta = b*b + 4.0*a*uy;
        if (delta > aTol_) { /// upper bound is less than maxima
          /// The bound on the variable now are
          /// \mathbb{R} \setminus (lb2, ub2) intersection [lb, ub]
          lb2 = (-b + sqrt(delta))/(2.0*a);//since a is -ve this term will be lb
          ub2 = (-b - sqrt(delta))/(2.0*a);//since a is -ve this term will be ub
          if (lx > lb2 + bTol_) {
            lb = ub2;
          }
          if (ux < ub2 - bTol_) {
            ub = lb2;
          }
        }
      }
    } else { /// infinite lower bound
      delta = b*b + 4.0*a*uy;
      if (delta > aTol_) {
        lb2 = (-b - sqrt(delta))/(2.0*a);
        ub2 = (-b + sqrt(delta))/(2.0*a);
        if (lx > lb2 + bTol_ && lx < ub2 - bTol_) {
          lb = ub2;
        }
        if (ux > lb2 + bTol_ && ux < ub2 - bTol_) {
          ub = lb2;
        }
      }
    }
  }
  if (updatePBounds_(v, lb, ub, c1) < 0) {
    return true;
  }
 return false; 
}

double QuadHandler::getBndByLP_(bool &is_inf) {
  double b;
  EngineStatus lpStatus;
  lpStatus = lpe_->solve();
  ++bStats_.nLP;
  is_inf = false;

  switch(lpStatus) {
    case (ProvenOptimal):
    case (EngineIterationLimit):
    case (ProvenUnbounded):
      b = lpe_->getSolution()->getObjValue();
      break;
    case (ProvenInfeasible):
    case (ProvenObjectiveCutOff):
      b = INFINITY;
      is_inf = true;
      break;
    case (EngineError):
    case (EngineUnknownStatus):
    default:
      logger_->msgStream(LogError) << me_ << "LP engine status at root= "
        << lpStatus << std::endl;
      assert(!"In QuadHandler: stopped at root. Check error log.");
      break;
  }
  return b;
}

void QuadHandler::setEngine(Engine* engine) {
  if (lpe_ && lpe_ != engine) {
    lpe_->clear();
  }
  lpe_ = engine;
}

bool QuadHandler::tightenLP_(bool *changed) {
  ConstraintPtr c;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf, lflp = 0;
  FunctionPtr flp = 0;
  VarSet lvars, qvars;
  VariablePtr v;
  std::map<VariablePtr, VariablePtr> pv_lpv;
  ProblemPtr lp;
  double lb, ub, clb, cub;
  bool is_inf;
  bool c1;

  if (p_->getSize()->consWithLin == 0) {
    return false;
  }

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                               ++cit) {
    c = *cit;
    lf = c->getFunction()->getLinearFunction();
    if (lf) {
      for (VariableGroupConstIterator lit = lf->termsBegin();
           lit != lf->termsEnd(); ++lit) {
        lvars.insert(lit->first);
      }
    }
  }

  for (VariableSet::iterator vit = bStats_.qvars.begin();
       vit != bStats_.qvars.end(); ++vit) {
    if (std::find(lvars.begin(), lvars.end(), *vit) != lvars.end()) {
      qvars.insert(*vit);
    }
  }

  if (qvars.size() == 0) {
    lvars.clear();
    return false;
  }

  lp = (ProblemPtr) new Problem(env_);
  for (VariableSet::iterator vit = lvars.begin(); vit != lvars.end(); ++vit) {
    v = lp->newVariable((*vit)->getLb(), (*vit)->getUb(), (*vit)->getType());
    pv_lpv.insert(std::make_pair(*vit, v));
  } 
  lvars.clear();

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                               ++cit) {
    c = *cit;
    lf = c->getFunction()->getLinearFunction();
    if (lf) {
      lflp = (LinearFunctionPtr) new LinearFunction();
      for (VariableGroupConstIterator lit = lf->termsBegin();
           lit != lf->termsEnd(); ++lit) {
        lflp->addTerm(pv_lpv[lit->first], lit->second);
      }

      clb = c->getLb();
      cub = c->getUb();
      qf = c->getFunction()->getQuadraticFunction();
      if (qf) {
        for (VariablePairGroupConstIterator qit = qf->begin();
             qit != qf->end(); ++qit) {
          getTermBnds_(qit->first.first, qit->first.second, qit->second,
                           lb, ub);
          clb = clb > -INFINITY ? clb - ub : -INFINITY;
          cub = cub < INFINITY ? cub - lb : INFINITY;
        }
      }

      if (clb > -INFINITY || cub < INFINITY) {//cons is redundant otherwise
        flp = (FunctionPtr) new Function(lflp);
        lp->newConstraint(flp, clb, cub);
      }
    }
  }

  if (lp->getNumCons() == 0) {
      qvars.clear();
      pv_lpv.clear();
      delete lpe_;
      delete lp;
      return false;
  }

  flp = (FunctionPtr) new Function();
  lp->newObjective(flp, 0.0, Minimize);
  //lp->setEngine(lpe_);
  lpe_->load(lp);

  for (VariableSet::iterator vit = qvars.begin(); vit != qvars.end(); ++vit) {
    lflp = (LinearFunctionPtr) new LinearFunction();
    lflp->addTerm(pv_lpv[*vit], 1.0);
    flp = (FunctionPtr) new Function(lflp);
    lp->changeObj(flp, 0.0);
    lb = getBndByLP_(is_inf);
    if (is_inf) {
      qvars.clear();
      pv_lpv.clear();
      delete lpe_;
      delete lp;
      return true;
    }
    (*flp) *= -1.0;
    lp->changeObj(flp, 0.0);
    ub = -getBndByLP_(is_inf);
    if (is_inf) {
      qvars.clear();
      pv_lpv.clear();
      delete lpe_;
      delete lp;
      return true;
    }
    c1 = false;
    if (updatePBounds_(*vit, lb, ub, &c1) < 0) {
      qvars.clear();
      pv_lpv.clear();
      delete lpe_;
      delete lp;
      return true;
    }
    if (c1 == true) {
      ++bStats_.vBndl;
      *changed = true;
    }
  }
  qvars.clear();
  pv_lpv.clear();
  delete lpe_;
  delete lp;
  return false;
}

bool QuadHandler::tightenQuad_(bool *changed) {
  ConstraintPtr c;
  double implLb, implUb;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  VarVector qvars;
  DoubleVector fwdLb, fwdUb;
  DoubleVector::iterator liter, uiter;
  double lb, ub, clb, cub;
  bool c1, c2;

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                               ++cit) {
    c = *cit;
    implLb = 0.0;
    implUb = 0.0;
    // If the constraint is bilinear then simple tightening suffices
    if (c->getFunctionType() == Quadratic) {
      lf = c->getFunction()->getLinearFunction();
      if (lf) {
        qf = c->getFunction()->getQuadraticFunction();
        if (qf) {
          // Forward Propagation
          for (VariablePairGroupConstIterator qit = qf->begin();
               qit != qf->end(); ++qit) {
            if (qit->first.first->getIndex() == qit->first.second->getIndex()
                && lf->hasVar(qit->first.first)) {
              qvars.push_back(qit->first.first);
              getTermBnds_(qit->first.first, qit->second,
                          lf->getWeight(qit->first.first), lb, ub);
              implLb += lb;
              implUb += ub;
              fwdLb.push_back(lb);
              fwdUb.push_back(ub);
            } else {
              getTermBnds_(qit->first.first, qit->first.second, qit->second,
                               lb, ub);
              implLb += lb;
              implUb += ub;
              fwdLb.push_back(lb);
              fwdUb.push_back(ub);
            }
          }
          if (qvars.size() == 0) {
            fwdLb.clear();
            fwdUb.clear();
            continue;
          }
          for (VariableGroupConstIterator lit = lf->termsBegin();
               lit != lf->termsEnd(); ++lit) {
            if (std::find(qvars.begin(), qvars.end(),
                lit->first) == qvars.end()) {
              getTermBnds_(lit->first, lit->second, lb, ub);
              implLb += lb;
              implUb += ub;
              fwdLb.push_back(lb);
              fwdUb.push_back(ub);
            }
          }
          
          clb = c->getLb();
          cub = c->getUb();
          
          // Implied bounds already better than constraint bounds.
          // The constraint is redundant.
          if (implLb > clb + aTol_ && implUb < cub - aTol_) {
            p_->markDelete(c);
            *changed = true;
            fwdLb.clear();
            fwdUb.clear();
            continue;
          }
          // constraint is infeasible
          if (implLb > cub + aTol_ || implUb < clb - aTol_) {
            return true;
          }

          // Backward Propagation
          liter = fwdLb.begin();
          uiter = fwdUb.begin();
          clb = clb > implLb ? clb : implLb;
          cub = cub < implUb ? cub : implUb;
          if (clb > c->getLb() + aTol_) {
            p_->changeBound(c, Lower, clb);
            ++bStats_.cBndq;
          }
          if (cub < c->getUb() - aTol_) {
            p_->changeBound(c, Upper, cub);
            ++bStats_.cBndq;
          }
          for (VariablePairGroupConstIterator qit = qf->begin();
               qit != qf->end(); ++qit) {
            if (qit->first.first->getIndex() == qit->first.second->getIndex()
                && std::find(qvars.begin(), qvars.end(),
                             qit->first.first) != qvars.end()) {
              if (implUb >= INFINITY || *uiter >= INFINITY) {
                lb = -INFINITY;
              } else {
                lb = clb - (implUb - *uiter);
              }

              if (implLb <= -INFINITY || *uiter <= -INFINITY) {
                ub = INFINITY;
              } else {
                ub = cub - (implLb - *liter);
              }
              c1 = false;
              if (calcVarBnd_(qit->first.first, qit->second,
                              lf->getWeight(qit->first.first), lb, ub,
                              &c1)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                ++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            } else {
              if (implUb >= INFINITY || *uiter >= INFINITY) {
                lb = -INFINITY;
              } else {
                lb = clb - (implUb - *uiter);
              }

              if (implLb <= -INFINITY || *uiter <= -INFINITY) {
                ub = INFINITY;
              } else {
                ub = cub - (implLb - *liter);
              }
              c1 = false;
              c2 = false;
              if (calcVarBnd_(qit->first.first, qit->first.second, qit->second,
                              lb, ub, &c1, &c2)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                ++bStats_.vBndq;
                *changed = true;
              }
              if (c2 == true) {
                ++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            }
          }
          
          for (VariableGroupConstIterator lit = lf->termsBegin();
               lit != lf->termsEnd(); ++lit) {
            if (std::find(qvars.begin(), qvars.end(),
                lit->first) == qvars.end()) {
              if (implUb >= INFINITY || *uiter >= INFINITY) {
                lb = -INFINITY;
              } else {
                lb = clb - (implUb - *uiter);
              }

              if (implLb <= -INFINITY || *uiter <= -INFINITY) {
                ub = INFINITY;
              } else {
                ub = cub - (implLb - *liter);
              }
              c1 = false;
              if (calcVarBnd_(lit->first, lit->second, lb, ub, &c1)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                ++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            }
          }
          fwdLb.clear();
          fwdUb.clear();
        }
      }
    }
  }
  return false;
}

bool QuadHandler::tightenSimple_(bool *changed) {
  double implLb, implUb;
  DoubleVector fwdLb, fwdUb;
  DoubleVector::iterator liter, uiter;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  double clb, cub, lb, ub;
  bool c1, c2;

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
                               ++cit) {
    c = *cit;
    implLb = 0.0;
    implUb = 0.0;
    if (c->getFunctionType() == Quadratic || c->getFunctionType() == Bilinear){
      // delete the constraint if unbounded on both sides
      if (cub == INFINITY && clb == -INFINITY) {
        p_->markDelete(c);
        *changed = true;
        continue;
      }
      lf = c->getFunction()->getLinearFunction();
      qf = c->getFunction()->getQuadraticFunction();
      // Forward Propagation
      if (lf) {
        for (VariableGroupConstIterator lit = lf->termsBegin();
             lit != lf->termsEnd(); ++lit) {
          getTermBnds_(lit->first, lit->second, lb, ub);
          implLb += lb;
          implUb += ub;
          fwdLb.push_back(lb);
          fwdUb.push_back(ub);
        }
      }

      if (qf) {
        for (VariablePairGroupConstIterator qit = qf->begin();
             qit != qf->end(); ++qit) {
          getTermBnds_(qit->first.first, qit->first.second, qit->second,
                           lb, ub);
          implLb += lb;
          implUb += ub;
          fwdLb.push_back(lb);
          fwdUb.push_back(ub);
        }
      }
      clb = c->getLb();
      cub = c->getUb();

      // Implied bounds already better than constraint bounds.
      // The constraint is redundant.
      if (implLb > clb + aTol_ && implUb < cub - aTol_) {
        p_->markDelete(c);
        *changed = true;
        fwdLb.clear();
        fwdUb.clear();
        continue;
      }
      // constraint is infeasible
      if (implLb > cub + aTol_ || implUb < clb - aTol_) {
        return true;
      }
      // Backward Propagation
      liter = fwdLb.begin();
      uiter = fwdUb.begin();
      clb = clb > implLb ? clb : implLb;
      cub = cub < implUb ? cub : implUb;
      if (clb > c->getLb() + aTol_) {
        p_->changeBound(c, Lower, clb);
        ++bStats_.cBnds;
      }
      if (cub < c->getUb() - aTol_) {
        p_->changeBound(c, Upper, cub);
        ++bStats_.cBnds;
      }
      if (lf) {
        for (VariableGroupConstIterator lit = lf->termsBegin();
             lit != lf->termsEnd(); ++lit) {
          if (implUb >= INFINITY || *uiter >= INFINITY) {
            lb = -INFINITY;
          } else {
            lb = clb - (implUb - *uiter);
          }

          if (implLb <= -INFINITY || *uiter <= -INFINITY) {
            ub = INFINITY;
          } else {
            ub = cub - (implLb - *liter);
          }
          c1 = false;
          if (calcVarBnd_(lit->first, lit->second, lb, ub, &c1)) {
            fwdLb.clear();
            fwdUb.clear();
            return true;
          }
          if (c1 == true) {
            ++bStats_.vBnds;
            *changed = true;
          }
          ++liter;
          ++uiter;
        }
      }

      if (qf) {
        for (VariablePairGroupConstIterator qit = qf->begin();
             qit != qf->end(); ++qit) {
          lb = clb - (implUb - *uiter);
          ub = cub - (implLb - *liter);
          c1 = false;
          c2 = false;
          if (calcVarBnd_(qit->first.first, qit->first.second, qit->second,
                          lb, ub, &c1, &c2)) {
            fwdLb.clear();
            fwdUb.clear();
            return true;
          }
          if (c1 == true) {
            ++bStats_.vBnds;
            *changed = true;
          }
          if (c2 == true) {
            ++bStats_.vBnds;
            *changed = true;
          }
          ++liter;
          ++uiter;
        }
      }
      fwdLb.clear();
      fwdUb.clear();
    }
  }
  return false;
}

int QuadHandler::updatePBounds_(VariablePtr v, double lb, double ub,
                                bool *changed)
{
  if (ub < v->getLb() - bTol_ || lb > v->getUb() + bTol_) { 
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "inconsistent bounds of"
                                    << v->getName() << " " << v->getLb()
                                    << " " << v->getUb() << std::endl;
#endif
    return -1;
  }
  
  if (ub < v->getUb() - bTol_ && (v->getUb() == INFINITY ||
                                  ub < v->getUb()-fabs(v->getUb())*rTol_)) {
    p_->changeBound(v, Upper, ub);
    *changed = true;
    ++pStats_.vBnd;
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "new ub of " << v->getName()
                                    << " = " << v->getUb() << std::endl;
#endif
  }
  if (lb > v->getLb() + aTol_ && (v->getLb() == -INFINITY ||
                                  lb > v->getLb()+fabs(v->getLb())*rTol_)) {
    p_->changeBound(v, Lower, lb);
    *changed = true;
    ++pStats_.vBnd;
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "new lb of " << v->getName()
                                    << " = " << v->getLb() << std::endl;
#endif
  }

  return 0;
}


int QuadHandler::updatePBounds_(VariablePtr v, double lb, double ub,
                                RelaxationPtr rel, bool mod_rel,
                                bool *changed, ModVector &p_mods,
                                ModVector &r_mods)
{
  VarBoundMod2Ptr b2mod;
  VarBoundModPtr bmod;

  if (lb > v->getUb()+bTol_ || ub < v->getLb()-bTol_) {
    return -1;
  }

  if (lb > v->getLb()+bTol_ && ub < v->getUb()-bTol_ &&
      (v->getLb() == -INFINITY || lb > v->getLb()+rTol_*fabs(v->getLb())) &&
      (v->getUb() ==  INFINITY || ub < v->getUb()-rTol_*fabs(v->getUb()))) {
    *changed = true;
    ++pStats_.vBnd;
    b2mod  = (VarBoundMod2Ptr) new VarBoundMod2(v, lb, ub);
    b2mod->applyToProblem(p_);
    p_mods.push_back(b2mod);
#if SPEW
    b2mod->write(logger_->msgStream(LogDebug2));
#endif 

    if (true == mod_rel) {
      b2mod = (VarBoundMod2Ptr)
              new VarBoundMod2(rel->getRelaxationVar(v), lb, ub);
      b2mod->applyToProblem(rel);
      r_mods.push_back(b2mod);
    }
  } else if (lb > v->getLb()+bTol_ && 
             (v->getLb()==-INFINITY || lb>v->getLb()+rTol_*fabs(v->getLb()))) {
    ++pStats_.vBnd;
    *changed = true;
    bmod  = (VarBoundModPtr) new VarBoundMod(v, Lower, lb);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
#if SPEW
    bmod->write(logger_->msgStream(LogDebug2));
#endif 

    if (true == mod_rel) {
      bmod = (VarBoundModPtr)
             new VarBoundMod(rel->getRelaxationVar(v), Lower, lb);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  } else if (ub < v->getUb()-bTol_ &&
             (v->getUb()== INFINITY || ub<v->getUb()-rTol_*fabs(v->getUb()))) {
    ++pStats_.vBnd;
    *changed = true;
    bmod  = (VarBoundModPtr) new VarBoundMod(v, Upper, ub);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
#if SPEW
    bmod->write(logger_->msgStream(LogDebug2));
#endif 
    
    if (true == mod_rel) {
      bmod  = (VarBoundModPtr)
               new VarBoundMod(rel->getRelaxationVar(v), Upper, ub);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  } 
  return 0;
}


void QuadHandler::upBilCon_(LinBil* lx0x1, RelaxationPtr rel, ModVector
                            &r_mods)
{
  LinConModPtr lmod;
  LinearFunctionPtr lf;
  VariablePtr y  = rel->getRelaxationVar(lx0x1->getY());
  VariablePtr x0 = rel->getRelaxationVar(lx0x1->getX0());
  VariablePtr x1 = rel->getRelaxationVar(lx0x1->getX1());
  double l0 = x0->getLb();
  double u0 = x0->getUb();
  double l1 = x1->getLb();
  double u1 = x1->getUb();
  double a0, a1;
  double rhs;
  double eps = aTol_/10.0;
  ConstraintPtr con;

  // all constraints in the relaxation are of (<= rhs) type.
  // y >= l1x0 + l0x1 - l1l0: binding at (l0, l1), (l0, u1), (u0, l1)
  con = lx0x1->getC0();
  lf = con->getLinearFunction();
  a0 = lf->getWeight(x0);
  a1 = lf->getWeight(x1);
  if (a0*l0 + a1*l1 - l0*l1 < con->getUb() - eps ||
      a0*l0 + a1*u1 - l0*u1 < con->getUb() - eps || 
      a0*u0 + a1*l1 - u0*l1 < con->getUb() - eps) {
    lf = getNewBilLf_(x0, l0, u0, x1, l1, u1, y, 0, rhs);
    lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
  }

  // y >= u1x0 + u0x1 - u1u0: binding at (l0, u1), (u0, l1), (u0, u1)
  con = lx0x1->getC1();
  lf = con->getLinearFunction();
  a0 = lf->getWeight(x0);
  a1 = lf->getWeight(x1);
  if (a0*l0 + a1*u1 - l0*u1 < con->getUb() - eps ||
      a0*u0 + a1*l1 - u0*l1 < con->getUb() - eps || 
      a0*u0 + a1*u1 - u0*u1 < con->getUb() - eps) {
    lf = getNewBilLf_(x0, l0, u0, x1, l1, u1, y, 1, rhs);
    lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
  }


  // y <= u1x0 + l0x1 - l0u1: binding at (l0, l1), (l0, u1), (u0, u1)
  con = lx0x1->getC2();
  lf = con->getLinearFunction();
  a0 = lf->getWeight(x0);
  a1 = lf->getWeight(x1);
  if (a0*l0 + a1*l1 + l0*l1 < con->getUb() - eps ||
      a0*l0 + a1*u1 + l0*u1 < con->getUb() - eps || 
      a0*u0 + a1*u1 + u0*u1 < con->getUb() - eps) {
    lf = getNewBilLf_(x0, l0, u0, x1, l1, u1, y, 2, rhs);
    lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
  }

  // y <= l1x0 + u0x1 - u0l1: binding at (l0, l1), (u0, l1), (u0, u1)
  con = lx0x1->getC3();
  lf = con->getLinearFunction();
  a0 = lf->getWeight(x0);
  a1 = lf->getWeight(x1);
  if (a0*l0 + a1*l1 + l0*l1 < con->getUb() - eps ||
      a0*u0 + a1*l1 + u0*l1 < con->getUb() - eps || 
      a0*u0 + a1*u1 + u0*u1 < con->getUb() - eps) {
    lf = getNewBilLf_(x0, l0, u0, x1, l1, u1, y, 3, rhs);
    lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
  }
}


void QuadHandler::upSqCon_(ConstraintPtr con, VariablePtr x, VariablePtr y, 
                           RelaxationPtr rel, ModVector &r_mods)
{
  LinearFunctionPtr lf = con->getLinearFunction();
  double a_x = lf->getWeight(x);
  double lb = x->getLb();
  double ub = x->getUb();
  LinConModPtr lmod;
  double rhs;
  double eps = aTol_/10.0;

  assert(fabs(lf->getWeight(y) - 1.0) <= 1e-8);
  // y - (lb+ub)x <= -ub*lb
  if ((lb*lb + a_x*lb < con->getUb() - eps) ||
      (ub*ub + a_x*ub < con->getUb() - eps)) {
    lf = getNewSqLf_(x, y, x->getLb(), x->getUb(), rhs);
    lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
  }
}

bool QuadHandler::varBndsFromCons_(bool *changed)
{
  bool is_inf = false;
  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
    is_inf = propSqrBnds_(it, changed);
    if (true==is_inf) {
      return true;
    }
  }
  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
    is_inf = propBilBnds_(*it, changed);
    if (true==is_inf) {
      return true;
    }
  }
  return false;
}


void QuadHandler::writeStats(std::ostream &out) const
{
  out << me_ << "Statistics for presolve by QuadHandler:"        << std::endl
    << me_ << "Number of iterations           = "<< pStats_.iters  << std::endl
    << me_ << "Time taken in initial presolve = "<< pStats_.time   << std::endl
    << me_ << "Time taken in node presolves   = "<< pStats_.timeN  << std::endl
    << me_ << "Times variables tightened      = "<< pStats_.vBnd   << std::endl
    << me_ << "Changes in nodes               = "<< pStats_.nMods  << std::endl
    ;

  out << me_ << "Statistics for separation by QuadHandler:"        << std::endl
    << me_ << "Number of calls to separate    = "<< sStats_.iters  << std::endl
    << me_ << "Number of cuts added           = "<< sStats_.cuts   << std::endl
    << me_ << "Time taken in separation       = "<< sStats_.time   << std::endl
    ;
}

void QuadHandler::writeBTStats_(std::ostream &out, bool flag) {
  if (flag) {
    out << me_ << "Statistics for Bound tightening:" << std::endl << me_ <<
    "Time taken for presolve      ="
    << bStats_.time << std::endl << me_ <<
    "Average Range                = "
    << bStats_.avg_range << std::endl << me_ <<
    "Standard deviation of Range  = "
    << bStats_.sd_range << std::endl << me_ <<
    "Length of Body Diagonal      = "
    << bStats_.body_diag << std::endl;
  } else {
    out << me_ << "Statistics for Bound Tightening:" << std::endl << me_ <<
    "Time taken in solving LPs      ="
    << bStats_.timeLP << std::endl << me_ <<
    "Average Range after LP tightening                = "
    << bStats_.avg_range << std::endl << me_ <<
    "Standard deviation of Range after LP tightening  = "
    << bStats_.sd_range << std::endl << me_ <<
    "Length of Body Diagonal after LP tightening      = "
    << bStats_.body_diag << std::endl;
  }
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
