//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file BnbPar.h
 * \brief Define the BnbPar class.
 * \author Meenarli Sharma and Prashant Palkar, IIT Bombay
 */

#ifndef BNBPAR_H
#define BNBPAR_H

#include "Types.h"
#include "AMPLInterface.h"
#include "LPEngine.h"
#include "ParBranchAndBound.h"
#include "ParReliabilityBrancher.h"
#include "Presolver.h"
#include "Solver.h"

namespace Minotaur {
  /**
   * The BnbPar class sets up methods for solving a convex MINLP instance using
   * parallel implementation of Branch-and-Bound.
   */
  class BnbPar : public Solver {
    public:
      /// Default constructor.
      BnbPar(EnvPtr env);

      /// Destroy.
      ~BnbPar();

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

      BrancherPtr createBrancher_(HandlerVector handlers, EnginePtr e);
      ParBranchAndBound* createParBab_(UInt numThreads, EnginePtr e, RelaxationPtr relCopy[],
                                  ParPCBProcessorPtr nodePrcssr[],
                                  ParNodeIncRelaxerPtr parNodeRlxr[],
                                  HandlerVector handlersCopy[],
                                  EnginePtr eCopy[]);
      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
      void setInitialOptions_();
      void writeNLPStats_(std::string name, std::vector<double> stats);
      void writeParBnbStatus_(ParBranchAndBound *parbab, double wallTimeStart, clock_t clockTimeStart);
  };
}
#endif

