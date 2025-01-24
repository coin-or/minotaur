//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqVars.h
 * \brief Declare class for storing auxiliary variables equivalent to a sum of
 * a variable and a constant.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURYEQVARS_H
#define MINOTAURYEQVARS_H

#include "Types.h"
#include "OpCode.h"

namespace Minotaur {
class CNode;

class YEqVars
{
public:
  YEqVars(UInt n);
  VariablePtr findY(VariablePtr x, double k);
  void insert(VariablePtr auxvar, VariablePtr x, double k);

private:
  DoubleVector k_;
  UIntVector hash_;
  UInt n_;
  std::vector<VariablePtr> x_;
  VarVector y_;
};
}



#endif

