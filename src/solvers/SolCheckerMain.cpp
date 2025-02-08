//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file SolcheckerMain.cpp
 * \brief The main function for checking a solution for feasibility
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"
#include "SolChecker.h"

using namespace Minotaur;


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  SolChecker ch(env);
  int err = 0;
  std::string dname, fname;
  ProblemPtr p = 0;
 
  ch.doSetup();

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  if (0!=ch.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if (""==fname) {
    ch.showHelp();
    goto CLEANUP;
  }

  p = ch.readProblem(fname, dname, "mcheck", err);
  if (err) {
    goto CLEANUP;
  }

  err = ch.solve(p);
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
