// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
// 

/** 
 * \file QGHandlerPDE.cpp
 * \Briefly define a handler for the textbook type Quesada-Grossmann
 * Algorithm.
 * \Authors Ashutosh Mahajan and Meenarli Sharma, Indian Institute of
 * Technology Bombay 
 */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "QGHandlerPDE.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include "QuadraticFunction.h"

// #define SPEW 1

using namespace Minotaur;


typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string QGHandlerPDE::me_ = "QGHandlerPDE: ";

QGHandlerPDE::QGHandlerPDE()
: env_(EnvPtr()),      
  intTol_(1e-6),
  linCoeffTol_(1e-6),
  minlp_(ProblemPtr()),
  minlpC_(ProblemPtr()),
  lpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  nlpWs_(WarmStartPtr()),
  numCuts_(0),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  solAbsTol_(1e-5),
  solRelTol_(1e-5),
  stats_(0)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}

QGHandlerPDE::QGHandlerPDE(EnvPtr env, ProblemPtr minlp, EnginePtr lpe) 
: env_(env),
  intTol_(1e-6),
  linCoeffTol_(1e-6),
  minlp_(minlp),
  minlpC_(ProblemPtr()),
  lpe_(lpe),
  nlpStatus_(EngineUnknownStatus),
  nlpWs_(WarmStartPtr()),
  numCuts_(0),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  solAbsTol_(1e-5),
  solRelTol_(1e-5)
{
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("handler_log_level")->getValue());

  stats_   = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->cuts = 0;
}

QGHandlerPDE::~QGHandlerPDE()
{ 
  if (stats_) {
    delete stats_;
  }

  env_.reset();
  minlp_.reset();
  lpe_.reset();
}

void QGHandlerPDE::addInitLinearX_(const double *x)
{ 
  ConstraintPtr con, newcon;
  double act = 0;
  double c;
  FunctionPtr f, f2;
  LinearFunctionPtr lf = LinearFunctionPtr();
  ObjectivePtr o;
  int error=0;

  error=0;  
  o = minlp_->getObjective();
  if (oNl_ && o) {
    f = o->getFunction();
      //f->write(std::cout);
    act = o->eval(x, &error);
    if(error==0) {
      linearAt_(f, act, x, &c, &lf);
      lf->addTerm(objVar_, -1.0);
      f2 = (FunctionPtr) new Function(lf);
      newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut");
      newcon->write(std::cout);
      ++(stats_->cuts);
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "initial obj cut: " << std::endl
        << std::setprecision(9);
      newcon->write(logger_->msgStream(LogDebug));
#endif
    }	else {
      logger_->msgStream(LogError) << me_ <<"Objective not defined at this point"
        << std::endl;
    }
  }	else {
    logger_->msgStream(LogDebug) << "QG Handler: Problem does not have a " 
      << "nonlinear objective." << std::endl;
  }
}

void QGHandlerPDE::cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                           bool *sol_found, SeparationStatus *status)
{
  const double *x = sol->getPrimal();
  //double *y = new double;
  FunctionPtr f;
  ObjectivePtr o;
  o = minlp_->getObjective();
  double act, alpha;
  int error =0;
  //for (UInt  i=0; i <numvars_ ; i++) {
        ///[>(y+i) = *(x+i);
        //y[i] = x[i];
  //}
  if(o && oNl_){
    act = minlp_->getObjValue(x,&error);
    if (error == 0){
      alpha = x[objVar_->getIndex()];
      // Updating upper bound and adding solution to solution pool if required
      // conditions satisfy 

      double bestval = s_pool->getBestSolutionValue();
      std::cout <<"bestval " << bestval << std::endl;
      std::cout <<"act " << act << std::endl;
      std::cout <<"alpha " << alpha << std::endl;
      if (act <= bestval ){
        s_pool->addSolution(x, act);
        *sol_found = true;
#if SPEW
   logger_->msgStream(LogDebug) 
      << me_ << "new solution found, value = " << act << std::endl;
#endif
      }
      if (act-alpha > solAbsTol_ && (act-alpha > (fabs(alpha)*solRelTol_))){
        //change y to x
        numCuts_=OAFromPoint_(x, status);
      }
    } else {
      logger_->msgStream(LogError) << me_
                    << "Objective not defined at this solution point"<<  std::endl;
    }
  }
  //MS: check if this the only way to do this.
  if (numCuts_ != 0) {
    *status = SepaResolve;
  }else{
#if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "number of cuts from OAFromPoint_ is 0" << std::endl;
#endif 
    *status = SepaPrune;
  }
}

void QGHandlerPDE::fixInts_()
{
  VariablePtr v;
  double xval;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlpC_->varsBegin(); vit!=minlpC_->varsEnd(); 
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = 1;
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlpC_);
      nlpMods_.push(m);
    }
  }
}

bool QGHandlerPDE::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, 
                           bool &, double & )
{

  FunctionPtr f;
  ConstraintPtr c;
  double act;
  const double *x = sol->getPrimal();
  int error=0;
  rel_ = rel;
  error=0;
  if (true == oNl_ ) {
    double alpha = x[objVar_->getIndex()]; 
    relobj_ = alpha;
    act = minlp_->getObjValue(x, &error);
    if(error==0){

      if (alpha < act - solAbsTol_ && alpha < act -(fabs(act)*solRelTol_) ) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "objective not feasible" 
          << std::endl;
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ 
        <<" Objective not defined at this point"<< std::endl;
      return false;
    }
  }
  logger_->msgStream(LogDebug) << me_ << "Looks feasible" 
    << std::endl;
  return true;
}

void QGHandlerPDE::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf)
{

  int n = rel_->getNumVars();
  double *a = new double[n];
  VariableConstIterator vbeg = rel_->varsBegin();
  VariableConstIterator vend = rel_->varsEnd();
  int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  if(error==0){
    *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_); 
    *c  = fval - InnerProduct(x, a, numvars_);
    delete [] a;
  }	else {
    logger_->msgStream(LogError) << me_ <<" Gradient not defined at this point "
      <<  std::endl;
  }
}


void QGHandlerPDE::linearizeObj_(RelaxationPtr rel)
{
  ObjectivePtr o;
  o = minlp_->getObjective();
  std::string name     = "obj_dummy_var";
  if (!o) {
    assert(!"need objective in QG!");
  } else if (o->getFunctionType() != Linear) {
    VariablePtr vPtr     = rel->newVariable(-INFINITY, INFINITY, 
                                            Continuous, name); 
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

    FunctionPtr f;

    assert(o->getObjectiveType()==Minimize);

    rel->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel->newObjective(f, 0.0, o->getObjectiveType());

    oNl_    = true;
    objVar_ = vPtr;
  }
}

int QGHandlerPDE::OAFromPoint_(const double *x, SeparationStatus *status)
{
  double act=-INFINITY;
  ConstraintPtr con, newcon;
  double c;
  LinearFunctionPtr lf = LinearFunctionPtr(); 
  FunctionPtr f, f2;
  ObjectivePtr o;
  UInt num_cuts = 0;
  int error=0;
  double lpvio;
  *status=SepaContinue;
  o = minlp_->getObjective();
  f = o->getFunction();
  act=o->eval(x,&error);
  linearAt_(f, act, x, &c, &lf);
  lpvio = std::max(c+lf->eval(x)-relobj_, 0.0);
    
  if (lpvio>1e-4 && lpvio >(fabs(relobj_+c)*solRelTol_)) {
    lf->addTerm(objVar_, -1.0);
    f2 = (FunctionPtr) new Function(lf);
    newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut"); 
    ++(stats_->cuts);
    ++num_cuts;
    *status = SepaResolve;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "OA cut: " << std::endl
    << std::setprecision(9);
    newcon->write(logger_->msgStream(LogDebug));
#endif
    
  } else{
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "No objective OA cut: " << std::endl;
#endif
  }
  
  return num_cuts;
}


void QGHandlerPDE::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}

void QGHandlerPDE::relaxInitInc(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}

void QGHandlerPDE::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{

  //Does nothing
}

void QGHandlerPDE::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}

void QGHandlerPDE::relax_(RelaxationPtr rel, bool *is_inf)
{
  ConstraintPtr c;
  rel_ = rel;
  ObjectivePtr o;
  FunctionPtr f = (FunctionPtr) new Function();
  //double d;
  const double *x;
  o = minlp_->getObjective();
  minlpC_ = minlp_->clone();
  *is_inf = false;

  // Remove objective from the problem
  minlpC_->removeObjective();
  minlpC_->newObjective(f,0,o->getObjectiveType());

  // Fix all binary variables to 1
  fixInts_();

  lpe_->load(minlpC_);
  //Solve resultant LP
  solveNLP_();

  //Unfix above fixing
  unfixInts_();

  // Add objective back to the problem

  //f = o->getFunction();
  //d = o->getConst();

  // Linearize objective
  linearizeObj_(rel);
  numvars_ = minlp_->getNumVars();
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    x = lpe_->getSolution()->getPrimal();
    lpe_->getSolution()->writePrimal(std::cout);
    addInitLinearX_(x); 
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    *is_inf = true;
    break;
  case (ProvenUnbounded):
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status = " 
  << nlpStatus_ << std::endl;
    assert(!"QGHandlerPDE: Cannot proceed further");
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Number of nonlinear constraints "
    " = " << nlCons_.size() << std::endl;
  logger_->msgStream(LogDebug) << me_ << "Nonlinear solver used = "
    " = " << lpe_->getName() << std::endl;
#endif

#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "Initial relaxation:" 
    << std::endl;
  rel_->write(logger_->msgStream(LogDebug2));
#endif
}

void QGHandlerPDE::separate(ConstSolutionPtr sol, NodePtr , RelaxationPtr rel, 
                         CutManager *, SolutionPoolPtr s_pool,
                         bool *sol_found, SeparationStatus *status)
{      
  VariableConstIterator v_iter;
  VariableType v_type;
  double value;
  const double *x = sol->getPrimal();
  bool is_int_feas = true; 
  rel_=rel;

  for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd(); 
       ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type==Binary || v_type==Integer) {
      value = x[(*v_iter)->getIndex()];
      if (fabs(value - floor(value+0.5)) > intTol_) {
        is_int_feas = false;
        break;
      }
    }
  }

  if (is_int_feas) {
#if SPEW
    logger_->msgStream(LogDebug)<< me_ 
      << "solution is integer feasible, may need to add cuts" << std::endl;	  
#endif
    cutIntSol_(sol, s_pool, sol_found, status);
  } else{
#if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "solution is not integer feasible" << std::endl;	  
#endif
  }
}

void QGHandlerPDE::solveNLP_()
{
  nlpStatus_ = lpe_->solve();
  ++(stats_->nlpS);
}


void QGHandlerPDE::unfixInts_()
{
  Modification *m = 0;
  while(nlpMods_.empty() == false) {
    m = nlpMods_.top();
    m->undoToProblem(minlpC_);
    nlpMods_.pop();
    delete m;
  }
}

void QGHandlerPDE::writeStats(std::ostream &out) const
{
  out
    << me_ << "number of nlps solved       = " << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps   = " << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps     = " << stats_->nlpF << std::endl
    << me_ << "number of cuts added        = " << stats_->cuts << std::endl;
}

std::string QGHandlerPDE::getName() const
{
  return "QG Handler (Quesada-Grossmann)";
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
