// this is the class for a linear functions, which constains some variables,
// and every variable has a coefficients. Note that there is no constant for
// the the linear function. In order to deal with the possible constant,
// constraint should be involved.
// ============================================================
// the main function of this class:
// 1. represent the feasible and infeasbile cut
// 2. convert to/from constraints; difference with constraints

#ifndef LINEARFUNC_HH_
#define LINEARFUNC_HH_

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <list>
#include <assert.h>
#include <math.h>
#include "sparsevector.hpp"
#include "variablehandler.hpp"

using namespace std;
using Type::double;
using Type::EPSILON;
class linearfunc : public genobj{
private:
  const variableHandler* pvar_;
  sparseVector           coef_;
  
public:
  linearfunc()
    :genobj(0,"slf"), pvar_(NULL){};
  linearfunc(const variableHandler* const pvar)
    :genobj(0,"slf"), pvar_(pvar), coef_(pvar->size()){};
  
  linearfunc(const variableHandler* const pvar,
	     const sparseVector &coef)
    :genobj(0,"slf"), pvar_(pvar), coef_(coef){};
  
  linearfunc(const variableHandler* const pvar,
	     const vector<size_t>    &indx,
	     const vector<double>    &coef)
    :genobj(0,"slf"), pvar_(pvar), coef_(pvar->size(), indx, coef){};
  
  size_t   size()const{return pvar_->size();}; // number of total vars
  
  istream& read(istream& fin,      const bool sparse = true);
  size_t   read(const string& str, const bool sparse = true);
  
  void     extract(vector<size_t> &idx, vector<double> &val)const;
  
  linearfunc sublf(const size_t k, const variableHandler* const pv);
  
  friend ostream& operator<<(ostream& os, const linearfunc& lf);
  
  double value()const;
  // ========================================
  // compute the value of the linear function
  // for the active variables only
  
  linearfunc  operator  *(const double &c)      
  {linearfunc lf(*this); lf*= c; return lf;};
  linearfunc& operator *=(const double &c)      {coef_ *= c;        return *this;};
  linearfunc& operator +=(const linearfunc& lfb){coef_ += lfb.coef_;return *this;};
  linearfunc& operator -=(const linearfunc& lfb){coef_ -= lfb.coef_;return *this;};
  
};
#endif
