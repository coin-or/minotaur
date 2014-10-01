// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
// 

/**
 * \file Operations.cpp
 * \brief Define some commonly used functions.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#include <cassert>
#include <cmath>
#include <iostream> 
#include <iomanip> 
#include <sstream>

#include "MinotaurConfig.h"
#include "Operations.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

Double Minotaur::InnerProduct(const VariableGroup &v1, const VariableGroup &v2) 
{
  VariableGroup::const_iterator i1, i2, e1, e2;
  VariableGroup::key_compare compare = v1.key_comp();
  i1 = v1.begin();
  e1 = v1.end();
  i2 = v2.begin();
  e2 = v2.end();

  Double sum = 0.0;
  while ((i1 != e1) && (i2 != e2)) {
    if (compare(i1->first, i2->first)) {
      ++i1;
    } 
    else if (compare(i2->first, i1->first)) {
      ++i2;
    } 
    else {
      assert(i1->first == i2->first);
      sum += i1->second * i2->second;
      ++i1;
      ++i2;
    }
  }
  return sum;
}


//??? Should we just be using CoinPackedVector and such?
//  Ublas sparse vectors?
Double Minotaur::InnerProduct(const std::vector<Double> &x, 
    const VariableGroup &g)
{
  Double sum = 0;
  for (VariableGroup::const_iterator it = g.begin(); it != g.end(); ++it) {
    sum += it->second * x[it->first->getIndex()];
  }
  return sum;
}


Double Minotaur::InnerProduct(const Double *x, const Double *a, Int n) 
{
  Double sum = 0;
  for (Int i=0; i<n; ++i) {
    sum += (*x) * (*a);
    ++x;
    ++a;
  }
  return sum;
}


Bool Minotaur::IsInt(Double v, Double tol) 
{
  return (fabs(floor(v+0.5)-v)<tol);
}


void Minotaur::symMatDotV(UInt nz, const Double *mat, const UInt *irow,
                          const UInt *jcol, const Double *v, double *prod)
{
  for (UInt i=0; i<nz; ++i) {
    if (irow[i]==jcol[i]) {
      prod[irow[i]] += mat[i]*v[jcol[i]];
    } else {
      prod[irow[i]] += mat[i]*v[jcol[i]];
      prod[jcol[i]] += mat[i]*v[irow[i]];
    }
  }
}


void Minotaur::BoundsOnDiv(Double l0, Double u0, Double l1, Double u1, 
                           Double &lb, Double &ub)
{
  BoundsOnRecip(l1, u1, l1, u1);
  BoundsOnProduct(l0, u0, l1, u1, lb, ub);
}


void Minotaur::BoundsOnProduct(ConstVariablePtr x0, ConstVariablePtr x1,
                               Double &lb, Double &ub)
{
  BoundsOnProduct(x0->getLb(), x0->getUb(), x1->getLb(), x1->getUb(), lb, ub);
}


void Minotaur::BoundsOnProduct(Double l0, Double u0, Double l1, Double u1, 
                               Double &lb, Double &ub)
{

  double prod;

  if ((l1==-INFINITY && u1==INFINITY) || 
     (l0==-INFINITY && u0==INFINITY)) {
    lb = -INFINITY;
    ub =  INFINITY;
  } else if (fabs(l0)<=1e-10 && fabs(u0)<=1e-10) {
    lb = 0.0;
    ub = 0.0;
  } else if (fabs(l1)<=1e-10 && fabs(u1)<=1e-10) {
    lb = 0.0;
    ub = 0.0;
  } else {
    prod = l0*l1;
    if (std::isnan(prod)) {
      prod = -INFINITY;
    } 
    lb = prod;
    ub = prod;

    prod = u0*l1;
    if (std::isnan(prod)) {
      prod = INFINITY;
    } 
    lb = std::min(lb, prod);
    ub = std::max(ub, prod);

    prod = u0*u1;
    if (std::isnan(prod)) {
      prod = -INFINITY;
    }
    lb = std::min(lb, prod);
    ub = std::max(ub, prod);

    prod = l0*u1;
    if (std::isnan(prod)) {
      prod = INFINITY;
    } 
    lb = std::min(lb, prod);
    ub = std::max(ub, prod);
  }
}


void Minotaur::BoundsOnRecip(Double l0, Double u0, Double &lb, Double &ub)
{
  /*
   * This code _looks_ OK but returns bad values when (l0,u0) = (-inf,0)
   */
  /*
  if (l0<0 && u0>0) {
    lb = -INFINITY;
    ub = INFINITY;
  } else if {
    lb = 1.0/u0;
    ub = 1.0/l0;
  } 
  */
  if ((fabs(u0) < 1e-10) && (fabs(l0) < 1e-10)) {
    lb = -INFINITY;
    ub =  INFINITY;
  } else if (l0<-1e-10 && u0>1e-10) {
    lb = -INFINITY;
    ub = INFINITY;
  } else if ((fabs(u0)<1e-10) && l0<0) {
    lb = -INFINITY;
    ub = 1.0/l0;
  } else if ((fabs(l0)<1e-10) && u0<0) {
    lb = 1.0/u0;
    ub = INFINITY;
  } else {
    lb = 1.0/u0;
    ub = 1.0/l0;
  } 
}


void Minotaur::BoundsOnSquare(ConstVariablePtr x1, double &lb, double &ub)
{
  double l1 = x1->getLb();
  double u1 = x1->getUb();

  if (u1 < 0.) {         // both bounds are negative.
    lb = u1*u1;
    ub = l1*l1;
  } else if (l1 > 0.) {  // both bounds are positive.
    lb = l1*l1;
    ub = u1*u1;
  } else {               // lb is negative, ub is positive.
    lb = 0.;
    ub = std::max(l1*l1, u1*u1);
  }
}


void Minotaur::BoundsOnSquare(const double &l1, const double &u1, double &lb, 
                              double &ub)
{
  if (u1 < 0.) {         // both bounds are negative.
    lb = u1*u1;
    ub = l1*l1;
  } else if (l1 > 0.) {  // both bounds are positive.
    lb = l1*l1;
    ub = u1*u1;
  } else {               // lb is negative, ub is positive.
    lb = 0.;
    ub = std::max(l1*l1, u1*u1);
  }
}


void Minotaur::displayArray(const double* point, UInt n, std::ostream &out)
{
  for (UInt i=0; i<n; ++i, ++point) {
    out << *point << "\t";
  }
  out << std::endl;
}


Double Minotaur::Gcd(Double d1, Double d2, const Double &etol)
{
  Double rem;
  d1 = fabs(d1);
  d2 = fabs(d2);

  if (d2<d1) {
    rem = d1; // rem used as a temporary variable.
    d1  = d2;
    d2  = rem;
  }

  if (d1<etol) {
    return d2;
  }

  do {
    rem = fmod(d2, d1);
    d2 = d1;
    d1 = rem;
  } while (rem > etol);

  return d2;
}


Double Minotaur::getDistance(const Double* Pointa, const Double* Pointb, UInt n)
{
  Double dist = 0;

  for(UInt i=0; i<n; ++i, ++Pointa, ++Pointb){
    dist += pow(*Pointa - *Pointb, 2);
  }
  return sqrt(dist);
}


Double Minotaur::minArray(const Double* A, UInt n)
{
  Double min = A[0];

  for (UInt i=0; i<n; ++i, ++A){
    if (min > *A){
      min = *A;
    }
  }
  return min;
}


void Minotaur::sort(VarVector &vvec, Double *x, Bool ascend) 
{
  if (vvec.size()<=1) {
   return;
  }

  if (ascend == false) {
    for (Int i=vvec.size()-1; i>-1; --i) {
      x[i] *= -1;
    }
  }

  sortRec(vvec, x, 0, vvec.size()-1, vvec.size()/2);

  if (ascend == false) {
    for (Int i=vvec.size()-1; i>-1; --i) {
      x[i] *= -1;
    }
  }
}


void Minotaur::sortRec(VarVector &vvec, Double *x, int left, int right,
                       int pivotind) 
{
  Double pval;
  Double tmpval;
  VariablePtr tmpvar;
  int sind;

  // move pivot to the rightmost
  tmpvar = vvec[pivotind];
  vvec[pivotind] = vvec[right];
  vvec[right] = tmpvar;

  pval = x[pivotind];
  x[pivotind] = x[right];
  x[right] = pval;

  // move any value <= pval to the left side.
  sind = left;
  for (int i=left; i<right; ++i) {
    if (x[i] <= pval) {
      tmpvar = vvec[sind]; vvec[sind] = vvec[i]; vvec[i] = tmpvar;
      tmpval = x[sind]; x[sind] = x[i]; x[i] = tmpval;
      ++sind;
    }
  }

  // move pivot to sind
  tmpvar = vvec[sind]; vvec[sind] = vvec[right]; vvec[right] = tmpvar;
  tmpval = x[sind]; x[sind] = x[right]; x[right] = tmpval;

  if (sind-left>1) {
    sortRec(vvec, x, left, sind-1, (sind+left)/2);
  }
  if (right-sind>1) {
    sortRec(vvec, x, sind, right, (sind+right)/2);
  }
}


void Minotaur::toLowerCase(std::string &str)
{
  Int diff = ('z'-'Z');
  for (UInt i=0; i<str.size(); ++i) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      str[i] += diff;
    }
  }
}


std::string Minotaur::toClockTime(Double t)
{
  std::string s;
  std::stringstream ss;
  Int it = (int) (t*100);
  ss << std::setfill ('0') << std::setw(2) << (int)(it/360000);
  it = it%(360000);
  ss << ":" << std::setfill ('0') << std::setw(2) << (int)(it/6000);
  it = it%(6000);
  ss << ":" << std::setfill ('0') << std::setw(2) << (int)(it/100);
  it = it%(100);
  ss << ":" << std::setfill ('0') << std::setw(2) << it;
  s = ss.str();
  return s;
}

// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
