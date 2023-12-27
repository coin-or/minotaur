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
  // A map of constraints with the number of unfixed variables in the constratint
  std::map<ConstraintPtr, UInt> consNumVar_;

  // Environment
  EnvPtr env_;

  // For printing messages
  static const std::string me_;

  // Multiplier for binary variables to create a priority
  UInt mbin_;

  // Multiplier for nonlinear variables to create a priority
  UInt mnl_;

  // Variable fixing mods done
  std::stack<Modification*> mods_;

  // Problem
  ProblemPtr p_;

  // Statistics
  FixVarsHeurStats* stats_;

  // fix the variable v
  void fix_(VariablePtr v);

  // Fix Variables
  void FixVars_(UIntSet& unfixedVars);

  // initialize the correct data structures for heuristic
  void initialize_();

  // Check whether x is feasible
  bool isFeasible_(const double* x);

  // Compares two elements in a map
  bool mapCompare_(std::pair<UInt, UInt>& p1, std::pair<UInt, UInt>& p2);

  // gets the best variable to fix
  VariablePtr selectVarToFix_(std::map<UInt, UInt>& unfixedVars);

  // update Map of each variable in the constraint once it is covered
  void updateMap_(ConstraintPtr c, std::map<UInt, UInt>& unfixedVars);
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
