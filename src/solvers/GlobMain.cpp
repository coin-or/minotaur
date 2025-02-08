//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file GlobMain.cpp
 * \brief The main function for solving instances with glob
 * \author Mustafa Vora, IIT Bombay
 */

#include "Environment.h"
#include "Glob.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"

#include <iostream>

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  ProblemPtr inst = 0; // instance that needs to be solved.
  std::string dname, fname;
  int err = 0;
  Glob glob(env);

  glob.doSetup();

  // read user-specified options
  env->readOptions(argc, argv);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);

  if(0 != glob.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if("" == fname) {
    glob.showHelp();
    goto CLEANUP;
  }

  inst = glob.readProblem(fname, dname, "mglob", err);
  if(err) {
    goto CLEANUP;
  }

  if(!(inst->isLinear() || inst->isQP() || inst->isQuadratic())) {
    env->getLogger()->msgStream(LogError)
        << "mglob error : "
        << "mglob only solves Mixed Integer Quadratically Constrained Quadratic"
        << " problems (MIQCQP). " << fname << " is not an MIQCQP." << std::endl
        << "Problem not solved" << std::endl;
    goto CLEANUP;
  }

  glob.solve(inst);

CLEANUP:
  if(inst) {
    delete inst;
  }
  if(env) {
    delete env;
  }

  return 0;
}
