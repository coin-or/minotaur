//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
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
}

FixVarsHeur::~FixVarsHeur()
{
  consNumVar_.clear();
  delete stats_;
}

void FixVarsHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  bool restart = true;
  UInt min_iter = 5, max_iter = 20, iter = 0;
  std::map<UInt, UInt> unfixedVars;

  while(iter < min_iter || (restart && iter < max_iter)) {
    ++iter;
    restart = false;
    initialize_();
    for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
        ++vit) {
      unfixedVars.insert({(*vit)->getIndex(), (*vit)->getItmp()});
    }
    while(unfixedVars.size() > 0) {
      FixVars_(unfixedVars);
      if(presolve_()) {
        unfixVars_();
        restart = true;
        break;
      }
    }
    if(!restart) {
      foundNewSol_(s_pool);
      ++stats_->numSol;
    }
  }
}

void FixVarsHeur::initialize_()
{
  VariablePtr v;
  ConstraintPtr c;

  if(p_->getNumCons() <= 2) {
    mbin_ = 10;
    mnl_ = 1;
  } else {
    mbin_ = p_->getNumCons() + 1;
    mnl_ = ceil(p_->getNumCons() / 2.0);
  }

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

  consNumVar_.clear();
  for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    consNumVar_.insert({c, c->getFunction()->getNumVars()});
  }
}

void FixVarsHeur::fix_(VariablePtr v)
{
  double fix_at = rand() % 10 < 8 ? v->getLb() : v->getUb();
  VarBoundMod2* m = 0;

  m = new VarBoundMod2(v, fix_at, fix_at);
  m->applyToProblem(p_);
  mods_.push(m);
}

void FixVarsHeur::FixVars_(std::map<UInt, UInt>& unfixedVars)
{
  ConstrSet covered;
  ConstraintPtr c;
  FunctionPtr f;
  VariablePtr v;

  covered.clear();
  for(std::map<ConstraintPtr, UInt>::iterator it = consNumVar_.begin();
      it != consNumVar_.end(); ++it) {
    if(it->second == 0) {
      covered.insert(it->first);
    } else if(it->second == 1) {
      f = it->first->getFunction();
      for(VariableConstIterator vit = f->varsBegin(); vit != f->varsEnd();
          ++vit) {
        v = *vit;
        if(unfixedVars.find(v->getIndex()) != unfixedVars.end()) {
          fix_(v);
          for(ConstrSet::iterator cit = v->consBegin(); cit != v->consEnd();
              ++cit) {
            c = *cit;
            updateMap_(c, unfixedVars);
            covered.insert(c);
            --(consNumVar_[c]);
          }
          unfixedVars.erase(v->getIndex());
          break;
        }
      }
    }
  }

  while(covered.size() < p_->getNumCons()) {
    v = selectVarToFix_(unfixedVars);
    fix_(v);
    for(ConstrSet::iterator cit = v->consBegin(); cit != v->consEnd(); ++cit) {
      c = *cit;
      updateMap_(c, unfixedVars);
      covered.insert(c);
      --(consNumVar_[c]);
    }
    unfixedVars.erase(v->getIndex());
  }
}

bool mapCompare_(std::pair<UInt, UInt>& p1, std::pair<UInt, UInt>& p2)
{
  return p1.second < p2.second;
}

VariablePtr FixVarsHeur::selectVarToFix_(std::map<UInt, UInt>& unfixedVars)
{
  std::map<UInt, UInt>::iterator it =
      std::max_element(unfixedVars.begin(), unfixedVars.end(), mapCompare_);
  return p_->getVariable(it->first);
}

void FixVarsHeur::updateMap_(ConstraintPtr c, std::map<UInt, UInt>& unfixedVars)
{
  FunctionPtr f = c->getFunction();
  VariablePtr v;

  for(VariableConstIterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    v = *vit;
    if(v->getType() == Binary || v->getType() == ImplBinary) {
      unfixedVars[v->getIndex()] -= mbin_;
    } else if(v->getFunType != Linear && v->getFunType() != Constant) {
      unfixedVars[v->getIndex()] -= mnl_;
    } else {
      --(unfixedVars[v->getIndex()]);
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
