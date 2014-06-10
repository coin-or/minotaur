//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file relax.cpp
 * \brief Apply quadratic cuts to a relaxation of a non-convex QP
 * \author Hongbo Dong, Washington State University
 */

#include <cmath> // for INFINITY
#include <iomanip>
#include <iostream>
#include <fstream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "Function.h"
#include "IpoptEngine.h"
#include "Option.h"
#include "LinearFunction.h"
#include "Problem.h"
#include "QuadraticFunction.h"
#include "Solution.h"
#include "Timer.h"

using namespace Minotaur;


void addCut()
{
}


ProblemPtr relaxProblem(EnvPtr env, ProblemPtr p)
{
  ProblemPtr r = (ProblemPtr) new Problem();
  return r;
}


EngineStatus solveRelax(EnvPtr env, ProblemPtr p, ConstSolutionPtr &sol)
{
  EnginePtr e = (FilterSQPEnginePtr) new FilterSQPEngine(env);
  EngineStatus status;

  e->load(p);
  status = e->solve();
  sol = e->getSolution();
  return status;
}


ProblemPtr readProblem(EnvPtr env)
{
  ProblemPtr p = (ProblemPtr) new Problem();
  std::ifstream fs;
  std::string fname = env->getOptions()->findString("problem_file")->getValue();
  int n = 0;
  FunctionPtr f;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  VariablePtr v;
  ConstSolutionPtr sol;

  fs.open(fname.c_str(), std::ios::in);

  // read the problem from the file here
  // This is just an example

  // First the 'v' variable
  v = p->newVariable(-INFINITY, INFINITY, Continuous, "v");
  n = 3;
  for (int i=0; i<n; ++i) {
    p->newVariable(0.0, 1.0, Continuous);
  }

  // variable v has index 0, x1 has 1, x2 has 2, x3 has 3
  // Objective: min v
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(v, 1.0);
  f = (FunctionPtr) new Function(lf);
  p->newObjective(f, 0.0, Minimize);

  // add a constraint
  // v >= 3x1x2 + 5x1x3 + 7x2^2 + 8x1 - 2x3
  qf = (QuadraticFunctionPtr) new QuadraticFunction();
  qf->incTerm(p->getVariable(1), p->getVariable(2), 3.0);
  qf->incTerm(p->getVariable(1), p->getVariable(3), 5.0);
  qf->incTerm(p->getVariable(2), p->getVariable(2), 7.0);

  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(p->getVariable(1), 8.0);
  lf->addTerm(p->getVariable(3), -2.0);
  lf->addTerm(v, -1.0);

  f = (FunctionPtr) new Function(lf, qf);

  p->newConstraint(f, -INFINITY, 0.0);

  fs.close();

  p->setNativeDer();
  p->write(std::cout);
  return p;
}


int main(int argc, char **argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  int err = 0;
  ProblemPtr p, r;
  EngineStatus status;
  std::string me = "sdp-cuts: ";
  ConstSolutionPtr sol;

  env->startTimer(err);

  // read the command line options
  env->readOptions(argc, argv);

  p = readProblem(env);

  r = relaxProblem(env, p);

  status = solveRelax(env, p, sol);

  if (status == ProvenOptimal || status == ProvenLocalOptimal) {
    std::cout << me << "feasible solution found" << std::endl;
    sol->writePrimal(std::cout);
  } else {
    std::cout << me << "engine has a problem. status = " << status << std::endl;
  }


  while (false) {
    addCut();
    solveRelax(env, p, sol);
  }

  return 0;
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
