//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

 /**
  * \file YEqBivar.h
  * \brief Declare class for storing auxiliary variables equivalent to a bivar.
  * \author Mustafa Vora, IEOR, IIT Bombay
  */

#ifndef MINOTAURYEQBIVAR_H
#define MINOTAURYEQBIVAR_H

#include "Types.h"

namespace Minotaur {

class YEqBivar
{
public:
  YEqBivar();
  VariablePtr findY(VariablePtr v1, VariablePtr v2);
  void insert(VariablePtr auxvar, VariablePtr v1, VariablePtr v2);

private:
  UIntVector hash1_;
  UIntVector hash2_;
  std::vector<VariablePtr> v1_;
  std::vector<VariablePtr> v2_;
  VarVector y_;
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
