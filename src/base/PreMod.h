//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file PreMod.h
 * \brief Declare the PreMod class for saving changes in presolve.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPREMOD_H
#define MINOTAURPREMOD_H

#include "Types.h"

namespace Minotaur {

  /** 
   * PreMod class is an abstract base class. It has a method to transform a
   * solution of the presolved problem in to a solution of a problem in which
   * some of the modifications have been undone.
   */
  class PreMod {
    public:
      /// Constructor.
      PreMod() {};

      /// Destroy.
      virtual ~PreMod() {};

      /// Restore x.
      virtual void postsolveGetX(const DoubleVector &x, DoubleVector *newx) = 0;

  };

}
#endif

