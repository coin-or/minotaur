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

#include <cmath>
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
#include "Types.h"

using namespace Minotaur;

struct AuxVars {
  int x1;            // index of the first variable
  int x2;            // index of the second variable
  char productType;  // v - both first and second variables are "Variables"
                     // c - both first and second variables are "Constraints"
                     // m - first is "Variable", second is "Constriant"
  int y;             // index of the auxiliary variable
};

typedef AuxVars* AuxVarsPtr;
typedef std::vector<AuxVarsPtr> AuxVarVector;

void usage() {
  std::cout << "./scuts {name-of-.nl-file} "
            << "--{any-other-minotaur-option}\n";
  return;
}

int findY(AuxVarVector aux, int x1, int x2, char productType) {
  AuxVarsPtr aptr;
  for (AuxVarVector::iterator it = aux.begin(); it != aux.end(); ++it) {
    aptr = (*it);
    if (aptr->productType == productType) {
      if (aptr->x1 == x1 && aptr->x2 == x2) {
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

  delete lhand;
  delete qhand;

  return rel;
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, VariablePtr x, double lb,
                  double ub) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;

  // Secant
  lf->incTerm(y, 1.0);
  lf->incTerm(x, -(lb + ub));
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -INFINITY, -lb * ub);

  // Tangent at lb
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x, -2 * lb);
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -lb * lb, INFINITY);

  // Tangent at ub
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x, -2 * ub);
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -ub * ub, INFINITY);
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, VariablePtr x1,
                  VariablePtr x2, double l1, double u1, double l2, double u2) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;

  // Secant at (l1, u2)
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, -u2);
  lf->incTerm(x2, -l1);
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -INFINITY, -l1 * u2);

  // Secant at (u1, l2)
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, -l2);
  lf->incTerm(x2, -u1);
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -INFINITY, -u1 * l2);

  // Tangent at (l1, l2)
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, -l2);
  lf->incTerm(x2, -l1);
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -l1 * l2, INFINITY);

  // Tangent at (u1, u2)
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(y, 1.0);
  lf->incTerm(x1, -u2);
  lf->incTerm(x2, -u1);
  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, -u1 * u2, INFINITY);
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, LinearFunctionPtr lf,
                  double d, double lb, double ub) {
  LinearFunctionPtr lf1 = lf->clone();
  FunctionPtr f;

  // Secant
  lf1->multiply(-(lb + ub));
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, -INFINITY, (lb + ub) * d - lb * ub);

  // Tangent at lb
  lf1 = lf->clone();
  lf1->multiply(-2 * lb);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, 2 * lb * d - lb * lb, INFINITY);

  // Tangent at ub
  lf1 = lf->clone();
  lf1->multiply(-2 * ub);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, 2 * ub * d - ub * ub, INFINITY);
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, LinearFunctionPtr lf1,
                  double d1, LinearFunctionPtr lf2, double d2, double l1,
                  double u1, double l2, double u2) {
  LinearFunctionPtr lfnew, lf1clone, lf2clone;
  FunctionPtr f;

  // Secant at (l1, u2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(-u2);
  lf2clone->multiply(-l1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  rel->newConstraint(f, -INFINITY, u2 * d1 + l1 * d2 - l1 * u2);
  delete lf1clone;
  delete lf2clone;

  // Secant at (u1, l2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(-l2);
  lf2clone->multiply(-u1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  rel->newConstraint(f, -INFINITY, l2 * d1 + u1 * d2 - u1 * l2);
  delete lf1clone;
  delete lf2clone;

  // Tangent at (l1, l2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(-l2);
  lf2clone->multiply(-l1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  rel->newConstraint(f, l2 * d1 + l1 * d2 - l1 * l2, INFINITY);
  delete lf1clone;
  delete lf2clone;

  // Tangent at (u1, u2)
  lf1clone = lf1->clone();
  lf2clone = lf2->clone();
  lf1clone->multiply(-u2);
  lf2clone->multiply(-u1);
  lfnew = lf1clone->copyAdd(lf2clone);
  lfnew->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lfnew);
  rel->newConstraint(f, u2 * d1 + u1 * d2 - u1 * u2, INFINITY);
  delete lf1clone;
  delete lf2clone;
}

void addMcCormick(RelaxationPtr rel, VariablePtr y, VariablePtr x,
                  LinearFunctionPtr lf, double d, double l1, double u1,
                  double l2, double u2) {
  LinearFunctionPtr lf1 = lf->clone();
  FunctionPtr f;

  // Secant at (l1, u2)
  lf1->multiply(-l1);
  lf1->incTerm(x, -u2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, -INFINITY, l1 * d - l1 * u2);

  // Secant at (u1, l2)
  lf1 = lf->clone();
  lf1->multiply(-u1);
  lf1->incTerm(x, -l2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, -INFINITY, u1 * d - u1 * l2);

  // Tangent at (l1, l2)
  lf1 = lf->clone();
  lf1->multiply(-l1);
  lf1->incTerm(x, -l2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, l1 * d - l1 * l2, INFINITY);

  // Tangent at (u1, u2)
  lf1 = lf->clone();
  lf1->multiply(-u1);
  lf1->incTerm(x, -u2);
  lf1->incTerm(y, 1.0);
  f = (FunctionPtr) new Function(lf1);
  rel->newConstraint(f, u1 * d - u1 * u2, INFINITY);
}

int getNewVar(RelaxationPtr rel, SimplexQuadCutGenPtr cutgen, int x1, int x2,
              char productType) {
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
        v = rel->newVariable(lb, ub, Continuous);
        addMcCormick(rel, v, v1, l1, u1);
      } else {
        v1 = rel->getVariable(x1);
        v2 = rel->getVariable(x2);
        l1 = v1->getLb();
        u1 = v1->getUb();
        l2 = v2->getLb();
        u2 = v2->getUb();
        BoundsOnProduct(true, l1, u1, l2, u2, lb, ub);
        v = rel->newVariable(lb, ub, Continuous);
        addMcCormick(rel, v, v1, v2, l1, u1, l2, u2);
      }
      break;
    case 'c':
      if (x1 == x2) {
        l1 = cutgen->getSlackLb(x1);
        u1 = cutgen->getSlackUb(x1);
        BoundsOnSquare(l1, u1, lb, ub);
        v = rel->newVariable(lb, ub, Continuous);
        lf1 = (LinearFunctionPtr) new LinearFunction();
        d1 = 0.0;
        cutgen->getAffineFnForSlack(rel, x1, lf1, d1);
        addMcCormick(rel, v, lf1, d1, l1, u1);
        delete lf1;
      } else {
        l1 = cutgen->getSlackLb(x1);
        u1 = cutgen->getSlackUb(x1);
        l2 = cutgen->getSlackLb(x2);
        u2 = cutgen->getSlackUb(x2);
        BoundsOnProduct(true, l1, u1, l2, u2, lb, ub);
        v = rel->newVariable(lb, ub, Continuous);
        lf1 = (LinearFunctionPtr) new LinearFunction();
        d1 = 0.0;
        cutgen->getAffineFnForSlack(rel, x1, lf1, d1);
        lf2 = (LinearFunctionPtr) new LinearFunction();
        d2 = 0.0;
        cutgen->getAffineFnForSlack(rel, x2, lf2, d2);
        addMcCormick(rel, v, lf1, d1, lf2, d2, l1, u1, l2, u2);
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
      v = rel->newVariable(lb, ub, Continuous);
      lf2 = (LinearFunctionPtr) new LinearFunction();
      d2 = 0.0;
      cutgen->getAffineFnForSlack(rel, x2, lf2, d2);
      addMcCormick(rel, v, v1, lf2, d2, l1, u1, l2, u2);
      delete lf2;
  }
  return v->getIndex();
}

void addTerm(std::map<int, double>& t, int v, double coef) {
  if (fabs(coef) > 1e-6) {
    std::map<int, double>::iterator it = t.find(v);
    if (it == t.end()) {
      t.insert(std::make_pair(v, coef));
    } else {
      double nv = it->second + coef;
      if (fabs(nv) < 1e-6) {
        t.erase(v);
      } else {
        it->second = nv;
      }
    }
  }
}

void addCutToRel(RelaxationPtr rel, SimplexQuadCutGenPtr cutgen,
                 std::map<int, double> cutCoefo, std::map<int, double> cutCoefs,
                 double cutConst, double clb, double cub) {
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  LinearFunctionPtr lfs;
  FunctionPtr f;
  double ds;
  std::map<int, double>::iterator it;

  for (it = cutCoefo.begin(); it != cutCoefo.end(); ++it) {
    lf->incTerm(rel->getVariable(it->first), it->second);
  }

  for (it = cutCoefs.begin(); it != cutCoefs.end(); ++it) {
    lfs = (LinearFunctionPtr) new LinearFunction();
    ds = 0.0;
    cutgen->getAffineFnForSlack(rel, it->first, lfs, ds);
    lfs->multiply(it->second);
    lf->add(lfs);
    cutConst += it->second * ds;
    delete lfs;
  }

  f = (FunctionPtr) new Function(lf);
  rel->newConstraint(f, clb - cutConst, cub - cutConst);
}

void updateRel(EnvPtr env, RelaxationPtr rel, SimplexQuadCutGenPtr cutgen,
               AuxVarVector& auxVars, const double* x, LPEnginePtr lpe,
               ConstraintPtr c) {
  QuadTerm oxo, oxs, sxs;
  std::map<int, double> cutCoefo, cutCoefs;
  double cutConst = 0.0;
  double etol = 1e-6;
  int y;
  AuxVarsPtr aptr;

  cutgen->preprocessSimplexTab();
  cutgen->getQuadratic(c, x, rel, oxo, oxs, sxs, cutCoefo, cutCoefs, cutConst);

  for (QuadTerm::iterator it = oxo.begin(); it != oxo.end(); ++it) {
    if (fabs(it->second) < etol) {
      continue;
    }
    y = findY(auxVars, it->first.first, it->first.second, 'v');
    if (y == -1) {
      y = getNewVar(rel, cutgen, it->first.first, it->first.second, 'v');
      aptr = (AuxVarsPtr) new AuxVars();
      aptr->x1 = it->first.first;
      aptr->x2 = it->first.second;
      aptr->productType = 'v';
      aptr->y = y;
      auxVars.push_back(aptr);
    }
    addTerm(cutCoefo, y, it->second);
  }

  for (QuadTerm::iterator it = oxs.begin(); it != oxs.end(); ++it) {
    if (fabs(it->second) < etol) {
      continue;
    }
    y = findY(auxVars, it->first.first, it->first.second, 'm');
    if (y == -1) {
      y = getNewVar(rel, cutgen, it->first.first, it->first.second, 'm');
      aptr = (AuxVarsPtr) new AuxVars();
      aptr->x1 = it->first.first;
      aptr->x2 = it->first.second;
      aptr->productType = 'm';
      aptr->y = y;
      auxVars.push_back(aptr);
    }
    addTerm(cutCoefo, y, it->second);
  }

  for (QuadTerm::iterator it = sxs.begin(); it != sxs.end(); ++it) {
    if (fabs(it->second) < etol) {
      continue;
    }
    y = findY(auxVars, it->first.first, it->first.second, 'c');
    if (y == -1) {
      y = getNewVar(rel, cutgen, it->first.first, it->first.second, 'c');
      aptr = (AuxVarsPtr) new AuxVars();
      aptr->x1 = it->first.first;
      aptr->x2 = it->first.second;
      aptr->productType = 'c';
      aptr->y = y;
      auxVars.push_back(aptr);
    }
    addTerm(cutCoefo, y, it->second);
  }

  addCutToRel(rel, cutgen, cutCoefo, cutCoefs, cutConst, c->getLb(),
              c->getUb());
  oxo.clear();
  oxs.clear();
  sxs.clear();
  cutCoefo.clear();
  cutCoefs.clear();
}

bool isFeasible(EnvPtr env, ProblemPtr p, ConstSolutionPtr sol,
                RelaxationPtr rel, AuxVarVector& auxVars, LPEnginePtr lpe) {
  ConstraintPtr c;
  double act, clb, cub;
  const double* x = sol->getPrimal();
  int error = 0;
  bool is_feas = true;
  double vio;
  bool ubinf, lbinf;
  double at = 1e-6;
  double rt = 1e-4;

  SimplexQuadCutGenPtr cutgen =
      (SimplexQuadCutGenPtr) new SimplexQuadCutGen(env, p, lpe);

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
          updateRel(env, rel, cutgen, auxVars, x, lpe, c);
        }
      } else {
        std::cout << c->getName() << " Constraint not defined at this point."
                  << std::endl;
        is_feas = false;
      }
    }
  }

  return is_feas;
}

RelaxationPtr solveRelaxation(EnvPtr env, ProblemPtr p, RelaxationPtr rel,
                              EngineFactory* efac, bool& is_feas, int& status,
                              AuxVarVector& auxVars) {
  LPEnginePtr lpe = efac->getLPEngine();
  ConstSolutionPtr sol;
  RelaxationPtr newrel = rel->clone(env);

  /* TO DO : Instead of directly loading and solving the problem
   * we should get the dual information of the previous iteration
   * and then set extra dual variables to zero to warm start
   * dual simplex algorithm
   */
  lpe->load(rel);
  lpe->solve();
  sol = lpe->getSolution();

  std::cout << "Lower bound = " << sol->getObjValue() << std::endl;
  ;

  is_feas = isFeasible(env, p, sol, newrel, auxVars, lpe);
  if (is_feas) {
    std::cout << "Feasible Solution found" << std::endl;
    std::cout << "Upper bound = " << sol->getObjValue() << std::endl;
    return 0;
  }

  // delete sol;
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
  if (objType != Linear || objType != Constant) {
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
    auxVars.push_back(aptr);
  }
  map4origAux.clear();
  if (status == 1) {
    std::cout << "Problem identified as infeasible" << std::endl;
    goto CLEANUP;
  }
  for (int i = 0; i < rounds; ++i) {
    if (is_feas) {
      break;
    }
    rel = solveRelaxation(env, p, rel, efac, is_feas, status, auxVars);
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

  return 0;
}

// Made changes
