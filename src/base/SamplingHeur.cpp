//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
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
  stats_->numSol = 0;
  stats_->time = 0;
  stats_->checked = 0;
}

SamplingHeur::~SamplingHeur()
{
  delete stats_;
}

void SamplingHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  const Timer* timer = env_->getTimer();
  double stime = timer->query();
  UInt n = p_->getNumVars();
  double* x = new double[n];
  double* xl = new double[n];
  double* xu = new double[n];
  double best_obj = s_pool->getBestSolutionValue();
  double curr_obj;
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
    ++stats_->checked;
    if(isFeasible_(x)) {
      curr_obj = obj->eval(x, &error);
#if SPEW
      env_->getLogger()->msgStream(LogDebug2)
          << me_ << " zero is feasible. Objective value: " << curr_obj
          << std::endl;
#endif
      if(curr_obj < best_obj - 1e-6) {
        s_pool->addSolution(x, curr_obj);
        best_obj = curr_obj;
      }
      ++stats_->numSol;
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

  ++stats_->checked;
  if(isFeasible_(xl)) {
    error = 0;
    curr_obj = obj->eval(xl, &error);
#if SPEW
    env_->getLogger()->msgStream(LogDebug2)
        << me_ << " LB is feasible. Objective value: " << curr_obj << std::endl;
#endif
    if(curr_obj < best_obj - 1e-6) {
      s_pool->addSolution(xl, curr_obj);
      best_obj = curr_obj;
    }
    ++stats_->numSol;
  }
  ++stats_->checked;
  if(isFeasible_(xu)) {
    error = 0;
    curr_obj = obj->eval(xu, &error);
#if SPEW
    env_->getLogger()->msgStream(LogDebug2)
        << me_ << " UB is feasible. Objective value: " << curr_obj << std::endl;
#endif
    if(curr_obj < best_obj - 1e-6) {
      s_pool->addSolution(xu, curr_obj);
      best_obj = curr_obj;
    }
    ++stats_->numSol;
  }

  stats_->checked += maxRand_;
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
      curr_obj = obj->eval(x, &error);
#if SPEW
      env_->getLogger()->msgStream(LogDebug2)
          << me_
          << " Found a feasible solution with objective value: " << curr_obj
          << std::endl;
#endif
      if(curr_obj < best_obj - 1e-6) {
        s_pool->addSolution(x, curr_obj);
        best_obj = curr_obj;
      }
      ++stats_->numSol;
    }
  }

  if(stats_->numSol > 0) {
    stats_->checked += maxRand_;
    for(UInt i = 0; i < maxRand_; ++i) {
      getNewPoint_(x, xl, xu, s_pool);
      if(isFeasible_(x)) {
        error = 0;
        curr_obj = obj->eval(x, &error);
#if SPEW
        env_->getLogger()->msgStream(LogDebug2)
            << me_
            << " Found a feasible solution with objective value: " << curr_obj
            << std::endl;
#endif
        if(curr_obj < best_obj - 1e-6) {
          s_pool->addSolution(x, curr_obj);
          best_obj = curr_obj;
        }
        ++stats_->numSol;
      }
    }
  }

  stats_->time += timer->query() - stime;
  delete[] xl;
  delete[] xu;
  delete[] x;
}

void SamplingHeur::getNewPoint_(double* x, double* xl, double* xu,
                                SolutionPoolPtr s_pool)
{
  const double* best = s_pool->getBestSolution()->getPrimal();
  UInt r, ind;
  double ldist, udist;
  VariablePtr v;

  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    v = *vit;
    ind = v->getIndex();
    r = rand() % 100;
    if(r < 95) {
      x[ind] = best[ind];
    } else if(r < 98) {
      ldist = best[ind] - xl[ind];
      udist = xu[ind] - best[ind];
      if(ldist < udist) {
        x[ind] = xu[ind];
      } else if(udist < ldist) {
        x[ind] = xl[ind];
      } else {
        x[ind] = rand() % 2 == 0 ? xl[ind] : xu[ind];
      }
    } else {
      if(v->getType() == Binary || v->getType() == ImplBin) {
        x[ind] = best[ind] > 0.5 ? 0 : 1;
      } else if(v->getType() == Integer || v->getType() == ImplInt) {
        ldist = best[ind] - xl[ind];
        udist = xu[ind] - best[ind];
        if(ldist < udist) {
          x[ind] = ceil(xu[ind] - udist / 2.0);
        } else if(udist < ldist) {
          x[ind] = floor(xl[ind] + ldist / 2.0);
        } else {
          if(rand() % 2 == 0) {
            x[ind] = floor(xl[ind] + ldist / 2.0);
          } else {
            x[ind] = ceil(xu[ind] - udist / 2.0);
          }
        }
      } else {
        ldist = best[ind] - xl[ind];
        udist = xu[ind] - best[ind];
        if(ldist < udist) {
          x[ind] = xu[ind] - udist / 2.0;
        } else if(udist < ldist) {
          x[ind] = xl[ind] + ldist / 2.0;
        } else {
          if(rand() % 2 == 0) {
            x[ind] = xl[ind] + ldist / 2.0;
          } else {
            x[ind] = xu[ind] - udist / 2.0;
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
  out << me_ << "number of points checked for feasibility : " << stats_->checked
      << std::endl;
  out << me_ << "number of feasible solutions found : " << stats_->numSol
      << std::endl;
  out << me_ << "Time taken : " << stats_->time << std::endl;
}
