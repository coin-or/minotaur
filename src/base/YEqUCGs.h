//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqUCGs.h
 * \brief Declare the class for storing auxiliary variables for univariate
 * nonlinear functions that are expressed as computational graphs.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURYEQUCGS_H
#define MINOTAURYEQUCGS_H

#include "Types.h"
#include "OpCode.h"

namespace Minotaur {
class CGraph;
class CNode;
typedef CGraph* CGraphPtr;

class YEqUCGs {
public:
  YEqUCGs();
  ~YEqUCGs();
  VariablePtr findY(CGraphPtr cg);
  void insert(VariablePtr auxvar, CGraphPtr cg);

private:
  std::vector<CGraphPtr> cg_;
  DoubleVector hash_;
  std::vector<OpCode> op_;
  DoubleVector rand_;
  VarVector x_;
  VarVector y_;
  double evalHash_(const CNode* node, UInt rank);
};
}
#endif

