//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file PreDelVars.h
 * \brief Declare the PreDelVars class for saving and restoring variables in
 * presolve.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPREDELVARS_H
#define MINOTAURPREDELVARS_H

#include "PreMod.h"
#include "Variable.h"

namespace Minotaur {

  class PreDelVars : public PreMod {
    public:
      /// Constructor
      PreDelVars();

      /// Destroy
      ~PreDelVars();

      /// Add a new variable to the list.
      void insert(VariablePtr v);

      /// Restore x.
      void postsolveGetX(const DoubleVector &x, DoubleVector *newx);

    private:
      /// A queue of variables deleted.
      VarQueue vars_;

  };

  typedef PreDelVars* PreDelVarsPtr;
  typedef const PreDelVars* ConstPreDelVarsPtr;
}
#endif

