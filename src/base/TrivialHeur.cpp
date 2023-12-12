//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file TrivialHeur.cpp
 * \brief Define the TrivialHeur class for diving heuristics for MINLPs.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 * Implements the class TrivialHeur.
 */

#include <cmath>
#include <cstring>

#include "Environment.h"
#include "MinotaurConfig.h"
#include "Problem.h"
#include "SolutionPool.h"
#include "TrivialHeur.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

//#define SPEW 0

const std::string TrivialHeur::me_ = "Trivial Heuristic: ";

TrivialHeur::TrivialHeur(EnvPtr env, ProblemPtr p)
  : env_(env),
    p_(p)
{
  stats_ = (TrivialHeurStats*)new TrivialHeurStats();
  stats_->isZero = false;
  stats_->isLB = false;
  stats_->isUB = false;
  stats_->numRand = 0;
  stats_->isLockPoint = false;
}

TrivialHeur::~TrivialHeur()
{
  delete stats_;
}

void TrivialHeur::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  UInt n = p_->getNumVars();
  double* x = new double[n];
  double* xl = new double[n];
  double* xu = new double[n];
  SolutionPtr sol;
  VariablePtr v;
  ObjectivePtr obj = p_->getObjective();
  bool checkzero = true, checklb = true, checkub = true;
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

  for(VariableConstIterator vit = p_->varsBegin(); vit != p_->varsEnd();
      ++vit) {
    v = *vit;
    std::memset(xl, 0, n * sizeof(double));
    std::memset(xu, 0, n * sizeof(double));
    if(v->getLb() > -INFINITY) {
      xl[v->getIndex()] = v->getLb();
    } else {
      checklb = false;
    }
    if(v->getUb() < INFINITY) {
      xu[v->getIndex()] = v->getUb();
    } else {
      checkub = false;
    }
  }

  if(checklb) {
    if(isFeasible_(xl)) {
      error = 0;
      sol = (SolutionPtr) new Solution(obj->eval(xl, &error), xl, p_);
      s_pool->addSolution(sol);
      stats_->isLB = true;
    }
  }
  if(checkub) {
    if(isFeasible_(xu)) {
      error = 0;
      sol = (SolutionPtr) new Solution(obj->eval(xu, &error), xu, p_);
      s_pool->addSolution(sol);
      stats_->isUB = true;
    }
  }

  delete[] xl;
  delete[] xu;

  if(getLockPoint_(x)) {
    if(isFeasible_(x)) {
      sol = (SolutionPtr) new Solution(obj->eval(x, &error), x, p_);
      s_pool->addSolution(sol);
      stats_->isLockPoint = true;
    }
  }

  delete[] x;
}

bool TrivialHeur::getLockPoint_(double* x)
{
  FunctionPtr f;
  LinearFunctionPtr lf;
  ConstraintPtr c;
  int* lockNum = new int[p_->getNumVars()];
  int consType = 0; // consType = 0 means lock point is not computed,
                    //            1 then that variable is set to UB,
                    //            -1 then that variable is set to LB
  double tol = 1e-8;

  std::memset(lockNum, 0, p_->getNumVars() * sizeof(int));

  for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
      ++cit) {
    c = *cit;
    f = c->getFunction();
    if(f->getType() == Linear) {
      if(c->getLb() > -INFINITY && c->getUb() < INFINITY) {
        continue;
      } else if(c->getLb() > -INFINITY) {
        consType = 1;
      } else {
        consType = -1;
      }
      lf = f->getLinearFunction();
      for(VariableGroupConstIterator it = lf->termsBegin();
          it != lf->termsEnd(); ++it) {
        if(it->second > tol) {
          lockNum[it->first->getIndex()] += consType;
        } else {
          lockNum[it->first->getIndex()] -= consType;
        }
      }
    }
  }

  for(UInt i = 0; i < p_->getNumVars(); ++i) {
    if(lockNum[i] == 0) {
      return false;
    } else if(lockNum[i] > 0) {
      x[i] = p_->getVariable(i)->getUb();
    } else {
      x[i] = p_->getVariable(i)->getLb();
    }
  }
  return true;
}

bool TrivialHeur::isFeasible_(const double* x)
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

void TrivialHeur::writeStats(std::ostream& out) const
{
  out << me_ << " 0 was feasible? : " << stats_->isZero << std::endl;
  out << me_ << " lb was feasible? : " << stats_->isLB << std::endl;
  out << me_ << " ub was feasible? : " << stats_->isUB << std::endl;
  out << me_ << " number of random solution feasible : " << stats_->numRand
      << std::endl;
  out << me_ << " lockpoint was feasible? : " << stats_->isLockPoint
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
