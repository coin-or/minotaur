//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file YEqQfBil.cpp
 * \brief Define class for storing auxiliary variables equivalebt to a
 * product of two variables.
 * \author Mustafa Vora, IEOR, IIT Bombay
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"

#include "Variable.h"
#include "YEqQfBil.h"

using namespace Minotaur;

YEqQfBil::YEqQfBil()
{

}

VariablePtr YEqQfBil::findY(VariablePtr v1, VariablePtr v2)
{
  UInt key1 = v1->getId();
  UInt key2 = v2->getId();
  for (UInt i=0; i<v1_.size(); ++i) {
    if (hash1_[i] == key1 && hash2_[i] == key2 &&
         v1 == v1_[i] && v2 == v2_[i]) {
      return y_[i];
    }
  }
  return VariablePtr();
}

void YEqQfBil::insert(VariablePtr auxvar, VariablePtr v1, VariablePtr v2)
{
  v1_.push_back(v1);
  v2_.push_back(v2);
  hash1_.push_back(v1->getId());
  hash2_.push_back(v2->getId());
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
