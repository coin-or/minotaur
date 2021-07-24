// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
// 

/**
 * \file CplexLPEngine.cpp
 * \brief Implement an interface to the Cplex LP solver
 * \author Prashant Palkar and Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <iomanip>

#include "MinotaurConfig.h"
#include "CplexLPEngine.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "LinearFunction.h"
#include "HessianOfLag.h"
#include "Logger.h"
#include "Objective.h"
#include "Option.h"
#include "Problem.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Timer.h"
#include "Variable.h"

using namespace Minotaur;

//#define SPEW 1

const std::string CplexLPEngine::me_ = "CplexLPEngine: ";

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CpxLPWarmStart::CpxLPWarmStart()
  : mustDelete_(true),
  sol_(0)
  //constat_(0),
  //varstat_(0)
{
}


CpxLPWarmStart::~CpxLPWarmStart()
{
  if (sol_) {
    sol_ = 0;
  }
  //if (varstat_) {
    //delete [] varstat_; varstat_ = 0;
  //}
  //if (constat_) {
    //delete [] constat_; constat_ = 0;
  //}
}


bool CpxLPWarmStart::hasInfo()
{
  //if (constat_ || varstat_ || sol_) {
    //return true;
  //} else {
    //return false;
  //}
  //Assume that Cplex's warmstart info from the previous solve is good
  return true;
}


//void CpxLPWarmStart::setVarStat(int* varstat, int numvars)
//{
  //if (varstat_) {
    //delete [] varstat_;
  //}
  //varstat_ = new int[numvars];
  //for (int i=0; i < numvars; ++i) {
    //varstat_[i] = varstat[i];
  //}
//}

//void CpxLPWarmStart::setConStat(int* constat, int numcons)
//{
  //if (constat_) {
    //delete [] constat_;
  //}
  //constat_ = new int[numcons];
  //for (int i=0; i < numcons; ++i) {
    //constat_[i] = constat[i];
  //}
//}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CplexLPEngine::CplexLPEngine(EnvPtr env)
  : bndChanged_(true),
    consChanged_(true),
    env_(env),
    maxIterLimit_(10000),
    objChanged_(true),
    problem_(0),
    sol_(0),
    strBr_(false),
    ws_(0)
{
  cpxenv_ = 0;
  timer_ = env->getNewTimer();
  stats_ = new CplexLPStats();
  stats_->calls    = 0;
  stats_->strCalls = 0;
  stats_->time     = 0;
  stats_->strTime  = 0;
  stats_->iters    = 0;
  stats_->strIters = 0;
  logger_ = env->getLogger();
  timeLimit_ = INFINITY;
  dualObjLimit_ = INFINITY;
}


CplexLPEngine::~CplexLPEngine()
{
  if (cpxenv_ != NULL) {
    cpxstatus_ = CPXXcloseCPLEX (&cpxenv_);

    /* Note that CPXXcloseCPLEX produces no output,
       so the only way to see the cause of the error is to use
       CPXXgeterrorstring.  For other CPLEX routines, the errors will
       be seen if the CPXPARAM_ScreenOutput indicator is set to CPX_ON. */
    if (cpxstatus_) {
      logger_->msgStream(LogError) << "Could not close CPLEX environment.\n";
    }
  }
  if (timer_) {
    delete timer_;
  }
  if (stats_) {
    delete stats_;
  }
  if (problem_) {
    problem_->unsetEngine();
    problem_ = 0;
  }
  if (sol_) {
    delete sol_;
  }
  if (ws_) {
    delete ws_; ws_ = 0;
  }
}


void CplexLPEngine::addConstraint(ConstraintPtr con)
{
  LinearFunctionPtr lf = con->getLinearFunction();
  int nnz = lf->getNumTerms();
  int *cols = new int[nnz];
  double *elems = new double[nnz];
  double *conrhs = new double;
  *conrhs = con->getUb();
  char **conname = new char*[1];
  conname[0] = new char[con->getName().length() + 1];
  strcpy(conname[0], con->getName().c_str());
  char* sense = new char;
  *sense = 'L';
  CPXNNZ *start = new CPXNNZ[1];
  start[0] = 0;

  VariableGroupConstIterator it;
  int i=0;

  for (it = lf->termsBegin(); it != lf->termsEnd(); ++it, ++i){
    cols[i] = it->first->getIndex();
    elems[i] = it->second;
  }

  cpxstatus_ = CPXXaddrows (cpxenv_, cpxlp_, 0, 1, nnz, conrhs, sense, start,
                       cols, elems, NULL, conname);
  if (cpxstatus_) {
    assert(!"unable to add constraint!");
  }
  delete [] cols;
  delete [] elems;
  delete conrhs;
  delete [] conname[0];
  delete [] conname;
  delete sense;
  delete [] start;
  consChanged_ = true;
}


void CplexLPEngine::changeBound(ConstraintPtr, BoundType, double)
{
  assert(!"implement me!");
}


void CplexLPEngine::changeBound(VariablePtr var, BoundType lu, double new_val)
{
  //XXX: need a better map than the following for mapping variables to indices
  //and vice versa
  CPXDIM *index = new int[1];
  char *bndType = new char[1];
  double *value = new double[1];
  index[0] = var->getIndex();
  value[0] = new_val;

  switch (lu) {
   case Lower:
     bndType[0] = 'L';
     cpxstatus_ = CPXXchgbds (cpxenv_, cpxlp_, 1, index, bndType, value);
     break;
   case Upper:
     bndType[0] = 'U';
     cpxstatus_ = CPXXchgbds (cpxenv_, cpxlp_, 1, index, bndType, value);
     break;
   default:
     break;
  }
  bndChanged_ = true;
  delete [] index;
  delete [] bndType;
  delete [] value;
}


void CplexLPEngine::changeBound(VariablePtr, double, double)
{
  assert(!"implement me!");
}


void CplexLPEngine::changeConstraint(ConstraintPtr, LinearFunctionPtr, 
                                   double, double)
{
  assert(!"implement me!");
}


void CplexLPEngine::changeConstraint(ConstraintPtr, NonlinearFunctionPtr)
{
  assert(!"Cannot change a nonlinear function in CplexLPEngine");
}


void CplexLPEngine::changeObj(FunctionPtr, double)
{
  assert(!"implement me!");
}


void CplexLPEngine::clear()
{
  assert(!"implement me!");
}


void CplexLPEngine::disableStrBrSetup()
{
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "disabling strong branching."
                               << std::endl;
#endif
  strBr_ = false;
}


EnginePtr CplexLPEngine::emptyCopy()
{
  return new CplexLPEngine(env_);
}


void CplexLPEngine::enableStrBrSetup()
{
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "enabling strong branching."
                               << std::endl;
#endif
  strBr_ = true;
}


void CplexLPEngine::fillStats(std::vector<double> &lpStats)
{
  if (lpStats.size()) {
    lpStats[0] += stats_->calls;
    lpStats[1] += stats_->strCalls;
    lpStats[2] += stats_->time;
    lpStats[3] += stats_->strTime;
    lpStats[4] += stats_->iters;
    lpStats[5] += stats_->strIters;
  }
}

std::string CplexLPEngine::getName() const
{
  return "CplexLP";
}


double CplexLPEngine::getSolutionValue() 
{
  return sol_->getObjValue();
}


ConstSolutionPtr CplexLPEngine::getSolution() 
{
  return sol_;
}


EngineStatus CplexLPEngine::getStatus() 
{
  return status_;
}


WarmStartPtr CplexLPEngine::getWarmStartCopy()
{
  //Create a new copy of warm-start information from cplex
  CpxLPWarmStartPtr ws = 0;
  //if (ws_) {
    //ws = new CpxLPWarmStart();
    //// save it. It is our responsibility to free it.
    //ws->setVarStat(ws_->getVarStat(), problem_->getNumVars());
    //ws->setConStat(ws_->getConStat(), ws_->getNumCons());
    //ws->setNumCons(ws_->getNumCons());
  //}
  return ws;
}


void CplexLPEngine::printx(double *x, UInt size) {
  for (UInt i=0; i < size; i++) {
    logger_->msgStream(LogInfo) << i+1 << " " << x[i] << "\n";
  }
}


void CplexLPEngine::printx(const int *x, UInt size) {
  for (UInt i=0; i < size; i++) {
    logger_->msgStream(LogInfo) << i+1 << " " << x[i] << "\n";
  }
}


void CplexLPEngine::load(ProblemPtr problem)
{
  problem_ = problem;

  if (cpxenv_ != NULL) {
    cpxstatus_ = CPXXcloseCPLEX (&cpxenv_);
    /* Note that CPXXcloseCPLEX produces no output,
       so the only way to see the cause of the error is to use
       CPXXgeterrorstring.  For other CPLEX routines, the errors will
       be seen if the CPXPARAM_ScreenOutput indicator is set to CPX_ON. */
    if (cpxstatus_) {
      logger_->msgStream(LogError) << "Could not close CPLEX environment.\n";
    }
  }

  // Cplex objects
  int numvars = problem->getNumVars();
  int numcons = problem->getNumCons();
  if (ws_) {
    delete ws_; ws_ = 0;
  }
  //ws_ = new CpxLPWarmStart(numvars, numcons);
  ws_ = new CpxLPWarmStart();

  int i, j, rcnt;
  double obj_sense = 1, ztol = 1e-6;
  double *conlb = 0, *conub = 0, *conrhs = 0, *conrange = 0, *varlb = 0,
         *varub = 0, *obj = 0, *value = 0;
  CPXDIM *index = 0, *conind = 0;
  CPXNNZ *start = new CPXNNZ[numcons];
  memset(start, 0, numcons*sizeof(int));
  char **conname = new char*[numcons];
  char **varname = new char*[numvars];

  std::vector<UInt> conindvec;
  std::vector<double> conrangevec; 

  ConstraintConstIterator c_iter;
  VariableConstIterator v_iter;
  
  conlb = new double[numcons];
  conub = new double[numcons];
  conrhs = new double[numcons];
  varlb = new double[numvars];
  varub = new double[numvars];
  obj = new double[numvars];
  char *sense = new char[numcons];
  std::fill(sense, sense+numcons, 'L'); //assuming all cons <= to start with
  char *vartype = new char[numvars];
  std::fill(vartype, vartype+numvars, 'C'); //setting all vars as continuous
  int nnz = 0;  
  VariableGroupConstIterator it;
  
  LinearFunctionPtr lin;

  /* Initialize the CPLEX environment */
  cpxenv_ = CPXXopenCPLEX (&cpxstatus_);

  /* If an error occurs, the status value indicates the reason for
     failure.  A call to CPXXgeterrorstring will produce the text of
     the error message.  Note that CPXXopenCPLEX produces no output,
     so the only way to see the cause of the error is to use
     CPXXgeterrorstring.  For other CPLEX routines, the errors will
     be seen if the CPXXPARAM_ScreenOutput indicator is set to CPXX_ON.  */

  if (cpxenv_ == NULL) {
     char  errmsg[CPXMESSAGEBUFSIZE];
     logger_->msgStream(LogError) << me_ << "Could not open CPLEX environment."
       << std::endl;
     CPXXgeterrorstring (cpxenv_, cpxstatus_, errmsg);
     logger_->msgStream(LogError) << me_ << errmsg << std::endl;
     goto TERMINATE;
  }

  if (stats_->calls < 1) {
#if 0
    /* Turn on output to the screen (use a file to read parameters: TBD) */
    cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_ScreenOutput, CPX_ON);
    if (cpxstatus_) {
       logger_->msgStream(LogError) << me_ << "Failure to turn on screen indicator, error "
         << cpxstatus_ << std::endl;
       goto TERMINATE;
    }
#endif

#if 0
    /* Display information for each simplex iteration */
    cpxstatus_ = CPXXsetintparam (cpxenv_, CPX_PARAM_SIMDISPLAY, 2);
    if (cpxstatus_) {
       logger_->msgStream(LogError) << me_ << "Failure to turn on simplex iterations log, error "
         << cpxstatus_ << std::endl;
       goto TERMINATE;
    }
#endif

    /* Set number of threads (1 for LP solver) */
    cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Threads, 1);
    if (cpxstatus_) {
       logger_->msgStream(LogError) << me_ << "Failure to set number of threads, error "
         << cpxstatus_ << std::endl;
       goto TERMINATE;
    }
  }

  /* Create the problem. */
  cpxlp_ = CPXXcreateprob (cpxenv_, &cpxstatus_, "minoCpxProb");
  if (cpxlp_ == NULL) {
     logger_->msgStream(LogError) << me_ << "Failed to create LP." << std::endl;
     goto TERMINATE;
  }

  /* map the variables in this constraint to the function type (linear here) */

  //XXX Need to count the number of nnz in the problem 
  //     -- maybe add it to class later  
  i = 0;
  for (c_iter = problem->consBegin(); c_iter != problem->consEnd(); ++c_iter) {
    //XXX Don't want assert here, but not sure of eventually calling sequence
    //     and assumptions
    assert((*c_iter)->getFunctionType() == Linear);
    lin = (*c_iter)->getLinearFunction();
    nnz += lin->getNumTerms();
    conname[i] = new char[(*c_iter)->getName().length() + 1];
    strcpy(conname[i], (*c_iter)->getName().c_str());
    i++;
  }

  index = new int[nnz];
  value = new double[nnz];

  i = 0;
  j = 0;
  rcnt = 0;
  start[0] = 0;
  for (c_iter = problem->consBegin(); c_iter != problem->consEnd(); ++c_iter) {
    conlb[i] = (*c_iter)->getLb();
    conub[i] = (*c_iter)->getUb();
    conrhs[i] = conub[i];
    lin = (*c_iter)->getLinearFunction();
    for (it = lin->termsBegin(); it != lin->termsEnd(); ++it) {
      ConstVariablePtr vPtr = it->first;
      index[j] = vPtr->getIndex();
      value[j] = it->second;
      ++j;
    }
    
    if (fabs(conub[i] - conlb[i]) < ztol) {
      sense[i] = 'E';                              
    } else if (conlb[i] > -INFINITY) {
      if (conub[i] < INFINITY) {
        sense[i] = 'R';
        // always provide positive range values: conlb[i] <= constraintFunction <= conlb[i] + range[i]
        conrhs[i] = conlb[i];
        conindvec.push_back(i);
        conrangevec.push_back(conub[i] - conlb[i]);
        rcnt++;
      } else {
        sense[i] = 'G';
        conrhs[i] = conlb[i];
      }
    }

    ++i;
    if (i < numcons) {
      start[i]=j;
    }
  }

  i = 0;
  for (v_iter=problem->varsBegin(); v_iter!=problem->varsEnd(); ++v_iter, 
       ++i) {
    varlb[i] = (*v_iter)->getLb();
    varub[i] = (*v_iter)->getUb();
    varname[i] = new char[(*v_iter)->getName().length() + 1];
    strcpy(varname[i], (*v_iter)->getName().c_str());
    //if ((*v_iter)->getType() == Binary) {
      //vartype[i] = 'B';
    //} else if ((*v_iter)->getType() == Integer) {
      //vartype[i] = 'I';
    //}
  }

  //// XXX: check if linear function is NULL
  lin = problem->getObjective()->getLinearFunction();
  if (problem->getObjective()->getObjectiveType() == Minotaur::Maximize) {
    obj_sense = -1.;
  }

  i = 0;
  if (lin) {
    for (v_iter=problem->varsBegin(); v_iter!=problem->varsEnd(); ++v_iter, 
         ++i) {
      obj[i] = obj_sense*lin->getWeight(*v_iter);
    }
  } else {
    memset(obj, 0, numvars * sizeof(double));
  }

  cpxstatus_ = CPXXnewcols (cpxenv_, cpxlp_, numvars, obj, varlb, varub, NULL, varname);
  if (cpxstatus_)  goto TERMINATE;
  cpxstatus_ = CPXXaddrows (cpxenv_, cpxlp_, 0, numcons, nnz, conrhs, sense, start,
                       index, value, NULL, conname);
  if (cpxstatus_)  goto TERMINATE;
  
  /* Now change range values of Ranged constraints */
  conind = new int[rcnt];
  conrange = new double[rcnt];
  for (i = 0; i < rcnt; i++) {
    conind[i] = conindvec[i];
    conrange[i] = conrangevec[i];
  }
  cpxstatus_ = CPXXchgrngval (cpxenv_, cpxlp_, rcnt, conind, conrange);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to change range values of constraints."
       << std::endl;
     goto TERMINATE;
  }
  
  if (sol_) {
    delete sol_;
  }
  sol_ = (SolutionPtr) new Solution(1E20, 0, problem_);

TERMINATE:

  objChanged_ = true;
  bndChanged_ = true;
  consChanged_ = true;
  delete [] index;
  delete [] value;
  delete [] start;
  delete [] conlb;
  delete [] conub;
  delete [] conrhs;
  delete [] varlb;
  delete [] varub;
  delete [] obj;
  for (i=0; i < numvars; ++i) {
    if (varname[i]) {
      delete [] varname[i];
    }
  }
  delete [] varname;
  for (i=0; i < numcons; ++i) {
    if (conname[i]) {
      delete [] conname[i];
    }
  }
  delete [] conname;
  delete [] conind;
  delete [] conrange;
  delete [] sense;
  delete [] vartype;

  problem->setEngine(this);
}


//void CplexLPEngine::loadFromWarmStart(const WarmStartPtr ws)
//{
  //ConstCpxLPWarmStartPtr ws2 = dynamic_cast <const CpxLPWarmStart*> (ws);
  //assert (ws2);
  //int wsNumCons = ws2->getNumCons();
  //int probNumCons = problem_->getNumCons();

  //if (wsNumCons == probNumCons) {
    //cpxstatus_ = CPXXcopybase(cpxenv_, cpxlp_, ws2->getVarStat(), ws2->getConStat());
    //if (cpxstatus_) {
       //logger_->msgStream(LogError) << me_
         //<< "Failed to pass basis information for warm starting."
         //<< std::endl;
    //}
  //} else {
    //assert(!(probNumCons < wsNumCons));
    //int *constat = new int[probNumCons];
    //int *wsconstat = ws2->getConStat();
    //for (int i=0; i < wsNumCons; ++i) {
      //constat[i] = wsconstat[i];
    //}
    //for (int i=wsNumCons; i < probNumCons; ++i) {
      //constat[i] = 0; //assume dual/slack is nonbasic for new constraint
    //}
    //cpxstatus_ = CPXXcopybase(cpxenv_, cpxlp_, ws2->getVarStat(), constat);
    //if (cpxstatus_) {
       //logger_->msgStream(LogError) << me_
         //<< "Failed to pass extended basis information for warm starting."
         //<< std::endl;
    //}
    //delete [] constat;
  //}
//}


void CplexLPEngine::load_()
{
  assert(!"to be implemented!");
}


void CplexLPEngine::negateObj()
{
  assert(!"to be implemented!");
}


void CplexLPEngine::removeCons(std::vector<ConstraintPtr> &delcons)
{
  int *delinds = new int[CPXXgetnumrows(cpxenv_,cpxlp_)]{0};
  for (UInt i=0; i < delcons.size(); ++i) {
    delinds[delcons[i]->getIndex()] = 1;
  }
  cpxstatus_ = CPXXdelsetrows (cpxenv_, cpxlp_, delinds);
  if (cpxstatus_) {
    assert(!"error while deleting constraints!");
  }
  if (delinds) {
    delete [] delinds;
  }
  consChanged_ = true;
}


void CplexLPEngine::resetIterationLimit()
{
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Simplex_Limits_Iterations, maxIterLimit_);
}


void CplexLPEngine::setIterationLimit(int limit)
{
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Simplex_Limits_Iterations, limit);
}


int CplexLPEngine::setDualObjLimit(double val)
{
  dualObjLimit_ = val;
  return 0;
}
  

void CplexLPEngine::setTimeLimit(double timelimit)
{
  timeLimit_ = timelimit;
}
  

EngineStatus CplexLPEngine::solve()
{
  timer_->start();
  stats_->calls += 1;

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "in call number " << stats_->calls
                               << std::endl;
#endif

  int solstat;
  double objval;
  int cur_numrows = CPXXgetnumrows (cpxenv_, cpxlp_);
  int cur_numcols = CPXXgetnumcols (cpxenv_, cpxlp_);
  
  double *x = new double[cur_numcols];
  double *redCosts = new double[cur_numcols];
  double *dualOfCons = new double[cur_numrows];
  int *varstat = new int[cur_numcols];
  int *constat = new int[cur_numrows];

#if 0
  //problem_->write(std::cout, 9);
  /* Write a copy of the problem to a file. */
  writeLP();
#endif

  /* Optimize the problem and obtain solution. */
  if ((bndChanged_ || consChanged_) && (stats_->calls > 1)) {
    /* Because the problem is dual feasible with the rows added, using
      the dual simplex method is indicated.  */
    cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_LPMethod, CPX_ALG_DUAL);
    if (cpxstatus_) {
      logger_->msgStream(LogInfo) << me_
        << "Failed to set the optimization method, error." << std::endl;
    }
  }

  /* Set objective limits on LP solves (assume minimization) */
  cpxstatus_ = CPXXsetdblparam(cpxenv_, CPXPARAM_Simplex_Limits_UpperObj, dualObjLimit_);
  if (cpxstatus_) {
     logger_->msgStream(LogInfo) << me_ << "Failed to set dual objective limit."
       << std::endl;
  }

  // Solve the problem
  cpxstatus_ = CPXXlpopt (cpxenv_, cpxlp_);
  if (cpxstatus_) {
     logger_->msgStream(LogInfo) << me_ << "Failed to optimize LP." << std::endl;
  }

  // Access the solve information
  cpxstatus_ = CPXXsolution (cpxenv_, cpxlp_, &solstat, &objval, x, dualOfCons, NULL, redCosts);
  if (cpxstatus_) {
#if SPEW
     logger_->msgStream(LogInfo) << me_ << "Failed to obtain solution data." << std::endl;
#endif
     solstat = CPXXgetstat(cpxenv_, cpxlp_); //get solve status
  }

  // Solve status (replace with string later using CPXXgetstatstring(..))
  if (solstat == 1) {
    status_ = ProvenOptimal;
#if 0
    /* Print the solution. */
    printx(x, cur_numcols);
#endif
    sol_->setPrimal(x);
    sol_->setObjValue(objval + problem_->getObjective()->getConstant());
    sol_->setDualOfCons(dualOfCons);
    sol_->setDualOfVars(redCosts);

    // Store basis information for warm starting
    //cpxstatus_ = CPXXgetbase(cpxenv_, cpxlp_, varstat, constat);
    //ws_->setVarStat(varstat, cur_numcols);
    //ws_->setNumCons(cur_numrows);
    //ws_->setConStat(constat, cur_numrows);
  } else if (solstat == 3 || solstat == 5) {
    status_ = ProvenInfeasible;
    sol_->setObjValue(INFINITY);
    sol_->setDualOfCons(dualOfCons);
    sol_->setDualOfVars(redCosts);
  } else if(solstat == 2 || solstat == 20) {
    status_ = ProvenUnbounded;    // or it could be infeasible
    sol_->setObjValue(-INFINITY);
    sol_->setDualOfCons(dualOfCons);
    sol_->setDualOfVars(redCosts);
    //writeLP();
  } else if (solstat == 10) {
    status_ = EngineIterationLimit;
    sol_->setObjValue(INFINITY);
    sol_->setPrimal(x);
    sol_->setObjValue(objval + problem_->getObjective()->getConstant());
    sol_->setDualOfCons(dualOfCons);
    sol_->setDualOfVars(redCosts);
  } else if (solstat == 12) {
    status_ = ProvenObjectiveCutOff;
    sol_->setPrimal(x);
    sol_->setObjValue(objval + problem_->getObjective()->getConstant());
    sol_->setDualOfCons(dualOfCons);
    sol_->setDualOfVars(redCosts);
  } else {
    status_ = EngineUnknownStatus;
    sol_->setObjValue(INFINITY);
  }

  stats_->iters += CPXXgetitcnt(cpxenv_, cpxlp_);
  stats_->time  += timer_->query();

  if (strBr_) {
    ++(stats_->strCalls);
    stats_->strIters += CPXXgetitcnt(cpxenv_, cpxlp_);
    stats_->strTime  += timer_->query();
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "status = " << status_ << std::endl
                               << me_ << "solution value = "
                               << sol_->getObjValue() << std::endl
                               << me_ << "iterations = "
                               << CPXXgetitcnt(cpxenv_, cpxlp_) << std::endl;
#endif
  timer_->stop();
  bndChanged_ = false;
  consChanged_ = false;
  objChanged_ = false;

//TERMINATE:
  delete [] x;
  delete [] dualOfCons;
  delete [] redCosts;
  delete [] constat;
  delete [] varstat;
  return status_;
}


void CplexLPEngine::writeLP(const char *filename) const 
{
  int status = CPXXwriteprob (cpxenv_, cpxlp_, filename, NULL);
  if (status) {
   assert(!"Failed to write LP to disk.\n");
  }
}


void CplexLPEngine::writeLP()
{
  std::string filename = env_->getOptions()->findString("problem_file")->getValue() + ".lp";
  int status = CPXXwriteprob (cpxenv_, cpxlp_, filename.c_str(), NULL);
  if (status) {
   assert(!"Failed to write LP to disk.\n");
  }
}


void CplexLPEngine::writeStats(std::ostream &out) const
{
  if (stats_) {
    std::string me = "CplexLP: ";
    out << me << "total calls            = " << stats_->calls << std::endl
      << me << "strong branching calls = " << stats_->strCalls << std::endl
      << me << "total time in solving  = " << stats_->time  << std::endl
      << me << "time in str branching  = " << stats_->strTime << std::endl
      << me << "total iterations       = " << stats_->iters << std::endl
      << me << "strong br iterations   = " << stats_->strIters << std::endl;
  }
}


// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
