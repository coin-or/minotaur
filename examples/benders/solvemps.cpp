// Bare bones example of using COIN-OR OSI

#include <iostream>
#include "OsiClpSolverInterface.hpp"

using namespace std;
int main(int argc, char **argv)
{
  
  OsiSolverInterface *si = new OsiClpSolverInterface;
  
  si->setObjSense(1); si->messageHandler()->setLogLevel(0); 
  
  si->readMps(argv[1]);
  
  si->initialSolve();

  if ( si->isProvenOptimal() ) { 
    cout << si->getObjValue() << std::endl;
    int n = si->getNumCols();
    const double *solution;
    solution = si->getColSolution();
    for(int i=0;i<n;i++){
      cout << "x" << i << " " << solution[i] << endl;
    }
  } else {
    std::cout << "Didn't find optimal solution." << std::endl;
  }

  return 0;
}
