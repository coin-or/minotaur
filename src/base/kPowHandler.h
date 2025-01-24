//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file kPowHandler.h
 * \brief Declare a handler for simple Quadratic constraints of the form
 * \f$ y_1 = x_1x_2 \f$,
 * and
 * \f$ y_1 = x_1^2 \f$,
 * It does not handle any other Quadratic constraints.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURQuadraticHANDLER_H
#define MINOTAURQuadraticHANDLER_H

#include "CGraph.h"
#include "Handler.h"

namespace Minotaur {

class LinearFunction;
class Timer;
class LPEngine;
typedef LPEngine *LPEnginePtr;
typedef LinearFunction *LinearFunctionPtr;
typedef Engine *EnginePtr;

/**
 * \brief A structure to save information about constraints of the form \f$ y
 * \leq x^2 \f$.
 */
struct LinkPow {
  VariablePtr y;        ///> The variable y.
  VariablePtr x;        ///> The variable x.
  double k;             ///> The power k
  ConstraintPtr oeCon;  ///> The linear constraint that gives the over estimator
};
typedef LinkPow *LinkPowPtr;                  ///> Pointer to LinkPow
typedef std::vector<LinkPowPtr> LinkPowVec;   ///> Vector of LinkPow
typedef LinkPowVec::iterator LinkPowVecIter;  ///> Iterator for LinkPow

/// Map of 'x' and the LinkPow that is used for \f$ y = x^2 \f$.
typedef std::map<VariablePtr, LinkPowPtr, CompareVariablePtr> LinkPowMap;
typedef LinkPowMap::iterator LinkPowMapIter;  ///> Iterator for LinkPowMap

/**
 * A kPowHandler handles the Quadratic functions of a problem in a simplistic
 * fashion. For now, we will just handle squares of singleton variables e.g.
 * \f$x_1^2 = y_1\f$, and bilinear terms: \f$x_1x_2 = y_1\f$.
 */
class kPowHandler : public Handler {
 public:
  /// Default constructor.
  // kPowHandler(EnvPtr env, ProblemPtr problem);

  kPowHandler(EnvPtr env, ProblemPtr problem, ProblemPtr orig_p);

  /// Destroy
  ~kPowHandler();

  // base class method
  void addConstraint(ConstraintPtr newcon);

  // Implement Handler::getBranches().
  Branches getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel,
                       SolutionPoolPtr s_pool);

  // base class method
  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                              ModVector &mods, BrVarCandSet &cands,
                              BrCandVector &gencands, bool &is_inf);

  // base class method
  ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel,
                           BranchDirection dir);

  // base class method
  std::string getName() const;

  // base class method.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation,
                  bool &should_prune, double &inf_meas);

  // base class method.
  SolveStatus presolve(PreModQ *pre_mods, bool *changed, Solution **sol);

  // base class method. Tightens bounds.
  bool presolveNode(RelaxationPtr p, NodePtr node, SolutionPoolPtr s_pool,
                    ModVector &p_mods, ModVector &r_mods);

  // implement Handler::postSolveRootNode
  bool postSolveRootNode(RelaxationPtr rel, SolutionPoolPtr s_pool,
                         ConstSolutionPtr sol, ModVector &p_mods,
                         ModVector &r_mods);

  // implement Handler::fixNodeErr
  int fixNodeErr(RelaxationPtr rel, ConstSolutionPtr sol,
                 SolutionPoolPtr s_pool, bool &sol_found);

  // base class method. Adds linear inequalities
  void relaxInitFull(RelaxationPtr rel, SolutionPool *, bool *is_inf);

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // base class method. Adds linearlization cuts when available.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                CutManager *cutman, SolutionPoolPtr s_pool, ModVector &p_mods,
                ModVector &r_mods, bool *sol_found, SeparationStatus *status);

  void setBTEngine(LPEnginePtr engine);

  void setNLPEngine(EnginePtr engine);

  // base class method.
  void writeStats(std::ostream &out) const;

 private:
  /// Store statistics of presolving.
  struct SepaStats {
    int iters;    ///> Number of times separation routine called.
    int cuts;     ///> Number of cuts added.
    double time;  ///> Total time used in separation
  };

  /// Store statistics of presolving.
  struct PresolveStats {
    int iters;     ///> Number of iterations (main cycle).
    double time;   ///> Total time used in initial presolve.
    double timeN;  ///> Total time used in presolveNode.
    int vBnd;      ///> Number of times variable-bounds were tightened.
    int nMods;     ///> Number of changes made in all nodes.
  };

  struct NLPStats {
    bool flag;        ///> if true that means NLP was solved at least once
    UInt nlp;         ///> Number of NLPs solved
    UInt opt;         ///> Number of NLPs optimal
    UInt inf;         ///> Number of NLPs infeasible
    UInt iter_limit;  ///> Number of NLPs for which EngineIterationLimit
  };

  struct BoundTighteningStats {
    int niters;  ///> Number of iterations
    // VariableSet qvars; ///> The variables with Quadratic terms
    // int nqlb;          ///> Number of Quadratic variables with finite lb
    // int nqub;          ///> Number of Quadratic variables with finite ub
    // int nqlbs;         ///> Number of Quadratic variables whose lb was
    //                   ///> found by simple tightening
    // int nqubs;         ///> Number of Quadratic variables whose ub was
    //                   ///> found by simple tightening
    // int vBnds;         ///> Number of times bounds tightened by
    //                   ///> simple tightening
    // int cBnds;         ///> Number of times cons tightened by
    //                   ///> simple tightening
    // int nqlbq;         ///> Number of Quadratic variables whose lb was
    //                   ///> found by kPow tightening
    // int nqubq;         ///> Number of Quadratic variables whose ub was
    //                   ///> found by kPow tightening
    // int vBndq;         ///> Number of times bounds tightened by
    //                   ///> kPow tightening
    // int cBndq;         ///> Number of times cons tightened by
    //                   ///> kPow tightening
    // int nqlbl;         ///> Number of Quadratic variables whose lb was
    //                   ///> found by lp tightening
    // int nqubl;         ///> Number of Quadratic variables whose ub was
    //                   ///> found by lp tightening
    // int vBndl;         ///> Number of times bounds tightened by
    //                   ///> lp tightening
    int nLP;        ///> Number of LP solved
    int dlb;        ///> Number of variables for which default lb was added
    int dub;        ///> Number of variables for which default ub was added
    double timeLP;  ///> Time taken in solving LPs
    // double avg_range;  ///> Average range of bounds of Quadratic variables
    // double sd_range;   ///> Standard deviation of range
    // double body_diag;  ///> The length of body diagonal of hypercube formed
    // by
    //                   ///> the range of Quadratic variables
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

  /// Logger.
  LoggerPtr logger_;

  /// Bound tightening LP engine
  LPEnginePtr bte_;

  /// NLP engine
  EnginePtr nlpe_;

  /// NLP stats
  NLPStats nlpStats_;

  /// For printing messages.
  static const std::string me_;

  /// Original problem (not the transformed)
  ProblemPtr orig_;

  /// Transformed problem (not the relaxation).
  ProblemPtr p_;

  /// Statistics about presolve
  PresolveStats pStats_;

  /// if true only then tightenkPow_ will be called every node
  bool doQT_;

  /// Relative feasibility tolerance
  double rTol_;

  /// Statistics about separation
  SepaStats sStats_;

  /// Keep track of time
  const Timer *timer_;

  /**
   * \brief Container for all square functions. This should contain variables
   * and constraints of the problem, not the relaxation.
   */
  LinkPowMap xkFuns_;

  /**
   * \brief Add a gradient-based linearization inequality.
   * \param[in] x       The variable x in (y = x^2)
   * \param[in] y       The variable y in (y = x^2)
   * \param[in] xl      x coordinate at which the gradient is evaluated
   * \param[in] yl      y coordinate at which the gradient is evaluated
   * \param[in] xval    x coordinate of point that is to be cut off
   * \param[in] yval    y coordinate of point that is to be cut off
   * \param[in] rel     Relaxation pointer to which the cut is added
   * \param[out] ifcuts True if the new inequality cuts off the point
   *                    (xval,yval)
   */
  void addCut_(VariablePtr x, VariablePtr y, double xl, double yl, double xval,
               double yval, RelaxationPtr rel, bool &ifcuts, double k);

  /// This will add default variable bounds if finite bounds
  /// are not found by presolve
  double addDefaultBounds_(VariablePtr x, BoundType lu);

  /**
   * \brief Calculate the upper bound of a univariate Quadratic of the form
   * ax^2 + bx.
   * \param[in] a The coefficient of square term
   * \param[in] b The coefficient of linear term
   * \param[in] lx The lower bound of the variable
   * \param[in] ux The upper bound of the variable
   */
  double calcUpperUnivar_(double a, double b, double lx, double ux);

  /**
   * \brief Calculate bounds of the variable from a linear term bounds
   * \param[in] v The variable in the linear term
   * \param[in] coeff The coefficient of the linear term
   * \param[in] lb The lower bound of the linear term
   * \param[in] ub The upper bound of the linear term
   * \param[out] c1 true if the bounds on the variable are changed
   */
  bool calcVarBnd_(VariablePtr v, double coef, double lb, double ub, bool *c1);

  bool calcVarBnd_(RelaxationPtr rel, VariablePtr v, double coef, double lb,
                   double ub, bool *c1, ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Calculate bounds of the variables from a Quadratic term bounds
   * \param[in] v1 The first variable in the Quadratic term
   * \param[in] v2 The second variable in the Quadratic term
   * \param[in] coeff The coefficient of the Quadratic term
   * \param[in] lb The lower bound of the Quadratic term
   * \param[in] ub The upper bound of the Quadratic term
   * \param[out] c1 true if the bounds on first variable are changed
   * \param[out] c2 true if the bounds on second variable are changed
   */
  bool calcVarBnd_(VariablePtr v1, VariablePtr v2, double coef, double lb,
                   double ub, bool *c1, bool *c2);

  bool calcVarBnd_(RelaxationPtr rel, VariablePtr v1, VariablePtr v2,
                   double coef, double lb, double ub, bool *c1, bool *c2,
                   ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Calculate bounds of the variables from a univariate Quadratic
   * term bounds
   * \param[in] v The variable in the term
   * \param[in] a The coefficient of the kPowrtic term
   * \param[in] b The coefficient of the liner term
   * \param[in] ly The lower bound of the term
   * \param[in] uy The upper bound of the term
   * \param[out] changed true if the bounds on the variables are changed
   */
  bool calcVarBnd_(VariablePtr v, double a, double b, double ly, double uy,
                   bool *c1);

  bool calcVarBnd_(RelaxationPtr rel, VariablePtr v, double a, double b,
                   double ly, double uy, bool *c1, ModVector &p_mods,
                   ModVector &r_mods);

  void coeffImprov_();

  /**
   * \brief Find the point at which a gradient-based linearization inequality
   * can be added.
   * \param[in] xval x coordinate of point that we want to cut off
   * \param[in] yval y coordinate of point that we want to cut off
   * \param[in] xl x coordinate of point at which gradient can be evaluated
   * \param[in] yl y coordinate of point at which gradient can be evaluated
   */
  void findLinPt_(double xval, double yval, double &xl, double &yl, double k);

  /**
   * \brief Solve the lp and get the bounds of the variable.
   * Called by tightenLP_.
   * \param[in] e The engine where lp is loaded
   * \param[out] is_inf True if the lp is infeasible.
   */
  double getBndByLP_(bool &is_inf);

  /**
   * \brief Get bounds of a lf of a constraint
   * \param[in] qf Quadratic Function
   * \param[out] implLb the implied lower bound of the lf
   * \param[out] implUb the implied upper bound of the lf
   * \param[out] fwdLb vector of lower bounds for each term
   * \param[out] fwdUb vector of upper bounds for each term
   * \param[out] count_inf_lb # of infinities in lower bound
   * \param[out] count_inf_ub # of infinities in upper bound
   */
  void getLfBnds_(LinearFunctionPtr lf, double &implLb, double &implUb,
                  DoubleVector &fwdLb, DoubleVector &fwdUb, UInt &count_inf_lb,
                  UInt &count_inf_ub);

  /**
   * \brief Get one of the three linear functions and right hand sides for the
   * linear relaxation of a k power constraint y = x^k.
   * \param[in] y The y variable
   * \param[in] x The x variable
   * \param[in] k The k(>1) power
   * \param[in] lb The lower bound of x
   * \param[in] ub The upper bound of x
   * \param[out] rhs The rhs (upperbound) of the linear constraint
   * \return A linear function such that lf <= rhs is a relaxation of the
   * power constraint.
   * \param[in] type Could be 0,1,2. It tells the function which (out of the
   * four) linear constraints do we want.
   */
  LinearFunctionPtr getNewKpowLf_(VariablePtr y, VariablePtr x, double k,
                                  double lb, double ub, double &rhs, int type);

  /**
   * \brief Get bounds of a qf of a constraint
   * \param[in] qf Quadratic Function
   * \param[out] implLb the implied lower bound of the qf
   * \param[out] implUb the implied upper bound of the qf
   * \param[out] fwdLb vector of lower bounds for each term
   * \param[out] fwdUb vector of upper bounds for each term
   * \param[out] count_inf_lb # of infinities in lower bound
   * \param[out] count_inf_ub # of infinities in upper bound
   */
  void getQfBnds_(QuadraticFunctionPtr qf, double &implLb, double &implUb,
                  DoubleVector &fwdLb, DoubleVector &fwdUb, UInt &count_inf_lb,
                  UInt &count_inf_ub);

  /**
   * \brief Get bounds of a qf and lf of a constraint
   * \param[in] lf Linear Function
   * \param[in] qf Quadratic Function
   * \param[out] implLb the implied lower bound of the lf and qf
   * \param[out] implUb the implied upper bound of the lf and qf
   * \param[out] fwdLb vector of lower bounds for each term
   * \param[out] fwdUb vector of upper bounds for each term
   * \param[out] count_inf_lb # of infinities in lower bound
   * \param[out] count_inf_ub # of infinities in upper bound
   * \param[out] qvars the variables for which there is a term like ax^2 + b
   * return false if qvars has no element, true otherwise
   */
  bool getQfLfBnds_(LinearFunctionPtr lf, QuadraticFunctionPtr qf,
                    double &implLb, double &implUb, DoubleVector &fwdLb,
                    DoubleVector &fwdUb, UInt &count_inf_lb, UInt &count_inf_ub,
                    VarVector &qvars);

  bool getQfLfBnds_(RelaxationPtr rel, LinearFunctionPtr lf,
                    QuadraticFunctionPtr qf, double &implLb, double &implUb,
                    DoubleVector &fwdLb, DoubleVector &fwdUb,
                    UInt &count_inf_lb, UInt &count_inf_ub, VarVector &qvars);

  /**
   * \brief Calculate sum of a vector except current element
   * \param[in] b begin iterator of the vector
   * \param[in] e end iterator of the vector
   * \param[in] curr current element's iterator of the vector
   * \param[in] bt Lower bound or Upper bound
   * \param[in] bound Implied bound of the vector
   * \param[in] inf_count count of infinities in the vector
   * return the sum of the vector except current element
   */
  double getSumExcept1_(DoubleVector::iterator b, DoubleVector::iterator e,
                        DoubleVector::iterator curr, BoundType bt, double bound,
                        UInt inf_count);

  /**
   * \brief Calculate bounds of a linear term from the variable bounds
   * \param[in] v The variable in the linear term
   * \param[in] coeff The coefficient of the linear term
   * \param[out] lb The calculated lower bound of the linear term
   * \param[out] ub The calculated upper bound of the linear term
   */
  void getTermBnds_(VariablePtr v, double coef, double &lb, double &ub);

  /**
   * \brief Calculate bounds of a Quadratic term from the variable bounds
   * \param[in] v1 The first variable in the Quadratic term
   * \param[in] v2 The second variable in the Quadratic term
   * \param[in] coeff The coefficient of the Quadratic term
   * \param[out] lb The calculated lower bound of the Quadratic term
   * \param[out] ub The calculated upper bound of the Quadratic term
   */
  void getTermBnds_(VariablePtr v1, VariablePtr v2, double coef, double &lb,
                    double &ub);

  /**
   * \brief Calculate bounds of a univariate Quadratic term of the
   * form ax^2 + bx from the variable bounds
   * \param[in] v The variable in the term
   * \param[in] a The coefficient of the square term
   * \param[in] b The coefficient of the linear term
   * \param[out] lb The calculated lower bound of the term
   * \param[out] ub The calculated upper bound of the term
   */
  void getTermBnds_(VariablePtr v, double a, double b, double &lb, double &ub);

  /// Return true if xval is one of the bounds of variable x
  bool isAtBnds_(ConstVariablePtr x, double xval);

  /// Whether a given point is feasible to the relaxation
  bool isFeasibleToRelaxation_(RelaxationPtr rel, const double *x);

  /**
   * \brief Strengthen bounds of variables in a square constraint y=x0^2.
   * \param[in] lx2 The sqaure term
   * \param[out] changed True if any bounds of y or x0 have changed. False
   * otherwise.
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool propKPowBnds_(LinkPowMapIter lxk, bool *changed);

  /**
   * \brief Strengthen bounds of variables in a square constraint y=x0^2,
   * and save the modifications if any.
   * \param[in] lx2 The square term
   * \param[in] rel The relaxation that is currently being solved.
   * \param[in] mod_rel If true, then change the relaxation also. The original
   * (or transformed) problem p_ is always modified.
   * \param[out] changed True if any bounds of y or x0 have changed. False
   * otherwise.
   * \param[in] p_mods A vector to save the modifications to the problem
   * \param[in] r_mods A vector to save the modifications to the relaxation
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool propKPowBnds_(LinkPowMapIter lxk, RelaxationPtr rel, bool mod_rel,
                     bool *changed, ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Relax all the square constraints: y=x0^2 and bilinear constraints
   * y=x0x1 in the problem.
   * \param[in] rel The relaxation to which the new linear inequalities are
   * added.
   * \param[out] is_inf True if the relaxation is detected to be infeasible
   * (because of inconsistent bounds or other reasons).
   */

  void relax_(RelaxationPtr rel, bool *is_inf);

  /**
   * \brief Resetting the bounds on original variables
   * \param[in] varlb the vector of original lb
   * \param[in] varub the vector of original ub
   */
  void resetBoundsinOrig_(DoubleVector &varlb, DoubleVector &varub);

  /// Reset all statistics to zero.
  void resetStats_();

  /// Setting Itmp for each variable based on the solution from LP
  void setItmpFromSol_(const double *x);

  /**
   * \brief Bound tightening of the problem by solving LP after removing all
   * the Quadratic components of every constraint and minimizing and
   * maximizing all Quadratic variables. Returns true if the problem is found
   * to be infeasible, false otherwise.
   * \param[in] rel The relaxation ptr.
   * \param[out] changed True is some changes are made in the bounds on any
   * variables.
   */
  bool tightenLP_(RelaxationPtr rel, double bestSol, bool *changed,
                  ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Bound tightening of the problem by considering linear and Quadratic
   * terms simultaneously. Returns true if the problem is found to be
   * infeasible, false otherwise.
   * \pram[out] changed True is some changes are made in the bounds on any
   * variables.
   */
  bool tightenkPow_(bool *changed);

  bool tightenkPow_(RelaxationPtr rel, double bestSol, bool *changed,
                    ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Bound tightening of the problem by using simple interval
   * arithmetic. Returns true if the problem is found to be infeasible,
   * false otherwise.
   * \pram[out] changed True is some changes are made in the bounds on any
   * variables.
   */
  bool tightenSimple_(bool *changed);

  /**
   * \brief Update bounds of variable in original problem as the bounds
   * on the current node.
   * \param[in] rel Relaxation at the node
   * \param[in] x the lp solution at the current node
   * \param[out] varlb vector of lb on variables before bound changes
   * \param[out] varlb vector of ub on variables before bound changes
   */
  void updateBoundsinOrig_(RelaxationPtr rel, const double *x,
                           DoubleVector &varlb, DoubleVector &varub);

  /**
   * \brief Modify bounds of a variable in the problem to the new bounds lb
   * and ub if the new bounds are tighter.
   * \param[in] v The variable
   * \param[in] lb The new lower bound
   * \param[in] ub The new upper bound
   * \param[out] changed True if the new bounds are tighter than the existing
   * bounds. False otherwise.
   * \return the number of bound changes (1 or 2) if any bounds are changed.
   * Returns -1 if the new bounds make the problem infeasible. Returns 0
   * otherwise.
   */
  int updatePBounds_(VariablePtr v, double lb, double ub, bool *changed);

  /**
   * \brief Modify bounds of a variable in the problem to the new bounds lb
   * and ub if the new bounds are tighter. Additionally, save the
   * modifications in the appropriate vectors so that they can be reverted at
   * a later time.
   * \param[in] v The variable
   * \param[in] lb The new lower bound
   * \param[in] ub The new upper bound
   * \param[in] rel The relaxation that is currently being solved
   * \param[in] mod_rel If true, then the relaxation is also modified. The
   * original (or transformed) problem is always modified.
   * \param[out] changed True if the new bounds are tighter than the existing
   * bounds. False otherwise.
   * \param[in] p_mods A vector to save the modifications to the problem
   * \param[in] r_mods A vector to save the modifications to the relaxation
   * \return the number of bound changes (1 or 2) if any bounds are changed.
   * Returns -1 if the new bounds make the problem infeasible. Returns 0
   * otherwise.
   */
  int updatePBounds_(VariablePtr v, double lb, double ub, RelaxationPtr rel,
                     bool mod_rel, bool *changed, ModVector &p_mods,
                     ModVector &r_mods);
  /**
   * \brief After solving an NLP update ub of the problem.
   * \param[in] s_pool Current solution pool
   * \param[in] nlpval NLP objective value
   * \param[out] sol_found true if the NLP solution is better than all
   * solutions in s_pool
   */
  void updateUb_(SolutionPoolPtr s_pool, double nlpval, bool &sol_found);

  /**
   * \brief Update linear relaxation of the square constraints (y=x^2) after
   * some bounds have changed. The function checks whether the upper bounding
   * constraint is binding at the required extreme points of the box. If not,
   * the constraint is updated.
   * \param[in] con The linear constraint that can be updated
   * \param[in] x The variable x.
   * \param[in] y The variable y.
   * \param[in] rel The relaxation which contains the linear constraint
   * \param[in] r_mods A vector into which the modifications are appended so
   * that they can be reverted at a later time.
   */
  void upSqCon_(ConstraintPtr con, VariablePtr x, VariablePtr y, double k,
                RelaxationPtr rel, ModVector &r_mods);

  /**
   * \brief Tighten the bounds on variables of the original (or transformed)
   * problem on the basis of the Quadratic constraints y=x0^2 or y=x0x1.
   * Usually called in the initial presolve.
   * \param[out] changed True if any bounds are strengthened
   * \return true if some bounds become inconsistent, making the problem
   * infeasible. Flase otherwise.
   */
  bool varBndsFromCons_(bool *changed);
};

/// Shared pointer to kPowHandler.
typedef kPowHandler *kPowHandlerPtr;
}  // namespace Minotaur
#endif

