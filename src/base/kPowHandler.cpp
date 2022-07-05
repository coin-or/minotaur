//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2021 The MINOTAUR Team.
//

/**
 * \file kPowHandler.cpp
 * \brief Implement a handler for k power constraints of the form
 * \f$ y = x^k \f$,
 * where
 * \f$ k > 1 \f$,
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include "kPowHandler.h"

#include <string.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "BrVarCand.h"
#include "Branch.h"
#include "Constraint.h"
#include "CGraph.h"
#include "CNode.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "LinMods.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
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

const std::string kPowHandler::me_ = "kPowHandler: ";

kPowHandler::kPowHandler(EnvPtr env, ProblemPtr problem, ProblemPtr orig_p)
    : aTol_(1e-6),
      bTol_(1e-8),
      env_(env),
      bte_(0),
      cute_(0),
      nlpe_(0),
      rTol_(1e-7) {
  orig_ = orig_p;
  p_ = problem;
  modProb_ = false;
  modRel_ = true;
  logger_ = env->getLogger();
  resetStats_();
  timer_ = env->getTimer();
  defaultLb_ = 1e12;
  defaultUb_ = -1e12;
  doQT_ = false;
  simplexCut_ = 0;
}

kPowHandler::~kPowHandler() {
  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    delete it->second;
  }
  xkFuns_.clear();

  // bStats_.qvars.clear();
  if (nlpe_) {
    delete nlpe_;
  }
  if (bte_) {
    delete bte_;
  }

  if (simplexCut_) {
    delete simplexCut_;
  }
}

void kPowHandler::addConstraint(ConstraintPtr newcon) {
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf;
  CGraphPtr cg;
  VariablePtr y, x;
  double k;
  LinkPowPtr lxk;

  cons_.push_back(newcon);
  nlf = newcon->getNonlinearFunction();
  lf = newcon->getLinearFunction();

  assert(lf && nlf);
  assert(1 == lf->getNumTerms());
  assert(1 == nlf->numVars());  

  y = lf->termsBegin()->first;  
  cg = dynamic_cast<CGraph *>(nlf);
  
  k = cg->getOut()->getR()->getVal();
  x = VariablePtr(cg->getOut()->getL()->getV());
  lxk = new LinkPow();
  lxk->k = k;
  lxk->x = x;
  lxk->y = y;
  lxk->oeCon = ConstraintPtr();
  xkFuns_.insert(std::pair<VariablePtr, LinkPowPtr>(x, lxk));
}

double kPowHandler::addDefaultBounds_(VariablePtr v, BoundType lu) {
  if (lu == Lower) {
    if (defaultLb_ < 1e12) {
      p_->changeBound(v, lu, defaultLb_);
    } else {
      for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd();
           ++it) {
        if ((*it)->getLb() < defaultLb_ && (*it)->getLb() > -1e12) {
          defaultLb_ = (*it)->getLb();
        }
      }
      if (defaultLb_ < -aTol_ && defaultLb_ > -1e12) {
        defaultLb_ = 100 * defaultLb_;
      } else if (defaultLb_ > aTol_ && defaultLb_ < 1e12) {
        defaultLb_ = -100 * defaultLb_;
      } else {
        defaultLb_ = -1000;
      }
      p_->changeBound(v, lu, defaultLb_);
    }
#if SPEW
    logger_->msgStream(LogDebug2)
        << me_ << "WARNING: Adding Default lower bound for " << v->getName()
        << "Lower bound value = " << defaultLb_ << std::endl;
#endif
    ++bStats_.dlb;
    return defaultLb_;
  } else {
    if (defaultUb_ > -1e12) {
      p_->changeBound(v, lu, defaultUb_);
    } else {
      for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd();
           ++it) {
        if ((*it)->getUb() > defaultUb_ && (*it)->getUb() < 1e12) {
          defaultUb_ = (*it)->getUb();
        }
      }
      if (defaultUb_ < -aTol_ && defaultUb_ > -1e12) {
        defaultUb_ = -100 * defaultUb_;
      } else if (defaultUb_ > aTol_ && defaultUb_ < 1e12) {
        defaultUb_ = 100 * defaultUb_;
      } else {
        defaultUb_ = 1000;
      }
      p_->changeBound(v, lu, defaultUb_);
    }
#if SPEW
    logger_->msgStream(LogDebug2)
        << me_ << "WARNING: Adding Default upper bound for " << v->getName()
        << "Upper bound value = " << defaultUb_ << std::endl;
#endif
    ++bStats_.dub;
    return defaultUb_;
  }
}

void kPowHandler::findLinPt_(double xval, double yval, double &xl, double &yl, double k) {
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
  const double errlim = 1e-4;  // don't want too much accuracy.

  if (xval > 0) {
    a = sqrt(yval);
    b = xval;
  } else {
    a = xval;
    b = -sqrt(yval);
  }

  mu = a + alfa * (b - a);
  la = b - alfa * (b - a);
  mu_val = (mu - xval) * (mu - xval) + (pow(mu,k) - yval) * (pow(mu,k) - yval);
  la_val = (la - xval) * (la - xval) + (pow(la,k) - yval) * (pow(la,k) - yval);
  while ((b - a) > errlim) {
    if (mu_val < la_val) {
      a = la;
      la = mu;
      la_val = mu_val;
      mu = a + alfa * (b - a);
      mu_val = (mu - xval) * (mu - xval) + (pow(mu,k) - yval) * (pow(mu,k) - yval);
    } else {
      b = mu;
      mu = la;
      mu_val = la_val;
      la = b - alfa * (b - a);
      la_val = (la - xval) * (la - xval) + (pow(la,k) - yval) * (pow(la,k) - yval);
    }
  }
  xl = la;
  yl = pow(la,k);
}

void kPowHandler::updateBoundsinOrig_(RelaxationPtr rel, const double *x,
                                      DoubleVector &varlb,
                                      DoubleVector &varub) {
  VariablePtr v, relv;
  double xval;

  for (VariableConstIterator vit = orig_->varsBegin(); vit != orig_->varsEnd();
       ++vit) {
    v = *vit;
    varlb.push_back(v->getLb());
    varub.push_back(v->getUb());
    if (v->getType() == Binary || v->getType() == Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5);
      orig_->changeBound(v, xval, xval);
    } else {
      relv = rel->getVariable(v->getIndex());
      orig_->changeBound(v, relv->getLb(), relv->getUb());
    }
  }
}

void kPowHandler::resetBoundsinOrig_(DoubleVector &varlb, DoubleVector &varub) {
  VariablePtr v;
  DoubleVector::iterator lbit = varlb.begin(), ubit = varub.begin();

  for (VariableConstIterator vit = orig_->varsBegin(); vit != orig_->varsEnd();
       ++vit) {
    v = *vit;
    orig_->changeBound(v, *lbit, *ubit);
    ++lbit;
    ++ubit;
  }
}

void kPowHandler::updateUb_(SolutionPoolPtr s_pool, double nlpval,
                            bool &sol_found) {
  double bestval = s_pool->getBestSolutionValue();
  double *new_x = new double[p_->getNumVars()];

  if ((bestval - aTol_ > nlpval) ||
      (bestval != 0 && (bestval - fabs(bestval) * rTol_ > nlpval))) {
    const double *x = nlpe_->getSolution()->getPrimal();
    memcpy(new_x, x, orig_->getNumVars() * sizeof(double));
    s_pool->addSolution(new_x, nlpval);
    sol_found = true;
  }
  delete[] new_x;
}

int kPowHandler::fixNodeErr(RelaxationPtr rel, ConstSolutionPtr sol,
                            SolutionPoolPtr s_pool, bool &sol_found) {
  DoubleVector varlb, varub;
  EngineStatus status;
  int error;

  varlb.clear();
  varub.clear();
  // update the bounds of vars in orig_
  updateBoundsinOrig_(rel, sol->getPrimal(), varlb, varub);
  // Solve the NLP with x as the starting point and get solution
  orig_->setInitialPoint(sol->getPrimal());
  status = nlpe_->solve();
  ++nlpStats_.nlp;
  nlpStats_.flag = true;
  // Check if there is a solution or if it reports infeasibility
  switch (status) {
    case (ProvenOptimal):
    case (ProvenLocalOptimal): {
      double nlpval = nlpe_->getSolutionValue();
      updateUb_(s_pool, nlpval, sol_found);
      error = 0;
      ++nlpStats_.opt;
      break;
    }
    case (ProvenInfeasible):
    case (ProvenLocalInfeasible):
    case (ProvenObjectiveCutOff): {
      sol_found = false;
      error = 1;
      ++nlpStats_.inf;
      break;
    }
    case (EngineIterationLimit): {
      sol_found = false;
      error = 1;
      ++nlpStats_.iter_limit;
      logger_->msgStream(LogDebug2)
          << me_ << "NLP iteration limit reached"
          << " this node will be considered infeasible" << std::endl;
      break;
    }
    case (EngineError): {
      sol_found = false;
      error = 2;
      ++nlpStats_.inf;
      break;
    }
    case (FailedFeas):
    case (FailedInfeas):
    case (ProvenUnbounded):
    case (ProvenFailedCQFeas):
    case (EngineUnknownStatus):
    case (ProvenFailedCQInfeas):
    default:
      logger_->msgStream(LogError)
          << me_ << "NLP engine status = " << nlpe_->getStatusString()
          << std::endl;
      error = -1;
      break;
  }
  resetBoundsinOrig_(varlb, varub);
  return error;
}

Branches kPowHandler::getBranches(BrCandPtr cand, DoubleVector &x,
                                  RelaxationPtr rel, SolutionPoolPtr) {
  // BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  VariablePtr v2 = 0;
  double value = x[v->getIndex()];
  BranchPtr branch;
  VarBoundModPtr mod;
  Branches branches = (Branches) new BranchPtrVector();

  // can't branch on something that is at its bounds.
  assert(value > v->getLb() + 1e-8 && value < v->getUb() - 1e-8);

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
  branch->setActivity(0.5);  // TODO: set this correctly
  branches->push_back(branch);

  // up branch
  branch = (BranchPtr) new Branch();
  if (modProb_) {
    mod = (VarBoundModPtr) new VarBoundMod(v2, Lower, value);
    branch->addPMod(mod);
  }
  if (modRel_) {
    mod = (VarBoundModPtr) new VarBoundMod(v, Lower, value);
    branch->addRMod(mod);
  }
  branch->setActivity(0.5);  // TODO: set this correctly
  branches->push_back(branch);
  vcand->setNumBranches(2);

#if SPEW
  logger_->msgStream(LogDebug2)
      << me_ << "branching on " << v->getName() << " <= " << value << " or "
      << " >= " << value << std::endl;
#endif
  return branches;
}

void kPowHandler::getBranchingCandidates(RelaxationPtr rel,
                                         const DoubleVector &x, ModVector &,
                                         BrVarCandSet &cands, BrCandVector &,
                                         bool &is_inf) {
  double yval, xval, udist, ddist, k;
  BrVarCandPtr br_can;
  VariablePtr x0,x1;
  std::pair<BrVarCandIter, bool> ret;
  bool check;

  is_inf = false;

  // First check if there is a candidate x0 that violates y <= x0^2.
  for (LinkPowMapIter s_it = xkFuns_.begin(); s_it != xkFuns_.end(); ++s_it) {
    k = s_it->second->k;
    x0 = rel->getRelaxationVar(s_it->first);
    xval = x[x0->getIndex()];
    x1 = rel->getRelaxationVar(s_it->second->y);
    yval = x[x1->getIndex()];
    if (yval - pow(xval,k) > fabs(yval) * rTol_ &&
        yval - pow(xval,k) > aTol_) {
#if SPEW
      logger_->msgStream(LogDebug2)
          << std::setprecision(9) << me_
          << "branching candidate for x^k: " << s_it->first->getName()
          << " value = " << xval << " aux var: " << s_it->second->y->getName()
          << " value = " << yval << std::endl;
#endif
      ddist = (yval - pow(xval,k)) /
              sqrt(1.0 + pow((x0->getLb() + xval),k));
      udist = (yval - pow(xval,k)) /
              sqrt(1.0 + pow((x0->getUb() + xval),k));
      br_can = (BrVarCandPtr) new BrVarCand(x0, x0->getIndex(), ddist, udist);
      ret = cands.insert(br_can);
      if (false == ret.second) {  // already exists.
        delete br_can;
        br_can = *(ret.first);
        br_can->setDist(ddist + br_can->getDDist(), udist + br_can->getDDist());
      }
    }
  }
}

ModificationPtr kPowHandler::getBrMod(BrCandPtr cand, DoubleVector &x,
                                      RelaxationPtr rel, BranchDirection dir) {
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  LinearFunctionPtr lf;
  LinConModPtr lmod;
  // BrVarCandPtr vcand = boost::dynamic_pointer_cast <BrVarCand> (cand);
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  VariablePtr x0, y;
  double xval, yval, vio, rhs, k;
  UInt vind = v->getIndex();
  bool found = false;

  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    if (vind == it->first->getIndex()) {
      xval = x[it->first->getIndex()];
      yval = x[it->second->y->getIndex()];
      k = it->second->k;
      vio = pow(xval,k) - yval;
      if (vio > aTol_ && vio > fabs(yval) * rTol_) {
        x0 = rel->getRelaxationVar(it->first);
        y = rel->getRelaxationVar(it->second->y);
        if (dir == DownBranch) {
          lf = getNewKpowLf_(y, x0, k, x0->getLb(), xval, rhs,2);
        } else {
          lf = getNewKpowLf_(y, x0, k, xval, x0->getUb(), rhs,2);
        }
        lmod =
            (LinConModPtr) new LinConMod(it->second->oeCon, lf, -INFINITY, rhs);
        lmods->insert(lmod);
      }
    }
  }
  return lmods;
}

std::string kPowHandler::getName() const {
  return "kPowHandler (Handling k power terms of the form y=x^k where k>1).";
}

LinearFunctionPtr kPowHandler::getNewKpowLf_(VariablePtr y,VariablePtr x,double k, 
                                double lb, double ub, double &rhs,
                                int type){
  LinearFunctionPtr lf = LinearFunctionPtr();
  double new_lb = lb;
  double new_ub = ub;
  double eps = 1e-5;
  // std::cout<<"OLD LB:"<<lb<<"\n";
  // std::cout<<"OLD UB:"<<ub<<"\n";
  if (lb < 0) {
    // new_lb = addDefaultBounds_(x, Lower);
    assert("negative value raised to fractional power is imaginary!");
  }
  if (ub > 1e12) {
    new_ub = addDefaultBounds_(x, Upper);
  }
  std::cout<<"NEW LB:"<<lb<<"\n";
  std::cout<<"NEW UB:"<<ub<<"\n";
  assert(ub < 1e21 && lb >= 0);

  switch (type) {
    case (0):
      // tangent at lower bound
      lf->addTerm(y, 1.);
      lf->addTerm(x, -k*pow(ub,k-1));
      rhs = (1-k)*pow(ub,k);
      break;
    case (1):
      // tangent at lower bound
      lf->addTerm(y, 1.);
      lf->addTerm(x, -k*pow(lb,k-1));
      rhs = (1-k)*pow(lb,k);
      break;
    case (2):
      //Relaxed Overestimator
      assert(ub < 1e21 && lb >= 0);
      rhs = -ub * lb * (pow(ub,k-1)-pow(lb,k-1))/(ub-lb);
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(y, 1.);
      lf->addTerm(x, -1. * (pow(ub,k)-pow(lb,k))/(ub-lb));      // y >= l0x1 + l1x0 - l1l0
      // std::cout<<"OE:  "<<rhs<<" = ";
      // lf->write(std::cout);
      // std::cout<<std::endl;
      break;
    default:
      assert(!"getNewKpowLf_ called with wrong value of i");
      break;
  }
  return lf;
}


void kPowHandler::addCut_(VariablePtr x, VariablePtr y, double xl, double yl,
                          double xval, double yval, RelaxationPtr rel,
                          bool &ifcuts, double k) {
  // add the cut k(xl^(k-1))x-(k-1)(yl)-y <= 0
  ifcuts = false;
  if (k * pow(xl,k-1) * xval - yval - (k-1) * yl > 1e-5 &&
      k * pow(xl,k-1) * xval - yval - k * yl > yl * (1e-4)) {
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    FunctionPtr f;

    lf->addTerm(x, k*pow(xl,k-1));
    lf->addTerm(y, -1.0);
    f = (FunctionPtr) new Function(lf);
    ifcuts = true;
    ++sStats_.cuts;
#if SPEW
    {
      ConstraintPtr c = rel->newConstraint(f, -INFINITY, (k-1)*yl);
      logger_->msgStream(LogDebug2) << me_ << "new cut added" << std::endl;
      c->write(logger_->msgStream(LogDebug2));
    }
#else
    rel->newConstraint(f, -INFINITY, (k-1)*yl);
#endif
  } else {
#if SPEW
    logger_->msgStream(LogDebug2)
        << me_ << "Not adding cut because of "
        << "insufficient violation " << 2 * xl * xval - yval - xl * xl
        << std::endl;
#endif
  }
}

// void kPowHandler::coeffImprov_() {
//   ConstraintPtr c;
//   double min_coeff = INFINITY;
//   LinearFunctionPtr lf;
//   NonlinearFunctionPtr nlf;
//   double eps = 1e-3;  // don't want coefficient smaller than this

//   for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
//        ++cit) {
//     c = *cit;
//     lf = c->getFunction()->getLinearFunction();
//     nlf = c->getFunction()->getNonlinearFunction();
//     if (lf) {
//       for (VariableGroupConstIterator lit = lf->termsBegin();
//            lit != lf->termsEnd(); ++lit) {
//         if (fabs(lit->second) < min_coeff) {
//           min_coeff = fabs(lit->second);
//         }
//       }
//     }

//     if (nlf) {
//       for (VariablePairGroupConstIterator qit = nlf->begin(); qit != nlf->end();
//            ++qit) {
//         if (fabs(qit->second) < min_coeff) {
//           min_coeff = fabs(qit->second);
//         }
//       }
//     }

//     if (min_coeff < eps) {
//       if (lf) {
//         lf->multiply(1 / min_coeff);
//       }

//       if (nlf) {
//         nlf->multiply(1 / min_coeff);
//       }

//       if (c->getLb() > -INFINITY) {
//         p_->changeBound(c, Lower, c->getLb() / min_coeff);
//       }
//       if (c->getUb() < INFINITY) {
//         p_->changeBound(c, Upper, c->getUb() / min_coeff);
//       }
//     }
//   }
// }

// void kPowHandler::calcRangeOfQuadVars_() {
//  double tot_range = 0, tot_sqr_range = 0, range;
//  for (VariableSet::iterator vit = bStats_.qvars.begin();
//         vit != bStats_.qvars.end(); ++vit) {
//    range = (*vit)->getUb() - (*vit)->getLb();
//    tot_range += range;
//    tot_sqr_range += range*range;
//  }
//
//  bStats_.avg_range = tot_range/bStats_.qvars.size();
//  bStats_.body_diag = sqrt(tot_sqr_range);
//
//  tot_sqr_range = 0;
//  for (VariableSet::iterator vit = bStats_.qvars.begin();
//         vit != bStats_.qvars.end(); ++vit) {
//    range = (*vit)->getUb() - (*vit)->getLb();
//    tot_sqr_range += (range - bStats_.avg_range)*(range - bStats_.avg_range);
//  }
//  bStats_.sd_range = sqrt(tot_sqr_range/bStats_.qvars.size());
//}

bool kPowHandler::isAtBnds_(ConstVariablePtr x, double xval) {
  double lb = x->getLb();
  double ub = x->getUb();
  return (fabs(xval - lb) < bTol_ || fabs(xval - ub) < bTol_);
}

bool kPowHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                             double &inf_meas) {
  ConstraintPtr c;
  double act, clb, cub;
  const double *x = sol->getPrimal();
  int error = 0;
  bool is_feas = true;
  ObjectivePtr obj;
  double vio;
  for (ConstraintConstIterator cit = orig_->consBegin();
       cit != orig_->consEnd(); ++cit) {
    c = *cit;
    if (c->getFunctionType() == Nonlinear) {
      act = c->getActivity(x, &error);
      if (error == 0) {
        cub = c->getUb();
        clb = c->getLb();
        if ((act > cub + aTol_) &&
            (cub == 0 || act > cub + fabs(cub) * rTol_)) {
          is_feas = false;
          inf_meas += act - cub;
        }
        if ((act < clb - aTol_) &&
            (clb == 0 || act < clb - fabs(clb) * rTol_)) {
          is_feas = false;
          inf_meas += clb - act;
        }
      } 
      else {
        logger_->msgStream(LogError)
            << me_ << c->getName() << " Constraint not defined at this point."
            << std::endl;
        is_feas = false;
      }
    }
  }

  obj = orig_->getObjective();
  if (obj->getFunctionType() == Quadratic ||
      obj->getFunctionType() == Bilinear) {
    act = obj->eval(x, &error);
    if (error == 0) {
      vio = fabs(sol->getObjValue() - act);
      if (vio > fabs(act) * rTol_ && vio > aTol_) {
        is_feas = false;
        inf_meas += vio;
      }
    }
  }
  return is_feas;
}

bool kPowHandler::isFeasibleToRelaxation_(RelaxationPtr rel, const double *x) {
  ConstraintPtr c;
  double act, clb, cub;
  int error = 0;

  for (ConstraintConstIterator cit = rel->consBegin(); cit != rel->consEnd();
       ++cit) {
    c = *cit;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cub = c->getUb();
      clb = c->getLb();
      if ((act > cub + aTol_) && (cub == 0 || act > cub + fabs(cub) * rTol_)) {
        return false;
      }
      if ((act < clb - aTol_) && (clb == 0 || act < clb - fabs(clb) * rTol_)) {
        return false;
      }
    } else {
      logger_->msgStream(LogError)
          << me_ << c->getName() << " Constraint not defined at this point."
          << std::endl;
    }
  }
  return true;
}

// bool kPowHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr , bool &,
//                             double &inf_meas)
//{
//  double yval, xval, vio;
//  const double *x = sol->getPrimal();
//  bool is_feas = true;
//
//  for (LinSqrMapIter it=x2Funs_.begin(); it != x2Funs_.end(); ++it) {
//    // check if y <= x^2
//    xval  = x[it->first->getIndex()];
//    yval = x[it->second->y->getIndex()];
//    vio = fabs(yval - xval*xval);
//    if (vio > fabs(yval)*rTol_ && vio > aTol_) {
//      is_feas = false;
//      inf_meas += vio;
//    }
//  }
//#if SPEW
//  logger_->msgStream(LogDebug2) << me_ << "no branching candidates for y=x^2"
//                                << std::endl;
//#endif
//
//  for (LinBilSetIter it=x0x1Funs_.begin(); it != x0x1Funs_.end(); ++it) {
//    if ((*it)->isViolated(x, vio)) {
//      is_feas = false;
//      inf_meas += vio;
//    }
//  }
//
//#if SPEW
//  logger_->msgStream(LogDebug2) << me_ << "no branching candidates for y=x1x2"
//                                << std::endl;
//#endif
//  return is_feas;
//}
//

SolveStatus kPowHandler::presolve(PreModQ *, bool *changed, Solution **) {
  bool is_inf = false;
  SolveStatus status = Started;
  // QuadraticFunctionPtr nlf;
  // ObjectivePtr obj;

  *changed = false;

  if (cons_.size() > 0) {
    is_inf = varBndsFromCons_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }

  } else {
    //++bStats_.niters;
    // coeffImprov_();
    // for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
    //                             ++cit) {
    //  if ((*cit)->getFunctionType() == Quadratic ||
    //      (*cit)->getFunctionType() == Bilinear) {
    //    nlf = (*cit)->getFunction()->getQuadraticFunction();
    //    if (nlf) {
    //      for (VarIntMapConstIterator qit = nlf->varsBegin();
    //           qit != nlf->varsEnd(); ++qit) {
    //        bStats_.qvars.insert(qit->first);
    //      }
    //    }
    //  }
    //}
    // obj = p_->getObjective();
    // if (obj->getFunctionType() == Quadratic ||
    //    obj->getFunctionType() == Bilinear) {
    //  nlf = obj->getFunction()->getQuadraticFunction();
    //  if (nlf) {
    //    for (VarIntMapConstIterator qit = nlf->varsBegin();
    //         qit != nlf->varsEnd(); ++qit) {
    //      bStats_.qvars.insert(qit->first);
    //    }
    //  }
    //}
    // for (VariableSet::iterator vit = bStats_.qvars.begin();
    //     vit != bStats_.qvars.end(); ++vit) {
    //  if ((*vit)->getLb() > -INFINITY) {
    //    ++bStats_.nqlb;
    //  }
    //  if ((*vit)->getUb() < INFINITY) {
    //    ++bStats_.nqub;
    //  }
    //}
    is_inf = tightenSimple_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
    // for (VariableSet::iterator vit = bStats_.qvars.begin();
    //     vit != bStats_.qvars.end(); ++vit) {
    //  if ((*vit)->getLb() > -INFINITY) {
    //    ++bStats_.nqlbs;
    //  }
    //  if ((*vit)->getUb() < INFINITY) {
    //    ++bStats_.nqubs;
    //  }
    //}
    // bStats_.nqlbs -= bStats_.nqlb;
    // bStats_.nqubs -= bStats_.nqub;
    // p_->delMarkedCons();

    is_inf = tightenkPow_(changed);
    if (is_inf) {
      status = SolvedInfeasible;
      return status;
    }
    // for (VariableSet::iterator vit = bStats_.qvars.begin();
    //     vit != bStats_.qvars.end(); ++vit) {
    //  if ((*vit)->getLb() > -INFINITY) {
    //    ++bStats_.nqlbq;
    //  }
    //  if ((*vit)->getUb() < INFINITY) {
    //    ++bStats_.nqubq;
    //  }
    //}
    // bStats_.nqlbq -= bStats_.nqlb + bStats_.nqlbs;
    // bStats_.nqubq -= bStats_.nqub + bStats_.nqubs;
    p_->delMarkedCons();
  }

  if (Started == status) {
    status = Finished;
  }
  return status;
}

bool kPowHandler::presolveNode(RelaxationPtr rel, NodePtr,
                               SolutionPoolPtr s_pool, ModVector &p_mods,
                               ModVector &r_mods) {
  return false;
  bool lchanged = true;
  bool changed = true;
  bool is_inf = false;
  double stime = timer_->query();
  double ub = s_pool->getBestSolutionValue();
  std::cout<<"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n";

  // visit each quadratic constraint and see if bounds can be improved.
  ++bStats_.niters;
  while (true == changed) {
    ++pStats_.iters;
    changed = false;
    for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
      is_inf = propKPowBnds_(it, rel, modRel_, &lchanged, p_mods, r_mods);
      if (true == lchanged) {
        changed = true;
        lchanged = false;
      }
      if (is_inf) {
        return true;
      }
    }
  }
  std::cout<<"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n";

  if (doQT_ || bStats_.niters <= 1) {
    lchanged = false;
    is_inf = tightenkPow_(rel, ub, &lchanged, p_mods, r_mods);
    if (true == lchanged) {
      changed = true;
      lchanged = false;
    }
    if (is_inf) {
      return true;
    }
  }
  std::cout<<"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n";

  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    upSqCon_(it->second->oeCon, rel->getRelaxationVar(it->first),
             rel->getRelaxationVar(it->second->y), 
             it->second->k, rel, r_mods);
  }

  if (modRel_) {
    pStats_.nMods += r_mods.size();
  } else {
    pStats_.nMods += p_mods.size();
  }
  // rel->write(std::cout);

  std::cout<<"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n";
  pStats_.timeN += timer_->query() - stime;
  return false;
}


void BoundsOnKPow_(ConstVariablePtr x, double &lb, double &ub, double k){
  double l1 = x->getLb();
  double u1 = x->getUb();

  if ((u1 < 0.)||(l1 > 0.)) {         // both bounds are negative.
    assert("!!!negative bound(s). Infeasible for pow k");
  } else if (l1 > 0.) {  // both bounds are positive.
    lb = pow(l1,k);
    ub = pow(u1,k);
  }
}


bool kPowHandler::propKPowBnds_(LinkPowMapIter lxk, bool *changed) {
  VariablePtr x = lxk->first;  // x0 and y are variables in p_
  VariablePtr y = lxk->second->y;
  double k = lxk->second->k;
  double lb, ub;

  BoundsOnKPow_(x, lb, ub, k);
  if (updatePBounds_(y, lb, ub, changed) < 0) {
    return true;
  }

  // other direction.
  if (y->getUb() > bTol_) {
    ub = pow(y->getUb(),1/k);
    lb = -ub;
    assert(y->getLb() >= 0.0);  // square of a number.
    if (x->getLb() > -pow(y->getLb(),1/k) + bTol_) {
      lb = pow(y->getLb(),1/k);
    }
    if (updatePBounds_(x, lb, ub, changed) < 0) {
      return true;
    }
  } else if (y->getUb() < -bTol_) {
    return true;
  } 
  // else {
  //   if (updatePBounds_(x, 0.0, 0.0, changed) < 0) {
  //     std::cout<<"Case4\n";
  //     return true;
  //   }
  // }

  return false;
}

bool kPowHandler::propKPowBnds_(LinkPowMapIter lxk, RelaxationPtr rel,
                               bool mod_rel, bool *changed, ModVector &p_mods,
                               ModVector &r_mods) {
  double lb, ub;
  VariablePtr x = lxk->first;  // x and y are variables in p_
  VariablePtr y = lxk->second->y;
  double k = lxk->second->k;

  BoundsOnKPow_(x, lb, ub,k);
  if (updatePBounds_(y, lb, ub, rel, mod_rel, changed, p_mods, r_mods) < 0) {
    return true;  // infeasible
  }

  // other direction.
  if (y->getUb() > bTol_) {
    ub = pow(y->getUb(),1/k);
    lb = -ub;
    assert(y->getLb() >= 0.0);
    if (x->getLb() > -pow(y->getLb() + bTol_,1/k)) {
      lb = pow(y->getLb(),1/k);
    }
    if (updatePBounds_(x, lb, ub, rel, mod_rel, changed, p_mods, r_mods) < 0) {
      return true;  // infeasible
    }
  } else if (y->getUb() < -bTol_) {
    return true;  // infeasible
  } 
  // else {
  //   if (updatePBounds_(x, 0.0, 0.0, rel, mod_rel, changed, p_mods, r_mods) <
  //       0) {
  //     std::cout<<"YABABABABA0000000000000\n";
  //     return true;  // infeasible
  //   }
  // }

  return false;
}

bool kPowHandler::postSolveRootNode(RelaxationPtr rel, SolutionPoolPtr s_pool,
                                    ConstSolutionPtr sol, ModVector &p_mods,
                                    ModVector &r_mods) {
  if (!(env_->getOptions()->findBool("doPostSolve")->getValue())) {
    return true;
  }
  double vio1, vio2, range, yval, xval, ub, k;
  VariablePtr y, x0;
  double stime = timer_->query();
  bool is_inf, lchanged = false;
  const double *x = sol->getPrimal();
  const double max_vio = 1e-3;

  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit) {
    (*vit)->setItmp(0);
  }
  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    y = it->second->y;
    k = it->second->k;
    x0 = it->first;

    yval = x[y->getIndex()];
    xval = x[x0->getIndex()];
    vio1 = fabs(pow(xval,k) - yval);
    if (vio1 > max_vio && vio1 > 0.1 * fabs(yval)) {
      range = x0->getUb() - x0->getLb();
      if (range >= 2) {
        x0->setItmp(3);
      }
      range = y->getUb() - y->getLb();
      if (range >= 2) {
        vio1 = yval - y->getLb();
        if (vio1 > max_vio && vio1 > 0.1 * y->getLb()) {
          y->setItmp(3);
        } else {
          y->setItmp(2);
        }
      }
    }
  }

  ub = s_pool->getBestSolutionValue();
  is_inf = tightenLP_(rel, ub, &lchanged, p_mods, r_mods);
  if (is_inf) {
    logger_->msgStream(LogError)
        << me_ << "WARNING: OBBT returns infeasibility after "
        << "root node is solved" << std::endl;
  }
  bStats_.timeLP = timer_->query() - stime;

  if (true == lchanged) {
    for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
      upSqCon_(it->second->oeCon, rel->getRelaxationVar(it->first),
               rel->getRelaxationVar(it->second->y), 
               it->second->k,rel, r_mods);
    }

    if (modRel_) {
      pStats_.nMods += r_mods.size();
    } else {
      pStats_.nMods += p_mods.size();
    }
    if (isFeasibleToRelaxation_(rel, x)) {
      return true;
    } else {
      return false;
    }
  }
  return true;
}

void kPowHandler::relax_(RelaxationPtr rel, bool *) {
  double rhs;
  LinearFunctionPtr lf;
  VariablePtr y, x;
  FunctionPtr f;
  // ConstraintVector cons(3);

  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    x = rel->getRelaxationVar(it->second->x);
    y = rel->getRelaxationVar(it->second->y);
    lf = getNewKpowLf_(y, x, it->second->k, x->getLb(), x->getUb(),
                         rhs, 2);
    f = (FunctionPtr) new Function(lf);
    it->second->oeCon = rel->newConstraint(f, -INFINITY, rhs);
  }

  assert(0 == rel->checkConVars());

  if (bStats_.dlb > 0) {
    logger_->msgStream(LogError)
        << me_ << "WARNING: Default lower bound was assumed for " << bStats_.dlb
        << " variables" << std::endl;
  }
  if (bStats_.dub > 0) {
    logger_->msgStream(LogError)
        << me_ << "WARNING: Default upper bound was assumed for " << bStats_.dub
        << " variables" << std::endl;
  }
  return;
}

void kPowHandler::relaxInitFull(RelaxationPtr rel, bool *is_inf) {
  relax_(rel, is_inf);
}

void kPowHandler::relaxInitInc(RelaxationPtr rel, bool *is_inf) {
  relax_(rel, is_inf);
}

void kPowHandler::relaxNodeFull(NodePtr, RelaxationPtr, bool *) {
  assert(!"kPowHandler::relaxNodeFull not implemented!");
}

void kPowHandler::relaxNodeInc(NodePtr, RelaxationPtr, bool *) {
  // do nothing. Presolve will take care of tightening bounds
}

void kPowHandler::resetStats_() {
  pStats_.iters = 0;
  pStats_.time = 0.0;
  pStats_.timeN = 0.0;
  pStats_.vBnd = 0;
  pStats_.nMods = 0;

  sStats_.iters = 0;
  sStats_.cuts = 0;
  sStats_.time = 0.0;

  nlpStats_.flag = false;
  nlpStats_.nlp = 0;
  nlpStats_.opt = 0;
  nlpStats_.inf = 0;
  nlpStats_.iter_limit = 0;

  bStats_.niters = 0;
  // bStats_.qvars.clear();
  // bStats_.nqlb = 0;
  // bStats_.nqub = 0;
  // bStats_.nqlbs = 0;
  // bStats_.nqubs = 0;
  // bStats_.vBnds = 0;
  // bStats_.cBnds = 0;
  // bStats_.nqlbq = 0;
  // bStats_.nqubq = 0;
  // bStats_.vBndq = 0;
  // bStats_.cBndq = 0;
  // bStats_.nqlbl = 0;
  // bStats_.nqubl = 0;
  // bStats_.vBndl = 0;
  bStats_.nLP = 0;
  bStats_.dlb = 0;
  bStats_.dub = 0;
  // bStats_.time = 0;
  bStats_.timeLP = 0;
  // bStats_.avg_range = 0;
  // bStats_.sd_range = 0;
  // bStats_.body_diag = 0;
}

void kPowHandler::separate(ConstSolutionPtr sol, NodePtr, RelaxationPtr rel,
                           CutManager *, SolutionPoolPtr, ModVector &,
                           ModVector &, bool *, SeparationStatus *status) {
  std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";
  double yval, xval,k;
  double yl, xl;
  const double *x = sol->getPrimal();
  bool ifcuts;
  int ncuts;

  if ((!simplexCut_) &&
      (env_->getOptions()->findInt("simplex_cut_rounds")->getValue() > 0)) {
    simplexCut_ = (SimplexQuadCutGenPtr) new SimplexQuadCutGen(env_, p_, cute_);
  }

  if (simplexCut_) {
    ncuts = simplexCut_->generateCuts(rel, x);
    if (ncuts > 0) {
      *status = SepaResolve;
      return;
    }
  }

  ++sStats_.iters;
  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    xval = x[it->first->getIndex()];
    yval = x[it->second->y->getIndex()];
    k = it->second->k;
    if (pow(xval,k) - yval > rTol_ * fabs(yval) &&
        fabs(pow(xval,k) - yval) > aTol_) {
#if SPEW
      logger_->msgStream(LogDebug2)
          << me_ << "xval = " << xval << " yval = " << yval
          << " violation = " << xval * xval - yval << std::endl;
#endif
      findLinPt_(xval, yval, xl, yl, k);
      addCut_(rel->getRelaxationVar(it->first),
              rel->getRelaxationVar(it->second->y), xl, yl, xval, yval, rel,
              ifcuts, k);
      if (true == ifcuts) {
        *status = SepaResolve;
      }
    }
  }
}

double kPowHandler::calcUpperUnivar_(double a, double b, double lx, double ux) {
  double u, s, t, r;

  u = std::max(lx * (a * lx + b), ux * (a * ux + b));
  s = b / 2.0;
  t = s / (-a);
  if (t > lx) {
    r = (-2.0 * a) * ux;
    if (r > b) {
      u = std::max(u, s * t);
    }
  }
  return u;
}

void kPowHandler::getTermBnds_(VariablePtr v, double coef, double &lb,
                               double &ub) {
  if (coef > 0) {
    lb = v->getLb() > -INFINITY ? coef * v->getLb() : -INFINITY;
    ub = v->getUb() < INFINITY ? coef * v->getUb() : INFINITY;
  } else {
    lb = v->getUb() < INFINITY ? coef * v->getUb() : -INFINITY;
    ub = v->getLb() > -INFINITY ? coef * v->getLb() : INFINITY;
  }
}

void kPowHandler::getTermBnds_(VariablePtr v1, VariablePtr v2, double coef,
                               double &lb, double &ub) {
  double qlb, qub;
  if (v1->getIndex() == v2->getIndex()) {
    BoundsOnSquare(v1, qlb, qub);
  } else {
    BoundsOnProduct(true, v1, v2, qlb, qub);
  }
  if (coef > 0) {
    lb = qlb > -INFINITY ? coef * qlb : -INFINITY;
    ub = qub < INFINITY ? coef * qub : INFINITY;
  } else {
    lb = qub < INFINITY ? coef * qub : -INFINITY;
    ub = qlb > -INFINITY ? coef * qlb : INFINITY;
  }
}

void kPowHandler::getTermBnds_(VariablePtr v, double a, double b, double &lb,
                               double &ub) {
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

bool kPowHandler::calcVarBnd_(VariablePtr v, double coef, double lb, double ub,
                              bool *c1) {
  double vlb, vub;
  vlb = coef > 0 ? lb / coef : ub / coef;
  vub = coef > 0 ? ub / coef : lb / coef;

  if (updatePBounds_(v, vlb, vub, c1) < 0) {
    return true;
  }
  return false;
}

bool kPowHandler::calcVarBnd_(RelaxationPtr rel, VariablePtr v, double coef,
                              double lb, double ub, bool *c1, ModVector &p_mods,
                              ModVector &r_mods) {
  double vlb, vub;
  vlb = coef > 0 ? lb / coef : ub / coef;
  vub = coef > 0 ? ub / coef : lb / coef;

  if (updatePBounds_(v, vlb, vub, rel, modRel_, c1, p_mods, r_mods) < 0) {
    return true;
  }
  return false;
}

bool kPowHandler::calcVarBnd_(VariablePtr v1, VariablePtr v2, double coef,
                              double lb, double ub, bool *c1, bool *c2) {
  double qlb, qub, vlb, vub;
  qlb = coef > 0 ? lb / coef : ub / coef;
  qub = coef > 0 ? ub / coef : lb / coef;
  // if term is quadratic
  if (v1->getIndex() == v2->getIndex()) {
    if (qub > bTol_) {
      vub = sqrt(qub);
      vlb = -ub;
      qlb = qlb >= 0 ? qlb : 0;  // square of a number.
      if (v1->getLb() > -sqrt(qlb) + bTol_) {
        vlb = sqrt(qlb);
      }
      if (updatePBounds_(v1, vlb, vub, c1) < 0) {
        return true;
      }
    } else if (qub < -bTol_) {
      return true;
    } else {
      if (updatePBounds_(v1, 0.0, 0.0, c1) < 0) {
        return true;
      }
    }
    return false;
    // if term is bilinear
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

bool kPowHandler::calcVarBnd_(RelaxationPtr rel, VariablePtr v1, VariablePtr v2,
                              double coef, double lb, double ub, bool *c1,
                              bool *c2, ModVector &p_mods, ModVector &r_mods) {
  double qlb, qub, vlb, vub;
  qlb = coef > 0 ? lb / coef : ub / coef;
  qub = coef > 0 ? ub / coef : lb / coef;
  // if term is quadratic
  if (v1->getIndex() == v2->getIndex()) {
    if (qub > bTol_) {
      vub = sqrt(qub);
      vlb = -ub;
      qlb = qlb >= 0 ? qlb : 0;  // square of a number.
      if (v1->getLb() > -sqrt(qlb) + bTol_) {
        vlb = sqrt(qlb);
      }
      if (updatePBounds_(v1, vlb, vub, rel, modRel_, c1, p_mods, r_mods) < 0) {
        return true;
      }
    } else if (qub < -bTol_) {
      return true;
    } else {
      if (updatePBounds_(v1, 0.0, 0.0, rel, modRel_, c1, p_mods, r_mods) < 0) {
        return true;
      }
    }
    return false;
    // if term is bilinear
  } else {
    BoundsOnDiv(qlb, qub, v1->getLb(), v1->getUb(), vlb, vub);
    if (updatePBounds_(v2, vlb, vub, rel, modRel_, c2, p_mods, r_mods) < 0) {
      return true;
    }

    BoundsOnDiv(qlb, qub, v2->getLb(), v2->getUb(), vlb, vub);
    if (updatePBounds_(v1, vlb, vub, rel, modRel_, c1, p_mods, r_mods) < 0) {
      return true;
    }
    return false;
  }
}

bool kPowHandler::calcVarBnd_(VariablePtr v, double a, double b, double ly,
                              double uy, bool *c1) {
  double lx = v->getLb();
  double ux = v->getUb();
  double lb = -INFINITY, ub = INFINITY, delta, lb2, ub2;

  if (fabs(a) <= aTol_) {
    assert(fabs(b) > aTol_);
    lb = b > 0 ? ly / b : uy / b;
    ub = b > 0 ? uy / b : ly / b;
  } else if (a > aTol_) {  /// the term is convex
    if (uy < INFINITY) {   /// finite upper bound
      delta = b * b + 4.0 * a * uy;
      if (delta < -aTol_) {  /// upper bound is less than minima
        return true;
      } else if (fabs(delta) <= aTol_) {  /// upper bound is at minima
        lb = -b / (2.0 * a);
        ub = lb;
      } else {  /// upper bound is greater than minima
        lb = (-b - sqrt(delta)) / (2.0 * a);
        ub = (-b + sqrt(delta)) / (2.0 * a);
        delta = b * b + 4.0 * a * ly;
        if (delta > aTol_) {  /// lower bound is greater than minima
          /// The bound on the variable now are
          /// \mathbb{R} \setminus (lb2, ub2) intersection [lb, ub]
          lb2 = (-b - sqrt(delta)) / (2.0 * a);
          ub2 = (-b + sqrt(delta)) / (2.0 * a);
          if (lx > lb2 + bTol_) {
            lb = ub2;
          }
          if (ux < ub2 - bTol_) {
            ub = lb2;
          }
        }
      }
    } else {  /// infinite upper bound
      delta = b * b + 4.0 * a * ly;
      if (delta > aTol_) {
        lb2 = (-b - sqrt(delta)) / (2.0 * a);
        ub2 = (-b + sqrt(delta)) / (2.0 * a);
        if (lx > lb2 + bTol_ && lx < ub2 - bTol_) {
          lb = ub2;
        }
        if (ux > lb2 + bTol_ && ux < ub2 - bTol_) {
          ub = lb2;
        }
      }
    }
  } else {                 /// concave term
    if (ly > -INFINITY) {  /// finite lower bound
      delta = b * b + 4.0 * a * ly;
      if (delta < -aTol_) {  /// lower bound is greater than maxima
        return true;
      } else if (fabs(delta) <= aTol_) {  /// lower bound is at maxima
        lb = -b / (2.0 * a);
        ub = lb;
      } else {  /// lower bound is less than maxima
        lb = (-b + sqrt(delta)) /
             (2.0 * a);  // since a is -ve this term will be lb
        ub = (-b - sqrt(delta)) /
             (2.0 * a);  // since a is -ve this term will be ub
        delta = b * b + 4.0 * a * uy;
        if (delta > aTol_) {  /// upper bound is less than maxima
          /// The bound on the variable now are
          /// \mathbb{R} \setminus (lb2, ub2) intersection [lb, ub]
          lb2 = (-b + sqrt(delta)) /
                (2.0 * a);  // since a is -ve this term will be lb
          ub2 = (-b - sqrt(delta)) /
                (2.0 * a);  // since a is -ve this term will be ub
          if (lx > lb2 + bTol_) {
            lb = ub2;
          }
          if (ux < ub2 - bTol_) {
            ub = lb2;
          }
        }
      }
    } else {  /// infinite lower bound
      delta = b * b + 4.0 * a * uy;
      if (delta > aTol_) {
        lb2 = (-b + sqrt(delta)) / (2.0 * a);
        ub2 = (-b - sqrt(delta)) / (2.0 * a);
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

bool kPowHandler::calcVarBnd_(RelaxationPtr rel, VariablePtr v, double a,
                              double b, double ly, double uy, bool *c1,
                              ModVector &p_mods, ModVector &r_mods) {
  double lx = v->getLb();
  double ux = v->getUb();
  double lb = -INFINITY, ub = INFINITY, delta, lb2, ub2;

  if (fabs(a) <= aTol_) {
    assert(fabs(b) > aTol_);
    lb = ly / b;
    ub = uy / b;
  } else if (a > aTol_) {  /// the term is convex
    if (uy < INFINITY) {   /// finite upper bound
      delta = b * b + 4.0 * a * uy;
      if (delta < -aTol_) {  /// upper bound is less than minima
        return true;
      } else if (fabs(delta) <= aTol_) {  /// upper bound is at minima
        lb = -b / (2.0 * a);
        ub = lb;
      } else {  /// upper bound is greater than minima
        lb = (-b - sqrt(delta)) / (2.0 * a);
        ub = (-b + sqrt(delta)) / (2.0 * a);
        delta = b * b + 4.0 * a * ly;
        if (delta > aTol_) {  /// lower bound is greater than minima
          /// The bound on the variable now are
          /// \mathbb{R} \setminus (lb2, ub2) intersection [lb, ub]
          lb2 = (-b - sqrt(delta)) / (2.0 * a);
          ub2 = (-b + sqrt(delta)) / (2.0 * a);
          if (lx > lb2 + bTol_) {
            lb = ub2;
          }
          if (ux < ub2 - bTol_) {
            ub = lb2;
          }
        }
      }
    } else {  /// infinite upper bound
      delta = b * b + 4.0 * a * ly;
      if (delta > aTol_) {
        lb2 = (-b - sqrt(delta)) / (2.0 * a);
        ub2 = (-b + sqrt(delta)) / (2.0 * a);
        if (lx > lb2 + bTol_ && lx < ub2 - bTol_) {
          lb = ub2;
        }
        if (ux > lb2 + bTol_ && ux < ub2 - bTol_) {
          ub = lb2;
        }
      }
    }
  } else {                 /// concave term
    if (ly > -INFINITY) {  /// finite lower bound
      delta = b * b + 4.0 * a * ly;
      if (delta < -aTol_) {  /// lower bound is greater than maxima
        return true;
      } else if (fabs(delta) <= aTol_) {  /// lower bound is at maxima
        lb = -b / (2.0 * a);
        ub = lb;
      } else {  /// lower bound is less than maxima
        lb = (-b + sqrt(delta)) /
             (2.0 * a);  // since a is -ve this term will be lb
        ub = (-b - sqrt(delta)) /
             (2.0 * a);  // since a is -ve this term will be ub
        delta = b * b + 4.0 * a * uy;
        if (delta > aTol_) {  /// upper bound is less than maxima
          /// The bound on the variable now are
          /// \mathbb{R} \setminus (lb2, ub2) intersection [lb, ub]
          lb2 = (-b + sqrt(delta)) /
                (2.0 * a);  // since a is -ve this term will be lb
          ub2 = (-b - sqrt(delta)) /
                (2.0 * a);  // since a is -ve this term will be ub
          if (lx > lb2 + bTol_) {
            lb = ub2;
          }
          if (ux < ub2 - bTol_) {
            ub = lb2;
          }
        }
      }
    } else {  /// infinite lower bound
      delta = b * b + 4.0 * a * uy;
      if (delta > aTol_) {
        lb2 = (-b - sqrt(delta)) / (2.0 * a);
        ub2 = (-b + sqrt(delta)) / (2.0 * a);
        if (lx > lb2 + bTol_ && lx < ub2 - bTol_) {
          lb = ub2;
        }
        if (ux > lb2 + bTol_ && ux < ub2 - bTol_) {
          ub = lb2;
        }
      }
    }
  }
  if (updatePBounds_(v, lb, ub, rel, modRel_, c1, p_mods, r_mods) < 0) {
    return true;
  }
  return false;
}

double kPowHandler::getBndByLP_(bool &is_inf) {
  double b;
  EngineStatus lpStatus;
  lpStatus = bte_->solve();
  ++bStats_.nLP;
  is_inf = false;

  switch (lpStatus) {
    case (ProvenOptimal):
    case (EngineIterationLimit):
    case (ProvenUnbounded):
      b = bte_->getSolution()->getObjValue();
      break;
    case (ProvenInfeasible):
    case (ProvenObjectiveCutOff):
      b = INFINITY;
      is_inf = true;
      break;
    case (EngineError):
    case (EngineUnknownStatus):
    default:
      logger_->msgStream(LogError)
          << me_ << "LP engine status at root= " << lpStatus << std::endl;
      assert(!"In kPowHandler: stopped at root. Check error log.");
      b = INFINITY;
      break;
  }
  return b;
}

double kPowHandler::getSumExcept1_(DoubleVector::iterator b,
                                   DoubleVector::iterator e,
                                   DoubleVector::iterator curr, BoundType bt,
                                   double bound, UInt inf_count) {
  double sum_of_elems = 0.0;
  if (inf_count == 0) {
    return bound - *curr;
  } else if (inf_count == 1) {
    if (bt == Lower) {
      if (*curr <= -INFINITY) {
        for (DoubleVector::iterator it = b; it != e; ++it) {
          if (it != curr) {
            sum_of_elems += *it;
          }
        }
        return sum_of_elems;
      } else {
        return -INFINITY;
      }
    } else {
      if (*curr >= INFINITY) {
        for (DoubleVector::iterator it = b; it != e; ++it) {
          if (it != curr) {
            sum_of_elems += *it;
          }
        }
        return sum_of_elems;
      } else {
        return INFINITY;
      }
    }
  } else {
    return bt == Lower ? -INFINITY : INFINITY;
  }
}

void kPowHandler::setBTEngine(LPEnginePtr engine) {
  if (bte_ && bte_ != engine) {
    bte_->clear();
  }
  bte_ = engine;
}

void kPowHandler::setCutEngine(LPEnginePtr engine) {
  if (cute_ && cute_ != engine) {
    cute_->clear();
  }
  cute_ = engine;
}

void kPowHandler::setNLPEngine(EnginePtr engine) {
  if (nlpe_ && nlpe_ != engine) {
    nlpe_->clear();
  }
  nlpe_ = engine;
  nlpe_->load(orig_);
}

void kPowHandler::setItmpFromSol_(const double *x) {
  /**
   * itmp = 0 means the variable need not be tightened at all
   * itmp = 1 means only lower bound needs to be tightened
   * itmp = 2 means only upper bound needs to be tightened
   * itmp = 3 means both bounds need to be tightened
   */
  double xval, gap, allowed_gap = 0.01;
  VariablePtr v;
  UInt itmp;
  double lb, ub;

  for (VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
       ++vit) {
    v = *vit;
    lb = v->getLb();
    ub = v->getUb();
    xval = x[v->getIndex()];
    itmp = v->getItmp();
    if (itmp == 0) {
      continue;
    } else if (itmp == 1) {
      gap = (xval - lb) / (ub - lb);
      if (gap <= allowed_gap) {
        v->setItmp(0);
      }
    } else if (itmp == 2) {
      gap = (ub - xval) / (ub - lb);
      if (gap <= allowed_gap) {
        v->setItmp(0);
      }
    } else {  // itmp = 3
      gap = (xval - lb) / (ub - lb);
      if (gap <= allowed_gap) {
        v->setItmp(2);
      }
      gap = (ub - xval) / (ub - lb);
      if (gap <= allowed_gap) {
        v->setItmp(1);
      }
    }
  }
}

bool kPowHandler::tightenLP_(RelaxationPtr rel, double bestSol, bool *changed,
                             ModVector &p_mods, ModVector &r_mods) {
  ObjectivePtr obj;
  LinearFunctionPtr lflp = 0;
  FunctionPtr flp = 0, f;
  ProblemPtr lp;
  VariablePtr v;
  double lb, ub, cub;
  bool is_inf;
  bool c1;
  int err;
  UInt itmp;

  lp = rel->clone(env_);

  obj = lp->getObjective();
  cub = env_->getOptions()->findDouble("obj_cut_off")->getValue();
  cub = std::min(cub, bestSol);
  if (cub < INFINITY) {
    f = obj->getFunction();
    if (f) {
      flp = f->cloneWithVars(lp->varsBegin(), &err);
      assert(err == 0);
      lp->newConstraint(flp, -INFINITY, cub - obj->getConstant());
    }
  }

  bte_->load(lp);

  for (VariableConstIterator vit = lp->varsBegin(); vit != lp->varsEnd();
       ++vit) {
    v = *vit;
    lb = -INFINITY;
    ub = INFINITY;
    itmp = p_->getVariable(v->getIndex())->getItmp();
    if (itmp == 0) {
      continue;
    }
    if (itmp == 1 || itmp == 3) {
      lflp = (LinearFunctionPtr) new LinearFunction();
      lflp->addTerm(v, 1.0);
      flp = (FunctionPtr) new Function(lflp);
      lp->changeObj(flp, 0.0);
      lb = getBndByLP_(is_inf);
      if (is_inf) {
        continue;
      }
      v->setItmp(itmp - 1);
      setItmpFromSol_(bte_->getSolution()->getPrimal());
    }

    if (itmp == 2) {
      lflp = (LinearFunctionPtr) new LinearFunction();
      lflp->addTerm(*vit, -1.0);
      flp = (FunctionPtr) new Function(lflp);
      lp->changeObj(flp, 0.0);
      ub = -getBndByLP_(is_inf);
      if (is_inf) {
        continue;
      }
      v->setItmp(0);
      setItmpFromSol_(bte_->getSolution()->getPrimal());
    }
    c1 = false;
    if (updatePBounds_(p_->getVariable(v->getIndex()), lb, ub, rel, true, &c1,
                       p_mods, r_mods) < 0) {
      // delete bte_;
      delete lp;
      return true;
    }
    if (c1 == true) {
      //++bStats_.vBndl;
      *changed = true;
    }
  }
  // delete bte_;
  delete lp;
  return false;
}

bool kPowHandler::getQfLfBnds_(LinearFunctionPtr lf, QuadraticFunctionPtr nlf,
                               double &implLb, double &implUb,
                               DoubleVector &fwdLb, DoubleVector &fwdUb,
                               UInt &count_inf_lb, UInt &count_inf_ub,
                               VarVector &qvars) {
  double lb = 0, ub = 0;
  for (VariablePairGroupConstIterator qit = nlf->begin(); qit != nlf->end();
       ++qit) {
    if (qit->first.first->getIndex() == qit->first.second->getIndex() &&
        lf->hasVar(qit->first.first)) {
      qvars.push_back(qit->first.first);
      getTermBnds_(qit->first.first, qit->second,
                   lf->getWeight(qit->first.first), lb, ub);
      if (lb <= -INFINITY) {
        ++count_inf_lb;
      }
      if (ub >= INFINITY) {
        ++count_inf_ub;
      }
      implLb += lb;
      implUb += ub;
      fwdLb.push_back(lb);
      fwdUb.push_back(ub);
    } else {
      getTermBnds_(qit->first.first, qit->first.second, qit->second, lb, ub);
      if (lb <= -INFINITY) {
        ++count_inf_lb;
      }
      if (ub >= INFINITY) {
        ++count_inf_ub;
      }
      implLb += lb;
      implUb += ub;
      fwdLb.push_back(lb);
      fwdUb.push_back(ub);
    }
  }
  if (qvars.size() == 0) {
    fwdLb.clear();
    fwdUb.clear();
    return false;
  }
  for (VariableGroupConstIterator lit = lf->termsBegin(); lit != lf->termsEnd();
       ++lit) {
    if (std::find(qvars.begin(), qvars.end(), lit->first) == qvars.end()) {
      getTermBnds_(lit->first, lit->second, lb, ub);
      if (lb <= -INFINITY) {
        ++count_inf_lb;
      }
      if (ub >= INFINITY) {
        ++count_inf_ub;
      }
      implLb += lb;
      implUb += ub;
      fwdLb.push_back(lb);
      fwdUb.push_back(ub);
    }
  }
  return true;
}

bool kPowHandler::getQfLfBnds_(RelaxationPtr rel, LinearFunctionPtr lf,
                               QuadraticFunctionPtr nlf, double &implLb,
                               double &implUb, DoubleVector &fwdLb,
                               DoubleVector &fwdUb, UInt &count_inf_lb,
                               UInt &count_inf_ub, VarVector &qvars) {
  double lb = 0, ub = 0;
  VariablePtr v1, v2;
  for (VariablePairGroupConstIterator qit = nlf->begin(); qit != nlf->end();
       ++qit) {
    // Although getRelaxationVar requries problem var
    // and we have original variables here
    // But in the code we just return the variable with same id
    // which is what is intended here
    v1 = rel->getRelaxationVar(qit->first.first);
    v2 = rel->getRelaxationVar(qit->first.second);
    if (v1->getIndex() == v2->getIndex() && lf->hasVar(qit->first.first)) {
      qvars.push_back(qit->first.first);
      getTermBnds_(v1, qit->second, lf->getWeight(qit->first.first), lb, ub);
      if (lb <= -INFINITY) {
        ++count_inf_lb;
      }
      if (ub >= INFINITY) {
        ++count_inf_ub;
      }
      implLb += lb;
      implUb += ub;
      fwdLb.push_back(lb);
      fwdUb.push_back(ub);
    } else {
      getTermBnds_(v1, v2, qit->second, lb, ub);
      if (lb <= -INFINITY) {
        ++count_inf_lb;
      }
      if (ub >= INFINITY) {
        ++count_inf_ub;
      }
      implLb += lb;
      implUb += ub;
      fwdLb.push_back(lb);
      fwdUb.push_back(ub);
    }
  }
  if (qvars.size() == 0) {
    fwdLb.clear();
    fwdUb.clear();
    return false;
  }
  for (VariableGroupConstIterator lit = lf->termsBegin(); lit != lf->termsEnd();
       ++lit) {
    v1 = rel->getRelaxationVar(lit->first);
    if (std::find(qvars.begin(), qvars.end(), lit->first) == qvars.end()) {
      getTermBnds_(v1, lit->second, lb, ub);
      if (lb <= -INFINITY) {
        ++count_inf_lb;
      }
      if (ub >= INFINITY) {
        ++count_inf_ub;
      }
      implLb += lb;
      implUb += ub;
      fwdLb.push_back(lb);
      fwdUb.push_back(ub);
    }
  }
  return true;
}

bool kPowHandler::tightenkPow_(bool *changed) {
  ConstraintPtr c;
  double implLb, implUb;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr nlf;
  ObjectivePtr obj;
  VarVector qvars;
  DoubleVector fwdLb, fwdUb;
  DoubleVector::iterator liter, uiter;
  double lb, ub, clb, cub;
  bool c1, c2;
  UInt count_inf_lb = 0, count_inf_ub = 0;

  obj = p_->getObjective();
  cub = env_->getOptions()->findDouble("obj_cut_off")->getValue() -
        obj->getConstant();
  clb = -INFINITY;

  if (cub < INFINITY) {
    implLb = 0.0;
    implUb = 0.0;
    if (obj->getFunctionType() == Quadratic) {
      lf = obj->getFunction()->getLinearFunction();
      if (lf) {
        nlf = obj->getFunction()->getQuadraticFunction();
        if (nlf) {
          // Forward Propagation
          if (getQfLfBnds_(lf, nlf, implLb, implUb, fwdLb, fwdUb, count_inf_lb,
                           count_inf_ub, qvars)) {
            doQT_ = true;
            // Backward Propagation
            liter = fwdLb.begin();
            uiter = fwdUb.begin();
            clb = clb > implLb ? clb : implLb;
            cub = cub < implUb ? cub : implUb;
            for (VariablePairGroupConstIterator qit = nlf->begin();
                 qit != nlf->end(); ++qit) {
              if (qit->first.first->getIndex() ==
                      qit->first.second->getIndex() &&
                  lf->hasVar(qit->first.first)) {
                lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                          Upper, implUb, count_inf_ub);
                ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                          Lower, implLb, count_inf_lb);
                c1 = false;
                if (calcVarBnd_(qit->first.first, qit->second,
                                lf->getWeight(qit->first.first), lb, ub, &c1)) {
                  fwdLb.clear();
                  fwdUb.clear();
                  return true;
                }
                if (c1 == true) {
                  //++bStats_.vBndq;
                  *changed = true;
                }
                ++liter;
                ++uiter;
              } else {
                lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                          Upper, implUb, count_inf_ub);
                ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                          Lower, implLb, count_inf_lb);
                c1 = false;
                c2 = false;
                if (calcVarBnd_(qit->first.first, qit->first.second,
                                qit->second, lb, ub, &c1, &c2)) {
                  fwdLb.clear();
                  fwdUb.clear();
                  return true;
                }
                if (c1 == true) {
                  //++bStats_.vBndq;
                  *changed = true;
                }
                if (c2 == true) {
                  //++bStats_.vBndq;
                  *changed = true;
                }
                ++liter;
                ++uiter;
              }
            }

            for (VariableGroupConstIterator lit = lf->termsBegin();
                 lit != lf->termsEnd(); ++lit) {
              if (std::find(qvars.begin(), qvars.end(), lit->first) ==
                  qvars.end()) {
                lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                          Upper, implUb, count_inf_ub);
                ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                          Lower, implLb, count_inf_lb);
                c1 = false;
                if (calcVarBnd_(lit->first, lit->second, lb, ub, &c1)) {
                  fwdLb.clear();
                  fwdUb.clear();
                  return true;
                }
                if (c1 == true) {
                  //++bStats_.vBndq;
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
  }

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
       ++cit) {
    c = *cit;
    implLb = 0.0;
    implUb = 0.0;
    count_inf_lb = 0;
    count_inf_ub = 0;
    // If the constraint is bilinear then simple tightening suffices
    if (c->getFunctionType() == Quadratic) {
      lf = c->getFunction()->getLinearFunction();
      if (lf) {
        nlf = c->getFunction()->getQuadraticFunction();
        if (nlf) {
          // Forward Propagation
          if (false == getQfLfBnds_(lf, nlf, implLb, implUb, fwdLb, fwdUb,
                                    count_inf_lb, count_inf_ub, qvars)) {
            continue;
          }
          doQT_ = true;
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
            //++bStats_.cBndq;
          }
          if (cub < c->getUb() - aTol_) {
            p_->changeBound(c, Upper, cub);
            //++bStats_.cBndq;
          }
          for (VariablePairGroupConstIterator qit = nlf->begin();
               qit != nlf->end(); ++qit) {
            if (qit->first.first->getIndex() == qit->first.second->getIndex() &&
                lf->hasVar(qit->first.first)) {
              lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                        Upper, implUb, count_inf_ub);
              ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                        Lower, implLb, count_inf_lb);
              c1 = false;
              if (calcVarBnd_(qit->first.first, qit->second,
                              lf->getWeight(qit->first.first), lb, ub, &c1)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                //++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            } else {
              lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                        Upper, implUb, count_inf_ub);
              ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                        Lower, implLb, count_inf_lb);
              c1 = false;
              c2 = false;
              if (calcVarBnd_(qit->first.first, qit->first.second, qit->second,
                              lb, ub, &c1, &c2)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                //++bStats_.vBndq;
                *changed = true;
              }
              if (c2 == true) {
                //++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            }
          }

          for (VariableGroupConstIterator lit = lf->termsBegin();
               lit != lf->termsEnd(); ++lit) {
            if (std::find(qvars.begin(), qvars.end(), lit->first) ==
                qvars.end()) {
              lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                        Upper, implUb, count_inf_ub);
              ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                        Lower, implLb, count_inf_lb);
              c1 = false;
              if (calcVarBnd_(lit->first, lit->second, lb, ub, &c1)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                //++bStats_.vBndq;
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

bool kPowHandler::tightenkPow_(RelaxationPtr rel, double bestSol, bool *changed,
                               ModVector &p_mods, ModVector &r_mods) {
  ConstraintPtr c;
  double implLb, implUb;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr nlf;
  ObjectivePtr obj;
  VarVector qvars;
  DoubleVector fwdLb, fwdUb;
  DoubleVector::iterator liter, uiter;
  double lb, ub, clb, cub;
  bool c1, c2;
  UInt count_inf_lb = 0, count_inf_ub = 0;

  obj = orig_->getObjective();
  cub = bestSol - obj->getConstant();
  clb = -INFINITY;

  if (cub < INFINITY) {
    implLb = 0.0;
    implUb = 0.0;
    if (obj->getFunctionType() == Quadratic) {
      lf = obj->getFunction()->getLinearFunction();
      if (lf) {
        nlf = obj->getFunction()->getQuadraticFunction();
        if (nlf) {
          // Forward Propagation
          if (getQfLfBnds_(rel, lf, nlf, implLb, implUb, fwdLb, fwdUb,
                           count_inf_lb, count_inf_ub, qvars)) {
            // Backward Propagation
            liter = fwdLb.begin();
            uiter = fwdUb.begin();
            clb = clb > implLb ? clb : implLb;
            cub = cub < implUb ? cub : implUb;
            for (VariablePairGroupConstIterator qit = nlf->begin();
                 qit != nlf->end(); ++qit) {
              if (qit->first.first->getIndex() ==
                      qit->first.second->getIndex() &&
                  lf->hasVar(qit->first.first)) {
                lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                          Upper, implUb, count_inf_ub);
                ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                          Lower, implLb, count_inf_lb);
                c1 = false;
                if (calcVarBnd_(rel,
                                p_->getVariable(qit->first.first->getIndex()),
                                qit->second, lf->getWeight(qit->first.first),
                                lb, ub, &c1, p_mods, r_mods)) {
                  fwdLb.clear();
                  fwdUb.clear();
                  return true;
                }
                if (c1 == true) {
                  //++bStats_.vBndq;
                  *changed = true;
                }
                ++liter;
                ++uiter;
              } else {
                lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                          Upper, implUb, count_inf_ub);
                ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                          Lower, implLb, count_inf_lb);
                c1 = false;
                c2 = false;
                if (calcVarBnd_(
                        rel, p_->getVariable(qit->first.first->getIndex()),
                        p_->getVariable(qit->first.second->getIndex()),
                        qit->second, lb, ub, &c1, &c2, p_mods, r_mods)) {
                  fwdLb.clear();
                  fwdUb.clear();
                  return true;
                }
                if (c1 == true) {
                  //++bStats_.vBndq;
                  *changed = true;
                }
                if (c2 == true) {
                  //++bStats_.vBndq;
                  *changed = true;
                }
                ++liter;
                ++uiter;
              }
            }

            for (VariableGroupConstIterator lit = lf->termsBegin();
                 lit != lf->termsEnd(); ++lit) {
              if (std::find(qvars.begin(), qvars.end(), lit->first) ==
                  qvars.end()) {
                lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                          Upper, implUb, count_inf_ub);
                ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                          Lower, implLb, count_inf_lb);
                c1 = false;
                if (calcVarBnd_(rel, p_->getVariable(lit->first->getIndex()),
                                lit->second, lb, ub, &c1, p_mods, r_mods)) {
                  fwdLb.clear();
                  fwdUb.clear();
                  return true;
                }
                if (c1 == true) {
                  //++bStats_.vBndq;
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
  }

  for (ConstraintConstIterator cit = orig_->consBegin();
       cit != orig_->consEnd(); ++cit) {
    c = *cit;
    implLb = 0.0;
    implUb = 0.0;
    count_inf_lb = 0;
    count_inf_ub = 0;
    // If the constraint is bilinear then simple tightening suffices
    if (c->getFunctionType() == Quadratic) {
      lf = c->getFunction()->getLinearFunction();
      if (lf) {
        nlf = c->getFunction()->getQuadraticFunction();
        if (nlf) {
          // Forward Propagation
          if (false == getQfLfBnds_(rel, lf, nlf, implLb, implUb, fwdLb, fwdUb,
                                    count_inf_lb, count_inf_ub, qvars)) {
            continue;
          }
          clb = c->getLb();
          cub = c->getUb();

          // constraint is infeasible
          if (implLb > cub + aTol_ || implUb < clb - aTol_) {
            return true;
          }

          // Backward Propagation
          liter = fwdLb.begin();
          uiter = fwdUb.begin();
          clb = clb > implLb ? clb : implLb;
          cub = cub < implUb ? cub : implUb;
          for (VariablePairGroupConstIterator qit = nlf->begin();
               qit != nlf->end(); ++qit) {
            if (qit->first.first->getIndex() == qit->first.second->getIndex() &&
                lf->hasVar(qit->first.first)) {
              lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                        Upper, implUb, count_inf_ub);
              ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                        Lower, implLb, count_inf_lb);
              c1 = false;
              if (calcVarBnd_(rel,
                              p_->getVariable(qit->first.first->getIndex()),
                              qit->second, lf->getWeight(qit->first.first), lb,
                              ub, &c1, p_mods, r_mods)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                //++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            } else {
              lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                        Upper, implUb, count_inf_ub);
              ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                        Lower, implLb, count_inf_lb);
              c1 = false;
              c2 = false;
              if (calcVarBnd_(rel,
                              p_->getVariable(qit->first.first->getIndex()),
                              p_->getVariable(qit->first.second->getIndex()),
                              qit->second, lb, ub, &c1, &c2, p_mods, r_mods)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                //++bStats_.vBndq;
                *changed = true;
              }
              if (c2 == true) {
                //++bStats_.vBndq;
                *changed = true;
              }
              ++liter;
              ++uiter;
            }
          }

          for (VariableGroupConstIterator lit = lf->termsBegin();
               lit != lf->termsEnd(); ++lit) {
            if (std::find(qvars.begin(), qvars.end(), lit->first) ==
                qvars.end()) {
              lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter,
                                        Upper, implUb, count_inf_ub);
              ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter,
                                        Lower, implLb, count_inf_lb);
              c1 = false;
              if (calcVarBnd_(rel, p_->getVariable(lit->first->getIndex()),
                              lit->second, lb, ub, &c1, p_mods, r_mods)) {
                fwdLb.clear();
                fwdUb.clear();
                return true;
              }
              if (c1 == true) {
                //++bStats_.vBndq;
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

void kPowHandler::getLfBnds_(LinearFunctionPtr lf, double &implLb,
                             double &implUb, DoubleVector &fwdLb,
                             DoubleVector &fwdUb, UInt &count_inf_lb,
                             UInt &count_inf_ub) {
  double lb = 0, ub = 0;
  for (VariableGroupConstIterator lit = lf->termsBegin(); lit != lf->termsEnd();
       ++lit) {
    getTermBnds_(lit->first, lit->second, lb, ub);
    if (lb <= -INFINITY) {
      ++count_inf_lb;
    }
    if (ub >= INFINITY) {
      ++count_inf_ub;
    }
    implLb += lb;
    implUb += ub;
    fwdLb.push_back(lb);
    fwdUb.push_back(ub);
  }
}

void kPowHandler::getQfBnds_(QuadraticFunctionPtr nlf, double &implLb,
                             double &implUb, DoubleVector &fwdLb,
                             DoubleVector &fwdUb, UInt &count_inf_lb,
                             UInt &count_inf_ub) {
  double lb = 0, ub = 0;
  for (VariablePairGroupConstIterator qit = nlf->begin(); qit != nlf->end();
       ++qit) {
    getTermBnds_(qit->first.first, qit->first.second, qit->second, lb, ub);
    if (lb <= -INFINITY) {
      ++count_inf_lb;
    }
    if (ub >= INFINITY) {
      ++count_inf_ub;
    }
    implLb += lb;
    implUb += ub;
    fwdLb.push_back(lb);
    fwdUb.push_back(ub);
  }
}

bool kPowHandler::tightenSimple_(bool *changed) {
  double implLb, implUb;
  DoubleVector fwdLb, fwdUb;
  DoubleVector::iterator liter, uiter;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr nlf;
  ObjectivePtr obj;
  double clb, cub, lb, ub;
  bool c1, c2;
  UInt count_inf_lb = 0, count_inf_ub = 0;

  obj = p_->getObjective();
  cub = env_->getOptions()->findDouble("obj_cut_off")->getValue() -
        obj->getConstant();
  clb = -INFINITY;

  if (cub < INFINITY) {
    implLb = 0.0;
    implUb = 0.0;
    lf = obj->getFunction()->getLinearFunction();
    nlf = obj->getFunction()->getQuadraticFunction();
    // Forward Propagation
    if (lf) {
      getLfBnds_(lf, implLb, implUb, fwdLb, fwdUb, count_inf_lb, count_inf_ub);
    }
    if (nlf) {
      getQfBnds_(nlf, implLb, implUb, fwdLb, fwdUb, count_inf_lb, count_inf_ub);
    }
    liter = fwdLb.begin();
    uiter = fwdUb.begin();
    clb = clb > implLb ? clb : implLb;
    cub = cub < implUb ? cub : implUb;
    if (lf) {
      for (VariableGroupConstIterator lit = lf->termsBegin();
           lit != lf->termsEnd(); ++lit) {
        lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter, Upper,
                                  implUb, count_inf_ub);
        ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter, Lower,
                                  implLb, count_inf_lb);
        c1 = false;
        if (calcVarBnd_(lit->first, lit->second, lb, ub, &c1)) {
          fwdLb.clear();
          fwdUb.clear();
          return true;
        }
        if (c1 == true) {
          //++bStats_.vBnds;
          *changed = true;
        }
        ++liter;
        ++uiter;
      }
    }

    if (nlf) {
      for (VariablePairGroupConstIterator qit = nlf->begin(); qit != nlf->end();
           ++qit) {
        lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter, Upper,
                                  implUb, count_inf_ub);
        ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter, Lower,
                                  implLb, count_inf_lb);
        c1 = false;
        c2 = false;
        if (calcVarBnd_(qit->first.first, qit->first.second, qit->second, lb,
                        ub, &c1, &c2)) {
          fwdLb.clear();
          fwdUb.clear();
          return true;
        }
        if (c1 == true) {
          //++bStats_.vBnds;
          *changed = true;
        }
        if (c2 == true) {
          //++bStats_.vBnds;
          *changed = true;
        }
        ++liter;
        ++uiter;
      }
    }
    fwdLb.clear();
    fwdUb.clear();
  }

  // Constraint propagation
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
       ++cit) {
    c = *cit;
    implLb = 0.0;
    implUb = 0.0;
    count_inf_lb = 0;
    count_inf_ub = 0;
    if (c->getFunctionType() == Quadratic || c->getFunctionType() == Bilinear) {
      // delete the constraint if unbounded on both sides
      clb = c->getLb();
      cub = c->getUb();

      if (cub == INFINITY && clb == -INFINITY) {
        p_->markDelete(c);
        *changed = true;
        continue;
      }
      lf = c->getFunction()->getLinearFunction();
      nlf = c->getFunction()->getQuadraticFunction();
      // Forward Propagation
      if (lf) {
        getLfBnds_(lf, implLb, implUb, fwdLb, fwdUb, count_inf_lb,
                   count_inf_ub);
      }

      if (nlf) {
        getQfBnds_(nlf, implLb, implUb, fwdLb, fwdUb, count_inf_lb,
                   count_inf_ub);
      }
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
      // if (clb > c->getLb() + aTol_) {
      //  p_->changeBound(c, Lower, clb);
      //  //++bStats_.cBnds;
      //}
      // if (cub < c->getUb() - aTol_) {
      //  p_->changeBound(c, Upper, cub);
      //  //++bStats_.cBnds;
      //}
      if (lf) {
        for (VariableGroupConstIterator lit = lf->termsBegin();
             lit != lf->termsEnd(); ++lit) {
          lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter, Upper,
                                    implUb, count_inf_ub);
          ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter, Lower,
                                    implLb, count_inf_lb);
          c1 = false;
          if (calcVarBnd_(lit->first, lit->second, lb, ub, &c1)) {
            fwdLb.clear();
            fwdUb.clear();
            return true;
          }
          if (c1 == true) {
            //++bStats_.vBnds;
            *changed = true;
          }
          ++liter;
          ++uiter;
        }
      }

      if (nlf) {
        for (VariablePairGroupConstIterator qit = nlf->begin(); qit != nlf->end();
             ++qit) {
          lb = clb - getSumExcept1_(fwdUb.begin(), fwdUb.end(), uiter, Upper,
                                    implUb, count_inf_ub);
          ub = cub - getSumExcept1_(fwdLb.begin(), fwdLb.end(), liter, Lower,
                                    implLb, count_inf_lb);
          c1 = false;
          c2 = false;
          if (calcVarBnd_(qit->first.first, qit->first.second, qit->second, lb,
                          ub, &c1, &c2)) {
            fwdLb.clear();
            fwdUb.clear();
            return true;
          }
          if (c1 == true) {
            //++bStats_.vBnds;
            *changed = true;
          }
          if (c2 == true) {
            //++bStats_.vBnds;
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

int kPowHandler::updatePBounds_(VariablePtr v, double lb, double ub,
                                bool *changed) {
  if (ub < v->getLb() - bTol_ || lb > v->getUb() + bTol_) {
#if SPEW
    logger_->msgStream(LogDebug2)
        << me_ << "inconsistent bounds of" << v->getName() << " " << v->getLb()
        << " " << v->getUb() << std::endl;
#endif
    std::cout<<"v->getLb()="<<v->getLb()<<"\n";
    std::cout<<"v->getLb()="<<v->getUb()<<"\n";
    std::cout<<"bTol_="<<bTol_<<"\n";
    return -1;
  }

  if (ub < v->getUb() - bTol_ &&
      (v->getUb() == INFINITY || ub < v->getUb() - fabs(v->getUb()) * rTol_)) {
    p_->changeBound(v, Upper, ub);
    *changed = true;
    ++pStats_.vBnd;
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "new ub of " << v->getName()
                                  << " = " << v->getUb() << std::endl;
#endif
  }
  if (lb > v->getLb() + aTol_ &&
      (v->getLb() == -INFINITY || lb > v->getLb() + fabs(v->getLb()) * rTol_)) {
    p_->changeBound(v, Lower, lb);
    *changed = true;
    ++pStats_.vBnd;
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "new lb of " << v->getName()
                                  << " = " << v->getLb() << std::endl;
#endif
  }

  return 1;
}

int kPowHandler::updatePBounds_(VariablePtr v, double lb, double ub,
                                RelaxationPtr rel, bool mod_rel, bool *changed,
                                ModVector &p_mods, ModVector &r_mods) {
  VarBoundMod2Ptr b2mod;
  VarBoundModPtr bmod;

  if (lb > v->getUb() + bTol_ || ub < v->getLb() - bTol_) {
    return -1;
  }

  if (lb > v->getLb() + bTol_ && ub < v->getUb() - bTol_ &&
      (v->getLb() == -INFINITY || lb > v->getLb() + rTol_ * fabs(v->getLb())) &&
      (v->getUb() == INFINITY || ub < v->getUb() - rTol_ * fabs(v->getUb()))) {
    *changed = true;
    ++pStats_.vBnd;
    b2mod = (VarBoundMod2Ptr) new VarBoundMod2(v, lb, ub);
    b2mod->applyToProblem(p_);
    p_mods.push_back(b2mod);
#if SPEW
    b2mod->write(logger_->msgStream(LogDebug2));
#endif

    if (true == mod_rel) {
      b2mod =
          (VarBoundMod2Ptr) new VarBoundMod2(rel->getRelaxationVar(v), lb, ub);
      b2mod->applyToProblem(rel);
      r_mods.push_back(b2mod);
    }
  } else if (lb > v->getLb() + bTol_ &&
             (v->getLb() == -INFINITY ||
              lb > v->getLb() + rTol_ * fabs(v->getLb()))) {
    ++pStats_.vBnd;
    *changed = true;
    bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, lb);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
#if SPEW
    bmod->write(logger_->msgStream(LogDebug2));
#endif

    if (true == mod_rel) {
      bmod =
          (VarBoundModPtr) new VarBoundMod(rel->getRelaxationVar(v), Lower, lb);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  } else if (ub < v->getUb() - bTol_ &&
             (v->getUb() == INFINITY ||
              ub < v->getUb() - rTol_ * fabs(v->getUb()))) {
    ++pStats_.vBnd;
    *changed = true;
    bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, ub);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
#if SPEW
    bmod->write(logger_->msgStream(LogDebug2));
#endif

    if (true == mod_rel) {
      bmod =
          (VarBoundModPtr) new VarBoundMod(rel->getRelaxationVar(v), Upper, ub);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  }
  return 0;
}

void kPowHandler::upSqCon_(ConstraintPtr con, VariablePtr x, VariablePtr y,
                           double k, RelaxationPtr rel, ModVector &r_mods) {
  LinearFunctionPtr lf = con->getLinearFunction();
  double a_x = lf->getWeight(x);
  double lb = x->getLb();
  double ub = x->getUb();
  LinConModPtr lmod;
  double rhs;
  double eps = aTol_ / 10.0;

  assert(fabs(lf->getWeight(y) - 1.0) <= 1e-8);
  // y - (lb+ub)x <= -ub*lb
  if ((lb * lb + a_x * lb < con->getUb() - eps) ||
      (ub * ub + a_x * ub < con->getUb() - eps)) {
    lf = getNewKpowLf_(y, x, k, x->getLb(), x->getUb(), rhs, 2);
    lmod = (LinConModPtr) new LinConMod(con, lf, -INFINITY, rhs);
    lmod->applyToProblem(rel);
    r_mods.push_back(lmod);
  }
}

bool kPowHandler::varBndsFromCons_(bool *changed) {
  bool is_inf = false;
  for (LinkPowMapIter it = xkFuns_.begin(); it != xkFuns_.end(); ++it) {
    is_inf = propKPowBnds_(it, changed);
    if (true == is_inf) {
      return true;
    }
  }
  return false;
}

void kPowHandler::writeStats(std::ostream &out) const {
  out << me_ << "Statistics for presolve by kPowHandler:" << std::endl
      << me_ << "Number of iterations           = " << pStats_.iters
      << std::endl
      << me_ << "Time taken in initial presolve = " << pStats_.time << std::endl
      << me_ << "Time taken in node presolves   = " << pStats_.timeN
      << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd << std::endl
      << me_ << "Changes in nodes               = " << pStats_.nMods
      << std::endl;

  out << me_ << "Statistics for separation by kPowHandler:" << std::endl
      << me_ << "Number of calls to separate    = " << sStats_.iters
      << std::endl
      << me_ << "Number of cuts added           = " << sStats_.cuts << std::endl
      << me_ << "Time taken in separation       = " << sStats_.time
      << std::endl;

  out << me_ << "Statistics for Bound Tightening:" << std::endl
      << me_
      << "Number of LPs solved                               = " << bStats_.nLP
      << std::endl
      << me_
      << "Number of variables for which default lb was added = " << bStats_.dlb
      << std::endl
      << me_
      << "Number of variables for which default ub was added = " << bStats_.dub
      << std::endl
      << me_ << "Time taken in solving LPs                          = "
      << bStats_.timeLP << std::endl;

  if (nlpStats_.flag) {
    out << me_ << "Statistics for NLP solved by kPowHandler:" << std::endl
        << me_
        << "Number of NLPs solved                         = " << nlpStats_.nlp
        << std::endl
        << me_
        << "Number of NLPs optimal                        = " << nlpStats_.opt
        << std::endl
        << me_
        << "Number of NLPs infeasible                     = " << nlpStats_.inf
        << std::endl
        << me_ << "Number of NLPs for which EngineIterationLimit = "
        << nlpStats_.iter_limit << std::endl;
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
