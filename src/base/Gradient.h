// 
//      Minotaur -- It's only 1/2 bull
// 
//      (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file Gradient.h
 * \brief Get information about gradient of a function.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURGRADIENT_H
#define MINOTAURGRADIENT_H

#include "linalg/Vector.h"
#include "Variable.h"

namespace Minotaur {

  class Gradient {
    public:
      Gradient() {};

      // only the following coordinates have non-zero components in the
      // gradient.
      //virtual VariableIterator varsBegin() { return vars_.begin(); }
      //virtual VariableIterator varsEnd() { return vars_.end(); }

      // initialize
      virtual void initialize() {};

      // get no. of nonzeros in the gradient vector
      virtual UInt getNumNz() const {};

      // evaluate gradient at x
      virtual LinearAlgebra::ConstDoubleVectorPtr 
        eval(const std::vector<double> &x) const { assert(!"implement me!"); }

      // evaluate gradient at x
      virtual LinearAlgebra::ConstDoubleVectorPtr 
        eval (LinearAlgebra::ConstDoubleVectorPtr x) { assert(!"implement me!"); }
      //virtual bool isZero() const { return true; }

    protected:
      // LinearGradientPtr lgrad_;
      // QuadraticGradientPtr qgrad_;
  };

  typedef Gradient* GradientPtr;
  typedef const Gradient* ConstGradientPtr;  
}
#endif

