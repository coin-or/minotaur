//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

 /**
  * \file YEqUnivar.h
  * \brief Declare class for storing auxiliary variables equivalent to a univar.
  * \author Mustafa Vora, IEOR, IIT Bombay
  */

#ifndef MINOTAURYEQUNIVAR_H
#define MINOTAURYEQUNIVAR_H

#include "Types.h"

namespace Minotaur {

class YEqUnivar
{
public:
  YEqUnivar();
  VariablePtr findY(VariablePtr v, double a, double b);
  void insert(VariablePtr auxvar, VariablePtr v, double a, double b);

private:
  double randa_;
  double randb_;
  DoubleVector hash_;
  DoubleVector a_;
  DoubleVector b_;
  VarVector v_;
  VarVector y_;

  double evalHash_(UInt key, double a, double b);
};
}

#endif

