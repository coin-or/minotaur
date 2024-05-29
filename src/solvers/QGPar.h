//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file QGPar.h
 * \brief Define the QGPar class.
 * \author Meenarli Sharma and Prashant Palkar, IIT Bombay
 */

#ifndef QGPAR_H
#define QGPAR_H

#include "Types.h"
#include "AMPLInterface.h"
#include "BranchAndBound.h"
#include "LPEngine.h"
#include "Presolver.h"
#include "Solver.h"

namespace Minotaur {
  /**
   * The QGPar class sets up methods for solving a convex MINLP instance using
   * the LP/NLP based Quesada Grossmann algorithm.
   */
  class QGPar : public Solver {
    public:
      /// Default constructor.
      QGPar(EnvPtr env);

      /// Destroy.
      ~QGPar();

      void doSetup();

      //void sepDetection();

      /// show help messages
      void showHelp() const;

      /// Display information
      int showInfo();

      /// Solve the problem
      virtual int solve(ProblemPtr p);

      /// get status of the last solve.
      virtual SolveStatus getStatus() {return status_;};

      /// get status of the last solve.
      virtual SolutionPtr getSolution() {return sol_;};

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

      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
      void setInitialOptions_();
      int writeBnbStatus_(BranchAndBound *bab);
  };
}
#endif

