//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
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


void NonlinearFunction::addConst(const double, int &err)
{
  err = 1;
}


void NonlinearFunction::computeBounds(double *lb, double *ub, int *error)
{
  *lb = -INFINITY;
  *ub =  INFINITY;
  *error = 0;
}


std::string NonlinearFunction::getNlString(int *)
{
  return "";
}


FunctionType NonlinearFunction::getType() const
{
  return Nonlinear;
}


bool NonlinearFunction::hasVar(ConstVariablePtr v) const
{
  return (vars_.find(v)!=vars_.end());
}


NonlinearFunctionPtr NonlinearFunction::getPersp(VariablePtr, double,
                                                 int *err) const
{
  assert(!"implement me!");
  *err = 1;
  return NonlinearFunctionPtr();
}


void NonlinearFunction::sqrRoot(int &err)
{
  err = 1; 
}


void NonlinearFunction::write(std::ostream &out) const
{
  out << " nonlinear function ";
}


