//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file BnbDistParMain.cpp
 * \brief The main function for solving instances by distributed parallel Branch-and-Bound
 * (Bnb) algorithm.
 * \author Meenarli Sharma and Prashant Palkar, IIT Bombay
 */
#include <mpi.h>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"
#include "BnbDistPar.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  int nprocs, proc_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  EnvPtr env = (EnvPtr) new Environment();
  int err = 0;
  BnbDistPar bnbdistpar(env);
  ProblemPtr p = 0;
  std::string fname, dname;
 
  bnbdistpar.doSetup();
  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  if (0!=bnbdistpar.showInfo()) {
    goto CLEANUP;
  }

  dname = env->getOptions()->findString("debug_sol")->getValue();
  fname = env->getOptions()->findString("problem_file")->getValue();
  if (""==fname) {
    bnbdistpar.showHelp();
    goto CLEANUP;
  }

  p = bnbdistpar.readProblem(fname, dname, "bnbdistpar", err);
  if (err) {
    goto CLEANUP;
  }

  err = bnbdistpar.solve(p);
  if (err) {
    goto CLEANUP;
  }

CLEANUP:
  MPI_Finalize();
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
