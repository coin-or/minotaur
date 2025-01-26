//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file LinearCut.h
 * \brief Define the class of valid inequalities of the form \f$ ax \leq b\f$
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURLINEARCUT_H
#define MINOTAURLINEARCUT_H

#include "Cut.h"
#include "Types.h"

namespace Minotaur {

  /**
   * A linear cut is a valid inequality of the form \f$ l \leq ax \leq u \f$,
   * where \f$a \in \mathcal{R}^n, l \in \mathcal{R}, u \in \mathcal{R}\f$.
   */
  class LinearCut : public Cut {

    public:
      /// Default Constructor.
      LinearCut();

      /// Construct a cut of the form: \f$ax \leq b \f$
      LinearCut(LinearFunctionPtr lf, double lb, double ub);

      /// Destroy
      ~LinearCut();

      /// By how much does a given point x violate this cut.
      double getViolation(const double *){ return 0.; };

      /// Get ub of the inequality.
      double getUb(){ return ub_; };

      /// Get lb of the inequality.
      double getLb(){ return lb_; };

      /// Add this cut to problem.
      void applyToProblem(ProblemPtr);

      /// Remove this cut from the problem.
      void undoToProblem(ProblemPtr);

      /// Write this cut to the outstream.
      void write(std::ostream &out) const;


    protected:
      /// Pointer to the constraint that is added to the problem.
      ConstraintPtr cons_;

      /// Function or the lhs.
      FunctionPtr f_;

      /// lb.
      double lb_;

      /// Linear function or the lhs.
      LinearFunctionPtr lf_;

      /// ub.
      double ub_;
  };
  typedef LinearCut* LinearCutPtr;
  typedef std::vector< LinearCutPtr > LinearCutVector;
  typedef CutVector::iterator LinearCutIterator;
  typedef CutVector::const_iterator LinearCutConstIterator;
}
#endif

