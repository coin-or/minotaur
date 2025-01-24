// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file MaxInfBrancher.h
 * \brief Declare methods and data structures for hybrid-inference score branching.
 * \author Krushna Salunke, IIT Bombay
 */

#ifndef MINOTAURMAXINFBRANCHER_H
#define MINOTAURMAXINFBRANCHER_H

#include "Brancher.h"

namespace Minotaur {

class Engine;
class Timer;
typedef Engine* EnginePtr;

struct MaxInfBrStats {
    UInt bndChange;  /// Number of times variable bounds were changed.
    UInt calls;      /// Number of times called to find a branching candidate.
    UInt engProbs;   /// Number of times an unexpected engine status was met.
    UInt iters;      /// Number of iterations in strong-branching.
    UInt strBrCalls; /// Number of times strong branching on a variable.
    double time;     /// Total time spent in branching.
};

/// A class to select a variable for branching using maximum-inference branching.
class MaxInfBrancher : public Brancher {
public:
    /// Construct using an environment pointer and initialize.
    MaxInfBrancher(EnvPtr env, HandlerVector & handlers);

    /// Destroy.
    ~MaxInfBrancher();

    // Find a branching candidate. Returns NULL if x does not have any
    // thing to branch on or if no branching candidates are needed. 
    Branches findBranches(RelaxationPtr rel, NodePtr node, 
                          ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                          BrancherStatus & br_status, ModVector &mods);

    // Update pseudo-cost/other information after branching.
    void updateAfterSolve(NodePtr node, ConstSolutionPtr sol);

    // Write statistics.
    void writeStats(std::ostream &out) const;

    /// Return value of trustCutoff parameter.
    bool getTrustCutoff();

    /// Get iteration limit of engine.
    UInt getIterLim();

    // base class function.
    std::string getName() const;

    /// Return the threshhold value.
    UInt getThresh() const;

    /**
     * \brief Initialize data structures.
     *
     * \param[in] rel Relaxation for which this brancher is used.
     */
    void initialize(RelaxationPtr rel);

    /// Set value of trustCutoff parameter.
    void setTrustCutoff(bool val);

    /**
     * \brief Set engine.
     *
     * \param[in] engine Pointer to the engine used in strong branching.
     */
    void setEngine(EnginePtr engine);

    /**
     * \brief Set iteration limit of engine.
     *
     * \param [in] k The new iteration limit.
     */
    void setIterLim(UInt k);

    /**
     * \brief Set the depth at which we stop strong branching.
     *
     * \param[in] k The new depth value.
     */
    void setMaxDepth(UInt k);

    /**
     * \brief Don't do strong branching on a cand if we did it 'k' nodes
     * or less ago.
     * \param[in] k The new minimum value.
     */
    void setMinNodeDist(UInt k);

    /**
     * \brief Set reliability threshhold
     * 
     * The pseudo cost is considered reliable if it has been updated k
     * times.
     * \param[in] k The new threshhold value.
     */
    void setThresh(UInt k);

    // Print inference score
    void printMaxInferenceScores() const;

    // Vector to store up score
    std::vector<double> upScores_;

    // Vector to store down score
    std::vector<double> downScores_;

    //Infrence score function
    void infScore();
    void statsTable()const;

    //Normalize function to calculate score
    double gFunction(double x) const;
 
private:

    // inference functions
    void infAggregation_(int idx, int nvars, int nposbin, double wt1, double wt2, ConstraintPtr c);
    void infPrecedence_(int idx, int nvars, int nnegbin, int nposbin, ConstraintPtr c);
    void infVariableBound_(int idx, int nposbin, double wt, double wt1, double wt2, ConstraintPtr c);
    void infSetPartition_(int idx, int nvars, double wt);
    void infSetPack_(int idx, int nvars, double wt);
    void infCardinality_(int idx, int nvars, int nposbin, int nnegcoefone, double wt, ConstraintPtr c);
    void infEquationKnapsack_(int idx, int nvars, double wt, ConstraintPtr c, double sumnegwt);
    void infBinPack_(int idx, int nvars, double wt, ConstraintPtr c, double sumnegwt);
    void infKnapsack_(int idx, int nvars, double wt, ConstraintPtr c, int nposbin, double sumnegwt);
    void infMixed_(VariablePtr v, int idx, double wt, ConstraintPtr c);
    void findCandidates_();

    void freeCandidates_(BrCandPtr no_del);
    void getPCScore_(BrCandPtr cand, double *ch_down, double *ch_up, double *score);
    double getScore_(const double & up_score, const double & down_score);
    bool shouldPrune_(const double &chcutoff, const double &change, const EngineStatus & status, bool *is_rel);
    BrCandPtr findBestCandidate_(const double objval, double cutoff, 
                               NodePtr node);

    /** 
     * \brief Do strong branching on candidate.
     * \param[in] cand Candidate for strong branching.
     * \param[out] obj_up objective value estimate in up branch.
     * \param[out] obj_down objective value estimate in down branch.
     * \param[out] status_up engine status in up branch.
     * \param[out] status_down engine status in down branch.
     */
    void strongBranch_(BrCandPtr cand, double & obj_up, double & obj_down, EngineStatus & status_up, EngineStatus & status_down);

    /**
     * \brief Update Pseudocost based on the new costs.
     *
     * \param[in] i Index of the candidate.
     * \param[in] new_cost The new cost estimate.
     * \param[in] cost The vector of costs of all candidates. cost[i] is to be updated
     * \param[in] count The vector that keeps a cound of how many times cost has been updated for each candidate.
     */
    void updatePCost_(const int &i, const double &new_cost, DoubleVector &cost, UIntVector &count);

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
    void useStrongBranchInfo_(BrCandPtr cand, const double & chcutoff, double & change_up, double & change_down, const EngineStatus & status_up, const EngineStatus & status_down);

    /** 
     * \brief Display score details of the candidate.
     *
     * \param[in] cand Candidate
     * \param[in] score Score of candidate.
     * \param[in] change_up Up change.
     * \param[in] change_down Down change.
     */
    void writeScore_(BrCandPtr cand, double score, double change_up, double change_down);
    void writeScores_(std::ostream &out);

    EnginePtr engine_;
    const double eTol_;
    UIntVector lastStrBranched_;
    UInt maxDepth_;
    UInt maxIterations_;
    UInt maxStrongCands_;
    UInt minNodeDist_;
    ModVector mods_;
    DoubleVector pseudoDown_;
    DoubleVector pseudoUp_;
    std::vector<BrCandPtr> relCands_;
    UIntVector timesDown_;
    UIntVector timesUp_;
    UInt thresh_;
    bool trustCutoff_;
    std::vector<BrCandPtr> unrelCands_;
    BrancherStatus status_;
    RelaxationPtr rel_;
    DoubleVector x_;
    HandlerVector handlers_;
    Timer *timer_;
    MaxInfBrStats *stats_;
    BrVarCandSet cands_;
    bool init_;
      /** 
   * \brief The vector that keeps a count of how many times a candidate
   * has been fraction, for each candidate.
   */
   UIntVector fracCount_;

  /** 
   * \brief The vector that keeps a count of how many times a candidate
   * has been not fixed, for each candidate.
  */
   UIntVector unfixedCount_;
  /**
   * \brief Update fractional count vector fracCount_ based upon current
   * set of candidates.
  */
   void updateFracCount_();
  /**
   * \brief Update unfixed count vector unfixedCount_ based upon current
   * relaxation.
  */ 
   void updateUnfixedCount_();

  /// Vector for actial times up 
   UIntVector actualTimesUp_;

  ///Vector for actual times down
   UIntVector actualTimesDown_;
   std::vector<std::string> variableNames_;


    /// Name
    const static std::string me_; 
};

typedef MaxInfBrancher* MaxInfBrancherPtr;

} // namespace Minotaur

#endif // MINOTAURMAXINFBRANCHER_H
