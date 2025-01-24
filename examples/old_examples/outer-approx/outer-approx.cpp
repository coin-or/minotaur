//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2011 The MINOTAUR Team
// 

/*! \brief Example implementing outer approximation with MINOTAUR
 *
 * Example driver code implementing outer approximation for MINLPs.
 * Illustrates the use of the following features in MINOTAUR:
 *   -# Setting options & providing some user control
 *   -# Reading a problem from AMPL with MINOTAUR
 *   -# Setting solvers (or engines) & solving an AMPL model
 *   -# Fixing variables & querying solvers.
 *   -# Defining a new problem (MILP master) & updating it.
 *   -# Solving an MILP with MINOTAUR.
 *
 * Subroutines are implemented in UtilOA.cpp, see UtilOA.h.
 * UtilOA.h also contains the remaining header-files needed here.
 * 
 * \author Sven Leyffer, Argonne National Laboratory, 2011.
 */

//! The following header file contains all other required headers
#include "utilOA.h"
#include "Constraint.h"
#include "Function.h"
#include "Option.h"

//! main routine implementing outer approximation
int main(int argc, char** argv)
{
  //! Declaration of Variables ============================================
  //! interface to AMPL (NULL):
  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();  
  MINOTAUR_AMPL::JacobianPtr jPtr;            //! Jacobian read from AMPL
  MINOTAUR_AMPL::HessianOfLagPtr hPtr;        //! Hessian read from AMPL

  //! environment, timers, options:
  EnvPtr env = (EnvPtr) new Environment();
  TimerFactory *tFactory = new TimerFactory();     
  Timer *timer=tFactory->getTimer();
  OptionDBPtr options;                        //! AMPL and MINOTAUR options

  //! problem pointers (MINLP, NLP, MILP):
  ProblemPtr minlp;                           //! MINLP instance to be solved
  ProblemPtr milp;                            //! MILP master problem

  //! solver pointers, including status
  FilterSQPEngine e(env);                     //! NLP engine: FilterSQP
  EngineStatus status;

  //! pointers to manipulate variables & constraints
  VariablePtr v, objVar;                      //! variable pointer (objective)
  ObjectivePtr objFun;                        //! remember objective function pt.

  //! local variables
  double *x, *xsoln;
  double *lobnd, *upbnd;
  double objfLo = -INFINITY, objfUp = INFINITY, objfNLP, objfMIP;
  double tol = 1E-2;
  int    feasibleNLP = 0, iterOA = 1, n;
  // ======================================================================

  //! start the timer
  timer->start();

  //! make sure solver is used correctly
  if (argc < 2) {
    usage();
    return -1;
  }

  //! add AMPL options & flags to environment: flags (options without values)
  options = env->getOptions();
  add_ampl_flags(options);
  env->readOptions(argc, argv);                  //! parse options
  if (!checkUserOK(options,env)) goto CLEANUP;   //! check if user needs help

  //! read MINLP from AMPL & create Hessian/Jacobian for NLP solves
  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) new MINOTAUR_AMPL::AMPLInterface(env);
  minlp = iface->readInstance(options->findString("problem_file")->getValue(),false);
  jPtr  = (MINOTAUR_AMPL::JacobianPtr)      new MINOTAUR_AMPL::Jacobian(iface);
  hPtr  = (MINOTAUR_AMPL::HessianOfLagPtr)  new MINOTAUR_AMPL::HessianOfLag(iface);
  minlp->setJacobian(jPtr);
  minlp->setHessian(hPtr);
    
  //! Display number of constraints and variables, and MINLP problem
  minlp->calculateSize();
  n = minlp->getNumVars();
  std::cout << "No. of vars, cons = " << minlp->getNumVars() 
            << minlp->getNumCons() << std::endl;
  std::cout << std::endl << "The MINLP problem is: " << std::endl;
  minlp->write(std::cout);

  //! load the MINLP into the NLP solver (FilterSQP)
  e.load(minlp);

  //! get initial point & save original bounds
  x     = new double[n];
  xsoln = new double[n];
  lobnd = new double[n];
  upbnd = new double[n];
  std::copy(iface->getInitialPoint(),iface->getInitialPoint()+n,x);
  for (VariableConstIterator i=minlp->varsBegin(); i!=minlp->varsEnd(); ++i) {
    v = *i;
    lobnd[v->getId()] = v->getLb();
    upbnd[v->getId()] = v->getUb();
  }

  //! initialize the MILP master problem by copying variables & linear c/s
  milp   = (ProblemPtr) new Problem();
  objFun = minlp->getObjective();
  objVar = VariablePtr();
  initMaster(minlp, milp, objVar, objFun, x);

  while ((objfLo <= objfUp)&&(iterOA<4)){

    std::cout << "Iteration " << iterOA << std::endl 
              << "===============" << std::endl << std::endl;

    //! set-up and solve NLP(y) with fixed integers
    solveNLP(minlp, e, x, objfNLP, feasibleNLP, n);
    std::cout << "Solved NLP " << iterOA << "  objective = " << objfNLP 
              << "  NLPfeasible = " << feasibleNLP << std::endl;
    if (feasibleNLP && (objfNLP-tol < objfUp)) {
      objfUp = objfNLP - tol;
      std::copy(x,x+n,xsoln);
    }
        
    //! update MILP master problem by adding outer approximations
    updateMaster(minlp, milp, objVar, objFun, objfUp, x, n);

    //! solve MILP master problem
    solveMaster(env, milp, x, &objfMIP, n);
    objfLo = objfMIP;

    iterOA = iterOA + 1;

  } // end while (objfLo <= objfUp) 

  //! output final result & timing
  std::cout << std::endl << "END outer-approximation: f(x) = " << objfUp 
            << "   time used = " << timer->query() << std::endl;

CLEANUP:
  //! free storage 
   delete timer;
   delete tFactory;
   if (minlp) {
     minlp->clear();
     delete [] x;
     delete [] xsoln;
     delete [] lobnd;
     delete [] upbnd;
   }
   if (milp) {
     milp->clear();
   }
  return 0;
} // end outer approximation main


