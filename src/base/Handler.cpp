//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

//
// Handler class handles different kinds of constraints.
//

#include "Handler.h"

#include "MinotaurConfig.h"
#include "Modification.h"
#include "Problem.h"
#include "Relaxation.h"
#include "Types.h"

using namespace Minotaur;

int Handler::fixNodeErr(RelaxationPtr, ConstSolutionPtr, SolutionPoolPtr, bool&)
{
  assert(!"FixNodeErr not implemented for the Handler");
  return 0;
}

bool Handler::getStrongerMods(RelaxationPtr rel, NodePtr node,
                              SolutionPoolPtr s_pool, ModVector& p_mods,
                              ModVector& r_mods)
{
  bool oldModProb = modProb_;
  bool oldModRel = modRel_;
  bool is_inf;

  modProb_ = true;
  modRel_ = true;
  is_inf = presolveNode(rel, node, s_pool, p_mods, r_mods);
  modProb_ = oldModProb;
  modRel_ = oldModRel;

  return is_inf;
}

void Handler::undoStrongerMods(ProblemPtr p, RelaxationPtr rel,
                               ModVector& p_mods, ModVector& r_mods)
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;

  ModificationRConstIterator rend = p_mods.rend();

  for(mod_iter = p_mods.rbegin(); mod_iter != rend; ++mod_iter)
  {
    mod = *mod_iter;
    mod->undoToProblem(p);
    delete mod;
  }
  p_mods.clear();

  rend = r_mods.rend();
  for(mod_iter = r_mods.rbegin(); mod_iter != rend; ++mod_iter)
  {
    mod = *mod_iter;
    mod->undoToProblem(rel);
    delete mod;
  }
  r_mods.clear();
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
