//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file YEqUnivar.cpp
 * \brief Define class for storing auxiliary variables equivalent to a bivar.
 * \author Mustafa Vora, IEOR, IIT Bombay
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"

#include "Variable.h"
#include "YEqUnivar.h"

using namespace Minotaur;

YEqUnivar::YEqUnivar()
{
  randa_ = rand()/(RAND_MAX);
  randb_ = rand()/(RAND_MAX);
}

double YEqUnivar::evalHash_(UInt key, double a, double b) {
  return key + randa_*a + randb_*b;
}

VariablePtr YEqUnivar::findY(VariablePtr v, double a, double b)
{
  UInt key = v->getId();
  double etol = 1e-8;
  for (UInt i=0; i<v_.size(); ++i) {
    if (hash_[i] == evalHash_(key, a, b) && v == v_[i]
        && fabs(a - a_[i]) < etol && fabs(b - b_[i]) < etol) {
      return y_[i];
    }
  }
  return VariablePtr();
}

void YEqUnivar::insert(VariablePtr auxvar, VariablePtr v, double a, double b)
{
  v_.push_back(v);
  hash_.push_back(evalHash_(v->getId(), a, b));
  a_.push_back(a);
  b_.push_back(b);
  y_.push_back(auxvar);
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
//
