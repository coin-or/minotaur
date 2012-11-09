#include "objective.hpp"

objective::objective(const short             &relation,
		     const linearfunc        &lf,
		     const Double            &shift)
  :genobj(0,"obj"), active_(true), cons_(lf)
{
  Double lb = -INF, ub = INF;
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
		     const vector<Double>    &coef, 
		     const Double            &shift)
  :genobj(0,"obj"), active_(true), cons_(pvar, indx, coef)
{
  Double lb = -INF, ub = INF;
  if(relation > 0){
    ub = -shift;
  }else if(relation < 0){
    lb = -shift;
  }else{
    lb = ub = -shift;
  }
  cons_.bounds(lb, ub);
}

void objective::extract(vector<size_t> &idx, vector<Double> &val)const
{
  cons_.extract(idx, val);
  
  idx.push_back(cons_.size());
  val.push_back(-1);
}
