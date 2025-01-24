// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2024 The Minotaur Team.
// 

/**
 * \file LinearCut.cpp
 * \brief Implement the methods of LinearCut class. 
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Function.h"
#include "LinearCut.h"
#include "LinearFunction.h"
#include "Problem.h"

using namespace Minotaur;


LinearCut::LinearCut()
  : cons_(ConstraintPtr()),
    f_(FunctionPtr()),
    lb_(-INFINITY),
    lf_(LinearFunctionPtr()),
    ub_(INFINITY)
{
  info_.timesEnabled  = 0;
  info_.timesDisabled = 0;
  info_.lastEnabled   = 0;
  info_.lastDisabled  = 0;
}


LinearCut::LinearCut(LinearFunctionPtr lf, double lb, double ub)
  : cons_(ConstraintPtr()),
    lb_(lb),
    lf_(lf),
    ub_(ub)
{
  f_ = (FunctionPtr) new Function(lf_);
}


LinearCut::~LinearCut()
{

}


void LinearCut::applyToProblem(ProblemPtr problem) 
{
  cons_ = problem->newConstraint(f_, lb_, ub_, "linear_cut");
}


void LinearCut::undoToProblem(ProblemPtr)
{
}


void LinearCut::write(std::ostream &out) const
{
  out << lb_ << " <= ";
  lf_->write(out);
  out << " <= " << ub_;
}


