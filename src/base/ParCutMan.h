//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file ParCutMan.h
 * \brief Manages storage, addition and deletion of cuts to problem.
 * \author Meenarli Sharma, Prashant Palkar, IIT Bombay
 */

#ifndef MINOTAURPARCUTMAN_H
#define MINOTAURPARCUTMAN_H

#include <list>
#include "CutManager.h"
#include "Types.h"


namespace Minotaur {

  /**
   * \brief Derived class for managing cuts. Add and remove cuts based on
   * priority and violation.
   */
  class ParCutMan : public CutManager {

  public:
    /// Empty constructor.
    ParCutMan();

    /**
     * \brief Default constructor.
     *
     * \param [in] env Minotaur Environment pointer.
     * \param [in] p Problem pointer to which cuts will be added or deleted.
     */
    ParCutMan(EnvPtr env, ProblemPtr p);

    /// Destroy.
    ~ParCutMan();

    // Base class method.
    void addCut(CutPtr c);

    void addCutToPool(CutPtr cut);

    std::vector<ConstraintPtr> getPoolCons();

    ConstraintPtr addCut(ProblemPtr, FunctionPtr, double, double, bool, bool)
    {
      return ConstraintPtr();
    };

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
                  size_t *n_added);

    // Base class method.
    void write(std::ostream &out) const;

    // Base class method.
    void writeStats(std::ostream &out) const;

  private:
    /// Absolute tolerance limit for comparing double values.
    double absTol_;

    /// Cut storage for disabled cuts, i.e., those not added to the problem.
    CutList disCuts_;

    /// Cut storage for enabled cuts, i.e. those added to the problem.
    CutList enCuts_;

    /// Environment.
    EnvPtr env_;

    /// For logging.
    LoggerPtr logger_;

    /// Maximum number of iterations before which a disabled cut is deleted.
    size_t maxDisCutAge_;

    /**
     * \brief Maximum number of iterations before which an inactive cut is
     * moved out of the problem.
     */
    size_t maxInactCutAge_;

    /// For logging.
    const static std::string me_;

    /**
     * Cut storage for new cuts, i.e. those sent to pool after previous
     * separate or postSolveUpdate().
     */
    CutList newCuts_;

    /// The relaxation problem that cuts are added to and deleted from.
    ProblemPtr p_;

    CutList pool_;

    void addToRel_(CutPtr cons, bool new_cut);

    void addToPool_(CutPtr cons);
  };

  //typedef ParCutMan * ParCutManPtr;
}  //namespace Minotaur
#endif
