//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file FixVarsHeur.h
 * \brief Define the class FixVarsHeur derived from base class Heuristics.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 * Declares the class FixVarsHeur.
 */

#ifndef MINOTAURFIXVARSHEUR_H
#define MINOTAURFIXVARSHEUR_H

#include "Heuristic.h"
#include "Types.h"

namespace Minotaur
{
class Problem;
/**
    * \brief A statistic struct for trivial heuristic
    */
struct FixVarsHeurStats {
  UInt numSol; // Number of solutions found
  double time; // Time taken in this heuristic
};

class FixVarsHeur : public Heuristic
{
public:
  /// default constructor
  FixVarsHeur(EnvPtr env, ProblemPtr p);

  /// default destructor
  ~FixVarsHeur();

  /// call to heuristic
  void solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// writing the statistics to the logger
  void writeStats(std::ostream& out) const;

protected:
  // Environment
  EnvPtr env_;

  // For printing messages
  static const std::string me_;

  // Problem
  ProblemPtr p_;

  // Statistics
  FixVarsHeurStats* stats_;

  // Check whether x is feasible
  bool isFeasible_(const double* x);
};

typedef FixVarsHeur* FixVarsHeurPtr;
} // namespace Minotaur
#endif

// Local Variables:
// mode: c++
// eval: (c-set-style "k&r")
// eval: (c-set-offset 'innamespace 0)
// eval: (setq c-basic-offset 2)
// eval: (setq fill-column 78)
// eval: (auto-fill-mode 1)
// eval: (setq column-number-mode 1)
// eval: (setq indent-tabs-mode nil)
// End:
