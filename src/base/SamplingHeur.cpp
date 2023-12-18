//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file SamplingHeur.cpp
 * \brief Define the SamplingHeur class for diving heuristics for MINLPs.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 * Implements the class SamplingHeur.
 */

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "Environment.h"
#include "MinotaurConfig.h"
#include "Problem.h"
#include "SamplingHeur.h"
#include "SolutionPool.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

//#define SPEW 0

const std::string SamplingHeur::me_ = "Sampling Heuristic: ";

SamplingHeur::SamplingHeur(EnvPtr env, ProblemPtr p)
  : env_(env),
    p_(p)
{
  maxRand_ = 100;
  stats_ = (SamplingHeurStats*)new SamplingHeurStats();
  stats_->isZero = false;
  stats_->isLB = false;
  stats_->isUB = false;
  stats_->numRand = 0;
}

SamplingHeur::~SamplingHeur()
{
  delete stats_;
}

void SamplingHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  UInt n = p_->getNumVars();
  double* x = new double[n];
  double* xl = new double[n];
  double* xu = new double[n];
  SolutionPtr sol;
  VariablePtr v;
  ObjectivePtr obj = p_->getObjective();
  bool checkzero = true;
  bool lbinf, ubinf;
  int error = 0;

  std::memset(x, 0, n * sizeof(double));
  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    v = *vit;
    if(v->getLb() > 0 || v->getUb() < 0) {
      checkzero = false;
      break;
    }
  }
  if(checkzero) {
    if(isFeasible_(x)) {
      sol = (SolutionPtr) new Solution(obj->eval(x, &error), x, p_);
      s_pool->addSolution(sol);
      stats_->isZero = true;
    }
  }

  std::memset(xl, 0, n * sizeof(double));
  std::memset(xu, 0, n * sizeof(double));
  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    v = *vit;
    lbinf = v->getLb() <= -INFINITY;
    ubinf = v->getUb() >= INFINITY;
    if(lbinf && ubinf) {
      xl[v->getIndex()] = -1000;
      xu[v->getIndex()] = 1000;
    } else if(lbinf) {
      xl[v->getIndex()] = -(fabs(v->getUb()) * 10);
      xu[v->getIndex()] = v->getUb();
    } else if(ubinf) {
      xl[v->getIndex()] = v->getLb();
      xu[v->getIndex()] = fabs(v->getLb()) * 10;
    } else {
      xl[v->getIndex()] = v->getLb();
      xu[v->getIndex()] = v->getUb();
    }
  }

  if(isFeasible_(xl)) {
    error = 0;
    sol = (SolutionPtr) new Solution(obj->eval(xl, &error), xl, p_);
    s_pool->addSolution(sol);
    stats_->isLB = true;
  }
  if(isFeasible_(xu)) {
    error = 0;
    sol = (SolutionPtr) new Solution(obj->eval(xu, &error), xu, p_);
    s_pool->addSolution(sol);
    stats_->isUB = true;
  }

  for(UInt i = 0; i < maxRand_; ++i) {
    for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
        ++vit) {
      v = *vit;
      if(rand() % 2 == 0) {
        x[v->getIndex()] = xl[v->getIndex()];
      } else {
        x[v->getIndex()] = xu[v->getIndex()];
      }
    }
    if(isFeasible_(x)) {
      error = 0;
      sol = (SolutionPtr) new Solution(obj->eval(x, &error), x, p_);
      s_pool->addSolution(sol);
      ++stats_->numRand;
    }
  }

  if(stats_->isZero || stats_->isLB || stats_->isUB || stats_->numRand > 0) {
    for(UInt i = 0; i < maxRand_; ++i) {
      getNewPoint_(x, s_pool);
      if(isFeasible_(x)) {
        error = 0;
        sol = (SolutionPtr) new Solution(obj->eval(x, &error), x, p_);
        s_pool->addSolution(sol);
        ++stats_->numRand;
      }
    }
  }

  delete[] xl;
  delete[] xu;
  delete[] x;
}

void SamplingHeur::getNewPoint_(double* x, SolutionPoolPtr s_pool)
{
  const double* best = s_pool->getBestSolution()->getPrimal();
  UInt r, ind;
  double ldist, udist;
  VariablePtr v;

  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    v = *vit;
    ind = v->getIndex();
    r = rand() % 10;
    if(r < 8) {
      x[ind] = best[ind];
    } else if(r == 8) {
      ldist = best[ind] - v->getLb();
      udist = v->getUb() - best[ind];
      if(ldist < udist) {
        x[ind] = v->getUb();
      } else if(udist < ldist) {
        x[ind] = v->getLb();
      } else {
        x[ind] = rand() % 2 == 0 ? v->getLb() : v->getUb();
      }
    } else {
      if(v->getType() == Binary || v->getType() == ImplBin) {
        x[ind] = best[ind] > 0.5 ? 0 : 1;
      } else if(v->getType() == Integer || v->getType() == ImplInt) {
        ldist = best[ind] - v->getLb();
        udist = v->getUb() - best[ind];
        if(ldist < udist) {
          x[ind] = ceil(v->getUb() - udist / 2.0);
        } else if(udist < ldist) {
          x[ind] = floor(v->getLb() + ldist / 2.0);
        } else {
          if(rand() % 2 == 0) {
            x[ind] = floor(v->getLb() + ldist / 2.0);
          } else {
            x[ind] = ceil(v->getUb() - udist / 2.0);
          }
        }
      } else {
        ldist = best[ind] - v->getLb();
        udist = v->getUb() - best[ind];
        if(ldist < udist) {
          x[ind] = v->getUb() - udist / 2.0;
        } else if(udist < ldist) {
          x[ind] = v->getLb() + ldist / 2.0;
        } else {
          if(rand() % 2 == 0) {
            x[ind] = v->getLb() + ldist / 2.0;
          } else {
            x[ind] = v->getUb() - udist / 2.0;
          }
        }
      }
    }
  }
}

bool SamplingHeur::isFeasible_(const double* x)
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

void SamplingHeur::writeStats(std::ostream& out) const
{
  out << me_ << " 0 was feasible? : " << stats_->isZero << std::endl;
  out << me_ << " lb was feasible? : " << stats_->isLB << std::endl;
  out << me_ << " ub was feasible? : " << stats_->isUB << std::endl;
  out << me_ << " number of random solution feasible : " << stats_->numRand
      << std::endl;
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
