// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file MaxVioBrancher.h
 * \brief Declare methods and data structures for maximum violation branching.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURMAXVIOBRANCHER_H
#define MINOTAURMAXVIOBRANCHER_H

#include "Brancher.h"

namespace Minotaur {

  class Timer;

  struct MaxVioBrStats {
    UInt calls;      /// Number of times called to find a branching candidate.
    double time;     /// Total time spent in branching.
  };

  /// A class to select a variable for branching using maximum-violated branching.
  class MaxVioBrancher : public Brancher {
    
    public:
      /// Construct using an environment pointer and initialize .
      MaxVioBrancher(EnvPtr env, HandlerVector & handlers);

      /// Destroy.
      ~MaxVioBrancher();

      // find branches after a node is processed.
      Branches findBranches(RelaxationPtr rel, NodePtr node, 
                            ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                            BrancherStatus & br_status, ModVector &mods);

      /// Update pseudo-cost/other information after branching.
      virtual void updateAfterSolve(NodePtr , ConstSolutionPtr );

      // write statistics.
      void writeStats(std::ostream &out) const;

      /// Return name.
      std::string getName() const;

    private:
      /// Set of candidates (variables for branching).
      BrVarCandSet cands_; 

      /// Vector of candidates (general candidates for branching).
      BrCandVector gencands_;

      /**
       * \brief Handlers that will be used to find candidates and also the
       * relaxations after branching.
       */
      HandlerVector handlers_;

      /// Name
      const static std::string me_; 

      /// The problem that is being solved at this node.
      RelaxationPtr rel_;

      /// Statistics.
      MaxVioBrStats * stats_;

      /// Timer
      Timer *timer_;

      DoubleVector x_;

      /**
       * Clean up candidates, except for the no_del candidate.
       */
      void freeCandidates_(BrCandPtr no_del);

      /// Fills up the set of candidates in the cands_ array. 
      void findCandidates_(ModVector &mods, bool &should_prune);

      /// Returns the candidate that was violated maximally.
      BrCandPtr findBestCandidate_();

  };
  typedef MaxVioBrancher* MaxVioBrancherPtr;
}
#endif

