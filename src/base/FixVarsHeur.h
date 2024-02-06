//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
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

  /// set handlers of the problem
  void setHandlers(HandlerVector& handlers);

  /// writing the statistics to the logger
  void writeStats(std::ostream& out) const;

protected:
  // A map of constraints with the number of unfixed variables in the constratint
  std::map<ConstraintPtr, UInt> consNumVar_;

  // Environment
  EnvPtr env_;

  // Handlers for the problem
  HandlerVector handlers_;

  // For printing messages
  static const std::string me_;

  // Multiplier for binary variables to create a priority
  UInt mbin_;

  // Multiplier for nonlinear variables to create a priority
  UInt mnl_;

  // Variable fixing mods done
  ModVector mods_;

  // Problem
  ProblemPtr p_;

  // Statistics
  FixVarsHeurStats* stats_;

  // fix the variable v
  void fix_(VariablePtr v);

  // Fix Variables
  void FixVars_(std::map<UInt, UInt>& unfixedVars);

  // Update the solution in solution pool if found
  void foundNewSol_(SolutionPoolPtr s_pool, bool& restart);

  // initialize the correct data structures for heuristic
  void initialize_();

  // Check whether x is feasible
  bool isFeasible_(const double* x);

  // Compares two elements in a map
  static bool mapCompare_(const std::pair<UInt, UInt>& p1,
                          const std::pair<UInt, UInt>& p2);

  // presolve here.
  bool presolve_(SolutionPoolPtr s_pool, std::map<UInt, UInt>& unfixedVars);

  // gets the best variable to fix
  VariablePtr selectVarToFix_(std::map<UInt, UInt>& unfixedVars);

  // Unfix fixed variables.
  void unfixVars_();

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
