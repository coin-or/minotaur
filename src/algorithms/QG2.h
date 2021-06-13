//
// MINOTAUR -- It's only half bull!
//
// (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file QG.h
 * \brief Define the QG class.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef QG2_H
#define QG2_H

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
  class QG2 : public Solver {
    public:
      /// Default constructor.
      QG2(EnvPtr env);

      /// Destroy.
      ~QG2();

      /// show help messages
      void showHelp() const;

      /// Display information
      int showInfo();

      /// Solve the problem
      virtual int solve(ProblemPtr p);

      /// get status of the last solve.
      virtual SolveStatus getStatus();

      /// get status of the last solve.
      virtual DoubleVector getSolution();

      /// Return the upper bound for the optimal value
      double getUb();

      /// Return the lower bound for the optimal value
      double getLb();

    private:
      const static std::string me_;
      double objSense_;
      ProblemPtr oinst_;
      SolveStatus status_;

      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
      int writeBnbStatus_(BranchAndBound *bab);
  };
}
#endif

