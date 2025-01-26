//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//


#ifndef MINOTAURNLPRELAXATION_H
#define MINOTAURNLPRELAXATION_H

#include "Relaxation.h"

namespace Minotaur {

  // /**
  // A nonlinear relaxation is of the form:
  //
  // min f(x)
  //
  // s.t.  l_g <= g(x) <= u_g
  //
  //       l_x <=    x <= u_x
  //
  //        x real or integer
  //
  //   where g(x) may have a nonlinear function in one or more constraints.
  // */
  class NLPRelaxation : public Relaxation {
    public:
      // /**
      // Construct an empty Nonlinear Relaxation.
      // */
      NLPRelaxation(EnvPtr env);

      // /**
      // Construct relaxation from a problem. For now, just copy the whole problem.
      // */
      NLPRelaxation(ProblemPtr problem, EnvPtr env);

      // /**
      // Destroy
      // */
      virtual ~NLPRelaxation() { };
      
    protected:
      // /**
      // Pointer to the original problem
      // */
      ConstProblemPtr originalProblem_;
  };

  typedef NLPRelaxation*       NLPRelaxationPtr;
  typedef const NLPRelaxation* ConstNLPRelaxationPtr;
}
#endif

