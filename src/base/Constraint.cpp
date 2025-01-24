//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file Constraint.cpp
 * \brief Define the Constraint class.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "Constraint.h"
#include "MinotaurConfig.h"

using namespace Minotaur;

Constraint::Constraint()
  : f_(FunctionPtr()),
    numAct_(0),
    id_(0),
    index_(0),
    lb_(-INFINITY),
    name_(""),
    src_(ConsOrig),
    state_(NormalCons),
    type_(General),
    ub_(INFINITY),
    convex_(Unknown)
{ }

Constraint::Constraint(UInt id, UInt index, FunctionPtr f, double lb, double ub,
                       std::string name)
  : f_(f),
    numAct_(0),
    id_(id),
    index_(index),
    lb_(lb),
    name_(name),
    src_(ConsOrig),
    state_(NormalCons),
    type_(General),
    ub_(ub),
    convex_(Unknown)
{ }

Constraint::~Constraint()
{
  if(f_) {
    delete f_;
  }
}

void Constraint::add_(double cb)
{
  lb_ -= cb;
  ub_ -= cb;
}

void Constraint::changeLf_(LinearFunctionPtr lf)
{
  f_->changeLf(lf);
}

void Constraint::changeNlf_(NonlinearFunctionPtr nlf)
{
  f_->changeNlf(nlf);
}

void Constraint::delFixedVar_(VariablePtr v, double val)
{
  if(f_ && f_->hasVar(v)) {
    double offset = f_->getFixVarOffset(v, val);
    lb_ -= offset;
    ub_ -= offset;
    f_->removeVar(v, val);
    //    if (f_->getNumVars() == 0 &&
    //        !(f_->getLinearFunction() ||
    //          f_->getQuadraticFunction() ||
    //          f_->getNonlinearFunction())) {
    //      setState_(DeletedCons);
    //    }
  }
}

double Constraint::getActivity(const double* x, int* error) const
{
  return f_->eval(x, error);
}

FunctionType Constraint::getFunctionType() const
{
  return f_->getType();
}

LinearFunctionPtr Constraint::getLinearFunction() const
{
  return f_->getLinearFunction();
}

const std::string Constraint::getName() const
{
  return name_;
}

NonlinearFunctionPtr Constraint::getNonlinearFunction() const
{
  return f_->getNonlinearFunction();
}

QuadraticFunctionPtr Constraint::getQuadraticFunction() const
{
  return f_->getQuadraticFunction();
}

void Constraint::reverseSense_()
{
  double tmp = lb_;
  (*f_) *= -1;
  lb_ = -1 * ub_;
  ub_ = -1 * tmp;
}

void Constraint::setName_(std::string name)
{
  name_ = name;
}

void Constraint::subst_(VariablePtr out, VariablePtr in, double rat,
                        bool* instay)
{
  *instay = false;
  if(f_ && f_->hasVar(out)) {
    f_->subst(out, in, rat);
    *instay = f_->hasVar(in);
  }
}

void Constraint::write(std::ostream& out) const
{

  out << "subject to " << name_ << ": ";

  if(f_) {
    if(lb_ > -INFINITY) {
      out << lb_ << " <= ";
    }
    f_->write(out);
    if(ub_ < INFINITY) {
      out << " <= " << ub_;
    }

    if(lb_ <= -INFINITY && ub_ <= -INFINITY) {
      out << lb_ << " <= ";
      f_->write(out);
      out << " <= " << ub_;
    }
  } else {
    out << "no function in this constraint" << std::endl;
  }
  out << ";" << std::endl;
}

