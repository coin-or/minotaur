
//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file UnivarQuadHandler.h
 * \brief Declare a handler for quadratic constraints of the form
 * \f$ y = ax^2 + bx \f$,
 * \f$ y = (x_1+x_2)^2 \f$,
 * and
 * \f$ y = (x_1-x_2)^2 \f$,
 * It does not handle any other quadratic constraints.
 * \author Mustafa Vora, IIT Bombay
 */

#ifndef MINOTAURUNIVARQUADHANDLER_H
#define MINOTAURUNIVARQUADHANDLER_H

#include "Handler.h"

namespace Minotaur {

class LinearFunction;
class Timer;
typedef LinearFunction* LinearFunctionPtr;

/**
 * \brief A structure to save information about constraints of the form \f$ y
 * = ax^2 + bx \f$.
 */
struct LinUnivar {
  VariablePtr   y;     ///> The variable y.
  VariablePtr   x;     ///> The variable x.
  double a;            ///> Coefficient of x^2.
  double b;            ///> Coefficient of x.
  ConstraintPtr oeCon; ///> The linear constraint that gives the over estimator
  ConstraintPtr tan1;  ///> The linear underestimator at the lower bound of x
  ConstraintPtr tan2;  ///> The linear underestimator at the upper bound of x
};
typedef LinUnivar* LinUnivarPtr;                 ///> Pointer to LinUnivar
typedef std::vector<LinUnivarPtr> LinUnivarVec;  ///> Vector of LinUnivar
typedef LinUnivarVec::iterator LinUnivarVecIter; ///> Iterator for LinUnivar

/**
 * \brief A structure to save information about constraints of the form \f$ y
 * = (x_1 + x_2) \f$ or \f$ y = (x_1 - x_2) \f$.
 */
struct LinBivar {
  VariablePtr y;          ///> The variable y.
  VariablePtr x1;         ///> The variable x_1.
  VariablePtr x2;         ///> The variable x_2.
  bool pos;               ///> Sign between x_1 and x_2. If true then + else -.
  ConstraintPtr oeCon1;   ///> The linear constraint that gives the over estimator
  ConstraintPtr oeCon2;   ///> The linear constraint that gives the over estimator
  ConstraintPtr tan1;     ///> The linear underestimator at (l1, l2)
  ConstraintPtr tan2;     ///> The linear underestimator at (l1, u2)
  ConstraintPtr tan3;     ///> The linear underestimator at (u1, l2)
  ConstraintPtr tan4;     ///> The linear underestimator at (u1, u2)
};
typedef LinBivar* LinBivarPtr;                 ///> Pointer to LinBivar
typedef std::vector<LinBivarPtr> LinBivarVec;  ///> Vector of LinBivar
typedef LinBivarVec::iterator LinBivarVecIter; ///> Iterator for LinBivar

/// Map of 'x' and the LinSqr that is used for \f$ y = x^2 \f$.
//typedef std::map<VariablePtr, LinSqrPtr, CompareVariablePtr> LinSqrMap;
//typedef LinSqrMap::iterator LinSqrMapIter; ///> Iterator for LinSqrMap

class UnivarQuadHandler : public Handler {
public:
  /// Default constructor.
  UnivarQuadHandler(EnvPtr env, ProblemPtr problem);

  /// Destroy
  ~UnivarQuadHandler();

  // base class method
  void addConstraint(ConstraintPtr newcon);

  // Implement Handler::getBranches().
  Branches getBranches(BrCandPtr cand, DoubleVector & x,
                       RelaxationPtr rel, SolutionPoolPtr s_pool);

  // base class method
  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                              ModVector &mods, BrVarCandSet &cands, 
                              BrCandVector &gencands, bool &is_inf);

  double getBranchingPt_(double vio, double lb, double ub, double allowed_vio);

  // base class method
  ModificationPtr getBrMod(BrCandPtr , DoubleVector &, 
                           RelaxationPtr , BranchDirection )
  { return 0; };

  // base class method
  std::string getName() const;

  // base class method.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool &should_prune, double &inf_meas);

  // base class method.
  SolveStatus presolve(PreModQ *pre_mods, bool *changed, Solution **);

  // base class method. Tightens bounds.
  bool presolveNode(RelaxationPtr p, NodePtr node,
                    SolutionPoolPtr s_pool, ModVector &p_mods,
                    ModVector &r_mods);

  // base class method. Adds linear inequalities
  void relaxInitFull(RelaxationPtr rel, SolutionPool *sp, bool *is_inf);

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, SolutionPool *sp, bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf);


  // base class method. Adds linearlization cuts when available.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, ModVector &p_mods,
                ModVector &r_mods, bool *sol_found, SeparationStatus *status);

  // base class method. 
  void writeStats(std::ostream &out) const;

private:
  /// Store statistics of presolving.
  struct SepaStats 
  {
    int iters;   ///> Number of times separation routine called. 
    int cuts;    ///> Number of cuts added.
    double time; ///> Total time used in separation
  };

  /// Store statistics of presolving.
  struct PresolveStats 
  {
    int iters;   ///> Number of iterations (main cycle).
    double time; ///> Total time used in initial presolve.
    double timeN;///> Total time used in presolveNode.
    int vBnd;    ///> Number of times variable-bounds were tightened.
    int nMods;   ///> Number of changes made in all nodes.
  };

  /// Absolute feasibility tolerance
  double aTol_;

  /// Update bounds if they can be improved by at least this quantity
  double bTol_;

  /**
   * \brief Container for all functions of LinBivar. This should contain
   * variables and constraints of the problem, not the relaxation.
   */
  LinBivarVec bivarFuns_;

  /// Logger.
  LoggerPtr logger_;
      
  /// For printing messages.
  static const std::string me_;

  /// Transformed problem (not the relaxation).
  ProblemPtr p_;

  /// Statistics about presolve
  PresolveStats pStats_;

  /// Relative feasibility tolerance
  double rTol_;

  /// Statistics about separation
  SepaStats sStats_;

  /// Keep track of time
  const Timer* timer_;

  /**
   * \brief Container for all functions of Linunivar. This should contain
   * variables and constraints of the problem, not the relaxation.
   */
  LinUnivarVec univarFuns_;

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
  void addCut_(RelaxationPtr rel, LinUnivarPtr luv, double xl, double xval,
               double yval, bool &ifcuts);

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
  void addCut_(RelaxationPtr rel, LinBivarPtr lbv, double x1l, double x2l,
               double x1val, double x2val, double yval, bool &ifcuts);

  /**
   * \brief Backward propagate the bounds of LinBivar term to the x variables.
   * \param[in] lbv The bivar term
   * \param[out] changed True if any bounds of x1 or x2 have changed. False
   * otherwise.
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool bwdPropBivar_(LinBivarPtr lbv, bool *changed);
  
  /**
   * \brief Backward propagate the bounds of LinUnivar term to the x variable.
   * \param[in] lbv The bivariate term
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
  bool bwdPropBivar_(LinBivarPtr lbv, RelaxationPtr rel, bool mod_rel,
                    bool *changed, ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Backward propagate the bounds of LinUnivar term to the x variable.
   * \param[in] luv The univar term
   * \param[out] changed True if any bounds of x have changed. False
   * otherwise.
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool bwdPropUnivar_(LinUnivarPtr luv, bool *changed);
  
  /**
   * \brief Backward propagate the bounds of LinUnivar term to the x variable.
   * \param[in] luv The univariate term
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
  bool bwdPropUnivar_(LinUnivarPtr luv, RelaxationPtr rel, bool mod_rel,
                    bool *changed, ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Calculate the upper bound of a univariate quadratic term
   * of the form ax^2 + bx.
   * \param[in] a The coefficient of the square term.
   * \param[in] b The coefficient of the linear term.
   * \param[in] lx Lower bound of variable x.
   * \param[in] ux Upper bound of variable x.
   * Return the upper bound of the univariate quadratic.
   */
  double calcUpperUnivar_(double a, double b, double lx, double ux);
  
  /**
   * \brief Find the point at which a gradient-based linearization inequality
   * can be added.
   * \param[in] xval x coordinate of point that we want to cut off
   * \param[in] yval y coordinate of point that we want to cut off
   * \param[in] luv the univariate function
   * \param[in] xl x coordinate of point at which gradient can be evaluated
   * \param[in] yl y coordinate of point at which gradient can be evaluated
   */
  void findLinPt_(double xval, double yval, LinUnivarPtr luv, double &xl);
  
  /**
   * \brief Forward propagate the bounds of LinBivar term to the y variable.
   * \param[in] lbv The bivar term
   * \param[out] changed True if any bounds of y have changed. False
   * otherwise.
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool fwdPropBivar_(LinBivarPtr lbv, bool *changed);

  /**
   * \brief Forward propagate the bounds of LinBivar term to the y variable.
   * \param[in] lbv The univariate term
   * \param[in] rel The relaxation that is currently being solved.
   * \param[in] mod_rel If true, then change the relaxation also. The original
   * (or transformed) problem p_ is always modified.
   * \param[out] changed True if any bounds of y have changed. False
   * otherwise.
   * \param[in] p_mods A vector to save the modifications to the problem 
   * \param[in] r_mods A vector to save the modifications to the relaxation 
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool fwdPropBivar_(LinBivarPtr lbv, RelaxationPtr rel, bool mod_rel,
                    bool *changed, ModVector &p_mods, ModVector &r_mods);
  
  /**
   * \brief Forward propagate the bounds of LinUnivar term to the y variable.
   * \param[in] luv The bivar term
   * \param[out] changed True if any bounds of y have changed. False
   * otherwise.
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool fwdPropUnivar_(LinUnivarPtr luv, bool *changed);

  /**
   * \brief Forward propagate the bounds of LinUnivar term to the y variable.
   * \param[in] luv The univariate term
   * \param[in] rel The relaxation that is currently being solved.
   * \param[in] mod_rel If true, then change the relaxation also. The original
   * (or transformed) problem p_ is always modified.
   * \param[out] changed True if any bounds of y have changed. False
   * otherwise.
   * \param[in] p_mods A vector to save the modifications to the problem 
   * \param[in] r_mods A vector to save the modifications to the relaxation 
   * Return true if the new bounds are inconsistent (i.e., lb > ub for any of
   * the two variables)
   */
  bool fwdPropUnivar_(LinUnivarPtr luv, RelaxationPtr rel, bool mod_rel,
                    bool *changed, ModVector &p_mods, ModVector &r_mods);

  /**
   * \brief Get the secant of the univariate quadratic
   * param[in] y The variable y of the relaxation.
   * param[in] x The variable x of the relaxation.
   * param[in] a The coefficient of x^2 in the quadratic.
   * param[in] b The coefficient of x in the quadratic.
   */
  LinearFunctionPtr getSecant_(VariablePtr y, VariablePtr x,
                               double a, double b);

  /**
   * \brief Get the secant of the bivariate quadratic
   * param[in] y The variable y of the relaxation.
   * param[in] x1 The variable x1 of the relaxation.
   * param[in] x2 The variable x2 of the relaxation.
   * param[in] pos The sign between x1 and x2
   * param[out] rhs The rhs values of the secants
   */
  std::vector<LinearFunctionPtr> getSecant_(VariablePtr y, VariablePtr x1,
                VariablePtr x2, bool pos, DoubleVector &rhs);
  
  /**
   * \brief Get the tangent of the univariate quadratic at the given point
   * \param[in] y The variable y of the relaxation.
   * \param[in] x The variable x of the relaxation.
   * \param[in] a The coefficient of x^2 in the quadratic.
   * \param[in] b The coefficient of x in the quadratic.
   * \param[in] pt The point at which tangent should be added
   */
  LinearFunctionPtr getTangent_(VariablePtr y, VariablePtr x,
                               double a, double b, double pt);

  /**
   * \brief Get the tangent of the bivariate quadratic at the given point
   * \param[in] y The variable y of the relaxation.
   * \param[in] x1 The variable x1 of the relaxation.
   * \param[in] x2 The variable x2 of the relaxation.
   * \param[in] pos The sign between x1 and x2.
   * \param[in] i The tangent number to be returned
   * \param[out] rhs The rhs value of the tangent
   */
  LinearFunctionPtr getTangent_(VariablePtr y, VariablePtr x1, VariablePtr x2,
                               bool pos, UInt i, double &rhs);

  /// Return true if xval is one of the bounds of variable x
  bool isAtBnds_(ConstVariablePtr x, double xval);

  /**
   * \brief Check whether secants are violated after bound changes in a bivar
   * \param[in] c1 The first secant.
   * \param[in] c2 The second secant.
   * \param[in] x1 Variable x1 in bivar.
   * \param[in] x2 Variable x2 in bivar.
   * \param[in] l1 Lower bound of x1.
   * \param[in] u1 Upper bound of x1.
   * \param[in] l2 Lower bound of x2.
   * \param[in] u2 Upper bound of x2.
   * \param[in] pos sign between x1 and x2
   */
  bool isViolatedAtBounds_(ConstraintPtr c1, ConstraintPtr c2, VariablePtr x1,
                      VariablePtr x2, double l1, double u1, double l2,
                      double u2, bool pos);

  /**
   * \brief Relax all the constraints
   * \param[in] rel The relaxation to which the new linear inequalities are
   * added.
   * \param[out] is_inf True if the relaxation is detected to be infeasible
   * (because of inconsistent bounds or other reasons).
   */
  void relax_(RelaxationPtr rel, bool *is_inf);

  /// Reset all statistics to zero.
  void resetStats_();

  /**
   * \brief Update linear relaxation of the bivar constraints after some
   * bounds have changed. The function checks whether both of the
   * constraints are binding at the required extreme points of the box. If
   * not, the constraints are updated.
   * \param[in] lbv The bivar term
   * \param[in] rel The relaxation which contains the four linear constraints
   * \param[in] r_mods A vector into which the modifications are appended so
   * that they can be reverted at a later time.
   */
  void upBivarCon_(LinBivarPtr lbv, RelaxationPtr rel, ModVector &r_mods);

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
   * \brief Update linear relaxation of the univar constraints after
   * some bounds have changed. The function checks whether the upper bounding
   * constraint is binding at the required extreme points of the box. If not,
   * the constraint is updated.
   * \param[in] con The linear constraint that can be updated
   * \param[in] x The variable x. 
   * \param[in] y The variable y.
   * \param[in] a The coefficient of x^2
   * \param[in] b The coeficient of x
   * \param[in] rel The relaxation which contains the linear constraint
   * \param[in] r_mods A vector into which the modifications are appended so
   * that they can be reverted at a later time.
   */
  void upUnivarCon_(ConstraintPtr con, VariablePtr x, VariablePtr y, double a, 
                    double b, RelaxationPtr rel, ModVector &r_mods);

  /**
   * \brief Tighten the bounds on variables of the original (or transformed)
   * problem on the basis of the quadratic constraints y=x0^2 or y=x0x1.
   * Usually called in the initial presolve.
   * \param[out] changed True if any bounds are strengthened
   * \return true if some bounds become inconsistent, making the problem
   * infeasible. Flase otherwise.
   */
  bool varBndsFromCons_(bool *changed);
};

/// Shared pointer to UnivarQuadHandler.
typedef UnivarQuadHandler* UnivarQuadHandlerPtr;
}
#endif

