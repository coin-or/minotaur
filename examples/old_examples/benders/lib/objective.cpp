#include "objective.hpp"

objective::objective(const short             &relation,
		     const linearfunc        &lf,
		     const double            &shift)
  :genobj(0,"obj"), active_(true), cons_(lf)
{
  double lb = -INF, ub = INF;
  if(relation > 0){
    ub = -shift;
  }else if(relation < 0){
    lb = -shift;
  }else{
    lb = ub = -shift;
  }
  
  cons_.bounds(lb, ub);
}


objective::objective(const short             &relation,
		     const variableHandler* const pvar,
		     const vector<size_t>    &indx,
		     const vector<double>    &coef, 
		     const double            &shift)
  :genobj(0,"obj"), active_(true), cons_(pvar, indx, coef)
{
  double lb = -INF, ub = INF;
  if(relation > 0){
    ub = -shift;
  }else if(relation < 0){
    lb = -shift;
  }else{
    lb = ub = -shift;
  }
  cons_.bounds(lb, ub);
}

void objective::extract(vector<size_t> &idx, vector<double> &val)const
{
  cons_.extract(idx, val);
  
  idx.push_back(cons_.size());
  val.push_back(-1);
}
