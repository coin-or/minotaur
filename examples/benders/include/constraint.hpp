// =============================================
// this is the class for a typical constraint as:
// lb <= Ax  <= ub. note that there is always an
// extra variale z as the last variable, but the
// variables does not go into the linearfunc, it
// shows up when extract the coefficients.
// =============================================

#ifndef CONSTRAINT_HH_
#define CONSTRAINT_HH_

#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <assert.h>
#include "genobj.hpp"
#include "linearfunc.hpp"
#include "variable.hpp"
#include "type.hpp"
#include "agent.hpp"

using namespace std;
using Type::Double;
using Type::EPSILON;
using Type::INF;
class constraint:public genobj{
private:
  bool             active_;
  
  linearfunc       func_;
  Double           lb_, ub_;	// lower and upper bound
  Double           level_, margin_;
  
public:
  constraint(const variableHandler* const pvar)
    :genobj(0,"con"), active_(false), func_(pvar), lb_(-INF), ub_(INF){};
  
  constraint(const linearfunc        &func,
	     const Double            &lb = -INF,
	     const Double            &ub =  INF)
    :genobj(0,"con"), active_(true), func_(func), lb_(lb), ub_(ub){};
  
  constraint(const variableHandler*  const pvar,
	     const vector<size_t>    &indx,
	     const vector<Double>    &coef, 
	     const Double            &lb = -INF,
	     const Double            &ub =  INF)
    :genobj(0,"con"), active_(true), func_(pvar, indx, coef), lb_(lb), ub_(ub){};
  
  bool   active()const{return active_;};
  void   on() {active_ = true; };
  void   off(){active_ = false;};
  
  size_t size()const{return func_.size();};
  bool   set_status(const short rule = 0); // on or off
  void   reset(){on();};  
  
  void   extract(vector<size_t> &idx, vector<Double> &val)const;
  
  linearfunc lf()const{return func_;};
  
  // Double value(const short rule = 1)const{return func_.value(rule);};
  
  Double margin()const{return margin_;};
  void   margin(const Double& m){margin_ = m;};

  Double level()const{return level_;};
  void   level(const Double& l){level_ = l;};
  
  Double lb()const{return lb_;};
  Double ub()const{return ub_;};
  void   lb(const Double& b);
  void   ub(const Double& b);
  
  void   bounds(const Double& lb, const Double& ub);
  
  linearfunc sub(const size_t k, const variableHandler* const pv)
  {return func_.sublf(k,pv);};
  // ============================================================
  // return a linear function which only contains the variables
  // from idex ia to iz;
};
#endif
