//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file PreDelVars.cpp
 * \brief Postsolver for substituted variables.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include "MinotaurConfig.h"
#include "PreDelVars.h"

using namespace Minotaur;

PreDelVars::PreDelVars()
{
  vars_.clear();
}


PreDelVars::~PreDelVars()
{
  vars_.clear();
}


void PreDelVars::insert(VariablePtr v)
{
  vars_.push_front(v);
}


void PreDelVars::postsolveGetX(const DoubleVector &x, DoubleVector *newx)
{
  UInt n = x.size()+vars_.size();
  bool *filled = new bool[n];
  VariablePtr v;
  DoubleVector::const_iterator dit;
  DoubleVector::iterator dit2;

  std::fill(filled, filled+n, false);
  newx->resize(n);
  for (VarQueueConstIter it=vars_.begin(); it!=vars_.end(); ++it) {
    v = (*it);
    (*newx)[v->getIndex()] = v->getLb();
    filled[v->getIndex()] = true;
  }


  dit=x.begin();
  dit2=newx->begin();
  for (bool *it=filled; it!=filled+n; ++it, ++dit2) {
    if ((*it) == false) {
      *dit2 = *dit;
      ++dit;
    }
  }
  delete [] filled;
}


