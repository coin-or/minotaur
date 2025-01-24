//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file WeakBrancher.h
 * \brief Declare methods and data structures for weak branching.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 */

#ifndef MINOTAURWEAKBRANCHER_H
#define MINOTAURWEAKBRANCHER_H

#include "Brancher.h"

namespace Minotaur
{

class Engine;
class Timer;
typedef Engine* EnginePtr;

struct WeakBrStats {
  UInt calls;      /// Number of calls to find branching candidate.
  UInt nodePruned; /// Number of times node was pruned by strong branching.
  double time;     /// Time taken in weak branching
};
// A class to select a variable for branching using weak branching.
class WeakBrancher : public Brancher
{
public:
  /**
   * \brief Construct using an environment pointer and handlers.
   * \param [in] env Environment from which options etc. are obtained.
   * \param [in] handlers Array of handlers which are used to obtain branching
   * candidates and modifications. This array can not be changed while
   * branch-and-bound is running.
   */
  WeakBrancher(EnvPtr env, HandlerVector& handlers);

  /// Destructor.
  ~WeakBrancher();

  // base class function.
  Branches findBranches(RelaxationPtr rel, NodePtr node, ConstSolutionPtr sol,
                        SolutionPoolPtr s_pool, BrancherStatus& br_status,
                        ModVector& mods);

  // base class function.
  std::string getName() const;

  /**
   * \brief Set problem
   *
   * \param[in] p problem.
   */
  void setProblem(ProblemPtr p);

  // base class function.
  void updateAfterSolve(NodePtr node, ConstSolutionPtr sol);

  /// Write statistics.
  void writeStats(std::ostream& out) const;

private:
  /// A vector of candidates
  BrVarCandSet cands_;

  /// Tolerance
  const double eTol_;

  /// Vector of general candidates for branching
  BrCandVector gencands_;

  /**
   * \brief Handlers that will be used to find candidates and the relaxations
   * after branching.
   */
  HandlerVector handlers_;

  /// Logger.
  LoggerPtr logger_;

  /// Name
  const static std::string me_;

  /// Modifications that can be applied to the problem.
  ModVector mods_;

  /// Problem
  ProblemPtr p_;

  /// Relaxation solved at the node
  RelaxationPtr rel_;

  /// Vector of Reduced costs
  const double* rCost_;

  /// Statistics.
  WeakBrStats* stats_;

  /// Brancher status
  BrancherStatus status_;

  /// Values of the varaible in the current solution of the relaxation
  DoubleVector x_;

  /// Timer
  const Timer* timer_;

  double computeObjChange_(DoubleVector oldLb, DoubleVector oldUb);

  /**
   * \brief Find the variable that should be selected for branching.
   *
   * This method can also find modifications based upon the
   * results of weak branching.
   * \param[in] objval Optimal objective value of the current relaxation.
   * \param[in] cutoff The cutoff value for objective function (an upper
   * bound).
   */
  BrCandPtr findBestCandidate_(SolutionPoolPtr s_pool);

  /**
   * \brief Find candidates for branching.
   *
   * Fills up the set of candidates in the cands_ array by calling all handlers.
   */
  void findCandidates_(bool& should_prune);

  /**
   * Clean up candidates, except for the no_del
   * candidate.
   */
  void freeCandidates_(BrCandPtr no_del);

  /**
   * \brief Calculate score from the up score and down score.
   *
   * \param[in] up_score Up score.
   * \param[in] down_score Down score.
   */
  double getScore_(const double& up_score, const double& down_score);

  /**
   * \brief Do weak branching on candidate.
   * \param[in] cand Candidate for weak branching.
   * \param[out] obj_up objective value estimate in up branch.
   * \param[out] obj_down objective value estimate in down branch.
   */
  void weakBranch_(BrCandPtr cand, double& obj_up, double& obj_down,
                   SolutionPoolPtr s_pool);

  /**
   * \brief Display score details of the candidate.
   *
   * \param[in] cand Candidate
   * \param[in] score Score of candidate.
   * \param[in] change_up Up change.
   * \param[in] change_down Down change.
   */
  void writeScore_(BrCandPtr cand, double score, double change_up,
                   double change_down);
};
typedef WeakBrancher* WeakBrancherPtr;
} // namespace Minotaur
#endif

