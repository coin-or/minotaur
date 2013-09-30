// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2013 The MINOTAUR Team.
// 

/**
 * \file RandomBrancher.h
 * \brief Declare methods and data structures for Random branching.
 * \author Suresh B, IIT Bombay
 */


#ifndef MINOTAURRANDOMBRANCHER_H
#define MINOTAURRANDOMBRANCHER_H

#include "Brancher.h"

namespace Minotaur {

  struct RandomBrStats {
    UInt calls;      /// Number of times called, to find a branching candidate.
    Double time;     /// Total time spent in branching.
  };

  class Timer;

  /**
   * A class to select random branching candidate from the available ones.
   */
  class RandomBrancher : public Brancher {
    
    public:
      /// Default constructor.
      RandomBrancher();

      /// Construct with an environment.
      RandomBrancher(EnvPtr env, HandlerVector handlers);

      /// Destroy.
      ~RandomBrancher();

      // Find a branching candidate. Returns NULL if x does not have any
      // thing to branch on or if no branching candidates are needed. 
      // (e.g. when we realize that problem is infeasible). 
      Branches findBranches(RelaxationPtr rel, NodePtr node, 
          ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
          BrancherStatus & br_status, ModificationPtr &mod);

      // Write the statistics.
      void writeStats();

      // Return name.
      std::string getName() const;

    private:
      /// Name
      const static std::string me_; 

      /// Log manager.
      Logger* logger_;

      /// Handlers that provide branching candidates.
      HandlerVector handlers_;

      /// Timer
      Timer *timer_;

      /// Statistics.
      RandomBrStats *stats_;

      /// Status of problem after using this brancher
      BrancherStatus status_;

      /// Seed to random number generator
      UInt seed_;

  };
  typedef boost::shared_ptr<RandomBrancher> RandomBrancherPtr;
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
