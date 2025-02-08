//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Variable.cpp
 * \brief Define Variables class for storing variables of a problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Variable.h"
#include "Constraint.h"

using namespace Minotaur;

Variable::Variable()
{
  cons_.clear();
}


Variable::Variable(UInt id, UInt index, double lb, double ub,
                   VariableType vtype, std::string name)
  : ftype_(Constant),
    id_(id),
    index_(index),
    lb_(lb),
    name_(name),
    initVal_(0.0),
    state_(NormalVar),
    stype_(VarOrig),
    ub_(ub),
    vtype_(vtype),
    itmp_(0)
{
  cons_.clear();
  if (Binary == vtype_) {
    if (lb_ < 0.0) {
      lb_ = 0.0;
    }
    if (ub_ > 1.0) {
      ub_ = 1.0;
    }
  }
}


VariablePtr Variable::clone(UInt id) const
{
  // id_ is not copied. id is used instead.
  VariablePtr newvar =
      (VariablePtr) new Variable(id, index_, lb_, ub_, vtype_, name_);
  newvar->stype_ = stype_;
  newvar->initVal_ = initVal_;
  return newvar;
}


Variable::~Variable()
{
  cons_.clear();
}


void Variable::clearConstraints_()
{
  cons_.clear();
}


ConstrSet::iterator Variable::consBegin()
{
  return cons_.begin();
}


ConstrSet::iterator Variable::consEnd()
{
  return cons_.end();
}


const std::string Variable::getName() const
{
  return name_;
}


UInt Variable::getNumCons() const
{
  return cons_.size();
}

UInt Variable::getItmp() const
{
  return itmp_;
}

void Variable::inConstraint_(ConstraintPtr cPtr)
{
  cons_.insert(cPtr);
}


void Variable::outOfConstraint_(ConstraintPtr cPtr)
{
  cons_.erase(cPtr);
}

void Variable::setItmp(UInt itmp)
{
  itmp_ = itmp;
}

void Variable::setType_(VariableType vtype)
{
  vtype_ = vtype;
  if (Binary == vtype) {
    if (lb_ < 0.0) {
      lb_ = 0.0;
    }
    if (ub_ > 1.0) {
      ub_ = 1.0;
    }
  }
  return;
}

void Variable::write(std::ostream &out) const
{
  out << "var " << getName() << " ";

  if (lb_ > -INFINITY) {
    out << ">= " << lb_ << " ";
  }
  if (ub_ < INFINITY) {
    out << "<= " << ub_ << " ";
  }
  switch (vtype_) {
  case (Binary):
    out << " binary ";
    break;
  case (Continuous):
    break;
  case (Integer):
    out << " integer ";
    break;
  case (ImplBin):
    out << " binary ";
    break;
  case (ImplInt):
    out << " integer ";
    break;
  default:
    out << " unknown type ";
    break;
  }
  out << ";" << std::endl;
  return;
}


void Variable::writeConstraintMap(std::ostream &out) const
{
  ConstrSet::const_iterator cIter;
  out << std::endl << "in constraint set for variable " << getName();
  out << ", number of constraints = " << cons_.size() << ": " << std::endl;

  for (cIter = cons_.begin(); cIter != cons_.end(); ++cIter) {
    out << (*cIter)->getName();
    out << std::endl;
  }
}
