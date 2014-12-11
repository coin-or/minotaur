//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file QuadHandler.h
 * \brief Declare a handler for simple quadratic constraints of the form
 * \f$ y = x_1x_2 \f$,
 * and
 * \f$ y = x_1^2 \f$,
 * It does not handle any other quadratic constraints.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURQUADRATICHANDLER_H
#define MINOTAURQUADRATICHANDLER_H

#include "Handler.h"

namespace Minotaur {

class Function;
class LinearFunction;
class Objective;
class Problem;
class QuadraticFunction;
typedef boost::shared_ptr<Function> FunctionPtr;
typedef boost::shared_ptr<LinearFunction> LinearFunctionPtr;
typedef boost::shared_ptr<Objective> ObjectivePtr;
typedef boost::shared_ptr<const Problem> ConstProblemPtr;
typedef boost::shared_ptr<QuadraticFunction> QuadraticFunctionPtr;

/// Save information about constraints of the form \f$ y \leq x^2 \f$.
struct LinSqr {
  VariablePtr   y;     /// The variable y.
  VariablePtr   x;     /// The variable x.
  ConstraintPtr oeCon; /// The linear constraint that gives the over estimator
};


typedef LinSqr* LinSqrPtr;                 /// Pointer to LinSqr
typedef std::vector<LinSqrPtr> LinSqrVec;  /// Vector for over estimators
typedef LinSqrVec::iterator LinSqrVecIter; /// Vector-iterator for over estimator

/// Map of 'x' and the LinSqr that is used for \f$ y = x^2 \f$.
typedef std::map<VariablePtr, LinSqrPtr, CompareVariablePtr> LinSqrMap;
typedef LinSqrMap::iterator LinSqrMapIter; /// Iterator for LinSqrMap


/**
 * A LinBil object stores some information about LinBil inequalities 
 * for the constraints of the form \f$x_0x_1 = y\f$ 
 */
class LinBil {
private:
  /// First variable.
  VariablePtr x0_;

  /// Second variable.
  VariablePtr x1_;

  /// Auxiliary variable.
  VariablePtr y_;

  /// Constraint 0.
  ConstraintPtr c0_;
        
  /// Constraint 1.
  ConstraintPtr c1_;

  /// Constraint 2.
  ConstraintPtr c2_;

  /// Constraint 3.
  ConstraintPtr c3_;

public:
  /// Default constructor. 
  LinBil(VariablePtr y, VariablePtr x0, VariablePtr x1);

  /// Destroy.
  ~LinBil();

  /// Get one of the four constraints.
  ConstraintPtr getC0() {return c0_;};
  ConstraintPtr getC1() {return c1_;};
  ConstraintPtr getC2() {return c2_;};
  ConstraintPtr getC3() {return c3_;};

  /// Get the auxiliary variable.
  VariablePtr getY() {return y_;};

  /// Get \f$x_0\f$
  VariablePtr getX0() {return x0_;};

  /// Get \f$x_1\f$
  VariablePtr getX1() {return x1_;};

  /// Get the variable other than x, in the product.
  VariablePtr getOtherX(ConstVariablePtr x) const;

  /// Check if a bilinear constraint is violated at the current point x.
  bool isViolated(const double *x, const double &tol) const;

  /**
   * Check if a bilinear constraint is violated for the given values of
   * \f$x_0, x_1, y\f$.
   */
  bool isViolated(const double x0val, const double x1val, 
                  const double y0val, const double tol) const;

  void setCons(ConstraintPtr c0, ConstraintPtr c1, ConstraintPtr c2,
               ConstraintPtr c3);
};

typedef LinBil* LinBilPtr;

/**
 * Compare two LinBil objects. Since we keep them in a set, we need to
 * sort them. We use lexicographic ordering (i.e. based on ids of 
 * \f$(x_0, x_1)\f$).
 */
struct CompareLinBil {
  bool operator() (LinBilPtr b0, LinBilPtr b1) const;
};

/// A set of bilinear objects.
typedef std::set<LinBilPtr, CompareLinBil> LinBilSet;

/// Iterator of LinBil objects over a set.
typedef LinBilSet::iterator LinBilSetIter;


/**
 * An QuadHandler handles the convex parts of quadratic functions of a
 * problem. For now, we will just handle squares of singleton variables e.g.
 * \f$\sum_ix_i^2 \leq u_0\f$. \f$u_0\f$ could be an auxiliary varible or a
 * constant. Later we will introduce sums of squares of
 * general linear functions as well.
 */
class QuadHandler : public Handler {
public:
  /// Default constructor.
  QuadHandler(EnvPtr env, ProblemPtr problem);

  /// Destroy
  ~QuadHandler();

  // base class method
  void addConstraint(ConstraintPtr newcon);

  void findLinPt_(double xval, double yval, double &xl, double &yl);
  // Implement Handler::getBranches().
  Branches getBranches(BrCandPtr cand, DoubleVector & x,
                       RelaxationPtr rel, SolutionPoolPtr s_pool);

  // base class method
  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                              ModVector &mods, BrVarCandSet &cands, 
                              BrCandVector &gencands, bool &is_inf);

  // Implement Handler::getBrMod().
  ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x, 
                           RelaxationPtr rel, BranchDirection dir);

  // Write name
  std::string getName() const;

  // base class method.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool &is_inf);

  // presolve.
  SolveStatus presolve(PreModQ *pre_mods, bool *changed);

  // Implement Handler::presolveNode().
  bool presolveNode(RelaxationPtr p, NodePtr node,
                    SolutionPoolPtr s_pool, ModVector &p_mods,
                    ModVector &r_mods);

  // Does nothing.
  void relaxInitFull(RelaxationPtr rel, bool *is_inf);

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf);


  /**
   * Not implemented yet.
   */
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, bool *sol_found,
                SeparationStatus *status);

private:
  /// Tolerance.
  double eTol_;

  /// Logger.
  LoggerPtr logger_;
      
  /// For printing.
  static const std::string me_;

  /// Transformed problem (not the relaxation).
  ProblemPtr p_;

  /**
   * \brief Container for all bilinear functions. This should contain
   * variables and constraints of the problem, not the relaxation.
   */
  LinBilSet x0x1Funs_;

  /**
   * \brief Container for all square functions. This should contain variables and
   * constraints of the problem, not the relaxation.
   */
  LinSqrMap x2Funs_;

  void addCut_(VariablePtr x, VariablePtr y, double xl, double yl, double xval,
               double yval, RelaxationPtr rel, bool &ifcuts);

  void binToLin_();

  void binToLinFun_(FunctionPtr f, LinearFunctionPtr lf2);

  /// Get linear function and right hand side (r) for the linear overestimator
  /// constraint of bilinear type.
  LinearFunctionPtr getNewBilLf_(VariablePtr x0, double lb0, double ub0,
                                 VariablePtr x1, double lb1, double ub1,
                                 VariablePtr y, int type, double &rhs);

  /// Get linear function and right hand side (r) for the linear overestimator
  /// constraint for the square type.
  LinearFunctionPtr getNewSqLf_(VariablePtr x, VariablePtr y, 
                                double lb, double ub, double & r);


  bool propBilBnds_(LinBilPtr, bool *changed);

  bool propBilBnds_(LinBilPtr lx0x1, RelaxationPtr rel, bool mod_rel,
                    bool *changed, ModVector &p_mods, ModVector &r_mods);

  bool propSqrBnds_(LinSqrMapIter lx2, bool *changed);

  bool propSqrBnds_(LinSqrMapIter lx2, RelaxationPtr rel, bool mod_rel,
                    bool *changed, ModVector &p_mods, ModVector &r_mods);

  void relax_(RelaxationPtr rel, bool *is_inf);

  /**
   * \brief Add quadratic/linear relaxations of the quadratic range constraint
   * 'cons'.
   */
  void relax_(ConstraintPtr cons, VariablePtr y, VariablePtr x1,
              VariablePtr x2, RelaxationPtr rel);

  void removeFixed_();

  void removeFixedFun_(FunctionPtr f, LinearFunctionPtr lf2, double *c);

  /**
   * \brief Modify bounds of a variable in the problem if the new bounds are
   * tighter.
   * \return the number of bound changes (1 or 2) if any bounds are changed.
   * Returns -1 if the new bounds make the problem infeasible. Returns 0
   * otherwise.
   */
  int updatePBounds_(VariablePtr v, double lb, double ub, bool *changed);
  int updatePBounds_(VariablePtr v, double lb, double ub, RelaxationPtr rel,
                     bool mod_rel, bool *changed, ModVector &p_mods,
                     ModVector &r_mods);

  int upBilCon_(LinBilPtr lx0x1, RelaxationPtr rel, ModVector &r_mods);
  int upSqCon_(ConstraintPtr con, VariablePtr x, VariablePtr y, 
               RelaxationPtr rel, ModVector &r_mods);

  bool varBndsFromCons_(bool *changed);
};

/// Shared pointer to QuadHandler.
typedef boost::shared_ptr<QuadHandler> QuadHandlerPtr;
}
#endif

// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
