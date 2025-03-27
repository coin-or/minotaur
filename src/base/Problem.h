//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file Problem.h
 * \brief Declare base class Problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPROBLEM_H
#define MINOTAURPROBLEM_H

#include <ios>
#include "Types.h"
#include "Constraint.h"
#include "Engine.h"
#include "HessianOfLag.h"
#include "Jacobian.h"
#include "Objective.h"
#include "ProblemSize.h"
#include "SOS.h"
#include "Types.h"
#include "Variable.h"
#include <limits>
namespace Minotaur
{

  /**
   * \brief The Problem that needs to be solved.
   *
   * The Problem class contains the Variables, Constraints, Objectives that
   * collectively define a particular problem that can be solved. A
   * problem can be described as
   *
   * min/max f(x)
   *
   * s.t.  l_i <= g_i(x) <= u_i, i = 1,2, ... , m
   *
   *       l_i <= x_i <= u_i, i = 1, 2, ..., n
   *
   * A way of setting up an problem is to first ask it to create 'n'
   * variables. Then the constraints and objective can be added one by one.
   *
   * A Problem is a very generic class. The Relaxation classes
   * are derived from it.
   */
  struct ConstraintStats
  {
    int nvars = 0;
    int nterms = 0;
    int nsqterm = 0;
    int nbilterm = 0;
    int nposcoefone = 0;
    int nnegcoefone = 0;
    int nposcoef = 0;
    int nnegcoef = 0;
    int nposcont = 0;
    int nnegcont = 0;
    int nposbin = 0;
    int nnegbin = 0;
    int nposint = 0;
    int nnegint = 0;
    int eqwt = 0;
    double wt1 = 0.0;
    double wt2 = 0.0;
    double sumnegwt = 0.0;
    int con = 0;
  };

class Problem
{
public:
  /// Default constructor
  Problem(EnvPtr env);

  /// Destroy
  virtual ~Problem();

  /// Add 'c' to both lb and ub of a constraint.
  virtual void addToCons(ConstraintPtr cons, double c);

  /// Add a linear function to the objective.
  virtual void addToObj(LinearFunctionPtr lf);

  /// Add a constant term to the objective.
  virtual void addToObj(double cb);


  /// Fill up the statistics about the size of the problem into size_.
  virtual void calculateSize(bool shouldRedo = false);

  /// Change a bound (lower or upper) on a variable with ID=id.
  virtual void changeBoundByInd(UInt ind, BoundType lu, double new_val);

  /// Change both bounds (lower and upper) on a variable with ID=id
  virtual void changeBoundByInd(UInt ind, double new_lb, double new_ub);

  /// Change a bound (lower or upper) on a variable 'var'.
  virtual void changeBound(VariablePtr var, BoundType lu, double new_val);

  /// Change lower and upper bounds on the variable 'var'
  virtual void changeBound(VariablePtr var, double new_lb, double new_ub);

  /// Change a bound (lower or upper) on a constraint 'con'.
  virtual void changeBound(ConstraintPtr con, BoundType lu, double new_val);

  /// Change lower and upper bounds on the constraint 'con'
  virtual void changeBound(ConstraintPtr con, double new_lb, double new_ub);

  /**
   * \brief Change the linear function, and the bounds of a constraint.
   * \param [in] con Original constraint that is to be changed.
   * \param [in] lf The new linear function.
   * \param [in] lb The new lower bound.
   * \param [in] ub The new upper bound.
   */
  virtual void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf,
                                double lb, double ub);

  /**
   * \brief Change the nonlinear function and bounds of a constraint.
   * \param [in] con Original constraint that is to be changed.
   * \param [in] nlf The new nonlinear function.
   */
  virtual void changeConstraint(ConstraintPtr con, NonlinearFunctionPtr nlf);

  /**
   * \brief Replace the objective function with a new function.
   *
   * \param[in] f The new obejctive function. f is cloned. If f is modified
   * after this call, it won't affect the objective.
   * \param[in] cb The new objective constant.
   */
  virtual void changeObj(FunctionPtr f, double cb);

  /**
   * \brief Check whether variables used in the constraints belong to the
   * problem or not.
   *
   * This is a sanity check, used only for debugging.
   * \returns 1 if the check failed, 0 if passed.
   */
  virtual int checkConVars() const;

  // Classification of Constraints
  virtual void classifyCon();

  /**
   * \brief Clone the given Problem class. Jacobian and Hessian in the cloned
   * problem are NULL.
   *
   * The variables are created. If the functions are stored in native format,
   * they are also cloned. Problem size and the initial point are cloned as
   * well.
   * \param[in] env Pointer to environment for the clone.
   */
  ProblemPtr clone(EnvPtr env) const;

  /**
   * \brief shuffle variables and constraints while making a clone of the
   * problem
   *
   * \param[in] varshuff If true, the variables are to be shuffled
   * \param[in] conshuff If true, the constraints are to be shuffled
   * \param[in] env Environment pointer for the cloned problem
   */
  ProblemPtr shuffle(bool varshuff, bool conshuff, EnvPtr env);

  /**
   * \brief Convert all quadratic forms that are stored as CGraph into
   * QuadraticFunction. The order of constraints changes after this
   * conversion.
   */
  void cg2qf();

  /// Iterate over constraints. Returns the 'begin' iterator.
  virtual ConstraintConstIterator consBegin() const
  {
    return cons_.begin();
  }

  /// Iterate over constraints. Returns the 'end' iterator.
  virtual ConstraintConstIterator consEnd() const
  {
    return cons_.end();
  }

  /// Delete marked constraints.
  virtual void delMarkedCons();

  /**
   * Remove marked variables from the problem.
   *
   * \param[in] keep If false, the variable pointer is freed from memory.
   * If true, then it is not deleted, and can possibly be revisited (e.g.
   * for restoring variables in postsolve). Default is false.
   */
  virtual void delMarkedVars(bool keep = false);

  /**
   * \brief Return what type of problem it is. May result in re-calculation of
   * the problem size.
   */
  virtual ProblemType findType();

  /// Return a pointer to the constraint with a given index
  virtual ConstraintPtr getConstraint(UInt index) const;

  /**
   * \brief Give a pointer to the debug solution.
   *
   * \Returns null if there is none, otherwise a pointer to a vector of
   * doubles.
   */
  virtual DoubleVector* getDebugSol() const;

  /// Return the hessian of the lagrangean. Could be NULL.
  virtual HessianOfLagPtr getHessian() const;

  /// Return the jacobian. Could be NULL.
  virtual JacobianPtr getJacobian() const;

  /// Return the number of constraints.
  virtual UInt getNumCons() const
  {
    return cons_.size();
  }

  /// Return the number of constraints marked for deletion.
  virtual UInt getNumDCons() const
  {
    return numDCons_;
  }

  /// Return the number of variables marked for deletion.
  virtual UInt getNumDVars() const
  {
    return numDVars_;
  }

  /**
   * \brief Return the number of non-zeros in the hessian of the lagrangean of
   * the problem.
   *
   * The lagrangean is defined as:
   * \\sigma . f(x) + \\sum_{i=0}^{m-1}\\lambda_i . g_i(x),
   * where \\sigma \\in R^1 and \\lambda \\in R^m are the dual multipliers.
   * The hessian, w.r.t. x, is thus a square symmetric matrix. usually the
   * multipliers are provided by NLP solvers.
   * Such solvers may require during initialization, the number of non-zeros
   * in the lower triangular of the hessian.
   */
  virtual UInt getNumHessNnzs() const;

  /// Return the number of non zerors in the jacobian of the constraints.
  virtual UInt getNumJacNnzs() const;

  /// Return the number of linear constraints in the problem.
  UInt getNumLinCons();

  /// Return the number of SOS Type 1 constraints.
  UInt getNumSOS1();

  /// Return the number of SOS Type 2 constraints.
  UInt getNumSOS2();

  /// Return the number of variables.
  virtual UInt getNumVars() const
  {
    return vars_.size();
  }

  /// Return a pointer to the objective Function
  virtual ObjectivePtr getObjective() const;

  /// Return the value of objective function at given point x.
  double getObjValue(const double* x, int* err) const;

  /// Fill up the statistics about the size of the problem into size_.
  ConstProblemSizePtr getSize() const;

  /// Calculate and return a measure of the size of the problem.
  double getSizeEstimate();

  /// Return a pointer to the variable with a given index
  virtual VariablePtr getVariable(UInt index) const;

  /**
   * \brief Return true if the derivative is available through Minotaur's own
   * routines for storing nonlinear functions.
   */
  virtual bool hasNativeDer() const;

  /**
   * \brief Returns true if the debug solution is cut off by the constraints
   * of the problem.
   */
  virtual bool isDebugSolFeas(double atol, double rtol);

  /**
   * \brief Returns true if the problem has only linear constraints and linear
   * objectives.
   */
  virtual bool isLinear();

  /// Return true if a constraint is marked deleted.
  virtual bool isMarkedDel(ConstConstraintPtr con);

  /// Return true if a constraint is marked deleted.
  virtual bool isMarkedDel(ConstVariablePtr var);

  /**
   * \brief Returns true if the problem has
   * (1) linear or quadratic objective, and
   * (2) linear constraints only.
   */
  virtual bool isQP();

  /**
   * \brief Returns true if the problem has only linear or quadratic
   * constraints and linear or quadratic objectives.  Returns false if a
   * problem is linear. Returns false if problem is nonlinear.
   */
  virtual bool isQuadratic();

  /**
   * \brief Mark a constraint for deleting.
   *
   * The constraint is not deleted, just marked. Call Problem::delMarkedCons()
   * to actually delete all the marked constraints.
   * \param[in] con The constraint to be marked.
   */
  virtual void markDelete(ConstraintPtr con);

  // Helper function for common quadratic checks
  inline bool checkQuadCond(const ConstraintStats& stats)
  {
    return stats.nsqterm == stats.nterms + stats.nvars;
  }

  // Helper function for coefficient checks
  inline bool checkCoeffCond(const ConstraintStats& stats,
                                        bool positive)
  {
    if (positive) {
      return stats.nposcoefone == stats.nterms + stats.nvars ||
             (stats.nposcoef == stats.nterms + stats.nvars &&
              stats.eqwt == stats.nterms + stats.nvars);
    } else {
      return stats.nnegcoefone == stats.nterms + stats.nvars ||
             (stats.nnegcoef == stats.nterms + stats.nvars &&
              stats.eqwt == stats.nterms + stats.nvars);
    }
  }

  /**
   * \brief Mark a variable as deleted.
   *
   * The variable is not deleted, just marked. Call Problem::delMarkedVars()
   * to actually delete all the marked variables.
   * \param[in] var The variable to be marked.
   */
  virtual void markDelete(VariablePtr var);

  /// The objective is multiplied by -1.
  virtual void negateObj();

  /**
   * \brief Add a new binary variable and return a pointer to it. A name is
   * automatically generated by default.
   */
  virtual VariablePtr newBinaryVariable();

  /**
   * \brief Add a new binary variable.
   *
   * \param[in] name The predefined name for this variable.
   */
  virtual VariablePtr newBinaryVariable(std::string name);

  /**
   * \brief Add a new constraint and return a pointer to it. A name is
   * automatically generated by default.
   *
   * \param[in] f Pointer to the Function in the constraint. It is not cloned.
   * The pointer is saved as it is.
   * \param[in] lb The lower bound of the constraint. May be -INFINITY.
   * \param[in] ub The upper bound of the constraint. May be +INFINITY.
   */
  virtual ConstraintPtr newConstraint(FunctionPtr f, double lb, double ub);

  /**
   * \brief Add a new constraint and return a pointer to it.
   *
   * \param[in] f Pointer to the Function in the constraint. It is not cloned.
   * The pointer is saved as it is.
   * \param[in] lb The lower bound of the constraint. May be -INFINITY.
   * \param[in] ub The upper bound of the constraint. May be +INFINITY.
   * \param[in] name The name for the constraint.
   */
  virtual ConstraintPtr newConstraint(FunctionPtr f, double lb, double ub,
                                      std::string name);

  /**
   * \brief Add a new objective. A name is automatically generated by
   * default.
   *
   * \param[in] f Pointer to the Function in the objective. It is not
   * cloned. The pointer is saved as it is.
   * \param[in] cb The constant term in the objective function.
   * \param[in] otyp Whether the objective is to Minimize or Maximize.
   */
  virtual ObjectivePtr newObjective(FunctionPtr f, double cb,
                                    ObjectiveType otyp);

  virtual ObjectivePtr newObjective(double cb, ObjectiveType otyp);

  /**
   * \brief Add a new objective.
   *
   * \param[in] f Pointer to the Function in the objective. It is not cloned.
   * The pointer is saved as it is.
   * \param[in] cb The constant term in the objective function.
   * \param[in] otyp Whether the objective is to Minimize or Maximize.
   * \param[in] name The name for the objective function.
   *
   * \returns Pointer to the newly added objective function.
   */
  virtual ObjectivePtr newObjective(FunctionPtr f, double cb,
                                    ObjectiveType otyp, std::string name);

  /**
   * \brief Add a new SOS constraint with a name.
   *
   * \param[in] n Number of variables in this SOS constraint.
   * \param[in] type SOS1 (SOS type 1) or SOS2 (SOS type 2).
   * \param[in] weights Values of coefficients of variables in the SOS
   * constraint or just relative weights.
   * \param[in] vars Variables in the constraint.
   * \param[in] priority The priority provided by the user for this
   * constraint.
   * \param[in] name The name provided by the user for this
   * SOS.
   *
   * \returns Pointer to the newly added SOS data.
   */
  virtual SOSPtr newSOS(int n, SOSType type, const double* weights,
                        const VarVector& vars, int priority,
                        std::string name);

  /**
   * \brief Add a new SOS constraint (name generated automatically).
   *
   * \param[in] n Number of variables in this SOS constraint.
   * \param[in] type SOS1 (SOS type 1) or SOS2 (SOS type 2).
   * \param[in] weights Values of coefficients of variables in the SOS
   * constraint or just relative weights.
   * \param[in] vars Variables in the constraint.
   * \param[in] priority The priority provided by the user for this
   * constraint.
   *
   * \returns Pointer to the newly added SOS data.
   */
  virtual SOSPtr newSOS(int n, SOSType type, const double* weights,
                        const VarVector& vars, int priority);

  /**
   * \brief Add a new continuous, unbounded variable to the Problem.
   * \param[in] stype The source of the variable
   */
  virtual VariablePtr newVariable(VarSrcType stype = VarOrig);

  /**
   * \brief Add a new variable with bounds, type. A name is automatically
   * generated by default.
   *
   * \param[in] lb The lower bound on the variable. May be -INFINITY.
   * \param[in] ub The upper bound on the variable. May be +INFINITY.
   * \param[in] vtype Type of the variable: Integer, Continuous, Binary.
   * \param[in] stype The source of the variable
   */
  virtual VariablePtr newVariable(double lb, double ub, VariableType vtype,
                                  VarSrcType = VarOrig);

  /**
   * \brief Add a new variable.
   *
   * \param[in] lb The lower bound on the variable. May be -INFINITY.
   * \param[in] ub The upper bound on the variable. May be +INFINITY.
   * \param[in] vtype Type of the variable: Integer, Continuous, Binary.
   * \param[in] name Name of the variable.
   * \param[in] stype The source of the variable
   */
  virtual VariablePtr newVariable(double lb, double ub, VariableType vtype,
                                  std::string name, VarSrcType = VarOrig);

  /**
   * \brief Clone the variables pointed by the iterators and add them.
   *
   * Given starting and stopping iterators of variables, clone these
   * variables and add the copies to this problem. Do not add them to any
   * constraints or objectives. The IDs are not copied.
   * \param[in] v_begin The 'begin' iterator of the variable vector.
   * \param[in] v_end The 'end' iterator of the variable vector.
   * \param[in] stype The source of the variables
   */
  virtual void newVariables(VariableConstIterator v_begin,
                            VariableConstIterator v_end,
                            VarSrcType stype = VarOrig);

  /**
   * \brief Setup problem data-structures for solving it.
   *
   * Prepare necessary data structures for the next solve. e.g.
   * if constraints have been modified, then re-evalate the sparsity
   * pattern of Jacobian and Hessian.
   */
  virtual void prepareForSolve();

  /**
   *  Move the nonlinear part of the objective function to constraints
   *  set using an auxiliary variable
   */
  virtual void objToCons();

  /// Remove objective from the Problem.
  virtual void removeObjective();

  /// Remove the quadratic part of objective and return it.
  virtual QuadraticFunctionPtr removeQuadFromObj();

  virtual NonlinearFunctionPtr removeNonlinFromObj();

  /**
   * Remove the jacobian and hessian data structures. Useful when you want to
   * re-compute the derivatives after a problem has been modified.
   */
  virtual void resetDer();

  virtual void resConTypCnt();

  /**
   * \brief Reverse the sense of a constraint.
   *
   * \param[in] cons The constraint whose sense has to be reversed.
   */
  virtual void reverseSense(ConstraintPtr cons);

  /**
   * \brief Set a solution that can be checked for accidental cutting off by
   * cuts, branching, reformulations etc.
   *
   * \param[in] x A vector of double values in the same order as variables
   * in the problem.
   */
  virtual void setDebugSol(const DoubleVector& x);

  /**
   * \brief Set the engine that is used to solve this problem.
   *
   * The problem contains a pointer to the engine so that whenever the problem
   * is modified, the engine also gets the modifications. This function sets
   * the engine that must be modified whenever the problem is modified.
   * \param[in] engine The engine pointer.
   */
  virtual void setEngine(Engine* engine);

  /**
   * \brief Add a pointer to the hessian of the Lagrangean.
   *
   * \param[in] hessian Pointer to the HessianOfLag object.
   */
  virtual void setHessian(HessianOfLagPtr hessian);

  /**
   * \brief Set an initial point.
   *
   * Initial point is used by some engines like IpoptEngine. If the
   * initial point has already been set before, it is overwritten by the
   * new point.
   * \param[in] x An array of double values containing the coordinates of the
   * initial point.
   */
  virtual void setInitialPoint(const double* x);

  /**
   * \brief Set an initial point.
   *
   * Same as function Problem::setInitialPoint, but only set values for the
   * first 'k' variables. Put in because of AMPL's defined variables.
   * \param[in] x An array of double values containing the coordinates of the
   * initial point.
   * \param[in] k The first 'k' variables will be initialized.
   */
  virtual void setInitialPoint(const double* x, size_t k);

  virtual void setInitVal(VariablePtr v, double val);
  virtual void setInitValByInd(UInt ind, double val);

  /**
   * \brief Set the jacobian of the constraints.
   *
   * \param[in] jacobian Pointer to the Jacobian object.
   */
  virtual void setJacobian(JacobianPtr jacobian);

  /**
   * \brief Ask Problem to construct its own jacobian and hessian using
   * Minotaur's native code for nonlinear functions.
   */
  void setNativeDer();

  /**
   * \brief Change the variable type.
   *
   * \param[in] var The variable pointer whose type needs to be changed.
   * \param[in] type The new VariableType.
   */
  virtual void setVarType(VariablePtr var, VariableType type);

  virtual SOSConstIterator sos1Begin() const
  {
    return sos1_.begin();
  };
  virtual SOSConstIterator sos1End() const
  {
    return sos1_.end();
  };
  virtual SOSConstIterator sos2Begin() const
  {
    return sos2_.begin();
  };
  virtual SOSConstIterator sos2End() const
  {
    return sos2_.end();
  };

  /**
   * \brief Substitute a variable 'out' with the variable 'in' through out the
   * problem.
   *
   * \param[in] out The variable that is to be substituted out.
   * \param[in] in The variable that replaces the variable 'out'.
   * \param[in] rat The ratio of substitution.
   * \f$v_{in} = rat \times v_{out}\f$.
   */
  virtual void subst(VariablePtr out, VariablePtr in, double rat = 1.0);

  /// Should be called in the Engine's destructor
  virtual void unsetEngine();

  /// Iterate over variables.
  virtual VariableConstIterator varsBegin() const
  {
    return vars_.begin();
  }

  /// Iterate over variables.
  virtual VariableConstIterator varsEnd() const
  {
    return vars_.end();
  }

  /// only for debugging, developing etc.
  virtual void write(std::ostream& out, std::streamsize out_p = 6) const;

  /// Write the problem size to logger_
  virtual void writeSize(std::ostream& out) const;

  //Print statistics in log_level 2 
  void statistics(std::ostream& out) const;

  const double INFTY = std::numeric_limits<double>::infinity();

  protected:
    //To check type of constraint
    bool isAggregation_(ConstraintPtr c);
    bool isPrecedence_(ConstraintPtr c, const ConstraintStats &stats);
    bool isVariableBound_(ConstraintPtr c, const ConstraintStats &stats);
    bool isSetPartitioning_(ConstraintPtr c, const ConstraintStats &stats);
    bool isSetPacking_(ConstraintPtr c, const ConstraintStats &stats);
    bool isSetCovering_(ConstraintPtr c, const ConstraintStats &stats);
    bool isCardinality_(ConstraintPtr c, const ConstraintStats &stats);
    bool isInvariantKnapsack_(ConstraintPtr c, const ConstraintStats &stats);
    bool isEquationKnapsack_(ConstraintPtr c, const ConstraintStats &stats);
    bool isBinPacking_(ConstraintPtr c, const ConstraintStats &stats);
    bool isKnapsack_(ConstraintPtr c, const ConstraintStats &stats);
    bool isIntegerKnapsack_(ConstraintPtr c, const ConstraintStats &stats);
    bool isMixedBinary_(ConstraintPtr c, const ConstraintStats &stats);
    bool isNoSpecificStructure_(ConstraintPtr c);
    bool isDiagonalQuadratic_(ConstraintPtr c, const ConstraintStats& stats);
    bool isSimpleBall_(ConstraintPtr c, const ConstraintStats& stats);
    bool isEllipsoid_(ConstraintPtr c, const ConstraintStats& stats);
    bool isComplementEllipsoid_(ConstraintPtr c, const ConstraintStats& stats);
    bool isComplementSimpleBall_(ConstraintPtr c, const ConstraintStats& stats);
    bool isSocSimple_(ConstraintPtr c, const ConstraintStats& stats);
    bool isPureBilinear_(ConstraintPtr c, const ConstraintStats& stats);
    bool isOtherQuadType_(ConstraintPtr c, const ConstraintStats& stats);
    
    //Print Count table for constraint size
    void printConstraintStatistics_();

    //Print Count table for Quadratic constraint size
    void printConstraintStatisticsQuad_();

  // function for lock number
  void lockNum_();

  /// Vector of constraints.
  ConstraintVector cons_;

  /**
   * \brief Flag that is turned on if the constraints are added or modified.
   *
   * When the problem is changed, say constraints are added or deleted,
   * all associated changes do not happen at that time. For example, when
   * new constraint is added, the hessian and jacobian do not change.
   * This variable is true if constraints have changed since the last
   * time all changes were applied.
   */
  bool consModed_;

  /**
   * \brief A solution to be used for debugging against accidentally cutting
   * of feasible points.
   */
  DoubleVector* debugSol_;

  /// Engine that must be updated if problem is loaded to it, could be null
  Engine* engine_;

  /// Pointer to the hessian of the lagrangean. Could be NULL.
  HessianOfLagPtr hessian_;

  /// Pointer to the jacobian of constraints. Can be NULL.
  JacobianPtr jacobian_;

  /// Pointer to the log manager. All output messages are sent to it.
  LoggerPtr logger_;

  /// For logging
  static const std::string me_;

  /// If true, set up our own Hessian and Jacobian.
  bool nativeDer_;

  /// ID of the next constraint.
  UInt nextCId_;

  /// ID of the next SOS constraint.
  int nextSId_;

  /// ID of the next variable.
  UInt nextVId_;

  /// Number of constraints marked for deletion
  UInt numDCons_;

  /// Number of variables marked for deletion
  UInt numDVars_;

  /// Objective, could be NULL.
  ObjectivePtr obj_;

  /// Size statistics for this Problem.
  ProblemSizePtr size_;

  /// SOS1 constraints.
  SOSVector sos1_;

  /// SOS2 constraints.
  SOSVector sos2_;

  /// Vector of variables.
  VarVector vars_;

  /**
   * \brief Vector of variables removed from the problem but not yet freed
   * from memory.
   */
  VarVector varsRem_;

  /// True if variables delete, added or their bounds changed.
  bool varsModed_;

  /// Count the types of constraints and fill the values in size_.
  virtual void countConsTypes_();

  /// Count the types of objectives and fill the values in size_.
  virtual void countObjTypes_();

  /// Fill up the size_ with number of variables of each type.
  virtual void countVarTypes_();

  /**
   * \brief Update the function types of all variables based on whether they
   * appear as linear, quadratic or nonlinear in each objective and
   * constraint.
   */
  virtual void findVarFunTypes_();

  bool isPolyp_();

  void setIndex_(VariablePtr v, UInt i);
};
}  // namespace Minotaur
#endif