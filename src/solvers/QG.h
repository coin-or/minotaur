//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file QG.h
 * \brief Define the QG class.
 * \author Ashutosh Mahajan and Meenarli Sharma, IIT Bombay
 */

#ifndef QG_H
#define QG_H

#include "Types.h"
#include "AMPLInterface.h"
#include "BranchAndBound.h"
#include "LPEngine.h"
#include "Presolver.h"
#include "Solver.h"

namespace Minotaur {
  /**
   * The QG class sets up methods for solving a convex MINLP instance using
   * the LP/NLP based Quesada Grossmann algorithm.
   */
  class QG : public Solver {
    public:
      /// Default constructor.
      QG(EnvPtr env);

      /// Destroy.
      ~QG();

      void doSetup();

      void sepDetection();

      /// show help messages
      void showHelp() const;

      /// Display information
      int showInfo();

      /// Solve the problem
      virtual int solve(ProblemPtr p);

      /// get status of the last solve.
      virtual SolveStatus getStatus() {return status_;};

      /// get status of the last solve.
      virtual SolutionPtr getSol() {return sol_;};

      /// Return the upper bound for the optimal value
      double getUb() {return ub_;};

      /// Return the lower bound for the optimal value
      double getLb() {return lb_;};

    private:
      const static std::string me_;
      double objSense_;
      SolutionPtr sol_;
      double lb_, ub_;
      ProblemPtr oinst_;
      SolveStatus status_;
      // SolutionPtr sol_;

      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
      void setInitialOptions_();
      int writeBnbStatus_(BranchAndBound *bab);
  };
}
#endif

