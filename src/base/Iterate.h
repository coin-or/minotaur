//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

#ifndef MINOTAURITERATE_H
#define MINOTAURITERATE_H

// This is an iterate the countains the status, activities, and 
// variable values for the primal variables and multipliers.  What 
// about solvers or
// engines that do not compute multipliers, such as derivative-free
// optimization methods?

#include <vector>

#include "Types.h"

namespace Minotaur {
  class Iterate {
  public:
    Iterate(std::vector<double> &x, std::vector<double> &l);
    ~Iterate() { };

  private:
    std::vector<double> x_;
    std::vector<double> l_;
  };
}

#endif

