// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2012 The MINOTAUR Team.
// 

/**
 * \file Branch.cpp
 * \author Ashutosh Mahajan, Argonne National Laboratory.
 * \brief Define the class Branch for describing branches in branch-and-bound.
 */

#include <cmath>

#include "MinotaurConfig.h"
#include "BrCand.h"
#include "Branch.h"
#include "Modification.h"
#include "Operations.h"
#include "Variable.h"

using namespace Minotaur;


Branch::Branch()
: mods_(0),
  activity_(INFINITY),
  brCand_(BrCandPtr()) // NULL
{

}


void Branch::addMod(ModificationPtr mod) 
{
  mods_.push_back(mod);
}


Double Branch::getActivity() const
{
  return activity_;
}


void Branch::setActivity(Double value) 
{
  activity_ = value;
}

void Branch::write(std::ostream &out) const
{
  for (ModificationConstIterator it = mods_.begin();
       it != mods_.end(); ++it) {
    (*it)->write(out);
  }
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
