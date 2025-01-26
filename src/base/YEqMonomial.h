//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqMonomial.h
 * \brief Declare class for storing auxiliary variables for monomial
 * expressions.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURYEQMONOMIAL_H
#define MINOTAURYEQMONOMIAL_H

#include "Types.h"

namespace Minotaur {

class MultilinearTermsHandler;
class MonomialFunction;
typedef MonomialFunction* MonomialFunPtr;
typedef MultilinearTermsHandler* MultilinearTermsHandlerPtr;


class YEqMonomial
{
public:
  YEqMonomial(UInt n);
  VariablePtr findY(MonomialFunPtr mf);
  void insert(VariablePtr auxvar, MonomialFunPtr mf);

private:
  std::vector<MonomialFunPtr> mf_;
  DoubleVector hash_;
  UInt n_;
  DoubleVector rand_;
  VarVector y_;
  double evalHash_(MonomialFunPtr mf_);
};
}

#endif

