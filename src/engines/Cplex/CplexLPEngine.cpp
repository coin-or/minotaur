// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/**
 * \file CplexLPEngine.cpp
 * \brief Implement an interface to the Cplex LP solver
 * \author Prashant Palkar and Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <iomanip>

//#include "coin/CoinPragma.hpp"
//#include "coin/CbcModel.hpp"
//#include "coin/OsiClpSolverInterface.hpp"

// #undef F77_FUNC_
// #undef F77_FUNC

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

CplexLPEngine::CplexLPEngine(EnvPtr env)
  : env_(env)
{
  timer_ = env->getNewTimer();
  stats_ = new CplexLPStats();
  stats_->calls    = 0;
  stats_->time     = 0;
  logger_ = env->getLogger();
  timeLimit_ = INFINITY;
  upperCutoff_ = INFINITY;
}


CplexLPEngine::~CplexLPEngine()
{
  delete stats_;
  delete timer_;
  if (problem_) {
    problem_->unsetEngine();
    problem_.reset();
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
  char* cstr;
  cstr = new char[con->getName().length() + 1];
  strcpy(cstr, con->getName().c_str());
  conname[0] = cstr;
  char* sense = new char;
  *sense = 'L';
  CPXNNZ *start = new CPXNNZ[1];
  //start[0] = CPXXgetnumnz (cpxenv_, cpxlp_);
  start[0] = 0;

  VariableGroupConstIterator it;
  int i=0;

  for (it = lf->termsBegin(); it != lf->termsEnd(); ++it, ++i){
    cols[i] = it->first->getIndex();
    elems[i] = it->second;
  }

  //osilp_->addRow(nz, cols, elems, con->getLb(), con->getUb());
  cpxstatus_ = CPXXaddrows (cpxenv_, cpxlp_, 0, 1, nnz, conrhs, sense, start,
                       cols, elems, NULL, conname);
  if (cpxstatus_) {
    assert(!"unable to add constraint!");
  }
  delete [] cols;
  delete [] elems;
  delete conrhs;
  delete [] cstr;
  delete [] conname;
  delete sense;
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


EnginePtr CplexLPEngine::emptyCopy()
{
  return new CplexLPEngine(env_);
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

void CplexLPEngine::printx(double *x, UInt size) {
  for (UInt i=0; i < size; i++) {
    logger_->msgStream(LogInfo) <<  i+1 << " " << x[i] << "\n";
  }
}

void CplexLPEngine::load(ProblemPtr problem)
{
  objChanged_ = true;
  bndChanged_ = true;
  consChanged_ = true;
  problem_ = problem;
  
  // Cplex objects
  int numvars = problem->getNumVars();
  int numcons = problem->getNumCons();
  int i, j, rcnt;
  double obj_sense = 1, ztol = 1e-6;
  double *conlb, *conub, *conrhs, *conrange, *varlb, *varub, *obj;
  double *value;
  //int *index;
  CPXDIM *index, *conind;
  //int *start = new int[numcons];
  CPXNNZ *start = new CPXNNZ[numcons];
  memset(start, 0, numcons*sizeof(int));
  //char *varname = new char[numvars];
  char **varname = new char*[numvars];
  //char *conname[numcons];
  char **conname = new char*[numcons];
  char *cstr; //temporary

  std::vector<UInt> conindvec;
  std::vector<double> conrangevec; 

  ConstraintConstIterator c_iter;
  VariableConstIterator v_iter;
  
  conlb = new double[numcons];
  conub = new double[numcons];
  conrhs = new double[numcons];
  varlb = new double[numvars];
  varub = new double[numvars];
  //double *rhs = new double[numcons];
  char *sense = new char[numcons];
  std::fill(sense, sense+numcons, 'L'); //assuming all cons <= to start with
  //char *vartype = new char[numvars];
  //std::fill(vartype, vartype+numvars, 'C'); //assuming all vars continuous
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

  if ( cpxenv_ == NULL ) {
     char  errmsg[CPXMESSAGEBUFSIZE];
     logger_->msgStream(LogInfo) << me_ << "Could not open CPLEX environment."
       << std::endl;
     CPXXgeterrorstring (cpxenv_, cpxstatus_, errmsg);
     logger_->msgStream(LogInfo) << me_ << errmsg << std::endl;
     goto TERMINATE;
  }

#if SPEW
  /* Turn on output to the screen (use a file to read parameters LATER!) */
  cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_ScreenOutput, CPX_ON);
  if ( cpxstatus_ ) {
     logger_->msgStream(LogInfo) << me_ << "Failure to turn on screen indicator, error " 
       << cpxstatus_ << std::endl;
     goto TERMINATE;
  }
#endif

  /* Create the problem. */
  cpxlp_ = CPXXcreateprob (cpxenv_, &cpxstatus_, "minoCpxProb");
  if ( cpxlp_ == NULL ) {
     logger_->msgStream(LogInfo) << me_ << "Failed to create LP." << std::endl;
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
    cstr = new char[(*c_iter)->getName().length() + 1];
    strcpy(cstr, (*c_iter)->getName().c_str());
    conname[i] = cstr;
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
        //assert(!"Ranged constraints not allowed right now!");
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
    cstr = new char[(*v_iter)->getName().length() + 1];
    strcpy(cstr, (*v_iter)->getName().c_str());
    varname[i] = cstr;
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
  obj = new double[numvars];
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
  if ( cpxstatus_ ) {
     logger_->msgStream(LogInfo) << me_ << "Failed to change range values of constraints." 
       << std::endl;
     goto TERMINATE;
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
  delete [] cstr;
  delete [] varname;
  delete [] conname;
  //delete [] sense;
  problem->setEngine(this);

}


void CplexLPEngine::load_()
{
}



void CplexLPEngine::negateObj()
{
}


void CplexLPEngine::removeCons(std::vector<ConstraintPtr> &)
{
  consChanged_ = true;
}


void CplexLPEngine::resetIterationLimit()
{
}


void CplexLPEngine::setIterationLimit(int)
{
}
  

void CplexLPEngine::setTimeLimit(double timelimit)
{
  timeLimit_ = timelimit;
}


void CplexLPEngine::setUpperCutoff(double cutoff)
{
  upperCutoff_ = cutoff;
}
  

EngineStatus CplexLPEngine::solve()
{
  int solstat;
  double objval;
  //int cur_numrows = CPXXgetnumrows (cpxenv_, cpxlp_);
  int cur_numcols = CPXXgetnumcols (cpxenv_, cpxlp_);
  
  double *x = new double[cur_numcols];

#if SPEW
  /* Write a copy of the problem to a file. */
  writeLP("minoCpx.lp"); 
#endif

  /* Optimize the problem and obtain solution. */
  if (bndChanged_ || consChanged_) {
    /* Because the problem is dual feasible with the rows added, using
      the dual simplex method is indicated.  */
    cpxstatus_ = CPXXsetintparam (cpxenv_, CPXPARAM_LPMethod, CPX_ALG_DUAL);
    if (cpxstatus_) {
      logger_->msgStream(LogInfo) << me_
        << "Failed to set the optimization method, error." << std::endl;
      //goto TERMINATE;
    }
  }

  cpxstatus_ = CPXXlpopt (cpxenv_, cpxlp_);
  if ( cpxstatus_ ) {
     logger_->msgStream(LogInfo) << me_ << "Failed to optimize LP." << std::endl;
     //goto TERMINATE;
  }
  solstat = CPXXgetstat (cpxenv_, cpxlp_);

#if SPEW
  /* Write the output to the screen. */
  logger_->msgStream(LogInfo) << me_ << "Solution status = " << solstat << std::endl;
#endif

  /* Get the (primal) objective value. */
  cpxstatus_ = CPXXgetobjval (cpxenv_, cpxlp_, &objval);
  if ( cpxstatus_ ) {
     logger_->msgStream(LogInfo) << me_ 
       << "No LP objective value available. Exiting..." << std::endl;
  }
#if SPEW
  logger_->msgStream(LogInfo) << me_ << "Solution value = " << objval << std::endl;
#endif

  /* Get the solution. */
  cpxstatus_ = CPXXgetx (cpxenv_, cpxlp_, x, 0, cur_numcols-1);
  if (cpxstatus_) {
   logger_->msgStream(LogInfo) << me_ << "Failed to get optimal integer x." 
     << std::endl;
   //goto TERMINATE;
  }
  
#if SPEW
  /* Write the solution. */
  //for (int j = 0; j < cur_numcols; j++) {
   //logger_->msgStream(LogInfo) << colname[j] << ": " << x[j] << std::endl;
  //}
#endif

  // Solve status (replace with string later using CPXXgetstatstring(..))
  if (solstat == 1) {
    status_ = ProvenOptimal;
    sol_->setPrimal(x);
    sol_->setObjValue(objval); 
  } else if (solstat == 3) {
    status_ = ProvenInfeasible;
    sol_->setObjValue(INFINITY);
    //*objLb = -INFINITY;
  } else if(solstat == 2 || solstat == 20) {
    status_ = ProvenUnbounded;    // or it could be infeasible
    sol_->setObjValue(-INFINITY);
  //} else if(model->isProvenDualInfeasible()) {
    //status_ = ProvenUnbounded;    // primal is not infeasible but dual is.
    //sol_->setObjValue(-INFINITY);
    //std::cout << " dual inf \n";
  } else if (solstat == 10 || solstat == 11 ) {
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

  //stats_->time  += timer_->query();

//TERMINATE:
  delete [] x;
  return status_;
}


void CplexLPEngine::writeLP(const char *filename) const 
{
  int status = CPXXwriteprob (cpxenv_, cpxlp_, filename, NULL);
  if (status) {
   assert(!"Failed to write LP to disk.\n");
  }
}


void CplexLPEngine::writeStats(std::ostream &out) const
{
  if (stats_) {
    out << me_ << "total calls            = " << stats_->calls << std::endl
      << me_ << "total time in solving  = " << stats_->time  << std::endl;
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
