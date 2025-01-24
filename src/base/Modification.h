// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file Problem.h
 * \brief Declare the base class Modification.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURMODIFICATION_H
#define MINOTAURMODIFICATION_H

#include "Types.h"

namespace Minotaur {
  class   Modification;
  class   Problem;
  class   Relaxation;
  typedef Relaxation* RelaxationPtr;

  /**
   * Modification is a (pure) abstract class for changes that can be done to a 
   * problem   
   */
  class Modification {
    protected:
      /// Default constructor
      Modification() {}

    public:
      /// Default destroy
      virtual ~Modification() {};

      /**
       * \brief Covert a modification for a relaxation to one for its original
       * problem. 
       * \param[in] rel Relaxation for which this mod is applicable.
       * \param[in] p Problem for which the new mod will be applicable.
       * \returns Modification  applicable to p
       */
      virtual ModificationPtr fromRel(RelaxationPtr rel, ProblemPtr p) const
        = 0;

      /**
       * \brief Covert a modification for a problem to one for its
       * relaxation. 
       * \param[in] p Problem for which this mod is applicable.
       * \param[in] rel Relaxation for which the new mod will be applicable.
       * \returns Modification  applicable to rel.
       */
      virtual ModificationPtr toRel(ProblemPtr p, RelaxationPtr rel) const = 0;

      /// Apply it to the problem.
      virtual void applyToProblem(ProblemPtr problem) = 0;

      /// Restore the modification for a problem.
      virtual void undoToProblem(ProblemPtr problem) = 0;

      /// Write it to 'out'.
      virtual void write(std::ostream &out) const = 0;
  };   

}

#endif

