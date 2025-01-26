// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file LexicoBrancher.h
 * \brief Declare methods and data structures for lexicographic branching.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURLEXICOBRANCHER_H
#define MINOTAURLEXICOBRANCHER_H

#include "Brancher.h"

namespace Minotaur {

  struct LexicoBrStats {
    UInt calls;      /// Number of times called to find a branching candidate.
    double time;     /// Total time spent in branching.
  };

  class Timer;

  /**
   * A class to select the first branching candidate that is avialable. Can
   * be used for demonstration or when we don't have any idea how to branch.
   */
  class LexicoBrancher : public Brancher {
    
    public:
      /// Default constructor.
      LexicoBrancher();

      /// Construct with an environment.
      LexicoBrancher(EnvPtr env, HandlerVector handlers);

      /// Destroy.
      ~LexicoBrancher();

      /**
       * Find a branching candidate. Returns NULL if x does not have any
       * fractional values for integer constrained variables.
       */
      Branches findBranches(RelaxationPtr rel, NodePtr node, 
                            ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                            BrancherStatus & br_status, ModVector &mods);

      /// Write the statistics.
      void writeStats(std::ostream &out) const;

      /// Return name.
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
      LexicoBrStats *stats_;

  };
  typedef LexicoBrancher* LexicoBrancherPtr;
}
#endif

