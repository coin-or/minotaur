#ifndef VARIABLE_HH_
#define VARIABLE_HH_

#include <assert.h>
#include "genobj.hpp"
#include "uuid.hpp"
#include "type.hpp"

using namespace std;
using Type::Double;
using Type::INF;
using Type::EPSILON;


// all the internale representation of a variable is a continuous variable
class variable : public genobj{
protected:
  bool   active_;
  short  status_;  // -1: undefined, 0, fixed, 1, normal
  
  Double value_, lb_, ub_, margin_;
  
  bool   check(const Double &v, const short p=0);
  void   defaultInit();
public:
  variable();
  variable(const Double &lb, const Double &ub);
  
  void   on() {active_ = true;};
  void   off(){active_ = false;};
  bool   active()const{return active_;};
  
  void   fix(){status_ = 0; lb_ = ub_ = value_;};
  void   fix(const Double &val){status_ = 0; lb_ = ub_ = value_ = val;};
  bool   fixed()const{return status_ == 0 ? true : false;};
  void   free(){status_ = 1; lb_ = -INF; ub_ = INF;};
  void   free(const Double lb, const Double ub){status_ = 1; lb_ = lb; ub_ = ub;};
  
  void   reset(){on(); free();};
  Double value()const{assert(status_ !=-1); return value_;};
  bool   value(const Double& val, const bool force = false);
  
  Double margin()const{return margin_;};
  void   margin(const Double v){margin_ = v;};
  
  Double lb()const{return lb_;};
  Double ub()const{return ub_;};
  void   lb(const Double& b);
  void   ub(const Double& b);
  
  void   bounds(const Double& lb, const Double& ub);
};

#endif
