// GTG ==========================================================
// class of objective; it is just a constraint, the only thing
// special for the objective is:
// 1. the coef for the last variable is always -1
// 2. initally, there is only one bound specified
// GTG ==========================================================

#ifndef OBJECTIVE_HH_
#define OBJECTIVE_HH_

// ==============================================
// this is the class for a typical objective like
// constaint as: z >= Ax + b;
// acutally this is another constaint as
// z >= Ax + b: -INF <= -z + Ax <= -b
// z <= Ax + b:   -b <= -z + Ax <=  INF
// z == Ax + b:   -b <= -z + Ax <= -b
// ==============================================

#include <vector>
#include <list>
#include <assert.h>
#include "constraint.hpp"
#include "linearfunc.hpp"
#include "variable.hpp"
#include "type.hpp"

using namespace std;
using Type::Double;
using Type::EPSILON;

class objective :public genobj{
private:
  bool              active_;
  constraint        cons_;
public:
  objective(const variableHandler* const pvar)
    :genobj(0,"obj"), active_(false), cons_(pvar){cons_.ub(0);};
  
  objective(const short             &relation,
	    const linearfunc        &lf,
	    const Double            &shift = 0.0);
  
  objective(const short             &relation,
	    const variableHandler* const pvar,
	    const vector<size_t>    &indx,
	    const vector<Double>    &coef, 
	    const Double            &shift = 0.0);
  
  bool   active()const{return active_;};
  void   on() {active_ = true;};
  void   off(){active_ = false;};
  
  void   reset(){on();};
  
  Double lb()const{return cons_.lb();};
  Double ub()const{return cons_.ub();};
  
  linearfunc sub(const size_t k, const variableHandler* const pv)
  {return cons_.sub(k, pv);};
  
  void   extract(vector<size_t> &idx, vector<Double> &val)const;
  
  // Double value(const short rule = 1)const
  // {return cons_.value();}
  // compute the value of the objective function
  // ========================================
  // pass the value request to linearfunc, and add
  // the const shift or not indicated by flag "f";
  // rule =  0 : all the off() variables
  // rule =  1 : all the active variables are counted
  
};
  
#endif
