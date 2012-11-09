
//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
//

/**
 This is example driver code to convert multilinear constraints into linear if
 possible
*/

// Put config stuff (by hand) here.  Why?
#define MINOTAUR_RUSAGE

//#include "MinotaurConfig.h"

#include <AMPLInterface.h>
#undef filename
#include <MultilinearHandler.h>
#include <Problem.h>
#include <LPRelaxation.h>
#include <OsiLPEngine.h>
#include <Operations.h>
#include <Function.h>
#include <vector>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <string>
#include <Environment.h>
#include <Constraint.h>
#include <Objective.h>
#include <QuadraticFunction.h>
#include <Variable.h>
#include <iostream>

using namespace Minotaur;
using namespace std;
void usage()
{
  std::cout << "u.r. dum.  usage: ./multilinear-relax {name-of-.nl-file}\n";
}

int
main(int argc, char** argv)
{
  if (argc < 2) {
    usage();
    return -1;
  }

  // build the name of the output file
  string fileName = argv[1];
  fileName.erase(fileName.size()-1);
  fileName.erase(fileName.size()-1);
  fileName.erase(fileName.size()-1);
  fileName += ".txt";

  char *outFileName = (char*)fileName.c_str();
  FILE *pFile;
  FILE *qFile;
  qFile = fopen("nameList.txt", "a");
  fprintf(qFile, "%s\n", outFileName);
  fclose(qFile);
  pFile = fopen(outFileName, "w");
  Minotaur::EnvPtr env = (Minotaur::EnvPtr) new Minotaur::Environment();
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface(env);
  ProblemPtr inst, newInst;
  inst = iface->readInstance(argv[1]);
  int numVars = inst->getNumVars();
  int numCons = inst->getNumCons();
  
  fprintf(pFile,"%d\n", numVars);

  // find the number of linear constraints
  int numLinCons = 0;
  for(ConstraintConstIterator cIter=inst->consBegin(); cIter!=inst->consEnd(); ++cIter) {
    FunctionType ct = (*cIter)->getFunctionType();
    if(ct == Linear) {
      numLinCons++;
    }
  }

  // find the number of quadratic constraints
  int numQuadCons = 0;
  for(ConstraintConstIterator cIter=inst->consBegin(); cIter!=inst->consEnd(); ++cIter) {
    FunctionType ct = (*cIter)->getFunctionType();
    if(ct == Quadratic) {
      numQuadCons++;
    }
  }

  fprintf(pFile, "%d\n", numQuadCons);
  fprintf(pFile, "%d\n", numLinCons);

  ObjectivePtr obj = inst->getObjective(0);
  FunctionPtr objF = obj->getFunction();
  LinearFunctionPtr objLF = objF->getLinearFunction();
  QuadraticFunctionPtr objQF = objF->getQuadraticFunction();

  double **objQ = new double* [numVars];
  for(int i = 0; i < numVars; i++) 
    objQ[i] = new double[numVars];
  for(int i = 0; i < numVars; i++){
    for(int j = 0; j < numVars; j++) {
      objQ[i][j] = 0;
    }
  }

  double *objA = new double [numVars];
  for(int i = 0; i < numVars; i++)
    objA[i] = 0;

  //find objQ
  if(objQF) {
    for(VariableConstIterator varit1 = inst->varsBegin(); varit1 != inst->varsEnd(); ++varit1) {
      for(VariableConstIterator varit2 = inst->varsBegin(); varit2 != inst->varsEnd(); ++varit2) {
	for(VariablePairGroupConstIterator it = objQF->begin(); it != objQF->end(); ++it) {
	  if(((it->first).first)->getId() == (*varit1)->getId() && ((it->first).second)->getId() == (*varit2)->getId()){
	    int var1Id = (*varit1)->getId();
	    int var2Id = (*varit2)->getId();
	    objQ[var1Id][var2Id] = it->second;
	  }
	}
      }
    }
  }
  double ij;
  double ji;
  for(int i = 0; i < numVars; i++) {
    for(int j = i+1; j < numVars; j++) {
      ij = objQ[i][j];
      ji = objQ[j][i];
      objQ[i][j] = (ij+ji)/2;
      objQ[j][i] = (ij+ji)/2;
    }
  }
  
  for(int i = 0; i < numVars; i++) {
    for(int j = 0; j < numVars; j++) {
      fprintf(pFile, "%f\t", objQ[i][j]);
    }
    fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n\n");

  //find objA
  if(objLF) {
    for(VariableConstIterator varit = inst->varsBegin(); varit != inst->varsEnd(); ++varit) {
      for(VariableGroupConstIterator it = objLF->termsBegin(); it != objLF->termsEnd(); ++it) {
	if((*varit)->getId() == (it->first)->getId()) {
	  objA[(*varit)->getId()] = it->second;
	}
      }
    }
  }

  for(int i = 0; i < numVars; i++)
    fprintf(pFile, "%f\t", objA[i]);
  fprintf(pFile, "\n");
  fprintf(pFile, "\n\n");

  //take care of the quadratic constraints
  for(ConstraintConstIterator cIter=inst->consBegin(); cIter!=inst->consEnd(); ++cIter) {
    FunctionType ct = (*cIter)->getFunctionType();
    if(ct == Quadratic) {
      // if it's an equality constraint
      if((*cIter)->getUb() - (*cIter)->getLb() >= -1e-6 && (*cIter)->getUb() - (*cIter)->getLb() <= 1e-6)  {
	fprintf(pFile, "%d\n", 1);
	fprintf(pFile, "%f\n\n", (*cIter)->getUb());
      }
      // if it's a <= constraint
      else if((*cIter)->getUb() <= 1e6)  {
	fprintf(pFile, "%d\n", 2);
	fprintf(pFile, "%f\n\n", (*cIter)->getUb());
      }
      else if ((*cIter)->getLb() >= -1e6) {
	fprintf(pFile, "%d\n", 3);
	fprintf(pFile, "%f\n\n", (*cIter)->getLb());	
      }

      FunctionPtr F = (*cIter)->getFunction();
      LinearFunctionPtr LF = F->getLinearFunction();
      QuadraticFunctionPtr QF = F->getQuadraticFunction();
      
      double **Q = new double* [numVars];
      for(int i = 0; i < numVars; i++) 
	Q[i] = new double[numVars];
      for(int i = 0; i < numVars; i++){
	for(int j = 0; j < numVars; j++) {
	  Q[i][j] = 0;
	}
      }
      
      double *A = new double [numVars];
      for(int i = 0; i < numVars; i++)
	A[i] = 0;
      
      //find Q
      if(QF) {
	for(VariableConstIterator varit1 = inst->varsBegin(); varit1 != inst->varsEnd(); ++varit1) {
	  for(VariableConstIterator varit2 = inst->varsBegin(); varit2 != inst->varsEnd(); ++varit2) {
	    for(VariablePairGroupConstIterator it = QF->begin(); it != QF->end(); ++it) {
	      if(((it->first).first)->getId() == (*varit1)->getId() && ((it->first).second)->getId() == (*varit2)->getId()){
		int var1Id = (*varit1)->getId();
		int var2Id = (*varit2)->getId();
		Q[var1Id][var2Id] = it->second;
	      }
	    }
	  }
	}
      }
      double ij;
      double ji;
      for(int i = 0; i < numVars; i++) {
	for(int j = i+1; j < numVars; j++) {
	  ij = Q[i][j];
	  ji = Q[j][i];
	  Q[i][j] = (ij+ji)/2;
	  Q[j][i] = (ij+ji)/2;
	}
      }
      
      for(int i = 0; i < numVars; i++) {
	for(int j = 0; j < numVars; j++) {
	  fprintf(pFile, "%f\t", Q[i][j]);
	}
	fprintf(pFile, "\n");
      }
      fprintf(pFile, "\n\n");
      
      //find A
      if(LF) {
	for(VariableConstIterator varit = inst->varsBegin(); varit != inst->varsEnd(); ++varit) {
	  for(VariableGroupConstIterator it = LF->termsBegin(); it != LF->termsEnd(); ++it) {
	    if((*varit)->getId() == (it->first)->getId()) {
	      A[(*varit)->getId()] = it->second;
	    }
	  }
	}
      }
      
      for(int i = 0; i < numVars; i++)
	fprintf(pFile, "%f\t", A[i]);
      fprintf(pFile, "\n");  
      fprintf(pFile, "\n\n");      
    }
  }
  //take care of the linear constraints
  for(ConstraintConstIterator cIter=inst->consBegin(); cIter!=inst->consEnd(); ++cIter) {
    FunctionType ct = (*cIter)->getFunctionType();
    if(ct == Linear) {
      // if it's an equality constraint
      if((*cIter)->getUb() - (*cIter)->getLb() >= -1e-6 && (*cIter)->getUb() - (*cIter)->getLb() <= 1e-6)  {
	fprintf(pFile, "%d\n", 1);
	fprintf(pFile, "%f\n\n", (*cIter)->getUb());
      }
      // if it's a <= constraint
      else if((*cIter)->getUb() <= 1e6)  {
	fprintf(pFile, "%d\n", 2);
	fprintf(pFile, "%f\n\n", (*cIter)->getUb());
      }
      else if ((*cIter)->getLb() >= -1e6) {
	fprintf(pFile, "%d\n", 3);
	fprintf(pFile, "%f\n\n", (*cIter)->getLb());	
      }

      FunctionPtr F = (*cIter)->getFunction();
      LinearFunctionPtr LF = F->getLinearFunction();
      double *A = new double [numVars];
      for(int i = 0; i < numVars; i++)
	A[i] = 0;
      if(LF) {
	for(VariableConstIterator varit = inst->varsBegin(); varit != inst->varsEnd(); ++varit) {
	  for(VariableGroupConstIterator it = LF->termsBegin(); it != LF->termsEnd(); ++it) {
	    if((*varit)->getId() == (it->first)->getId()) {
	      A[(*varit)->getId()] = it->second;
	    }
	  }
	}
      }
      
      for(int i = 0; i < numVars; i++)
	fprintf(pFile, "%f\t", A[i]);
      fprintf(pFile, "\n");  
      fprintf(pFile, "\n\n");            
    }
  }

  // get the bound matrices
  double **lb = new double *[numVars+1];
  for(int i=0; i<numVars+1; i++)
    lb[i] = new double [numVars+1];
  double **ub = new double *[numVars+1];
  for(int i=0; i<numVars+1; i++)
    ub[i] = new double [numVars+1];
  lb[0][0]=1;
  ub[0][0]=1;

  for(VariableConstIterator varit1 = inst->varsBegin(); varit1 != inst->varsEnd(); ++varit1) {
    int var1Id = (*varit1)->getId();
    int var1Lb = (*varit1)->getLb();
    int var1Ub = (*varit1)->getUb();
    lb[0][var1Id+1] = var1Lb;
    ub[0][var1Id+1] = var1Ub;
    lb[var1Id+1][0] = var1Lb;
    ub[var1Id+1][0] = var1Ub;
    
    for(VariableConstIterator varit2 = inst->varsBegin(); varit2 != inst->varsEnd(); ++varit2) {
      int var2Id = (*varit2)->getId();
      int var2Lb = (*varit2)->getLb();
      int var2Ub = (*varit2)->getUb();
      if(var2Id >= var1Id) {
	double prodLb;
	double prodUb;
	BoundsOnProduct(var1Lb, var1Ub, var2Lb, var2Ub, prodLb, prodUb);
	lb[var1Id+1][var2Id+1] = prodLb;
	ub[var1Id+1][var2Id+1] = prodUb;
	lb[var2Id+1][var1Id+1] = prodLb;
	ub[var2Id+1][var1Id+1] = prodUb;

      }
    }
  }

  //print lb
  for(int i = 0; i < numVars+1; i++) {
    for(int j = 0; j < numVars+1; j++) {
      fprintf(pFile, "%f\t", lb[i][j]);
    }
    fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n\n");

  //print ub
  for(int i = 0; i < numVars+1; i++) {
    for(int j = 0; j < numVars+1; j++) {
      fprintf(pFile, "%f\t", ub[i][j]);
    }
    fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n\n");

  /*
  /// 
  int newObjVarCnt;
  if(obj->getFunctionType() == Linear)
    newObjVarCnt = inst->getNumVars();
  else
    newObjVarCnt = inst->getNumVars() + 1;
  printf("newObjVarCnt = %d\n", newObjVarCnt);
  printf("inst num vars = %d\n", inst->getNumVars());
  printf("inst num cons = %d\n", inst->getNumCons());

  LPRelaxationPtr lpr = LPRelaxationPtr(new LPRelaxation());

  MultilinearHandlerPtr mhandler = MultilinearHandlerPtr(new MultilinearHandler(inst));

  bool isFeasible;
  int nCons = inst->getNumCons();
  int nVars = inst->getNumVars();
  vector<bool> c_list(nCons);
  */
}



