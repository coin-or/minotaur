//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file LinConMod.cpp
 * \brief Implement the Modification class LinConMod, that is used to store
 * modifications to the linear parts and rhs of a constraint.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <iostream>
#include <cmath>

#include "MinotaurConfig.h"
#include "Constraint.h"
#include "Engine.h"
#include "LinConMod.h"
#include "LinearFunction.h"
#include "Problem.h"
#include "Variable.h"

using namespace Minotaur;


LinConMod::LinConMod(ConstraintPtr con, LinearFunctionPtr new_lf,
                     double new_lb, double new_ub)
  : con_(con),       // NULL
    newLf_(new_lf),   // NULL
    newLb_(new_lb),
    newUb_(new_ub)
{
  oldLf_ = con->getLinearFunction()->clone();
  oldUb_ = con->getUb();
  oldLb_ = con->getLb();
}


LinConMod::~LinConMod()
{
  //con_.reset(); //changed: need to delete using different methods
  //newLf_.reset();
  //oldLf_.reset();
  if (oldLf_) {
    delete oldLf_; oldLf_ = 0;
  }
  if (newLf_) {
    delete newLf_; newLf_ = 0;
  }
}


void LinConMod::applyToProblem(ProblemPtr problem) 
{
  LinearFunctionPtr lf = newLf_->clone();
  problem->changeConstraint(con_, lf, newLb_, newUb_);
}


void LinConMod::undoToProblem(ProblemPtr problem) 
{
  LinearFunctionPtr lf = oldLf_->clone();
  problem->changeConstraint(con_, lf, oldLb_, oldUb_);
}


void LinConMod::write(std::ostream &out) const
{
  out << "LinConMod: old constraint: ";
  out << oldLb_ << " <= " ;
  oldLf_->write(out);
  out << " <= " << oldUb_ << std::endl;
  out << "LinConMod: new constraint: ";
  out << newLb_ << " <= " ;
  newLf_->write(out);
  out << " <= " << newUb_ << std::endl;
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
