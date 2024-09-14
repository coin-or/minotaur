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
using Type::double;
using Type::EPSILON;
using Type::INF;
class constraint:public genobj{
private:
  bool             active_;
  
  linearfunc       func_;
  double           lb_, ub_;	// lower and upper bound
  double           level_, margin_;
  
public:
  constraint(const variableHandler* const pvar)
    :genobj(0,"con"), active_(false), func_(pvar), lb_(-INF), ub_(INF){};
  
  constraint(const linearfunc        &func,
	     const double            &lb = -INF,
	     const double            &ub =  INF)
    :genobj(0,"con"), active_(true), func_(func), lb_(lb), ub_(ub){};
  
  constraint(const variableHandler*  const pvar,
	     const vector<size_t>    &indx,
	     const vector<double>    &coef, 
	     const double            &lb = -INF,
	     const double            &ub =  INF)
    :genobj(0,"con"), active_(true), func_(pvar, indx, coef), lb_(lb), ub_(ub){};
  
  bool   active()const{return active_;};
  void   on() {active_ = true; };
  void   off(){active_ = false;};
  
  size_t size()const{return func_.size();};
  bool   set_status(const short rule = 0); // on or off
  void   reset(){on();};  
  
  void   extract(vector<size_t> &idx, vector<double> &val)const;
  
  linearfunc lf()const{return func_;};
  
  // double value(const short rule = 1)const{return func_.value(rule);};
  
  double margin()const{return margin_;};
  void   margin(const double& m){margin_ = m;};

  double level()const{return level_;};
  void   level(const double& l){level_ = l;};
  
  double lb()const{return lb_;};
  double ub()const{return ub_;};
  void   lb(const double& b);
  void   ub(const double& b);
  
  void   bounds(const double& lb, const double& ub);
  
  linearfunc sub(const size_t k, const variableHandler* const pv)
  {return func_.sublf(k,pv);};
  // ============================================================
  // return a linear function which only contains the variables
  // from idex ia to iz;
};
#endif
