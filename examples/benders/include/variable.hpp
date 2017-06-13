#ifndef VARIABLE_HH_
#define VARIABLE_HH_

#include <assert.h>
#include "genobj.hpp"
#include "uuid.hpp"
#include "type.hpp"

using namespace std;
using Type::double;
using Type::INF;
using Type::EPSILON;


// all the internale representation of a variable is a continuous variable
class variable : public genobj{
protected:
  bool   active_;
  short  status_;  // -1: undefined, 0, fixed, 1, normal
  
  double value_, lb_, ub_, margin_;
  
  bool   check(const double &v, const short p=0);
  void   defaultInit();
public:
  variable();
  variable(const double &lb, const double &ub);
  
  void   on() {active_ = true;};
  void   off(){active_ = false;};
  bool   active()const{return active_;};
  
  void   fix(){status_ = 0; lb_ = ub_ = value_;};
  void   fix(const double &val){status_ = 0; lb_ = ub_ = value_ = val;};
  bool   fixed()const{return status_ == 0 ? true : false;};
  void   free(){status_ = 1; lb_ = -INF; ub_ = INF;};
  void   free(const double lb, const double ub){status_ = 1; lb_ = lb; ub_ = ub;};
  
  void   reset(){on(); free();};
  double value()const{assert(status_ !=-1); return value_;};
  bool   value(const double& val, const bool force = false);
  
  double margin()const{return margin_;};
  void   margin(const double v){margin_ = v;};
  
  double lb()const{return lb_;};
  double ub()const{return ub_;};
  void   lb(const double& b);
  void   ub(const double& b);
  
  void   bounds(const double& lb, const double& ub);
};

#endif
