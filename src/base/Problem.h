//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2012 The MINOTAUR Team.
//


/**
 * \file Problem.h
 * \brief Declare base class Problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPROBLEM_H
#define MINOTAURPROBLEM_H

#include "Types.h"
#include "Variable.h"

namespace Minotaur {

class DenseMatrix;
class Engine;
class Function;
class HessianOfLag;
class Jacobian;
class LinearFunction;
class Objective;
struct ProblemSize;
class QuadraticFunction;
class SparseMatrix;
typedef boost::shared_ptr<Function> FunctionPtr;
typedef boost::shared_ptr<Jacobian> JacobianPtr;
typedef boost::shared_ptr<HessianOfLag> HessianOfLagPtr;
typedef boost::shared_ptr<LinearFunction> LinearFunctionPtr;
typedef boost::shared_ptr<Objective> ObjectivePtr;
typedef boost::shared_ptr<ProblemSize> ProblemSizePtr;
typedef boost::shared_ptr<QuadraticFunction> QuadraticFunctionPtr;
typedef boost::shared_ptr<const ProblemSize> ConstProblemSizePtr;

/**
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
class Problem {
public:
  // ---------------------------------------------------------------------
  // Friendships
  // ---------------------------------------------------------------------
  friend class Presolver;

  // ---------------------------------------------------------------------
  // constructors, destructors
  // ---------------------------------------------------------------------

  /// Default constructor
  Problem();

  /** 
   * Construct a Problem with 'n' variables with lower bounds and upper
   * bounds and with the specified type (integer, binary, continuous). 
   * 'n' is the size of either vector. This method should throw an error
   * if vTypes and bounds are conflicting, e.g. a binary variable is given
   * bounds [-10, 10].
   */
  Problem(std::vector<Double> lbs, std::vector<Double> ubs, 
          std::vector<VariableType> vTypes);

  /** 
   * Construct a Problem with 'n' variables with lower bounds and upper
   * bounds. 'n' is the size of either vector.
   */
  Problem(std::vector<Double> lbs, std::vector<Double> ubs);

  /// Destroy
  virtual ~Problem();

  // ---------------------------------------------------------------------
  // build the Problem
  // ---------------------------------------------------------------------

  /// Clone the given Problem class. Jacobian and Hessian in the cloned
  /// problem are NULL.
  boost::shared_ptr<Problem> clone() const;

  /// Add a new continuous, unbounded variable to the Problem. 
  virtual VariablePtr newVariable(); 

  /**
   * Add a new variable with bounds, type. This method throws error 
   * if the bounds and type conflict.
   */
  virtual VariablePtr newVariable(Double lb, Double ub, VariableType vType); 
                                            
  /**
   * Add a new variable with bounds, type and name. This method throws 
   * error if the bounds and type conflict.
   */
  virtual VariablePtr newVariable(Double lb, Double ub, VariableType vType,
                                  std::string name); 

  /// Clone the variables pointed by the iterators and add them.
  /**
   * Given starting and stopping iterators of variables, clone these
   * variables and them to the relaxation. Do not add them to any
   * constraints or objectives. The IDs are not copied.
   */
  virtual void newVariables(VariableConstIterator v_begin, 
                            VariableConstIterator v_end);
                                            
  /// Add a new binary variable.
  virtual VariablePtr newBinaryVariable(); 
                                            
  /// Add a new binary variable, with predefined name.
  virtual VariablePtr newBinaryVariable(std::string name); 
                                            
  /// Add a new constraint using function, lb, ub. 
  virtual ConstraintPtr newConstraint(FunctionPtr f, Double lb, Double ub);

  /// Add a new constraint using function, lb, ub, name. 
  virtual ConstraintPtr newConstraint(FunctionPtr f, Double lb, Double ub, 
                                      std::string name);

  /**
   * Add a new objective from a function pointer, a constant offset and
   * objective type (min/max).
   */
  virtual ObjectivePtr newObjective(FunctionPtr fun, Double cb, 
                                    ObjectiveType otyp);

  /// Add an objective with a name.
  virtual ObjectivePtr newObjective(FunctionPtr fun, Double cb, 
                                    ObjectiveType otyp, std::string name);

  /**
   * Set an initial point. used by some engines like IpoptEngine. If the
   * initial point has already been set before, it is overwritten by the
   * new point.
   */
  virtual void setInitialPoint(const Double *x);

  /** 
   * Same as above, but only set values for the first 'k' variables. Put
   * in because of AMPL's defined variables.
   */
  virtual void setInitialPoint(const Double *x, Size_t k);

  /**
   * Set the jacobian of the constraints. Currently there is no other way to
   * generate/set the jacobian.
   */
  virtual void setJacobian(JacobianPtr jacobian);

  /**
   * Set the hessian of the lagrangean. Currently there is no other way to
   * generate/set the hessian.
   */
  virtual void setHessian(HessianOfLagPtr hessian);

  /// Initialize the Jacobian for a native computational graph problem 
  void setNativeDer();

  /// Set the log manager
  virtual void  setLogger(LoggerPtr logger);

  /**
   * Prepare necessary data structures for the next solve. e.g.
   * if constraints have been modified, then re-evalate the sparsity
   * pattern of Jacobian and Hessian.
   */
  virtual void prepareForSolve();

  // ---------------------------------------------------------------------
  // get, query methods 
  // ---------------------------------------------------------------------

  /// Fill up the statistics about the size of the problem into size_.
  ConstProblemSizePtr getSize() const;

  /// Iterate over variables.
  virtual VariableConstIterator varsBegin() const { return vars_.begin(); }

  /// Iterate over variables.
  virtual VariableConstIterator varsEnd() const { return vars_.end(); }

  /// Iterate over constraints.
  virtual ConstraintConstIterator consBegin() const 
    { return cons_.begin(); }

  /// Iterate over constraints.
  virtual ConstraintConstIterator consEnd() const { return cons_.end(); }


  /// Return what type of problem it is. May result in re-calculation of the
  /// problem size.
  virtual ProblemType findType();

  /// Return a pointer to the variable with ID=id.
  virtual VariablePtr getVariable(UInt id) const;

  /// Return a pointer to the constraint with ID=id.
  virtual ConstraintPtr getConstraint(UInt id) const;

  /// Return a pointer to the objective Function
  virtual ObjectivePtr getObjective() const;

  /// Return the value of objective function at given point x.
  Double getObjValue(const Double *x, Int *err) const;

  /// Return the number of constraints.
  virtual UInt getNumCons() const { return cons_.size(); }

  /// Return the number of variables.
  virtual UInt getNumVars() const { return vars_.size(); }

  /// Return the number of variables marked for deletion.
  virtual UInt getNumDVars() const { return numDVars_; }

  /// Return the number of constraints marked for deletion.
  virtual UInt getNumDCons() const { return numDCons_; }

  /**
   * get the initial point. Used by some engines like IpoptEngine. The
   * pointer returned from this function should not be changed or deleted.
   */
  virtual const Double * getInitialPoint() const { return initialPt_; }

  /// Return the number of non zerors in the jacobian of the constraints.
  virtual UInt getNumJacNnzs() const;

  /// Return the jacobian. Could be NULL.
  virtual JacobianPtr getJacobian() const;

  /**
   * Return the number of non-zeros in the hessian of the lagrangean of the 
   * problem. The lagrangean is defined as:
   * \\sigma . f(x) + \\sum_{i=0}^{m-1}\\lambda_i . g_i(x), 
   * where \\sigma \\in R^1 and \\lambda \\in R^m are the dual multipliers. 
   * The hessian, w.r.t. x, is thus a square symmetric matrix. usually the
   * multipliers are provided by NLP solvers. 
   * Such solvers may require during initialization, the number of non-zeros
   * in the lower triangular of the hessian.
   */
  virtual UInt getNumHessNnzs() const;

  /// Return the hessian of the lagrangean. Could be NULL.
  virtual HessianOfLagPtr getHessian() const;

  /// Get pointer to the log manager. Could be NULL.
  virtual LoggerPtr  getLogger();

  virtual Bool hasNativeDer() const;

  /// True if a constraint is marked deleted.
  virtual Bool isMarkedDel(ConstVariablePtr var);

  /// True if a constraint is marked deleted.
  virtual Bool isMarkedDel(ConstConstraintPtr con);

  /**
   * Returns true if the problem has only linear constraints and linear
   * objectives
   */
  virtual Bool isLinear();

  /**
   * Returns true if the problem has 
   * (1) linear or quadratic objective, and
   * (2) linear constraints only.
   */
  virtual Bool isQP();

  /**
   * Returns true if the problem has only linear or quadratic constraints
   * and linear or quadratic objectives.  Returns false if a problem is
   * linear. Returns false if problem is nonlinear.
   */
  virtual Bool isQuadratic();

  /// only for debugging, developing etc.
  virtual void write(std::ostream &out, std::streamsize out_p=6) const;
    
  /// Write the problem size to logger_
  virtual void writeSize(std::ostream &out) const;

  // ----------------------------------------------------------------------
  // evaluate methods 
  // ----------------------------------------------------------------------
      
  /// Fill up the statistics about the size of the problem into size_.
  virtual void calculateSize(Bool shouldRedo=false);

  /**
   * Check if a given vector satisfies integrality of the integer
   * constrained variables.
   */
  virtual Bool isSolIntegral(const Double *x);


  // ----------------------------------------------------------------------
  // change, modify, delete methods
  // ----------------------------------------------------------------------

  /// Change a bound (lower or upper) on a variable with ID=id.
  virtual void changeBound(UInt id, BoundType lu, Double new_val);

  /// Change both bounds (lower and upper) on a variable with ID=id
  virtual void changeBound(UInt id, Double new_lb, Double new_ub);

  /// Change a bound (lower or upper) on a variable 'var'. 
  virtual void changeBound(VariablePtr var, BoundType lu, Double new_val);

  /// Change lower and upper bounds on the variable 'var'
  virtual void changeBound(VariablePtr var, Double new_lb, 
                           Double new_ub); 

  /// Change a bound (lower or upper) on a constraint 'con'. 
  virtual void changeBound(ConstraintPtr con, BoundType lu, Double new_val);

  /// Change lower and upper bounds on the constraint 'con'
  virtual void changeBound(ConstraintPtr con, Double new_lb, 
                           Double new_ub); 

  /// Change the variable type.
  virtual void setVarType(VariablePtr var, VariableType type);

  /// Reverse the sense of a constraint.
  virtual void reverseSense(ConstraintPtr cons);

  /// Add 'c' to both lb and ub of a constraint.
  virtual void addToCons(ConstraintPtr cons, Double c);

  /// Add a linear function to the objective.
  virtual void addToObj(LinearFunctionPtr lf);

  /// Add a constant term to the objective.
  virtual void addToObj(Double cb);

  /**
   * Substitute a variable 'out' with the variable 'in' through out the
   * problem.
   */
  virtual void subst(VariablePtr out, VariablePtr in, Double rat=1.0);

  /// Remove the quadratic part of objective and return it.
  virtual QuadraticFunctionPtr removeQuadFromObj();

  /// The objective is multiplied by -1.
  virtual void negateObj();

  /**
   * Replace the objective function with the function f. Function f is
   * cloned. If f is modified after this call, it won't affect the
   * objective.
   */
  virtual void changeObj(FunctionPtr f, Double cb);

  /**
   * \brief Change the linear function, and the bounds of a constraint.
   * \param [in] c Original constraint that is to be changed.
   * \param [lf] The new linear function.
   * \param [lb] The new lower bound.
   * \param [ub] The new upper bound.
   */
  virtual void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf, 
                                Double lb, Double ub);

  /// Mark a variable as deleted.
  virtual void markDelete(VariablePtr var);
      
  /// Delete marked variables.
  virtual void delMarkedVars();

  /// Mark a constraint as deleted.
  virtual void markDelete(ConstraintPtr con);
      
  /// Delete marked constraints.
  virtual void delMarkedCons();

  /// Add the given linear function to the function of a constraint
  virtual void addToConstraint(UInt id, LinearFunctionPtr lf);

  /// Remove objective from the Problem.
  virtual void removeObjective(); 

  /// Should only be called by the Engine that loads this
  virtual void setEngine(Engine* engine);

  /// Should be called in the Engine's destructor
  virtual void unsetEngine();

  /**
   * Delete the whole Problem. Variables and constraints are so
   * interlinked that we just cant call the destructor. This function just
   * deletes all the constraints. The variables and functions can still be
   * used after this is called.
   */
  virtual void clear();

  UInt getNumLinCons(); //{ return size_->linCons; }

protected:
  /// If true, set up our own Hessian and Jacobian.
  Bool nativeDer_;

  /// Vector of variables.
  std::vector<VariablePtr> vars_;

  /// Vector of constraints.
  std::vector<ConstraintPtr> cons_;

  /// Objective, could be NULL.
  ObjectivePtr obj_;

  /// Initial point. Could be NULL.
  Double * initialPt_;

  /**
   * Tolerance value used to check integrality.
   * \todo Methods to get/set this.
   */
  Double intTol_;

  /// Pointer to the jacobian of constraints. Could be NULL.
  JacobianPtr jacobian_;

  /// ID of the next constraint.
  UInt nextCId_;

  /// ID of the next variable.
  UInt nextVId_;

  /// Pointer to the hessian of the lagrangean. Could be NULL.
  HessianOfLagPtr hessian_;

  /**
   * Pointer to the log manager. All output messages are sent to it. Could
   * be NULL.
   */
  LoggerPtr logger_;

  /// Size statistics for this Problem.
  ProblemSizePtr size_;

  /// Number of variables marked for deletion
  UInt numDVars_;	

  /// Number of constraints marked for deletion
  UInt numDCons_;
	
  /// Engine that must be updated if problem is loaded to it, could be null 
  Engine* engine_;

  /**
   * JIM: Note, I am trying to change this below...
   *  When the problem is changed, say constraints are added or deleted,
   *  all associated changes do not happen at that time. For example, when
   *  new constraint is added, the hessian and jacobian do not change.
   *  This variable is true if constraints have changed since the last
   *  time all changes were applied.
   */
  Bool consModed_;
      
  /// True if variables delete, added or their bounds changed.
  Bool varsModed_;

  /// Count the types of constraints and fill the values in size_.
  virtual void countConsTypes_();

  /// Count the types of objectives and fill the values in size_.
  virtual void countObjTypes_();

  /// Fill up the size_ with number of variables of each type.
  virtual void countVarTypes_();

  /**
   * Update the function types of all variables based on if they appear as
   * linear, quadratic or nonlinear in each objective and constraint.
   */
  virtual void findVarFunTypes_();

  Bool isPolyp_();

  void setIndex_(VariablePtr v, UInt i);
      
};
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
