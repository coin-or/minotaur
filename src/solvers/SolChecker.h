//
// MINOTAUR -- It's only half bull!
//
// (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file SolChecker.h
 * \brief Define the SolChecker class.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef SOLCHECKER_H
#define SOLCHECKER_H

#include "Types.h"
#include "Solver.h"

namespace Minotaur {
/**
 * The SolChecker class sets up methods for solving a convex MINLP instance
 * using the NLP based Branch-and-Bound
 */
class SolChecker : public Solver {
public:
  /// Default constructor.
  SolChecker(EnvPtr env);

  /// Destroy.
  ~SolChecker();

  void doSetup();

  /// show help messages
  void showHelp() const;

  /// Display information
  int showInfo();

  /// Solve the problem
  int solve(ProblemPtr p);

  /// get status of the last solve.
  SolveStatus getStatus();

  /// get solution of the last solve.
  // virtual DoubleVector getSolution();

  /// Return the upper bound for the optimal value
  double getUb();

  /// Return the lower bound for the optimal value
  double getLb();

private:
  const static std::string me_;
  double objSense_;
  ProblemPtr oinst_;
  SolveStatus status_;

};
}
#endif



