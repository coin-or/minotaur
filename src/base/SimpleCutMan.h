//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file SimpleCutMan.h
 * \brief Very simple implementation of a cut manager.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURSIMPLECUTMAN_H
#define MINOTAURSIMPLECUTMAN_H

#include <list>
#include "CutManager.h"
#include "Types.h"


namespace Minotaur {

  /**
   * \brief Derived class for managing cuts. Adds all violated cuts from the
   * storage to the relaxation and never removes any. If a new cut is reported
   * but not violated by the current solution then it is added to storage.
   * This manager does not check for duplicacy or any other numerical problems
   * in cuts.
   */
  class SimpleCutMan : public CutManager {

  public:
    /// Empty constructor.
    SimpleCutMan();

    /**
     * \brief Default constructor.
     *
     * \param [in] env Minotaur Environment pointer.
     * \param [in] p Problem pointer to which cuts will be added or deleted.
     */
    SimpleCutMan(EnvPtr env, ProblemPtr p);

    /// Destroy.
    ~SimpleCutMan();

    // Base class method.
    void addCut(CutPtr c);

    // Base class method.
    ConstraintPtr addCut(ProblemPtr p, FunctionPtr f, double lb, double ub,
                         bool directToRel, bool neverDelete);

    // Base class method.
    void addCuts(CutVectorIter cbeg, CutVectorIter cend);

    // Base class method.
    size_t getNumCuts() const;

    // Base class method.
    size_t getNumEnabledCuts() const;

    // Base class method.
    size_t getNumDisabledCuts() const;

    // Base class method.
    size_t getNumNewCuts() const;

    // Base class method.
    void postSolveUpdate(ConstSolutionPtr sol, EngineStatus eng_status);

    // Base class method.
    void separate(ProblemPtr p, ConstSolutionPtr sol, bool *separated,
                  size_t *added);

    // Base class method.
    void write(std::ostream &out) const;

    // Base class method.
    void writeStats(std::ostream &out) const;

  private:
    /// Cuts already in the relaxation.
    size_t enCuts_;

    /// Environment.
    EnvPtr env_;

    /// For logging.
    LoggerPtr logger_;

    /// For logging.
    const static std::string me_;

    /**
     * Cut storage for new cuts, i.e. those sent to pool after previous
     * separate or postSolveUpdate().
     */
    CutList newCuts_;

    /// The relaxation problem that cuts are added to and deleted from.
    ProblemPtr p_;

    /// Pool of cuts that were left unviolated. They may be added in the
    /// future.
    CutList pool_;

    /// A cut will be added only if the violation exceeds violAbs_.
    double violAbs_;

    /// A cut will be added only if the violation exceeds absolute
    /// value of the the activity times the violRel_.
    double violRel_;

    /// Append the newCuts_ to pool_ and clear newCuts_.
    void mvNewToPool_();
  };

  //typedef CutManager* CutManagerPtr;
}  //namespace Minotaur
#endif
