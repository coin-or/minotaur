//
// MINOTAUR -- It's only half bull!
//
// (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file SolCheck.h
 * \brief SolCheck class for checking feasibility of a solution
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef SOLCHECK_H
#define SOLCHECK_H

#include "Types.h"
#include "Solver.h"

namespace Minotaur {
/**
 * For a given MINLO model and a solution, report constraint violations and
 * objective value of the solution. Used for debugging.
 */
class SolCheck : public Solver {
public:
  /// Default constructor.
  SolCheck(EnvPtr env);

  /// Destroy.
  ~SolCheck();

  void doSetup();

  /// show help messages
  void showHelp() const;

  /// Display information
  int showInfo();

  /// Solve the problem
  int solve(ProblemPtr p);

  /// get status of the last solve.
  std::string getAbout();

  /// get status of the last solve.
  SolveStatus getStatus();

  /// get solution of the last solve.
  // virtual DoubleVector getSolution();
  virtual SolutionPtr getSol() {return NULL;};

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



