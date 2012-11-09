// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
// 

#include "MinotaurConfig.h"
#include "AlgebraicConstraint.h"

using namespace Minotaur;

#define DEBUG_CONSTRAINT 0

AlgebraicConstraint::AlgebraicConstraint() : f_(0), lb_(0), ub_(0)
{
}

AlgebraicConstraint::AlgebraicConstraint(FunctionPtr f, Double lb, Double ub) :
	    f_(f), lb_(lb), ub_(ub)
{
}

AlgebraicConstraint::~AlgebraicConstraint()
{
}

void
AlgebraicConstraint::getSlackVals(std::vector<boost::tuple<ConstVariablePtr, Double> >& varvals,
    Double &lbslack, Double &ubslack) const
{    
  Double fval = f_.eval(varvals);  
  lbslack = fval - lb;
  ubslack = ub - fval;
}

Double 
AlgebraicConstraint::getViolation(std::vector<boost::tuple<ConstVariablePtr, Double> >
	&varvals) const
{
  Double lbslack, ubslack;
  getSlackVals(varvals, lbslack, ubslack);
  Double minslack = (lbslack < ubslack) ? lbslack : ubslack;
  return (minslack < 0) ? -minslack : 0.0;
}


void
AlgebraicConstraint::print(std::ostream &s) const
{
  s << "lb = " << lb_ << endl;
  s << "ub = " << ub_ << endl;
  f_->print(s);
}

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
