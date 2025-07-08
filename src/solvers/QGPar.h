//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
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
#include "LPEngine.h"
#include "ParBranchAndBound.h"
#include "ParReliabilityBrancher.h"
#include "ParQGBranchAndBound.h"
#include "Presolver.h"
#include "Solver.h"

namespace Minotaur {
  typedef SolutionPool* SolutionPoolPtr;
  /**
   * The QGPar class sets up methods for solving a convex MINLP instance using
   * Parallel LP/NLP based Quesada Grossmann algorithm.
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


      BrancherPtr createBrancher_(ProblemPtr p, HandlerVector handlers,
                           EnginePtr e);
      ParQGBranchAndBound* createParBab_(UInt numThreads, NodePtr &node,
                                  RelaxationPtr relCopy[], ProblemPtr pCopy[],
                                  ParPCBProcessorPtr nodePrcssr[],
                                  ParNodeIncRelaxerPtr parNodeRlxr[],
                                  SolutionPoolPtr solPool,
                                  HandlerVector handlersCopy[],
                                  LPEnginePtr lpeCopy[], EnginePtr eCopy[],
                                  bool &prune);
      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
      void setInitialOptions_();
      void writeLPStats_(std::string name, std::vector<double> stats);
      void writeNLPStats_(std::string name, std::vector<double> stats);
      void writeParQGStats_(ParQGBranchAndBound *parbab, UInt numThreads, HandlerVector handlersCopy[]);
      void writeParBnbStatus_(ParQGBranchAndBound *parbab, double wallTimeStart, clock_t clockTimeStart);
  };
}
#endif

