//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
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
  UInt numSol;      // Number of solutions found
  UInt numPresolve; // Number of times presolve was done
  double time;      // Time taken in this heuristic
};

class FixVarsHeur : public Heuristic
{
public:
  /// default constructor
  FixVarsHeur(EnvPtr env, ProblemPtr p);

  /// default destructor
  ~FixVarsHeur();

  /// create handlers for the problem
  void createHandlers();

  /// call to heuristic
  void solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// set handlers of the problem
  void setHandlers(HandlerVector& handlers);

  /// writing the statistics to the logger
  void writeStats(std::ostream& out) const;

private:
  // A map of constraints with the number of unfixed variables in the constratint
  std::map<ConstraintPtr, UInt> consNumVar_;

  // Down lock for each variable
  UIntVector downLock_;

  // Environment
  EnvPtr env_;

  // Handlers for the problem
  HandlerVector handlers_;

  // Check with lock number done?
  bool lock_;

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

  // Up lock for each variable
  UIntVector upLock_;

  // Compute lock number of each variable
  void computeLocks_();

  // fix the variable v
  void fix_(VariablePtr v);

  // fix the variable v based on its lock number
  void fixByLock_(VariablePtr v);

  // Fix Variables
  void FixVars_(std::map<VariablePtr, UInt>& unfixedVars);

  // Update the solution in solution pool if found
  void foundNewSol_(SolutionPoolPtr s_pool, bool& restart);

  // initialize the correct data structures for heuristic
  void initialize_();

  // Check whether x is feasible
  bool isFeasible_(const double* x);

  // Compares two elements in a map
  static bool mapCompare_(const std::pair<VariablePtr, UInt>& p1,
                          const std::pair<VariablePtr, UInt>& p2);

  // presolve here.
  bool presolve_(SolutionPoolPtr s_pool,
                 std::map<VariablePtr, UInt>& unfixedVars);

  // gets the best variable to fix
  VariablePtr selectVarToFix_(std::map<VariablePtr, UInt>& unfixedVars);

  // Unfix fixed variables.
  void unfixVars_();

  // update itmp of each variable in the constraint if it is covered right from start
  void updateItmp_(ConstraintPtr c);

  // update Map of each variable in the constraint once it is covered
  void updateMap_(ConstraintPtr c, std::map<VariablePtr, UInt>& unfixedVars);
};

typedef FixVarsHeur* FixVarsHeurPtr;
} // namespace Minotaur
#endif

