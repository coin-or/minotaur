//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
// 

/**
 * \file Constraint.cpp
 * \brief Define the Constraint class.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

#include "MinotaurConfig.h"
#include "Constraint.h"
#include "Function.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"
#include "Variable.h"

using namespace Minotaur;


Constraint::Constraint() 
  : f_(FunctionPtr()),
    lb_(-INFINITY),
    ub_(INFINITY),
    state_(NormalCons),
    id_(0),
    index_(0),
    name_("")
{
}

Constraint::Constraint(UInt id, UInt index, FunctionPtr f, Double lb, Double ub, 
    std::string name)
  :f_(f),
   lb_(lb),
   ub_(ub),
   state_(NormalCons),
   id_(id),
   index_(index),
   name_(name)
{
}

Constraint::~Constraint()
{
  f_.reset();
}


void Constraint::reverseSense_()
{
  Double tmp = lb_;
  (*f_) *= -1;
  lb_    = -1*ub_;
  ub_    = -1*tmp;
}


void Constraint::setName_(std::string name)
{
  name_ = name;
}


Double Constraint::getActivity(const Double *x, Int *error) const
{
  // XXX: report status of evaluation
  return f_->eval(x, error);
}


FunctionType Constraint::getFunctionType() const
{ 
  return f_->getType();
}


const LinearFunctionPtr Constraint::getLinearFunction() const
{ 
  return f_->getLinearFunction();
}


const QuadraticFunctionPtr Constraint::getQuadraticFunction() const 
{ 
  return f_->getQuadraticFunction();
}


const NonlinearFunctionPtr Constraint::getNonlinearFunction() const 
{ 
  return f_->getNonlinearFunction();
}


const std::string Constraint::getName() const 
{
  return name_;
}


void Constraint::write(std::ostream &out) const
{

  out << "subject to " << name_ << ": ";

  if (f_) {
    if (lb_ > -INFINITY) {
      out << lb_ << " <= ";
    } 
    f_->write(out);
    if (ub_ < INFINITY) {
      out << " <= " << ub_;
    }

    if (lb_<=-INFINITY && ub_<=-INFINITY) {
      out << lb_ << " <= ";
      f_->write(out);
      out << " <= " << ub_;
    }
  } else {
    out << "no function in this constraint" << std::endl;
  }
  out << ";" << std::endl;
}


void Constraint::add_(Double cb)
{
  lb_-=cb;
  ub_-=cb;
}


void Constraint::changeLf_(LinearFunctionPtr lf)
{
  f_->changeLf(lf);
}


void Constraint::delFixedVar_(VariablePtr v, Double val)
{
  if (f_ && f_->hasVar(v)) {
    Double offset = f_->getFixVarOffset(v, val);
    lb_ -= offset;
    ub_ -= offset;
    f_->removeVar(v, val);
  }
}


void Constraint::subst_(VariablePtr out, VariablePtr in, Double rat,
                        Bool *instay)
{
  *instay = false;
  if (f_ && f_->hasVar(out)) {
    f_->subst(out, in, rat);
    *instay = f_->hasVar(in);
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
