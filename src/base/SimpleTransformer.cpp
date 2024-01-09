//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file SimpleTransformer.cpp
 * \brief Define class for simple reformulations a problem to make it suitable
 * for handlers.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include "SimpleTransformer.h"

#include <cmath>
#include <iostream>

#include "CGraph.h"
#include "CNode.h"
#include "Constraint.h"
#include "CxUnivarHandler.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "IntVarHandler.h"
#include "LinBil.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Option.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "QuadHandler.h"
#include "QuadraticFunction.h"
#include "Solution.h"
#include "Timer.h"
#include "Variable.h"
#include "YEqCGs.h"
#include "YEqLFs.h"
#include "YEqQfBil.h"
#include "YEqUCGs.h"
#include "YEqVars.h"
#include "kPowHandler.h"

// #define SPEW 1

using namespace Minotaur;
const std::string SimpleTransformer::me_ = "SimpleTransformer: ";

SimpleTransformer::SimpleTransformer()
  : Transformer(),
    yBiVars_(0)
{
  resetStats_();
}

SimpleTransformer::SimpleTransformer(EnvPtr env, ProblemPtr p, LPEnginePtr bte,
                                     LPEnginePtr cute, EnginePtr nlpe)
  : Transformer(env, p),
    bte_(bte),
    cute_(cute),
    nlpe_(nlpe),
    yBiVars_(0)
{
  resetStats_();
}

SimpleTransformer::~SimpleTransformer()
{
  delete yBiVars_;
  delete yQfBil_;
}

void SimpleTransformer::absRef_(LinearFunctionPtr lfl, VariablePtr vl,
                                double dl, VariablePtr& v, double& d)
{
  if(lfl) {
    vl = newVar_(lfl, dl, newp_);
  } else if(vl && fabs(dl) > zTol_) {
    vl = newVar_(vl, dl, newp_);
  }
  if(vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    CNode* n1 = cg->newNode(vl);
    CNode* n2 = 0;

    n1 = cg->newNode(OpAbs, n1, n2);
    cg->setOut(n1);
    cg->finalize();
    v = newVar_(cg, newp_);
  } else {
    d = fabs(dl);
  }
}

void SimpleTransformer::bilRef_(LinearFunctionPtr lfl, VariablePtr vl,
                                double dl, LinearFunctionPtr lfr,
                                VariablePtr vr, double dr,
                                LinearFunctionPtr& lf, VariablePtr& v,
                                double& d)
{
  if(lfl) {
    // vl = newVar_(lfl, dl, newp_);
    if(vr) {
      vr = newVar_(vr, dr, newp_);
    } else if(lfr) {
      vr = newVar_(lfr, dr, newp_);
    }
    if(vr) {
      // lf.reset();
      vl = newVar_(lfl, dl, newp_);
      lf = 0;
      d = 0;
      v = newBilVar_(vl, vr);
    } else {
      lf = lfl;
      lf->multiply(dr);
      d = dl * dr;
      v = 0;
    }
  } else if(vl) {
    vl = newVar_(vl, dl, newp_);
    if(lfr) {
      vr = newVar_(lfr, dr, newp_);
    } else if(vr) {
      vr = newVar_(vr, dr, newp_);
    }
    if(vr) {
      v = newBilVar_(vl, vr);
      // lf.reset();
      lf = 0;
      d = 0;
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(vl, dr);
      v = 0;
      d = 0;
    }
  } else if(lfr) {
    lf = lfr;
    lf->multiply(dl);
    d = dl * dr;
    v = 0;
  } else if(vr) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(vr, dl);
    v = 0;
    d = 0;
  } else {
    // lf.reset();
    lf = 0;
    v = 0;
    d = dl * dr;
  }
}

std::string SimpleTransformer::getName() const
{
  return "SimpleTransformer";
}

SolutionPtr SimpleTransformer::getSolOrig(ConstSolutionPtr, int& err)
{
  err = 1;
  return SolutionPtr();
}

SolutionPtr SimpleTransformer::getSolTrans(ConstSolutionPtr, int& err)
{
  err = 1;
  return SolutionPtr();
}

VariablePtr SimpleTransformer::newBilVar_(VariablePtr vl, VariablePtr vr)
{
  CGraphPtr cg = (CGraphPtr) new CGraph();
  CNode* n1 = cg->newNode(vl);
  CNode* n2 = 0;
  VariablePtr ov = VariablePtr();
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr cnew;

  if(vl == vr) {
    n2 = cg->newNode(OpSqr, n1, n2);
  } else {
    n2 = cg->newNode(vr);
    n2 = cg->newNode(OpMult, n1, n2);
  }
  cg->setOut(n2);
  cg->finalize();

  if(vl == vr) {
    ov = newVar_(cg, newp_);
  } else {
    ov = yBiVars_->findY(cg);
    if(!ov) {
      ov = newp_->newVariable();
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(ov, -1.0);
      f = (FunctionPtr) new Function(lf, cg);
      cnew = newp_->newConstraint(f, 0.0, 0.0);
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "added new constraint" << std::endl;
      cnew->write(logger_->msgStream(LogDebug));
#endif
      qHandler_->addConstraint(cnew);
      yBiVars_->insert(ov, cg);
    }
  }
  return ov;
}

void SimpleTransformer::powKRef_(LinearFunctionPtr lfl, VariablePtr vl,
                                 double dl, double k, LinearFunctionPtr& lf,
                                 VariablePtr& v, double& d)
{
  CNode *n1, *n2;
  if(k < 1) {
    assert(!"powers less than one can not be handled yet!");
  } else if(k < -zTol_) {
    assert(!"negative powers can not be handled yet!");
  }
  // else if (fabs(k / 2 - floor(k / 2 + 0.5)) > zTol_) {
  //   logger_->errStream() << "odd powers can not be handled yet!" <<
  //   std::endl;
  // }

  if(lfl) {
    vl = newVar_(lfl, dl, newp_);
  }
  if(vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    n1 = cg->newNode(vl);
    n2 = cg->newNode(k);
    n2 = cg->newNode(OpPowK, n1, n2);
    cg->setOut(n2);
    cg->finalize();
    v = 0;
    // lf.reset();
    lf = 0;
    v = newVar_(cg, newp_);
    d = 0;
  } else {
    d = pow(dl, k);
  }
}

// Returns one of the following four:
// #1 lf + d,
// #2  v + d, or
// d.
// d may be zero, lf and v may simultaneously be NULL.
// TODO: return an error code if there is an error?
void SimpleTransformer::recursRef_(const CNode* node, LinearFunctionPtr& lf,
                                   VariablePtr& v, double& d)
{
  double dl = 0;
  double dr = 0;
  LinearFunctionPtr lfl = LinearFunctionPtr();
  LinearFunctionPtr lfr = LinearFunctionPtr();
  VariablePtr vl = VariablePtr();
  VariablePtr vr = VariablePtr();
  VariablePtr v2 = VariablePtr();
  CNode* n1 = 0;

  lf = LinearFunctionPtr(); // NULL
  v = VariablePtr();
  d = 0.0;

  switch(node->getOp()) {
  case(OpAbs):
  case(OpAcos):
  case(OpAcosh):
  case(OpAsin):
  case(OpAsinh):
  case(OpAtan):
  case(OpAtanh):
  case(OpCeil):
  case(OpCos):
  case(OpCosh):
  case(OpCPow):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case(OpDiv):
    // (lfl+vl+dl)/(lfr+vr+dr), there are many sub-cases
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);

    if(!lfl && !vl && !lfr && !vr && fabs(dl) < zTol_ && fabs(dr) < zTol_) {
      logger_->msgStream(LogDebug) << "seeing zero by zero" << std::endl;
    } else if(!lfr && !vr && fabs(dr) < zTol_) {
      logger_->msgStream(LogDebug) << "seeing division by zero" << std::endl;
    } else if(!lfl && !vl && fabs(dl) < zTol_) {
      d = 0.0;
    } else if(!lfr && !vr && fabs(dr - 1.0) < zTol_) {
      d = dl;
      lf = lfl;
      v = vl;
    } else if(!lfr && !vr) {
      d = dl / dr;
      lf = lfr->clone();
      lf->multiply(1.0 / dr);
    } else {
      CGraphPtr cg = (CGraphPtr) new CGraph();
      CNode* n2 = 0;
      if(lfr) {
        v2 = newVar_(lfr, dr, newp_);
      } else {
        v2 = newVar_(vr, dr, newp_);
      }

      // 1/v2
      n1 = cg->newNode(1.0);
      n2 = cg->newNode(v2);
      n1 = cg->newNode(OpDiv, n1, n2);
      cg->setOut(n1);
      cg->finalize();
      v2 = newVar_(cg, newp_);

      // lfr.reset();
      lfr = 0;
      // now we have to do (lfl + vl + dl)*v2
      bilRef_(lfl, vl, dl, lfr, v2, 0.0, lf, v, d);
    }
    break;
  case(OpExp):
  case(OpFloor):
  case(OpIntDiv):
  case(OpLog):
  case(OpLog10):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case(OpMinus):
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);
    d = dl - dr;
    if(!vr && !lfr) {
      v = vl;
      lf = lfl;
    } else if(!vl && !lfl) {
      if(lfr) {
        lf = lfr;
        lf->multiply(-1.0);
      } else if(vr) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(vr, -1.0);
      }
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
      if(lfl) {
        lf->add(lfl);
      } else if(vl) {
        lf->incTerm(vl, 1.0);
      }
      if(lfr) {
        lfr->multiply(-1.0);
        lf->add(lfr);
      } else if(vr) {
        lf->incTerm(vr, -1.0);
      }
      v = 0;
    }
    break;
  case(OpMult):
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);
    bilRef_(lfl, vl, dl, lfr, vr, dr, lf, v, d);
    break;
  case(OpNone):
    break;
  case(OpInt):
  case(OpNum):
    d = node->getVal();
    break;
  case(OpPlus):
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);
    d = dl + dr;
    if(!vl && !lfl) {
      v = vr;
      lf = lfr;
    } else if(!vr && !lfr) {
      v = vl;
      lf = lfl;
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
      if(lfl) {
        lf->add(lfl);
      } else if(vl) {
        lf->incTerm(vl, 1.0);
      }
      if(lfr) {
        lf->add(lfr);
      } else if(vr) {
        lf->incTerm(vr, 1.0);
      }
      v = 0;
    }
    break;
  case(OpPow):
    assert(!"not implemented!");
    break;
  case(OpPowK):
    recursRef_(node->getL(), lfl, vl, dl);
    powKRef_(lfl, vl, dl, node->getR()->getVal(), lf, v, d);
    break;
  case(OpRound):
    break;
  case(OpSin):
    break;
  case(OpSinh):
    break;
  case(OpSqr):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case(OpSqrt):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case(OpSumList):
    d = 0;
    lf = (LinearFunctionPtr) new LinearFunction();
    for(CNode** it = node->getListL(); it != node->getListR(); ++it) {
      n1 = *it;
      // lfl.reset();
      lfl = 0;
      vl = 0;
      dl = 0;
      recursRef_(n1, lfl, vl, dl);
      d += dl;
      if(lfl) {
        lf->add(lfl);
      } else if(vl) {
        lf->incTerm(vl, 1.0);
      }
    }
    break;
  case(OpTan):
    break;
  case(OpTanh):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case(OpUMinus):
    recursRef_(node->getL(), lfl, vl, dl);
    d = -1.0 * dl;
    if(lfl) {
      lf = lfl;
      lf->multiply(-1.0);
    } else if(vl) {
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(vl, -1.0);
    }
    break;
  case(OpVar):
    v = newp_->getVariable(node->getV()->getId());
    break;
  default:
    assert(!"cannot evaluate!");
  }

  assert(!lf || !v);
  if(lf && lf->getNumTerms() == 1 &&
     fabs(lf->termsBegin()->second - 1.0) < zTol_) { // return v, not lf
    v = lf->termsBegin()->first;
    // lf.reset();
    lf = 0;
  }
}

void SimpleTransformer::refNonlinCons_(ConstProblemPtr oldp)
{
  ConstraintPtr c, cnew;
  FunctionPtr f, f2;
  CGraphPtr cg;
  LinearFunctionPtr lf = 0, lf2;
  QuadraticFunctionPtr qf, qf2;
  double d, lb, ub;
  VariablePtr v = VariablePtr();

  assert(oldp && newp_);

  for(ConstraintConstIterator it = oldp->consBegin(); it != oldp->consEnd();
      ++it) {
    c = *it;
    f = c->getFunction();
    if(f && f->getType() != Constant && f->getType() != Linear) {
      lf = f->getLinearFunction();
      if(lf) {
        lf2 = lf->cloneWithVars(newp_->varsBegin());
      } else {
        lf2 = (LinearFunctionPtr) new LinearFunction();
      }
      // lf.reset();
      v = 0;
      d = 0.0;
      lf = 0;
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "reformulating the constraint" << std::endl;
      c->write(logger_->msgStream(LogDebug));
#endif
      qf = f->getQuadraticFunction();
      if(qf) {
        qf2 = qf->cloneWithVars(newp_->varsBegin());
        refQuadCons_(qf2, lf);
        delete qf2;
      }

      if(f->getNonlinearFunction()) {
        cg = dynamic_cast<CGraph*>(f->getNonlinearFunction());
        assert(cg || qf);
        recursRef_(cg->getOut(), lf, v, d);
      }
      if(lf) {
        lf2->add(lf);
        if(lf2->getNumTerms() > 1) {
          f2 = (FunctionPtr) new Function(lf2);
          cnew = newp_->newConstraint(f2, c->getLb() - d, c->getUb() - d);
          lHandler_->addConstraint(cnew);
        } else if(lf2->getNumTerms() == 1) {
          v = lf->termsBegin()->first;
          d = lf->termsBegin()->second;
          if(d > 0) {
            lb = c->getLb() / d;
            ub = c->getUb() / d;
          } else {
            lb = c->getUb() / d;
            ub = c->getLb() / d;
          }
          if(lb > v->getLb()) {
            newp_->changeBound(v, Lower, lb);
          }
          if(ub < v->getUb()) {
            newp_->changeBound(v, Upper, ub);
          }
          delete lf2;
#if SPEW
          logger_->msgStream(LogDebug)
              << me_ << "new bounds on variable " << std::endl;
          v->write(logger_->msgStream(LogDebug));
#endif
        } else if((lf2->getNumTerms() == 0) &&
                  (d > c->getUb() + zTol_ || d < c->getLb() - zTol_)) {
          logger_->msgStream(LogInfo)
              << me_ << "problem infeasible." << std::endl;
        }
        delete lf;
      } else if(v) {
        lf2->incTerm(v, 1.0);
        f2 = (FunctionPtr) new Function(lf2);
        cnew = newp_->newConstraint(f2, c->getLb() - d, c->getUb() - d);
        lHandler_->addConstraint(cnew);
      }
    } // other case already dealt with in copyLinear_()
  }
}

void SimpleTransformer::refNonlinObj_(ConstProblemPtr oldp)
{
  ObjectivePtr obj;
  FunctionPtr f, f2;
  double d = 0;
  VariablePtr v = VariablePtr();
  LinearFunctionPtr lf, lf2;
  QuadraticFunctionPtr qf, qf2;
  CGraphPtr cg;

  assert(newp_);
  assert(oldp);

  obj = oldp->getObjective();
  if(!obj) {
    // already dealt with this case in linearCopy_()
    return;
  }

  f = obj->getFunction();
  if(!f) {
    // already dealt with this case in linearCopy_()
    return;
  }

  if(f->getType() != Linear && f->getType() != Constant) {
    qf2 = (QuadraticFunctionPtr) new QuadraticFunction();
    lf = f->getLinearFunction();
    if(lf) {
      lf2 = lf->cloneWithVars(newp_->varsBegin());
    } else {
      lf2 = (LinearFunctionPtr) new LinearFunction();
    }
#if SPEW
    logger_->msgStream(LogDebug)
        << me_ << "reformulating the objective" << std::endl;
    obj->write(logger_->msgStream(LogDebug));
#endif
    qf = f->getQuadraticFunction();
    if(qf) {
      qf2 = qf->cloneWithVars(newp_->varsBegin());
      refQuadCons_(qf2, lf);
      d = obj->getConstant();
    }

    cg = dynamic_cast<CGraph*>(f->getNonlinearFunction());
    if(cg) {
      recursRef_(cg->getOut(), lf, v, d);
    }
    if(lf) {
      lf2->add(lf);
      if(lf2->getNumTerms() > 0) {
        f2 = (FunctionPtr) new Function(lf2);
      } else {
        f2 = FunctionPtr(); // NULL
      }
      obj = newp_->newObjective(f2, d, Minimize);
    } else if(v) {
      lf2->incTerm(v, 1.0);
      if(lf2->getNumTerms() > 0) {
        f2 = (FunctionPtr) new Function(lf2);
      } else {
        f2 = FunctionPtr(); // NULL
      }
      obj = newp_->newObjective(f2, d, Minimize);
    } else {
      // f2.reset();
      f2 = 0;
      obj = newp_->newObjective(f2, d, Minimize);
      logger_->msgStream(LogDebug)
          << "Problem objective reduced to a constant" << std::endl;
    }
    if(lf) {
      delete lf;
    }
    if(qf2) {
      delete qf2;
    }
  } // else the other case is already handled in copyLinear_()
}

// void SimpleTransformer::refQuadCons_(QuadraticFunctionPtr qf,
//                                     LinearFunctionPtr &lf)
//{
//  VarSet vars;
//  VariablePtr v = VariablePtr();
//  VariablePtr v2 = VariablePtr();
//  VariablePtr v3 = VariablePtr();
//  LinearFunctionPtr lf1,lf2;
//  QuadraticFunctionPtr qf1;
//  FunctionPtr f;
//  ConstraintPtr c;
//  lf = (LinearFunctionPtr) new LinearFunction();
//
//  for (VariablePairGroupConstIterator it = qf->begin();
//                                      it != qf->end(); ++it) {
//    v = it->first.first;
//    if (vars.find(v) == vars.end()) {
//      vars.insert(v);
//      lf1 = (LinearFunctionPtr) new LinearFunction();
//      for (VariablePairGroupConstIterator it1 = it; it1 != qf->end(); ++it1) {
//        if (v == it1->first.first) {
//          lf1->addTerm(it1->first.second, it1->second);
//        }
//      }
//      if (lf1->getNumTerms() == 1) {
//        v2 = yQfBil_->findY(v, lf1->termsBegin()->first);
//        if (!v2) {
//          v2 = newp_->newVariable();
//          lf2 = (LinearFunctionPtr) new LinearFunction();
//          lf2->addTerm(v2, -1.0);
//          qf1 = (QuadraticFunctionPtr) new QuadraticFunction();
//          qf1->addTerm(v, lf1->termsBegin()->first, 1.0);
//          f = (FunctionPtr) new Function(lf2, qf1);
//          c = newp_->newConstraint(f, 0.0, 0.0);
//#if SPEW
//          logger_->msgStream(LogDebug) << me_ << "added new constraint"
//                                          << std::endl;
//          c->write(logger_->msgStream(LogDebug));
//#endif
//          qHandler_->addConstraint(c);
//          yQfBil_->insert(v2, v, lf1->termsBegin()->first);
//        }
//        lf->addTerm(v2, lf1->termsBegin()->second);
//        delete lf1;
//      }
//      else {
//        v2 = yLfs_->findY(lf1, 0.0);
//        if (!v2) {
//          v2 = newp_->newVariable();
//          lf2 = (LinearFunctionPtr) new LinearFunction();
//          lf2 = lf1->cloneWithVars(newp_->varsBegin());
//          lf2->addTerm(v2, -1.0);
//          f = (FunctionPtr) new Function(lf2);
//          c = newp_->newConstraint(f, 0.0, 0.0);
//#if SPEW
//          logger_->msgStream(LogDebug) << me_ << "added new constraint"
//                                          << std::endl;
//          c->write(logger_->msgStream(LogDebug));
//#endif
//          lHandler_->addConstraint(c);
//          yLfs_->insert(v2, lf1, 0.0);
//        }
//        v3 = yQfBil_->findY(v, v2);
//        if (!v3) {
//          v3 = newp_->newVariable();
//          lf2 = (LinearFunctionPtr) new LinearFunction();
//          lf2->addTerm(v3, -1.0);
//          qf1 = (QuadraticFunctionPtr) new QuadraticFunction();
//          qf1->addTerm(v, v2, 1.0);
//          f = (FunctionPtr) new Function(lf2, qf1);
//          c = newp_->newConstraint(f, 0.0, 0.0);
//#if SPEW
//          logger_->msgStream(LogDebug) << me_ << "added new constraint"
//                                          << std::endl;
//          c->write(logger_->msgStream(LogDebug));
//#endif
//          qHandler_->addConstraint(c);
//          yQfBil_->insert(v3, v, v2);
//        }
//        lf->addTerm(v3, 1.0);
//      }
//    }
//  }
//  vars.clear();
//}

bool SimpleTransformer::checkQuadConvexity_()
{
  QfVector qf_vector;
  QfVector::iterator it;
  bool convex_cons;
  bool all_convex = true;
  ConstraintPtr c;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  Convexity sg = Unknown, sg_old = Unknown;

  for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    qf = c->getFunction()->getQuadraticFunction();
    nlf = c->getFunction()->getNonlinearFunction();
    if(nlf) {
      convex_cons = false;
      c->setConvexity(Nonconvex);
      all_convex = false;
      continue;
    }
    if(qf) {
      convex_cons = true;
      qf_vector = qf->findSubgraphs();
      for(it = qf_vector.begin(); it != qf_vector.end(); ++it) {
        sg = (*it)->isConvex();
        if(sg == Nonconvex) {
          convex_cons = false;
          c->setConvexity(Nonconvex);
          all_convex = false;
        } else {
          // to add y=qf part
          if(sg_old != Unknown && sg != sg_old) {
            convex_cons = false;
            c->setConvexity(Nonconvex);
            all_convex = false;
          }
          sg_old = sg;
        }
      }
      if(convex_cons) {
        if(sg == Convex) {
          if(c->getLb() > -INFINITY) {
            convex_cons = false;
            c->setConvexity(Nonconvex);
            all_convex = false;
          } else {
            c->setConvexity(Convex);
            ++stats_.nconv;
          }
        } else {
          if(c->getUb() < INFINITY) {
            convex_cons = false;
            c->setConvexity(Nonconvex);
            all_convex = false;
          } else {
            c->setConvexity(Convex);
            ++stats_.nconv;
          }
        }
      }
      for(it = qf_vector.begin(); it != qf_vector.end(); ++it) {
        delete *it;
      }
      qf_vector.clear();
    }
  }
  qf = p_->getObjective()->getFunction()->getQuadraticFunction();
  nlf = p_->getObjective()->getFunction()->getNonlinearFunction();
  if(nlf) {
    convex_cons = false;
    stats_.objConv = 2;
    all_convex = false;
  } else if(qf) {
    convex_cons = true;
    qf_vector = qf->findSubgraphs();
    for(it = qf_vector.begin(); it != qf_vector.end(); ++it) {
      sg = (*it)->isConvex();
      if(sg == Nonconvex || sg == Concave) {
        convex_cons = false;
        all_convex = false;
      }
    }
    for(it = qf_vector.begin(); it != qf_vector.end(); ++it) {
      delete *it;
    }
    qf_vector.clear();
    if(convex_cons) {
      stats_.objConv = 1;
    } else {
      stats_.objConv = 2;
    }
  }
  return all_convex;
}

void SimpleTransformer::refQuadCons_(QuadraticFunctionPtr qf,
                                     LinearFunctionPtr& lf)
{
  VariablePtr v = VariablePtr();
  LinearFunctionPtr lfnew;
  QuadraticFunctionPtr qfnew;
  FunctionPtr fnew;
  ConstraintPtr cnew;
  lf = (LinearFunctionPtr) new LinearFunction();

  for(VariablePairGroupConstIterator it = qf->begin(); it != qf->end(); ++it) {
    // Square of a binary variable is replaced with the variable itself
    if((it->first.first->getIndex() == it->first.second->getIndex()) &&
       (it->first.first->getType() == Binary)) {
      lf->addTerm(it->first.first, it->second);
      continue;
    }
    v = yQfBil_->findY(it->first.first, it->first.second);
    if(!v) {
      v = newp_->newVariable();
      ++stats_.nvars;
      lfnew = (LinearFunctionPtr) new LinearFunction();
      lfnew->addTerm(v, -1.0);
      qfnew = (QuadraticFunctionPtr) new QuadraticFunction();
      qfnew->addTerm(it->first.first, it->first.second, 1.0);
      fnew = (FunctionPtr) new Function(lfnew, qfnew);
      cnew = newp_->newConstraint(fnew, 0.0, 0.0);
      ++stats_.ncons;
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "added new constraint" << std::endl;
      cnew->write(logger_->msgStream(LogDebug));
#endif
      qHandler_->addConstraint(cnew);
      yQfBil_->insert(v, it->first.first, it->first.second);
    }
    lf->addTerm(v, it->second);

    // Linearizing the product of bilinear terms
    if((it->first.first->getType() == Binary) &&
       (it->first.second->getType() == Binary)) {
      // changing the bounds of the new variable
      newp_->changeBound(v, 0.0, 1.0);

      // y<=x1
      lfnew = (LinearFunctionPtr) new LinearFunction();
      lfnew->addTerm(v, 1.0);
      lfnew->addTerm(it->first.first, -1.0);
      fnew = (FunctionPtr) new Function(lfnew);
      cnew = newp_->newConstraint(fnew, -INFINITY, 0.0);
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "added new constraint" << std::endl;
      cnew->write(logger_->msgStream(LogDebug));
#endif
      lHandler_->addConstraint(cnew);

      // y<=x2
      lfnew = (LinearFunctionPtr) new LinearFunction();
      lfnew->addTerm(v, 1.0);
      lfnew->addTerm(it->first.second, -1.0);
      fnew = (FunctionPtr) new Function(lfnew);
      cnew = newp_->newConstraint(fnew, -INFINITY, 0.0);
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "added new constraint" << std::endl;
      cnew->write(logger_->msgStream(LogDebug));
#endif
      lHandler_->addConstraint(cnew);

      // y>=x1+x2-1
      lfnew = (LinearFunctionPtr) new LinearFunction();
      lfnew->addTerm(v, -1.0);
      lfnew->addTerm(it->first.first, 1.0);
      lfnew->addTerm(it->first.second, 1.0);
      fnew = (FunctionPtr) new Function(lfnew);
      cnew = newp_->newConstraint(fnew, -INFINITY, 1.0);
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "added new constraint" << std::endl;
      cnew->write(logger_->msgStream(LogDebug));
#endif
      lHandler_->addConstraint(cnew);
    }
  }
}

void SimpleTransformer::reformulate(ProblemPtr& newp, HandlerVector& handlers,
                                    int& status)
{
  assert(p_);

  double stime = env_->getTimer()->query();

  newp_ = (ProblemPtr) new Problem(env_);
  yLfs_ = new YEqLFs(2 * p_->getNumVars());
  yUniExprs_ = new YEqUCGs();
  yBiVars_ = new YEqCGs();
  yVars_ = new YEqVars(p_->getNumVars() + 40);
  yQfBil_ = new YEqQfBil();
  copyVars_(p_, newp_);

  // create handlers.
  if(p_->getSize()->bins > 0 || p_->getSize()->ints > 0) {
    IntVarHandlerPtr ihandler =
        (IntVarHandlerPtr) new IntVarHandler(env_, newp_);
    handlers.push_back(ihandler);
  }
  lHandler_ = (LinearHandlerPtr) new LinearHandler(env_, newp_);
  lHandler_->setPreOptPurgeVars(false);
  lHandler_->setPreOptPurgeCons(false);
  lHandler_->setPreOptDualFix(false);
  lHandler_->setModFlags(true, true);
  handlers.push_back(lHandler_);
  qHandler_ = (QuadHandlerPtr) new QuadHandler(env_, newp_, p_);
  qHandler_->setModFlags(true, true);
  if(bte_) {
    qHandler_->setBTEngine(bte_);
  }
  if(cute_) {
    qHandler_->setCutEngine(cute_);
  }
  if(nlpe_) {
    qHandler_->setNLPEngine(nlpe_);
  }
  handlers.push_back(qHandler_);
  uHandler_ = (CxUnivarHandlerPtr) new CxUnivarHandler(env_, newp_);
  handlers.push_back(uHandler_);
  kHandler_ = (kPowHandlerPtr) new kPowHandler(env_, newp_, p_);
  handlers.push_back(kHandler_);
  copyLinear_(p_, newp_);

  if(checkQuadConvexity_()) {
    status = 2; // status 2 means the problem is convex
    clearUnusedHandlers_(handlers);
    writeStats(logger_->msgStream(LogExtraInfo));
    delete newp_;
    return;
  }
  refNonlinCons_(p_);
  refNonlinObj_(p_);
  newp_->calculateSize();
  // newp_->write(std::cout);
#if DEBUG
  assert(0 == newp_->checkConVars());
#endif

  if(!(allConsAssigned_(newp_, handlers))) {
    status = 1;
    return;
  }
  clearUnusedHandlers_(handlers);
  status = 0;
  newp = newp_;
  stats_.time = env_->getTimer()->query() - stime;
  writeStats(logger_->msgStream(LogExtraInfo));
}

void SimpleTransformer::resetStats_()
{
  stats_.time = 0.0;
  stats_.nvars = 0;
  stats_.ncons = 0;
  stats_.nconv = 0;
  stats_.objConv = 0;
}

void SimpleTransformer::trigRef_(OpCode op, LinearFunctionPtr lfl,
                                 VariablePtr vl, double dl, VariablePtr& v,
                                 double& d)
{
  if(lfl) {
    vl = newVar_(lfl, dl, newp_);
  } else if(vl && fabs(dl) > zTol_) {
    vl = newVar_(vl, dl, newp_);
  }
  if(vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    CNode* n1 = cg->newNode(vl);
    CNode* n2 = 0;

    n1 = cg->newNode(op, n1, n2);
    cg->setOut(n1);
    cg->finalize();
    v = newVar_(cg, newp_);
  } else {
    d = fabs(dl);
  }
}

void SimpleTransformer::uniVarRef_(const CNode* n0, LinearFunctionPtr lfl,
                                   VariablePtr vl, double dl,
                                   LinearFunctionPtr& lf, VariablePtr& v,
                                   double& d)
{
  CNode *n1, *n2;
  int err = 0;
  if(lfl) {
    vl = newVar_(lfl, dl, newp_);
    dl = 0.0;
  }
  if(vl) {
    if(fabs(dl) > zTol_) {
      vl = newVar_(vl, dl, newp_);
    }
    CGraphPtr cg = (CGraphPtr) new CGraph();
    n1 = cg->newNode(vl);
    n2 = 0;
    n2 = cg->newNode(n0->getOp(), n1, n2);
    cg->setOut(n2);
    cg->finalize();
    v = 0;
    // lf.reset();
    lf = 0;
    v = newVar_(cg, newp_);
    d = 0;
  } else {
    d = n0->evalSingle(dl, &err);
    assert(0 == err);
  }
}

void SimpleTransformer::writeStats(std::ostream& out) const
{
  out << me_
      << "Statistics for transformation by SimpleTransformer:" << std::endl
      << me_
      << "Time taken in reformulation and Convexity detection = " << stats_.time
      << std::endl
      << me_ << "Number of variables added in transformation         = "
      << stats_.nvars << std::endl
      << me_ << "Number of constraints added in transformation       = "
      << stats_.ncons << std::endl
      << me_ << "Number of convex quadratic constraints              = "
      << stats_.nconv << std::endl
      << me_ << "Objective Function = ";
  switch(stats_.objConv) {
  case 0:
    out << "Linear" << std::endl;
    break;
  case 1:
    out << "Convex Quadratic" << std::endl;
    break;
  case 2:
    out << "Nonconvex Quadratic" << std::endl;
    break;
  default:
    break;
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
