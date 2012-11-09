//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
// 

/** This is example driver code to demonstrate nonseparable quadratic vub 
    ideas using minotaur
*/

#include <cstdlib>
#include <iostream>

// Put config stuff (by hand) here
#define MINOTAUR_RUSAGE


#include "AMPLInterface.h"
#include "Eigen.h"
#include "Problem.h"

using namespace Minotaur;
using namespace MINOTAUR_AMPL;
using namespace std;

int
main(int argc, char *argv[])
{
  if (argc !=2) {
    cerr << "Usage: nseqp <nlfile>" << endl;
    exit(1);
  }
  ProblemPtr original_inst; 
  AMPLInterfacePtr iface;
  iface = (AMPLInterfacePtr) new AMPLInterface();
  original_inst = iface->readInstance(argv[1]);
  original_inst->write(cout);

  ProblemPtr p; // This is the one you work with
  EigenCalculator eigcalc;

  for(ConstraintConstIterator it = original_inst->consBegin(); 
      it != original_inst->consEnd(); ++it) {
    ConstConstraintPtr c = *it;
    cout << "Constraint: " << c->getId() << endl;
    if (c->getFunctionType() == Quadratic) {      
      const QuadraticFunctionPtr q = c->getQuadraticFunction();
      EigenPtr eigs = eigcalc.findVectors(q);
      eigs->write(cout);
#if 0
      for(EigenPairConstIterator it2 = eigs->begin(); 
	  it2 != eigs->end(); ++it2) {
	(*it2)->write(cout);
      }
#endif
      
    }
  }

}

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
 
