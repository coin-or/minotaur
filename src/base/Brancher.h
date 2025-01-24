//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Brancher.h
 * \brief Declare the base class Brancher for finding and creating branches in
 * Branch-and-Bound.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURBRANCHER_H
#define MINOTAURBRANCHER_H

#include "Types.h"

namespace Minotaur {

  class   Node;
  class   Relaxation;
  class   Solution;
  class   SolutionPool;
  typedef Relaxation* RelaxationPtr;
  typedef const Solution* ConstSolutionPtr;
  typedef SolutionPool* SolutionPoolPtr;

  /**
   * \brief A brancher is used to find suitable branches for a given node. e.g.
   * LexicoBrancher. This class is abstract.
   */
  class Brancher {
    
    public:
      /// Default constructor.
      Brancher();

      /// Destroy.
      virtual ~Brancher();

      /**
       * \brief Find a branching candidate. 
       *
       * \return NULL if x does not have any
       * fractional values for integer constrained variables or if no branching
       * candidates are found (e.g. when we realize that problem is infeasible). 
       * \param[in] rel Relaxation at the current node.
       * \param[in] node The current node.
       * \param[in] sol The solution at the current node.
       * \param[in] s_pool Solution pool containing known feasible solutions.
       * \param[out] br_status Status returned by this brancher.
       * \param[out] mods Modification returned by this brancher. NULL if none
       * found.
       */
      virtual Branches findBranches(RelaxationPtr rel, NodePtr node, 
                                    ConstSolutionPtr sol,
                                    SolutionPoolPtr s_pool, 
                                    BrancherStatus & br_status,
                                    ModVector &mods) = 0;

      /// Return the name of this brancher.
      virtual std::string getName() const = 0;

      /**
       * \brief Update pseudo-costs after LP is solved.
       *
       * \param[in] node The node for which relaxation is solved. The pseudo cost of
       * branching candidate used in node->parent is updated.
       * \param[in] sol The solution of the relaxation at this node (not the
       * parent).
       */
      virtual void updateAfterSolve(NodePtr node, ConstSolutionPtr sol);

      /// Write statistics to the given out stream.
      virtual void writeStats(std::ostream &) const {};

    protected:
      /// Log manager.
      LoggerPtr logger_;

  };
  typedef Brancher* BrancherPtr;

}
#endif

