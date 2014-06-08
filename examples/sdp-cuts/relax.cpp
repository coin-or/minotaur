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

#include <iomanip>
#include <iostream>
#include <fstream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Function.h"
#include "Option.h"
#include "LinearFunction.h"
#include "Problem.h"
#include "QuadraticFunction.h"
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


void solveRelax()
{
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

  fs.open(fname.c_str(), std::ios::in);

  // read the problem from the file here
  // This is just an example
  // First the variables
  n = 3;
  for (int i=0; i<n; ++i) {
    p->newVariable(0.0, 1.0, Continuous);
  }

  // Objective: 3x_1x_2 + 5x_1x_3 + 7x_2^2 + 8x_1 - 2x_3
  qf = (QuadraticFunctionPtr) new QuadraticFunction();
  qf->addTerm(p->getVariable(0), p->getVariable(1), 3.0);
  qf->addTerm(p->getVariable(0), p->getVariable(2), 5.0);
  qf->addTerm(p->getVariable(1), p->getVariable(1), 7.0);

  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(p->getVariable(0), 8.0);
  lf->addTerm(p->getVariable(2), -2.0);

  f = (FunctionPtr) new Function(lf, qf);

  p->newObjective(f, 0.0, Minimize);

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

  env->startTimer(err);

  // read the command line options
  env->readOptions(argc, argv);

  p = readProblem(env);

  r = relaxProblem(env, p);

  solveRelax();

  while (false) {
    addCut();
    solveRelax();
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
