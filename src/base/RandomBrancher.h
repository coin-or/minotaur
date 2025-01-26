// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
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
    double time;     /// Total time spent in branching.
  };

  class Timer;

  /// A class to select random branching candidate from the available ones.
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
    Branches findBranches(RelaxationPtr rel, NodePtr node, 
                          ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                          BrancherStatus & br_status, ModVector &mods);

    // Write the statistics.
    void writeStats(std::ostream &out) const;

    // Return name.
    std::string getName() const;

  private:
    /// Name
    const static std::string me_; 

    /// Log manager.
    LoggerPtr logger_;

    /// Handlers that provide branching candidates.
    HandlerVector handlers_;

    /// Timer
    Timer *timer_;

    /// Statistics.
    RandomBrStats *stats_;

    /// Seed to random number generator
    UInt seed_;

  };
  typedef RandomBrancher* RandomBrancherPtr;
}
#endif

