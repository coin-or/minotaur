//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
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

