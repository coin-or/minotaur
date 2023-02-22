// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2021 The MINOTAUR Team.
// 

#include <cmath>
#include <iterator>
#include <iostream>

#include "MinotaurConfig.h"
#include "Function.h"
#include "Variable.h"

using namespace Minotaur;

Function::Function() 
  : lf_(0),
    nlf_(0),
    qf_(0)
{
  type_ = Constant;
  vars_.clear();
}


Function::Function(LinearFunctionPtr lf)
: lf_(lf),
  nlf_(0),
  qf_(0)
{
  if (lf) {
    type_ = Linear;
  } else {
    type_ = Constant;
  }
  collectVars_();
}


Function::Function(LinearFunctionPtr lf, QuadraticFunctionPtr qf)
: lf_(lf),
  nlf_(0),
  qf_(qf)
{
  if (qf) {
    // Check whether quadratic is bilinear or quadratic
    type_ = Bilinear;
    for(VariablePairGroupConstIterator it = qf_->begin(); it != qf_->end(); 
        ++it) {
      if (it->second != 0.0) {        
        if (it->first.first->getId() == it->first.second->getId()) {
          type_ = Quadratic;
          break;
        }
      }
    }
  } else if (lf) {
    type_ = Linear;
  } else {
    type_ = Constant;
  }
  collectVars_();
}


Function::Function(LinearFunctionPtr lf, NonlinearFunctionPtr nlf) 
: lf_(lf),
  nlf_(nlf),
  qf_(0)
{
  if (nlf_) {
    type_ = nlf_->getType();
  } else {
    type_ = Constant;
  }
  collectVars_();
}


Function::Function(LinearFunctionPtr lf, QuadraticFunctionPtr qf,
                   NonlinearFunctionPtr nlf) 
: lf_(lf),
  nlf_(nlf),
  qf_(qf)
{
  type_ = Constant;

  if (lf_) {
    type_ = Linear;
  }

  if (qf_) {
    type_ = Quadratic;
  }

  if (nlf_) {
    type_ = nlf_->getType();
  }

  collectVars_();
}


Function::Function(NonlinearFunctionPtr nlf) 
: lf_(0),
  nlf_(nlf),
  qf_(0)
{
  if (nlf_) {
    type_ = nlf_->getType();
  } else {
    type_ = Constant;
  }
  collectVars_();
}


Function::~Function()
{
  if (lf_) {
    delete lf_;
  }
  if (qf_) {
    delete qf_;
  }
  if (nlf_) {
    delete nlf_;
  }
  vars_.clear();
}


FunctionPtr Function::cloneWithVars(VariableConstIterator vbeg, int *err)
  const
{
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  FunctionPtr f;
  *err = 0;
  if (lf_) {
    lf = lf_->cloneWithVars(vbeg);
  } else {
    lf = 0;
  }
  if (qf_) {
    qf = qf_->cloneWithVars(vbeg);
  } else {
    qf = 0;
  }
  if (nlf_) {
    nlf = nlf_->cloneWithVars(vbeg, err);
  } else {
    nlf = 0;
  }
  f = (FunctionPtr) new Function(lf, qf, nlf);
  f->type_ = type_;

  return f;
}


FunctionPtr Function::cloneWithVarsPermute(VariableConstIterator vbeg, UIntVector variableaddress, int *err)
  const
{ 
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  FunctionPtr f;
  *err = 0;
  if (lf_) {
    lf = lf_->cloneWithVarsPermute(vbeg, variableaddress);
  } else {
    lf = 0;
  }
  if (qf_) {
    qf = qf_->cloneWithVars(vbeg);
  } else {
    qf = 0;
  }
  if (nlf_) {
    nlf = nlf_->cloneWithVars(vbeg, err);
  } else {
    nlf = 0;
  }
  f = (FunctionPtr) new Function(lf, qf, nlf);
  f->type_ = type_;

  return f;
}



void Function::subst(VariablePtr out, VariablePtr in, double rat)
{
  double w;

  if (lf_) {
    w = lf_->getWeight(out);
    lf_->incTerm(in, w*rat);
    lf_->incTerm(out, -w);
    if (0 == lf_->getNumTerms()) {
      delete lf_;
      lf_ = 0;
    }
  }

  if (qf_) {
    qf_->subst(out, in, rat);
    if (0 == qf_->getNumTerms()) {
      delete qf_;
      qf_ = 0;
    }
  }

  if (nlf_) {
    nlf_->subst(out, in, rat);
  }

  vars_.erase(out);
  if ((lf_ && lf_->hasVar(in)) ||
      (qf_ && qf_->hasVar(in)) ||
      (nlf_ && nlf_->hasVar(in))) {
    vars_.insert(in); 
  } else {
    vars_.erase(in); 
  }

  // type may change after substitution
  if (nlf_) {
    type_ = nlf_->getType();
  } else if (qf_) {
    type_ = Quadratic;
  } else if (lf_) {
    type_ = Linear;
  } else {
    type_ = Constant;
  }
}


void Function::operator*=(const double c)
{
  if (fabs(c) < 1e-7) {
    lf_  = LinearFunctionPtr();
    qf_  = QuadraticFunctionPtr();
    nlf_ = NonlinearFunctionPtr();
  } else {
    if (lf_) {
      lf_->multiply(c);
    }
    if (qf_) {
      qf_->multiply(c);
    }
    if (nlf_) {
      nlf_->multiply(c);
    }
  }
}


UInt Function::getNumVars() const
{
  return vars_.size();
}


bool Function::hasVar(VariablePtr var) const
{
  return (vars_.find(var) != vars_.end());
}


UInt Function::getNumNzInHess()
{
  if (nlf_) {
    assert(!"cannot evaluate hessian natively for nonlinear functions!");
  }
  if (qf_) {
    return qf_->getNumTerms();
  }
  return 0;
}


void Function::fillHessOffset(size_t *offset, size_t &pos,
    std::set<ConstVariablePair, CompareVariablePair> & v_pairs)
{
  if (nlf_) {
    assert(!"cannot evaluate hessian natively for nonlinear functions!");
  }
  if (qf_) {
    std::set<ConstVariablePair, CompareVariablePair>::iterator vp_iter;
    std::set<ConstVariablePair, CompareVariablePair>::iterator 
      vp_begin=v_pairs.begin();
    for (VariablePairGroupConstIterator iter=qf_->begin(); iter!=qf_->end(); 
        ++iter, ++pos) {
      vp_iter = v_pairs.find(iter->first);
      offset[pos] = std::distance(vp_begin,vp_iter); // order is important
    }
  }
}


void Function::evalHessian(double mult, const double *, 
                           const size_t *offset, double *values, int *error)
{
  *error = 0;
  if (nlf_) {
    assert(!"cannot evaluate hessian natively for nonlinear functions!");
  }
  if (qf_) {
    const VariablePair *vp;
    const size_t *o = offset;
    for (VariablePairGroupConstIterator iter=qf_->begin(); iter!=qf_->end(); 
        ++iter, ++o) {
      vp = &(iter->first);
      if (vp->first == vp->second) {
        values[*o] += 2.0*mult*iter->second;
      } else {
        values[*o] += mult*iter->second;
      }
    }
  }
}


void Function::evalHessian(const double mult, const double *x, 
                           const LTHessStor *stor, double *values , int *error)
{
  *error = 0;
  if (qf_) {
    qf_->evalHessian(mult, x, stor, values, error);
  }
  if (nlf_) {
    nlf_->evalHessian(mult, x, stor, values, error);
  }
}


void Function::add(ConstLinearFunctionPtr lPtr)
{
  if (lf_) {
    lf_->add(lPtr);
  } else {
    lf_ = lPtr->clone();
    if (type_== Constant) {
      type_ = Linear;
    }
  }
  collectVars_();
}


LinearFunctionPtr Function::removeLinear()
{
  LinearFunctionPtr lf = lf_;
  lf_ = LinearFunctionPtr();

  // change the type of the function.
  if (nlf_) {
    // TODO: Fix this. nlf_ may be a constant expression.
    // do nothing
  } else if (qf_) {
    type_ = Quadratic;
  } else {
    type_ = Constant;
  }
  collectVars_();
  return lf;
}



QuadraticFunctionPtr Function::removeQuadratic()
{
  QuadraticFunctionPtr qf = qf_;
  qf_ = QuadraticFunctionPtr();

  // change the type of the function.
  if (nlf_) {
    // TODO: Fix this. nlf_ may be a constant expression.
    // do nothing
  } else if (lf_) {
    type_ = Linear;
  } else {
    type_ = Constant;
  }
  collectVars_();
  return qf;
}

NonlinearFunctionPtr Function::removeNonlinear()
{
  NonlinearFunctionPtr nlf = nlf_;
  nlf_ = NonlinearFunctionPtr();

  // change the type of the function.
  if (qf_) {
    type_ = Quadratic;
    // do nothing
  } else if (lf_) {
    type_ = Linear;
  } else {
    type_ = Constant;
  }
  collectVars_();
  return nlf;
}


double Function::eval(const DoubleVector &x, int *error) const
{
  return eval(&(x[0]), error);
}


double Function::eval(const double *x, int *error) const
{
  double val = 0.0;
  *error = 0;
  if (lf_) {
    val += lf_->eval(x);
  }
  if (qf_) {
    val += qf_->eval(x);
  }
  if (nlf_) {
    val += nlf_->eval(x, error);
  }
  return val;
}


void Function::evalGradient(const double *x, double *grad_f, int *error) const
{
  *error = 0;
  if (lf_) {
    lf_->evalGradient(grad_f);
  }
  if (qf_) {
    qf_->evalGradient(x, grad_f);
  }
  if (nlf_) {
    nlf_->evalGradient(x, grad_f, error);
  }

}


void Function::prepJac() 
{
  if (lf_) {
    lf_->prepJac(vars_.begin(), vars_.end());
  }
  if (qf_) {
    qf_->prepJac(vars_.begin(), vars_.end());
  }
  if (nlf_) {
    nlf_->prepJac(vars_.begin(), vars_.end());
  }
}


void Function::fillHessStor(LTHessStor *stor)
{
  if (qf_) {
    qf_->fillHessStor(stor);
  }
  if (nlf_) {
    nlf_->fillHessStor(stor);
  }
}


void Function::finalHessStor(const LTHessStor *stor)
{
  if (qf_) {
    qf_->finalHessStor(stor);
  }
  if (nlf_) {
    nlf_->finalHessStor(stor);
  }
}


void Function::fillJac(const double *x, double *values, int *error) 
{
  *error = 0;
  if (lf_) {
    lf_->fillJac(values, error);
  }
  if (qf_) {
    qf_->fillJac(x, values, error);
  }
  if (nlf_) {
    nlf_->fillJac(x, values, error);
  }
}


FunctionType Function::getType()
{
  return type_;
}


void Function::changeLf(LinearFunctionPtr lf)
{
  if (lf_) {
    delete lf_;
  }
  lf_ = lf;
  collectVars_();
}


void Function::changeNlf(NonlinearFunctionPtr nlf)
{
  if (nlf_) {
    delete nlf_;
  }
  nlf_ = nlf;
  collectVars_();
}


void Function::collectVars_()
{
  vars_.clear();
  if (lf_) {
    lf_->getVars(&vars_);
  }
  if (qf_) {
    qf_->getVars(&vars_);
  } 
  if (nlf_) {
    nlf_->getVars(&vars_);
  }
}


LinearFunctionPtr Function::getLinearFunction() const 
{
  return lf_;
}


QuadraticFunctionPtr Function::getQuadraticFunction() const 
{
  return qf_;
}


NonlinearFunctionPtr Function::getNonlinearFunction() const 
{
  return nlf_;
}


bool Function::isLinearIn(ConstVariablePtr v)
{
  if (nlf_ && nlf_->hasVar(v)) {
    return false;
  }
  if (qf_ && qf_->hasVar(v)) {
    return false;
  }
  return true;
}


FunctionType Function::getVarFunType(ConstVariablePtr v)
{
  if (nlf_ && nlf_->hasVar(v)) {
    return Nonlinear;
  } else if (qf_ && qf_->hasVar(v)) {
    return Quadratic;
  } else if (lf_ && lf_->hasVar(v)) {
    return Linear;
  }
  return Constant;
}


VarSetConstIterator Function::varsBegin()
{
  return vars_.begin();
}


VarSetConstIterator Function::varsEnd()
{
  return vars_.end();
}


void Function::removeVar(VariablePtr v, double val)
{
  if (lf_) {
    lf_->removeVar(v, val);
  }
  if (qf_) {
    if (!lf_) {
      lf_ = (LinearFunctionPtr) new LinearFunction();
    }
    qf_->removeVar(v, val, lf_);
    if (lf_->getNumTerms() < 1) {
      delete lf_;
      lf_ = 0;
    }
  }
  if (nlf_) {
    nlf_->removeVar(v, val);
  }
  vars_.erase(v);

  // type may change after removal
  type_ = Constant;
  if (nlf_) {
    type_ = nlf_->getType();
  }
  if (qf_) {
    if (qf_->getNumTerms() == 0) {
      delete qf_;
      qf_ = 0;
    } else {
      type_ = funcTypesAdd(type_, Quadratic);
    }
  } 
  if (lf_) {
    if (lf_->getNumTerms() == 0) {
      delete lf_;
      lf_ = 0;
    } else {
      type_ = funcTypesAdd(type_, Linear);
    }
  }
}


double Function::getFixVarOffset(VariablePtr v, double val)
{
  double offset = 0.;
  if (lf_) {
    offset += lf_->getFixVarOffset(v, val);
  }
  if (qf_) {
    offset += qf_->getFixVarOffset(v, val);
  }
  if (nlf_) {
    offset += nlf_->getFixVarOffset(v, val);
  }
  return offset;
}



void Function::write(std::ostream &out) const
{
  if (lf_) {
    lf_->write(out);
  }
  if (qf_) {
    qf_->write(out);
  }
  if (nlf_) {
    out << " + ";
    nlf_->write(out);
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
