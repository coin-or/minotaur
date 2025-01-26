//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file NLPMultiStart.h
 * \brief Define a derived class from class Heuristic for multistart heuristic.
 * \author Jayash Koshal, Argonne National Laboratory
 * \author A. Mahajan, IIT Bombay
 * 
 * Defines the base class Heuristic. 
 */

#ifndef MINOTAURNLPMULTISTART_H
#define MINOTAURNLPMULTISTART_H

#include "Heuristic.h"
#include "Types.h"

namespace Minotaur {

  class Engine;
  class Problem;
  

  /// Statistic for Multistart heuristic
  struct MSHeurStats {
    UInt numNLPs;
    UInt numInfeas;
    UInt numImprove;
    UInt numBadstatus;
    double time;
    UInt iterations;
    double bestObjValue;
  };


  /**
   * \brief Multistart heuristic for continuous NLPs.
   *
   * A Heuristic used to find solutions for continuous NLPs by solving the
   * NLP using NLP engine. The engine is called multiple times from different
   * strategically constructed starting points.
   */
  class NLPMultiStart : public Heuristic {
    
    public:

      /// Default constructor.
      NLPMultiStart(EnvPtr env, ProblemPtr p, EnginePtr e);

      /// Destroy.
      ~NLPMultiStart(); 

      /// Use this heuristic.
      void solve(NodePtr node, RelaxationPtr rel, SolutionPoolPtr s_pool);

      // Write statistics to logger
      void writeStats(std::ostream &out) const;
 
      const static std::string me_; 
          
    private:
 
      /** bound on the distance. If radius of the ball is less than this
       * number then radius = distBound_
       */
      double distBound_;

      /// Engine being used to solve problem.
      EnginePtr e_;
   
      /// Environment
      EnvPtr env_;

      /// Logger.
      LoggerPtr logger_;
     
      /// Problem that is being solved.
      ProblemPtr p_;

      /// random search direction 
      double *random_;

      /// Statistics for Multistart heuristic
      MSHeurStats stats_;

      /** 
       * \brief New starting point construction.
       *
       * Construct an initial point outside a ball centered at current optimal
       * solution of the NLP solver and radius being distance between current
       * starting point amplified by a factor. 
       *
       * \param[in] Pointa Current initial point for the solver
       * \param[in] Pointb Current optimal solution
       * \param[in] rho The amplification factor
       * \param]in] vars Number of variables
       */
      void constructInitial_(double* a, const double* b, double rho, UInt vars);
      
  };

  typedef NLPMultiStart* NLPMSPtr;
}
#endif

