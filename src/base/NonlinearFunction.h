//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file NonlinearFunction.h
 * \brief Declare abstract base class NonlinearFunction.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURNONLINEARFUNCTION_H
#define MINOTAURNONLINEARFUNCTION_H

#include "Types.h"

namespace Minotaur {

  struct LTHessStor;
  class NonlinearFunction;
  class VarBoundMod;
  typedef NonlinearFunction *NonlinearFunctionPtr;
  typedef VarBoundMod *VarBoundModPtr;
  typedef std::vector<VarBoundModPtr> VarBoundModVector;
  typedef VarBoundModVector::iterator VarBoundModIter;

  /**
   * \brief Base class for nonlinear functions.
   */
  class NonlinearFunction {

  public:
    /// Default constructor
    NonlinearFunction();

    /// Destroy.
    virtual ~NonlinearFunction();

    /**
     * \brief Add a constant to the function
     *
     * Add a constant to the given nonlinear function. Adding a constant
     * alone usually does not make sense. However, it is useful as a
     * intermediate step. For instance, in writing the perspective:
     * z*f(x/(z+e))
     * \param [in] d The value of the constant.
     * \param [out] err must be nonzero if the constant could not be added.
     */
    virtual void addConst(const double d, int &err);

    /**
     * \brief Change the nonlinear function to its square-root.
     *
     * \param [out] err must be nonzero if the operation is unsuccessful
     */
    virtual void sqrRoot(int &err);

    /**
     * \brief Make a clone using new variables.
     *
     * \param [in] vbeg it points to the variable id 0.  vbeg+k points to
     * variable id k, where k>=0.
     * \param [out] err must be nonzero if function wasn't cloned.
     */
    virtual NonlinearFunctionPtr cloneWithVars(VariableConstIterator vbeg,
                                               int *err) const = 0;

    /**
     * \brief Calculate upper and lower bounds on the function using bounds of
     * the variables.
     * \param [out] lb Pointer that will contain the value of lower bound.
     * \param [out] ub Pointer that will contain the value of upper bound.
     * \param [out] error 0 if no error is encountered, nonzero otherwise.
     */
    virtual void computeBounds(double *lb, double *ub, int *error);

    /**
     * \brief Evaluate the function at a given point x.
     *
     * \param [in] x The size of the array x must exceed the highest index of
     * the variables used in the function.
     * \param [out] *error It should be set a positive value if there is
     * error encountered while evaluating. Leave undisturbed otherwise.
     * \return The value of function of x.
     */
    virtual double eval(const double *x, int *error) = 0;

    /**
     * \brief Evaluate and add gradient at a given point.
     *
     * \param [in] x The size of the array x must exceed the highest index of
     * the variables used in the function.
     * \param [out] grad_f The values of grad_f are incremented with the
     * gradient of this function at x. The array grad_f is dense.
     * \param [out] error It should be set a positive value if there is
     * error encountered while evaluating. Leave undisturbed otherwise.
     */
    virtual void evalGradient(const double *x, double *grad_f,
                              int *error) = 0;

    /**
     * \brief Evaluate and add hessian at a given point.
     *
     * \param [in] mult Multiplier for this objective/constraint function
     * \param [in] x The point where we need the hessian.
     * \param [in] stor The Hessian storage information.
     * \param [out] values The Hessian values to which we add Hessian of
     * this function.
     * \param [out] error We set it to nonzero if any errors are encountered.
     * Do not change it otherwise.
     */
    virtual void evalHessian(const double mult, const double *x,
                             const LTHessStor *stor, double *values,
                             int *error) = 0;

    /**
     * \brief Fill sparsity of hessian into hessian storage.
     *
     * \param [in,out] stor We add variables into stor->cols
     */
    virtual void fillHessStor(LTHessStor *stor) = 0;

    /**
     * \brief Evaluate and add gradient at a given point to the jacobian.
     *
     * \param [in] x The size of the array x must exceed the highest index of
     * the variables used in the function.
     * \param [out] values The values of jacobian are incremented with the
     * gradient of this function at x. 'values' only contains nonzeros of
     * jacobian. The indices (or offsets) where this nonlinear function
     * should put in the values should be calculated in the prepJac
     * function.
     * \param [out] error It should be set a zero value if there is
     * error encountered while evaluating. Leave undisturbed otherwise.
     */
    virtual void fillJac(const double *x, double *values, int *error) = 0;

    /**
     * \brief Finalize hessian preparation.
     *
     * \param [in] stor contains the sparsity pattern of hessian of
     * lagrangian. The nonlinear function should save offsets or make
     * other preparation to evaluate hessian.
     */
    virtual void finalHessStor(const LTHessStor *stor) = 0;

    /**
     * \brief Return a string in AMPL's .nl format (postfix notation) of this
     * nonlinear function.
     *
     * param [out] err 0 if no errors were encountered. <BR>
     * \return Return the string.
     */
    virtual std::string getNlString(int *err);

    /**
     * \brief If a variable is fixed at a given value and removed, what is
     * the constant (offset) needed to be added.
     *
     * param [in] v The variable that is fixed. <BR>
     * param [in] val The value at which v is to be fixed. <BR>
     * \return Return the value of the offset.
     */
    virtual double getFixVarOffset(VariablePtr /* v */, double /* val */)
    {
      assert(!"implment me!");
      return 0;
    };

    /// Return the type of function: polynomial, ...
    virtual FunctionType getType() const;

    /**
     * \brief Get variables used in this function
     *
     * \param [in] vars A set of variable-pointers into which variables are
     * to be inserted.
     */
    virtual void getVars(VariableSet *vars) = 0;

    /**
     * \brief Check if function contains a variable.
     *
     * \param [in] v The variable that we want to test.
     * \return True if this function is has v. False if it doesn't use it.
     */
    virtual bool hasVar(ConstVariablePtr v) const;

    /**
     * \brief Check if the function is a sum of square terms
     *
     * \return True if this function is a sum of squares, False otherwise
     */
    virtual bool isSumOfSquares() const { return true; }

    /**
     * \brief Multiply by a constant.
     *
     * \param [in] c double value with which we want to multiply.
     */
    virtual void multiply(double c) = 0;

    /// Return the number of variables in this function.
    virtual size_t numVars() { return vars_.size(); };

    /**
     * \brief Prepare for evaluating sparse jacobian.
     *
     * All the variables that are in this function can occur in one or more
     * of linear, quadratic and nonlinear functions. All variables that
     * occur in the whole function can be accessed by iterating between vbeg
     * and vend. This function is used to find the offset for variables that
     * occur in the nonlinear function.
     */
    virtual void prepJac(VarSetConstIter vbeg, VarSetConstIter vend) = 0;

    /// Remove a variable v from the function after fixing it to value val.
    virtual void removeVar(VariablePtr /* v */, double /* val */)
    {
      assert(!"implement me!");
    };

    /// Substitute a variable with another.
    virtual void subst(VariablePtr /* out */, VariablePtr /* in */,
                       double /* rat */)
    {
      assert(!"implement me!");
    };

    /**
     * \brief Take perspective of this function with respect to a given
     * variable.
     *
     * Perspective of a given function f(x) with respect to a given variable z
     * is g(x,z) = z.f(x/z)
     * \param  [in] z The variable for which you take the perspective
     * \param  [in] eps The tolerance to tackle function value at 0 value. If
     * the perspective is approximated in the computational graph, the epsilon
     * is added to the z variable. If the perspective uses exact graph, then
     * an approximation in Hessian is used when z < eps.
     * \param [out] err must be nonzero if function wasn't cloned.
     * \return A new nonlinear function with an additional variable that gives
     * the perspective of this function
     */
    virtual NonlinearFunctionPtr getPersp(VariablePtr z, double eps,
                                          int *err) const;

    /**
     * \brief Tighten variables based on function bounds
     *
     * Given a lower and upper bound on the allowed values of the function,
     * deduce bounds of variables and return the modifications in bounds. The
     * modifications are not applied to the variables.
     *
     * param  [in] lb A lower bound on function values <BR>
     * param  [in] ub An upper bound on function values <BR>
     * param  [out] mods A vector of modifications deduced by bound
     * propagation/tightening <BR>
     * param  [out] status Tells whether there was an error in executing this
     * routine or if the problem is infeasible etc.
     */
    virtual void varBoundMods(double /* lb */, double /* ub */,
                              VarBoundModVector & /* mods */,
                              SolveStatus * /* status */) {};

    /**
     * \brief First iterator for variables
     *
     * \return first iterator for the variables in this function.
     */
    virtual VariableSet::iterator varsBegin() { return vars_.begin(); };

    /**
     * \brief Last iterator for variables
     *
     * \return last iterator for the variables in this function.
     */
    virtual VariableSet::iterator varsEnd() { return vars_.end(); };

    /// Display the nonlinear function.
    virtual void write(std::ostream &out) const;

  protected:
    /// A set of variables used in this function.
    VariableSet vars_;
  };

  typedef NonlinearFunction *NonlinearFunctionPtr;
  typedef const NonlinearFunction *ConstNonlinearFunctionPtr;

}  //namespace Minotaur
#endif
