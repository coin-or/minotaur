//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file FixVarsHeur.cpp
 * \brief Define the FixVarsHeur class for diving heuristics for MINLPs.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 * Implements the class FixVarsHeur.
 */

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "Environment.h"
#include "FixVarsHeur.h"
#include "Handler.h"
#include "MinotaurConfig.h"
#include "Problem.h"
#include "SolutionPool.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

//#define SPEW 0

const std::string FixVarsHeur::me_ = "FixVars Heuristic: ";

FixVarsHeur::FixVarsHeur(EnvPtr env, ProblemPtr p)
  : env_(env),
    p_(p)
{
  consNumVar_.clear();
  mbin_ = 0;
  mnl_ = 0;
  stats_ = (FixVarsHeurStats*)new FixVarsHeurStats();
  stats_->numSol = 0;
  stats_->time = 0;
  handlers_.clear();
}

FixVarsHeur::~FixVarsHeur()
{
  consNumVar_.clear();
  delete stats_;
}

void FixVarsHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  bool restart = true;
  UInt min_iter = 3, max_iter = 10, iter = 0;
  std::map<VariablePtr, UInt> unfixedVars;
  double stime = env_->getTimer()->query();
  UInt numvars;

  initialize_();
  while(iter < min_iter || (restart && iter < max_iter)) {
    ++iter;
#if SPEW
    env_->getLogger()->msgStream(LogDebug)
        << me_ << "Iteration Number : " << iter << std::endl;
#endif
    restart = false;
    consNumVar_.clear();
    for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
        ++cit) {
      if((*cit)->getSrcType() == ConsOrig ||
         (*cit)->getSrcType() == ConsTranOrig) {
        numvars = (*cit)->getFunction()->getNumVars();
        if(numvars == 1) {
          // Just a bound constraint, should not be considered for convering.
          consNumVar_.insert({*cit, 0});
          if(iter == 1) {
            updateItmp_(*cit);
          }
        } else {
          consNumVar_.insert({*cit, numvars});
        }
      } else {
        consNumVar_.insert({*cit, 0});
        if(iter == 1) {
          updateItmp_(*cit);
        }
      }
    }
    unfixedVars.clear();
    for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
        ++vit) {
      unfixedVars.insert({*vit, (*vit)->getItmp()});
    }

    while(unfixedVars.size() > 0) {
      FixVars_(unfixedVars);
      if(presolve_(s_pool, unfixedVars)) {
        unfixVars_();
        restart = true;
        break;
      }
      for(std::map<VariablePtr, UInt>::iterator uit = unfixedVars.begin();
          uit != unfixedVars.end(); ++uit) {
        uit->second = uit->first->getItmp();
      }
    }
    if(!restart) {
      foundNewSol_(s_pool, restart);
      unfixVars_();
    }
  }
  stats_->time = env_->getTimer()->query() - stime;
}

void FixVarsHeur::foundNewSol_(SolutionPoolPtr s_pool, bool& restart)
{
  UInt n = p_->getNumVars();
  double* xl = new double[n];
  double best_obj = s_pool->getBestSolutionValue();
  double curr_obj;
  int error = 0;

  std::memset(xl, 0, n * sizeof(double));
  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    // We assume all variables are fixed here
    xl[(*vit)->getIndex()] = (*vit)->getLb();
  }
  if(isFeasible_(xl)) {
    ++stats_->numSol;
    error = 0;
    curr_obj = p_->getObjective()->eval(xl, &error);
#if SPEW
    env_->getLogger()->msgStream(LogDebug2)
        << me_ << "Found feasible solution. Objective value: " << curr_obj
        << std::endl;
#endif
    if(curr_obj < best_obj - 1e-6) {
      s_pool->addSolution(xl, curr_obj);
    }
    ++stats_->numSol;
  } else {
    restart = true;
  }

  delete[] xl;
}

void FixVarsHeur::initialize_()
{
  VariablePtr v;

  mbin_ = p_->getNumCons() + 1;
  mnl_ = 2;

  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    v = *vit;
    if(v->getType() == Binary || v->getType() == ImplBin) {
      v->setItmp(mbin_ * v->getNumCons());
    } else if(v->getFunType() != Linear && v->getFunType() != Constant) {
      v->setItmp(mnl_ * v->getNumCons());
    } else {
      v->setItmp(v->getNumCons());
    }
  }
}

void FixVarsHeur::fix_(VariablePtr v)
{
  double fix_at;
  bool lbinf = v->getLb() <= -INFINITY;
  bool ubinf = v->getUb() >= INFINITY;

  if(lbinf && ubinf) {
    fix_at = rand() % 10 < 8 ? -1000 : 1000;
  } else if(lbinf) {
    fix_at = v->getUb();
  } else if(ubinf) {
    fix_at = v->getLb();
  } else {
    fix_at = rand() % 10 < 8 ? v->getLb() : v->getUb();
  }
  VarBoundMod2* m = 0;

  m = new VarBoundMod2(v, fix_at, fix_at);
  m->applyToProblem(p_);
#if SPEW
  env_->getLogger()->msgStream(LogDebug)
      << me_ << v->getName() << " is fixed at " << fix_at << std::endl;
#endif
  mods_.push_back(m);
}

void FixVarsHeur::FixVars_(std::map<VariablePtr, UInt>& unfixedVars)
{
  ConstrSet covered;
  ConstraintPtr c;
  FunctionPtr f;
  VariablePtr v;
  std::pair<ConstrSet::iterator, bool> ret;

  covered.clear();
  for(std::map<ConstraintPtr, UInt>::iterator it = consNumVar_.begin();
      it != consNumVar_.end(); ++it) {
    if(it->second == 0) {
      covered.insert(it->first);
    } else if(it->second == 1) {
      f = it->first->getFunction();
      for(VarSetConstIterator vit = f->varsBegin(); vit != f->varsEnd();
          ++vit) {
        v = *vit;
        if(unfixedVars.find(v) != unfixedVars.end()) {
          fix_(v);
          for(ConstrSet::iterator cit = v->consBegin(); cit != v->consEnd();
              ++cit) {
            c = *cit;
            ret = covered.insert(c);
            if(ret.second == true) {
              updateMap_(c, unfixedVars);
            }
            if(consNumVar_[c] > 0) {
              --(consNumVar_[c]);
            }
          }
          unfixedVars.erase(v);
          break;
        }
      }
    }
  }

  while((covered.size() < p_->getNumCons()) && (unfixedVars.size() > 0)) {
    v = selectVarToFix_(unfixedVars);
    fix_(v);
    for(ConstrSet::iterator cit = v->consBegin(); cit != v->consEnd(); ++cit) {
      c = *cit;
      ret = covered.insert(c);
      if(ret.second == true) {
        updateMap_(c, unfixedVars);
      }
      if(consNumVar_[c] > 0) {
        --(consNumVar_[c]);
      }
    }
    unfixedVars.erase(v);
  }
}

bool FixVarsHeur::mapCompare_(const std::pair<VariablePtr, UInt>& p1,
                              const std::pair<VariablePtr, UInt>& p2)
{
  return p1.second < p2.second;
}

VariablePtr
FixVarsHeur::selectVarToFix_(std::map<VariablePtr, UInt>& unfixedVars)
{
  std::map<VariablePtr, UInt>::iterator it =
      std::max_element(unfixedVars.begin(), unfixedVars.end(), mapCompare_);
  return it->first;
}

void FixVarsHeur::updateMap_(ConstraintPtr c,
                             std::map<VariablePtr, UInt>& unfixedVars)
{
  FunctionPtr f = c->getFunction();
  VariablePtr v;

  for(VarSetConstIterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    v = *vit;
    if(unfixedVars.find(v) != unfixedVars.end()) {
      if(v->getType() == Binary || v->getType() == ImplBin) {
        unfixedVars[v] -= mbin_;
      } else if(v->getFunType() != Linear && v->getFunType() != Constant) {
        unfixedVars[v] -= mnl_;
      } else {
        --(unfixedVars[v]);
      }
    }
  }
}

void FixVarsHeur::updateItmp_(ConstraintPtr c)
{
  FunctionPtr f = c->getFunction();
  VariablePtr v;

  for(VarSetConstIterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    v = *vit;
    if(v->getType() == Binary || v->getType() == ImplBin) {
      v->setItmp(v->getItmp() - mbin_);
    } else if(v->getFunType() != Linear && v->getFunType() != Constant) {
      v->setItmp(v->getItmp() - mnl_);
    } else {
      v->setItmp(v->getItmp() - 1);
    }
  }
}

bool FixVarsHeur::isFeasible_(const double* x)
{
  ConstraintPtr c;
  double act, clb, cub;
  int error = 0;
  double aTol = 1e-6, rTol = 1e-7;

  for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    act = c->getActivity(x, &error);
    if(error == 0) {
      cub = c->getUb();
      clb = c->getLb();
      if((act > cub + aTol) && (cub == 0 || act > cub + fabs(cub) * rTol)) {
        return false;
      }
      if((act < clb - aTol) && (clb == 0 || act < clb - fabs(clb) * rTol)) {
        return false;
      }
    } else {
      env_->getLogger()->msgStream(LogError)
          << me_ << c->getName() << " Constraint not defined at this point."
          << std::endl;
      return false;
    }
  }
  return true;
}

bool FixVarsHeur::presolve_(SolutionPoolPtr s_pool,
                            std::map<VariablePtr, UInt>& unfixedVars)
{
  SolveStatus status = Started;
  ModVector pres_mod;
  VarVector gotFixed;
  double tol = 1e-3;
  VariablePtr v;
  ConstraintPtr c;

  for(HandlerIterator it = handlers_.begin(); it != handlers_.end(); ++it) {
    pres_mod.clear();
    (*it)->simplePresolve(p_, s_pool, pres_mod, status);
    mods_.insert(mods_.end(), pres_mod.begin(), pres_mod.end());
    if(status == SolvedInfeasible) {
      return true;
    }
  }

  gotFixed.clear();
  for(std::map<VariablePtr, UInt>::iterator it = unfixedVars.begin();
      it != unfixedVars.end(); ++it) {
    v = it->first;
    if(v->getUb() - v->getLb() < tol) {
      gotFixed.push_back(v);
#if SPEW
      env_->getLogger()->msgStream(LogDebug)
          << me_ << "Presolve fixed " << v->getName() << " at " << v->getLb()
          << std::endl;
#endif
    }
  }

  for(VariableIterator it = gotFixed.begin(); it != gotFixed.end(); ++it) {
    v = *it;
    for(ConstrSet::iterator cit = v->consBegin(); cit != v->consEnd(); ++cit) {
      c = *cit;
      if(consNumVar_[c] > 0) {
        --(consNumVar_[c]);
      }
    }
    unfixedVars.erase(v);
  }
  gotFixed.clear();
  return false;
}

void FixVarsHeur::setHandlers(HandlerVector& handlers)
{
  handlers_ = handlers;
}

void FixVarsHeur::unfixVars_()
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;

  for(mod_iter = mods_.rbegin(); mod_iter != mods_.rend(); ++mod_iter) {
    mod = *mod_iter;
    mod->undoToProblem(p_);
    delete mod;
  }
  mods_.clear();
}

void FixVarsHeur::writeStats(std::ostream& out) const
{
  out << me_ << "number of feasible solutions found : " << stats_->numSol
      << std::endl;
  out << me_ << "Time taken : " << stats_->time << std::endl;
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
