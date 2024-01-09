
//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file QuadTransformer.cpp
 * \brief Define class for reformulations a quadratic problem to make it suitable
 * for handlers.
 * \author Mustafa Vora, IIT Bombay
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Engine.h"
#include "Environment.h"
#include "Constraint.h"
#include "Function.h"
#include "IntVarHandler.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "Option.h"
#include "Objective.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "QuadHandler.h"
#include "QuadraticFunction.h"
#include "QuadTransformer.h"
#include "Solution.h"
#include "Timer.h"
#include "Variable.h"
#include "UnivarQuadHandler.h"
#include "YEqQfBil.h"
#include "YEqBivar.h"
#include "YEqUnivar.h"

// #define SPEW 1

using namespace Minotaur;
const std::string QuadTransformer::me_ = "QuadTransformer: ";


QuadTransformer::QuadTransformer()
  : Transformer(),
  yQfBil_(0),
  yBivar_(0),
  yUnivar_(0)
{
}


QuadTransformer::QuadTransformer(EnvPtr env, ProblemPtr p)
  : Transformer(env, p),
  yQfBil_(0),
  yBivar_(0),
  yUnivar_(0)
{
}


QuadTransformer::~QuadTransformer() 
{
  delete yBivar_;
  delete yUnivar_;
  delete yQfBil_;
}

std::string QuadTransformer::getName() const
{
  return "QuadTransformer";
}


SolutionPtr QuadTransformer::getSolOrig(ConstSolutionPtr, int &err )
{
  err = 1;
  return SolutionPtr();
}


SolutionPtr QuadTransformer::getSolTrans(ConstSolutionPtr, int &err )
{
  err = 1;
  return SolutionPtr();
}

void QuadTransformer::refCons_(ConstProblemPtr oldp)
{
  ConstraintPtr c, cnew;
  FunctionPtr f, f2;
  LinearFunctionPtr lf = 0, lf2;
  QuadraticFunctionPtr qf, qf2;

  assert (oldp && newp_);

  for (ConstraintConstIterator it=oldp->consBegin(); it!=oldp->consEnd();
       ++it) {
    c = *it;
    f = c->getFunction();
    lf = f->getLinearFunction();
    if (lf) {
      lf2 = lf->cloneWithVars(newp_->varsBegin());
    } else {
      lf2 = (LinearFunctionPtr) new LinearFunction();
    }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "reformulating the constraint"
                                 << std::endl;
    c->write(logger_->msgStream(LogDebug));
#endif
    qf = f->getQuadraticFunction();
    if (qf) {
      qf2 = qf->cloneWithVars(newp_->varsBegin());
      refQuadCons_(qf2, lf2, lf);
      f2 = (FunctionPtr) new Function(lf);
      cnew = newp_->newConstraint(f2, c->getLb(), c->getUb());
      lHandler_->addConstraint(cnew);
      delete qf2;
      delete lf2;
    } else {
      if (f->getNonlinearFunction()) {
        assert(!"QuadTransformer handles only MIQCQPs.");
      }
    }
  }
}

void QuadTransformer::refObj_(ConstProblemPtr oldp) 
{
  ObjectivePtr obj = oldp->getObjective();
  FunctionPtr f, f2;
  LinearFunctionPtr lf = 0, lf2;
  QuadraticFunctionPtr qf, qf2;
  double d;

  assert (oldp && newp_);

  if (!obj) {
    // already dealt with this case in copyLinear_()
    return;
  }

  f = obj->getFunction();
  if (!f) {
    // already dealt with this case in copyLinear_()
    return;
  }

  lf = f->getLinearFunction();
  if (lf) {
    lf2 = lf->cloneWithVars(newp_->varsBegin());
  } else {
    lf2 = (LinearFunctionPtr) new LinearFunction();
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "reformulating the objective"
                               << std::endl;
  obj->write(logger_->msgStream(LogDebug));
#endif
  d = obj->getConstant();
  qf = f->getQuadraticFunction();
  if (qf) {
    qf2 = qf->cloneWithVars(newp_->varsBegin());
    refQuadCons_(qf2, lf2, lf);
    f2 = (FunctionPtr) new Function(lf);
    obj = newp_->newObjective(f2, d, Minimize);
    delete qf2;
    delete lf2;
  } else {
    if (f->getNonlinearFunction()) {
      assert(!"QuadTransformer handles only MIQCQPs.");
    }
  }
}

void QuadTransformer::refQuadCons_(QuadraticFunctionPtr qf,
                LinearFunctionPtr lf, LinearFunctionPtr &lf1) {
  VariablePtr v = VariablePtr();
  LinearFunctionPtr lfnew;
  QuadraticFunctionPtr qf2, qfnew;
  FunctionPtr fnew;
  ConstraintPtr cnew;
  double a, b, etol = 1e-8;
  
  lf1 = (LinearFunctionPtr) new LinearFunction();
  qf2 = (QuadraticFunctionPtr) new QuadraticFunction();

  for (VariablePairGroupConstIterator it = qf->begin();
       it != qf->end(); ++it) {
    // add square terms to qf2
    if (it->first.first->getIndex() == it->first.second->getIndex()) {
      qf2->addTerm(it->first, it->second);
    }
  }

  for (VariablePairGroupConstIterator it = qf->begin();
       it != qf->end(); ++it) {
    // check if bilinear
    if (it->first.first->getIndex() != it->first.second->getIndex()) {
      // reformulate as 1/2*(x1 + x2)^2 - 1/2*x1^2 - 1/2*x2^2
      v = yBivar_->findY(it->first.first, it->first.second);
      if (!v) {
        v = newp_->newVariable();
        lfnew = (LinearFunctionPtr) new LinearFunction();
        lfnew->addTerm(v, -1.0);
        qfnew = (QuadraticFunctionPtr) new QuadraticFunction();
        qfnew->addTerm(it->first.first, it->first.first, 1.0);
        qfnew->addTerm(it->first.second, it->first.second, 1.0);
        qfnew->addTerm(it->first.first, it->first.second,2.0);
        fnew = (FunctionPtr) new Function(lfnew, qfnew);
        cnew = newp_->newConstraint(fnew, 0.0, 0.0);
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "added new constraint"
                                     << std::endl;
        cnew->write(logger_->msgStream(LogDebug));
#endif
        uqHandler_->addConstraint(cnew);
        yBivar_->insert(v, it->first.first, it->first.second);
      }
      lf1->addTerm(v, it->second/2.0);
      qf2->incTerm(it->first.first, it->first.first, -it->second/2.0);
      qf2->incTerm(it->first.second, it->first.second, -it->second/2.0);
    }
  }
  
  for (VariablePairGroupConstIterator it = qf2->begin();
       it != qf2->end(); ++it) {
    assert(it->first.first->getIndex() == it->first.second->getIndex());
    if (fabs(it->second) > etol) {
      if (lf->hasVar(it->first.first)) {
        a = it->second > etol ? it->second : -it->second;
        b = it->second > etol ? lf->getWeight(it->first.first)
                              : -lf->getWeight(it->first.first); 
        v = yUnivar_->findY(it->first.first, a, b);
        if (!v) {
          v = newp_->newVariable();
          lfnew = (LinearFunctionPtr) new LinearFunction();
          lfnew->addTerm(v, -1.0);
          lfnew->addTerm(it->first.first, b);
          qfnew = (QuadraticFunctionPtr) new QuadraticFunction();
          qfnew->addTerm(it->first.first, it->first.first, a);
          fnew = (FunctionPtr) new Function(lfnew, qfnew);
          cnew = newp_->newConstraint(fnew, 0.0, 0.0);
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "added new constraint"
                                       << std::endl;
          cnew->write(logger_->msgStream(LogDebug));
#endif
          uqHandler_->addConstraint(cnew);
          yUnivar_->insert(v, it->first.first, a, b);
        }
        lf1->addTerm(v, it->second > etol ? 1.0 : -1.0);
        lf->incTerm(it->first.first, -lf->getWeight(it->first.first));
      } else {
        v = yQfBil_->findY(it->first.first, it->first.first);
        if (!v) {
          v = newp_->newVariable();
          lfnew = (LinearFunctionPtr) new LinearFunction();
          lfnew->addTerm(v, -1.0);
          qfnew = (QuadraticFunctionPtr) new QuadraticFunction();
          qfnew->addTerm(it->first.first, it->first.first, 1.0);
          fnew = (FunctionPtr) new Function(lfnew, qfnew);
          cnew = newp_->newConstraint(fnew, 0.0, 0.0);
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "added new constraint"
                                       << std::endl;
          cnew->write(logger_->msgStream(LogDebug));
#endif
          qHandler_->addConstraint(cnew);
          yQfBil_->insert(v, it->first.first, it->first.second);
        }
        lf1->addTerm(v, it->second);
      }
    }
  }
  lf1->add(lf);
}

void QuadTransformer::reformulate(ProblemPtr &newp, HandlerVector &handlers,
                                  int &status)
{
  assert(p_);

  newp_ = (ProblemPtr) new Problem(env_);
  yQfBil_ = new YEqQfBil();
  yBivar_ = new YEqBivar();
  yUnivar_ = new YEqUnivar();
  copyVars_(p_, newp_);

  // create handlers.
  if (p_->getSize()->bins > 0 || p_->getSize()->ints > 0) {
    IntVarHandlerPtr ihandler = (IntVarHandlerPtr)
                                new IntVarHandler(env_, newp_);
    handlers.push_back(ihandler);
  }
  lHandler_ = (LinearHandlerPtr) new LinearHandler(env_, newp_);
  lHandler_->setPreOptPurgeVars(false);
  lHandler_->setPreOptPurgeCons(false);
  lHandler_->setPreOptDualFix(false);
  lHandler_->setModFlags(true, true);
  handlers.push_back(lHandler_);
  qHandler_ = (QuadHandlerPtr) new QuadHandler(env_, newp_);
  qHandler_->setModFlags(true, true);
  //qHandler_->setEngine(engine);
  handlers.push_back(qHandler_);
  uqHandler_ = (UnivarQuadHandlerPtr) new UnivarQuadHandler(env_, newp_);
  uqHandler_->setModFlags(true, true);
  handlers.push_back(uqHandler_);

  copyLinear_(p_, newp_);
  refCons_(p_);
  refObj_(p_);
  newp_->calculateSize();

#if DEBUG
  assert(0==newp_->checkConVars());
#endif 

  if (!(allConsAssigned_(newp_, handlers))) {
    status = 1;
    return;
  }
  clearUnusedHandlers_(handlers);
  status = 0;
  newp = newp_;
  // newp->write(std::cout);
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
