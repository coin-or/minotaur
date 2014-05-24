// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
// 

/**
 * \file Operations.h
 * \brief Declare some commonly used functions.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "LinearFunction.h"

namespace Minotaur {
  Bool IsInt(Double v, Double tol=1e-12);

  Double InnerProduct(const VariableGroup &g1, const VariableGroup &g2);

  Double InnerProduct(const std::vector<Double> &x, const VariableGroup &g2);

  /// Inner product of two arrays, x and a, each of size n.
  Double InnerProduct(const Double *x, const Double *a, Int n);

  void symMatDotV(UInt nz, const Double *mat, const UInt *irow, const UInt *jcol,
                  const Double *v, double *prod);

  void BoundsOnDiv(Double l0, Double u0, Double l1, Double u1, 
                   Double &lb, Double &ub);

  /// Get bounds on y, where \f$ y = x_0x_1 \f$.
  void BoundsOnProduct(ConstVariablePtr x0, ConstVariablePtr x1,
                       Double &lb, Double &ub);

  /// Get bounds on y, where \f$ y = x_0x_1 \f$.
  void BoundsOnProduct(Double l0, Double u0, Double l1, Double u1, Double &lb,
                       Double &ub);

  void BoundsOnRecip(Double l0, Double u0, Double &lb, Double &ub);

  /// Get lower and upper bounds on a square of variable.
  void BoundsOnSquare(ConstVariablePtr x1, Double &lb, Double &ub);

  /**
   * Get lower and upper bounds on a square of variable whose bounds are
   * provided.
   */
  void BoundsOnSquare(const Double &x_lb, const Double &y_lb, Double &lb, 
                      Double &ub);

  /**
   * Get gcd(d1, d2). Returns d2 if d1 is zero. Returns d1 if d2 is zero.
   * Otherwise, returns largest x > 0 s.t. d1/x and d2/x are integers.
   * e.g. Gcd(10.5, 7.0) = 3.5. If d1 is zero, returns abs(d2), if d2 is zero,
   * returns abs(d1).
   */
  Double Gcd(Double d1, Double d2, const Double &etol = 1e-9);

  /**
   * Get distance(a,b,n). Returns the Euclidian distance between point a and point b which 
   * in n-dimensional space.
   */
  Double getDistance(const Double* Pointa, const Double* Pointb, UInt n);

  /**
   * Get minArray. Returns the minimum element of a dynamic array.
   */
  Double minArray(const Double* A, UInt n);

  /// To display the initial point.
  void displayArray(const Double* point, UInt n, std::ostream &out);

  /// Sort a vector of variables according to values of x.
  void sort(VarVector &vvec, Double *x, Bool ascend=true);
  void sortRec(VarVector &vvec, Double *x, int left, int right, int pivotind);

  /// Convert a string to lower case.
  void toLowerCase(std::string &str);

  /// Convert a double representing seconds into wall clock time HH:MM:SS:mm
  std::string toClockTime(Double t);
}

#endif

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
