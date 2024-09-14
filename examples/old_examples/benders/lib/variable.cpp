#include "variable.hpp"

void variable::defaultInit()
{
  active_ = true;
  status_ = 1;
  lb_     = -INF;
  ub_     =  INF;
}

bool variable::check(const double& v, const short p)
{
  bool stat = true;
  switch(p){
  case -1:			// check lb
    if(lb_ - v > EPSILON) stat = false; 
    break;
  case  1:			// check ub
    if(v - ub_ > EPSILON) stat = false; 
    break;
  default:			// check both lb and ub
    if(lb_ - v > EPSILON or v - ub_ > EPSILON) stat = false; 
    break;
  }
  
  return stat;
}

variable::variable()
  :genobj(0,"var")
{
  defaultInit();
}

variable::variable(const double &lb, const double &ub)
  :genobj(0,"var")
{
  assert(ub >= lb);
  lb_ = lb;
  ub_ = ub;
}

bool variable::value(const double& val, const bool force)
{
  bool stat=check(val); 	// whether it is within the bounds
  
  if(force || stat){
    value_  = val; 
    status_ = 1;
  }
  return stat;
}

void variable::lb(const double& b)
{
  assert(b <= ub_);
  lb_ = b;
}

void variable::ub(const double& b)
{
  assert(b >= lb_);
  ub_ = b;
}

void variable::bounds(const double &lb, const double &ub)
{
  assert(ub >= lb);
  lb_ = lb;
  ub_ = ub;
}
