// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2021 The MINOTAUR Team.
// 

/**
 * \file UnivarQuadHandler.h
 * \brief Declare a handler for quadratic constraints of the form
 * \f$ y = ax^2 + bx \f$,
 * \f$ y = (x_1+x_2)^2 \f$,
 * and
 * \f$ y = (x_1-x_2)^2 \f$,
 * It does not handle any other quadratic constraints.
 * \author Mustafa Vora, IIT Bombay
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
#include "Function.h"
#include "LinearFunction.h"
#include "LinMods.h"
#include "Logger.h"
#include "Node.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "UnivarQuadHandler.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
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

const std::string UnivarQuadHandler::me_ = "UnivarQuadHandler: ";

UnivarQuadHandler::UnivarQuadHandler(EnvPtr env, ProblemPtr problem)
: aTol_(1e-6),
  bTol_(1e-5),
  rTol_(1e-4)
{
  p_ = problem; 
  modProb_ = false;
  modRel_  = true;
  logger_  = env->getLogger();
  resetStats_();
  timer_ = env->getTimer();
}

UnivarQuadHandler::~UnivarQuadHandler()
{
  for (LinUnivarVecIter it=univarFuns_.begin(); it != univarFuns_.end(); ++it) {
    delete *it;
  }
  univarFuns_.clear();

  for (LinBivarVecIter it=bivarFuns_.begin(); it!=bivarFuns_.end(); ++it) {
    delete *it;
  }
  bivarFuns_.clear();
}

void UnivarQuadHandler::addConstraint(ConstraintPtr newcon) {
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;
  UInt vid;
  VariablePtr v;
  LinUnivarPtr luv;
  LinBivarPtr lbv;
  
  cons_.push_back(newcon);
  qf = newcon->getQuadraticFunction();
  if (qf) {
    lf = newcon->getLinearFunction();
    assert(lf && qf);
    assert(lf->getNumTerms()<=2);
    if (lf->getNumTerms()==2) {
      assert(1==qf->getNumTerms());
      v = qf->begin()->first.first;
      vid = v->getId();
      assert(vid==qf->begin()->first.second->getId());
      luv = new LinUnivar();
      luv->x = v;
      luv->a = qf->begin()->second;
      for (VariableGroupConstIterator it = lf->termsBegin();
           it != lf->termsEnd(); ++it) {
        if (vid==it->first->getId()) {
          luv->b = it->second;
        } else {
          luv->y = it->first;
          assert(fabs(it->second)-1.0 < aTol_);
        }
      }
      luv->oeCon = ConstraintPtr();
      luv->tan1 = ConstraintPtr();
      luv->tan2 = ConstraintPtr();
      univarFuns_.push_back(luv);
    } else {
      assert(3==qf->getNumTerms());
      assert(2==qf->getNumVars());
      v = lf->termsBegin()->first;
      assert(fabs(lf->termsBegin()->second)-1.0 < aTol_);
      lbv = new LinBivar();
      lbv->y = v;
      for (VariablePairGroupConstIterator it = qf->begin();
           it != qf->end(); ++it) {
        if (it->first.first->getId() == it->first.second->getId()) {
          assert(fabs(it->second)-1.0 < aTol_);
        } else {
          assert(fabs(fabs(it->second)-2.0) < aTol_);
          lbv->x1 = it->first.first;
          lbv->x2 = it->first.second;
          lbv->pos = it->second > 0.0;
        }
      }
      lbv->oeCon1 = ConstraintPtr();
      lbv->oeCon2 = ConstraintPtr();
      lbv->tan1 = ConstraintPtr();
      lbv->tan2 = ConstraintPtr();
      lbv->tan3 = ConstraintPtr();
      lbv->tan4 = ConstraintPtr();
      bivarFuns_.push_back(lbv);
    }
  }
}

void UnivarQuadHandler::addCut_(RelaxationPtr rel, LinUnivarPtr luv, double xl,
                                double xval, double yval, bool &ifcuts)
{
  VariablePtr x = rel->getRelaxationVar(luv->x);
  VariablePtr y = rel->getRelaxationVar(luv->y);
  double a = luv->a, b = luv->b;
  double lf_eval = yval - (2*a*xl + b)*xval;

  if (a > aTol_) {
    if (-a*xl*xl - lf_eval > 1e-5) {
      LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
      FunctionPtr f;

      lf->addTerm(x, -(2*a*xl + b));
      lf->addTerm(y, 1.0);
      f = (FunctionPtr) new Function(lf);
      ifcuts = true;
      ++sStats_.cuts;
#if SPEW
      {
      ConstraintPtr c = rel->newConstraint(f, -a*xl*xl, INFINITY);
      logger_->msgStream(LogDebug2) << me_ << "new cut added" << std::endl;
      c->write(logger_->msgStream(LogDebug2));
      }
#else
      rel->newConstraint(f, -a*xl*xl, INFINITY);
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "Not adding cut because of "
                                    << "insufficient violation "
                                    << -a*xl*xl - lf_eval << std::endl;
#endif
    }
  } else {
    if (a*xl*xl + lf_eval > 1e-5) {
      LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
      FunctionPtr f;

      lf->addTerm(x, -(2*a*xl + b));
      lf->addTerm(y, 1.0);
      f = (FunctionPtr) new Function(lf);
      ifcuts = true;
      ++sStats_.cuts;
#if SPEW
      {
      ConstraintPtr c = rel->newConstraint(f, -INFINITY, a*xl*xl);
      logger_->msgStream(LogDebug2) << me_ << "new cut added" << std::endl;
      c->write(logger_->msgStream(LogDebug2));
      }
#else
      rel->newConstraint(f, -INFINITY, a*xl*xl);
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "Not adding cut because of "
                                    << "insufficient violation "
                                    << a*xl*xl + lf_eval << std::endl;
#endif
    }
  }
}

void UnivarQuadHandler::addCut_(RelaxationPtr rel, LinBivarPtr lbv,
                                double x1l, double x2l, double x1val,
                                double x2val, double yval, bool &ifcuts) {
  VariablePtr x1 = rel->getRelaxationVar(lbv->x1);
  VariablePtr x2 = rel->getRelaxationVar(lbv->x2);
  VariablePtr y = rel->getRelaxationVar(lbv->y);
  double lf_eval;

  if (lbv->pos) {
    lf_eval = yval - 2*(x1l + x2l)*(x1val + x2val);
    if (-(x1l + x2l)*(x1l + x2l) - lf_eval > 1e-5) {
      LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
      FunctionPtr f;

      lf->addTerm(x1, -2*(x1l+x2l));
      lf->addTerm(x2, -2*(x1l+x2l));
      lf->addTerm(y, 1.0);
      f = (FunctionPtr) new Function(lf);
      ifcuts = true;
      ++sStats_.cuts;
#if SPEW
      {
      ConstraintPtr c = rel->newConstraint(f, -(x1l+x2l)*(x1l+x2l), INFINITY);
      logger_->msgStream(LogDebug2) << me_ << "new cut added" << std::endl;
      c->write(logger_->msgStream(LogDebug2));
      }
#else
      rel->newConstraint(f, -(x1l+x2l)*(x1l+x2l), INFINITY);
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "Not adding cut because of "
                                    << "insufficient violation "
                                    << -(x1l+x2l)*(x1l+x2l) - lf_eval
                                    << std::endl;
#endif
    }
  } else {
    lf_eval = yval - 2*(x1l + x2l)*(x1val - x2val);
    if (-(x1l - x2l)*(x1l - x2l) - lf_eval > 1e-5) {
      LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
      FunctionPtr f;

      lf->addTerm(x1, -2*(x1l-x2l));
      lf->addTerm(x2, 2*(x1l-x2l));
      lf->addTerm(y, 1.0);
      f = (FunctionPtr) new Function(lf);
      ifcuts = true;
      ++sStats_.cuts;
#if SPEW
      {
      ConstraintPtr c = rel->newConstraint(f, -(x1l-x2l)*(x1l-x2l), INFINITY);
      logger_->msgStream(LogDebug2) << me_ << "new cut added" << std::endl;
      c->write(logger_->msgStream(LogDebug2));
      }
#else
      rel->newConstraint(f, -(x1l-x2l)*(x1l-x2l), INFINITY);
#endif
    } else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "Not adding cut because of "
                                    << "insufficient violation "
                                    << -(x1l-x2l)*(x1l-x2l) - lf_eval
                                    << std::endl;
#endif
    }
  }
}

bool UnivarQuadHandler::bwdPropBivar_(LinBivarPtr lbv, bool *changed) {
  VariablePtr y = lbv->y;
  VariablePtr x1 = lbv->x1;
  VariablePtr x2 = lbv->x2;
  double lx1x2 = lbv->pos ? x1->getLb() + x2->getLb() :
                            x1->getLb() - x2->getUb();
  double lbx1x2, ubx1x2, lbx1, ubx1, lbx2, ubx2;
  
  if (y->getUb() > bTol_) {
    ubx1x2 = sqrt(y->getUb());
    lbx1x2 = -ubx1x2;
    assert(y->getLb() >= 0.0);
    if (lx1x2 > -sqrt(y->getLb()) + bTol_) {
      lbx1x2 = sqrt(y->getLb());
    }
    lbx1 = lbv->pos ? lbx1x2 - x2->getUb() :
                      lbx1x2 + x2->getLb();
    ubx1 = lbv->pos ? ubx1x2 - x2->getLb() :
                      ubx1x2 + x2->getUb();
    lbx2 = lbv->pos ? lbx1x2 - x1->getUb() :
                      -ubx1x2 + x1->getLb();
    ubx2 = lbv->pos ? ubx1x2 - x1->getLb() :
                      -lbx1x2 + x1->getUb();
    if (updatePBounds_(x1, lbx1, ubx1, changed) < 0) {
      return true;
    }
    if (updatePBounds_(x2, lbx2, ubx2, changed) < 0) {
      return true;
    }
  } else if (y->getUb() < -bTol_) {
    return true;
  } else {
    lbx1 = lbv->pos ? -x2->getUb() : x2->getLb();
    ubx1 = lbv->pos ? -x2->getLb() : x2->getUb();
    lbx2 = lbv->pos ? -x1->getUb() : x1->getLb();
    ubx2 = lbv->pos ? -x1->getLb() : x1->getUb();
    if (updatePBounds_(x1, lbx1, ubx1, changed) < 0) {
      return true;
    }
    if (updatePBounds_(x2, lbx2, ubx2, changed) < 0) {
      return true;
    }
  }
  return false;
}

bool UnivarQuadHandler::bwdPropBivar_(LinBivarPtr lbv, RelaxationPtr rel,
                                       bool mod_rel, bool *changed,
                                       ModVector &p_mods, ModVector &r_mods) {
  VariablePtr y = lbv->y;
  VariablePtr x1 = lbv->x1;
  VariablePtr x2 = lbv->x2;
  double lx1x2 = lbv->pos ? x1->getLb() + x2->getLb() :
                            x1->getLb() - x2->getUb();
  double lbx1x2, ubx1x2, lbx1, ubx1, lbx2, ubx2;
  
  if (y->getUb() > bTol_) {
    ubx1x2 = sqrt(y->getUb());
    lbx1x2 = -ubx1x2;
    assert(y->getLb() >= 0.0);
    if (lx1x2 > -sqrt(y->getLb()) + bTol_) {
      lbx1x2 = sqrt(y->getLb());
    }
    lbx1 = lbv->pos ? lbx1x2 - x2->getUb() :
                      lbx1x2 + x2->getLb();
    ubx1 = lbv->pos ? ubx1x2 - x2->getLb() :
                      ubx1x2 + x2->getUb();
    lbx2 = lbv->pos ? lbx1x2 - x1->getUb() :
                      -ubx1x2 + x1->getLb();
    ubx2 = lbv->pos ? ubx1x2 - x1->getLb() :
                      -lbx1x2 + x1->getUb();
    if (updatePBounds_(x1, lbx1, ubx1, rel, mod_rel, changed, p_mods,
                       r_mods) < 0) {
      return true;
    }
    if (updatePBounds_(x2, lbx2, ubx2, rel, mod_rel, changed, p_mods,
                       r_mods) < 0) {
      return true;
    }
  } else if (y->getUb() < -bTol_) {
    return true;
  } else {
    lbx1 = lbv->pos ? -x2->getUb() : x2->getLb();
    ubx1 = lbv->pos ? -x2->getLb() : x2->getUb();
    lbx2 = lbv->pos ? -x1->getUb() : x1->getLb();
    ubx2 = lbv->pos ? -x1->getLb() : x1->getUb();
    if (updatePBounds_(x1, lbx1, ubx1, rel, mod_rel, changed, p_mods,
                       r_mods) < 0) {
      return true;
    }
    if (updatePBounds_(x2, lbx2, ubx2, rel, mod_rel, changed, p_mods,
                       r_mods) < 0) {
      return true;
    }
  }
  return false;
}

bool UnivarQuadHandler::bwdPropUnivar_(LinUnivarPtr luv, bool *changed) {
  double a = luv->a;
  double b = luv->b;
  double ly = luv->y->getLb();
  double uy = luv->y->getUb();
  VariablePtr x = luv->x;
  double lx = x->getLb();
  double ux = x->getUb();
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
        lb = (-b - sqrt(delta))/(2.0*a);
        ub = (-b + sqrt(delta))/(2.0*a);
        delta = b*b + 4.0*a*uy;
        if (delta > aTol_) { /// upper bound is less than maxima
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
  if (updatePBounds_(x, lb, ub, changed) < 0) {
    return true;
  }
 return false; 
}

bool UnivarQuadHandler::bwdPropUnivar_(LinUnivarPtr luv, RelaxationPtr rel,
                                       bool mod_rel, bool *changed,
                                       ModVector &p_mods, ModVector &r_mods) {
  double a = luv->a;
  double b = luv->b;
  double ly = luv->y->getLb();
  double uy = luv->y->getUb();
  VariablePtr x = luv->x;
  double lx = x->getLb();
  double ux = x->getUb();
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
        lb = (-b - sqrt(delta))/(2.0*a);
        ub = (-b + sqrt(delta))/(2.0*a);
        delta = b*b + 4.0*a*uy;
        if (delta > aTol_) { /// upper bound is less than maxima
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
  if (updatePBounds_(x, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
    return true;
  }
 return false; 
}

double UnivarQuadHandler::calcUpperUnivar_(double a, double b,
                                           double lx, double ux) {
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

void UnivarQuadHandler::findLinPt_(double xval, double yval,
                             LinUnivarPtr luv, double &xl) {
  double a = luv->a;
  double b = luv->b;
  double dx, ddx, step;
  double errlim = 1e-4;

  xl = xval;
  dx = (xl - xval) + (a*xl*xl + b*xl - yval)*(2*a*xl + b);
  ddx = 1 + (2*a*xl + b)*(2*a*xl + b) + 2*a*(a*xl*xl + b*xl - yval);
  while (fabs(dx) > errlim) {
    step = -dx/ddx;
    xl += step;
    dx = (xl - xval) + (a*xl*xl + b*xl - yval)*(2*a*xl + b);
    ddx = 1 + (2*a*xl + b)*(2*a*xl + b) + 2*a*(a*xl*xl + b*xl - yval);
  }
}

bool UnivarQuadHandler::fwdPropBivar_(LinBivarPtr lbv, bool *changed) {
  double lx1 = lbv->x1->getLb();
  double lx2 = lbv->x2->getLb();
  double ux1 = lbv->x1->getUb();
  double ux2 = lbv->x2->getUb();
  double lb, ub, ly, uy;

  if (lbv->pos) {
    lb = lx1 + lx2;
    ub = ux1 + ux2;
  } else {
    lb = lx1 - ux2;
    ub = ux1 - lx2;
  }

  BoundsOnSquare(lb, ub, ly, uy);
  if (updatePBounds_(lbv->y, ly, uy, changed) < 0) {
    return true;
  }
  return false;
}

bool UnivarQuadHandler::fwdPropBivar_(LinBivarPtr lbv, RelaxationPtr rel,
                                       bool mod_rel, bool *changed,
                                       ModVector &p_mods, ModVector &r_mods) {
  double lx1 = lbv->x1->getLb();
  double lx2 = lbv->x2->getLb();
  double ux1 = lbv->x1->getUb();
  double ux2 = lbv->x2->getUb();
  double lb, ub, ly, uy;

  if (lbv->pos) {
    lb = lx1 + lx2;
    ub = ux1 + ux2;
  } else {
    lb = lx1 - ux2;
    ub = ux1 - lx2;
  }

  BoundsOnSquare(lb, ub, ly, uy);
  if (updatePBounds_(lbv->y, ly, uy, rel, mod_rel, changed, p_mods,
                     r_mods) < 0) {
    return true;
  }
  return false;
}

bool UnivarQuadHandler::fwdPropUnivar_(LinUnivarPtr luv, bool *changed)
{
  VariablePtr y   = luv->y;
  double lb, ub;
  double a = luv->a;
  double b = luv->b;
  double lx = luv->x->getLb();
  double ux = luv->x->getUb();

  if (fabs(a) < aTol_) { // if a is zero than this is a linear term.
    if (lx*b > ux*b) {
      lb = ux*b;
      ub = lx*b;
    } else {
      ub = ux*b;
      lb = lx*b;
    }
  } else {
    if (lx > -aTol_) {
      ub = calcUpperUnivar_(a,b,lx,ux);
      lb = -calcUpperUnivar_(-a,-b,lx,ux);
    } else if (ux < aTol_) {
      ub = calcUpperUnivar_(a,-b,-ux,-lx);
      lb = -calcUpperUnivar_(-a,b,-ux,-lx);
    } else {
      ub = calcUpperUnivar_(a,b,0.0,ux);
      ub = std::max(ub, calcUpperUnivar_(a,-b,0.0,-lx));
      lb = -calcUpperUnivar_(-a,-b,0.0,ux);
      lb = std::min(lb, -calcUpperUnivar_(-a,b,0.0,-lx));
    }
  }

  if (updatePBounds_(y, lb, ub, changed) < 0) {
    return true;
  } 
  return false;
}

bool UnivarQuadHandler::fwdPropUnivar_(LinUnivarPtr luv, RelaxationPtr rel,
                               bool mod_rel, bool *changed, ModVector &p_mods,
                               ModVector &r_mods) {
  VariablePtr y   = luv->y;
  double lb, ub;
  double a = luv->a;
  double b = luv->b;
  double lx = luv->x->getLb();
  double ux = luv->x->getUb();

  if (fabs(a) < aTol_) { // if a is zero than this is a linear term.
    if (lx*b > ux*b) {
      lb = ux*b;
      ub = lx*b;
    } else {
      ub = ux*b;
      lb = lx*b;
    }
  } else {
    if (lx > -aTol_) {
      ub = calcUpperUnivar_(a,b,lx,ux);
      lb = -calcUpperUnivar_(-a,-b,lx,ux);
    } else if (ux < aTol_) {
      ub = calcUpperUnivar_(a,-b,-ux,-lx);
      lb = -calcUpperUnivar_(-a,b,-ux,-lx);
    } else {
      ub = calcUpperUnivar_(a,b,0.0,ux);
      ub = std::max(ub, calcUpperUnivar_(a,-b,0.0,-lx));
      lb = -calcUpperUnivar_(-a,-b,0.0,ux);
      lb = std::min(lb, -calcUpperUnivar_(-a,b,0.0,-lx));
    }
  }
  if (updatePBounds_(y, lb, ub, rel, mod_rel, changed, p_mods, r_mods)<0) {
    return true;
  }
  return false;
}

Branches UnivarQuadHandler::getBranches(BrCandPtr cand, DoubleVector &x,
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

void UnivarQuadHandler::getBranchingCandidates(RelaxationPtr rel,
                                         const DoubleVector &x, ModVector &,
                                         BrVarCandSet &cands,
                                         BrCandVector &, bool &is_inf)
{
  double yval, x1val, x2val, udist, ddist;
  BrVarCandPtr br_can;
  VariablePtr y, x1, x2;
  std::pair<BrVarCandIter, bool> ret;
  bool check;
  double vio;

  is_inf = false;

  // First check if there is a candidate x1 that violates y <= a*x1^2 + b*x1.
  for (LinUnivarVecIter it=univarFuns_.begin(); it!=univarFuns_.end(); ++it) {
    x1 = rel->getRelaxationVar((*it)->x);
    x1val = x[x1->getIndex()];
    y = rel->getRelaxationVar((*it)->y);
    yval = x[y->getIndex()];
    vio = yval - (*it)->a*x1val*x1val - (*it)->b*x1val;
    if (vio > fabs(yval)*rTol_ && vio > aTol_) {
      if (false == isAtBnds_(x1, x1val)) {
#if SPEW
        logger_->msgStream(LogDebug2) << std::setprecision(9) << me_ 
          << "branching candidate for LinUnivar: " << (*it)->x->getName()
          << " value = " << x1val << " aux var: " 
          << (*it)->y->getName() << " value = " << yval << std::endl;
#endif
        ddist = vio/sqrt(1.0+(*it)->a*(x1->getLb()+x1val)*(x1->getLb()+x1val)
                            +(*it)->b);
        udist = vio/sqrt(1.0+(*it)->a*(x1->getUb()+x1val)*(x1->getUb()+x1val)
                            +(*it)->b);
        br_can = (BrVarCandPtr) new BrVarCand(x1, x1->getIndex(), ddist,
                                              udist);
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getDDist());
        }
      } else {
        x1val = getBranchingPt_(vio, x1->getLb(), x1->getUb(),
                                fabs(yval)*rTol_);
#if SPEW
        logger_->msgStream(LogDebug2) << std::setprecision(9) << me_ 
          << "branching candidate for LinUnivar: " << (*it)->x->getName()
          << " value = " << x1val << " aux var: " 
          << (*it)->y->getName() << " value = " << yval << std::endl;
#endif
        ddist = vio/sqrt(1.0+(*it)->a*(x1->getLb()+x1val)*(x1->getLb()+x1val)
                            +(*it)->b);
        udist = vio/sqrt(1.0+(*it)->a*(x1->getUb()+x1val)*(x1->getUb()+x1val)
                            +(*it)->b);
        br_can = (BrVarCandPtr) new BrVarCand(x1, x1->getIndex(), ddist,
                                              udist);
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getDDist());
        }
        //logger_->msgStream(LogError) << std::setprecision(15) << me_ 
        // << "variable is at bounds, but we still want to branch on "
        // << "a univar constraint. " << x1->getName() << " = " << x1val
        // << " a = " << (*it)->a << " b = " << (*it)->b << " auxvar "
        // << y->getName() << " = " << yval  << std::endl;
      }
    }
  }

  // Now check if there is a violated constraint from bivar.
  // If so, add both x1 and x2 to the candidate set.
  for (LinBivarVecIter it=bivarFuns_.begin(); it!=bivarFuns_.end(); ++it) {
    x1 = rel->getRelaxationVar((*it)->x1);
    x2 = rel->getRelaxationVar((*it)->x2);
    x1val = x[x1->getIndex()];
    x2val = x[x2->getIndex()];
    y = rel->getRelaxationVar((*it)->y);
    yval  = x[y->getIndex()];
    vio = (*it)->pos ? yval - (x1val + x2val)*(x1val + x2val)
                  : yval - (x1val - x2val)*(x1val - x2val);
    if (vio > fabs(yval)*rTol_ && vio > aTol_) {
      check = false;
      // If a variable is at bounds, then it is not a candidate.
      if (false==isAtBnds_(x1, x1val)) {
        check = true;
        ddist = (*it)->pos ? std::min(x1val - x1->getLb(),
                                   x1val + sqrt(yval) + x2val)
                        : std::min(x1val - x1->getLb(),
                                   x1val - sqrt(yval) + x2val);
        udist = (*it)->pos ? std::min(x1->getUb() - x1val,
                                   x1val - sqrt(yval) + x2val)
                        : std::min(x1->getUb() - x1val,
                                   x1val + sqrt(yval) - x2val);
        br_can = (BrVarCandPtr) new BrVarCand(x1, x1->getIndex(), ddist,
                                              udist); 
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getUDist());
        }
      }

      if (false==isAtBnds_(x2, x2val)) {
        check = true;
        ddist = (*it)->pos ? std::min(x2val - x2->getLb(),
                                   x2val + sqrt(yval) + x1val)
                        : std::min(x2val - x2->getLb(),
                                   x2val + sqrt(yval) - x1val);
        udist = (*it)->pos ? std::min(x2->getUb() - x2val,
                                   x2val - sqrt(yval) + x1val)
                        : std::min(x2->getUb() - x2val,
                                   x2val - sqrt(yval) - x1val);
        br_can = (BrVarCandPtr) new BrVarCand(x2, x2->getIndex(), ddist,
                                              udist); 
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getUDist());
        }
      }
#if SPEW
    logger_->msgStream(LogDebug2) << std::setprecision(9) << me_ 
    << "branching candidate for bivar: " << x1->getName()
    << " = " << x1val << " " << x2->getName() << " = " << x2val 
    << " " << y->getName() << " = " << yval << " vio = " 
    << fabs(vio) << std::endl;
#endif
      if (false==check) {
        x1val = getBranchingPt_(vio, x1->getLb(), x1->getUb(),
                                fabs(yval)*rTol_);
        x2val = getBranchingPt_(vio, x2->getLb(), x2->getUb(),
                                fabs(yval)*rTol_);
        ddist = (*it)->pos ? std::min(x1val - x1->getLb(),
                                   x1val + sqrt(yval) + x2val)
                        : std::min(x1val - x1->getLb(),
                                   x1val - sqrt(yval) + x2val);
        udist = (*it)->pos ? std::min(x1->getUb() - x1val,
                                   x1val - sqrt(yval) + x2val)
                        : std::min(x1->getUb() - x1val,
                                   x1val + sqrt(yval) - x2val);
        br_can = (BrVarCandPtr) new BrVarCand(x1, x1->getIndex(), ddist,
                                              udist); 
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getUDist());
        }
        
        ddist = (*it)->pos ? std::min(x2val - x2->getLb(),
                                   x2val + sqrt(yval) + x1val)
                        : std::min(x2val - x2->getLb(),
                                   x2val + sqrt(yval) - x1val);
        udist = (*it)->pos ? std::min(x2->getUb() - x2val,
                                   x2val - sqrt(yval) + x1val)
                        : std::min(x2->getUb() - x2val,
                                   x2val - sqrt(yval) - x1val);
        br_can = (BrVarCandPtr) new BrVarCand(x2, x2->getIndex(), ddist,
                                              udist); 
        ret = cands.insert(br_can);
        if (false == ret.second) { // already exists.
          br_can = *(ret.first);
          br_can->setDist(ddist+br_can->getDDist(), udist+br_can->getUDist());
        }
        //logger_->msgStream(LogError) << std::setprecision(15) << me_ 
        // << "both variables are at bounds, but we still want to branch on "
        // << "a bivar constraint. " << x1->getName() << " = " << x1val
        // << " " << x2->getName() << " = " << x2val << " " << y->getName()
        // << " = " << yval  << std::endl;
      }
    }
  }
}

double UnivarQuadHandler::getBranchingPt_(double vio, double lb, double ub,
                                          double allowed_vio) {
  double ratio = vio/allowed_vio;
  return lb + (ub - lb)/ratio;
}

std::string UnivarQuadHandler::getName() const
{
  return "UnivarQuadHandler (Handling univariate quadratic expressions).";
}

LinearFunctionPtr UnivarQuadHandler::getSecant_(VariablePtr y, VariablePtr x,
                                                double a, double b) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  double x_coeff = a*(x->getLb() + x->getUb()) + b;
  double eps = 1e-5; // we don't want a coeff smaller than this in our LP
  
  if (fabs(x_coeff) >= eps) {
    lf->addTerm(y, 1.0);
    lf->addTerm(x, -x_coeff);
  } else {
    lf->addTerm(y, 1.0);
#if SPEW
    logger_->msgStream(LogDebug) << me_ << 
      "warning: generating a bound as a secant constraint." << std::endl;
#endif
  }
  return lf;
}

std::vector<LinearFunctionPtr> UnivarQuadHandler::getSecant_(VariablePtr y,
                VariablePtr x1, VariablePtr x2, bool pos, DoubleVector &rhs) {
  std::vector<LinearFunctionPtr> lfs(2);
  LinearFunctionPtr lf;
  FunctionPtr f;
  double alpha1, alpha2;
  DoubleVector fns(4);
  UInt mind;
  DoubleVector::iterator miter;
  double mfn, r;
  double l1 = x1->getLb(), u1 = x1->getUb();
  double l2 = x2->getLb(), u2 = x2->getUb();

  if (fabs(u1 - l1) < bTol_) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.0);
    r = pos ? u2 + l2 + u1 + l1 : u2 + l2 - u1 - l1;
    lf->addTerm(x2, -r);
    lfs[0] = lf;
    r = (l1 + u1)/2.0;
    rhs[0] = r*r - u2*l2;
    lfs[1] = lf;
    rhs[1] = rhs[0];
    return lfs;
  }

  if (fabs(u2 - l2) < bTol_) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.0);
    r = pos ? u1 + l1 + u2 + l2 : u1 + l1 - u2 - l2;
    lf->addTerm(x2, -r);
    lfs[0] = lf;
    r = (l2 + u2)/2.0;
    rhs[0] = r*r - u1*l1;
    lfs[1] = lf;
    rhs[1] = rhs[0];
    return lfs;
  }

  fns[0] = pos ? pow(l1 + l2, 2) : pow(l1 - l2, 2);
  fns[1] = pos ? pow(l1 + u2, 2) : pow(l1 - u2, 2);
  fns[2] = pos ? pow(u1 + l2, 2) : pow(u1 - l2, 2);
  fns[3] = pos ? pow(u1 + u2, 2) : pow(u1 - u2, 2);
  miter = std::max_element(fns.begin(), fns.end());
  mfn = *miter;
  mind = miter - fns.begin();
  switch(mind) {
    case 0:
      r = mfn - fns[3];
      alpha1 = (mfn - fns[2])/(u1 - l1);
      alpha2 = (mfn - fns[1])/(u2 - l2);
      if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 < r - aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, alpha1);
        lf->addTerm(x2, alpha2);
        lfs[0] = lf;
        rhs[0] = mfn + alpha1*l1 + alpha2*l2;
        miter = std::min_element(fns.begin(), fns.end());
        mfn = *miter;
        mind = miter - fns.begin();
        switch(mind) {
          case 0:
            alpha1 = (fns[2] - mfn)/(u1 - l1);
            alpha2 = (fns[1] - mfn)/(u2 - l2);
            r = fns[3] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 - alpha2*l2;
            break;
          case 1:
            alpha1 = (fns[3] - mfn)/(u1 - l1);
            alpha2 = (fns[0] - mfn)/(u2 - l2);
            r = fns[2] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 + alpha2*u2;
            break;
          case 2:
            alpha1 = (fns[0] - mfn)/(u1 - l1);
            alpha2 = (fns[3] - mfn)/(u2 - l2);
            r = fns[1] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 - alpha2*l2;
            break;
          case 3:
            alpha1 = (fns[1] - mfn)/(u1 - l1);
            alpha2 = (fns[2] - mfn)/(u2 - l2);
            r = fns[0] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 + alpha2*u2;
            break;
          default:
            assert(!"mind greater than 3");
            break;
        }
      } else if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 > r + aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, alpha1);
        lf->addTerm(x2, (r - (u1 - l1)*alpha1)/(u2 - l2));
        lfs[0] = lf;
        rhs[0] = mfn + alpha1*l1 + (r - (u1 - l1)*alpha1)*l2/(u2 - l2);
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, (r - (u2 - l2)*alpha2)/(u1 - l1));
        lf->addTerm(x2, alpha2);
        lfs[1] = lf;
        rhs[1] = mfn + alpha2*l2 + (r - (u2 - l2)*alpha2)*l1/(u1 - l1);
      } else {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, alpha1);
        lf->addTerm(x2, alpha2);
        lfs[0] = lf;
        rhs[0] = mfn + alpha1*l1 + alpha2*l2;
        lfs[1] = lf;
        rhs[1] = rhs[0];
      }
      break;
    case 1:
      r = mfn - fns[2];
      alpha1 = (mfn - fns[3])/(u1 - l1);
      alpha2 = (mfn - fns[0])/(u2 - l2);
      if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 < r - aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, alpha1);
        lf->addTerm(x2, -alpha2);
        lfs[0] = lf;
        rhs[0] = mfn + alpha1*l1 - alpha2*u2;
        miter = std::min_element(fns.begin(), fns.end());
        mfn = *miter;
        mind = miter - fns.begin();
        switch(mind) {
          case 0:
            alpha1 = (fns[2] - mfn)/(u1 - l1);
            alpha2 = (fns[1] - mfn)/(u2 - l2);
            r = fns[3] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 - alpha2*l2;
            break;
          case 1:
            alpha1 = (fns[3] - mfn)/(u1 - l1);
            alpha2 = (fns[0] - mfn)/(u2 - l2);
            r = fns[2] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 + alpha2*u2;
            break;
          case 2:
            alpha1 = (fns[0] - mfn)/(u1 - l1);
            alpha2 = (fns[3] - mfn)/(u2 - l2);
            r = fns[1] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 - alpha2*l2;
            break;
          case 3:
            alpha1 = (fns[1] - mfn)/(u1 - l1);
            alpha2 = (fns[2] - mfn)/(u2 - l2);
            r = fns[0] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 + alpha2*u2;
            break;
          default:
            assert(!"mind greater than 3");
            break;
        }
      } else if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 > r + aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, alpha1);
        lf->addTerm(x2, -(r - (u1 - l1)*alpha1)/(u2 - l2));
        lfs[0] = lf;
        rhs[0] = mfn + alpha1*l1 - (r - (u1 - l1)*alpha1)*u2/(u2 - l2);
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, (r - (u2 - l2)*alpha2)/(u1 - l1));
        lf->addTerm(x2, -alpha2);
        lfs[1] = lf;
        rhs[1] = mfn - alpha2*u2 + (r - (u2 - l2)*alpha2)*l1/(u1 - l1);
      } else {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, alpha1);
        lf->addTerm(x2, -alpha2);
        lfs[0] = lf;
        rhs[0] = mfn + alpha1*l1 - alpha2*u2;
        lfs[1] = lf;
        rhs[1] = rhs[0];
      }
      break;
    case 2:
      r = mfn - fns[1];
      alpha1 = (mfn - fns[0])/(u1 - l1);
      alpha2 = (mfn - fns[3])/(u2 - l2);
      if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 < r - aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -alpha1);
        lf->addTerm(x2, alpha2);
        lfs[0] = lf;
        rhs[0] = mfn - alpha1*u1 + alpha2*l2;
        miter = std::min_element(fns.begin(), fns.end());
        mfn = *miter;
        mind = miter - fns.begin();
        switch(mind) {
          case 0:
            alpha1 = (fns[2] - mfn)/(u1 - l1);
            alpha2 = (fns[1] - mfn)/(u2 - l2);
            r = fns[3] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 - alpha2*l2;
            break;
          case 1:
            alpha1 = (fns[3] - mfn)/(u1 - l1);
            alpha2 = (fns[0] - mfn)/(u2 - l2);
            r = fns[2] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 + alpha2*u2;
            break;
          case 2:
            alpha1 = (fns[0] - mfn)/(u1 - l1);
            alpha2 = (fns[3] - mfn)/(u2 - l2);
            r = fns[1] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 - alpha2*l2;
            break;
          case 3:
            alpha1 = (fns[1] - mfn)/(u1 - l1);
            alpha2 = (fns[2] - mfn)/(u2 - l2);
            r = fns[0] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 + alpha2*u2;
            break;
          default:
            assert(!"mind greater than 3");
            break;
        }
      } else if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 > r + aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -alpha1);
        lf->addTerm(x2, (r - (u1 - l1)*alpha1)/(u2 - l2));
        lfs[0] = lf;
        rhs[0] = mfn - alpha1*u1 + (r - (u1 - l1)*alpha1)*l2/(u2 - l2);
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -(r - (u2 - l2)*alpha2)/(u1 - l1));
        lf->addTerm(x2, alpha2);
        lfs[1] = lf;
        rhs[1] = mfn + alpha2*l2 - (r - (u2 - l2)*alpha2)*u1/(u1 - l1);
      } else {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -alpha1);
        lf->addTerm(x2, alpha2);
        lfs[0] = lf;
        rhs[0] = mfn - alpha1*u1 + alpha2*l2;
        lfs[1] = lf;
        rhs[1] = rhs[0];
      }
      break;
    case 3:
      r = mfn - fns[0];
      alpha1 = (mfn - fns[1])/(u1 - l1);
      alpha2 = (mfn - fns[2])/(u2 - l2);
      if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 < r - aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -alpha1);
        lf->addTerm(x2, -alpha2);
        lfs[0] = lf;
        rhs[0] = mfn - alpha1*u1 - alpha2*u2;
        miter = std::min_element(fns.begin(), fns.end());
        mfn = *miter;
        mind = miter - fns.begin();
        switch(mind) {
          case 0:
            alpha1 = (fns[2] - mfn)/(u1 - l1);
            alpha2 = (fns[1] - mfn)/(u2 - l2);
            r = fns[3] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 - alpha2*l2;
            break;
          case 1:
            alpha1 = (fns[3] - mfn)/(u1 - l1);
            alpha2 = (fns[0] - mfn)/(u2 - l2);
            r = fns[2] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, -alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn - alpha1*l1 + alpha2*u2;
            break;
          case 2:
            alpha1 = (fns[0] - mfn)/(u1 - l1);
            alpha2 = (fns[3] - mfn)/(u2 - l2);
            r = fns[1] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, -alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 - alpha2*l2;
            break;
          case 3:
            alpha1 = (fns[1] - mfn)/(u1 - l1);
            alpha2 = (fns[2] - mfn)/(u2 - l2);
            r = fns[0] - mfn;
            r -= (u1 - l1)*alpha1 + (u2 - l2)*alpha2;
            if (r > 0.0) {
              if ((u1 - l1) >= (u2 - l2)) {
                alpha1 += r/(u1 - l1);
              } else {
                alpha2 += r/(u2 - l2);
              }
            }
            lf = (LinearFunctionPtr) new LinearFunction();
            lf->addTerm(y, 1.0);
            lf->addTerm(x1, alpha1);
            lf->addTerm(x2, alpha2);
            lfs[1] = lf;
            rhs[1] = mfn + alpha1*u1 + alpha2*u2;
            break;
          default:
            assert(!"mind greater than 3");
            break;
        }
      } else if ((u1 - l1)*alpha1 + (u2 - l2)*alpha2 > r + aTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -alpha1);
        lf->addTerm(x2, -(r - (u1 - l1)*alpha1)/(u2 - l2));
        lfs[0] = lf;
        rhs[0] = mfn - alpha1*u1 - (r - (u1 - l1)*alpha1)*u2/(u2 - l2);
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -(r - (u2 - l2)*alpha2)/(u1 - l1));
        lf->addTerm(x2, -alpha2);
        lfs[1] = lf;
        rhs[1] = mfn - alpha2*u2 - (r - (u2 - l2)*alpha2)*u1/(u1 - l1);
      } else {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, 1.0);
        lf->addTerm(x1, -alpha1);
        lf->addTerm(x2, -alpha2);
        lfs[0] = lf;
        rhs[0] = mfn - alpha1*u1 + alpha2*u2;
        lfs[1] = lf;
        rhs[1] = rhs[0];
      }
      break;
    default:
      break;
  }
  return lfs;
}

LinearFunctionPtr UnivarQuadHandler::getTangent_(VariablePtr y, VariablePtr x,
                                          double a, double b, double pt) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  double x_coeff = 2.0*a*pt + b;

  lf->addTerm(y, 1.0);
  lf->addTerm(x, -x_coeff);
  return lf;
}

LinearFunctionPtr UnivarQuadHandler::getTangent_(VariablePtr y, VariablePtr x1,
                                                 VariablePtr x2, bool pos,
                                                 UInt i, double &rhs) {
  double fn, g1, g2;
  double l1 = x1->getLb(), u1 = x1->getUb();
  double l2 = x2->getLb(), u2 = x2->getUb();
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

  lf->addTerm(y, 1.0);
  switch(i) {
    case 0:
      fn = pos ? pow(l1 + l2, 2) : pow(l1 - l2, 2);
      g1 = pos ? 2.0*(l1 + l2) : 2.0*(l1 - l2);
      g2 = pos ? 2.0*(l1 + l2) : 2.0*(l2 - l1);
      lf->addTerm(x1, -g1);
      lf->addTerm(x2, -g2);
      rhs = fn - g1*l1 - g2*l2;
      break;
    case 1:
      fn = pos ? pow(l1 + u2, 2) : pow(l1 - u2, 2);
      g1 = pos ? 2.0*(l1 + u2) : 2.0*(l1 - u2);
      g2 = pos ? 2.0*(l1 + u2) : 2.0*(u2 - l1);
      lf->addTerm(x1, -g1);
      lf->addTerm(x2, -g2);
      rhs = fn - g1*l1 - g2*u2;
      break;
    case 2:
      fn = pos ? pow(u1 + l2, 2) : pow(u1 - l2, 2);
      g1 = pos ? 2.0*(u1 + l2) : 2.0*(u1 - l2);
      g2 = pos ? 2.0*(u1 + l2) : 2.0*(l2 - u1);
      lf->addTerm(x1, -g1);
      lf->addTerm(x2, -g2);
      rhs = fn - g1*u1 - g2*l2;
      break;
    case 3:
      fn = pos ? pow(u1 + u2, 2) : pow(u1 - u2, 2);
      g1 = pos ? 2.0*(u1 + u2) : 2.0*(u1 - u2);
      g2 = pos ? 2.0*(u1 + u2) : 2.0*(u2 - u1);
      lf->addTerm(x1, -g1);
      lf->addTerm(x2, -g2);
      rhs = fn - g1*u1 - g2*u2;
      break;
  }
  return lf;
}

bool UnivarQuadHandler::isAtBnds_(ConstVariablePtr x, double xval)
{
  double lb = x->getLb();
  double ub = x->getUb();
  return(fabs(xval-lb) < bTol_ || fabs(xval-ub) < bTol_);
}

bool UnivarQuadHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                             double &inf_meas)
{
  double x1val, x2val, yval, vio;
  const double *x = sol->getPrimal();
  bool is_feas = true;

  for (LinUnivarVecIter it=univarFuns_.begin(); it !=univarFuns_.end(); ++it) {
    // check if y <= x^2
    x1val  = x[(*it)->x->getIndex()];
    yval = x[(*it)->y->getIndex()];
    vio = fabs(yval - (*it)->a*x1val*x1val - (*it)->b*x1val);
    if (vio > fabs(yval)*rTol_ && vio > aTol_) {
      is_feas = false;
      inf_meas += vio;
    }
  }
#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "no branching candidates for Univar" 
                                << std::endl;
#endif

  for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
    x1val = x[(*it)->x1->getIndex()];
    x2val = x[(*it)->x2->getIndex()];
    yval = x[(*it)->y->getIndex()];
    vio = (*it)->pos ? fabs(yval - (x1val + x2val)*(x1val + x2val)) :
                    fabs(yval - (x1val - x2val)*(x1val - x2val));
    if (vio > fabs(yval)*rTol_ && vio > aTol_) {
      is_feas = false;
      inf_meas += vio;
    }
  }

#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "no branching candidates for Bivar" 
                                << std::endl;
#endif
  return is_feas;
}

bool UnivarQuadHandler::isViolatedAtBounds_(ConstraintPtr c1, ConstraintPtr c2,
                      VariablePtr x1, VariablePtr x2, double l1, double u1,
                      double l2, double u2, bool pos) {
  LinearFunctionPtr lf1 = c1->getLinearFunction();
  LinearFunctionPtr lf2 = c2->getLinearFunction();;
  double x1w1 = lf1->getWeight(x1);
  double x2w1 = lf1->getWeight(x2);
  double x1w2 = lf2->getWeight(x1);
  double x2w2 = lf2->getWeight(x2);
  double eps = aTol_/10.0;
  
  if (pos) {
    // check at (l1, l2)
    if ((l1 + l2)*(l1 + l2) + x1w1*l1 + x2w1*l2 < c1->getUb() - eps && 
       (l1 + l2)*(l1 + l2) + x1w2*l1 + x2w2*l2 < c2->getUb() - eps) {
          return true;
    }
    // check at (l1, u2)
    if ((l1 + u2)*(l1 + u2) + x1w1*l1 + x2w1*u2 < c1->getUb() - eps &&
        (l1 + u2)*(l1 + u2) + x1w2*l1 + x2w2*u2 < c2->getUb() - eps) {
          return true;
    }
    // check at (u1, l2)
    if ((u1 + l2)*(u1 + l2) + x1w1*u1 + x2w1*l2 < c1->getUb() - eps &&
        (u1 + l2)*(u1 + l2) + x1w2*u1 + x2w2*l2 < c2->getUb() - eps) {
          return true;
    }
    // check at (u1, u2)
    if ((u1 + u2)*(u1 + u2) + x1w1*u1 + x2w1*u2 < c1->getUb() - eps &&
        (u1 + u2)*(u1 + u2) + x1w2*u1 + x2w2*u2 < c2->getUb() - eps) {
          return true;
    }
  } else {
    // check at (l1, l2)
    if ((l1 - l2)*(l1 - l2) + x1w1*l1 + x2w1*l2 < c1->getUb() - eps &&
        (l1 - l2)*(l1 - l2) + x1w2*l1 + x2w2*l2 < c2->getUb() - eps) {
          return true;
    }
    // check at (l1, u2)
    if ((l1 - u2)*(l1 - u2) + x1w1*l1 + x2w1*u2 < c1->getUb() - eps &&
        (l1 - u2)*(l1 - u2) + x1w2*l1 + x2w2*u2 < c2->getUb() - eps) {
          return true;
    }
    // check at (u1, l2)
    if ((u1 - l2)*(u1 - l2) + x1w1*u1 + x2w1*l2 < c1->getUb() - eps &&
        (u1 - l2)*(u1 - l2) + x1w2*u1 + x2w2*l2 < c2->getUb() - eps) {
          return true;
    }
    // check at (u1, u2)
    if ((u1 - u2)*(u1 - u2) + x1w1*u1 + x2w1*u2 < c1->getUb() - eps &&
        (u1 - u2)*(u1 - u2) + x1w2*u1 + x2w2*u2 < c2->getUb() - eps) {
          return true;
    }
  }
  return false;
}

SolveStatus UnivarQuadHandler::presolve(PreModQ *, bool *changed)
{

  bool is_inf = false;
  SolveStatus status = Finished;

  *changed = false;

  is_inf = varBndsFromCons_(changed);
  if (is_inf) {
    status = SolvedInfeasible;
  }

  if (Started==status) {
    status = Finished;
  }

  return status;
}

bool UnivarQuadHandler::presolveNode(RelaxationPtr rel, NodePtr,
                               SolutionPoolPtr, ModVector &p_mods,
                               ModVector &r_mods)
{
  bool lchanged = true;
  bool changed = true;
  bool is_inf = false;
  double stime = timer_->query();

  // visit each constraint and see if bounds can be improved.
  while (true==changed) {
    ++pStats_.iters;
    changed = false;
    for (LinUnivarVecIter it = univarFuns_.begin(); it != univarFuns_.end();
         ++it) {
      is_inf = fwdPropUnivar_(*it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true==lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
    for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
      is_inf = fwdPropBivar_(*it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true==lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
    for (LinUnivarVecIter it = univarFuns_.begin(); it != univarFuns_.end();
         ++it) {
      is_inf = bwdPropUnivar_(*it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true==lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
    for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
      is_inf = bwdPropBivar_(*it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true==lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
  }

  for (LinUnivarVecIter it=univarFuns_.begin(); it != univarFuns_.end(); ++it) {
    upUnivarCon_((*it)->oeCon, rel->getRelaxationVar((*it)->x),
                 rel->getRelaxationVar((*it)->y), (*it)->a, (*it)->b,
                 rel, r_mods);
  }
  for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
    upBivarCon_(*it, rel, r_mods);
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

void UnivarQuadHandler::relax_(RelaxationPtr rel, bool *) {
  LinearFunctionPtr lf;
  VariablePtr y, x1, x2;
  FunctionPtr f;
  double rhs;
  std::vector<LinearFunctionPtr> lfs(2);
  DoubleVector rhs_vec(2);
  ConstraintVector cons(2);

  for (LinUnivarVecIter it=univarFuns_.begin(); it !=univarFuns_.end(); ++it) {
    y = rel->getRelaxationVar((*it)->y);
    x1 = rel->getRelaxationVar((*it)->x);
    if (x1->getLb() <= -INFINITY || x1->getUb() >= INFINITY) {
      logger_->errStream() << "can not relax " << x1->getName()
                           << "because the bounds on variables are too weak"
                           << std::endl;
      exit(500);
    }
    lf = getSecant_(y, x1, (*it)->a, (*it)->b);
    f = (FunctionPtr) new Function(lf);
    rhs = -(*it)->a*x1->getLb()*x1->getUb();
    if ((*it)->a >= 0) {
      (*it)->oeCon = rel->newConstraint(f, -INFINITY, rhs);
    } else {
      (*it)->oeCon = rel->newConstraint(f, rhs, INFINITY);
    }
    for (UInt i=0; i<2; ++i) {
      rhs = (1-i)*x1->getLb() + i*x1->getUb();
      lf = getTangent_(y, x1, (*it)->a, (*it)->b, rhs);
      f = (FunctionPtr) new Function(lf);
      rhs = -(*it)->a*rhs*rhs;
      if ((*it)->a >= 0) {
        cons[i] = rel->newConstraint(f, rhs, INFINITY);
      } else {
        cons[i] = rel->newConstraint(f, -INFINITY, rhs);
      }
    }
    (*it)->tan1 = cons[0];
    (*it)->tan2 = cons[1];
  }
  for (LinBivarVecIter it=bivarFuns_.begin(); it !=bivarFuns_.end(); ++it) {
    y = rel->getRelaxationVar((*it)->y);
    x1 = rel->getRelaxationVar((*it)->x1);
    x2 = rel->getRelaxationVar((*it)->x2);
    if (x1->getLb() <= -INFINITY || x1->getUb() >= INFINITY) {
      logger_->errStream() << "can not relax " << x1->getName()
                           << "because the bounds on variables are too weak"
                           << std::endl;
      exit(500);
    }
    if (x2->getLb() <= -INFINITY || x2->getUb() >= INFINITY) {
      logger_->errStream() << "can not relax " << x2->getName()
                           << "because the bounds on variables are too weak"
                           << std::endl;
      exit(500);
    }
    lfs = getSecant_(y, x1, x2, (*it)->pos, rhs_vec);
    f = (FunctionPtr) new Function(lfs[0]);
    (*it)->oeCon1 = rel->newConstraint(f, -INFINITY, rhs_vec[0]);
    f = (FunctionPtr) new Function(lfs[1]);
    (*it)->oeCon2 = rel->newConstraint(f, -INFINITY, rhs_vec[1]);
    cons.resize(4);
    for (UInt i=0; i<4; ++i) {
      lf = getTangent_(y, x1, x2, (*it)->pos, i, rhs);
      f = (FunctionPtr) new Function(lf);
      cons[i] = rel->newConstraint(f, rhs, INFINITY);
    }
    (*it)->tan1 = cons[0];
    (*it)->tan2 = cons[1];
    (*it)->tan3 = cons[2];
    (*it)->tan4 = cons[3];
    cons.resize(2);
  }
}

void UnivarQuadHandler::relaxInitFull(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}

void UnivarQuadHandler::relaxInitInc(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}

void UnivarQuadHandler::relaxNodeFull(NodePtr, RelaxationPtr, bool *)
{
  assert(!"UnivarQuadHandler::relaxNodeFull not implemented!");
}

void UnivarQuadHandler::relaxNodeInc(NodePtr, RelaxationPtr, bool *)
{
  // do nothing. Presolve will take care of tightening bounds
}

void UnivarQuadHandler::resetStats_()
{
  pStats_.iters  = 0;
  pStats_.time   = 0.0;
  pStats_.timeN  = 0.0;
  pStats_.vBnd   = 0;
  pStats_.nMods  = 0;
  
  sStats_.iters  = 0;
  sStats_.cuts   = 0;
  sStats_.time   = 0.0;
}

void UnivarQuadHandler::separate(ConstSolutionPtr sol, NodePtr ,
                           RelaxationPtr rel, CutManager *, SolutionPoolPtr ,
                           ModVector &, ModVector &,  bool *,
                           SeparationStatus *status)
{
  double yval, x1val, x2val;
  const double *x = sol->getPrimal();
  double x1l, x2l;
  bool ifcuts = false;

  ++sStats_.iters;
  for (LinUnivarVecIter it=univarFuns_.begin(); it !=univarFuns_.end(); ++it) {
    x1val = x[(*it)->x->getIndex()];
    yval = x[(*it)->y->getIndex()];
    if ((*it)->a > aTol_) {
      if ((*it)->a*x1val*x1val + (*it)->b*x1val - yval > rTol_*fabs(yval) &&
          fabs((*it)->a*x1val*x1val + (*it)->b*x1val - yval) > aTol_) {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << "xval = " << x1val
                            << " yval = " << yval << " violation = "
                            << (*it)->a*x1val*x1val + (*it)->b*x1val - yval
                            << std::endl;
#endif
        findLinPt_(x1val, yval, *it, x1l);
        addCut_(rel, *it, x1l, x1val, yval, ifcuts);
      }
    } else {
      if (yval - (*it)->a*x1val*x1val + (*it)->b*x1val > rTol_*fabs(yval) &&
          fabs((*it)->a*x1val*x1val + (*it)->b*x1val - yval) > aTol_) {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << "xval = " << x1val
                            << " yval = " << yval << " violation = "
                            << (*it)->a*x1val*x1val + (*it)->b*x1val - yval
                            << std::endl;
#endif
        findLinPt_(x1val, yval, *it, x1l);
        addCut_(rel, *it, x1l, x1val, yval, ifcuts);
      }
    }
  }

  for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
    x1val = x[(*it)->x1->getIndex()];
    x2val = x[(*it)->x2->getIndex()];
    yval = x[(*it)->y->getIndex()];
    if ((*it)->pos) {
      if ((x1val + x2val)*(x1val + x2val) - yval > rTol_*fabs(yval) &&
          fabs((x1val + x2val)*(x1val + x2val) - yval) > aTol_) {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << "x1val = " << x1val
                                      << " x2val = " << x2val
                                      << " yval = " << yval << " violation = "
                                      << (x1val + x2val)*(x1val + x2val) - yval
                                      << std::endl;
#endif
        x1l = x1val; /// Lin point is not calculated optimally
        x2l = x2val; /// Should do that.
        addCut_(rel, *it, x1l, x2l, x1val, x2val, yval, ifcuts);
      }
    } else {
      if ((x1val - x2val)*(x1val - x2val) - yval > rTol_*fabs(yval) &&
          fabs((x1val - x2val)*(x1val - x2val) - yval) > aTol_) {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << "x1val = " << x1val
                                      << " x2val = " << x2val
                                      << " yval = " << yval << " violation = "
                                      << (x1val - x2val)*(x1val - x2val) - yval
                                      << std::endl;
#endif
        x1l = x1val; /// Lin point is not calculated optimally
        x2l = x2val; /// Should do that.
        addCut_(rel, *it, x1l, x2l, x1val, x2val, yval, ifcuts);
      }
    }
  }
  if (true == ifcuts) {
    *status = SepaResolve;
  }
}

void UnivarQuadHandler::upBivarCon_(LinBivarPtr lbv, RelaxationPtr rel,
                                    ModVector &r_mods) {
  std::vector<LinearFunctionPtr> lfs(2);
  ConstraintPtr c1, c2;
  VariablePtr y = lbv->y;
  VariablePtr x1 = lbv->x1;
  VariablePtr x2 = lbv->x2;
  LinConModPtr lmod;
  double l1 = x1->getLb();
  double u1 = x1->getUb();
  double l2 = x2->getLb();
  double u2 = x2->getUb();
  DoubleVector rhs(2);

  c1 = lbv->oeCon1;
  c2 = lbv->oeCon2;

  if (isViolatedAtBounds_(c1, c2, x1, x2, l1, u1, l2, u2, lbv->pos)) {
    lfs = getSecant_(y, x1, x2, lbv->pos, rhs);
    lmod = (LinConModPtr) new LinConMod(c1, lfs[0], -INFINITY, rhs[0]);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
    //std::cout << "Bivar cons " << l1 << "    " << u1 << "    " << l2
    //          << "   " << u2 << std::endl;
    //std::cout << "lf1 : ";
    //lfs[0]->write(std::cout);
    //std::cout << "  rhs : " << rhs[0] << std::endl;
    lmod = (LinConModPtr) new LinConMod(c2, lfs[1], -INFINITY, rhs[1]);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
    //std::cout << "lf2 : ";
    //lfs[1]->write(std::cout);
    //std::cout << "  rhs : " << rhs[1] << std::endl;
  }
}

int UnivarQuadHandler::updatePBounds_(VariablePtr v, double lb, double ub,
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

int UnivarQuadHandler::updatePBounds_(VariablePtr v, double lb, double ub,
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

void UnivarQuadHandler::upUnivarCon_(ConstraintPtr con, VariablePtr x,
                                     VariablePtr y, double a, double b,
                                     RelaxationPtr rel, ModVector &r_mods) {
  LinearFunctionPtr lf = con->getLinearFunction();
  double a_x = lf->getWeight(x);
  double lb = x->getLb();
  double ub = x->getUb();
  LinConModPtr lmod;
  double rhs;
  double eps = aTol_/10.0;

  assert(fabs(lf->getWeight(y) - 1.0) <= 1e-8);
  if (a > aTol_) {
    // y - (a(lb+ub) + b)x <= -a*ub*lb
    if ((a*lb*lb + b*lb + a_x*lb < con->getUb() - eps) ||
        (a*ub*ub + b*ub + a_x*ub < con->getUb() - eps)) {
      lf = getSecant_(y, x, a, b);
      rhs = -a*ub*lb;
      //std::cout << "Univar x " << x->getName() << "   " <<
      //             a << "   " << b << "    " << ub << "   "
      //             << lb << std::endl;
      //lf->write(std::cout);
      //std::cout << "   rhs " << rhs << std::endl;
      lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
      lmod->applyToProblem(rel);
      r_mods.push_back(lmod);
    }
  } else {
    // y - (a(lb+ub) + b)x >= -a*ub*lb
    if ((a*lb*lb + b*lb + a_x*lb > con->getLb() + eps) ||
        (a*ub*ub + b*ub + a_x*ub > con->getLb() + eps)) {
      lf = getSecant_(y, x, a, b);
      rhs = -a*ub*lb;
      lmod = (LinConModPtr) new LinConMod(con, lf, rhs, INFINITY);
      lmod->applyToProblem(rel);
      r_mods.push_back(lmod);
    }
  }
}

bool UnivarQuadHandler::varBndsFromCons_(bool *changed)
{
  bool is_inf = false;
  for (LinUnivarVecIter it=univarFuns_.begin(); it !=univarFuns_.end(); ++it) {
    is_inf = fwdPropUnivar_(*it, changed);
    if (true==is_inf) {
      return true;
    }
  }
  for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
    is_inf = fwdPropBivar_(*it, changed);
    if (true==is_inf) {
      return true;
    }
  }
  for (LinUnivarVecIter it=univarFuns_.begin(); it !=univarFuns_.end(); ++it) {
    is_inf = bwdPropUnivar_(*it, changed);
    if (true==is_inf) {
      return true;
    }
  }
  for (LinBivarVecIter it=bivarFuns_.begin(); it != bivarFuns_.end(); ++it) {
    is_inf =bwdPropBivar_(*it, changed);
    if (true==is_inf) {
      return true;
    }
  }
  return false;
}


void UnivarQuadHandler::writeStats(std::ostream &out) const
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
