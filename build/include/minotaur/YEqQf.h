//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

 /**
  * \file YEqVars.h
  * \brief Declare class for storing auxiliary variables equivalent to a
  * product of two variables.
  * \author Mustafa Vora, IEOR, IIT Bombay
  */

#ifndef MINOTAURYEQQFBIL_H
#define MINOTAURYEQQFBIL_H

#include "Types.h"
#include "QuadraticFunction.h"

namespace Minotaur {

class YEqQf
{
public:
  YEqQf();
  VariablePtr findY(QuadraticFunctionPtr qf);
  void insert(VariablePtr auxvar, QuadraticFunctionPtr);

private:
  DoubleVector hash_;
  double randVar1_;
  double randVar2_;
  double randCoeff_;
  QfVector qf_;
  VarVector y_;
  double evalHash_(QuadraticFunctionPtr qf);
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

