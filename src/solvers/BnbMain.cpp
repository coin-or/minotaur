//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file BnbMain.cpp
 * \brief The main function for solving instances by Branch-and-Bound.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include "MinotaurConfig.h"
#include "Bnb.h"
#include "Problem.h"
#include "Types.h"

using namespace Minotaur;


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  Bnb bnb(env);
  int err = 0;
  std::string dname, fname;
  ProblemPtr p = 0;
 
  bnb.doSetup();

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  if (0!=bnb.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if (""==fname) {
    bnb.showHelp();
    goto CLEANUP;
  }

  p = bnb.readProblem(fname, dname, "mbnb", err);
  if (err) {
    goto CLEANUP;
  }

  err = bnb.solve(p);
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


