/*
 *     MINOTAUR -- It's only 1/2 bull
 *
 *     (C)opyright 2009-- The MINOTAUR Team
 * 
 *  This is a little driver program to build and test different
 *   relaxations for multilinear problems
 *
 *  Author: Jeff Linderoth
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <Instance.h>
#include <Modification.h>
#include <MultilinearFunction.h>
#include <OsiLPEngine.h>
#include <Types.h>

#include "MultilinearFormulation.h"

using namespace Minotaur;
using namespace std;

istream & operator>>(istream &stream, Instance &instance);



int
main(int argc, char *argv[]) 
{
  
  // Read the instance from a text file.
  InstancePtr instance = InstancePtr(new Instance());
  ifstream ifile(argv[1]);
  try { 
    ifile >> (*instance);
  }
  catch (...) {
    cerr << "Error in instance file: " << argv[1] << ".  Aborting" << endl;
    exit(1);
  }

 { 
    // Convex Hull
   MultilinearFormulationPtr convex_hull = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::CONVEX_HULL));
    
    OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine());
    lpE->load(convex_hull);
    lpE->solve();

    cout << "Convex Hull: ";
    cout << " , " << lpE->getStatus();
    cout << " , " << lpE->getSolutionValue() << endl;

    vector<VariablePtr> xvars;
    for(ConstVariableIterator it = convex_hull->varsBegin(); it != convex_hull->varsEnd();
	++it) {
      //!!! Sweet moses, this is bad, but we just do it to get something done now.

      VariablePtr v = boost::const_pointer_cast<Variable>(*it);     
      xvars.push_back(v);
    }
    VarBoundMod m1(xvars[0], Lower, 0.0);
    VarBoundMod m2(xvars[0], Upper, 0.0);

    m1.applyToEngine(lpE);
    m2.applyToEngine(lpE);
    lpE->solve();


    cout << "After solve, Convex Hull: ";
    cout << " , " << lpE->getStatus();
    cout << " , " << lpE->getSolutionValue() << endl;

    m1.undoToEngine(lpE);
    m2.undoToEngine(lpE);
    lpE->solve();

    cout << "After solve 2, Convex Hull: ";
    cout << " , " << lpE->getStatus();
    cout << " , " << lpE->getSolutionValue() << endl;

  }

  {
    // Row by Row
    MultilinearFormulationPtr row_by_row = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::ROW_BY_ROW));
    OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine());
    lpE->load(row_by_row);
    lpE->solve();

    cout << "Row by Row: ";
    cout << " , " << lpE->getStatus();
    cout << " , " << lpE->getSolutionValue() << endl;
  }

  {
    // Term by Term
    MultilinearFormulationPtr term_by_term = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::TERM_BY_TERM));
    OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine());
    lpE->load(term_by_term);
    lpE->solve();

    cout << "Term by Term: ";
    cout << " , " << lpE->getStatus();
    cout << " , " << lpE->getSolutionValue() << endl;
  }

  {
    // McCormick
    MultilinearFormulationPtr mcc = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::MCCORMICK));
    OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine());
    lpE->load(mcc);
    lpE->solve();

    cout << "Mccormick: ";
    cout << " , " << lpE->getStatus();
    cout << " , " << lpE->getSolutionValue() << endl;
  }
  

  return(0);
}



istream &
operator>>(istream &stream, Instance &instance)
{
  string key;
  int nv;
  std::vector<VariablePtr> vars;

  stream >> nv;
  for(int i = 0; i < nv; i++) {
    double lb, ub;
    stream >> lb >> ub;
    vars.push_back(instance.newVariable(lb, ub));
  }

  LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
  int nt;
  stream >> nt;
  for(int i = 0; i < nt; i++) {
    UInt ix;
    Double v;
    stream >> ix >> v;
    lf->addTerm(vars[ix],v);
  }
  instance.newObjective(lf, 0.0, Minimize);

  int nlc;
  stream >> nlc;
  //XXX Add linear constraints
  assert(nlc == 0);
  for(int i = 0; i < nlc; i++) {
  }

  int nmlc;
  stream >> nmlc;
  for(int i = 0; i < nmlc; i++) {
    MultilinearFunctionPtr mlf = MultilinearFunctionPtr(new MultilinearFunction());
    int nmt, nqt, nlt;
    stream >> nmt >> nqt >> nlt;
    for(int j = 0; j < nmt; j++) {
      Double v;
      UInt nt;
      set<ConstVariablePtr> t;
      stream >> v >> nt;
      for(int k = 0; k < nt; k++) {
	int ix;
	stream >> ix;
	t.insert(vars[ix]);
      }
      multilinearTerm term(v,t);
      mlf->addTerm(term);
    }
    QuadraticFunctionPtr qf = QuadraticFunctionPtr(new QuadraticFunction());
    for(int j = 0; j < nqt; j++) {
      double v;
      UInt ix1, ix2;
      stream >> v >> ix1 >> ix2;
      VariablePair vp(vars[ix1],vars[ix2]);
      qf->addTerm(vp, v);
    }
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
    for(int j = 0; j < nlt; j++) {
      double v;
      UInt ix;
      stream >> v >> ix;
      lf->addTerm(vars[ix],v);
    }
    char senx;
    double rhs;
    stream >> senx >> rhs;

    FunctionPtr f = FunctionPtr(new Function(lf, qf, mlf));
    if (senx == 'E') {
      ConstraintPtr c = ConstraintPtr(new Constraint(f, rhs, rhs));
      instance.addConstraint(c);
    }
    else if (senx == 'L') {
      ConstraintPtr c = ConstraintPtr(new Constraint(f, -INFINITY, rhs));
      instance.addConstraint(c);
    }
    else if (senx == 'G') {
      ConstraintPtr c = ConstraintPtr(new Constraint(f, rhs, INFINITY));
      instance.addConstraint(c);
    }
    else {
      assert(0);
    }
    
  }

  return stream;
}
