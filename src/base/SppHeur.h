//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
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

struct SppHeurMods {
  VarSetConstIter vit; 
  std::deque<Variable *> vq0; 
  ConstraintConstIterator cit;
  std::deque<Constraint *> cq; 
};



class SppHeur : public Heuristic
{
public:
  /// default constructor
  SppHeur(EnvPtr env, ProblemPtr p);

  /// default destructor
  ~SppHeur() {};

  /// call to heuristic
  void solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  void solveNode(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
             SolutionPoolPtr s_pool);
  
  /// writing the statistics to the logger
  void writeStats(std::ostream& out) const;

private:
  // Environment
  EnvPtr env_;

  // logging
  Logger* logger_;

  // Maximum random solutions to check
  // For printing messages
  static const std::string me_;

  // Problem
  ProblemPtr p_;

  // Statistics
  SppHeurStats stats_;

  // Vector of cons
  ConstraintVector cv2_;

  std::deque<SppHeurMods *> mstack_;
  std::deque<Variable *> vq_; 

  double addSol_(SolutionPool *spool) ;

  /// call to heuristic
  void enum_(NodePtr, RelaxationPtr, SolutionPoolPtr spool);
  void conWise_(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);
  void conWise2_(NodePtr, RelaxationPtr, SolutionPoolPtr spool);
  void varWise_(NodePtr, RelaxationPtr, SolutionPoolPtr spool);
  void sortVars_(VarVector &vvec);

  /// call to heuristic
  void conWiseNb_(const double *x, NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// check if the problem is set partitioning
  bool isSpp_();

  // Check whether x is feasible
  bool isFeasible_(const double* x);

  bool propVal_(VariablePtr v, int &num0, int &consfixed);
  bool propEnum_(Variable* v, ConstraintConstIterator cit2, VarSetConstIter vit2);

  void solveLP_(SolutionPool *spool);
};

} // namespace Minotaur
#endif

