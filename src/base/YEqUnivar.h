//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
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
