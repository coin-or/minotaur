// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file Solution.h
 * \brief Define base class Solution.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 * 
 * Define the base class Solution. 
 */


#ifndef MINOTAURSOLUTION_H
#define MINOTAURSOLUTION_H

#include "Types.h"

namespace Minotaur {

  class Solution;
  typedef Solution* SolutionPtr;
  typedef const Solution* ConstSolutionPtr;
  typedef std::vector<SolutionPtr> SolutionPtrVector;
  typedef std::vector<SolutionPtr>::iterator SolutionIterator;
  class Problem;

  class Solution {    

  public:
    /// Default constructor. 
    Solution();

    /// Construct a solution for a problem from an array of doubles.
    Solution(double objval, const double *x, ProblemPtr problem);

    /// Construct a solution for a problem from a vector of doubles.
    Solution(double objvalue, const DoubleVector &newx, 
             ProblemPtr problem); 

    /// Copy.
    Solution(ConstSolutionPtr sol);

    /// Destroy.
    virtual ~Solution();

    /// Copy values of variables.
    virtual void setPrimal(const double *x);

    /// Copy values of dual variables of constraints.
    virtual void setDualOfCons(const double *vals);

    /// Copy values of dual variables of variables.
    virtual void setDualOfVars(const double *vals);

    /// Set a new solution value.
    virtual void setObjValue(double new_val) {objValue_ = new_val;};

    /// Return solution value.
    virtual double getObjValue() const {return objValue_;};

    /// Return the 2nd norm of constraint violations.
    virtual double getConstraintViolation() {return consViol_;};

    /// Return a pointer to the solution.
    virtual const double * getPrimal() const {return x_;};

    /// Return a pointer to the solution.
    virtual const double * getDualOfCons() const {return dualCons_;};

    /// Return a pointer to the solution.
    virtual const double * getDualOfVars() const {return dualX_;};

    /// Write to a stream.
    virtual void write(std::ostream &out) const;

    /// Write primal values to a stream.
    virtual void writePrimal(std::ostream &out, const VarVector *v=0) const;

    /// Write dual values to a stream.
    virtual void writeDual(std::ostream &out) const;

  protected:
    /// Number of variables.
    UInt n_;

    /// Number of constraints.
    UInt m_;

    /// Original problem (For accessing variable names).
    ProblemPtr problem_;

    /// Store values of the variables in an array.
    double *x_;

    /// Store values of the dual variables associated with constraints 
    /// in an array.
    double *dualCons_;

    /// Store values of the dual variables associated with bounds on variables 
    /// in an array.
    double *dualX_;

    /// Violation.
    double consViol_;

    /// The objective value of this solution.
    double objValue_;

    /// Complementarity.
    double comple_;
  };
}
#endif

