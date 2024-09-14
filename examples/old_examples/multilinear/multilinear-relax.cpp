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
//#include <MultilinearHandler.h>
#include <Problem.h>
#include <LPRelaxation.h>
#include <OsiLPEngine.h>
#include <Function.h>
#include <vector>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <iostream>
#include <Environment.h>
#include <Constraint.h>
#include <Objective.h>
#include <Solution.h>

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
  std::cout << "u.r. dum.  usage: ./multilinear-relax {name-of-.nl-file}\n";
}

int
main(int argc, char** argv)
{
  if (argc < 2) {
    usage();
    return -1;
  }
  
  time_start();

  Minotaur::EnvPtr env = (Minotaur::EnvPtr) new Minotaur::Environment();
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface(env);
  ProblemPtr inst, newInst;
  inst = iface->readInstance(argv[1]);
  printf("**********************************************\n");
  printf("**********************************************\n");

  int numCons = 0;
  int numLinCons = 0;
  for(ConstraintConstIterator cIter=inst->consBegin(); cIter!=inst->consEnd(); ++cIter) {
    numCons++;
    FunctionType ct = (*cIter)->getFunctionType();
    if(ct == Linear) {
      numLinCons++;
    }
  }
  ObjectivePtr obj = inst->getObjective();

  printf("The original objective function is:\n");

  /// 
  int newObjVarCnt;
  if(obj->getFunctionType() == Linear)
    newObjVarCnt = inst->getNumVars();
  else
    newObjVarCnt = inst->getNumVars() + 1;
  printf("newObjVarCnt = %d\n", newObjVarCnt);
  printf("inst num vars = %d\n", inst->getNumVars());
  printf("inst num cons = %d\n", inst->getNumCons());

  LPRelaxationPtr lpr = (new LPRelaxation(env));

  //MultilinearHandlerPtr mhandler = MultilinearHandlerPtr(new MultilinearHandler(env, inst));

  bool isFeasible;
  int nCons = inst->getNumCons();
  int nVars = inst->getNumVars();
  vector<bool> c_list(nCons);
  int groupsCnt = 0;
  int totalCnt = 0;

  //mhandler->relax(lpr, isFeasible, c_list);
  //groupsCnt = mhandler->getNumGroups();
  //totalCnt = mhandler->getTotalRepetition();

  OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine(env));
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
  time_finish();
  double t = time_taken();
  ConstSolutionPtr lpSol = lpE->getSolution();
  const double * sol;
  cout << t << endl;
  sol = lpSol->getPrimal();
  /*
  for(int i=0; i<nv; i++)
    cout << " , " << sol[i];
    //    cout <<"var["<<i<<"] = " << sol[i]<<endl;
  cout << endl;
  */

  //PIPI
  FILE *out;
  out = fopen("result.txt", "a");
  fprintf(out, "%s\t%d\t%d\t%f\t%f\t%d\t%d\n", argv[1],numLinCons, numCons-numLinCons, lpE->getSolutionValue(), t, groupsCnt,totalCnt);

  fclose(out);


  /*
  lpr->write(std::cout);

  OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine());
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
  Const double *sol = lpE->getSolution();
  for(int i=0; i<nv; i++)
    cout << " , " << sol[i];
  cout << endl;

  time_finish();
  double t = time_taken();

  FILE * pFile;
  pFile = fopen ("results.txt","a");
  fprintf(pFile, "%s   %d   %d   %d   %d    %d   %d   %f    %f   \n", argv[1], lpE->getStatus(), nVars, nCons, numLinCons, lpr->getNumVars(), 
	  lpr->getNumCons(), lpE->getSolutionValue(), t);
  printf("\ntime: %f\n", t);
  fclose (pFile);
  */
}



