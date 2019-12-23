// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/**
 * \file CplexMILPEngine.cpp
 * \brief Implement an interface to the Cplex MILP solver
 * \author Prashant Palkar and Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "MinotaurConfig.h"
#include "CplexMILPEngine.h"
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

//#include <thread>
//#include <mutex>
//#include <chrono>

//std::mutex mtx;

using namespace Minotaur;

//#define SPEW 1

const std::string CplexMILPEngine::me_ = "CplexMILPEngine: ";

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CplexMILPEngine::CplexMILPEngine(EnvPtr env)
  : env_(env),
    sol_(0)
{
  cpxenv_ = 0;
  timer_ = env->getNewTimer();
  stats_ = new CplexMILPStats();
  stats_->calls    = 0;
  stats_->time     = 0;
  logger_ = env->getLogger();
  timeLimit_ = INFINITY;
  upperCutoff_ = INFINITY;
  writeMipStarts_ = env_->getOptions()->findBool("oa_use_mip_starts")->getValue();
  if (writeMipStarts_) {
    mipStartFile_ = env_->getOptions()->findString("problem_file")->getValue() + ".mst";
  }
}


CplexMILPEngine::~CplexMILPEngine()
{
  if (cpxlp_ != NULL) {
    cpxstatus_ = CPXXfreeprob(cpxenv_, &cpxlp_);
    if (cpxstatus_) {
      logger_->msgStream(LogError) << "Could not free CPLEX problem.\n";
    }
  }

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
  if (writeMipStarts_) {
    if (remove(mipStartFile_.c_str()) != 0) {
      logger_->msgStream(LogError) << "Could not remove mip starts file. \n";
    } else {
#if SPEW
      logger_->msgStream(LogError) << "Deleted mip starts file. \n";
#endif
    }
  }
}


void CplexMILPEngine::addConstraint(ConstraintPtr con)
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


void CplexMILPEngine::changeBound(ConstraintPtr, BoundType, double)
{
  assert(!"implement me!");
}


void CplexMILPEngine::changeBound(VariablePtr var, BoundType lu, double new_val)
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


void CplexMILPEngine::changeBound(VariablePtr, double, double)
{
  assert(!"implement me!");
}


void CplexMILPEngine::changeConstraint(ConstraintPtr, LinearFunctionPtr, 
                                   double, double)
{
  assert(!"implement me!");
}


void CplexMILPEngine::changeConstraint(ConstraintPtr, NonlinearFunctionPtr)
{
    assert(!"Cannot change a nonlinear function in CplexMILPEngine");
}


void CplexMILPEngine::changeObj(FunctionPtr, double)
{
  assert(!"implement me!");
}


void CplexMILPEngine::clear()
{
  assert(!"implement me!");
}


EnginePtr CplexMILPEngine::emptyCopy()
{
  return new CplexMILPEngine(env_);
}


std::string CplexMILPEngine::getName() const
{
  return "CplexMILP";
}


UInt CplexMILPEngine::getNumSols()
{
  return CPXXgetsolnpoolnumsolns(cpxenv_, cpxlp_);
}


double CplexMILPEngine::getSolutionValue() 
{
  return sol_->getObjValue();
}


ConstSolutionPtr CplexMILPEngine::getSolution() 
{
  return sol_;
}


ConstSolutionPtr CplexMILPEngine::getSolutionFromPool(int index)
{
  CPXDIM numcols = CPXXgetnumcols(cpxenv_, cpxlp_);
  double *x = new double[numcols];
  SolutionPtr sol = 0;
  double objval;
  cpxstatus_ = CPXXgetsolnpoolx (cpxenv_, cpxlp_, index, x, 0, numcols-1);
  if (cpxstatus_) {
    logger_->msgStream(LogError) << "Could not get solution from CPLEX \n";
    return sol;
  }
  cpxstatus_  = CPXXgetsolnpoolobjval(cpxenv_, cpxlp_, index, &objval);
  if (cpxstatus_) {
    logger_->msgStream(LogError)
      << "Could not get objective value of solution from CPLEX \n";
    return sol;
  }
  sol = (SolutionPtr) new Solution(1E20, 0, problem_);
  sol->setPrimal(x);
  sol->setObjValue(objval);
  return sol;
}


EngineStatus CplexMILPEngine::getStatus() 
{
  return status_;
}

void CplexMILPEngine::printx(double *x, UInt size) {
  for (UInt i=0; i < size; i++) {
    logger_->msgStream(LogInfo) << i+1 << " " << x[i] << "\n";
  }
}

void CplexMILPEngine::load(ProblemPtr problem)
{
  objChanged_ = true;
  bndChanged_ = true;
  consChanged_ = true;
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
  int i, j, rcnt;
  double obj_sense = 1, ztol = 1e-6;
  double *conlb = 0, *conub = 0, *conrhs = 0, *conrange = 0, *varlb = 0, *varub = 0, *obj = 0;
  double *value = 0;
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
  std::fill(vartype, vartype+numvars, 'C'); //assuming all vars continuous
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

  /* Turn on output to te screen (use a file to read parameters LATER!) */
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_ScreenOutput, CPX_ON);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to turn on screen indicator, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
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
    if ((*v_iter)->getType() == Binary) {
      vartype[i] = 'B';
    } else if ((*v_iter)->getType() == Integer) {
      vartype[i] = 'I';
    }
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

  cpxstatus_ = CPXXnewcols (cpxenv_, cpxlp_, numvars, obj, varlb, varub, vartype, varname);
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

  //problem->setEngine(this);

  //if (cpxenv_ != NULL) {
    //cpxstatus_ = CPXXcloseCPLEX (&cpxenv_);
    //if (cpxstatus_) {
      //logger_->msgStream(LogError) << "Could not close CPLEX environment.\n";
    //}
  //}
}


void CplexMILPEngine::load_()
{
}



void CplexMILPEngine::negateObj()
{
}


void CplexMILPEngine::removeCons(std::vector<ConstraintPtr> &)
{
  consChanged_ = true;
}


void CplexMILPEngine::resetIterationLimit()
{
}


void CplexMILPEngine::setIterationLimit(int)
{
}
  

void CplexMILPEngine::setTimeLimit(double timelimit)
{
  timeLimit_ = timelimit;
}


void CplexMILPEngine::setUpperCutoff(double cutoff)
{
  upperCutoff_ = cutoff;
}
  

EngineStatus CplexMILPEngine::solve()
{
  stats_->calls += 1;
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "in call number " << stats_->calls
                               << std::endl;
#endif
  int solstat;
  double objval = INFINITY;
  double cpxtimeStart = 0, cpxtimeEnd = 0;
  int cur_numcols = CPXXgetnumcols (cpxenv_, cpxlp_);
  double *x = new double[cur_numcols];
  std::ifstream mstfile;

#if 0
  /* Write a copy of the problem to a file. */
  problem_->write(std::cout);
  writeLP();
#endif

  /* Set time limit (wallclock) for this iteration */
  cpxstatus_ = CPXXsetdblparam (cpxenv_, CPXPARAM_TimeLimit, timeLimit_);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set wall time limit, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }
  
  /* Set upper cutoff (best solution value) for this iteration */
  if (upperCutoff_ < INFINITY) {
    cpxstatus_ = CPXXsetdblparam (cpxenv_, CPXPARAM_MIP_Tolerances_UpperCutoff,
                                  upperCutoff_);
    if (cpxstatus_) {
       logger_->msgStream(LogError) << me_ << "Failure to set upper cutoff, error "
         << cpxstatus_ << std::endl;
       goto TERMINATE;
    }
  }

  /* Set number of threads (default 1) */
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Threads,
                                env_->getOptions()->findInt("threads")->getValue());
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set number of threads, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }

  // Read MIP starts if they exist (in a file)
  if (writeMipStarts_) {
    mstfile.open(mipStartFile_.c_str());
    if (!mstfile.fail()) {
      cpxstatus_ = CPXXreadcopymipstarts(cpxenv_, cpxlp_, mipStartFile_.c_str());
      if (cpxstatus_) {
        logger_->msgStream(LogError) << me_ << "Failed to read MIP starts."
          << std::endl;
      }
      mstfile.close();
    }
  }

  /* Initialize Cplex time stamp */
  cpxstatus_ = CPXXgettime(cpxenv_, &cpxtimeStart);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to get CPLEX time stamp." << std::endl;
  }

  /* Optimize the problem and obtain solution. */
  cpxstatus_ = CPXXmipopt (cpxenv_, cpxlp_);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to optimize MILP." << std::endl;
     //goto TERMINATE;
  }

  /* Obtain Cplex time stamp after solve */
  cpxstatus_ = CPXXgettime(cpxenv_, &cpxtimeEnd);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to get CPLEX time stamp." << std::endl;
  }

  // Access the solve information
  cpxstatus_ = CPXXsolution (cpxenv_, cpxlp_, &solstat, &objval, x, NULL, NULL, NULL);
  if (cpxstatus_) {
     logger_->msgStream(LogInfo) << me_ << "Failed to obtain solution data." << std::endl;
     solstat = CPXXgetstat(cpxenv_, cpxlp_); //get solve status
  }

  // Write MIP start for the next iteration
  if (writeMipStarts_) {
    cpxstatus_ = CPXXwritemipstarts (cpxenv_, cpxlp_, mipStartFile_.c_str(), 0, 0);
    if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to write MIP starts."
       << std::endl;
    }
  }

  logger_->msgStream(LogInfo) << me_ << "status = " << solstat << std::endl
                               << me_ << "solution value = "
                               << objval << std::endl
                               << me_ << "nodes processed = "
                               << CPXXgetnodecnt(cpxenv_, cpxlp_) << std::endl;
#if SPEW
  /* Write the solution. */
  printx(x, cur_numcols);
#endif

  // Solve status (replace with string later using CPXXgetstatstring(..))
  if (solstat == 101 || solstat == 102) {
    status_ = ProvenOptimal;
    sol_->setPrimal(x);
    sol_->setObjValue(objval); 
  } else if (solstat == 103 || solstat == 119) {
    status_ = ProvenInfeasible;
    sol_->setObjValue(INFINITY);
  } else if(solstat == 118 || solstat == 119) {
    status_ = ProvenUnbounded;    // or it could be infeasible
    sol_->setObjValue(-INFINITY);
  } else if (solstat == 107 || solstat == 108 ) {
    status_ = EngineIterationLimit;
    if (solstat == 108) {
      sol_->setObjValue(INFINITY);
    } else {
      sol_->setPrimal(x);
      sol_->setObjValue(objval);
    }
  } else {
    status_ = EngineUnknownStatus;
    sol_->setObjValue(INFINITY);
    logger_->msgStream(LogInfo) << " unknown \n";
  }
  stats_->time += cpxtimeEnd - cpxtimeStart;

TERMINATE:
  delete [] x;
  return status_;
}


//static int CPXPUBLIC minoincumbentcallback(CPXCENVptr env, void *cbdata, int wherefrom,
             //void *userdata, double objval, double *x, int *isfeas_p, int *useraction_p)
//{  
  //mtx.lock();

  //STOAHandlerPtr stoaH = *(STOAHandlerPtr *)(userdata);
  //int cpxstatus = 0;
  //int numvars = stoaH->getRel()->getNumVars();
  ////double *cpxx = new double[numvars]; 
  //double ub;
  //const double *nlpx; 
 
//#if PRINT
  //std::cout << "\nIn incumCB: at x =\n";
  //printx(x, numvars);
//#endif
   
  //cpxstatus = CPXXgetcallbacknodeobjval (env, cbdata, wherefrom, &ub);
  //if (cpxstatus) {
    //std::cout << "Can't get node objective value.";
    //return cpxstatus;
  //}
 
  //*isfeas_p = 0;
  //if (stoaH->fixedNLP(x, nlpx)) {
    ////nlpx = stoaH->getFixedNLPSol();
    ////std::copy(nlpx, nlpx + numvars, x); //not happening
    //if (stoaH->isFeas(x)) {
      //*isfeas_p = 1;
    //}
  //}
  //*useraction_p = CPX_CALLBACK_SET;

  //mtx.unlock();
  //return 0;
//}


static int CPXPUBLIC minolazycallback(CPXCENVptr env, void *cbdata, int wherefrom,
             void *userdata, int *useraction_p)
{  
  //mtx.lock();
  double timeStart, timeEnd = 0;
  STOAHandlerPtr stoaH = *(STOAHandlerPtr *)(userdata);
  int numvars = stoaH->getRel()->getNumVars();
  double *cpxx = new double[numvars]; 
  double ub;
  
  int cpxstatus = CPXXgettime(env, &timeStart);
  if (cpxstatus) {
    std::cout << "Can't get start time stamp from Cplex.";
    return cpxstatus;
  }

  cpxstatus = CPXXgetcallbacknodeobjval (env, cbdata, wherefrom, &ub);
  if (cpxstatus) {
    std::cout << "Can't get node objective value.";
    return cpxstatus;
  }
 
  cpxstatus = CPXXgetcallbacknodex(env, cbdata, wherefrom, cpxx, 0, numvars - 1);
  if (cpxstatus != 0) {
    //mtx.unlock();
    return cpxstatus;
  }

  const double *x = cpxx;
  //if (!(stoaH->isFeasible(x))) {
    EnginePtr nlpe = 0;
    bool fixNlp = stoaH->fixedNLP(x, nlpe);
    if (fixNlp) {
      UInt vIdx;
      double rhs;
      std::vector<UInt> varIdx;
      std::vector<double> varCoeff;
      for (ConstraintConstIterator it = stoaH->consBegin();
           it != stoaH->consEnd(); ++it) {
        stoaH->OACutToCons(x, *it, &rhs, &varIdx, &varCoeff, nlpe);
        vIdx = varIdx.size();
        if (varIdx.size() > 0) {
          CPXDIM *cutind = new CPXDIM[vIdx];
          double *cutval = new double[vIdx];
          for (UInt i=0; i<vIdx; ++i) {
            cutind[i] = varIdx[i];
            cutval[i] = varCoeff[i];
          }
          cpxstatus = CPXXcutcallbackadd(env, cbdata, wherefrom, vIdx, rhs, 'L',
                                         cutind, cutval, CPX_USECUT_FORCE);
          if (cpxstatus != 0) {
            //mtx.unlock();
            return cpxstatus;
          }
          varIdx.clear();
          varCoeff.clear();
          delete [] cutind;
          delete [] cutval;
        }
      }
      stoaH->OACutToObj(x, &rhs, &varIdx, &varCoeff, ub, nlpe);
      vIdx = varIdx.size();
      if (varIdx.size() > 0) {
         CPXDIM *cutind = new CPXDIM[vIdx];
         double *cutval = new double[vIdx];
        for (UInt i=0; i<vIdx; ++i) {
          cutind[i] = varIdx[i];
          cutval[i] = varCoeff[i];
        }
        cpxstatus = CPXXcutcallbackadd(env, cbdata, wherefrom, vIdx, rhs, 'L',
                                       cutind, cutval, CPX_USECUT_FORCE);
        if (cpxstatus != 0) {
          cpxstatus = CPXXgettime(env, &timeEnd);
          if (cpxstatus) {
            std::cout << "Can't get end time stamp from Cplex.";
            return cpxstatus;
          }
          stoaH->setCbTime(stoaH->getCbTime() + timeEnd - timeStart);
          //mtx.unlock();
          return cpxstatus;
        }
        varIdx.clear();
        varCoeff.clear();
        delete [] cutind;
        delete [] cutval;
        
        if (cpxstatus != 0) {
          cpxstatus = CPXXgettime(env, &timeEnd);
          if (cpxstatus) {
            std::cout << "Can't get end time stamp from Cplex.";
            return cpxstatus;
          }
          stoaH->setCbTime(stoaH->getCbTime() + timeEnd - timeStart);
          //mtx.unlock();
          return cpxstatus;
        }
        *useraction_p = CPX_CALLBACK_SET;
      }
    }
    nlpe = 0;
  //}
  delete [] cpxx;
  cpxstatus = CPXXgettime(env, &timeEnd);
  if (cpxstatus) {
    std::cout << "Can't get end time stamp from Cplex.";
    return cpxstatus;
  }
  stoaH->setCbTime(stoaH->getCbTime() + timeEnd - timeStart);
  //mtx.unlock();
  return 0;
}

static int CPXPUBLIC minocallback (CPXCALLBACKCONTEXTptr context, CPXLONG where, void* userdata)
{  
  //assert(where==CPX_CALLBACKCONTEXT_CANDIDATE);
  //mtx.lock();
//#if SPEW
  ////logger_->msgStream(LogInfo) << "thread " << std::this_thread::get_id() << "\n";
//#endif
  //STOAHandlerPtr stoaH = *(STOAHandlerPtr *)(userdata);
  //int cpxstatus = 0;
  //int numvars = stoaH->getRel()->getNumVars();
  //double *cpxx = new double[numvars];
  //double ub;
  //const char* sense = "L";
  //CPXNNZ const rmatbeg =0;
  
  //CPXXcallbackgetcandidatepoint(context, cpxx, 0, numvars - 1, &ub);
  //if (cpxstatus != 0)
     //return cpxstatus;
  
  //const double *x = cpxx; //is this correct?
  //double newUb;
  ////if (!(stoaH->isFeasible(x))) {
    //if (stoaH->fixedNLP(x)) {
      //UInt vIdx;
      //double rhs;
      //std::vector<UInt> varIdx;
      //std::vector<double> varCoeff;
      //for (ConstraintConstIterator it = stoaH->consBegin();
           //it != stoaH->consEnd(); ++it) {
        //stoaH->OACutToCons(x, *it, &rhs, &varIdx, &varCoeff);
        //vIdx = varIdx.size();
        //if (varIdx.size() > 0) {
          //CPXDIM *cutind = new CPXDIM[vIdx];
          //double *cutval = new double[vIdx];
          //for (UInt i=0; i<vIdx; ++i) {
            //cutind[i] = varIdx[i];
            //cutval[i] = varCoeff[i];
          //}
          //const double* objrhs = &rhs;
          
          //cpxstatus = CPXXcallbackrejectcandidate(context, 1, vIdx, objrhs,
                                                  //sense, &rmatbeg, cutind, cutval);
          //if (cpxstatus != 0)
            //return cpxstatus;
          //varIdx.clear();
          //varCoeff.clear();
          //delete [] cutind;
          //delete [] cutval;
        //}
      //}
      //stoaH->OACutToObj(x, &rhs, &varIdx, &varCoeff, ub);
      //vIdx = varIdx.size();
      //if (varIdx.size() > 0) {
         //CPXDIM *cutind = new CPXDIM[vIdx];
         //double *cutval = new double[vIdx];
        //for (UInt i=0; i<vIdx; ++i) {
          //cutind[i] = varIdx[i];
          //cutval[i] = varCoeff[i];
        //}
        //const double* conrhs = &rhs;
        //cpxstatus = CPXXcallbackrejectcandidate(context, 1, vIdx, conrhs, sense,
                                                //&rmatbeg, cutind, cutval);
        //if (cpxstatus != 0) {
          //return cpxstatus;
        //}
        //varIdx.clear();
        //varCoeff.clear();
        //delete [] cutind;
        //delete [] cutval;
      //}

      //std::vector<UInt> solVarIdx;
      //std::vector<double> solVarVal;
      ////// either new ub found (fixed NLP feasible) otherwise infinite
      //newUb = stoaH->newUb(&solVarIdx, &solVarVal);
      //if (newUb != INFINITY) {
        //cpxstatus = CPXXcallbackgetincumbent(context, cpxx, 0, numvars-1, &ub);
        //if (ub > newUb) {
          ////send this solution to cplex
          //CPXDIM *solind = new CPXDIM[numvars];
          //double *solval = new double[numvars];
          //for (int i=0; i<numvars; ++i) {
            //solind[i] = solVarIdx[i];
            //solval[i] = solVarVal[i];
          //}
          //cpxstatus = CPXXcallbackpostheursoln(context, numvars, solind, solval, newUb,
                                             //CPXCALLBACKSOLUTION_CHECKFEAS );
          //if (cpxstatus != 0 ) {
            //delete [] solind;
            //delete [] solval;
            //return cpxstatus;
          //}
          //delete [] solind;
          //delete [] solval;
        //}
      //}
    //}
  ////}
  //delete [] cpxx;
  
  //mtx.unlock();
  
  return 0;
}


EngineStatus CplexMILPEngine::solveST(double *objLb, SolutionPtr* sol,
                                      STOAHandlerPtr stoa_hand,
                                      SolveStatus* solveStatus)
{
  int solstat;
  int cur_numcols = CPXXgetnumcols (cpxenv_, cpxlp_);
  double *x = new double[cur_numcols];
  double objval;

  CPXLONG where = 0;
  //int numvars = problem_->getNumVars();
  char errbuf[CPXMESSAGEBUFSIZE];
  //char *varname[numvars];
  VariableConstIterator v_iter;
  //char *cstr; //temporary
  //int i = 0;
  //for (v_iter=problem_->varsBegin(); v_iter!=problem_->varsEnd(); ++v_iter, 
       //++i) {
    //varlb[i] = (*v_iter)->getLb();
    //varub[i] = (*v_iter)->getUb();
    //cstr = new char[(*v_iter)->getName().length() + 1];
    //strcpy(cstr, (*v_iter)->getName().c_str());
    //varname[i] = cstr;
    //if ((*v_iter)->getType() == Binary) {
      //vartype[i] = 'B';
    //} else if ((*v_iter)->getType() == Integer) {
      //vartype[i] = 'I';
    //}
  //}

  /* Setup callbacks. We disable CPXPARAM_MIP_Strategy_CallbackReducedLP so
  * that indices in the callbacks can refer to the original model and we
  * don't have to translate the indices to the presolved model.
  * This also requires us to disable non-linear reductions so that cuts and
  * lazy constraints can always be crushed.
  */
  //cpxstatus_ = CPXXsetintparam(cpxenv_, CPXPARAM_MIP_Strategy_CallbackReducedLP, CPX_OFF);
  //if (cpxstatus_ != 0) {
     //fprintf(stderr, "Failed to disable reduced LP in callbacks: %s\n",
             //CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf));
     //goto TERMINATE;
  //}
  //cpxstatus_ = CPXXsetintparam(cpxenv_, CPXPARAM_Preprocessing_Linear, 0);
  //if (cpxstatus_ != 0 ) {
     //fprintf(stderr, "Failed to disable dual reductions: %s\n",
             //CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf));
     //goto TERMINATE;
  //}

  /* Set time limit (wallclock) */
  cpxstatus_ = CPXXsetdblparam (cpxenv_, CPXPARAM_TimeLimit,
                                env_->getOptions()->findDouble("bnb_time_limit")->getValue());
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set wall time limit, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }
  
  /* Set number of threads (default 1) */
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Threads,
                                env_->getOptions()->findInt("threads")->getValue());

  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set number of threads, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }

  where |= CPX_CALLBACKCONTEXT_CANDIDATE;

  cpxstatus_ = CPXXcallbacksetfunc(cpxenv_, cpxlp_, where, minocallback, (void*) &stoa_hand);
 
  if (cpxstatus_ != 0) {
     fprintf(stderr, "Failed to add callback: %s\n",
             CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf));
     goto TERMINATE;
  }

  /* Optimize the problem and obtain solution. */
  cpxstatus_ = CPXXmipopt (cpxenv_, cpxlp_);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to optimize MILP."
       << CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf) << std::endl;
     goto TERMINATE;
  }
  solstat = CPXXgetstat (cpxenv_, cpxlp_);

  /* Write the output to the screen. */
  logger_->msgStream(LogInfo) << me_ << "Solution status = "
    << CPXXgetstatstring(cpxenv_, solstat, errbuf) << std::endl;

  /* Get the (dual) best bound value. */
  cpxstatus_ = CPXXgetbestobjval(cpxenv_, cpxlp_, objLb);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_
       << "No MILP objective value available. Exiting..." << std::endl;
  }

  /* Get the (primal) objective value. */
  cpxstatus_ = CPXXgetobjval (cpxenv_, cpxlp_, &objval);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_
       << "No MILP objective value available. Exiting..." << std::endl;
  }
  //logger_->msgStream(LogInfo) << me_ << "Solution value = " << objval << std::endl;

  /* Get the solution. */
  cpxstatus_ = CPXXgetx (cpxenv_, cpxlp_, x, 0, cur_numcols-1);
  if (cpxstatus_) {
   logger_->msgStream(LogError) << me_ << "Failed to get optimal integer x."
     << std::endl;
   goto TERMINATE;
  }
  
#if SPEW
  /* Write the solution. */
  //for (int j = 0; j < cur_numcols; j++) {
   //printf ("Column %d:  Value = %10f\n", j, x[j]);
   //logger_->msgStream(LogInfo) << varname[j] << ": " << x[j] << std::endl;
  //}
#endif

  // Solve status (replace with string later using CPXXgetstatstring(..))
  if (solstat == 101 || solstat == 102) {
    status_ = ProvenOptimal;
    *solveStatus = SolvedOptimal;
    sol_->setPrimal(x);
    sol_->setObjValue(objval);
    *sol = sol_;
  } else if (solstat == 103) {
    status_ = ProvenInfeasible;
    *solveStatus = SolvedInfeasible;
    sol_->setObjValue(INFINITY);
  } else if(solstat == 118 || solstat == 119) {
    status_ = ProvenUnbounded;    // or it could be infeasible?
    *solveStatus = SolvedUnbounded;
    sol_->setObjValue(-INFINITY);
  //} else if(model->isProvenDualInfeasible()) {
    //status_ = ProvenUnbounded;    // primal is not infeasible but dual is.
    //sol_->setObjValue(-INFINITY);
    //std::cout << " dual inf \n";
  } else if (solstat == 107 || solstat == 108 ) {
    status_ = EngineIterationLimit;
    *solveStatus = TimeLimitReached;
    if (solstat == 108) {
      sol_->setObjValue(INFINITY);
    } else {
      sol_->setPrimal(x);
      sol_->setObjValue(objval);
    }
  //} else if (model->isNodeLimitReached() || model->isSecondsLimitReached() ||
             //model->isSolutionLimitReached()) {
    //status_ = EngineIterationLimit;
    //sol_->setPrimal(osilp_->getStrictColSolution());
    //sol_->setObjValue(osilp_->getObjValue()
        //+problem_->getObjective()->getConstant());
    //std::cout << " limit \n";
  //} else if(model->isAbandoned()) {
    //status_ = EngineError;
    //sol_->setObjValue(INFINITY);
    //std::cout << " abandoned \n";
  } else {
    status_ = EngineUnknownStatus;
    *solveStatus = SolveError;
    sol_->setObjValue(INFINITY);
    //std::cout << " unknown \n";
  }

  //stats_->time  += timer_->query();

//TERMINATE:

   ////Free up the problem as allocated by CPXXcreateprob, if necessary
  //if ( cpxlp_ != NULL ) {
     //cpxstatus_ = CPXXfreeprob (cpxenv_, &cpxlp_);
     //if (cpxstatus_) {
        //logger_->msgStream(LogError) << me_ << "CPXXfreeprob failed, error code "
        //  << cpxstatus_ << std::endl;

     //}
  //}

  ////Free up the CPLEX environment, if necessary
  //if ( cpxenv_ != NULL ) {
     //cpxstatus_ = CPXXcloseCPLEX (&cpxenv_);

     //[> Note that CPXXcloseCPLEX produces no output,
        //so the only way to see the cause of the error is to use
        //CPXXgeterrorstring.  For other CPLEX routines, the errors will
        //be seen if the CPXPARAM_ScreenOutput indicator is set to CPX_ON. */

     //if (cpxstatus_) {
        //char  errmsg[CPXMESSAGEBUFSIZE];
        //logger_->msgStream(LogError) << me_
        //  << "Could not close CPLEX environment." << std::endl;
        //CPXXgeterrorstring (cpxenv_, cpxstatus_, errmsg);
        //logger_->msgStream(LogError) << me_ << stderr << std::endl;
     //}
  //}
  
TERMINATE:
  //delete [] cstr;
  return status_;
}


EngineStatus CplexMILPEngine::solveSTLazy(double *objLb, SolutionPtr* sol,
                                      STOAHandlerPtr stoa_hand,
                                      SolveStatus* solveStatus)
{
  stats_->calls += 1;
  writeMipStarts_ = false;
  int solstat;
  int cur_numcols = CPXXgetnumcols (cpxenv_, cpxlp_);
  double *x = new double[cur_numcols];
  double objval = INFINITY;
  double cpxtimeStart = 0, cpxtimeEnd = 0;

  char errbuf[CPXMESSAGEBUFSIZE];
  VariableConstIterator v_iter;

  /* Setup callbacks. We disable CPXPARAM_MIP_Strategy_CallbackReducedLP so
  * that indices in the callbacks can refer to the original model and we
  * don't have to translate the indices to the presolved model.
  * This also requires us to disable non-linear reductions so that cuts and
  * lazy constraints can always be crushed.
  */
  cpxstatus_ = CPXXsetintparam(cpxenv_, CPXPARAM_MIP_Strategy_CallbackReducedLP, CPX_OFF);
  if (cpxstatus_ != 0) {
     fprintf(stderr, "Failed to disable reduced LP in callbacks: %s\n",
             CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf));
     goto TERMINATE;
  }
  cpxstatus_ = CPXXsetintparam(cpxenv_, CPXPARAM_Preprocessing_Linear, 0);
  if (cpxstatus_ != 0) {
     fprintf(stderr, "Failed to disable dual reductions: %s\n",
             CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf));
     goto TERMINATE;
  }

  /* Set time limit (wallclock) */
  cpxstatus_ = CPXXsetdblparam (cpxenv_, CPXPARAM_TimeLimit,
                                env_->getOptions()->findDouble("bnb_time_limit")->getValue());
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set wall time limit, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }
  
  /* Set number of threads (default 1) */
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Threads,
                                env_->getOptions()->findInt("threads")->getValue());

  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set number of threads, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }

  /* Set parallel search mode (default deterministic with callbacks) */
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_Parallel, CPX_PARALLEL_OPPORTUNISTIC);

  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set parallel mode, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }

#if SPEW
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_MIP_Interval, 1);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set MILP log interval, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_MIP_Display, 4);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failure to set MILP display parameter, error "
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }
#endif
  
  cpxstatus_ = CPXXsetlazyconstraintcallbackfunc(cpxenv_, minolazycallback,
                                             (void*) &stoa_hand);

  if (cpxstatus_ != 0) {
     fprintf(stderr, "Failed to add callback: %s\n",
             CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf));
     goto TERMINATE;
  }

  /* Initialize Cplex time stamp */
  cpxstatus_ = CPXXgettime(cpxenv_, &cpxtimeStart);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to get CPLEX time stamp." << std::endl;
  }

  /* Optimize the problem and obtain solution. */
  cpxstatus_ = CPXXmipopt (cpxenv_, cpxlp_);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to optimize MILP."
       << CPXXgeterrorstring(cpxenv_, cpxstatus_, errbuf) << std::endl;
     goto TERMINATE;
  }
  solstat = CPXXgetstat (cpxenv_, cpxlp_);

  /* Obtain Cplex time stamp after solve */
  cpxstatus_ = CPXXgettime(cpxenv_, &cpxtimeEnd);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_ << "Failed to get CPLEX time stamp." << std::endl;
  }

  /* Write the output to the screen. */
  logger_->msgStream(LogInfo) << me_ << "Solution status = "
    << CPXXgetstatstring(cpxenv_, solstat, errbuf) << std::endl;

  /* Get the (dual) best bound value. */
  cpxstatus_ = CPXXgetbestobjval(cpxenv_, cpxlp_, objLb);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_
       << "No MILP objective value available. Exiting..." << std::endl;
  }

  /* Get the (primal) objective value. */
  cpxstatus_ = CPXXgetobjval (cpxenv_, cpxlp_, &objval);
  if (cpxstatus_) {
     logger_->msgStream(LogError) << me_
       << "No MILP objective value available. Exiting..." << std::endl;
  }
  //logger_->msgStream(LogInfo) << me_ << "Solution value = " << objval << std::endl;

  /* Get the solution. */
  cpxstatus_ = CPXXgetx (cpxenv_, cpxlp_, x, 0, cur_numcols-1);
  if (cpxstatus_) {
   logger_->msgStream(LogError) << me_ << "Failed to get optimal integer x."
     << std::endl;
  }
 
#if 0
  /* Write a copy of the final problem to a file. */
  writeLP("minoCpxTerm.lp"); 
 
  /* Write the solution. */
  printx(x, cur_numcols);
#endif

  logger_->msgStream(LogInfo) << me_ << "status = " << solstat << std::endl
                               << me_ << "solution value = "
                               << objval << std::endl
                               << me_ << "nodes processed = "
                               << CPXXgetnodecnt(cpxenv_, cpxlp_)
                               << std::endl
                               << me_ << "time in callbacks = "
                               << stoa_hand->getCbTime()
                               << std::endl;

  // Solve status (replace with string later using CPXXgetstatstring(..))
  if (solstat == 101 || solstat == 102) {
    status_ = ProvenOptimal;
    *solveStatus = SolvedOptimal;
    sol_->setPrimal(x);
    sol_->setObjValue(objval);
    *sol = sol_;
  } else if (solstat == 103) {
    status_ = ProvenInfeasible;
    *solveStatus = SolvedInfeasible;
    sol_->setObjValue(INFINITY);
    *objLb = -INFINITY;
  } else if(solstat == 118 || solstat == 119) {
    status_ = ProvenUnbounded;    // or it could be infeasible?
    *solveStatus = SolvedUnbounded;
    sol_->setObjValue(-INFINITY);
    *objLb = -INFINITY;
  } else if (solstat == 107 || solstat == 108 ) {
    status_ = EngineIterationLimit;
    *solveStatus = TimeLimitReached;
    if (solstat == 108) {
      sol_->setObjValue(INFINITY);
    } else {
      sol_->setPrimal(x);
      sol_->setObjValue(objval);
    }
  } else {
    status_ = EngineUnknownStatus;
    *solveStatus = SolveError;
    sol_->setObjValue(INFINITY);
    logger_->msgStream(LogInfo) << " unknown \n";
  }
  stats_->time += cpxtimeEnd - cpxtimeStart;

TERMINATE:
  delete [] x;
  return status_;
}


void CplexMILPEngine::free_(char **ptr)
{
   if ( *ptr != NULL ) {
      free (*ptr);
      *ptr = NULL;
   }
} /* END free_and_null */


void CplexMILPEngine::writeLP(const char *filename) const 
{
  int status = CPXXwriteprob (cpxenv_, cpxlp_, filename, NULL);
  if (status) {
   assert(!"Failed to write LP to disk.\n");
  }
}

void CplexMILPEngine::writeLP()
{
  std::string filename = env_->getOptions()->findString("problem_file")->getValue() + ".lp";
  int status = CPXXwriteprob (cpxenv_, cpxlp_, filename.c_str(), NULL);
  if (status) {
   assert(!"Failed to write LP to disk.\n");
  }
}


void CplexMILPEngine::writeStats(std::ostream &out) const
{
  if (stats_) {
    out << me_ << "total calls            = " << stats_->calls << std::endl
      << me_ << std::setprecision(2) << "total time in solving  = "
      << stats_->time  << std::endl;
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
