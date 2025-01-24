//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqMonomial.cpp
 * \brief Define class for storing auxiliary variables for monomial
 * expressions.
 * \author Ashutosh Mahajan, Argonne National Laboratory.
 */

#include <cmath>

#include "MinotaurConfig.h"

#include "PolynomialFunction.h"
#include "Variable.h"
#include "YEqMonomial.h"

using namespace Minotaur;

YEqMonomial::YEqMonomial(UInt n)
{
  n_ = n;
  for (UInt i=0; i<n_; ++i) {
    rand_.push_back((double) rand()/(RAND_MAX));
  }
}


double YEqMonomial::evalHash_(MonomialFunPtr mf)
{
  double hash = 0.0;
  for (VarIntMapConstIterator it=mf->termsBegin(); it!=mf->termsEnd(); ++it) {
    hash += ((double) it->second)/(1.0+it->first->getId());
  }
  return hash;
}


VariablePtr YEqMonomial::findY(MonomialFunPtr mf)
{
  double hash = evalHash_(mf);
  VarIntMapConstIterator it, it2;
  bool found;

  for (UInt i=0; i<y_.size(); ++i) {
    if (fabs(hash-hash_[i])<1e-12 
        && mf->getDegree()==mf_[i]->getDegree()
        && fabs(mf->getCoeff()-mf_[i]->getCoeff())<1e-12) {
      it = mf->termsBegin();
      it2 = mf_[i]->termsBegin();
      found = true;
      for (; it!=mf->termsEnd(); ++it, ++it2) {
        if (it->first != it2->first || it->second != it2->second) {
          found = false;
          break;
        }
      }
      if (found) {
        return y_[i];
      }
    }
  }
  return VariablePtr();
}


void YEqMonomial::insert(VariablePtr auxvar, MonomialFunPtr mf)
{
  hash_.push_back(evalHash_(mf));
  y_.push_back(auxvar);
  mf_.push_back(mf);
}


