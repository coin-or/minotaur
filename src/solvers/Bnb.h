//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Bnb.h
 * \brief Define the Bnb class.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef BNB_H
#define BNB_H

#include "Types.h"
#include "AMPLInterface.h"
#include "BranchAndBound.h"
#include "Brancher.h"
#include "Presolver.h"
#include "Problem.h"
#include "Solver.h"

namespace Minotaur {
/**
 * The Bnb class sets up methods for solving a convex MINLP instance using
 * the NLP based Branch-and-Bound
 */
class Bnb : public Solver {
public:
  /// Default constructor.
  Bnb(EnvPtr env);

  /// Destroy.
  ~Bnb();
  
  void doSetup();

  /// show help messages
  void showHelp() const;

  /// Display information
  int showInfo();

  /// Solve the problem
  virtual int solve(ProblemPtr p);

  virtual std::string getAbout();

  /// get status of the last solve.
  virtual SolveStatus getStatus();

  /// get solution of the last solve.
  virtual SolutionPtr getSol();

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
  void writeBnbStatus_(BranchAndBound *bab);
  void setInitialOptions_();
};
}
#endif


