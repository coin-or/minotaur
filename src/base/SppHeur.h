//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file SppHeur.h
 * \brief Define the class SppHeur for finding solutions to set partitioning
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 */

#ifndef MINOTAURSPPHEUR_H
#define MINOTAURSPPHEUR_H

#include "Heuristic.h"
#include "Types.h"

namespace Minotaur
{

struct SppHeurStats {
  UInt calls;   // Number of times heuristic called
  UInt runs;   // Number of times heuristic runs
  UInt sols;    // Number of solutions found
  double time;  // Time taken in this heuristic
};


class SppHeur : public Heuristic
{
public:
  /// default constructor
  SppHeur(EnvPtr env, ProblemPtr p);

  /// default destructor
  ~SppHeur() {};

  /// call to heuristic
  virtual void solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// call to heuristic
  virtual void solveRand(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// writing the statistics to the logger
  virtual void writeStats(std::ostream& out) const;

private:
  // Environment
  EnvPtr env_;

  // logging
  Logger* logger_;

  // Maximum random solutions to check
  UInt maxRand_;

  // For printing messages
  static const std::string me_;

  // Problem
  ProblemPtr p_;

  // Statistics
  SppHeurStats* stats_;

  // Vector of cons
  ConstraintVector cv2_;

  void addSol_(SolutionPool *spool) ;

  /// call to heuristic
  void conWise_(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// call to heuristic
  void flip_(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// check if the problem is set partitioning
  bool isSpp_();

  // get new point from a feasible point
  void getNewPoint_(double* x, double* xl, double* xu, SolutionPoolPtr s_pool);

  // Check whether x is feasible
  bool isFeasible_(const double* x);

  bool propVal_(VariablePtr v, int &num0);

  void solveLP_(SolutionPool *spool);
};

} // namespace Minotaur
#endif

