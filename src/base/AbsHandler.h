////
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file AbsHandler.h
 * \brief Declare the handler for functions of type y = |x|.
 * \author Neeraj Kumar, IIT Bombay
 */

#ifndef MINOTAURABSHANDLER_H
#define MINOTAURABSHANDLER_H

#include "Handler.h"
#include "Types.h"

namespace Minotaur {

  class AbsHandler : public Handler {
  public:
    AbsHandler(EnvPtr env, ProblemPtr problem);
    ~AbsHandler();

    /**
     * Register y = |x| with the handler.
     * sense: 'E' (y = |x|), 'L' (y >= |x|), 'G' (y <= |x|)
     */
    void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                       ConstVariablePtr ovar, char sense = 'E');

    // base class method -- not used here
    void addConstraint(ConstraintPtr) override { assert(0); }

    void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) override {}
    void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf) override;
    void relaxNodeFull(NodePtr, RelaxationPtr, bool *) override {}
    void relaxNodeInc(NodePtr node, RelaxationPtr rel,
                      bool *is_infeasible) override;

    bool isFeasible(ConstSolutionPtr sol, RelaxationPtr rel,
                    bool &should_prune, double &inf_meas) override;

    // No cuts: the relaxation is exact once x is on one side of 0
    void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                  CutManager *cutman, SolutionPoolPtr s_pool,
                  ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                  SeparationStatus *status) override;

    void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                                ModVector &mods, BrVarCandSet &cands,
                                BrCandVector &gencands, bool &is_inf) override;

    ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x,
                             RelaxationPtr rel, BranchDirection dir) override;

    Branches getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel,
                         SolutionPoolPtr s_pool) override;

    SolveStatus presolve(PreModQ *pre_mods, bool *changed,
                         Solution **sol) override;

    bool presolveNode(RelaxationPtr rel, NodePtr node, SolutionPoolPtr s_pool,
                      ModVector &p_mods, ModVector &r_mods) override;

    std::string getName() const override;
    void writeStats(std::ostream &out) const override;

  private:
    struct AbsCons {
      ConstraintPtr con;     ///< Original constraint
      VariablePtr iv;        ///< Input variable x
      VariablePtr ov;        ///< Output variable y
      VariablePtr riv;       ///< Relaxation var for x
      VariablePtr rov;       ///< Relaxation var for y
      char sense;            ///< 'L','G','E'
      ConstraintPtr secCon;  ///< Upper-side row (chord), rewritten at every node

      AbsCons(ConstraintPtr c, VariablePtr i, VariablePtr o, char s)
        : con(c), iv(i), ov(o), riv(0), rov(0), sense(s), secCon(0)
      {
      }
    };

    typedef AbsCons *AbsConsPtr;
    typedef std::vector<AbsConsPtr> AbsConsVec;
    typedef AbsConsVec::iterator AbsConsIter;

    struct AbsPresolveStats {
      UInt iters;
      UInt vBnd;
      AbsPresolveStats() : iters(0), vBnd(0) {}
    };

    AbsConsVec consd_;
    static const std::string me_;
    AbsPresolveStats pStats_;
    double eTol_;   ///< violation tolerance (feasibility, branching)
    double vTol_;   ///< lb and ub closer than this are treated as equal
    ProblemPtr p_;
    LoggerPtr log_;

    // y >= x and y >= -x: global and exact, added once at the root
    void addLin_(AbsCons &cd, RelaxationPtr rel);

    // Row y - m*x <= c valid on the current bounds of x. Created at the root,
    // overwritten at every node; a free row if no valid upper side exists.
    void addSecant_(AbsCons &cd, RelaxationPtr rel);

    // Violation of y = |x|; 0 if x is on one side of 0 (relaxation exact)
    double getViol_(const AbsCons &cd, const DoubleVector &x) const;

    // Bounds on y from bounds on x and vice versa. Returns true if infeasible.
    bool propAbsBnds_(AbsConsPtr cd, bool *changed);

    int updatePBnds_(VariablePtr p, double newlb, double newub,
                     bool *changed);
  };

  typedef AbsHandler *AbsHandlerPtr;

}  // namespace Minotaur

#endif  // MINOTAURABSHANDLER_H
