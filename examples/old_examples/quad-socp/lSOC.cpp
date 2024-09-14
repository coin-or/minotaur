// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 -- The MINOTAUR Team
// 

#include "MinotaurConfig.h"
#include "lSOC.h"

using namespace Minotaur;

LSOC::LSOC()
  : l0_(LinearFunctionPtr()),
    sqrtK_(0.0),
    K_(0.0)
{
  sqrtTerms_.clear();
  sqrtKs_.clear();
  vars_.clear();
}


LSOC::LSOC(std::vector<LinearFunctionPtr> & p_terms, LinearFunctionPtr n_term, 
    std::vector<double> & p_const, double & n_const, double sqrtK)
  : l0_(n_term),
    sqrtK_(sqrtK),
    K_(n_const)
{
  sqrtTerms_.insert(sqrtTerms_.begin(), p_terms.begin(), p_terms.end());
  sqrtKs_.insert(sqrtKs_.begin(), p_const.begin(), p_const.end());
  vars_.clear();
  for (UInt i=0; i<sqrtKs_.size(); i++) {
    vars_.insert(sqrtTerms_[i]->varsBegin(), sqrtTerms_[i]->varsEnd());
  }
  vars_.insert(l0_->varsBegin(), l0_->varsEnd());
}


double LSOC::eval(const double *x) const
{
  double sqrtVal = 0;
  UInt i=0;
  for (i=0; i<sqrtKs_.size(); i++) {
    sqrtVal += pow(sqrtTerms_[i]->eval(x) + sqrtKs_[i], 2.0);
  }
  //std::cout << "x[0] = " << x[0] << ", x[1] = " << x[1] << std::endl;
  //std::cout << "evaluation = " << sqrt(sqrtVal + sqrtK_) - l0_->eval(x) << std::endl;
  //std::cout << "evaluation of l0 = " << l0_->eval(x) << std::endl;
  //l0_->write(std::cout);
  //std::cout << "evaluation of sqrtVal = " << sqrtVal << std::endl;
  //std::cout << "evaluation of sqrtK_ = " << sqrtK_ << std::endl;
  return sqrt(sqrtVal + sqrtK_) - l0_->eval(x);
}


FunctionType LSOC::getType() 
{
  return Quadratic;
}


void LSOC::evalGradient(const double *x, double *grad_f) const
{
  double sqrtVal = 0;
  double lval = 0;
  UInt i=0;
  for (i=0; i<sqrtKs_.size(); i++) {
    sqrtVal += pow(sqrtTerms_[i]->eval(x) + sqrtKs_[i], 2.0);
  }
  sqrtVal = sqrt(sqrtVal + sqrtK_);
  // visit each sq term
  for (i=0; i<sqrtKs_.size(); i++) {
    // evaluate the linear term.
    lval = sqrtTerms_[i]->eval(x) + sqrtKs_[i];
    // update gradient of each variable in this function
    for (VariableGroupConstIterator t=sqrtTerms_[i]->termsBegin(); t!=sqrtTerms_[i]->termsEnd(); t++) {
      ConstVariablePtr v = (t)->first;
      grad_f[v->getId()] += lval*t->second/sqrtVal;
    }
  }

  // visit the linear terms
  for (VariableGroupConstIterator t=l0_->termsBegin(); t!=l0_->termsEnd(); t++) {
    ConstVariablePtr v  = (t)->first;
    grad_f[v->getId()] -= t->second;
  }

  //std::cout << "sqrt = " << sqrtVal << std::endl;
  //for (i=0; i<2; i++) {
  //  std::cout << "var[" << i << "] has value = " << x[i] << std::endl;
  //  std::cout << "var[" << i << "] has gradient = " << grad_f[i] << std::endl;
  //}

}


//UInt LSOC::getNumNzInHess()
//{
  // not needed.
  // vp_inds.insert(make_pair(iter->first, 0));
 // return 0;
//}


void LSOC::getHessVarPairs( VarPairIntMap & vp_inds)
{
  for (VarSetConstIterator iter=vars_.begin(); iter!=vars_.end(); 
      iter++) {
    for (VarSetConstIterator iter2=iter; iter2!=vars_.end(); 
        iter2++) {
      vp_inds.insert(std::make_pair(std::make_pair((*iter), (*iter2)), 0));
    }
  }
}


void LSOC::evalHessian(const double mult, const double *x, 
    VarPairIntMap &vp_inds, double *values)
{
  double sumOfSq = 0;
  for (UInt i=0; i<sqrtKs_.size(); i++) {
    sumOfSq += pow(sqrtTerms_[i]->eval(x) + sqrtKs_[i], 2.0);
  }
  sumOfSq += sqrtK_;

  // visit each variable
  for (VarSetConstIterator iter=vars_.begin(); iter!=vars_.end(); 
      iter++) {

    double di = 0;
    for (UInt l=0; l<sqrtKs_.size(); l++) {
      ConstLinearFunctionPtr lPtr = sqrtTerms_[l];
      di += lPtr->getWeight(*iter)*lPtr->eval(x);
    }

    // visit each variable again
    for (VarSetConstIterator iter2=iter; iter2!=vars_.end(); iter2++) {
      // caculate H_{ij} for these 2 variables
      double hij = 0;
      double dj = 0;
      // visit each linear term and search for variables i,j
      for (UInt l=0; l<sqrtKs_.size(); l++) {
        ConstLinearFunctionPtr lPtr = sqrtTerms_[l];
        hij += lPtr->getWeight(*iter)*lPtr->getWeight(*iter2);
        dj += lPtr->getWeight(*iter2)*lPtr->eval(x);
      }
      hij = hij/sqrt(sumOfSq);
      hij -= di*dj/pow(sumOfSq, 1.5);
      VariablePair vp = std::make_pair((*iter), (*iter2));
      values[vp_inds[vp]] += mult*hij;
      //std::cout << "d[" << (*iter)->getId() << "] = "<< di << std::endl; 
      //std::cout << "d[" << (*iter2)->getId() << "] = "<< dj << std::endl; 
      //std::cout << "H[" << (*iter)->getId() << "," << (*iter2)->getId() << "] = " << hij << std::endl; 
      //std::cout << "vp_inds[vp] = " << vp_inds[vp] << std::endl;
    }
  }
}


void LSOC::write(std::ostream &out) const
{
  UInt i=0;
  out << " sqrt( ";
  for (std::vector<LinearFunctionPtr>::const_iterator it = sqrtTerms_.begin();
      it!=sqrtTerms_.end(); it++) {
    out << "(";
    (*it)->write(out);
    if (sqrtKs_[i]<0) {
      out << sqrtKs_[i];
    } else {
      out << "+ " << sqrtKs_[i];
    }
    out << ")^2";
    i++;
  }
  if (sqrtK_ < 0) {
    out << sqrtK_;
  } else {
    out << "+ " << sqrtK_;
  }
  out << ")" ;

  out << " - (";
  l0_->write(out);
  if (K_<0) {
    out << K_; 
  } else {
    out << "+ " << K_;
  }
  out << ")";
}

