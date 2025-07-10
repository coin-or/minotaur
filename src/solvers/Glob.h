//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Glob.h
 * \brief The Glob class for solving MIQCQP instances using ampl (.nl) format.
 * \author Mustafa Vora, IIT Bombay
 */

#ifndef GLOB_H
#define GLOB_H

#include "AMPLInterface.h"
#include "BranchAndBound.h"
#include "Brancher.h"
#include "NLPEngine.h"
#include "Presolver.h"
#include "Solver.h"
#include "Types.h"

namespace Minotaur
{
/**
 * The Glob class sets up methods for solving an MIQCQP instance to
 * global optimality
 */
class Glob : public Solver
{
public:
  /// Default constructor.
  Glob(EnvPtr env);

  /// Destroy.
  ~Glob();

  /// Starting setup for glob
  void doSetup();

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

  // virtual SolutionPtr getSol() {return sol_;};
  virtual SolutionPtr getSol() {return sol_;};
  /// Return the upper bound for the optimal value
  double getUb();

  /// Return the lower bound for the optimal value
  double getLb();

private:
  const static std::string me_;
  double objSense_;
  ProblemPtr inst_;
  ProblemPtr newp_;
  SolveStatus status_;
  SolutionPtr sol_;

  BranchAndBound* createBab_(EnginePtr e, HandlerVector& handlers);
  PresolverPtr createPres_(HandlerVector& handlers);
  void fwd2Bnb_(VarVector *orig_v);
  void fwd2QG_();
  BrancherPtr getBrancher_(HandlerVector handlers, Engine* e);
  LPEnginePtr getEngine_();
  NLPEnginePtr getNLPEngine_();
  void setInitialOptions_();
  int transform_(ProblemPtr& newp, HandlerVector& handlers, LPEnginePtr engine);
  void writeStatus_(BranchAndBound* bab);
};
} // namespace Minotaur
#endif
