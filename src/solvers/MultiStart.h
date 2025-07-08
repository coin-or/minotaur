//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file MultiStart.h
 * \brief Define the MultiStart class
 * \author Prashant Palkar and Ashutosh Mahajan, IIT Bombay
 */

#ifndef MULTISTART_H
#define MULTISTART_H

#include "Types.h"
#include "AMPLInterface.h"
#include "BranchAndBound.h"
#include "Brancher.h"
#include "Presolver.h"
#include "Solver.h"

namespace Minotaur {
/**
 * The MultiStart class sets up methods for heuristically solving a nonconvex
 * MINLP instance using the NLP based Branch-and-Bound. NLP is started from
 * multiple starting points to get a good estimate of the lower bound.
 */
class MultiStart : public Solver {
public:
  /// Default constructor.
  MultiStart(EnvPtr env);

  /// Destroy.
  ~MultiStart();

  /// show help messages
  void showHelp() const;

  /// Display information
  int showInfo();

  /// Solve the problem
  virtual int solve(ProblemPtr p);

  /// get status of the last solve.
  virtual SolveStatus getStatus();

  /// get solution of the last solve.
  virtual DoubleVector getSolution();
  virtual SolutionPtr getSol() {return sol_;};

  /// Return the upper bound for the optimal value
  double getUb();

  /// Return the lower bound for the optimal value
  double getLb();

private:
  const static std::string me_;
  double objSense_;
  ProblemPtr oinst_;
  SolveStatus status_;
  SolutionPtr sol_;

  BranchAndBound* getBab_(Engine *engine, HandlerVector &handlers);
  BrancherPtr getBrancher_(HandlerVector handlers, Engine *e);
  int getEngine_(Engine **e);
  PresolverPtr presolve_(HandlerVector &handlers);
  void setInitialOptions_();
  int writeBnbStatus_(BranchAndBound *bab);
};
}
#endif


