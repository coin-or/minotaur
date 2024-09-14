#include "linearfunc.hpp"

istream& linearfunc::read(istream& fin, const bool sparse)
{
  string entry; 
  getline(fin, entry);
  coef_.read(entry, sparse);
  return fin;
}

size_t linearfunc::read(const string& str, const bool sparse)
{
  return coef_.read(str, sparse);
}

void linearfunc::extract(vector<size_t> &idx, vector<double> &val)const 
{
  coef_.extract(idx,val);
}

linearfunc linearfunc::sublf(const size_t k, const variableHandler* const psv)
{
  sparseVector  cc(coef_.subv(k));
  linearfunc sublf(psv, cc);
  return sublf;
}

ostream& operator<<(ostream& os, const linearfunc& lf)
{
  os << "LF:" << lf.coef_ << endl;
  return os;
}

double linearfunc::value()const
{
  double v = 0.0;
  
  vector<size_t> idx;
  vector<double> val;
  
  extract(idx, val);
  
  for(size_t i=0;i<idx.size();i++){
    v += val[i] * (*pvar_)[i]->value();
  }
  
  return v;
}

// linearfunc& linearfunc::operator *=(const double &c)
// {
//   size_t n = this->size();
//   for(size_t i=0;i<n;i++) coef_[i] *= c;
//   
//   return *this;
// }
// 
// linearfunc& linearfunc::operator +=(const linearfunc& lfb)
// {
//   size_t n = this->size();
//   assert(n == lfb.size());
//   vector<double> bc = lfb.coef();
//   for(size_t i=0;i<n;i++){
//     coef_[i] += bc[i];
//   }
//   return *this;
// }
// 
// linearfunc& linearfunc::operator -=(const linearfunc& lfb)
// {
//   size_t n = this->size();
//   assert(n == lfb.size());
//   vector<double> bc = lfb.coef();
//   for(size_t i=0;i<n;i++){
//     coef_[i] -= bc[i];
//   }
//   return *this;
// }
// 
// 
// bool linearfunc::check(const short rule)const
// {
//   size_t i, n = size();
//   bool active = true;		// default is true
//   switch (rule){
//   case 0:			// off is all variables are inactive 
//     for(i=0;i<n;i++){
//       // if((vars_[i])->active() && fabs(coef_[i]) > EPSILON) break;
//       if((vars_[i])->active()) break;
//     }
//     if(i==n) active = false;
//     break;
//   case 1:
//     for(i=0;i<n;i++){
//       if(! vars_[i]->active()) {
// 	active = false;
// 	break;
//       }
//     }
//     break;
//   }
//   return active;
// }
// 
// void linearfunc::prune()
// {
//   vector<double>    coef;
//   vector<variable*> vars;
//   size_t n = vars_.size();
//   
//   for(size_t i=0;i<n;i++){
//     if((vars_[i])->active()){
//       coef.push_back(coef_[i]);
//       vars.push_back(vars_[i]);
//     };
//   }
//   coef_ = coef;
//   vars_ = vars;
// }
