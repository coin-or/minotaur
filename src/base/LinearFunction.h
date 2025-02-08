//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file LinearFunction.h
 * \author Ashutosh Mahajan, Argonne National Laboratory.
 * \brief Declare the class LinearFunction for storing and modifying a
 * linear function.
 */

#ifndef MINOTAURLINEARFUNCTION_H
#define MINOTAURLINEARFUNCTION_H

#include "Types.h"

namespace Minotaur {
  class LinearFunction;
  class QuadraticFunction;
  class Variable;

  typedef LinearFunction *LinearFunctionPtr;
  typedef const LinearFunction *ConstLinearFunctionPtr;
  typedef QuadraticFunction *QuadraticFunctionPtr;
  typedef const QuadraticFunction *ConstQuadraticFunctionPtr;


  /// The base class linear function is of the form c'x.
  class LinearFunction {
  public:
    /// Default constructor
    LinearFunction();

    /**
     * Constructor with a tolerance level below which a coefficient is
     * considered to be zero.
     */
    LinearFunction(const double tol);

    /**
     * Construct a linear function from a coefficient array of size n and n
     * variables.
     */
    LinearFunction(double *a, VariableConstIterator vbeg,
                   VariableConstIterator vend, double tol);


    /// Destroy
    ~LinearFunction();

    void add(LinearFunctionPtr lf);

    void add(ConstLinearFunctionPtr lf);

    /**
     * Add new a linear term to this linear function, with coefficient a. Use
     * this method only when you are sure that the linear function does not
     * already contain this variable. Otherwise use incTerm(). If the weight
     * "a" is zero, then nothing is added.
     */
    void addTerm(ConstVariablePtr var, const double a);

    /**
     * Removes all terms from the function
     */
    void clearAll();

    /**
     * Copy the linear function. Variables and weights are copied. The weights
     * in the clone and the original do not share the same space in memory.
     */
    LinearFunctionPtr clone() const;

    LinearFunctionPtr cloneWithVars(VariableConstIterator vbeg) const;

    LinearFunctionPtr cloneWithVarsPermute(VariableConstIterator vbeg,
                                           UIntVector variableaddress) const;

    /**
     * \brief Get bounds based on lower and upperbounds of each variable
     * \param [out] l This pointer should contain lower bound.
     * \param [out] u This pointer should contain upper bound.
     */
    void computeBounds(double *l, double *u);

    /**
     * Evaluate the value of this linear function at a given vector of
     * doubles. It is assumed that the x[i] has the value for variable with id
     * 'i'.
     */
    double eval(const std::vector<double> &x) const;

    /**
     * Evaluate the value of this linear function at a given vector of
     * doubles. It is assumed that the x[i] has the value for variable with id
     * 'i'.
     */
    double eval(const double *x) const;

    /**
     * Evaluate the gradient of this linear function. It is assumed that x[id]
     * will have the gradient along the direction of variable with ID=id.
     */
    void evalGradient(double *grad_f) const;

    void fillJac(double *values, int *error);

    double getFixVarOffset(VariablePtr v, double val);

    /// Get the number of terms in this function.
    size_t getNumTerms() const { return (terms_.size()); }

    void getVars(VariableSet *vars);

    /**
     * Get the weight of a variable in this function. If the variable is not
     * found, it returns zero. Conversely, if the weight returned is zero,
     * then variable is not stored in the data structures of this function.
     */
    double getWeight(ConstVariablePtr var) const;

    /**
     * \brief Check if function contains a variable.
     *
     * \param[in] v The variable that we want to test.
     * \return True if this function is has v. False if it doesn't use it.
     */
    bool hasVar(ConstVariablePtr v) const;

    /**
     * Add new a linear term to this linear function, with coefficient a. If
     * the function already contains this variable, then the value is
     * incremented. If the new value becomes zero, the variable is dropped.
     */
    void incTerm(ConstVariablePtr var, const double a);

    /**
     * Document me.
     */
    void minus(LinearFunctionPtr lf);

    //void mult(LinearFunctionPtr lf);

    /// Multiply the linear function by a number.

    void multiply(double d);

    void prepJac(UInt s, VarSetConstIter vbeg, VarSetConstIter vend);

    /// Remove a variable v from the function.
    void removeVar(VariablePtr v, double val);

    /// Square the function.
    QuadraticFunctionPtr square();

    /// Iterate over the terms in the linear function: begin.
    VariableGroupConstIterator termsBegin() const;

    /// Iterate over the terms in the linear function: end.
    VariableGroupConstIterator termsEnd() const;

    /// Writes the function to a stream.
    void write(std::ostream &out) const;


    /**
     * Create a new LinearFunction by copying this function and adding another
     * one.
     */
    LinearFunctionPtr copyAdd(ConstLinearFunctionPtr l1);

    /**
     * Create a new LinearFunction by copying this function and subtracting
     * another one from it.
     */
    LinearFunctionPtr copyMinus(ConstLinearFunctionPtr l1);

    /**
     * Create a new LinearFunction by copying this function and multiplying
     * by a constant.
     */
    LinearFunctionPtr copyMult(const double c);

    /**
     * Create a new QuadraticFunction by multiplying this function and
     * another LinearFunction.
     */
    QuadraticFunctionPtr copyMult(ConstLinearFunctionPtr l1);

  private:
    /**
     * True if terms in linear function are modified since previous call to
     * prepJac.
     */
    bool hasChanged_;

    /// Offsets for jacobian.
    DoubleVector off_;

    /**
     * terms_ is a map with variables as keys and their coefficients as
     * values.
     */
    VariableGroup terms_;

    /// Tolerance below which a coefficient is considered 0.
    double tol_;

    /// Copy constructor is not allowed.
    LinearFunction(const LinearFunction &l);

    /// Copy by assignment is not allowed.
    LinearFunction &operator=(const LinearFunction &l);
  };
}  //namespace Minotaur
#endif
