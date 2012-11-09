//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2012 The MINOTAUR Team.
//

/**
 * \file NonlinearFunction.cpp
 * \brief Define some basic functions for abstract base class NonlinearFunction.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <iostream>
#include <cmath>

#include "MinotaurConfig.h"
#include "NonlinearFunction.h"

using namespace Minotaur;

NonlinearFunction::NonlinearFunction()
  : vars_()
{

}


NonlinearFunction::~NonlinearFunction()
{
  vars_.clear();
}


void NonlinearFunction::computeBounds(Double *lb, Double *ub, Int *error)
{
  *lb = -INFINITY;
  *ub =  INFINITY;
  *error = 0;
}


FunctionType NonlinearFunction::getType() const
{
  return Nonlinear;
}


Bool NonlinearFunction::hasVar(ConstVariablePtr v) const
{
  return (vars_.find(v)!=vars_.end());
}


void NonlinearFunction::write(std::ostream &out) const
{
  out << " nonlinear function ";
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
