// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2012 The MINOTAUR Team.
// 

#ifndef MINOTAURALGEBRAICCONSTRAINT_H
#define MINOTAURALGEBRAICCONSTRAINT_H

#include <iostream>

#include "Constraint.h"
#include "Function.h"

namespace Minotaur {

  class AlgebraicConstraint : public Constraint
  {
  public:
    AlgebraicConstraint();

    /// Create an algebraic constraint
    AlgebraicConstraint(FunctionPtr f, Double lb, Double ub);

    ~AlgebraicConstraint();
    
    /// feasible iff both slacks are nonnegative 
    //void getSlackVals(std::vector<boost::tuple<ConstVariablePtr, Double> >
//	&varvals, Double &lbslack, Double &ubslack) const;
   	    
    ConstFunctionPtr getFunction() const { return f_; };	    
    Double getUb() const { return ub_; }
    Double getLb() const { return lb_; }

    //Double getViolation(std::vector<boost::tuple<ConstVariablePtr, Double> >
//	&varvals) const;
    virtual void print(std::ostream &s) const;
    virtual void print() const { print(std::cout); }

  private:

    FunctionPtr f_;
    Double lb_;
    Double ub_;

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
