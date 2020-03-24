//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file SimpleTransformer.cpp
 * \brief Define class for storing auxiliary variables equivalebt to a
 * product of two variables.
 * \author Mustafa Vora, IEOR, IIT Bombay
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "YEqQf.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

YEqQf::YEqQf()
{
  randVar1_ = (double) rand()/(RAND_MAX);
  randVar2_ = (double) rand()/(RAND_MAX);
  randCoeff_ = (double) rand()/(RAND_MAX);
}

VariablePtr YEqQf::findY(QuadraticFunctionPtr qf)
{
  double hash = evalHash_(qf);
  for (UInt i=0; i<y_.size(); ++i) {
    if (fabs(hash-hash_[i])<1e-10) {
      if (qf->isIdenticalTo(qf_[i])) {
        return y_[i];
      }
    }
  }
  return VariablePtr();
}

void YEqQf::insert(VariablePtr auxvar, QuadraticFunctionPtr qf)
{
  qf_.push_back(qf);
  hash_.push_back(evalHash_(qf));
  y_.push_back(auxvar);
}

double YEqQf::evalHash_(QuadraticFunctionPtr qf) {
  UInt i = 1;
  double hash = 0.0;
  for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
      ++it) {
    hash += i*(randVar1_*(it->first.first->getId()));
    hash += i*(randVar2_*(it->first.second->getId()));
    hash += i*(randCoeff_*(it->second));
    ++i;
  }
  return hash;
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
//
