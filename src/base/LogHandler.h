//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file LogHandler.h
 * \brief Implement the handler for functions of type
 * y=log(x).
 * \author Neeraj Kumar, IIT Bombay
 */


#ifndef MINOTAURLOGHANDLER_H
#define MINOTAURLOGHANDLER_H

#include "Handler.h"
#include "Types.h"
namespace Minotaur {

  class Engine;
  class Function;
  class LinearFunction;
  class Objective;
  class Problem;
  typedef LinearFunction *LinearFunctionPtr;
  typedef Objective *ObjectivePtr;


  class LogHandler : public Handler {


  private:
    // ---------------------------------------------------------
    // Per-constraint data for log constraints (data only)
    // ---------------------------------------------------------
    struct LogCons
    {
      ConstraintPtr con;         ///< Original constraint
      ConstVariablePtr iv;       ///< Input variable x
      ConstVariablePtr ov;       ///< Output variable y
      VariablePtr riv;           ///< Relaxation var for x
      VariablePtr rov;           ///< Relaxation var for y
      char sense;                ///< 'L','G','E'
      ConstraintPtr secCon;      ///< Secant constraint
      ConstraintVector linCons;  ///< Linearization constraints

      LogCons(ConstraintPtr newcon, ConstVariablePtr ivar,
              ConstVariablePtr ovar, char s)
        : con(newcon),
          iv(ivar),
          ov(ovar),
          riv(0),
          rov(0),
          sense(s),
          secCon(0),
          linCons()
      {
      }
    };

    typedef LogCons *LogConsPtr;
    typedef std::vector<LogConsPtr> LogConsVec;
    typedef LogConsVec::iterator LogConsIter;

    // All log constraints data
    LogConsVec consd_;

    // For printing
    static const std::string me_;

    // ---------------------------------------------------------
    // Presolve statistics containers
    // ---------------------------------------------------------
    struct LogPresolveStats
    {
      UInt iters;
      double time;
      UInt conDel;
      UInt vBnd;
      UInt nMods;

      LogPresolveStats()
        : iters(0),
          time(0.0),
          conDel(0),
          vBnd(0),
          nMods(0)
      {
      }
    };

    struct BoundTighteningStats
    {
      int niters;
      int nLP;
      int dlb;
      int dub;
      double timeLP;
    };

    BoundTighteningStats bStats_;
    LogPresolveStats pStats_;

    // Default bounds for presolve
    double LBd_;
    double UBd_;
    double bTol_;  ///< minimum tightening threshold

    // Environment
    EnvPtr env_;

    // Original problem
    ProblemPtr orig_;

    // Optional cuts
    ConstraintVector optCuts_;

    
    /**
     * \Add or update linearization cuts for y = log(x) around sample points.
     * \Used for convex underestimators of log(x) and equality constraints.
     */
    void addLin_(LogCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                 DoubleVector &grad, ModVector &mods, bool init);


    /**
    * \Add or update the secant inequality for y = log(x).
    * \Used in both initRelaxFor() and updateRelaxFor().
    */
    void addSecant_(LogCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                    ModVector &mods, bool init);

    /**
     * \Compute scaled violation of log constraint for branching.
     *
     * \Evaluates f(x) for the stored constraint:
     *    - For y = log(x):  check f(x) ∈ [lb - tol , ub + tol]
     * \Returns:
     *    - Relative violation (0 = feasible, >0 = violated)
     *
     * \Used inside getBranchingCandidates().
     */

    //Marks duplicate constraint if changed == True
    void dupRows_(bool *changed);
    //branching decision
    BranchPtr doBranch_(BranchDirection UpOrDown, ConstVariablePtr v,
                        double bvalue);


    double getViol_(const LogCons &cd, const DoubleVector &x) const;

    /**
     * \Initialize relaxation components for a single log constraint.
     * \Called once when relaxation is constructed at a node.
     */
    void initRelax_(LogCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                    DoubleVector &grad);


    // For a given y = log(x) constraint,
    // derive bounds on y from bounds on x. Also derive bounds on x from bounds
    // on y.
    bool propLogBnds_(LogConsPtr lcd, bool *changed);

    //Deletes duplicate row which was marked by dupRows
    bool treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double mult,
                       bool *changed);

  

    /**
     * \Update the relaxation for a log constraint when bounds have changed.
     * \Performs:
     *  \ Recompute/update the secant inequality
     *  \Update linearizations at current variable bounds
     *  \ Append Modifications (mods) so that the B&B node receives them
     * \Called inside relaxNodeInc().
     */
    void updateRelax_(LogCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                      DoubleVector &grad, ModVector &mods);

    /// Update problem bounds
    int updatePBnds_(VariablePtr p, double newlb, double newub,
                     bool *changed);

    bool varBndsFromCons_(bool *changed);


    // Absolute feasibility tolerance
    double aTol_;

    // Relative feasibility tolerance
    double rTol_;

    // Tolerances for log constraints
    double eTol_;

    // Tolerance for when lower and upper bounds are considered equal
    double vTol_;

    // Problem pointer
    ProblemPtr p_;

    // Logger
    LoggerPtr log_;

    // Temporary vectors for evaluations
    DoubleVector tmpX_;
    DoubleVector grad_;

  public:
    // Constructor
    LogHandler(EnvPtr env, ProblemPtr problem);

    // Destructor
    ~LogHandler();

    /**
     * \Register a log constraint with the handler.
     * \ ivar: input variable x
     * \ ovar: output variable y
     * \ sense: 'L', 'G', or 'E'
     */
    void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                       ConstVariablePtr ovar, char sense = 'E');

    // base class method—not used here
    void addConstraint(ConstraintPtr) { assert(0); }

   
    // Full relaxation (unused)
    void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) {}

    // Create initial incremental relaxation.
    void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf);

    // Check feasibility.
    bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation,
                    bool &should_prune, double &inf_meas);

    // Separation (cuts) — not implemented yet.
    void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                  CutManager *cutman, SolutionPoolPtr s_pool,
                  ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                  SeparationStatus *status);

    // Full relaxation at a node (unused for LogHandler)
    void relaxNodeFull(NodePtr, RelaxationPtr, bool *) {}

    // Incremental relaxation at a node.
    void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *isInfeasible);

    // Branching candidates.
    void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                                ModVector &mods, BrVarCandSet &cands,
                                BrCandVector &gencands, bool &is_inf);

    // Branch modification for a variable.
    ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x,
                             RelaxationPtr rel, BranchDirection dir);

    // Generate branches.
    Branches getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel,
                         SolutionPoolPtr s_pool);

    // Top-level presolve.
    SolveStatus presolve(PreModQ *pre_mods, bool *changed, Solution **sol);

    // Node-level presolve.
    bool presolveNode(RelaxationPtr p, NodePtr node, SolutionPoolPtr s_pool,
                      ModVector &p_mods, ModVector &r_mods);

    // Name of the handler.
    std::string getName() const override;

    // Stats
    void writeStats(std::ostream &out) const override;
  };

}  // namespace Minotaur

#endif  // MINOTAURLOGHANDLER_H
