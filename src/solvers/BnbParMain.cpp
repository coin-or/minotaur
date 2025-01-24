//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file BnbParMain.cpp
 * \brief The main function for solving instances by parallel Branch-and-Bound (Bnb)
 * algorithm.
 * \author Meenarli Sharma and Prashant Palkar, IIT Bombay
 */

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"
#include "BnbPar.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  int err = 0;
  BnbPar bnbpar(env);
  ProblemPtr p = 0;
  std::string fname, dname;
 
  bnbpar.doSetup();

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  if (0!=bnbpar.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if (""==fname) {
    bnbpar.showHelp();
    goto CLEANUP;
  }

  p = bnbpar.readProblem(fname, dname, "bnbpar", err);
  if (err) {
    goto CLEANUP;
  }

  err = bnbpar.solve(p);
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


