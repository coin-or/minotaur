#include "constraint.hpp"

void constraint::extract(vector<size_t> &idx, vector<double> &val)const
{
  func_.extract(idx,val);
}

void constraint::lb(const double& b)
{
  assert(b <= ub_);
  lb_ = b;
}

void constraint::ub(const double& b)
{
  assert(b >= lb_);
  ub_ = b;
}

void constraint::bounds(const double &lb, const double &ub)
{
  assert(ub >= lb);
  lb_ = lb;
  ub_ = ub;
}

