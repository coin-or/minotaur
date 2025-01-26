//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//


#ifndef MINOTAURLPRELAXATION_H
#define MINOTAURLPRELAXATION_H

#include "Modification.h"
#include "Relaxation.h"

namespace Minotaur {

  // /**
  // LPRelaxation is a relaxation in which all constraints and objectives
  // have only linear functions.
  // */
  class LPRelaxation : public Relaxation {
    public:
      // /**
      // Construct an empty Nonlinear Relaxation.
      // */
      LPRelaxation(EnvPtr env);

      // /** 
      // Construct from a problem.
      // */
      LPRelaxation(ProblemPtr problem, EnvPtr env);

      // /** 
      // Destroy
      // */
      virtual ~LPRelaxation() { };

    protected:
      // /**
      // Pointer to the original problem
      // */
      ConstProblemPtr originalProblem_;
  };

  typedef LPRelaxation*       LPRelaxationPtr;
  typedef const LPRelaxation* ConstLPRelaxationPtr;
}
#endif

