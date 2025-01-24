//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file QGParMain.cpp
 * \brief The main function for solving instances by Quesada-Grossmann (QG)
 * algorithm.
 * \author Meenarli Sharma and Prashant Palkar, IIT Bombay
 */

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"
#include "QGPar.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  int err = 0;
  QGPar qgpar(env);
  ProblemPtr p = 0;
  std::string fname, dname;
 
  qgpar.doSetup();

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  if (0!=qgpar.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if (""==fname) {
    qgpar.showHelp();
    goto CLEANUP;
  }

  p = qgpar.readProblem(fname, dname, "qgpar", err);
  if (err) {
    goto CLEANUP;
  }

  err = qgpar.solve(p);
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


