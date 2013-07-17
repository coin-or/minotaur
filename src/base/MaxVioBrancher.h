// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2013 The MINOTAUR Team.
// 

/**
 * \file MaxVioBrancher.h
 * \brief Declar methods and data structures for maximum-violated branching.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURMAXVIOBRANCHER_H
#define MINOTAURMAXVIOBRANCHER_H

#include "Brancher.h"

namespace Minotaur {

  class Timer;

  struct MaxVioBrStats {
    UInt calls;      /// Number of times called to find a branching candidate.
    Double time;     /// Total time spent in branching.
  };

  /// A class to select a variable for branching using reliability branching.
  class MaxVioBrancher : public Brancher {
    
    public:
      /// Construct using an environment pointer and initialize .
      MaxVioBrancher(EnvPtr env, HandlerVector & handlers);

      /// Destroy.
      ~MaxVioBrancher();

      /**
       * Find a branching candidate. Returns NULL if x does not have any
       * thing to branch on or if no branching candidates are needed. 
       * (e.g. when * we realize that problem is infeasible). 
       */
      Branches findBranches(RelaxationPtr rel, NodePtr node, 
          ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
          BrancherStatus & br_status, ModificationPtr &mod);

      /// Update pseudo-cost/other information after branching.
      virtual void updateAfterLP(NodePtr , ConstSolutionPtr );

      // write statistics.
      void writeStats();

      /// Return name.
      std::string getName() const;

    private:
      /// Name
      const static std::string me_; 

      /// Tolerance for avoiding division by zero.
      const Double eTol_;

      /// The problem that is being solved at this node.
      RelaxationPtr rel_;

      DoubleVector x_;

      /**
       * Handlers that will be used to find candidates and the relaxations
       * after branching.
       */
      HandlerVector handlers_;

      /// Timer
      Timer *timer_;

      /// Statistics.
      MaxVioBrStats * stats_;

      /// Set of candidates.
      BrCandSet cands_; 

      /**
       * Fills up the set of candidates in the cands_ array. 
       */
      void findCandidates_();

      /**
       * Returns the candidate that was violated maximally.
       */
      BrCandPtr findBestCandidate_();

  };
  typedef boost::shared_ptr<MaxVioBrancher> MaxVioBrancherPtr;
}
#endif

// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
