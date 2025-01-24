// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file Function.h
 * \brief Get information about a Function.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURFUNCTION_H
#define MINOTAURFUNCTION_H

#include "Types.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"

namespace Minotaur {

  struct LTHessStor;

  /**
   * The class Function is meant to model functions that are used to specify
   * constraints and objectives in a problem. A function can have three or
   * more components: linear, quadratic and nonlinear. Each of these types is
   * implemented as a separate class. A Function thus keeps a pointer to each
   * of these components and has methods to consolidate methods of these
   * components. For instance, a function evaluation will just call these
   * evaluation routines of the components and add them together. If any of the
   * individual components of a function (linear, quadratic, nonlinear)
   * are modified, one should delete the existing function and create a new
   * one as some of the data-structures may not get updated with the changes.
   */
  class Function {
  public:

    /// Default constructor
    Function();

    /// Construct a function using only a linear function. 
    Function(LinearFunctionPtr lf);

    /**
     * Construct a function using a linear function and a quadratic
     * function.
     */
    Function(LinearFunctionPtr lf, QuadraticFunctionPtr qf);

    /// Construct a function using a linear function and a nonlinear function.
    Function(LinearFunctionPtr lf, NonlinearFunctionPtr nlf);

    /**
     * Construct a function using a linear function, a quadratic
     * function and a nonlinear function.
     */
    Function(LinearFunctionPtr lf, QuadraticFunctionPtr qf,
             NonlinearFunctionPtr nlf);

    /// Construct a function using only a nonlinear function.
    Function(NonlinearFunctionPtr nlf);

    /// Destroy.
    virtual ~Function();

    /// Make a clone using new variables. vbeg points to the variable id 0.
    /// vbeg+k points to variable id k, where k>=0.
    virtual FunctionPtr cloneWithVars(VariableConstIterator vbeg, int *err)
      const;

    /// similar to above function, but for shuffling variables
    virtual FunctionPtr cloneWithVarsPermute(VariableConstIterator vbeg, UIntVector variableaddress, int *err)
      const;

    /**
     * Evaluate the function at a given point x. error must be zero if no
     * errors were encountered.
     */
    virtual double eval(const DoubleVector &x, int *error) const;

    /// Evaluate the function at a given point x.
    virtual double eval(const double *x, int *error) const;

    virtual void prepJac();

    /**
     * Evaluate Gradient at the given point x and fill in the array grad_f.
     */
    virtual void evalGradient(const double *x, double *grad_f, int *error) 
      const;

    virtual void fillJac(const double *x, double *values, int *error);
    /**
     * Get number of terms in the hessian of the function. We only count
     * terms that are nonzero in the lower-triangular half (including the
     * diagonal) of the hessian matrix.
     */
    virtual UInt getNumNzInHess();

    /**
     * Return true if variable var is one of the arguments of this function.
     * Return false otherwise.
     */
    virtual bool hasVar(VariablePtr var) const;

    /**
     * Return the type of a function: linear, quadratic, nonlinear etc. If
     * the return type is quadratic, we may still have a linear component.
     */
    virtual FunctionType getType();

    /**
     * \brief Check if function is linear in a variable.
     *
     * \param[in] v The variable that we want to test.
     * \return True if this function is linear or constant in variable v. 
     */
    bool isLinearIn(ConstVariablePtr v);
    virtual FunctionType getVarFunType(ConstVariablePtr v);

    /// Return the linear part of the function.
    virtual LinearFunctionPtr getLinearFunction() const;

    /// Return the quadratic part of the function.
    virtual QuadraticFunctionPtr getQuadraticFunction() const;

    /// Return the nonlinear part of the function.
    virtual NonlinearFunctionPtr getNonlinearFunction() const;

    /// Return a begin-iterator for the variables that are in this function.
    VarSetConstIterator varsBegin();

    /// Return an end-iterator for the variables that are in this function.
    VarSetConstIterator varsEnd();

    /// Return the number of variables that are in this function
    virtual UInt getNumVars() const;

    /**
     * Evaluate the hessian at a given point 'x'. Multiply the evaluated
     * values by the lagrangean multiplier 'mult' and add the given matrix
     * to the existing values in 'values'
     */
    virtual void evalHessian(const double mult, const double *x, 
                             const size_t *offset, double *values , int *error);
    virtual void evalHessian(double mult, const double *x, 
                             const LTHessStor *stor, double *values , int *error);


    /// Fill in the values of offset, starting from position pos. 
    virtual void fillHessOffset(size_t *offset, size_t &pos, 
                                std::set<ConstVariablePair, CompareVariablePair> & v_pairs);

    virtual void  fillHessStor(LTHessStor *stor);
    virtual void  finalHessStor(const LTHessStor *stor);

    /// Change the linear function part
    virtual void changeLf(LinearFunctionPtr lf);

    /// Change the nonlinear function part
    virtual void changeNlf(NonlinearFunctionPtr nlf);

    /// Substitute a variable with another.
    virtual void subst(VariablePtr out, VariablePtr in, double rat);

    /**
     * Multiply with a constant. If c is zero, then function becomes empty. We
     * make the components NULL in such a case
     */
    virtual void operator*=(const double c);

    /**
     * Remove the quadratic part of the function and return a pointer to the
     * quadratic part.
     */
    QuadraticFunctionPtr removeQuadratic();
 
    /**
     * Remove the linear part of the function and return a pointer to the
     * linear part.
     */   

    LinearFunctionPtr removeLinear();

    /**
     * Remove the nonlinear part of the function and return a pointer to 
     * the nonlinear part.
     */
    NonlinearFunctionPtr removeNonlinear();

    /**
     * Add a linear function. If the function already has a linear function,
     * the new function is algebraically added, otherwise the new function is
     * cloned.
     */
    void add(ConstLinearFunctionPtr lPtr);

    void removeVar(VariablePtr v, double val);
    double getFixVarOffset(VariablePtr v, double val);

    /// Express the function and send the output to the output stream "out".
    friend std::ostream &operator<<(std::ostream &out, const Function &f);

    /// Express the function and send the output to the output stream "out".
    virtual void write(std::ostream &out) const;

  protected:
    /// The type of the function.
    FunctionType type_;

    /// The variables that occur in this function
    VariableSet vars_;

    private:

    /// The linear function part of the function. Could be NULL.
    LinearFunctionPtr lf_;

    /// The nonlinear function part of the function. Could be NULL.
    NonlinearFunctionPtr nlf_;

    /// The quadratic function part of the function. Could be NULL.
    QuadraticFunctionPtr qf_;

    /**
     * Collect all the variables from the components into a single vector.
     * Duplicates are removed.
     */
    void collectVars_();

    //
    //void createHessMap_();
  };

}
#endif

