//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file CutManager.h
 * \brief Manages addition and deletion of cuts to problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURCUTMANAGER_H
#define MINOTAURCUTMANAGER_H

#include "Types.h"


namespace Minotaur {

  class Environment;
  class Function;
  class Problem;
  class Solution;
  class Cut;
  class CutManager;
  typedef CutManager *CutManagerPtr;
  typedef const Solution *ConstSolutionPtr;

  /**
   * \brief Abstract base class to manage cuts in the relaxation.
   *
   * The CutManager class is meant to ``manage'' the cutting planes generated
   * by different cut generators and handlers. By manage we mean:
   * -# Removing cuts if they become inactive.
   * -# Adding cuts from storage if violated.
   * -# Adding newly generated cuts.
   * -# Keeping a list of constraints that are cuts.
   * This class is not meant to generate cuts (a job of CutGenerator) or even
   * store them (CutStorage).
   */
  class CutManager {

  public:
    /// Empty constructor.
    CutManager() {};

    /// Destroy.
    virtual ~CutManager() {};

    /**
     * \brief Add a cut to be managed by the cut manager.
     *
     * \param [in] c Cut pointer to be added. Do not change a cut after it is
     * added.
     */
    virtual void addCut(CutPtr c) = 0;

    /**
     * \brief Add a cut to the pool managed by the cut manager.
     *
     * \param [in] c Cut pointer to be added.
     */
    virtual void addCutToPool(CutPtr c) = 0;

    /**
     * \brief Add a cut
     * \param [in] p Problem to add a cut to
     * \param [in] f Function of the cut
     * \param [in] lb Lower bound
     * \param [in] ub Upper bound
     * \param [in] direct_to_rel Whether add the cut directly to rel
     * \param [in] never_del Whether the cut should always remain in the pool
     * \return A pointer to the constraint added to rel
     */
    virtual ConstraintPtr addCut(ProblemPtr p, FunctionPtr f, double lb,
                                 double ub, bool direct_to_rel,
                                 bool never_del) = 0;

    /**
     * \brief Add a cut to be managed by the cut manager. The cut is not added
     * to the problem by this function (See separate() for it).
     *
     * \param [in] cbeg 'Begin' iterator of the vector of cuts to be added.
     * \param [in] cend 'End' iterator of the vector of cuts to be added.
     */
    virtual void addCuts(CutVectorIter cbeg, CutVectorIter cend) = 0;

    /// Get the total number of cuts available to the manager.
    virtual size_t getNumCuts() const = 0;

    /**
     * \brief Get the total number of cuts currently enabled, i.e., added to
     * the relaxation.
     */
    virtual size_t getNumEnabledCuts() const = 0;

    /// Get the total number of cuts currently not enabled.
    virtual size_t getNumDisabledCuts() const = 0;

    /**
     * \brief Get the total number of cuts added to manager (but not to
     * problem) before the last call to separate() or postSolveUpdate().
     */
    virtual size_t getNumNewCuts() const = 0;

    virtual std::vector<ConstraintPtr> getPoolCons() = 0;

    /**
     * \brief Update information on a node which is branched.
     * \param[in] NodePtr branched node.
     * \param[in] ConstSolutionPtr solution of the lp solve in the node.
     * \param[in] int number of children
     */
    virtual void nodeIsBranched(NodePtr, ConstSolutionPtr, int) {};

    /**
     * \brief Update information when a node is processed.
     * \param[in] NodePtr node which has already been processed
     */
    virtual void nodeIsProcessed(NodePtr){};

    /**
     * Updates information about the cuts after problem is solved.
     * Also removes cuts from relaxation and/or the storage.
     * \param[in] sol Solution from previous solve.
     * \param[in] e_status Status of the engine
     */
    virtual void postSolveUpdate(ConstSolutionPtr sol,
                                 EngineStatus e_status) = 0;

    /**
     * \brief Separate a given point using the cuts in the storage.
     * \param[in] sol Solution that needs to be separated.
     * \param[out] separated True if solution is separated by cuts. False
     * otherwise.
     * \param[out] n_added The number of cuts added by this routine to the
     * problem.
     */
    virtual void separate(ProblemPtr p, ConstSolutionPtr sol, bool *separated,
                          size_t *n_added) = 0;

    /**
     * \brief Update information of the cuts in the pool
     * \param [in] problem Pointer to the relaxation problem
     * \param [in] sol Solution of the LP solve
     * \param [in] node Node in which information gets updated
     */
    virtual void updatePool(ProblemPtr, ConstSolutionPtr){};

    /**
     * \brief Update information about cuts in the relaxation
     * \param [in] sol Solution of the LP solve
     * \param [in] problem Pointer to the relaxation problem
     */
    virtual void updateRel(ConstSolutionPtr, ProblemPtr){};

    /// Write cuts to output.
    virtual void write(std::ostream &out) const = 0;

    /// Write statistics to output.
    virtual void writeStats(std::ostream &out) const = 0;
  };
}  //namespace Minotaur
#endif
