//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file PreAuxVars.h
 * \brief Declare the PreAuxVars class for saving additional variables
 * variables added during presolve.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPREAUXVARS_H
#define MINOTAURPREAUXVARS_H

#include "PreMod.h"
#include "Variable.h"

namespace Minotaur {

class PreAuxVars : public PreMod {
public:
  /// Constructor.
  PreAuxVars();

  /// Destroy.
  ~PreAuxVars();

  /// New variable that was created.
  void insert(VariablePtr v);

  /// Remove aux-vars from the solution x.
  void postsolveGetX(const DoubleVector &x, DoubleVector *newx);

  /// Return the number of additions.
  UInt getSize();

private:
  std::deque<VariablePtr> vars_;

};

typedef PreAuxVars* PreAuxVarsPtr;
}
#endif

