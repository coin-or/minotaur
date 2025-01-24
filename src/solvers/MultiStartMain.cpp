//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file MultiStartMain.cpp
 * \brief The main function for solving instances by a multi-start heuristic
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"
#include "MultiStart.h"

using namespace Minotaur;


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  MultiStart ms(env);
  int err = 0;
  std::string dname, fname;
  ProblemPtr p = 0;
 
  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  if (0!=ms.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if (""==fname) {
    ms.showHelp();
    goto CLEANUP;
  }

  p = ms.readProblem(fname, dname, "mmultistart", err);
  if (err) {
    goto CLEANUP;
  }

  err = ms.solve(p);
  if (err) {
    goto CLEANUP;
  }

CLEANUP:
  if (p) {
    delete p;
  }
  delete env;

  return 0;
}


