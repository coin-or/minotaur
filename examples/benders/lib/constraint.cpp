#include "constraint.hpp"

void constraint::extract(vector<size_t> &idx, vector<Double> &val)const
{
  func_.extract(idx,val);
}

void constraint::lb(const Double& b)
{
  assert(b <= ub_);
  lb_ = b;
}

void constraint::ub(const Double& b)
{
  assert(b >= lb_);
  ub_ = b;
}

void constraint::bounds(const Double &lb, const Double &ub)
{
  assert(ub >= lb);
  lb_ = lb;
  ub_ = ub;
}

