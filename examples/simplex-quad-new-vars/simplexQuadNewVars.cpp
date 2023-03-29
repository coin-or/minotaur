//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2009--2022 The MINOTAUR Team
//

/**
 This is example driver code to take a problem create a linear
 relaxation, solve and then add vars according to optimal simplex
 and resolve iteratively
*/

#include <string.h>

#include <cmath>
#include <cstring>
#include <iostream>

#include "EngineFactory.h"
#include "Environment.h"
#include "Function.h"
#include "Glob.h"
#include "LPEngine.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "NlPresHandler.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "Presolver.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "QuadHandler.h"
#include "Relaxation.h"
#include "SimpleTransformer.h"
#include "SimplexQuadCutGen.h"
#include "Solution.h"
#include "Timer.h"
#include "Types.h"

using namespace Minotaur;

bool showCuts = false;
bool showQuadVars = false;
bool allVars = true;
int numCons = 0;
int numVars = 0;
double* dualVecCons = 0;
double* dualVecVars = 0;
int variant = 6;
std::vector<double> optSol = {0, 0, 1, 1,   0,   0,   0,   0,
                              0,  // ex9_2_6
                              0, 0, 0, 0.5, 0.5, 0.5, 0.5, 1};
// std::vector<double> optSol = {50, 75.485880502600196, 93.262254147831101,
//                              8.748134650431270, 128.43232444132138};

struct AuxVars {
  int x1;            // index of the first variable
  int x2;            // index of the second variable
  char productType;  // v - both first and second variables are "Variables"
                     // c - both first and second variables are "Constraints"
                     // m - first is "Variable", second is "Constriant"
  int y;             // index of the auxiliary variable
  double scale;      // Scale y by this amount
  bool isScaled;     // is y scaled?
};

typedef AuxVars* AuxVarsPtr;
typedef std::vector<AuxVarsPtr> AuxVarVector;

void usage() {
  std::cout << "./scuts {name-of-.nl-file} "
            << "--{any-other-minotaur-option}\n";
  return;
}

int findAuxVar(int n, AuxVarVector auxVars) {
  int i = 0;
  for (AuxVarVector::iterator it = auxVars.begin(); it != auxVars.end(); ++it) {
    if ((*it)->y == n) {
      return i;
    }
    ++i;
  }
  std::cout << "COULD NOT FIND\nTHIS SHOULD NOT HAPPEN" << std::endl;
  return -1;
}

double calcSlackVal(int ind, RelaxationPtr rel) {
  ConstraintPtr c = rel->getConstraint(ind);
  double* sol = optSol.data();
  int error = 0;
  double act = c->getActivity(sol, &error);

  if (c->getUb() < INFINITY) {
    return c->getUb() - act;
  } else {
    return c->getLb() - act;
  }
}

double calcyVal(AuxVarsPtr aptr, RelaxationPtr rel) {
  double prod = 0.0;
  switch (aptr->productType) {
    case 'v':
      prod = optSol[aptr->x1] * optSol[aptr->x2];
      break;
    case 'm':
      prod = optSol[aptr->x1] * calcSlackVal(aptr->x2, rel);
      break;
    case 'c':
      prod = calcSlackVal(aptr->x1, rel) * calcSlackVal(aptr->x2, rel);
      break;
  }
  return aptr->isScaled ? prod / aptr->scale : prod;
}

void updateOptSol(RelaxationPtr rel, AuxVarVector auxVars) {
  int nOpt = optSol.size();
  int nVars = rel->getNumVars();
  int found;

  if (nVars == nOpt) {
    return;
  }

  found = findAuxVar(nOpt, auxVars);
  for (int i = found; i < auxVars.size(); ++i) {
    optSol.push_back(calcyVal(auxVars[i], rel));
  }
}

bool checkSol(RelaxationPtr rel, AuxVarVector auxVars) {
  bool is_feas = true;
  ConstraintPtr c;
  int error = 0;
  double act, clb, cub;
  bool ubinf, lbinf;
  double at = 1e-6;
  double rt = 1e-4;

  updateOptSol(rel, auxVars);
  double* x = optSol.data();
  for (ConstraintConstIterator cit = rel->consBegin(); cit != rel->consEnd();
       ++cit) {
    c = *cit;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cub = c->getUb();
      clb = c->getLb();
      ubinf = (act > cub + at) && (cub == 0 || act > cub + fabs(cub) * rt);
      lbinf = (act < clb - at) && (clb == 0 || act < clb - fabs(clb) * rt);
      if (ubinf || lbinf) {
        is_feas = false;
        c->write(std::cout);
      }
    } else {
      std::cout << c->getName() << " Constraint not defined at this point."
                << std::endl;
      is_feas = false;
    }
  }

  return is_feas;
}

int findY(AuxVarVector aux, int x1, int x2, char productType, double& scale,
          bool& isScaled) {
  AuxVarsPtr aptr;
  for (AuxVarVector::iterator it = aux.begin(); it != aux.end(); ++it) {
    aptr = (*it);
    if (aptr->productType == productType) {
      if (aptr->x1 == x1 && aptr->x2 == x2) {
        scale = aptr->scale;
        isScaled = aptr->isScaled;
        return aptr->y;
      }
    }
  }
  return -1;  // Does not exist
}

PresolverPtr createPres(EnvPtr env, ProblemPtr inst, HandlerVector& handlers) {
  // create handlers for presolve
  PresolverPtr pres = PresolverPtr();  // NULL

  inst->calculateSize();
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler = (LinearHandlerPtr) new LinearHandler(env, inst);
    handlers.push_back(lhandler);
    if (inst->isQP() || inst->isQuadratic() || inst->isLinear()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    lhandler->setPreOptDualFix(true);

    if (!inst->isLinear() && (inst->isQP() || inst->isQuadratic())) {
      if (true == env->getOptions()->findBool("cgtoqf")->getValue()) {
        QuadHandlerPtr qhand = (QuadHandlerPtr) new QuadHandler(env, inst);
        handlers.push_back(qhand);
      } else {
        if (true == env->getOptions()->findBool("nl_presolve")->getValue()) {
          NlPresHandlerPtr nlhand =
              (NlPresHandlerPtr) new NlPresHandler(env, inst);
          handlers.push_back(nlhand);
        }
      }
    }

    if (!(inst->isLinear() || inst->isQP() || inst->isQuadratic()) &&
        true == env->getOptions()->findBool("nl_presolve")->getValue()) {
      NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env, inst);
      handlers.push_back(nlhand);
    }

    // write the names.
    env->getLogger()->msgStream(LogExtraInfo)
        << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); ++h) {
      env->getLogger()->msgStream(LogExtraInfo)
          << "  " << (*h)->getName() << std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(inst, env, handlers);
  pres->standardize();
  return pres;
}

void writeSol(EnvPtr env, VarVector* orig_v, PresolverPtr pres, SolutionPtr sol,
              SolveStatus status, MINOTAUR_AMPL::AMPLInterface* iface) {
  Solution* final_sol = 0;

  if (sol) {
    final_sol = pres->getPostSol(sol);
  }

  if (env->getOptions()->findFlag("AMPL")->getValue() ||
      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(final_sol, status);
  }

  if (final_sol &&
      env->getOptions()->findBool("display_solution")->getValue()) {
    final_sol->writePrimal(env->getLogger()->msgStream(LogNone), orig_v);
  }

  if (final_sol) {
    delete final_sol;
  }
}

RelaxationPtr getRelaxation(EnvPtr env, ProblemPtr p, int& status,
                            std::map<std::pair<int, int>, int>& map4origAux) {
  LinearHandlerPtr lhand = (LinearHandlerPtr) new LinearHandler(env, p);
  QuadHandlerPtr qhand = (QuadHandlerPtr) new QuadHandler(env, p);
  RelaxationPtr rel = (RelaxationPtr) new Relaxation(env);
  RelaxationPtr shuffrel;
  FunctionType ftype;
  ConstraintPtr c;
  bool is_inf = false;

  rel->setProblem(p);
  for (ConstraintConstIterator cit = p->consBegin(); cit != p->consEnd();
       ++cit) {
    // p is a transformed problem hence will have linear objective
    c = *cit;
    ftype = c->getFunctionType();
    if (Linear != ftype) {
      qhand->addConstraint(c);
    }
  }
  lhand->relaxInitFull(rel, &is_inf);
  if (is_inf) {
    status = 1;
  }
  qhand->relaxInitFull(rel, &is_inf);
  if (is_inf) {
    status = 1;
  }

  qhand->fillmap4auxVars(map4origAux);
  // shuffrel =
  //  (RelaxationPtr) new Relaxation(rel->shuffle(false, true, env), env);
  // shuffrel->setProblem(p);

  delete lhand;
  delete qhand;
  // delete rel;

  return rel;
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, VariablePtr x, double lb,
                  double ub, double scale, bool isScaled) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;
  ConstraintPtr c;

  // Secant
  lf->incTerm(y, 1.0);
  lf->incTerm(x, isScaled ? -(lb + ub) / scale : -(lb + ub));
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, -INFINITY, isScaled ? -lb * ub / scale : -lb * ub);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at lb
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x, isScaled ? -2 * lb / scale : -2 * lb);
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, isScaled ? -lb * lb / scale : -lb * lb, INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at ub
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x, isScaled ? -2 * ub / scale : -2 * ub);
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, isScaled ? -ub * ub / scale : -ub * ub, INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, VariablePtr x1,
                  VariablePtr x2, double l1, double u1, double l2, double u2,
                  double scale, bool isScaled) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;
  ConstraintPtr c;

  // Secant at (l1, u2)
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, isScaled ? -u2 / scale : -u2);
  lf->incTerm(x2, isScaled ? -l1 / scale : -l1);
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, -INFINITY, isScaled ? -l1 * u2 / scale : -l1 * u2);
  if (showCuts) {
    c->write(std::cout);
  }

  // Secant at (u1, l2)
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, isScaled ? -l2 / scale : -l2);
  lf->incTerm(x2, isScaled ? -u1 / scale : -u1);
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, -INFINITY, isScaled ? -u1 * l2 / scale : -u1 * l2);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at (l1, l2)
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, isScaled ? -l2 / scale : -l2);
  lf->incTerm(x2, isScaled ? -l1 / scale : -l1);
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, isScaled ? -l1 * l2 / scale : -l1 * l2, INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at (u1, u2)
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, isScaled ? -u2 / scale : -u2);
  lf->incTerm(x2, isScaled ? -u1 / scale : -u1);
  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, isScaled ? -u1 * u2 / scale : -u1 * u2, INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, LinearFunctionPtr lf,
                  double d, double lb, double ub, double scale, bool isScaled) {
  LinearFunctionPtr lf1 = lf->clone();
  FunctionPtr f;
  ConstraintPtr c;

  // Secant
  lf1->multiply(isScaled ? -(lb + ub) / scale : -(lb + ub));
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, -INFINITY,
      isScaled ? ((lb + ub) * d - lb * ub) / scale : (lb + ub) * d - lb * ub);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at lb
  lf1 = lf->clone();
  lf1->multiply(isScaled ? -2 * lb / scale : -2 * lb);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, isScaled ? (2 * lb * d - lb * lb) / scale : 2 * lb * d - lb * lb,
      INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at ub
  lf1 = lf->clone();
  lf1->multiply(isScaled ? -2 * ub / scale : -2 * ub);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, isScaled ? (2 * ub * d - ub * ub) / scale : 2 * ub * d - ub * ub,
      INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, LinearFunctionPtr lf1,
                  double d1, LinearFunctionPtr lf2, double d2, double l1,
                  double u1, double l2, double u2, double scale,
                  bool isScaled) {
  LinearFunctionPtr lfnew, lf1clone, lf2clone;
  FunctionPtr f;
  ConstraintPtr c;

  // Secant at (l1, u2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(isScaled ? -u2 / scale : -u2);
  lf2clone->multiply(isScaled ? -l1 / scale : -l1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  c = rel->newConstraint(f, -INFINITY,
                         isScaled ? (u2 * d1 + l1 * d2 - l1 * u2) / scale
                                  : u2 * d1 + l1 * d2 - l1 * u2);
  if (showCuts) {
    c->write(std::cout);
  }
  delete lf1clone;
  delete lf2clone;

  // Secant at (u1, l2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(isScaled ? -l2 / scale : -l2);
  lf2clone->multiply(isScaled ? -u1 / scale : -u1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  c = rel->newConstraint(f, -INFINITY,
                         isScaled ? (l2 * d1 + u1 * d2 - u1 * l2) / scale
                                  : l2 * d1 + u1 * d2 - u1 * l2);
  if (showCuts) {
    c->write(std::cout);
  }
  delete lf1clone;
  delete lf2clone;

  // Tangent at (l1, l2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(isScaled ? -l2 / scale : -l2);
  lf2clone->multiply(isScaled ? -l1 / scale : -l1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  c = rel->newConstraint(f,
                         isScaled ? (l2 * d1 + l1 * d2 - l1 * l2) / scale
                                  : l2 * d1 + l1 * d2 - l1 * l2,
                         INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }
  delete lf1clone;
  delete lf2clone;

  // Tangent at (u1, u2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(isScaled ? -u2 / scale : -u2);
  lf2clone->multiply(isScaled ? -u1 / scale : -u1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  c = rel->newConstraint(f,
                         isScaled ? (u2 * d1 + u1 * d2 - u1 * u2) / scale
                                  : u2 * d1 + u1 * d2 - u1 * u2,
                         INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }
  delete lf1clone;
  delete lf2clone;
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, VariablePtr x,
                  LinearFunctionPtr lf, double d, double l1, double u1,
                  double l2, double u2, double scale, bool isScaled) {
  LinearFunctionPtr lf1 = lf->clone();
  FunctionPtr f;
  ConstraintPtr c;

  // Secant at (l1, u2)
  lf1->multiply(isScaled ? -l1 / scale : -l1);
  lf1->incTerm(x, isScaled ? -u2 / scale : -u2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, -INFINITY, isScaled ? (l1 * d - l1 * u2) / scale : l1 * d - l1 * u2);
  if (showCuts) {
    c->write(std::cout);
  }

  // Secant at (u1, l2)
  lf1 = lf->clone();
  lf1->multiply(isScaled ? -u1 / scale : -u1);
  lf1->incTerm(x, isScaled ? -l2 / scale : -l2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, -INFINITY, isScaled ? (u1 * d - u1 * l2) / scale : u1 * d - u1 * l2);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at (l1, l2)
  lf1 = lf->clone();
  lf1->multiply(isScaled ? -l1 / scale : -l1);
  lf1->incTerm(x, isScaled ? -l2 / scale : -l2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, isScaled ? (l1 * d - l1 * l2) / scale : l1 * d - l1 * l2, INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }

  // Tangent at (u1, u2)
  lf1 = lf->clone();
  lf1->multiply(isScaled ? -u1 / scale : -u1);
  lf1->incTerm(x, isScaled ? -u2 / scale : -u2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  c = rel->newConstraint(
      f, isScaled ? (u1 * d - u1 * u2) / scale : u1 * d - u1 * u2, INFINITY);
  if (showCuts) {
    c->write(std::cout);
  }
}

int getNewVar(RelaxationPtr rel, SimplexQuadCutGenPtr cutgen, int x1, int x2,
              char productType, double& scale, bool& isScaled) {
  double l1, u1, l2, u2;
  double lb = 0.0, ub = 0.0;
  VariablePtr v, v1, v2;
  LinearFunctionPtr lf1, lf2;
  double d1, d2;

  switch (productType) {
    case 'v':
      if (x1 == x2) {
        v1 = rel->getVariable(x1);
        l1 = v1->getLb();
        u1 = v1->getUb();
        BoundsOnSquare(l1, u1, lb, ub);
        scale = std::max(fabs(lb), fabs(ub));
        isScaled = scale > 1000;
        if (isScaled) {
          lb = lb / scale;
          ub = ub / scale;
        }
        v = rel->newVariable(lb, ub, Continuous);
        if (showQuadVars) {
          if (scale > 1000) {
            std::cout << scale << "*" << v->getName() << " = " << v1->getName()
                      << "^2" << std::endl;
          } else {
            std::cout << v->getName() << " = " << v1->getName() << "^2"
                      << std::endl;
          }
        }
        addMcCormick(rel, v, v1, l1, u1, scale, isScaled);
      } else {
        v1 = rel->getVariable(x1);
        v2 = rel->getVariable(x2);
        l1 = v1->getLb();
        u1 = v1->getUb();
        l2 = v2->getLb();
        u2 = v2->getUb();
        BoundsOnProduct(true, l1, u1, l2, u2, lb, ub);
        scale = std::max(fabs(lb), fabs(ub));
        isScaled = scale > 1000;
        if (isScaled) {
          lb = lb / scale;
          ub = ub / scale;
        }
        v = rel->newVariable(lb, ub, Continuous);
        if (showQuadVars) {
          if (scale > 1000) {
            std::cout << scale << "*" << v->getName() << " = " << v1->getName()
                      << "*" << v2->getName() << std::endl;
          } else {
            std::cout << v->getName() << " = " << v1->getName() << "*"
                      << v2->getName() << std::endl;
          }
        }
        addMcCormick(rel, v, v1, v2, l1, u1, l2, u2, scale, isScaled);
      }
      break;
    case 'c':
      if (x1 == x2) {
        l1 = cutgen->getSlackLb(x1);
        u1 = cutgen->getSlackUb(x1);
        BoundsOnSquare(l1, u1, lb, ub);
        scale = std::max(fabs(lb), fabs(ub));
        isScaled = scale > 1000;
        if (isScaled) {
          lb = lb / scale;
          ub = ub / scale;
        }
        v = rel->newVariable(lb, ub, Continuous);
        lf1 = (LinearFunctionPtr) new LinearFunction();
        d1 = 0.0;
        cutgen->getAffineFnForSlack(rel, x1, lf1, d1);
        if (showQuadVars) {
          if (scale > 1000) {
            std::cout << scale << "*" << v->getName() << " = "
                      << rel->getConstraint(x1)->getName() << "^2" << std::endl;
          } else {
            std::cout << v->getName() << " = "
                      << rel->getConstraint(x1)->getName() << "^2" << std::endl;
          }
        }
        addMcCormick(rel, v, lf1, d1, l1, u1, scale, isScaled);
        delete lf1;
      } else {
        l1 = cutgen->getSlackLb(x1);
        u1 = cutgen->getSlackUb(x1);
        l2 = cutgen->getSlackLb(x2);
        u2 = cutgen->getSlackUb(x2);
        BoundsOnProduct(true, l1, u1, l2, u2, lb, ub);
        scale = std::max(fabs(lb), fabs(ub));
        isScaled = scale > 1000;
        if (isScaled) {
          lb = lb / scale;
          ub = ub / scale;
        }
        v = rel->newVariable(lb, ub, Continuous);
        lf1 = (LinearFunctionPtr) new LinearFunction();
        d1 = 0.0;
        cutgen->getAffineFnForSlack(rel, x1, lf1, d1);
        lf2 = (LinearFunctionPtr) new LinearFunction();
        d2 = 0.0;
        cutgen->getAffineFnForSlack(rel, x2, lf2, d2);
        if (showQuadVars) {
          if (scale > 1000) {
            std::cout << scale << "*" << v->getName() << " = "
                      << rel->getConstraint(x1)->getName() << "*"
                      << rel->getConstraint(x2)->getName() << std::endl;
          } else {
            std::cout << v->getName() << " = "
                      << rel->getConstraint(x1)->getName() << "*"
                      << rel->getConstraint(x2)->getName() << std::endl;
          }
        }
        addMcCormick(rel, v, lf1, d1, lf2, d2, l1, u1, l2, u2, scale, isScaled);
        delete lf1;
        delete lf2;
      }
      break;
    case 'm':
      v1 = rel->getVariable(x1);
      l1 = v1->getLb();
      u1 = v1->getUb();
      l2 = cutgen->getSlackLb(x2);
      u2 = cutgen->getSlackUb(x2);
      BoundsOnProduct(true, l1, u1, l2, u2, lb, ub);
      scale = std::max(fabs(lb), fabs(ub));
      isScaled = scale > 1000;
      if (isScaled) {
        lb = lb / scale;
        ub = ub / scale;
      }
      v = rel->newVariable(lb, ub, Continuous);
      lf2 = (LinearFunctionPtr) new LinearFunction();
      d2 = 0.0;
      cutgen->getAffineFnForSlack(rel, x2, lf2, d2);
      if (showQuadVars) {
        if (scale > 1000) {
          std::cout << scale << "*" << v->getName() << " = " << v1->getName()
                    << "*" << rel->getConstraint(x2)->getName() << std::endl;
        } else {
          std::cout << v->getName() << " = " << v1->getName() << "*"
                    << rel->getConstraint(x2)->getName() << std::endl;
        }
      }
      addMcCormick(rel, v, v1, lf2, d2, l1, u1, l2, u2, scale, isScaled);
      delete lf2;
  }
  return v->getIndex();
}

void addTerm(std::map<int, double>& t, int v, double coef, double scale,
             bool isScaled) {
  coef = isScaled ? scale * coef : coef;
  if (fabs(coef) > 1e-6) {
    std::map<int, double>::iterator it = t.find(v);
    if (it == t.end()) {
      t.insert(std::make_pair(v, coef));
    } else {
      double nv = it->second + coef;
      if (fabs(nv) < 1e-6) {
        t.erase(v);
      } else {
        t[v] = nv;
      }
    }
  }
}

void addCutToRel(RelaxationPtr rel, SimplexQuadCutGenPtr cutgen,
                 double* cutCoefo, double* cutCoefs, double cutConst,
                 double clb, double cub, int numcons) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction(
      cutCoefo, rel->varsBegin(), rel->varsEnd(), 1e-9);
  LinearFunctionPtr lfs;
  FunctionPtr f;
  double ds;
  std::map<int, double>::iterator it;
  ConstraintPtr c;
  double lb, ub;

  if (cutCoefs) {
    for (int i = 0; i < numcons; ++i) {
      if (fabs(cutCoefs[i]) < 1e-6) {
        continue;
      }
      lb = rel->getConstraint(i)->getLb();
      ub = rel->getConstraint(i)->getUb();
      if (fabs(ub - lb) < 1e-6) {
        continue;
      }
      lfs = (LinearFunctionPtr) new LinearFunction();
      ds = 0.0;
      cutgen->getAffineFnForSlack(rel, i, lfs, ds);
      lfs->multiply(cutCoefs[i]);
      lf->add(lfs);
      cutConst += cutCoefs[i] * ds;
      delete lfs;
    }
  }

  f = (FunctionPtr) new Function(lf);
  c = rel->newConstraint(f, clb > -INFINITY ? clb - cutConst : -INFINITY,
                         cub < INFINITY ? cub - cutConst : INFINITY);
  if (showCuts) {
    std::cout << "Actual cut" << std::endl;
    c->write(std::cout);
  }
}

void linearize(double x1val, double x2val, double coef, int v1, int v2,
               double* t1, double* t2, double& cutConst) {
  cutConst -= coef * x1val * x2val;
  t1[v1] += coef * x2val;
  t2[v2] += coef * x1val;
}

void dualLinearze(double coef, int v1, int v2, bool lower1, bool lower2,
                  double l1, double u1, double l2, double u2, double w1,
                  double w2, double* t1, double* t2, double& cutConst,
                  bool under) {
  double normal = w1 + w2;
  w1 = w1 / normal;
  w2 = w2 / normal;
  if (under) {
    if (coef > 1e-8) {
      if (lower2) {
        std::swap(w1, w2);
      }
      t1[v1] += coef * (w1 * l2 + w2 * u2);
      t2[v2] += coef * (w1 * l1 + w2 * u1);
      cutConst -= coef * (w1 * l1 * l2 + w2 * u1 * u2);
    } else {
      if (!lower1) {
        std::swap(w1, w2);
      }
      t1[v1] += coef * (w1 * u2 + w2 * l2);
      t2[v2] += coef * (w1 * l1 + w2 * u1);
      cutConst -= coef * (w1 * l1 * u2 + w2 * u1 * l2);
    }
  } else {
    if (coef < -1e-8) {
      if (lower2) {
        std::swap(w1, w2);
      }
      t1[v1] += coef * (w1 * l2 + w2 * u2);
      t2[v2] += coef * (w1 * l1 + w2 * u1);
      cutConst -= coef * (w1 * l1 * l2 + w2 * u1 * u2);
    } else {
      if (!lower1) {
        std::swap(w1, w2);
      }
      t1[v1] += coef * (w1 * u2 + w2 * l2);
      t2[v2] += coef * (w1 * l1 + w2 * u1);
      cutConst -= coef * (w1 * l1 * u2 + w2 * u1 * l2);
    }
  }
}

bool updateRel(EnvPtr env, RelaxationPtr rel, SimplexQuadCutGenPtr cutgen,
               AuxVarVector& auxVars, const double* x, LPEnginePtr lpe,
               ConstraintPtr c, bool ubinf, bool lbinf, double obj) {
  SubstQuadPtr oxo = 0, oxs = 0, sxs = 0, oxo1 = 0, oxs1 = 0;
  int oldnumvars = rel->getNumVars();
  int oldnumcons = rel->getNumCons();
  double* cutCoefo = new double[oldnumvars];
  double* cutCoefo1 = 0;
  double* cutCoefs = 0;
  double cutConst = 0.0;
  double etol = 1e-8;
  int y;
  AuxVarsPtr aptr;
  double scale = 1.0;
  double clb, cub;
  bool isScaled = false;
  bool addVar, lower1, lower2, quad2 = false;
  VariablePtr v1, v2;
  int numTerms, error = 0;
  std::vector<int>::size_type oxosize = 0, oxssize = 0, sxssize = 0,
                              oxo1size = 0, oxs1size = 0;
  std::map<int, double> newVars, newVars1;

  memset(cutCoefo, 0, oldnumvars * sizeof(double));

  oxo = (SubstQuadPtr) new SubstQuad();
  oxs = (SubstQuadPtr) new SubstQuad();
  if (variant == 6) {
    int count = 0;
    while (count < 2) {
      ++count;
      if (count == 1) {
        cutgen->getQuadraticBNB(c, x, rel, oxo, oxs, cutCoefo, cutConst, count);
        oxosize = oxo->ind1.size();
        oxssize = oxs->ind1.size();
      }
      if (count == 2) {
        cutCoefo1 = new double[rel->getNumVars()];
        memset(cutCoefo1, 0, oldnumvars * sizeof(double));
        oxo1 = (SubstQuadPtr) new SubstQuad();
        oxs1 = (SubstQuadPtr) new SubstQuad();
        cutgen->getQuadraticBNB(c, x, rel, oxo1, oxs1, cutCoefo1, cutConst,
                                count);
        oxo1size = oxo1->ind1.size();
        oxs1size = oxs1->ind1.size();
        quad2 = true;
      }
    }
  } else {
    cutCoefs = new double[rel->getNumCons()];
    memset(cutCoefs, 0, rel->getNumCons() * sizeof(double));
    sxs = (SubstQuadPtr) new SubstQuad();
    cutgen->getQuadratic(c, x, rel, oxo, oxs, sxs, cutCoefo, cutCoefs,
                         cutConst);
    oxosize = oxo->ind1.size();
    oxssize = oxs->ind1.size();
    sxssize = sxs->ind1.size();
  }

  if (variant == 1) {
    numTerms = oxosize + oxssize + sxssize;
    std::cout << "Num Terms = " << numTerms << std::endl;
    if (numTerms > ceil(0.1 * (rel->getNumVars() + rel->getNumCons())) &&
        numTerms > 10) {
      return false;
    }
  }

  for (std::vector<int>::size_type i = 0; i < oxosize; ++i) {
    if (fabs(oxo->val[i]) < etol) {
      continue;
    }
    y = findY(auxVars, oxo->ind1[i], oxo->ind2[i], 'v', scale, isScaled);
    if (y == -1) {
      if (oxo->ind1[i] == oxo->ind2[i]) {
        if (allVars) {
          addVar = true;
        } else {
          v1 = rel->getVariable(oxo->ind1[i]);
          addVar = false;
          if ((ubinf && oxo->val[i] > etol) || (lbinf && oxo->val[i] < etol)) {
            // assume that xval at the current point is at its bound because of
            // nonbasic var
            linearize(x[oxo->ind1[i]], x[oxo->ind2[i]], oxo->val[i],
                      oxo->ind1[i], oxo->ind2[i], cutCoefo, cutCoefo, cutConst);
          } else {
            linearize(v1->getLb(), v1->getUb(), oxo->val[i], oxo->ind1[i],
                      oxo->ind2[i], cutCoefo, cutCoefo, cutConst);
          }
        }
      } else {
        if (allVars) {
          addVar = true;
        } else {
          v1 = rel->getVariable(oxo->ind1[i]);
          v2 = rel->getVariable(oxo->ind2[i]);
          lower1 = fabs(x[oxo->ind1[i]] - v1->getLb()) < etol;
          lower2 = fabs(x[oxo->ind2[i]] - v2->getLb()) < etol;
          if ((ubinf && oxo->val[i] > etol) || (lbinf && oxo->val[i] < etol)) {
            if (lower1 == lower2) {
              addVar = false;
              linearize(x[oxo->ind1[i]], x[oxo->ind2[i]], oxo->val[i],
                        oxo->ind1[i], oxo->ind2[i], cutCoefo, cutCoefo,
                        cutConst);
            } else {
              if (variant == 2) {
                double dualProd =
                    fabs(dualVecVars[oxo->ind1[i]] * dualVecVars[oxo->ind2[i]]);
                if (dualProd >= obj) {
                  addVar = true;
                } else {
                  dualLinearze(oxo->val[i], oxo->ind1[i], oxo->ind2[i], lower1,
                               lower2, v1->getLb(), v1->getUb(), v2->getLb(),
                               v2->getUb(), fabs(dualVecVars[oxo->ind1[i]]),
                               fabs(dualVecVars[oxo->ind2[i]]), cutCoefo,
                               cutCoefo, cutConst, ubinf);
                }
              } else {
                addVar = true;
              }
            }
          } else {
            if (lower1 == lower2) {
              if (variant == 2) {
                double dualProd =
                    fabs(dualVecVars[oxo->ind1[i]] * dualVecVars[oxo->ind2[i]]);
                if (dualProd >= obj) {
                  addVar = true;
                } else {
                  dualLinearze(oxo->val[i], oxo->ind1[i], oxo->ind2[i], lower1,
                               lower2, v1->getLb(), v1->getUb(), v2->getLb(),
                               v2->getUb(), fabs(dualVecVars[oxo->ind1[i]]),
                               fabs(dualVecVars[oxo->ind2[i]]), cutCoefo,
                               cutCoefo, cutConst, ubinf);
                }
              } else {
                addVar = true;
              }
            } else {
              addVar = false;
              linearize(x[oxo->ind1[i]], x[oxo->ind2[i]], oxo->val[i],
                        oxo->ind1[i], oxo->ind2[i], cutCoefo, cutCoefo,
                        cutConst);
            }
          }
        }
      }
      if (addVar) {
        y = getNewVar(rel, cutgen, oxo->ind1[i], oxo->ind2[i], 'v', scale,
                      isScaled);
        aptr = (AuxVarsPtr) new AuxVars();
        aptr->x1 = oxo->ind1[i];
        aptr->x2 = oxo->ind2[i];
        aptr->productType = 'v';
        aptr->y = y;
        aptr->isScaled = isScaled;
        aptr->scale = isScaled ? scale : 1.0;
        auxVars.push_back(aptr);
        addTerm(newVars, y, oxo->val[i], scale, isScaled);
      }
    } else {
      addTerm(newVars, y, oxo->val[i], scale, isScaled);
    }
  }

  for (std::vector<int>::size_type i = 0; i < oxssize; ++i) {
    if (fabs(oxs->val[i]) < etol) {
      continue;
    }
    clb = rel->getConstraint(oxs->ind2[i])->getLb();
    cub = rel->getConstraint(oxs->ind2[i])->getUb();
    if (cub - clb < etol) {
      continue;
    }
    y = findY(auxVars, oxs->ind1[i], oxs->ind2[i], 'm', scale, isScaled);
    if (y == -1) {
      if (allVars) {
        addVar = true;
      } else {
        v1 = rel->getVariable(oxs->ind1[i]);
        lower1 = fabs(x[oxs->ind1[i]] - v1->getLb()) < etol;
        // We assume nonbasic slack is at zero
        lower2 = cutgen->getSlackUb(oxs->ind2[i]) > etol;
        if ((ubinf && oxs->val[i] > etol) || (lbinf && oxs->val[i] < etol)) {
          if (lower1 == lower2) {
            addVar = false;
            linearize(x[oxs->ind1[i]], 0.0, oxs->val[i], oxs->ind1[i],
                      oxs->ind2[i], cutCoefo, cutCoefs, cutConst);
          } else {
            if (variant == 2) {
              double dualProd =
                  fabs(dualVecVars[oxs->ind1[i]] * dualVecCons[oxs->ind2[i]]);
              if (dualProd >= obj) {
                addVar = true;
              } else {
                dualLinearze(oxs->val[i], oxs->ind1[i], oxs->ind2[i], lower1,
                             lower2, v1->getLb(), v1->getUb(),
                             cutgen->getSlackLb(oxs->ind2[i]),
                             cutgen->getSlackUb(oxs->ind2[i]),
                             fabs(dualVecVars[oxs->ind1[i]]),
                             fabs(dualVecCons[oxs->ind2[i]]), cutCoefo,
                             cutCoefs, cutConst, ubinf);
              }
            } else {
              addVar = true;
            }
          }
        } else {
          if (lower1 == lower2) {
            if (variant == 2) {
              double dualProd =
                  fabs(dualVecVars[oxs->ind1[i]] * dualVecVars[oxs->ind2[i]]);
              if (dualProd >= obj) {
                addVar = true;
              } else {
                dualLinearze(oxs->val[i], oxs->ind1[i], oxs->ind2[i], lower1,
                             lower2, v1->getLb(), v1->getUb(),
                             cutgen->getSlackLb(oxs->ind2[i]),
                             cutgen->getSlackUb(oxs->ind2[i]),
                             fabs(dualVecVars[oxs->ind1[i]]),
                             fabs(dualVecCons[oxs->ind2[i]]), cutCoefo,
                             cutCoefs, cutConst, ubinf);
              }
            } else {
              addVar = true;
            }
          } else {
            linearize(x[oxs->ind1[i]], 0.0, oxs->val[i], oxs->ind1[i],
                      oxs->ind2[i], cutCoefo, cutCoefs, cutConst);
          }
        }
      }
      if (addVar) {
        y = getNewVar(rel, cutgen, oxs->ind1[i], oxs->ind2[i], 'm', scale,
                      isScaled);
        aptr = (AuxVarsPtr) new AuxVars();
        aptr->x1 = oxs->ind1[i];
        aptr->x2 = oxs->ind2[i];
        aptr->productType = 'm';
        aptr->y = y;
        aptr->isScaled = isScaled;
        aptr->scale = isScaled ? scale : 1.0;
        auxVars.push_back(aptr);
        addTerm(newVars, y, oxs->val[i], scale, isScaled);
      }
    } else {
      addTerm(newVars, y, oxs->val[i], scale, isScaled);
    }
  }

  for (std::vector<int>::size_type i = 0; i < sxssize; ++i) {
    if (fabs(sxs->val[i]) < etol) {
      continue;
    }
    clb = rel->getConstraint(sxs->ind1[i])->getLb();
    cub = rel->getConstraint(sxs->ind1[i])->getUb();
    if (cub - clb < etol) {
      continue;
    }
    clb = rel->getConstraint(sxs->ind2[i])->getLb();
    cub = rel->getConstraint(sxs->ind2[i])->getUb();
    if (cub - clb < etol) {
      continue;
    }
    y = findY(auxVars, sxs->ind1[i], sxs->ind2[i], 'c', scale, isScaled);
    if (y == -1) {
      if (sxs->ind1[i] == sxs->ind2[i]) {
        if (allVars) {
          addVar = true;
        } else {
          addVar = false;
          if ((ubinf && sxs->val[i] > etol) || (lbinf && sxs->val[i] < etol)) {
            // assume nonbasic slack variable are at zero
            linearize(0.0, 0.0, sxs->val[i], sxs->ind1[i], sxs->ind2[i],
                      cutCoefs, cutCoefs, cutConst);
          } else {
            linearize(cutgen->getSlackLb(sxs->ind1[i]),
                      cutgen->getSlackUb(sxs->ind2[i]), sxs->val[i],
                      sxs->ind1[i], sxs->ind2[i], cutCoefs, cutCoefs, cutConst);
          }
        }
      } else {
        if (allVars) {
          addVar = true;
        } else {
          lower1 = cutgen->getSlackUb(sxs->ind1[i]) > etol;
          lower2 = cutgen->getSlackUb(sxs->ind2[i]) > etol;
          if ((ubinf && sxs->val[i] > etol) || (lbinf && sxs->val[i] < etol)) {
            if (lower1 == lower2) {
              addVar = false;
              linearize(0.0, 0.0, sxs->val[i], sxs->ind1[i], sxs->ind2[i],
                        cutCoefs, cutCoefs, cutConst);
            } else {
              if (variant == 2) {
                double dualProd =
                    fabs(dualVecVars[sxs->ind1[i]] * dualVecVars[sxs->ind2[i]]);
                if (dualProd >= obj) {
                  addVar = true;
                } else {
                  dualLinearze(sxs->val[i], sxs->ind1[i], sxs->ind2[i], lower1,
                               lower2, cutgen->getSlackLb(sxs->ind1[i]),
                               cutgen->getSlackUb(sxs->ind1[i]),
                               cutgen->getSlackLb(sxs->ind2[i]),
                               cutgen->getSlackUb(sxs->ind2[i]),
                               fabs(dualVecCons[sxs->ind1[i]]),
                               fabs(dualVecCons[sxs->ind2[i]]), cutCoefs,
                               cutCoefs, cutConst, ubinf);
                }
              } else {
                addVar = true;
              }
            }
          } else {
            if (lower1 == lower2) {
              if (variant == 2) {
                double dualProd =
                    fabs(dualVecVars[sxs->ind1[i]] * dualVecVars[sxs->ind2[i]]);
                if (dualProd >= obj) {
                  addVar = true;
                } else {
                  dualLinearze(sxs->val[i], sxs->ind1[i], sxs->ind2[i], lower1,
                               lower2, cutgen->getSlackLb(sxs->ind1[i]),
                               cutgen->getSlackUb(sxs->ind1[i]),
                               cutgen->getSlackLb(sxs->ind2[i]),
                               cutgen->getSlackUb(sxs->ind2[i]),
                               fabs(dualVecCons[sxs->ind1[i]]),
                               fabs(dualVecCons[sxs->ind2[i]]), cutCoefs,
                               cutCoefs, cutConst, ubinf);
                }
              } else {
                addVar = true;
              }
            } else {
              addVar = false;
              linearize(0.0, 0.0, sxs->val[i], sxs->ind1[i], sxs->ind2[i],
                        cutCoefs, cutCoefs, cutConst);
            }
          }
        }
      }
      if (addVar) {
        y = getNewVar(rel, cutgen, sxs->ind1[i], sxs->ind2[i], 'c', scale,
                      isScaled);
        aptr = (AuxVarsPtr) new AuxVars();
        aptr->x1 = sxs->ind1[i];
        aptr->x2 = sxs->ind2[i];
        aptr->productType = 'c';
        aptr->y = y;
        aptr->isScaled = isScaled;
        aptr->scale = isScaled ? scale : 1.0;
        auxVars.push_back(aptr);
        addTerm(newVars, y, sxs->val[i], scale, isScaled);
      }
    } else {
      addTerm(newVars, y, sxs->val[i], scale, isScaled);
    }
  }

  double* tmp = new double[rel->getNumVars()];
  memcpy(tmp, cutCoefo, oldnumvars * sizeof(double));
  delete[] cutCoefo;
  cutCoefo = tmp;
  memset(cutCoefo + oldnumvars, 0,
         (rel->getNumVars() - oldnumvars) * sizeof(double));
  for (std::map<int, double>::iterator it = newVars.begin();
       it != newVars.end(); ++it) {
    cutCoefo[it->first] += it->second;
  }
  addCutToRel(rel, cutgen, cutCoefo, cutCoefs, cutConst,
              lbinf ? c->getLb() : -INFINITY, ubinf ? c->getUb() : INFINITY,
              oldnumcons);

  if (quad2) {
    for (std::vector<int>::size_type i = 0; i < oxo1size; ++i) {
      if (fabs(oxo1->val[i]) < etol) {
        continue;
      }
      y = findY(auxVars, oxo1->ind1[i], oxo1->ind2[i], 'v', scale, isScaled);
      if (y == -1) {
        y = getNewVar(rel, cutgen, oxo1->ind1[i], oxo1->ind2[i], 'v', scale,
                      isScaled);
        aptr = (AuxVarsPtr) new AuxVars();
        aptr->x1 = oxo1->ind1[i];
        aptr->x2 = oxo1->ind2[i];
        aptr->productType = 'v';
        aptr->y = y;
        aptr->isScaled = isScaled;
        aptr->scale = isScaled ? scale : 1.0;
        auxVars.push_back(aptr);
        addTerm(newVars1, y, oxo1->val[i], scale, isScaled);
      } else {
        addTerm(newVars1, y, oxo1->val[i], scale, isScaled);
      }
    }

    for (std::vector<int>::size_type i = 0; i < oxs1size; ++i) {
      if (fabs(oxs1->val[i]) < etol) {
        continue;
      }
      clb = rel->getConstraint(oxs1->ind2[i])->getLb();
      cub = rel->getConstraint(oxs1->ind2[i])->getUb();
      if (cub - clb < etol) {
        continue;
      }
      y = findY(auxVars, oxs1->ind1[i], oxs1->ind2[i], 'm', scale, isScaled);
      if (y == -1) {
        y = getNewVar(rel, cutgen, oxs1->ind1[i], oxs1->ind2[i], 'm', scale,
                      isScaled);
        aptr = (AuxVarsPtr) new AuxVars();
        aptr->x1 = oxs1->ind1[i];
        aptr->x2 = oxs1->ind2[i];
        aptr->productType = 'm';
        aptr->y = y;
        aptr->isScaled = isScaled;
        aptr->scale = isScaled ? scale : 1.0;
        auxVars.push_back(aptr);
        addTerm(newVars1, y, oxs1->val[i], scale, isScaled);
      } else {
        addTerm(newVars1, y, oxs1->val[i], scale, isScaled);
      }
    }

    double* tmp = new double[rel->getNumVars()];
    memcpy(tmp, cutCoefo1, oldnumvars * sizeof(double));
    delete[] cutCoefo1;
    cutCoefo1 = tmp;
    memset(cutCoefo1 + oldnumvars, 0,
           (rel->getNumVars() - oldnumvars) * sizeof(double));
    for (std::map<int, double>::iterator it = newVars.begin();
         it != newVars.end(); ++it) {
      cutCoefo1[it->first] += it->second;
    }
    addCutToRel(rel, cutgen, cutCoefo1, cutCoefs, cutConst,
                lbinf ? c->getLb() : -INFINITY, ubinf ? c->getUb() : INFINITY,
                oldnumcons);
  }

  if (oxo) {
    delete oxo;
  }
  if (oxs) {
    delete oxs;
  }
  if (sxs) {
    delete sxs;
  }
  if (oxo1) {
    delete oxo1;
  }
  if (oxs1) {
    delete oxs1;
  }
  delete[] cutCoefo;
  if (cutCoefo1) {
    delete[] cutCoefo1;
  }
  if (cutCoefs) {
    delete[] cutCoefs;
  }
  return true;
}

double getTarget() {
  double max = 0;
  for (int i = 0; i < numVars; ++i) {
    if (fabs(dualVecVars[i]) > max) {
      max = fabs(dualVecVars[i]);
    }
  }
  for (int i = 0; i < numCons; ++i) {
    if (fabs(dualVecCons[i]) > max) {
      max = fabs(dualVecCons[i]);
    }
  }
  return max * max / 4.0;
}

bool isFeasible(EnvPtr env, ProblemPtr p, ConstSolutionPtr sol,
                RelaxationPtr rel, AuxVarVector& auxVars, LPEnginePtr lpe,
                bool& separated) {
  ConstraintPtr c;
  double act, clb, cub;
  const double* x = sol->getPrimal();
  int error = 0;
  bool is_feas = true;
  double vio;
  bool ubinf, lbinf;
  double at = 1e-6;
  double rt = 1e-4;
  double target = getTarget();

  SimplexQuadCutGenPtr cutgen =
      (SimplexQuadCutGenPtr) new SimplexQuadCutGen(env, p, lpe);
  cutgen->preprocessSimplexTab();

  for (ConstraintConstIterator cit = p->consBegin(); cit != p->consEnd();
       ++cit) {
    c = *cit;
    if (c->getFunctionType() == Quadratic || c->getFunctionType() == Bilinear) {
      act = c->getActivity(x, &error);
      if (error == 0) {
        cub = c->getUb();
        clb = c->getLb();
        ubinf = (act > cub + at) && (cub == 0 || act > cub + fabs(cub) * rt);
        lbinf = (act < clb - at) && (clb == 0 || act < clb - fabs(clb) * rt);
        if (ubinf || lbinf) {
          is_feas = false;
          if (updateRel(env, rel, cutgen, auxVars, x, lpe, c, ubinf, lbinf,
                        target)) {
            separated = true;
          }
        }
      } else {
        std::cout << c->getName() << " Constraint not defined at this point."
                  << std::endl;
        is_feas = false;
      }
    }
  }

  delete cutgen;
  return is_feas;
}

void resizeDualVec(int newConsSize, int newVarsSize) {
  double* newVec = new double[newConsSize];
  std::memset(newVec, 0.0, newConsSize * sizeof(double));
  std::memcpy(newVec, dualVecCons, numCons * sizeof(double));

  numCons = newConsSize;
  delete[] dualVecCons;
  dualVecCons = newVec;

  newVec = new double[newVarsSize];
  std::memset(newVec, 0.0, newVarsSize * sizeof(double));
  std::memcpy(newVec, dualVecVars, numVars * sizeof(double));

  numVars = newVarsSize;
  delete[] dualVecVars;
  dualVecVars = newVec;
}

RelaxationPtr solveRelaxation(EnvPtr env, ProblemPtr p, RelaxationPtr rel,
                              EngineFactory* efac, bool& is_feas, int& status,
                              AuxVarVector& auxVars, bool last) {
  LPEnginePtr lpe = efac->getLPEngine();
  ConstSolutionPtr sol;
  RelaxationPtr newrel = rel->clone(env);
  const double* dualCons;
  const double* dualVars;
  bool separated = false;
  const Timer* timer = env->getTimer();

  std::cout << "Number of variables = " << rel->getNumVars() << std::endl;
  std::cout << "Number of Constraints = " << rel->getNumCons() << std::endl;

  // if (!checkSol(rel, auxVars)) {
  //  std::cout << "NOT FEASIBLE HERE" << std::endl;
  //}

  lpe->load(rel);
  if (dualVecCons) {
    resizeDualVec(rel->getNumCons(), rel->getNumVars());
    lpe->loadDualWarmStart(numCons, dualVecCons);
    lpe->solve();
  } else {
    numCons = rel->getNumCons();
    dualVecCons = new double[numCons];
    numVars = rel->getNumVars();
    dualVecVars = new double[numVars];
    lpe->solve();
  }
  sol = lpe->getSolution();

  dualCons = sol->getDualOfCons();
  std::memcpy(dualVecCons, dualCons, numCons * sizeof(double));
  dualVars = sol->getDualOfVars();
  std::memcpy(dualVecVars, dualVars, numVars * sizeof(double));

  std::cout << "Lower bound = " << sol->getObjValue() << std::endl;
  if (last) {
    if (newrel) {
      delete newrel;
    }
    return 0;
  }

  double stime = timer->query();
  is_feas = isFeasible(env, p, sol, newrel, auxVars, lpe, separated);
  if (is_feas) {
    std::cout << "Feasible Solution found" << std::endl;
    std::cout << "Upper bound = " << sol->getObjValue() << std::endl;
    return 0;
  }

  if (!separated) {
    std::cout << "All quadratic functions are has sparsity more than "
                 "0.1\nHence point is not separated"
              << std::endl;
    return 0;
  }
  std::cout << "Time taken in cut generation : " << timer->query() - stime
            << std::endl;

  delete lpe;
  delete rel;
  return newrel;
}

void bringObjToCons(ProblemPtr inst) {
  QuadraticFunctionPtr qf = inst->getObjective()->getQuadraticFunction();
  LinearFunctionPtr lf = inst->getObjective()->getLinearFunction();
  LinearFunctionPtr conslf;
  QuadraticFunctionPtr consqf;
  FunctionPtr f;
  double lb = 0.0, ub = 0.0, qlb = 0.0, qub = 0.0;
  VariablePtr y, v1, v2;
  double coef;

  for (VariablePairGroupConstIterator qit = qf->begin(); qit != qf->end();
       ++qit) {
    v1 = qit->first.first;
    v2 = qit->first.second;
    coef = qit->second;
    if (v1->getIndex() == v2->getIndex()) {
      BoundsOnSquare(v1, qlb, qub);
    } else {
      BoundsOnProduct(true, v1, v2, qlb, qub);
    }
    if (coef > 0) {
      lb += qlb > -INFINITY ? coef * qlb : -INFINITY;
      ub += qub < INFINITY ? coef * qub : INFINITY;
    } else {
      lb += qub < INFINITY ? coef * qub : -INFINITY;
      ub += qlb > -INFINITY ? coef * qlb : INFINITY;
    }
  }

  y = inst->newVariable(lb, ub, Continuous, "yobj");
  conslf = (LinearFunctionPtr) new LinearFunction();
  conslf->addTerm(y, -1.0);
  consqf = qf->clone();
  f = (FunctionPtr) new Function(conslf, consqf);
  inst->newConstraint(f, 0.0, 0.0);
  lf->addTerm(y, 1.0);
  inst->removeQuadFromObj();
}

void addTangentsForSqVars(RelaxationPtr rel, AuxVarVector auxVars) {
  AuxVarsPtr aptr;
  double lb, ub;
  VariablePtr x, y;
  LinearFunctionPtr lf;
  FunctionPtr f;

  for (AuxVarVector::iterator it = auxVars.begin(); it != auxVars.end(); ++it) {
    aptr = *it;
    if (aptr->x1 == aptr->x2) {
      if (aptr->productType != 'v') {
        continue;
      }
      x = rel->getVariable(aptr->x1);
      y = rel->getVariable(aptr->y);
      lb = x->getLb();
      ub = x->getUb();
      if (fabs(lb) > 1e-6) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, -1.0);
        lf->addTerm(x, 2 * lb);
        f = (FunctionPtr) new Function(lf);
        rel->newConstraint(f, -INFINITY, lb * lb);
      }
      if (fabs(ub) > 1e-6) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(y, -1.0);
        lf->addTerm(x, 2 * ub);
        f = (FunctionPtr) new Function(lf);
        rel->newConstraint(f, -INFINITY, ub * ub);
      }
    }
  }
}

int main(int argc, char** argv) {
  EnvPtr env = (EnvPtr) new Environment();
  ProblemPtr inst = 0;  // instance that needs to be solved.
  VarVector* orig_v = 0;
  std::string dname, fname;
  MINOTAUR_AMPL::AMPLInterface* iface;
  OptionDBPtr options = env->getOptions();
  HandlerVector handlers;
  int objSense = 1;
  int status = 0;
  TransformerPtr trans;
  EngineFactory* efac = new EngineFactory(env);
  LPEnginePtr bte = LPEnginePtr();
  ProblemPtr p = 0;
  RelaxationPtr rel;
  PresolverPtr pres = 0, pres2;
  FunctionType objType;
  int rounds, err = 0;
  bool is_feas = false;
  std::map<std::pair<int, int>, int> map4origAux;
  AuxVarVector auxVars;
  AuxVarsPtr aptr;

  // read user-specified options
  env->readOptions(argc, argv);
  // any other value not allowed
  options->findInt("pres_freq")->setValue(1);
  options->findBool("use_native_cgraph")->setValue(true);
  options->findBool("cgtoqf")->setValue(true);
  options->findBool("presolve")->setValue(true);
  options->findBool("nl_presolve")->setValue(true);
  options->findBool("lin_presolve")->setValue(true);
  options->findBool("msheur")->setValue(true);
  options->findString("nlp_engine")->setValue("IPOPT");
  variant = options->findInt("simplex_cut_variant")->getValue();
  switch(variant) {
    case 0:
    case 6:
      allVars = true;
      break;
    case 2:
    case 3:
      allVars = false;
      break;
    default:
      std::cout << "Invalid variant" << std::endl;
      std::cout << "Assumed variant 6";
      variant = 6;
      allVars = true;
  }
  fname = options->findString("problem_file")->getValue();
  if ("" == fname) {
    usage();
    goto CLEANUP;
  }

  env->startTimer(err);

  iface = new MINOTAUR_AMPL::AMPLInterface(env, "mglob");
  options->findString("interface_type")->setValue("AMPL");
  inst = iface->readInstance(fname);
  if (!(inst->isLinear() || inst->isQP() || inst->isQuadratic())) {
    env->getLogger()->msgStream(LogError)
        << "Error : "
        << "this example solves Mixed Integer Quadratically Constrained "
           "Quadratic"
        << " problems (MIQCQP). " << fname << " is not an MIQCQP." << std::endl
        << "Problem not solved" << std::endl;
    goto CLEANUP;
  }

  inst->cg2qf();
  inst->calculateSize();
  if (options->findBool("display_problem")->getValue() == true) {
    inst->write(env->getLogger()->msgStream(LogNone), 9);
  }
  if (options->findBool("display_size")->getValue() == true) {
    inst->writeSize(env->getLogger()->msgStream(LogNone));
  }

  if (inst->getObjective() &&
      inst->getObjective()->getObjectiveType() == Maximize) {
    objSense = -1;
    env->getLogger()->msgStream(LogInfo)
        << "objective sense: maximize (will be converted to Minimize)"
        << std::endl;
  } else {
    env->getLogger()->msgStream(LogInfo)
        << "objective sense: minimize" << std::endl;
  }

  handlers.clear();
  orig_v = new VarVector(inst->varsBegin(), inst->varsEnd());
  pres = createPres(env, inst, handlers);
  if (options->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
  for (HandlerVector::iterator it = handlers.begin(); it != handlers.end();
       ++it) {
    delete (*it);
  }
  handlers.clear();

  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo)
        << "status of presolve: " << getSolveStatusString(pres->getStatus())
        << std::endl;
    writeSol(env, orig_v, pres, pres->getSolution(), pres->getStatus(), iface);
    goto CLEANUP;
  }

  objType = inst->getObjective()->getFunctionType();
  if (objType != Linear && objType != Constant) {
    bringObjToCons(inst);
  }

  inst->setNativeDer();
  bte = efac->getLPEngine();
  trans = (SimpTranPtr) new SimpleTransformer(env, inst, bte, 0, 0);
  trans->reformulate(p, handlers, status);
  delete trans;
  env->getLogger()->msgStream(LogExtraInfo)
      << "Presolving transformed problem ... " << std::endl;
  pres2 = (PresolverPtr) new Presolver(p, env, handlers);

  pres2->solve();
  env->getLogger()->msgStream(LogExtraInfo)
      << "Finished presolving transformed problem" << std::endl;

  std::cout << "Number of variables in original problem = "
            << inst->getNumVars() << std::endl;
  std::cout << "Number of Constraints in original problem = "
            << inst->getNumCons() << std::endl;

  rounds = options->findInt("simplex_cut_rounds")->getValue();
  status = 0;
  map4origAux.clear();
  rel = getRelaxation(env, p, status, map4origAux);
  for (std::map<std::pair<int, int>, int>::iterator itr = map4origAux.begin();
       itr != map4origAux.end(); ++itr) {
    aptr = (AuxVarsPtr) new AuxVars();
    aptr->x1 = itr->first.first;
    aptr->x2 = itr->first.second;
    aptr->y = itr->second;
    aptr->productType = 'v';
    aptr->scale = 1.0;
    aptr->isScaled = false;
    auxVars.push_back(aptr);
  }

  addTangentsForSqVars(rel, auxVars);
  map4origAux.clear();
  if (status == 1) {
    std::cout << "Problem identified as infeasible" << std::endl;
    goto CLEANUP;
  }
  for (int i = 0; i <= rounds; ++i) {
    if (is_feas) {
      break;
    }
    std::cout << "Iteration : " << i << std::endl;
    rel = solveRelaxation(env, p, rel, efac, is_feas, status, auxVars,
                          i == rounds);
  }

CLEANUP:
  for (AuxVarVector::iterator it = auxVars.begin(); it != auxVars.end(); ++it) {
    delete (*it);
  }
  auxVars.clear();

  for (HandlerVector::iterator it = handlers.begin(); it != handlers.end();
       ++it) {
    delete (*it);
  }

  if (efac) {
    delete efac;
  }

  if (iface) {
    delete iface;
    iface = 0;
  }

  if (pres) {
    delete pres;
  }

  if (pres2) {
    delete pres2;
  }

  if (p) {
    delete p;
  }

  if (inst) {
    delete inst;
    inst = 0;
  }

  if (rel) {
    delete rel;
  }

  if (env) {
    delete env;
  }

  if (orig_v) {
    delete orig_v;
  }

  if (dualVecCons) {
    delete[] dualVecCons;
  }

  if (dualVecVars) {
    delete[] dualVecVars;
  }

  return 0;
}

// Made changes
