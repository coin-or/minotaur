// A simple file that solves a nonlinear relaxation of a given problem using
// FilterSQP.

#include <iostream>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "AMPLInterface.h"
#include "Problem.h"
#include "Timer.h"
#include "MinotaurDeconfig.h"
#include "NLPRelaxation.h"
#include "Variable.h"
#include "AMPLJacobian.h"
#include "AMPLHessian.h"
#include "Types.h"
#include "Option.h"

using namespace Minotaur;

int main(int argc, char** argv) 
{
  // Ampl interface, jacobian and hessian.
  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();  
  JacobianPtr jPtr;            //! Jacobian read from AMPL
  HessianOfLagPtr hPtr;        //! Hessian read from AMPL
  
  // environment, timers and options:
  EnvPtr env = (EnvPtr) new Environment();
  OptionDBPtr options;

  // problem to be solved.
  ProblemPtr minlp;
  
  // solver pointers, including status.
  FilterSQPEngine e(env);
  EngineStatus status;
  
  // Default options
  env->getOptions()->findBool("presolve")->setValue(false);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(false);

  // parse options
  env->readOptions(argc, argv);
  options = env->getOptions();
  options->findString("interface_type")->setValue("AMPL");

  // read minlp from AMPL.
  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) new MINOTAUR_AMPL::AMPLInterface(env);
  
  minlp = iface->readInstance(options->findString("problem_file")->getValue());
  minlp->calculateSize();
  minlp->prepareForSolve();
  
  minlp->setJacobian(jPtr);
  minlp->setHessian(hPtr);
  minlp->setNativeDer();

  minlp->calculateSize();
  minlp->prepareForSolve();
  minlp->setNativeDer();

  // load problem.
  e.load(minlp);

  // Solve problem.
  status = e.solve();
  
  // Solution objective value
  double obj = e.getSolutionValue();

  std::cout << "Relaxation objective value = " << obj << std::endl; 
  
  return 0;
}

