//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file simple-bnb-cg.cpp
 * \brief This is example driver code to read a qp from an .nl file and create
 * a readable file for Matlab/Octave. The format of the output file is
 * described in README
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"
#include "AMPLHessian.h"
#include "Constraint.h"
#include "Environment.h"
#include "LinearFunction.h"
#include "Objective.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "Variable.h"

using namespace Minotaur;

void usage()
{
  std::cout << "usage: ./qp2octave {name-of-.nl-file}\n";
}

int main(int argc, char** argv)
{
  int rc = 0;
  FILE *ofile = 0;
  EnvPtr env;
  MINOTAUR_AMPL::AMPLInterfacePtr iface;
  ProblemPtr inst;
  int n, m, c, v, nnz, *Arows, *Acols, err;
  UInt *Hrows, *Hcols; 
  double infty = 1e20;
  double *clb, *cub, *vlb, *vub, *Avals, *Hvals, *x, *cmult, *cv;
  LinearFunctionPtr lf;

  if (argc < 2) {
    usage();
    return -1;
  }

  env = (Minotaur::EnvPtr) new Environment();
  iface = new MINOTAUR_AMPL::AMPLInterface(env);
  inst = iface->readInstance(argv[1]);
  inst->setNativeDer();
  inst->calculateSize();


  n = inst->getNumVars();
  m = inst->getNumCons();
  clb = new double[m];
  cub = new double[m];
  vlb = new double[n];
  vub = new double[n];
  cv  = new double[n];
  nnz = inst->getSize()->linTerms;
  Acols = new int[nnz];
  Arows = new int[nnz];
  Avals = new double[nnz];
  nnz = inst->getNumHessNnzs();
  Hcols = new UInt[nnz];
  Hrows = new UInt[nnz];
  Hvals = new double[nnz];
  cmult = new double[m];

  // constraint bounds
  c = 0;
  {
  for(ConstraintConstIterator cit = inst->consBegin(); cit!=inst->consEnd();
      ++cit, ++c) {
    FunctionType ctype = (*cit)->getFunctionType();
    if(ctype != Linear) {
      std::cerr << "Constraint is not linear, exiting" << std::endl;
      rc = 101;
      goto CLEANUP;
    }
    clb[c] = std::max((*cit)->getLb(), -infty);
    cub[c] = std::min((*cit)->getUb(), infty);
  }
  }

  // constraint bounds
  v = 0;
  {
  for(VariableConstIterator vit = inst->varsBegin(); vit!=inst->varsEnd();
      ++vit, ++v) {
    vlb[v] = std::max((*vit)->getLb(), -infty);
    vub[v] = std::min((*vit)->getUb(), infty);
  }
  }

  // constraint matrix 
  c = 0;
  nnz = 0;
  {
  for(ConstraintConstIterator cit = inst->consBegin(); cit!=inst->consEnd();
      ++cit, ++c) {
    lf = (*cit)->getLinearFunction();
    for(VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
        ++it,++nnz) {
      Arows[nnz] = c;
      Acols[nnz] = (it)->first->getIndex();
      Avals[nnz] = (it)->second;
    }
  }
  }

  // cost vector
  v = 0;
  lf = inst->getObjective()->getLinearFunction();
  if (lf) {
    for(VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
        ++it,++v) {
      cv[v] = (it)->second;
    }
  }


  // Evaluate hessian at 0.
  v = 0;
  nnz = 0;
  err = 0;
  inst->getHessian()->fillRowColIndices(Hrows, Hcols);
  inst->getHessian()->fillRowColValues(x, 1.0, cmult, Hvals, &err);


  ofile = fopen("qp.m","w");
  fprintf(ofile, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
  fprintf(ofile, "%% QP data for file %s\n",argv[1]);
  fprintf(ofile, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n");

  fprintf(ofile, "%%Variables\nn = %d\n\n%%Constraints\nm = %d\n\n",n,m);

  fprintf(ofile, "%% variable lower bounds \nvlb = [");
  for (v=0; v<n; ++v) {
    fprintf(ofile, "%f ", vlb[v]);
  }
  fprintf(ofile, "];\n\n");
  fprintf(ofile, "%% variable upper bounds \nvub = [");
  for (v=0; v<n; ++v) {
    fprintf(ofile, "%f ", vub[v]);
  }
  fprintf(ofile, "];\n\n");

  fprintf(ofile, "%% constraint lower bounds \nclb = [");
  for (c=0; c<m; ++c) {
    fprintf(ofile, "%f ",clb[c]);
  }
  fprintf(ofile, "];\n\n");
  fprintf(ofile, "%% constraint upper bounds \ncub = [");
  for (c=0; c<m; ++c) {
    fprintf(ofile, "%f ", cub[c]);
  }
  fprintf(ofile, "];\n\n");

  fprintf(ofile, "%% cost vector \ncv = [");
  for (c=0; c<n; ++c) {
    fprintf(ofile, "%f ", cv[c]);
  }
  fprintf(ofile, "];\n\n");

  fprintf(ofile, "%% constraint matrix \nArows = [");
  for (c=0; c<inst->getSize()->linTerms; ++c) {
    fprintf(ofile, "%d ", Arows[c]);
  }
  fprintf(ofile, "];\n");
  fprintf(ofile, "Acols = [");
  for (c=0; c<inst->getSize()->linTerms; ++c) {
    fprintf(ofile, "%d ", Acols[c]);
  }
  fprintf(ofile, "];\n");
  fprintf(ofile, "Avals = [");
  for (c=0; c<inst->getSize()->linTerms; ++c) {
    fprintf(ofile, "%f ", Avals[c]);
  }
  fprintf(ofile, "];\n\n");

  fprintf(ofile, "%% Hessian matrix \nHrows = [");
  for (c=0; c<inst->getNumHessNnzs(); ++c) {
    fprintf(ofile, "%d ", Hrows[c]);
  }
  fprintf(ofile, "];\n");
  fprintf(ofile, "Hcols = [");
  for (c=0; c<inst->getNumHessNnzs(); ++c) {
    fprintf(ofile, "%d ", Hcols[c]);
  }
  fprintf(ofile, "];\n");
  fprintf(ofile, "Hvals = [");
  for (c=0; c<inst->getNumHessNnzs(); ++c) {
    fprintf(ofile, "%f ", Hvals[c]);
  }
  fprintf(ofile, "];\n\n");


CLEANUP:
  if (fclose) {
    fclose(ofile);
  }
  delete [] clb;
  delete [] cub;
  delete [] vlb;
  delete [] vub;
  delete [] Acols;
  delete [] Arows;
  delete [] Avals;
  delete [] Hcols;
  delete [] Hrows;
  delete [] Hvals;
  return rc;
}

