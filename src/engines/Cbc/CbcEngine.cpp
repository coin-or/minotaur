// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/**
 * \file CbcEngine.cpp
 * \brief Implement an interface to the Coin-OR Branch and Cut, Cbc solver
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <iomanip>

#include "coin/CoinPragma.hpp"
#include "coin/CbcModel.hpp"
#include "coin/OsiClpSolverInterface.hpp"

#include "MinotaurConfig.h"
#include "CbcEngine.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Objective.h"
#include "Option.h"
#include "Problem.h"
#include "Solution.h"
#include "Timer.h"
#include "Variable.h"

using namespace Minotaur;

//#define SPEW 1

const std::string CbcEngine::me_ = "CbcEngine: ";

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CbcEngine::CbcEngine(EnvPtr env)
: env_(env),
  sol_(0)
{
  logger_ = env_->getLogger();
  timer_  = env->getNewTimer();
  osilp_  = 0;
  stats_  = new CbcStats();
  stats_->calls    = 0;
  stats_->time     = 0;
}


CbcEngine::~CbcEngine()
{
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
  if (osilp_) {
    delete osilp_;
  }
  if (sol_) {
    delete sol_;
  }
}


void CbcEngine::addConstraint(ConstraintPtr )
{
}


void CbcEngine::changeBound(ConstraintPtr , BoundType , double )
{
}


void CbcEngine::changeBound(VariablePtr , BoundType , double )
{
}


void CbcEngine::changeBound(VariablePtr , double , double )
{
}


void CbcEngine::changeConstraint(ConstraintPtr , LinearFunctionPtr , 
                                   double , double )
{
}


void CbcEngine::changeConstraint(ConstraintPtr, NonlinearFunctionPtr)
{
    assert(!"Cannot change a nonlinear function in CbcEngine");
}


void CbcEngine::changeObj(FunctionPtr , double)
{
}


void CbcEngine::clear()
{
  if (problem_) {
    problem_->unsetEngine();
    problem_ = 0;
  }
}


EnginePtr CbcEngine::emptyCopy()
{
  return new CbcEngine(env_);
}


std::string CbcEngine::getName() const
{
  return "Cbc";
}


double CbcEngine::getSolutionValue() 
{
  return sol_->getObjValue();
}


ConstSolutionPtr CbcEngine::getSolution() 
{
  return sol_;
}


EngineStatus CbcEngine::getStatus() 
{
  return status_;
}


void CbcEngine::load(ProblemPtr problem)
{
  objChanged_ = true;
  bndChanged_ = true;
  consChanged_ = true;
  problem_ = problem;
  problem->setEngine(this);
}


void CbcEngine::load_()
{
  int numvars = problem_->getNumVars();
  int numcons = problem_->getNumCons();
  int i,j;
  double obj_sense = 1.;

  CoinPackedMatrix *r_mat;
  double *conlb, *conub, *varlb, *varub, *obj;
  double *value;
  int *index;
  CoinBigIndex *start;

  ConstraintConstIterator c_iter;
  VariableConstIterator v_iter;
  
  conlb = new double[numcons];
  conub = new double[numcons];
  varlb = new double[numvars];
  varub = new double[numvars];

  VariableGroupConstIterator it;
  /* map the variables in this constraint to the function type (linear here) */

  //XXX Need to count the number of nnz in the problem_ 
  //     -- maybe add it to class later  
  LinearFunctionPtr lin;
  int nnz = 0;
  for (c_iter = problem_->consBegin(); c_iter != problem_->consEnd(); ++c_iter) {
    //XXX Don't want assert here, but not sure of eventually calling sequence
    //     and assumptions
    assert((*c_iter)->getFunctionType() == Linear);
    lin = (*c_iter)->getLinearFunction();
    nnz += lin->getNumTerms();
  }

  index = new int[nnz];
  value = new double[nnz];
  start = new CoinBigIndex[numcons+1];
    
  i = 0;
  j=0;
  start[0] = 0;
  for (c_iter = problem_->consBegin(); c_iter != problem_->consEnd(); ++c_iter) {
    conlb[i] = (*c_iter)->getLb();
    conub[i] = (*c_iter)->getUb();
    lin = (*c_iter)->getLinearFunction();
    for (it = lin->termsBegin(); it != lin->termsEnd(); ++it){
      ConstVariablePtr vPtr = it->first;
      index[j] = vPtr->getIndex();
      value[j] = it->second;
      ++j;
    }
    ++i;
    start[i]=j;
  }
  
  i = 0;
  for (v_iter=problem_->varsBegin(); v_iter!=problem_->varsEnd(); ++v_iter, 
       ++i) {
    varlb[i] = (*v_iter)->getLb();
    varub[i] = (*v_iter)->getUb();
  }

  // XXX: check if linear function is NULL
  lin = problem_->getObjective()->getLinearFunction();
  if (problem_->getObjective()->getObjectiveType() == Minotaur::Maximize) {
    obj_sense = -1.;
  }
  obj = new double[numvars];
  i = 0;
  if (lin) {
    for (v_iter=problem_->varsBegin(); v_iter!=problem_->varsEnd(); ++v_iter, 
         ++i) {
      obj[i]   = obj_sense*lin->getWeight(*v_iter);
    }
  } else {
    memset(obj, 0, numvars * sizeof(double));
  }

  r_mat = new CoinPackedMatrix(false, numvars, numcons, nnz, value, index, 
                               start, NULL);

  if (osilp_) {
    delete osilp_;
  }
  osilp_ = new OsiClpSolverInterface();
  osilp_->setHintParam(OsiDoReducePrint);
  osilp_->messageHandler()->setLogLevel(0); 

  osilp_->loadProblem(*r_mat, varlb, varub, obj, conlb, conub);
  for (v_iter=problem_->varsBegin(); v_iter!=problem_->varsEnd(); ++v_iter, 
       ++i) {
    if (Binary==(*v_iter)->getType() || Integer==(*v_iter)->getType()) {
      osilp_->setInteger((*v_iter)->getIndex());
    }
  }

  if (sol_) {
    delete sol_;
  }
  sol_ = (SolutionPtr) new Solution(1E20, 0, problem_);

  objChanged_ = true;
  bndChanged_ = true;
  consChanged_ = true;

  osilp_->writeLp("foo");
  delete r_mat;
  delete [] index;
  delete [] value;
  delete [] start;
  delete [] conlb;
  delete [] conub;
  delete [] varlb;
  delete [] varub;
  delete [] obj;

  // osilp_->writeLp("stub");
  // exit(0);
}



void CbcEngine::negateObj()
{
}


void CbcEngine::removeCons(std::vector<ConstraintPtr> &)
{
  consChanged_ = true;
}


void CbcEngine::resetIterationLimit()
{
}


void CbcEngine::setIterationLimit(int )
{
}
  

void CbcEngine::setTimeLimit(double )
{
}


void CbcEngine::setUpperCutoff(double)
{
}


EngineStatus CbcEngine::solve()
{
  CbcModel *model = 0;
  std::string params = "-loglevel 0 -solve ";

  timer_->start();
  if (true==objChanged_ || true==bndChanged_ || true==consChanged_) {
    load_();
  }

  stats_->calls += 1;
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "in call number " << stats_->calls
                               << std::endl;
#endif
  
  model = new CbcModel(*osilp_);    
  callCbc(params, *model);

  if (model->isProvenOptimal()) {
    status_ = ProvenOptimal;  
    sol_->setPrimal(osilp_->getColSolution());
    sol_->setObjValue(model->getObjValue()*model->getObjSense() + 
                      problem_->getObjective()->getConstant());
  } else if (model->isProvenInfeasible()) {
    status_ = ProvenInfeasible;
    sol_->setObjValue(INFINITY);
  } else if(model->isContinuousUnbounded()) {
    status_ = ProvenUnbounded;    // or it could be infeasible
    sol_->setObjValue(-INFINITY);
  } else if(model->isProvenDualInfeasible()) {
    status_ = ProvenUnbounded;    // primal is not infeasible but dual is.
    sol_->setObjValue(-INFINITY);
  } else if (model->isNodeLimitReached() || model->isSecondsLimitReached() ||
             model->isSolutionLimitReached()) {
    status_ = EngineIterationLimit;
    sol_->setPrimal(osilp_->getStrictColSolution());
    sol_->setObjValue(osilp_->getObjValue()
        +problem_->getObjective()->getConstant());
  } else if(model->isAbandoned()) {
    status_ = EngineError;
    sol_->setObjValue(INFINITY);
  } else {
    status_ = EngineUnknownStatus;
    sol_->setObjValue(INFINITY);
  }

  stats_->time  += timer_->query();

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "status = " << status_ << std::endl
                               << me_ << "solution value = " 
                               << sol_->getObjValue() << std::endl;
#endif
  timer_->stop();
  bndChanged_ = false;
  consChanged_ = false;
  objChanged_ = false;

  delete model;
  return status_;
}


void CbcEngine::writeLP(const char *) const 
{ 
}


void CbcEngine::writeStats(std::ostream &out) const
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
