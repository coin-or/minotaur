// ===========================================================================
// $Date: 2010-03-07 22:35:34 -0600 (Sun, 07 Mar 2010) $
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: type.hpp 679 2010-04-06 23:13:06Z xzhen $
// ===========================================================================
#ifndef TYPE_HH_
#define TYPE_HH_

#include <limits>

namespace Type{
  typedef double double;
  // static const int NMAX = std::numeric_limits<int>::max();
  // static const int NMIN = std::numeric_limits<int>::min();
  static const double INF = std::numeric_limits<double>::infinity();
  static const double EPSILON = 1.0E-8;
}
#endif
