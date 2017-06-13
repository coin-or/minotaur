#include <iostream>

#include "Environment.h"
#include "BqpdEngine.h"
#include "Problem.h"
#include "AMPLInterface.h"

using namespace Minotaur;

// Read a qp from a .nl file and solve it using filter.
int main()
{
  EnvPtr env = (EnvPtr) new Environment();
  MINOTAUR_AMPL::AMPLInterface iface(env);
  BqpdEngine e(env);
  ProblemPtr p; // the problem to be solved.
  EngineStatus status;

  // read problem using ampl interface.
  p = iface.readInstance("hs021.nl");
  
  // Find the number of constraints and variables:
  p->calculateSize();
  std::cout << "problem has " << p->getNumVars() << " variables." << std::endl;
  std::cout << "problem has " << p->getNumCons() << " constraints." 
            << std::endl << std::endl;

  // display the problem at standard output.
  std::cout << "The problem is: " << std::endl;
  p->write(std::cout);

  // solve it.
  p->setNativeDer();
  p->prepareForSolve();
  e.load(p);
  status = e.solve();

  // status, value and filter-sqp statistics.
  e.writeStats(std::cout);
  std::cout << "solution status code = " << status << std::endl;
  std::cout << "solution status = " << e.getStatusString() << std::endl;
  std::cout << "solution value = " << e.getSolutionValue() << std::endl;

  p->clear();
  return 0;
}

