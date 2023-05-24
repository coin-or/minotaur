//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file StrongBrancher.h
 * \brief Declare methods and data structures for strong branching.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 */

#ifndef MINOTAURSTRONGBRANCHER_H
#define MINOTAURSTRONGBRANCHER_H

#include "Brancher.h"

namespace Minotaur {

class Engine;
class Timer;
typedef Engine *EnginePtr;

struct StrBrStats {
  UInt calls;       /// Number of calls to find branching candidate.
  UInt engProbs;    /// Number of times an unexpected engine status was met.
  UInt nodePruned;  /// Number of times node was pruned by strong branching.
  double time;      /// Time taken in strong branching
};
// A class to select a variable for branching using strong branching.
class StrongBrancher : public Brancher {
 public:
  /**
   * \brief Construct using an environment pointer and handlers.
   * \param [in] env Environment from which options etc. are obtained.
   * \param [in] handlers Array of handlers which are used to obtain branching
   * candidates and modifications. This array can not be changed while
   * branch-and-bound is running.
   */
  StrongBrancher(EnvPtr env, HandlerVector &handlers);

  /// Destructor.
  ~StrongBrancher();

  // base class function.
  Branches findBranches(RelaxationPtr rel, NodePtr node, ConstSolutionPtr sol,
                        SolutionPoolPtr s_pool, BrancherStatus &br_status,
                        ModVector &mods);

  // base class function.
  std::string getName() const;

  /**
   * \brief Set engine.
   *
   * \param[in] engine Pointer to the engine used in strong branching.
   */
  void setEngine(EnginePtr engine);

  /**
   * \brief Set maximum candidates for limited strong branching.
   *
   * \param[in] max_cands Maximum candidates to be strong branched on.
   */
  void setMaxCands(UInt max_cands);

  /**
   * \brief Set maximum iterations while limited strong branching
   *
   * \param[in] max_iter Maximum iterations to do in strong branching
   */
  void setMaxIter(UInt max_iter);

  // base class function.
  void updateAfterSolve(NodePtr node, ConstSolutionPtr sol);

  /// Write statistics.
  void writeStats(std::ostream &out) const;

 private:
  /// A vector of candidates
  BrVarCandSet cands_;

  /// Engine for strong branching
  EnginePtr engine_;

  /// Tolerance
  const double eTol_;

  /// Vector of general candidates for branching
  BrCandVector gencands_;

  /**
   * \brief Handlers that will be used to find candidates and the relaxations
   * after branching.
   */
  HandlerVector handlers_;

  /// Do strong branching on maximum these many candidates
  UInt maxCands_;

  /**
   * \brief Maximum number of iterations to be performed in each call to the
   * engine.
   */
  UInt maxIterations_;

  /// Name
  const static std::string me_;

  /// Modifications that can be applied to the problem.
  ModVector mods_;

  /// Relaxation solved at the node
  RelaxationPtr rel_;

  /// Statistics.
  StrBrStats *stats_;

  /// Brancher status
  BrancherStatus status_;

  /// Timer
  const Timer *timer_;

  /// Values of the varaible in the current solution of the relaxation
  DoubleVector x_;

  /**
   * \brief Find the variable that was selected for branching.
   *
   * This method can also find modifications based upon the
   * results of strong branching.
   * \param[in] objval Optimal objective value of the current relaxation.
   * \param[in] cutoff The cutoff value for objective function (an upper
   * bound).
   * \param[in] node The node at which we are branching.
   */
  BrCandPtr findBestCandidate_(const double objval, double cutoff);

  /**
   * \brief Find candidates for branching.
   *
   * Fills up the set of candidates in the cands_ array by calling all handlers.
   */
  void findCandidates_(bool &should_prune);

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
  double getScore_(const double &up_score, const double &down_score);

  /**
   * \brief Check if branch can be pruned on the basis of engine status and
   * objective value.
   *
   * Check status and tell if we can prune the branch. is_rel is false if
   * the engine reports failure in convergence.
   * \param[in] chcutoff The minimum change in objective that will lead to
   * cutoff. A change greater than this value will exceed the
   * objective-cutoff.
   * \param[in] change The observed change in the objective function value.
   * \param[in] status Solve status of the engine.
   * \param[out] is_rel True if the engine status is reliable enough to use
   * the solution value as a reliable bound.
   */
  bool shouldPrune_(const double &chcutoff, const double &change,
                    const EngineStatus &status, bool *is_rel);

  /**
   * \brief Do strong branching on candidate.
   * \param[in] cand Candidate for strong branching.
   * \param[out] obj_up objective value estimate in up branch.
   * \param[out] obj_down objective value estimate in down branch.
   * \param[out] status_up engine status in up branch.
   * \param[out] status_down engine status in down branch.
   */
  void strongBranch_(BrCandPtr cand, double &obj_up, double &obj_down,
                     EngineStatus &status_up, EngineStatus &status_down);

  /**
   * \brief Analyze the strong-branching results.
   *
   * \param[in] cand Candidate for which we performed strong branching.
   * \param[in] chcutoff The minimum change in objective function value that
   * will result in cutoff.
   * \param[in,out] change_up Change observed in the objective function value
   * in the up branch. It is modified if the engine status is error or
   * unknown.
   * \param[in,out] change_down Change observed in the objective function
   * value in the down branch. It is modified if the engine status is error or
   * unknown.
   * \param[in] status_up The engine status in up branch.
   * \param[in] status_down The engine status in down branch.
   */
  void useStrongBranchInfo_(BrCandPtr cand, const double &chcutoff,
                            double &change_up, double &change_down,
                            const EngineStatus &status_up,
                            const EngineStatus &status_down);

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
typedef StrongBrancher *StrongBrancherPtr;
}  // namespace Minotaur
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
