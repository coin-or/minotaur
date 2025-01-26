//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file SamplingHeur.h
 * \brief Define the class SamplingHeur derived from base class Heuristics.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 * Declares the class SamplingHeur.
 */

#ifndef MINOTAURSAMPLINGHEUR_H
#define MINOTAURSAMPLINGHEUR_H

#include "Heuristic.h"
#include "Types.h"

namespace Minotaur
{
class Problem;
/**
    * \brief A statistic struct for trivial heuristic
    */
struct SamplingHeurStats {
  UInt numSol;  // Number of solutions found
  double time;  // Time taken in this heuristic
  UInt checked; // Number of points checked
};

class SamplingHeur : public Heuristic
{
public:
  /// default constructor
  SamplingHeur(EnvPtr env, ProblemPtr p);

  /// default destructor
  ~SamplingHeur();

  /// call to heuristic
  void solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// writing the statistics to the logger
  void writeStats(std::ostream& out) const;

private:
  // Environment
  EnvPtr env_;

  // Maximum random solutions to check
  UInt maxRand_;

  // For printing messages
  static const std::string me_;

  // Problem
  ProblemPtr p_;

  // Statistics
  SamplingHeurStats* stats_;

  // get new point from a feasible point
  void getNewPoint_(double* x, double* xl, double* xu, SolutionPoolPtr s_pool);

  // Check whether x is feasible
  bool isFeasible_(const double* x);
};

typedef SamplingHeur* SamplingHeurPtr;
} // namespace Minotaur
#endif

