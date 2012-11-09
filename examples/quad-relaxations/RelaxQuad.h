//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 -- 2010 The MINOTAUR Team.
//


// /**
// \file RelaxQuad.h
// \brief Define class for creating different relaxations of a quadratic
// optimization problem.
// \author Ashutosh Mahajan, Argonne National Laboratory
//
// Define class for creating different relaxations of a quadratic
// optimization problem.
//
// */

#ifndef MINOTAURRELAXQUAD_H
#define MINOTAURRELAXQUAD_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Types.h"
#include "Exception.h"
#include "Variable.h"
#include "Constraint.h"
#include "Objective.h"
#include "Logger.h"
#include "LPEngine.h"
#include "Option.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "Relaxation.h"


namespace Minotaur {

  class RelaxQuad {

    public:
      typedef enum {
        TERMBYTERM_OA,     // each term individually, convex terms are 
                           // linearized.
        TERMBYTERM_OA_CUTS,// same as above, intersection cuts.
        TERMBYTERM,        // each term individually, convex terms as nonlinear.
        EV_OA_SECANT,      // Eigen Value, convex terms linearized.
        EV_SECANT,         // Eigen Value, convex terms nonlinear.
        EV_OA_CUTS         // Eigen Value, convex terms linearized, 
                           // intersection cuts.
      } RelaxType;

      /// Constructor
      RelaxQuad(ProblemPtr problem, OptionDBPtr options);

      /// Destroy
      ~RelaxQuad();

      /// Create different kinds of relaxations, depending upon the options
      void createRelaxation(RelaxType r_type);

      /// return relaxation
      ProblemPtr getRelaxation() { return rel_; };

      void solveUsingOA(LPEnginePtr lp_e);
      void solveUsingOA_Cuts(LPEnginePtr lp_e);
    private:
      /// Original Problem
      ProblemPtr problem_;

      RelaxationPtr rel_;

      /// Options
      OptionDBPtr options_;

      /// tolerance
      Double etol_;

      /// First term is the variable x, second is variable w. The constraint
      /// that is being linearized is
      /// x^2 - w <= 0.
      std::vector< VariablePair > sq_vars_;

      /// -x^2 + w <= 0.
      std::vector< VariablePair > neg_sq_vars_;

      /// Term by term relaxation, convex nonlinear
      void termByTerm_(Bool keep_squares=true);

      /// Term by term relaxation, linear
      void termByTermOA_();

      FunctionPtr getSecantConstraint_(VariablePtr x, 
          VariablePtr y, Double &rhs, Bool &is_bound);

      FunctionPtr getOAConstraint_(VariablePtr v1, VariablePtr v2, 
          Double xval, Double wval, Double &rhs);
      FunctionPtr getICut_(VariablePtr v1, VariablePtr v2, Double xval, 
          Double wval, Double &rhs, LPEnginePtr lp_e);
      Double findQuadRoot_(Double a, Double b, Double c, Int &root_status,
          Bool positive_root);
  };
} // namespace
#endif

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
