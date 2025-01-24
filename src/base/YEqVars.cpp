//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqVars.cpp
 * \brief Define class for storing auxiliary variables equivalent to a sum of
 * a variable and a constant.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"

#include "LinearFunction.h"
#include "Objective.h"
#include "Variable.h"
#include "YEqVars.h"

// #define SPEW 1

using namespace Minotaur;


YEqVars::YEqVars(UInt n)
{
  n_ = n;
}


VariablePtr YEqVars::findY(VariablePtr x, double k)
{
  UInt key = x->getId();
  for (UInt i=0; i<x_.size(); ++i){
    if (hash_[i] == key && fabs(k-k_[i])<1e-12 && x==x_[i]) {
      return y_[i];
    }
  }
  return VariablePtr();
}


void YEqVars::insert(VariablePtr auxvar, VariablePtr x, double k)
{
  UInt ind = x->getId()%n_;
  k_.push_back(k);
  x_.push_back(x);
  y_.push_back(auxvar);
  hash_.push_back(ind);
}


