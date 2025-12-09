//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file  #LogHandler.h
 * \brief Define the  #LogHandler class for handling convex univariate
 * functions.
 * \author Neeraj kumar
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


  class LogConstraintData {
  
  friend class LogHandler;
  public:
    /// Creates initial relaxations
    void initRelax(RelaxationPtr rel, DoubleVector &tmpX, DoubleVector &grad);

    /// Update the current relaxation based on current variable bounds
    void updateRelax(RelaxationPtr rel, DoubleVector &tmpX,
                     DoubleVector &grad, ModVector &mods);

    double getViol(const std::vector<double> &x);

    // Creates and adds the secant inequality defined by current constraint to
    // the relaxation
    // Returns the constraint that was added, which may be null if it was not
    // possible to add a constraint (e.g., due to an infinite bound)
    void addSecant(RelaxationPtr rel, ConstVariablePtr iv,
                   ConstVariablePtr ov, FunctionPtr fn, DoubleVector &tmpX,
                   bool init, ModVector &mods);

    // Creates and adds linearization inequalities to approximate the lower
    // envelope of the convex function
    // Returns a vector of constraints that were added
    void addLin(RelaxationPtr rel, ConstVariablePtr iv, ConstVariablePtr ov,
                FunctionPtr fn, DoubleVector &tmpX, DoubleVector &grad,
                bool init, ModVector &mods);

    ConstraintPtr getOriginalCon() const { return con_; }
    ConstraintPtr getSecantCon() const { return secCon_; }

    ConstVariablePtr getROutVar() const { return rov_; };

    ConstVariablePtr getRInputVar() const { return riv_; };

    //ConstraintVector & getLinCons
    ConstraintIterator linConsBegin() { return linCons_.begin(); }
    ConstraintIterator linConsEnd() { return linCons_.end(); }


    char getSense() { return sense_; };

    /// Default constructor.
    LogConstraintData(double eTol, double vTol, ConstraintPtr newcon,
                      ConstVariablePtr ovar, ConstVariablePtr ivar,
                      char sense);

    /// Destroy
    ~LogConstraintData() {};

  private:

    Logger *log_;

    /// For printing.
    static const std::string me_;

    /// Tolerance for constraint violation.
    double eTol_;

    /// Tolerance for when upper and lower bounds considered equal.
    double vTol_;

    /// This is the constraint of the orginal problem
    ConstraintPtr con_;

    /// Input variable, i.e., x in y = f(x)
    /// This should be a pointer to the `original' variable, look at
    /// corresponding bounds in the relaxation to get bounds
    ConstVariablePtr iv_;

    // Points to the relaxation version of the input variable
    // Updated only after relaxInit is called
    VariablePtr riv_;

    /// Output variable, i.e., y in y = f(x)
    /// This is a pointer to the `original' variable
    ConstVariablePtr ov_;

    // Points to the relaxation version of the output variable
    // Updated only after relaxInit is called
    VariablePtr rov_;

    /// 'L' => y <= f(x), so only add secants
    /// 'G' -> y >= f(x), so only add linearizations
    /// 'E' -> y == f(x), so do both
    char sense_;

    /// Secant constraint in the relaxation
    ConstraintPtr secCon_;

    /// Array of linearization constraints in the relaxation
    ConstraintVector linCons_;


  };

  typedef LogConstraintData *LogConsPtr;
  typedef std::vector<LogConsPtr> LogConsVec;
  typedef LogConsVec::iterator LogConsIter;


  /**
   * A  #LogHandler handles log functions. The upper relaxation is
   * handled with the single secant inequality, the lower relaxation is
   * handled with linearizations.
   */

  /**
   *  TODO: This class could easily be extended to handle perspective cuts if
   * a binary varaible is known which turns off the input variable
   *
   */
  class LogHandler : public Handler {


  protected:
    /// Tolerance for constraint violation.
    double eTol_;

    /// Tolerance for when upper and lower bounds considered equal.
    double vTol_;

    /// Original problem.
    ProblemPtr p_;

    /// Logger.
    LoggerPtr log_;

    /// For printing.
    static const std::string me_;

    /// Internal data associated with each constraint
    LogConsVec consd_;

    /// A temporary vector of zeros, for evaluating functions
    DoubleVector tmpX_;

    /// A temporary vector of zeros, for getting gradients
    DoubleVector grad_;

  public:
    /// Default constructor.
    LogHandler(EnvPtr env, ProblemPtr problem);

    /// Destroy
    ~LogHandler();

    /**
     * Adds constraint to list (as all handlers), but also constructs the
     * associated constraint data.
     */
    void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                       ConstVariablePtr ovar, char sense = 'E');

    // base class method.
    void addConstraint(ConstraintPtr) { assert(0); };

    /**
     *  For this handler, nothing is different at root or any node when doing
     * full relax
     */
    void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) {};

    void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf);

    /**
     * Check feasibility.
     */
    bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation,
                    bool &should_prune, double &inf_meas);

    /**
     * Not implemented yet. Eventually, could add violated linearization
     * inequalities for underestimator portion
     */
    void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                  CutManager *cutman, SolutionPoolPtr s_pool,
                  ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                  SeparationStatus *status);


    /**
     * Create a relaxation by adding the secant inequality for the upper
     * estimator, and some number of linearization inequalities, at a minimum
     * from the end points
     */
    virtual void relaxNodeFull(NodePtr /* node */, RelaxationPtr /* rel */,
                               bool * /* should_prune */) {};

    /**
     * Create a relaxation by updating the secant inequality for the upper
     * estimator, and adding lineariations at the end points, if they are new
     */
    virtual void relaxNodeInc(NodePtr node, RelaxationPtr rel,
                              bool *isInfeasible);

    // base class method
    virtual void getBranchingCandidates(RelaxationPtr rel,
                                        const DoubleVector &x,
                                        ModVector &mods, BrVarCandSet &cands,
                                        BrCandVector &gencands, bool &is_inf);

    // Implement Handler::getBrMod().
    virtual ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x,
                                     RelaxationPtr rel, BranchDirection dir);

    // Implement Handler::getBranches().
    virtual Branches getBranches(BrCandPtr cand, DoubleVector &x,
                                 RelaxationPtr rel, SolutionPoolPtr s_pool);

    // presolve.
    virtual SolveStatus presolve(PreModQ *pre_mods, bool *changed,
                                 Solution **sol);

    // Implement Handler::presolveNode().
    virtual bool presolveNode(RelaxationPtr p, NodePtr node,
                              SolutionPoolPtr s_pool, ModVector &p_mods,
                              ModVector &r_mods);
    // Write name
    virtual std::string getName() const;

    //base class method
    void writeStats(std::ostream &out) const;


  private:
    /// Store statistics of presolving.

    struct LogPresolveStats
    {
      UInt iters;   // number of presolve calls
      double time;  // total time in presolve (optional)
      UInt conDel;  // log constraints deleted
      UInt vBnd;    // variable bounds tightened
      UInt nMods;   // other modifications

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
      int niters;     ///> Number of iterations
      int nLP;        ///> Number of LP solved
      int dlb;        ///> Number of variables for which default lb was added
      int dub;        ///> Number of variables for which default ub was added
      double timeLP;  ///> Time taken in solving LPs
    };

    /// Absolute feasibility tolerance
    double aTol_;

    /// Statistics about bound tightening
    BoundTighteningStats bStats_;

    /// Update bounds if they can be improved by at least this quantity
    double bTol_;

    /// Lower bound to be used for a variable if presolve does not
    /// give us a finite lower bound
    double defaultLb_;

    /// Upper bound to be used for a variable if presolve does not
    /// give us a finite upper bound
    double defaultUb_;

    EnvPtr env_;


    /// Original problem (not the transformed)
    ProblemPtr orig_;

    /// if true only then tightenQuad_ will be called every node
    bool doQT_;

    /// Optional cuts that can be removed
    ConstraintVector optCuts_;

    /// Statistics about presolve
    LogPresolveStats pStats_;


    /// Relative feasibility tolerance
    double rTol_;


    //bound propagation for presolve 1
    bool propLogBnds_(LogConsPtr consd_, bool *changed);


    // Helper functions
    BranchPtr doBranch_(BranchDirection UpOrDown, ConstVariablePtr v,
                        double bvalue);

    //identify duplicate rows and delete one.returns false if no identical
    //constraint
    void dupRows_(bool *changed);


    //deleting the identical constraint and updating the tighter bound
    bool treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double mult,
                       bool *changed);

    // Similar as QuadHandler::tightenSimple_:
    // return true  -> problem detected infeasible
    // return false -> no infeasibility, maybe tightened bounds
    // *changed set to true if any var bound improved

    bool tightenSimple_(bool *changed);

    // Update bounds on variable p with new [newlb, newub]
    // Returns: -1 if infeasible, 0 otherwise
    int updatePBnds_(VariablePtr p, double newlb, double newub,
                     bool *changed);

    //iterates over all log constraint return true if infeasibility detetcted
    bool varBndsFromCons_(bool *changed);
  };

}  //namespace Minotaur
#endif
