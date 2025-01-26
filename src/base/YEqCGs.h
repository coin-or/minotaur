//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqCGs.h
 * \brief Declare the class for storing auxiliary variables for nonlinear
 * functions that are expressed as computational graphs.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURYEQCG_H
#define MINOTAURYEQCG_H

#include "Types.h"
#include "OpCode.h"

namespace Minotaur {
class CGraph;
typedef CGraph* CGraphPtr;

class YEqCGs {
public:
  YEqCGs();
  VariablePtr findY(CGraphPtr cg);
  void insert(VariablePtr auxvar, CGraphPtr cg);

private:
  DoubleVector hash_;
  DoubleVector rand_;
  VarVector y_;
  std::vector<CGraphPtr> cg_;
  double evalHash_(const CNode* node, UInt rank);
};
}
#endif

