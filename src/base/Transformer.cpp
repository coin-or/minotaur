//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file Transformer.cpp
 * \brief Define class for reformulating a problem suitable for handlers.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"

#include "Environment.h"
#include "CGraph.h"
#include "CNode.h"
#include "Constraint.h"
#include "CxQuadHandler.h"
#include "CxUnivarHandler.h"
#include "Function.h"
#include "IntVarHandler.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "NonlinearFunction.h"
#include "Option.h"
#include "Objective.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "QuadraticFunction.h"
#include "Solution.h"
#include "Transformer.h"
#include "Variable.h"
#include "YEqLFs.h"
#include "YEqUCGs.h"
#include "YEqVars.h"

// #define SPEW 1

using namespace Minotaur;
const std::string Transformer::me_ = "Transformer: ";


Transformer::Transformer()
  : env_(EnvPtr()), // NULL
    logger_(LoggerPtr()),
    yLfs_(0),
    yUniExprs_(0),
    zTol_(1e-12)
{
}


Transformer::Transformer(EnvPtr env)
  : env_(env),
    yLfs_(0),
    yUniExprs_(0),
    zTol_(1e-12)
{
  logger_ = (LoggerPtr) new Logger((LogLevel)(env->getOptions()
                                              ->findInt("trans_log_level")
                                              ->getValue()));
}


Transformer::~Transformer() 
{
}


Bool Transformer::allConsAssigned_(ProblemPtr p, HandlerVector &handlers)
{
  BoolVector asgn(p->getNumCons(), false);
  for (HandlerVector::const_iterator it=handlers.begin(); it!=handlers.end();
       ++it) {
    for (ConstraintVector::const_iterator it2=(*it)->consBegin();
         it2!=(*it)->consEnd(); ++it2) {
      if (asgn[(*it2)->getIndex()] == true) {
        std::cout << "constraint " << (*it2)->getName() <<
          " with index " << (*it2)->getIndex() << 
          " is assigned to more than one handlers!" << std::endl;
        (*it2)->write(std::cout);
        return false;
      }
      asgn[(*it2)->getIndex()] = true;
    }
  }
  for (UInt it=0; it<p->getNumCons(); ++it) {
    if (asgn[it] != true) {
      std::cout << "constraint " << p->getConstraint(it)->getName() <<
        " with index " << it << " is not assigned to any handler!" << std::endl;
      p->getConstraint(it)->write(std::cout);
      return false;
    }
  }
  return true;
}


void Transformer::assignHandler_(CGraphPtr cg, ConstraintPtr c)
{
  switch (cg->getOut()->getOp()) {
  case OpMult:
  case OpSqr:
    qHandler_->addConstraint(c);
    break;
  default:
    {
    VariablePtr iv;
    VariablePtr ov = VariablePtr();
    LinearFunctionPtr lf = c->getFunction()->getLinearFunction();
    if (lf) {
      assert(lf->getNumTerms()==1);
      ov = lf->termsBegin()->first;
    }
    iv = *(c->getFunction()->getNonlinearFunction()->varsBegin());
    uHandler_->addConstraint(c, iv, ov, 'E');
    }
  }
}


void Transformer::clearUnusedHandlers_(HandlerVector &handlers)
{
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end();) {
    if (false == (*it)->isNeeded()) {
      it = handlers.erase(it);
    } else {
      ++it;
    }
  }
}


void Transformer::copyVars_(ConstProblemPtr p, ProblemPtr newp)
{
  // first copy all variables from p to newp
  //XXX  (Need to setup map here?)
  newp->newVariables(p->varsBegin(), p->varsEnd());
}


void Transformer::copyLinear_(ConstProblemPtr p, ProblemPtr newp)
{
  FunctionPtr f;
  LinearFunctionPtr newlf;
  ConstConstraintPtr c;
  ConstraintPtr newc;
  ObjectivePtr obj;

  // copy linear constraints.
  for (ConstraintConstIterator it=p->consBegin(); it!=p->consEnd(); ++it) {
    c = *it;
    if (Linear==c->getFunction()->getType()) {
      // create a clone of this linear function.
      newlf = c->getLinearFunction()->cloneWithVars(newp->varsBegin());
      f = (FunctionPtr) new Function(newlf);
      newc = newp->newConstraint(f, c->getLb(), c->getUb());
      lHandler_->addConstraint(newc);
    }
  }

  // copy linear objective.
  obj = p->getObjective();
  switch (obj->getFunctionType()) {
  case (Constant):
    f = FunctionPtr(); // NULL
    newp->newObjective(f, obj->getConstant(), obj->getObjectiveType(),
                       obj->getName());
    break;
  case (Linear):
    newlf = obj->getFunction()->getLinearFunction()->
            cloneWithVars(newp->varsBegin());
    f = (FunctionPtr) new Function(newlf); 
    newp->newObjective(f, obj->getConstant(), obj->getObjectiveType(),
                       obj->getName());
    break;
  default:
    break;
  }
}


void Transformer::makeObjectiveLinear(ProblemPtr p)
{
  ObjectivePtr obj;
  assert(p);

  obj = p->getObjective();
  if (!obj) {
    return;
  }
  
  if (obj->getFunctionType() != Linear && obj->getFunctionType() != Constant) {
    VariablePtr eta = p->newVariable(-INFINITY, INFINITY, Continuous);
    FunctionPtr etaf;
    FunctionPtr f = obj->getFunction();
    LinearFunctionPtr lz = LinearFunctionPtr(new LinearFunction());
    LinearFunctionPtr lz2;
    ObjectiveType otype = obj->getObjectiveType();
    ConstraintPtr objcon;

    lz->addTerm(eta, -1.0);
    f->add(lz);
    
    if (otype == Minimize) {
      //XXX Do you want to keep track of the objective constraint?
      objcon = p->newConstraint(f, -INFINITY, 0.0);
      if (lHandler_) {
        lHandler_->addConstraint(objcon);
      }
    } else {
      objcon = p->newConstraint(f, 0.0, INFINITY);
      if (lHandler_) {
        lHandler_->addConstraint(objcon);
      }
    }

    lz2 = (LinearFunctionPtr) new LinearFunction();
    lz2->addTerm(eta, 1.0);
    etaf = (FunctionPtr) new Function(lz2);

    p->removeObjective();
    p->newObjective(etaf, obj->getConstant(), otype);
  }
}

    
VariablePtr Transformer::newVar_(VariablePtr iv, Double d, ProblemPtr newp)
{
  if (fabs(d)<zTol_) {
    return iv;
  } else {
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    VariablePtr ov;
    FunctionPtr f;
    ConstraintPtr cnew;

    ov = yVars_->findY(iv, d);
    if (!ov) {
      ov = newp->newVariable();
      yVars_->insert(ov, iv, d);
      lf->addTerm(iv, 1.0);
      lf->addTerm(ov, -1.0);
      f = (FunctionPtr) new Function(lf);
      cnew = newp->newConstraint(f, -d, -d);
      lHandler_->addConstraint(cnew);
    }
    return ov;
  }
}


VariablePtr Transformer::newVar_(LinearFunctionPtr lf, Double d,
                                 ProblemPtr newp)
{
  VariablePtr ov;
  FunctionPtr f;
  ConstraintPtr cnew;

  ov = yLfs_->findY(lf, d);
  if (!ov) {
    LinearFunctionPtr lf2 = lf->clone();
    ov = newp->newVariable();
    yLfs_->insert(ov, lf2, d);
    lf->addTerm(ov, -1.0);
    f = (FunctionPtr) new Function(lf);
    cnew = newp->newConstraint(f, -d, -d);

    lHandler_->addConstraint(cnew);
  }
  return ov;
}


VariablePtr Transformer::newVar_(CGraphPtr cg, ProblemPtr newp)
{
  VariablePtr iv;
  VariablePtr ov = VariablePtr(); // NULL
  FunctionPtr f;
  LinearFunctionPtr lf;
  ConstraintPtr cnew;
  assert(cg);

  if (OpSumList!=cg->getOut()->getOp()) {
    ov = yUniExprs_->findY(cg);
  } 

  if (!ov) {
    ov = newp->newVariable();
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(ov, -1.0);
    f = (FunctionPtr) new Function(lf, cg);
    cnew = newp->newConstraint(f, 0.0, 0.0);
    assignHandler_(cg, cnew);

    yUniExprs_->insert(ov, cg);
  }
  return ov;
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
