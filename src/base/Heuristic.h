//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file Heuristic.h
 * \brief Define abstract base class for heuristics of various kinds.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 * 
 * Defines the base class Heuristic. 
 */

#ifndef MINOTAURHEURISTIC_H
#define MINOTAURHEURISTIC_H

#include "Types.h"
#include "SolutionPool.h"

namespace Minotaur {

  class   Node;
  class   Relaxation;
  typedef Relaxation* RelaxationPtr;


  /**
   */
  class Heuristic {
    public:

      /// Default constructor.
      Heuristic() {};

      /// Destroy
      virtual ~Heuristic() {};

      /// Use this heuristic.
      virtual void solve(NodePtr node, RelaxationPtr rel, 
          SolutionPoolPtr s_pool) = 0;

      /// Use this heuristic.
      virtual void solveNode(ConstSolutionPtr , NodePtr , RelaxationPtr , 
          SolutionPoolPtr )
      {}
  

      /// Write statistics to the logger.
      virtual void writeStats(std::ostream &out) const = 0;
  };

}
#endif

