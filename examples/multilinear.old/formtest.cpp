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

static vector<double> randPointInBox(ConstInstancePtr instance);

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


  vector<int> lcid;
  vector<int> mlcid;
  for(ConstConstraintIterator it = instance->consBegin(); 
      it != instance->consEnd(); ++it) {
    FunctionType ft = (*it)->getFunctionType();
    if (ft == Multilinear) {
      mlcid.push_back((*it)->getId());
    }    
    else if (ft == Bilinear) {
      mlcid.push_back((*it)->getId());
    }
    else if (ft == Linear) {
      lcid.push_back((*it)->getId());
    }
  }
  

  // Convex Hull
  MultilinearFormulationPtr convex_hull = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::CONVEX_HULL));  
  OsiLPEnginePtr convex_hull_lp = OsiLPEnginePtr(new OsiLPEngine());
  convex_hull_lp->load(convex_hull);

  vector<VariablePtr> xvars_convex_hull;
  for(ConstVariableIterator it = convex_hull->varsBegin(); it != convex_hull->varsEnd();
      ++it) {
    //!!! Sweet moses, this is bad, but we just do it to get something done now.
    
    VariablePtr v = boost::const_pointer_cast<Variable>(*it);     
    xvars_convex_hull.push_back(v);
  }


  // Row by Row
  MultilinearFormulationPtr row_by_row = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::ROW_BY_ROW));
  OsiLPEnginePtr row_by_row_lp = OsiLPEnginePtr(new OsiLPEngine());
  row_by_row_lp->load(row_by_row);
  vector<VariablePtr> xvars_row_by_row;
  for(ConstVariableIterator it = row_by_row->varsBegin(); it != row_by_row->varsEnd(); ++it) {
    //!!! Sweet moses, this is bad, but we just do it to get something done now.
    
    VariablePtr v = boost::const_pointer_cast<Variable>(*it);     
    xvars_row_by_row.push_back(v);
  }

  // Term by Term
  MultilinearFormulationPtr term_by_term = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::TERM_BY_TERM));
  OsiLPEnginePtr term_by_term_lp = OsiLPEnginePtr(new OsiLPEngine());
  term_by_term_lp->load(term_by_term);
  vector<VariablePtr> xvars_term_by_term;
  for(ConstVariableIterator it = term_by_term->varsBegin(); it != term_by_term->varsEnd(); ++it) {    //!!! Sweet moses, this is bad, but we just do it to get something done now.    
    VariablePtr v = boost::const_pointer_cast<Variable>(*it);     
    xvars_term_by_term.push_back(v);
  }

  // McCormick
  MultilinearFormulationPtr mccormick = MultilinearFormulationPtr(new MultilinearFormulation(instance, MultilinearFormulation::MCCORMICK));
  OsiLPEnginePtr mccormick_lp = OsiLPEnginePtr(new OsiLPEngine());
  mccormick_lp->load(mccormick);
  vector<VariablePtr> xvars_mccormick;
  for(ConstVariableIterator it = mccormick->varsBegin(); it != mccormick->varsEnd(); ++it) {
    //!!! Sweet moses, this is bad, but we just do it to get something done now.
    
    VariablePtr v = boost::const_pointer_cast<Variable>(*it);     
    xvars_mccormick.push_back(v);
  }

  const int NN = 1000;
  double ch_sum = 0.0;
  double rbr_sum = 0.0;
  double tbt_sum = 0.0;
  double mc_sum = 0.0;
  double zsum = 0.0;
  for(int xxx = 0; xxx < NN; xxx++) {
    
    vector<double> xfix = randPointInBox(instance);

    // Have to push back zeros for z vars to make InnerProduct work right!!
    vector<double> xeval = xfix;
    xeval.push_back(0.0);
    xeval.push_back(0.0);

#if 0
    for (UInt j = 0; j < xfix.size(); j++) {
      cout << "x[" << j << "] = " << xfix[j] << endl;
    }
#endif

    double zval = 0.0;
    for(UInt i = 0; i < mlcid.size(); i++) {
      const ConstraintPtr mlc = instance->getConstraint(mlcid[i]);
      const FunctionPtr f = mlc->getFunction();

      double t = f->eval(xeval);
      //  Hacky for the talk
      if (i == 1) {
	t *= -1;
      }
      //      cout << "MLC Row " << i << " z: " << zval << endl;
      zval += t;
    }
    zsum += zval;
    
    vector<VarBoundModPtr> ch_mods;
    for(int j = 0; j < xfix.size(); j++) {
      VarBoundModPtr m1 = VarBoundModPtr(new VarBoundMod(xvars_convex_hull[j], Lower, xfix[j]));
      VarBoundModPtr m2 = VarBoundModPtr(new VarBoundMod(xvars_convex_hull[j], Upper, xfix[j]));
      ch_mods.push_back(m1);
      ch_mods.push_back(m2);
    }
    
    for(vector<VarBoundModPtr>::iterator it = ch_mods.begin(); it != ch_mods.end();
	++it) {
      (*it)->applyToEngine(convex_hull_lp);
    }
    convex_hull_lp->solve();

    EngineStatus ch_stat = convex_hull_lp->getStatus();
    assert(ch_stat == 0);
    double ch_zlp;
    ch_zlp = convex_hull_lp->getSolutionValue();

    vector<VarBoundModPtr> rbr_mods;
    for(int j = 0; j < xfix.size(); j++) {
      VarBoundModPtr m1 = VarBoundModPtr(new VarBoundMod(xvars_row_by_row[j], Lower, xfix[j]));
      VarBoundModPtr m2 = VarBoundModPtr(new VarBoundMod(xvars_row_by_row[j], Upper, xfix[j]));
      rbr_mods.push_back(m1);
      rbr_mods.push_back(m2);
      //cout << "Fixing to : " << xfix[j] << endl;
    }
    
    for(vector<VarBoundModPtr>::iterator it = rbr_mods.begin(); it != rbr_mods.end();
	++it) {
      (*it)->applyToEngine(row_by_row_lp);
    }
    row_by_row_lp->solve();

    EngineStatus rbr_stat = row_by_row_lp->getStatus();
    assert(rbr_stat == 0);
    double rbr_zlp;
    rbr_zlp = row_by_row_lp->getSolutionValue();

    vector<VarBoundModPtr> tbt_mods;
    for(int j = 0; j < xfix.size(); j++) {
      VarBoundModPtr m1 = VarBoundModPtr(new VarBoundMod(xvars_term_by_term[j], Lower, xfix[j]));
      VarBoundModPtr m2 = VarBoundModPtr(new VarBoundMod(xvars_term_by_term[j], Upper, xfix[j]));
      tbt_mods.push_back(m1);
      tbt_mods.push_back(m2);
      //cout << "Fixing to : " << xfix[j] << endl;
    }
    
    for(vector<VarBoundModPtr>::iterator it = tbt_mods.begin(); it != tbt_mods.end();
	++it) {
      (*it)->applyToEngine(term_by_term_lp);
    }
    term_by_term_lp->solve();

    EngineStatus tbt_stat = term_by_term_lp->getStatus();
    assert(tbt_stat == 0);
    double tbt_zlp;
    tbt_zlp = term_by_term_lp->getSolutionValue();

    vector<VarBoundModPtr> mc_mods;
    for(int j = 0; j < xfix.size(); j++) {
      VarBoundModPtr m1 = VarBoundModPtr(new VarBoundMod(xvars_mccormick[j], Lower, xfix[j]));
      VarBoundModPtr m2 = VarBoundModPtr(new VarBoundMod(xvars_mccormick[j], Upper, xfix[j]));
      mc_mods.push_back(m1);
      mc_mods.push_back(m2);
      //cout << "Fixing to : " << xfix[j] << endl;
    }
    
    for(vector<VarBoundModPtr>::iterator it = mc_mods.begin(); it != mc_mods.end();
	++it) {
      (*it)->applyToEngine(mccormick_lp);
    }
    mccormick_lp->solve();

    EngineStatus mc_stat = mccormick_lp->getStatus();
    assert(mc_stat == 0);
    double mc_zlp;
    mc_zlp = mccormick_lp->getSolutionValue();

    if (ch_stat != 0 || rbr_stat != 0 || tbt_stat != 0 || mc_stat != 0) {
      cout << "DANGER, INFEASIBLE!" << endl;
    }
    cout << zval << " , " << ch_zlp << " , " << rbr_zlp << " , " << tbt_zlp << " , " << mc_zlp << endl; 


    ch_sum += ch_zlp;
    rbr_sum += rbr_zlp;
    tbt_sum += tbt_zlp;
    mc_sum += mc_zlp;


  }

  cout << "FINAL" << endl;
  cout << (zsum/NN) << " " << (ch_sum/NN) << " " << (rbr_sum/NN) << " " 
       << (tbt_sum/NN) << " " << (mc_sum/NN) << endl; 
  // For min
  cout << (zsum/NN - ch_sum/NN) << " " << (zsum/NN - rbr_sum/NN) << " " 
       << (zsum/NN - tbt_sum/NN) << " " << (zsum/NN - mc_sum/NN) << endl;
  // For max
  cout << (-zsum/NN - ch_sum/NN) << " " << (-zsum/NN - rbr_sum/NN) << " " 
       << (-zsum/NN - tbt_sum/NN) << " " << (-zsum/NN - mc_sum/NN) << endl;
    
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

static vector<double> 
randPointInBox(ConstInstancePtr instance)
{
  vector<double> x;
  for(ConstVariableIterator it = instance->varsBegin(); it != instance->varsEnd(); ++it) {
    double lb = (*it)->getLb();
    double ub = (*it)->getUb();
    x.push_back(lb + drand48()*(ub-lb));
  }

  // Delete x for 2 eta/z vars variable
  x.pop_back();
  x.pop_back();
  return x;
  
}
