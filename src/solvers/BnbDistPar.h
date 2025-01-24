//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file BnbDistPar.h
 * \brief Define the BnbDistPar class.
 * \author Meenarli Sharma and Prashant Palkar, IIT Bombay
 */

#ifndef BNBDISTPAR_H
#define BNBDISTPAR_H

#include "Types.h"
#include "AMPLInterface.h"
#include "LPEngine.h"
#include "DistParBranchAndBound.h"
#include "ParReliabilityBrancher.h"
#include "Presolver.h"
#include "Solver.h"

namespace Minotaur {
  /**
   * The BnbDistPar class sets up methods for solving a convex MINLP instance using
   * parallel implementation of Branch-and-Bound.
   */
  class BnbDistPar : public Solver {
    public:
      /// Default constructor.
      BnbDistPar(EnvPtr env);

      /// Destroy.
      ~BnbDistPar();

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

      BrancherPtr createBrancher_(HandlerVector handlers, EnginePtr e, int proc_rank, int num_procs);
      DistParBranchAndBound* createDistParBab_(UInt numThreads, EnginePtr e, RelaxationPtr relCopy[],
                                  ParPCBProcessorPtr nodePrcssr[],
                                  ParNodeIncRelaxerPtr parNodeRlxr[],
                                  HandlerVector handlersCopy[],
                                  EnginePtr eCopy[], int proc_rank, int num_procs);
      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
      void setInitialOptions_();
      void writeNLPStats_(std::string name, std::vector<double> stats);
      void writeDistParBnbStatus_(DistParBranchAndBound *parbab, double wallTimeStart, clock_t clockTimeStart);
  };
}
#endif

