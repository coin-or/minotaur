///
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file RecipHandler.h
 * \brief Implement the handler for functions of type
 * y = 1/x
 * \author Neeraj Kumar, IIT Bombay
 */

#ifndef MINOTAURRECIPHANDLER_H
#define MINOTAURRECIPHANDLER_H

#include "Handler.h"
#include "Types.h"

namespace Minotaur {

  class LinearFunction;
  typedef LinearFunction *LinearFunctionPtr;

  class RecipHandler : public Handler {
  public:
    RecipHandler(EnvPtr env, ProblemPtr problem);
    ~RecipHandler();

    /**
     * Register y = 1/x with the handler.
     * sense: 'E' (y = 1/x), 'L' (y >= 1/x), 'G' (y <= 1/x)
     */
    void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                       ConstVariablePtr ovar, char sense = 'E');

    // base class method -- not used here
    void addConstraint(ConstraintPtr) override { assert(0); }

    void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) override {}
    void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf) override;
    void relaxNodeFull(NodePtr, RelaxationPtr, bool *) override {}
    void relaxNodeInc(NodePtr node, RelaxationPtr rel,
                      bool *isInfeasible) override;

    bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation,
                    bool &should_prune, double &inf_meas) override;

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

    bool presolveNode(RelaxationPtr p, NodePtr node, SolutionPoolPtr s_pool,
                      ModVector &p_mods, ModVector &r_mods) override;

    std::string getName() const override;
    void writeStats(std::ostream &out) const override;

  private:
    struct RecipCons {
      ConstraintPtr con;         ///< Original constraint
      ConstVariablePtr iv;       ///< Input variable x
      ConstVariablePtr ov;       ///< Output variable y
      VariablePtr riv;           ///< Relaxation var for x
      VariablePtr rov;           ///< Relaxation var for y
      char sense;                ///< 'L','G','E'
      ConstraintPtr secCon;      ///< Secant (chord) row
      ConstraintVector linCons;  ///< Three tangent rows

      RecipCons(ConstraintPtr newcon, ConstVariablePtr ivar,
                ConstVariablePtr ovar, char s)
        : con(newcon), iv(ivar), ov(ovar), riv(0), rov(0), sense(s),
          secCon(0), linCons(3)
      {
      }
    };

    typedef RecipCons *RecipConsPtr;
    typedef std::vector<RecipConsPtr> RecipConsVec;
    typedef RecipConsVec::iterator RecipConsIter;

    struct SepaStats {
      int iters;        ///< Number of times separate called
      int tangentcuts;  ///< Number of tangent cuts added
    };

    struct RecipPresolveStats {
      UInt iters;
      UInt vBnd;
      RecipPresolveStats() : iters(0), vBnd(0) {}
    };

    RecipConsVec consd_;
    static const std::string me_;
    RecipPresolveStats pStats_;
    SepaStats sStats_;
    double eTol_;   ///< violation tolerance (feasibility, branching, cuts)
    double vTol_;   ///< lb and ub closer than this are treated as equal
    ProblemPtr p_;
    LoggerPtr log_;

    // Tangent cut at the current point (if violated)
    void addCut_(VariablePtr x, VariablePtr y, double xval, double yval,
                 RelaxationPtr rel, bool &ifcuts);

    // Tangent of y = 1/x at x0 (x0 = 0: free placeholder row). Creates the
    // row if 'row' is null, otherwise overwrites it.
    ConstraintPtr addTangent_(VariablePtr x, VariablePtr y, double x0,
                              RelaxationPtr rel, ConstraintPtr row);

    // Three tangent rows over the current bounds of x
    void addLin_(RecipCons &cd, RelaxationPtr rel);

    // Chord row over the current bounds of x
    void addSecant_(RecipCons &cd, RelaxationPtr rel);

    // Branching point for x (relaxation variable)
    double getBrPoint_(VariablePtr v, double xval) const;

    // Violation of y = 1/x at the given solution vector
    double getViol_(const RecipCons &cd, const DoubleVector &x) const;

    bool isFeasible_(ConstSolutionPtr sol, RelaxationPtr relaxation,
                     bool &should_prune, double &inf_meas);

    // Build/refresh the rows of one constraint. Returns true if the node
    // is infeasible.
    bool relaxRecip_(RecipCons &cd, RelaxationPtr rel, bool init);

    // Bounds on y from bounds on x and vice versa. Returns true if infeasible.
    bool propRecipBnds_(RecipConsPtr cd, bool *changed);

    int updatePBnds_(VariablePtr p, double newlb, double newub,
                     bool *changed);

    bool varBndsFromCons_(bool *changed);
  };

}  // namespace Minotaur
#endif  // MINOTAURRECIPHANDLER_H
