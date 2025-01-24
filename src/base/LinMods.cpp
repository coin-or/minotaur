//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file LinMods.cpp
 * \brief Implement the Modification class LinMods, that is used to store
 * modifications to the linear parts and rhs of a constraint.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include "LinMods.h"

#include <iostream>

#include "MinotaurConfig.h"
#include "Relaxation.h"

using namespace Minotaur;

LinMods::LinMods()
{
  bmods_.clear();
  bmods2_.clear();
  lmods_.clear();
}

LinMods::~LinMods()
{
  for(VarBoundModConstIter it = bmods_.begin(); it != bmods_.end(); ++it)
  {
    if(*it)
    {
      delete *it;
    }
  }
  for(VarBoundMod2ConstIter it = bmods2_.begin(); it != bmods2_.end(); ++it)
  {
    if(*it)
    {
      delete *it;
    }
  }
  for(LinConModConstIter it = lmods_.begin(); it != lmods_.end(); ++it)
  {
    if(*it)
    {
      delete *it;
    }
  }
  bmods_.clear();
  bmods2_.clear();
  lmods_.clear();
}

void LinMods::applyToProblem(ProblemPtr problem)
{
  for(VarBoundModConstIter it = bmods_.begin(); it != bmods_.end(); ++it)
  {
    (*it)->applyToProblem(problem);
  }
  for(VarBoundMod2ConstIter it = bmods2_.begin(); it != bmods2_.end(); ++it)
  {
    (*it)->applyToProblem(problem);
  }
  for(LinConModConstIter it = lmods_.begin(); it != lmods_.end(); ++it)
  {
    (*it)->applyToProblem(problem);
  }
}

void LinMods::undoToProblem(ProblemPtr problem)
{
  for(VarBoundModConstIter it = bmods_.begin(); it != bmods_.end(); ++it)
  {
    (*it)->undoToProblem(problem);
  }
  for(VarBoundMod2ConstIter it = bmods2_.begin(); it != bmods2_.end(); ++it)
  {
    (*it)->undoToProblem(problem);
  }
  for(LinConModConstIter it = lmods_.begin(); it != lmods_.end(); ++it)
  {
    (*it)->undoToProblem(problem);
  }
}

void LinMods::insert(VarBoundModPtr bmod)
{
  bmods_.push_back(bmod);
}

void LinMods::insert(VarBoundMod2Ptr bmod2)
{
  bmods2_.push_back(bmod2);
}

void LinMods::insert(LinConModPtr lmod)
{
  lmods_.push_back(lmod);
}

bool LinMods::isEmpty() const
{
  return (lmods_.empty() && bmods2_.empty() && bmods_.empty());
}

ModificationPtr LinMods::fromRel(RelaxationPtr rel, ProblemPtr) const
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  VarBoundModPtr bm;
  VarBoundMod2Ptr bm2;
  VariablePtr v;
  double newval, newlb, newub;
  BoundType lu;

  for(VarBoundModConstIter it = bmods_.begin(); it != bmods_.end(); ++it)
  {
    v = rel->getOriginalVar((*it)->getVar());
    newval = (*it)->getNewVal();
    lu = (*it)->getLU();
    bm = (VarBoundModPtr) new VarBoundMod(v, lu, newval);
    lmods->insert(bm);
  }
  for(VarBoundMod2ConstIter it = bmods2_.begin(); it != bmods2_.end(); ++it)
  {
    v = rel->getOriginalVar((*it)->getVar());
    newlb = (*it)->getNewLb();
    newub = (*it)->getNewUb();
    bm2 = (VarBoundMod2Ptr) new VarBoundMod2(v, newlb, newub);
    lmods->insert(bm2);
  }
  return lmods;
}

ModificationPtr LinMods::toRel(ProblemPtr, RelaxationPtr rel) const
{
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  VarBoundModPtr bm;
  VarBoundMod2Ptr bm2;
  VariablePtr v;
  double newval, newlb, newub;
  BoundType lu;

  for(VarBoundModConstIter it = bmods_.begin(); it != bmods_.end(); ++it)
  {
    v = rel->getRelaxationVar((*it)->getVar());
    newval = (*it)->getNewVal();
    lu = (*it)->getLU();
    bm = (VarBoundModPtr) new VarBoundMod(v, lu, newval);
    lmods->insert(bm);
  }
  for(VarBoundMod2ConstIter it = bmods2_.begin(); it != bmods2_.end(); ++it)
  {
    v = rel->getRelaxationVar((*it)->getVar());
    newlb = (*it)->getNewLb();
    newub = (*it)->getNewUb();
    bm2 = (VarBoundMod2Ptr) new VarBoundMod2(v, newlb, newub);
    lmods->insert(bm2);
  }
  return lmods;
}

void LinMods::write(std::ostream& out) const
{
  out << "LinMods: " << std::endl;
  for(VarBoundModConstIter it = bmods_.begin(); it != bmods_.end(); ++it)
  {
    (*it)->write(out);
  }
  for(VarBoundMod2ConstIter it = bmods2_.begin(); it != bmods2_.end(); ++it)
  {
    (*it)->write(out);
  }
  for(LinConModConstIter it = lmods_.begin(); it != lmods_.end(); ++it)
  {
    (*it)->write(out);
  }
}
