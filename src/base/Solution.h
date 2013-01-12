// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2013 The MINOTAUR Team.
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
  typedef boost::shared_ptr<Solution> SolutionPtr;
  typedef boost::shared_ptr<const Solution> ConstSolutionPtr;
  typedef std::vector<SolutionPtr> SolutionPtrVector;
  typedef std::vector<SolutionPtr>::iterator SolutionIterator;
  class Problem;
  typedef boost::shared_ptr<Problem> ProblemPtr;

  class Solution {    

    public:
      /// Default constructor. 
      Solution();

      /// Construct a solution for a problem from an array of doubles.
      Solution(Double objval, const Double *x, ProblemPtr problem);

      /// Construct a solution for a problem from a vector of doubles.
      Solution(Double objvalue, const DoubleVector &newx, 
               ProblemPtr problem); 

      /// Copy.
      Solution(ConstSolutionPtr sol);

      /// Destroy.
      virtual ~Solution();

      /// Copy values of variables.
      virtual void setPrimal(const Double *x);

      /// Copy values of dual variables of constraints.
      virtual void setDualOfCons(const Double *vals);

      /// Copy values of dual variables of variables.
      virtual void setDualOfVars(const Double *vals);

      /// Set a new solution value.
      virtual void setObjValue(Double new_val) {objValue_ = new_val;};

      /// Return solution value.
      virtual Double getObjValue() const {return objValue_;};

      /// Return the 2nd norm of constraint violations.
      virtual Double getConstraintViolation() {return consViol_;};

      /// Return a pointer to the solution.
      virtual const Double * getPrimal() const {return x_;};

      /// Return a pointer to the solution.
      virtual const Double * getDualOfCons() const {return dualCons_;};

      /// Return a pointer to the solution.
      virtual const Double * getDualOfVars() const {return dualX_;};

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
      Double *x_;

      /// Store values of the dual variables associated with constraints 
      /// in an array.
      Double *dualCons_;

      /// Store values of the dual variables associated with bounds on variables 
      /// in an array.
      Double *dualX_;

      /// Violation.
      Double consViol_;

      /// The objective value of this solution.
      Double objValue_;

      /// Complementarity.
      Double comple_;
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
