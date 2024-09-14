//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
//

/**
 This is example driver code to convert quadratic constraints into socps if
 possible and solve them using a nonlinear solver.
*/

// Put config stuff (by hand) here.  Why?
#define MINOTAUR_RUSAGE

//#include "MinotaurConfig.h"
#include <iomanip>
#include <iostream>
#include <AMPLInterface.h>
#undef filename
#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "IpoptEngine.h"
#include "LinearHandler.h"
#include "PCBProcessor.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "OsiLPEngine.h"
#include "Problem.h"
#include "QGHandler.h"
#include "Relaxation.h"
#include "LPRelaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"
#include "AMPLInterface.h"
#include <Problem.h>

#include <Function.h>
#include <vector>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

using namespace Minotaur;
using namespace std;
static long t_start, t_finish;
static struct tms t_usage;
static double timeTaken = 0.0;
static long int clktck = sysconf (_SC_CLK_TCK);
static double timelimit = 1000000.00;


void time_start(){

  times (&t_usage);
  t_start = t_usage.tms_utime + t_usage.tms_stime;
 
  //printf("TT: %g\n", timeTaken );
}


void time_finish(){
 
  times (&t_usage);
  t_finish = t_usage.tms_utime + t_usage.tms_stime;    
  timeTaken += (double)(t_finish - t_start)/clktck;
  //printf("TT: %g\n", timeTaken );
}


bool time_limit(){
 
  times (&t_usage);
  t_finish= t_usage.tms_utime + t_usage.tms_stime;
  //printf("TT: %g\n", timeTaken );
  if(timeTaken + (double)(t_finish - t_start)/clktck > timelimit)
    return true;
  return false;

}

double time_taken(){
 
  times (&t_usage);
  t_finish= t_usage.tms_utime + t_usage.tms_stime;
  // printf("TT: %g\n", timeTaken );
  return (timeTaken + (double)(t_finish - t_start)/clktck);

}


void usage()
{
  cout << "u.r. dum.  usage: ./bilinear-relax {name-of-.nl-file}\n";
}

int
main(int argc, char** argv)
{
    if (argc < 2) {
    usage();
    return -1;
  }

time_start();

  Minotaur::EnvPtr env1 = new Minotaur::Environment();
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface(env1);
  Minotaur::ProblemPtr inst, newInst;
  inst = iface->readInstance(argv[1]);

  //  inst->calculateSize();

  //  if (inst->isLinear() || inst->isQuadratic()) {
  //    inst->writeSize();
  //  }

  /*
  LPRelaxationPtr lpr1 = LPRelaxationPtr(new LPRelaxation(inst));
  OsiLPEnginePtr lpE1 = OsiLPEnginePtr(new OsiLPEngine());
  lpE1->load(lpr1);
  lpE1->writeLP("1st");
  */
  int numLinCons = 0;
  for(ConstraintConstIterator cIter=inst->consBegin(); cIter!=inst->consEnd(); ++cIter) {
    FunctionType ct = (*cIter)->getFunctionType();
    if(ct == Linear) {
      numLinCons++;
    }
  }
  ObjectivePtr obj = inst->getObjective();

  printf("The original objective function is:\n");
  obj->write(cout);

  /// 
  int newObjVarCnt;
  if(obj->getFunctionType() == Linear)
    newObjVarCnt = inst->getNumVars();
  else
    newObjVarCnt = inst->getNumVars() + 1;
  printf("newObjVarCnt = %d\n", newObjVarCnt);
  printf("inst num vars = %d\n", inst->getNumVars());
  printf("inst num cons = %d\n", inst->getNumCons());
  ///

  LPRelaxationPtr lpr = new LPRelaxation(env1);

  //BilinearHandlerPtr bhandler = BilinearHandlerPtr(new BilinearHandler(inst));
  bool isFeasible;
  int nCons = inst->getNumCons();
  int nVars = inst->getNumVars();
  vector<bool> c_list(nCons);

  //bhandler->relax(lpr, isFeasible, c_list);
  lpr->write(cout);

  OsiLPEnginePtr lpE = new OsiLPEngine(env1);
  int nv = lpr->getNumVars();
  
  lpE->load(lpr);
  lpE->solve();
  string status = lpE->getStatusString();

  lpE->writeLP("blah");

  cout << "After solve : ";
  cout << lpE->getStatusString();
  cout << " , " <<lpE->getStatus();
  cout << " , " << lpr->getNumVars();
  cout << " , " << lpr->getNumCons();
  cout << " , " << lpE->getSolutionValue() << endl;
  ConstSolutionPtr sol = lpE->getSolution();  // Correct type for the solution pointer
const double* primalVals = sol->getPrimal();         // Access primal variable values

 int numVars = lpE-> getNumCols();  // Get the number of variables in the problem

// Print the solution values
for (int i = 0; i < numVars; ++i) {
    std::cout << "x[" << i << "] = " << primalVals[i] << " , ";
}

  time_finish();
  double t = time_taken();

  FILE * pFile;
  pFile = fopen ("results.txt","a");
  fprintf(pFile, "%s   %d   %d   %d   %d    %d   %d   %f    %f   \n", argv[1], lpE->getStatus(), nVars, nCons, numLinCons, lpr->getNumVars(), 
	  lpr->getNumCons(), lpE->getSolutionValue(), t);
  printf("\ntime: %f\n", t);
  /////////////
  srand (45);

  int objCnt = 0;
  double ** objVector = new double *[objCnt];
  /*
  for(int i=0; i<objCnt; i++)
    objVector[i] = new double [lpr->getNumVars()];

  for(int i=0; i<objCnt; i++) {
    for(int j=0; j<lpr->getNumVars(); j++) {
      if(j<inst->getNumVars()) {
	objVector[i][j] = 10*(1-(((double)rand()/RAND_MAX)*2));
      }
      else {
	objVector[i][j] = 0;
      }
    }
    LinearFunctionPtr newLinObj = LinearFunctionPtr(new LinearFunction());
    int counter = 0;
    for(VariableConstIterator v_iter = lpr->varsBegin(); v_iter != lpr->varsEnd(); ++v_iter) {
      VariablePtr svar;
      svar = (*v_iter);
      if(counter < newObjVarCnt)
	newLinObj->addTerm(svar, objVector[i][counter]);
      else
	break;
      counter++;
    }
    ObjectivePtr oPtr = lpr->getObjective(0);
    //    oPtr->write(cout);
    LinearFunctionPtr linObjF = oPtr->getLinearFunction();
    LinearFunctionPtr negLinObjF = -1*linObjF;
    oPtr->add(negLinObjF);
    oPtr->add(newLinObj);
    
    ObjectivePtr oPtr1 = lpr->getObjective();

    lpE->load(lpr);
    lpE->solve();
    lpE->writeLP("blah2");
    
    fprintf (pFile, "%f\n", lpE->getSolutionValue());
    printf ("%f\n", lpE->getSolutionValue());
  }
  */
  fclose (pFile);
  /*
	///////////////////
	for(int i=0; i<objCnt; i++) {
	  for(int j=0; j<lpr->getNumVars(); j++) {
		printf("%f  ",objVector[i][j]);
	  }
	  printf("\n");
	}

	///////////////////
	*/
  /////////////

}



