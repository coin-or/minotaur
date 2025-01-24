// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 -- 2010 The MINOTAUR Team.
// 

/**
 * \file quasiQGHandler.cpp
 * \brief Define a processor for the textbook type Quesada-Grossmann
 * Algorithm.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "CGraph.h"
#include "Constraint.h"
#include "CutManager.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "HessianOfLag.h"
#include "Jacobian.h"
#include "LinearCut.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "quasiQGHandler.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include "QuadraticFunction.h"

#define SPEW1 0

//#define QUASI
//#define DEBUG1
#define L1PROJ
//#define DEBUG2
//#define DEBUG3
//#define DEBUG4
//#define DEBUG5
//#define NCTMNGR
//#define CTMNGR
#define EXTRACUT
#define NewPOINTPROBLEM
//#define EXTRACUTCVX
//#define PrEXCUT
//#define TimerLS

using namespace Minotaur;


typedef std::vector<ConstraintPtr>::const_iterator CCIter;
//typedef boost::shared_ptr<CutManager> CutManagerPtr;

const std::string quasiQGHandler::me_ = "quasiQGHandler: ";
//const std::string quasiACProblem::me_ = "quasiACProblem: ";

void write_cut(double *gr, double rhs, double gap, int numvars);
void write_point(const double *xo, const double *xp, int numvars0);
void write_point(const double *xo, const double *xp, int numvars0, int numvars1);
void write_array(double *x, int num, std::string str1)
{
  std::cout << "********************  " << str1 << "  ********************\n";
  for (int i = 0; i < num; i++)
    {
      if (fabs(x[i]) > 1e-5)
      {
        std::cout << str1 << "[" << i+1 << "]=" << x[i] << ", ";
      }
    }
  std::cout <<"\n";
}

void write_array(const double *x, int num, std::string str1)
{
  for (int i = 0; i < num; i++)
    std::cout << str1 << "[" << i+1 << "]=" << x[i] << ", ";
  std::cout <<"\n";
}

quasiQGHandler::quasiQGHandler()
  : env_(EnvPtr()),         // NULL
    minlp_(ProblemPtr()),
    proj_minlp_(ProblemPtr()),	
    rel_(RelaxationPtr()),
//    ctMngr_(CutManagerPtr()),
    nlpe_(EnginePtr()),
//    proj_nlpe_(EnginePtr()),
    nlpStatus_(EngineUnknownStatus),
    projnlpStatus_(EngineUnknownStatus),
    nlpWs_(WarmStartPtr()), // NULL
    objVar_(VariablePtr()),
    ctMngr_(),
    nlCons_(0),
    oNl_(false),
    isFeas_(true),
    linCoeffTol_(1e-6),
    stats_(0),
    solAbsTol_(1e-5),
    solRelTol_(1e-5),
    eTol_(1e-1),
    eLinTol_(1e-6),
    handleQuasiConvex_(false),
    num_cuts_(0),
    last_node_(0),
    lastOptVal_(INFINITY),
    partialFix_(false),
    cuttingPoint_(false),
    numExLin_(0),
    ExtLinFr_(3),
    grdTime_(0),
    timer_(0),
    ctmngrTime_(0),
    ECP_(false),
    pntE_(EnginePtr())
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
  intTol_   = env_->getOptions()->findDouble("int_tol")->getValue();
  doACCPM_ = env_->getOptions()->findBool("qg_accpm")->getValue();
  partialFix_ = env_->getOptions()->findBool("partial_bb")->getValue();
}

quasiQGHandler::quasiQGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe) 
  : env_(env),
    minlp_(minlp),
    proj_minlp_(ProblemPtr()),
//    proj_minlp_(0),
    rel_(RelaxationPtr()),
//    ctMngr_(CutManagerPtr()),
    nlpe_(nlpe),
    proj_nlpe_(EnginePtr()),
    nlpStatus_(EngineUnknownStatus),
    projnlpStatus_(EngineUnknownStatus),
    nlpWs_(WarmStartPtr()), // NULL
    objVar_(VariablePtr()),
    ctMngr_(),
    nlCons_(0),
    oNl_(false),
    isFeas_(true),
    linCoeffTol_(1e-6),
    solAbsTol_(1e-5),
    solRelTol_(1e-5),
    eTol_(1e-1),
    eLinTol_(1e-6),
    handleQuasiConvex_(false),
    num_cuts_(0),
    last_node_(0),
    lastOptVal_(INFINITY),
    partialFix_(false),
    cuttingPoint_(false),
    numExLin_(0),
    ExtLinFr_(3),
    grdTime_(0),
    ctmngrTime_(0),
    ECP_(false),
    pntE_(EnginePtr())
{
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("QG_log_level")->getValue());

  intTol_   = env_->getOptions()->findDouble("int_tol")->getValue();
  doACCPM_ = env_->getOptions()->findBool("qg_accpm")->getValue(); 
  partialFix_ = env_->getOptions()->findBool("partial_bb")->getValue();
  handleQuasiConvex_ = env_->getOptions()->findBool("convexity_type")->getValue();
  timer_ = env_->getNewTimer();

  last_sol_ = new double[minlp_->getNumVars()];
  stats_       = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpP = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->cuts = 0;
  stats_->looseCuts = 0;
  stats_->nlpP1 = 0;
  stats_->infCuts = 0;
  stats_->nlpPC =0;
  stats_->p2Cuts = 0;
  stats_->p1Cuts = 0;
  stats_->CoCuts = 0;
  stats_->adCuts = 0;
  stats_->strTime = 0.0;
  stats_->ecpCalls = 0;
//  mylogfile_.open("mylogfile.txt", std::ios::out);
//  mylogfile_ << env_->getOptions()->findString("problem_file")->getValue() << "\n";

/*
  rootSol_ = new double [nvars];
  incSol_ = new double [nvars];
  LPrelSol_ = new double [nvars];
  BrootSol_ = false;
  BincSol_ = false;
  BLPrelSol_ = false;
*/

}

quasiQGHandler::quasiQGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe, EnginePtr ACnlpe) 
  : env_(env),
    minlp_(minlp),
    proj_minlp_(ProblemPtr()),
//    proj_minlp_(0),
    rel_(RelaxationPtr()),
//    ctMngr_(CutManagerPtr()),
    nlpe_(nlpe),
    ACnlpe_(ACnlpe),
    proj_nlpe_(EnginePtr()),
    nlpStatus_(EngineUnknownStatus),
    projnlpStatus_(EngineUnknownStatus),
    nlpWs_(WarmStartPtr()), // NULL
    objVar_(VariablePtr()),
    ctMngr_(),
    nlCons_(0),
    oNl_(false),
    isFeas_(true),
    linCoeffTol_(1e-6),
    solAbsTol_(1e-5),
    solRelTol_(1e-5),
    eTol_(1e-1),
    eLinTol_(1e-6),
    handleQuasiConvex_(false),
    num_cuts_(0),
    last_node_(0),
    lastOptVal_(INFINITY),
    partialFix_(false),
    cuttingPoint_(false),
    relobj_(0.0),
    objCutOff(false),
    numExLin_(0),
    ExtLinFr_(3),
    grdTime_(0),
    ctmngrTime_(0),
    ECP_(false),
    pntE_(EnginePtr())
{
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("QG_log_level")->getValue());
  intTol_   = env_->getOptions()->findDouble("int_tol")->getValue();
  doACCPM_ = env_->getOptions()->findBool("qg_accpm")->getValue(); 
  partialFix_ = env_->getOptions()->findBool("partial_bb")->getValue();
  last_sol_ = new double[minlp_->getNumVars()];
  handleQuasiConvex_ = env_->getOptions()->findBool("convexity_type")->getValue();
  timer_ = env_->getNewTimer();

  stats_       = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpP = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->cuts = 0;
  stats_->looseCuts = 0;
  stats_->nlpP1 = 0;
  stats_->infCuts = 0;
  stats_->nlpPC =0;
  stats_->p2Cuts = 0;
  stats_->p1Cuts = 0;
  stats_->CoCuts = 0;
  stats_->adCuts = 0;
  stats_->strTime = 0.0;
  stats_->ecpCalls = 0;
//  mylogfile_.open("mylogfile.txt", std::ios::out);
//  mylogfile_ << env_->getOptions()->findString("problem_file")->getValue() << "\n";

/*
  rootSol_ = new double [nvars];
  incSol_ = new double [nvars];
  LPrelSol_ = new double [nvars];
  BrootSol_ = false;
  BincSol_ = false;
  BLPrelSol_ = false;
*/
}

quasiQGHandler::quasiQGHandler(EnvPtr env, ProblemPtr minlp, ProblemPtr proj_inst, ProblemPtr l1proj_inst,
                     EnginePtr nlpe, EnginePtr proj_nlpe, EnginePtr l1proj_nlpe, EnginePtr ACnlpe)
  : env_(env),
    minlp_(minlp),
    proj_minlp_(proj_inst),
    l1proj_minlp_(l1proj_inst),
    rel_(RelaxationPtr()),
//    ctMngr_(CutManagerPtr()),
    nlpe_(nlpe),
    ACnlpe_(ACnlpe),
    proj_nlpe_(proj_nlpe),
    l1proj_nlpe_(l1proj_nlpe),
    nlpStatus_(EngineUnknownStatus),
    projnlpStatus_(EngineUnknownStatus),
    nlpWs_(WarmStartPtr()), // NULL
    objVar_(VariablePtr()),
    ctMngr_(),
    nlCons_(0),
    oNl_(false),
    isFeas_(true),
    linCoeffTol_(1e-6),
    solAbsTol_(1e-6),
    solRelTol_(1e-4),
    eTol_(1e-1),
    eLinTol_(5e-4),
    handleQuasiConvex_(false),
    num_cuts_(0),
    constCoef_(1),
    onlyBinary_(false),
    numvars_(0),
    last_node_(0),
    lastOptVal_(INFINITY),
    partialFix_(false),
    cuttingPoint_(false),
    numExLin_(0),
    ExtLinFr_(3),
    grdTime_(0),
    ctmngrTime_(0),
    ECP_(false),
    pntE_(EnginePtr())
{
  lf_pure_ = (LinearFunctionPtr) new LinearFunction();
  qf_pure_ = (QuadraticFunctionPtr) new QuadraticFunction();
  qfPoint_ = (QuadraticFunctionPtr) new QuadraticFunction();
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("qg_log_level")->getValue());

  intTol_   = env_->getOptions()->findDouble("int_tol")->getValue();
  doACCPM_ = env_->getOptions()->findBool("qg_accpm")->getValue(); 
  partialFix_ = env_->getOptions()->findBool("partial_bb")->getValue();
  handleQuasiConvex_ = env_->getOptions()->findBool("convexity_type")->getValue();
  timer_ = env_->getNewTimer();

  //const char* fname;
//  fname = env_->getOptions()->findString("problem_file")->getValue();
//  fname_ = "";
//  fname_.append(fname.begin(),fname.end()-3);
//  std::cout << "alaki = " << fname_ << "\n";
//  mylogfile_.open("mylogfile.txt", std::ios::out);
//  mylogfile_ << env_->getOptions()->findString("problem_file")->getValue() << "\n";
//  mylogfile_ (fname_, std::ios::app);

  stats_       = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpP = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->cuts = 0;
  stats_->looseCuts = 0;
  stats_->nlpP1 = 0;
  stats_->infCuts = 0;
  stats_->nlpPC =0;
  stats_->p2Cuts = 0;
  stats_->p1Cuts = 0;
  stats_->CoCuts = 0;
  stats_->adCuts = 0;
  stats_->strTime = 0.0;
  stats_->ecpCalls = 0;

  numvars_ = proj_minlp_->getNumVars();
  last_sol_ = new double[numvars_];
  if (numvars_ - proj_minlp_->getSize()->conts - proj_minlp_->getSize()->bins ==0)
    onlyBinary_=true;


  xnlp_ = new double[numvars_];

  FunctionPtr proj_fn;
  VariablePtr v; 
  VariablePtr v1;
  HessianOfLagPtr hPtr; 
  LinearFunctionPtr l1_lf;  
  LinearFunctionPtr l1_obj = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr l1_fn;
  VariablePtr vplus;
  VariablePtr vminus;
  ConstraintPtr newcon;

  proj_lf_ = (LinearFunctionPtr) new LinearFunction();
  proj_qf_ = (QuadraticFunctionPtr) new QuadraticFunction();
    
//  
  constCoef_ = constCoef_ * constCoef_;

//  std::cout << "11**************l1proj_minlp_ **********\n";
//  l1proj_minlp_->write(std::cout);

  for (int i = 0; i <numvars_ ; ++i)
  {
    l1_lf = (LinearFunctionPtr) new LinearFunction();
//    l1_fn = (FunctionPtr) new Function();
    v = proj_minlp_->getVariable(i);
    v1 = l1proj_minlp_->getVariable(i);
    qf_pure_->addTerm(v, v, 0.5 * constCoef_);
    lf_pure_->addTerm(v, -1.0*constCoef_);
    if (v->getType() == Binary || v->getType() == Integer)
    {
//#ifdef L1PRO0J
      vplus  = l1proj_minlp_->newVariable(0.0,INFINITY,Continuous);
      vminus = l1proj_minlp_->newVariable(0.0,INFINITY,Continuous);
      l1_obj->addTerm(vplus, 1.0);
      l1_obj->addTerm(vminus, 1.0);
      l1_lf->addTerm(v1, 1.0);
      l1_lf->addTerm(vplus, -1.0);
      l1_lf->addTerm(vminus, 1.0);
      l1_fn= (FunctionPtr) new Function(l1_lf);
      newcon = l1proj_minlp_->newConstraint(l1_fn, 100, 100, "l1_linearization");
      l1_linConst_.push_back(newcon);
      IntVarID_.push_back(v1->getId());
//#endif
      proj_qf_->addTerm( v, v, 0.5*constCoef_);
      proj_lf_->addTerm( v, -1.0*constCoef_);
    } 
  }
/*
  std::vector<VariablePtr>l1IntVars;
  for (VariableConstIterator it = l1proj_minlp_->varsBegin(); 
                             it!=  l1proj_minlp_->varsBegin(); ++it)
  {
    v1 = *it;
    if (v1->getType() == Binary || v1->getType() == Integer){
      l1IntVars.push_back(v1);
      vplus  = l1proj_minlp_->newVariable(0.0,INFINITY,Continuous);
      vminus = l1proj_minlp_->newVariable(0.0,INFINITY,Continuous);
      l1_obj->addTerm(vplus, 1.0);
      l1_obj->addTerm(vminus, 1.0);
      l1_lf->addTerm(v1, 1.0); 
      l1_lf->addTerm(vplus, -1.0);
      l1_lf->addTerm(vminus, 1.0);
      l1_fn = (FunctionPtr) new Function(l1_fn);
      newcon = l1proj_minlp_->newConstraint(l1_fn, 100, 100, "l1_linearization");
      l1_linConst_.push_back(newcon);      
      IntVarID_.push_back(v1->getId());
    }
  }
*/
  l1_proj_ = (FunctionPtr) new Function(l1_obj);
  l1proj_minlp_->removeObjective();
  l1proj_minlp_->newObjective(l1_proj_, 0, Minimize);
//  l1proj_minlp_->newObjective(proj_fn, 0, Minimize);
  l1proj_minlp_->calculateSize(true);

//  std::cout << "22******************l1proj_minlp_*************\n";
//  l1proj_minlp_->write(std::cout);

  l1proj_minlp_->setNativeDer();
  l1proj_nlpe_->load(l1proj_minlp_);
//*/

  proj_fn = (FunctionPtr) new Function(proj_lf_, proj_qf_);
  proj_minlp_->removeObjective();
  proj_minlp_->newObjective(proj_fn, 0, Minimize);
  proj_minlp_->calculateSize(true);
  proj_minlp_->setNativeDer();

  /// TODO: Should this be turned back on?
  
  //proj_minlp_->calculateSize(true);
  //hPtr = (HessianOfLagPtr) new HessianOfLag(proj_minlp_.get());
  //proj_minlp_->setHessian(hPtr);
  //proj_minlp_->setQuadraticHessian();

  proj_nlpe_->load(proj_minlp_);

  x_star = new double [numvars_];
  a_ = new double [numvars_];
  x_pr_ = new double [numvars_];
  x_OA_ = new double [numvars_];
  
//  proj_minlp_->write(std::cout);

/*
  rootSol_ = new double [nvars];
  incSol_ = new double [nvars];
  LPrelSol_ = new double [nvars];
  BrootSol_ = false;
  BincSol_ = false;
  BLPrelSol_ = false;
*/
//  gr_ = new double [nvars];

//  readSol_();
}

quasiQGHandler::~quasiQGHandler()
{
  if (stats_) {
    writeStats(logger_->msgStream(LogInfo));
    delete stats_;
  }
//  delete [] rootSol_;
//  delete [] incSol_;
//  delete [] LPrelSol_;
//  delete [] xnlp_;
  //timer_.reset();
  env_.reset();
  minlp_.reset();
  ctMngr_.reset();
  if (handleQuasiConvex_ == 1)
  {
    proj_minlp_.reset();
    l1proj_minlp_.reset();
  } 
  if (ECP_)
    quasiSol_.reset();
}



void quasiQGHandler::relax_(RelaxationPtr rel, Bool *is_inf)
{
  ConstraintPtr c;
  rel_ = rel;

  linearizeObj_(rel);
  
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
       ++it) {
    c = *it;
    if (c->getFunctionType()!=Constant && c->getFunctionType() != Linear) {
      nlCons_.push_back(c);
    }
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Number of nonlinear constraints "
    " = " << nlCons_.size() << std::endl;
  logger_->msgStream(LogDebug) << me_ << "Nonlinear solver used = "
    " = " << nlpe_->getName() << std::endl;
#endif
//  if (!partialFix_)
  initLinear_(is_inf);
//  else *is_inf = false;
#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "Initial relaxation:" 
                                << std::endl;
  rel_->write(logger_->msgStream(LogDebug2));
#endif
 
  //doACCPM_=false;
  if (doACCPM_){
    if (handleQuasiConvex_){
//      qgACP_ = (quasiACProblemPtr) new quasiACProblem(env_, ACnlpe_, rel_,false,nlCons_);
    } else {
//      qgACP_ = (quasiACProblemPtr) new quasiACProblem(env_, ACnlpe_, rel_,true,nlCons_);
    }
  }
  //rel_->write(std::cout);
  // exit(0);
}


void quasiQGHandler::relaxInitFull(RelaxationPtr rel, Bool *is_inf)
{
  relax_(rel, is_inf);
}


void quasiQGHandler::relaxInitInc(RelaxationPtr rel, Bool *is_inf)
{
  relax_(rel, is_inf);
}


void quasiQGHandler::relaxNodeFull(NodePtr , RelaxationPtr , Bool *)
{
  //assert(!"quasiQGHandler::relaxNodeFull not implemented!");
}

void quasiQGHandler::relaxNodeInc(NodePtr , RelaxationPtr , Bool *)
{
  // do nothing.
}

void quasiQGHandler::initLinear_(Bool *isInf)
{
  const double *x;

  *isInf = false;
  /*
  if (BincSol_) {
    minlp_->setInitialPoint(incSol_);
  } else if (BLPrelSol_) {
      minlp_->setInitialPoint(LPrelSol_);
    } else if (BrootSol_) {
        minlp_->setInitialPoint(rootSol_);
    }
  */
    
  solveNLP_();

  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    x = nlpe_->getSolution()->getPrimal();
    addInitLinearX_(x);

    //rootSol_ = x;
    //BrootSol_ = true;
    // change lb of objective.
    // rel_->changeBound(objVar_, Lower, nlpe_->getSolutionValue());
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    *isInf = true;
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
    assert(!"quasiQGHandler: Cannot proceed further");
  }
#ifdef NewPOINTPROBLEM
  if (ECP_){
    quasiSol_->addSol(x);
    createPointProb_();
  }
//  if (ECP_){
//    addPointToProb_(x);
//  }
#endif
//  delete [] x;
}

void quasiQGHandler::setECP(Bool ecp){
  if (ecp==true){
    ECP_=true;
    quasiSol_ = (quasiSolutionPoolPtr) new quasiSolutionPool(numvars_);
  }
  else{
   ECP_=false;
  }
}

void quasiQGHandler::addInitLinearX_(const double *x)
{
  ConstraintPtr con;
  ConstraintPtr newcon;
  double act, c;
  FunctionPtr f, f2;
  LinearFunctionPtr lf = LinearFunctionPtr();
  ObjectivePtr o;
  int error;

//  numvars_ = minlp_->getNumVars();
//  double *gr = new double[numvars_];

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    f = con->getFunction();
    linearAt_(f, act, x, &c, &lf);
    f2 = (FunctionPtr) new Function(lf);

    /// Looks like this was a bug when we didn't check the direction of the 
    /// constraint to add the linearization
    if (con->getUb() < INFINITY)
    {
      if (!handleQuasiConvex_){
//        std::cout << "add <= convex initLinear for const = " << con->getId() << "\n";
/////////////////
//	newcon = (ConstraintPtr) new Constraint(f2, -INFINITY, con->getUb()-c);
        if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2,-INFINITY,con->getUb()-c,true,false);
	else
          newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c, "lnrztn_cut");

/////////
        ++(stats_->cuts);
      } else {
//        std::fill(gr,gr+numvars_,0.0); 
//        f->evalGradient(x,gr,&error);
        if (act - con->getUb() > -solAbsTol_){
//          std::cout << "add <= quasiconvex initLinear for const = " << con->getId() << "\n";
//          double rhs0 = InnerProduct(gr, x, numvars_);
//          std::cout << "rhs0 = " << rhs0 << ", rhs1 = " << act-c;
//          newcon = rel_->newConstraint(f2,-INFINITY, InnerProduct(gr, x, numvars_), "lnrztn_cut");
//          newcon->write(std::cout); 
//          std::cout << "\n";
	  if (ctMngr_)
	    newcon = ctMngr_->addCut(rel_,f2,-INFINITY,act - c,true,false);
	  else
            newcon = rel_->newConstraint(f2,-INFINITY, act - c, "lnrztn_cut");
          ++(stats_->cuts);
        }
      }
    }

    if (con->getLb() > -INFINITY)
    {
      if (!handleQuasiConvex_){
//        std::cout << "add >= convex initLinear for const = " << con->getId() << "\n";
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,con->getLb()-c,INFINITY,true,false);
	else
          newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY, "lnrztn_cut");

        ++(stats_->cuts);
 #if SPEW
    logger_->msgStream(LogDebug) << me_ << "initial constr. cut: ";
    newcon->write(logger_->msgStream(LogDebug));
#endif
     } else {
        if (con->getLb() - act > -solAbsTol_) {
//          std::cout << "add >= convex initLinear for const = " << con->getId() << "\n";
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,act - c,INFINITY,true,false);
	else
          newcon = rel_->newConstraint(f2, act-c, INFINITY, "lnrztn_cut");

#if SPEW
	  ++(stats_->cuts);
    logger_->msgStream(LogDebug) << me_ << "initial constr. cut: ";
    newcon->write(logger_->msgStream(LogDebug));
#endif
       }
      }
            
    }
  }


  o = minlp_->getObjective();
  if (oNl_ && o) {
//    std::cout << "add obj cut in initLinear\n";
    f = o->getFunction();
    act = o->eval(x, &error);
    linearAt_(f, act, x, &c, &lf);
    lf->addTerm(objVar_, -1.);
    f2 = (FunctionPtr) new Function(lf);
    if (ctMngr_)
      newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-1.0*c,true,true);
    else
      newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut");
    //ObjId_ = newcon->getId();
    ++(stats_->cuts);
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "initial obj cut: " << std::endl
                                 << std::setprecision(9);
    newcon->write(logger_->msgStream(LogDebug));
#endif
  } else {
    logger_->msgStream(LogDebug) << "QG Handler: Problem does not have a " 
	 			<< "nonlinear objective." << std::endl;
  }
}


void quasiQGHandler::linearizeObj_(RelaxationPtr rel)
{
  ObjectivePtr o;
  o = minlp_->getObjective();

  if (!o) {
    assert(!"need objective in QG!");
  } else if (o->getFunctionType() != Linear) {
    std::string name     = "obj_dummy_var";
    VariablePtr vPtr     = rel->newVariable(-INFINITY, INFINITY, 
                                            Continuous, name); 
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
//    LinearFunctionPtr lf1 = (LinearFunctionPtr) new LinearFunction();
//    QuadraticFunctionPtr qf1 = (QuadraticFunctionPtr) new QuadraticFunction();
//    NonlinearFunctionPtr nlf1 = (NonlinearFunctionPtr) new NonlinearFunction();

    FunctionPtr f;
//    FunctionPtr f1 = (FunctionPtr) new Function();
//    f1 = o->getFunction();
//    nlf1 = f1->getNonlinearFunction();
//    qf1 = f1->getQuadraticFunction();
//    lf1 = f1->getLinearFunction();
//    lf1->addTerm(vPtr, -1.0);
    
//    FunctionPtr f2 = (FunctionPtr) new Function(lf1, qf1, nlf1);
//    rel_->newConstraint(f2, -INFINITY, 0.0, "objlinear");
    
    assert(o->getObjectiveType()==Minimize);
    
    rel->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel->newObjective(f, 0.0, o->getObjectiveType());
    oNl_    = true;
    objVar_ = vPtr;
  }
}


Bool quasiQGHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, 
                           Bool &)
{

//  std::cout << " isfeasible ";
  FunctionPtr f;
  ConstraintPtr c;
  double act;
  const double *x = sol->getPrimal();
  int error;

  rel_ = rel;

/*
  for (int i = 0; i < rel_->getNumVars();i++)
    std::cout << " x1[" << i+1 << "] = " << x[i]; 
  std::cout << "\n";
*/

  // check violation by nonlinear constraints.
//  std::cout << " size of nlCons_ = " << nlCons_.size();
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    c = *it;
    act = c->getActivity(x, &error);
//    std::cout << "activiy[" << c->getId() << "] = " << act << ", u =" << c->getUb() << ", l =" << c->getLb() << std::endl;
    if (act > c->getUb() + solAbsTol_ || act < c->getLb() - solAbsTol_) {
      if ( act > c->getUb() + fabs(c->getUb())*solRelTol_ || act < c->getLb() - fabs(c->getLb())*solRelTol_ ){
#if SPEW
        logger_->msgStream(LogDebug) 
          << me_ << "constraint not feasible" << std::endl
          << me_;
        c->write(logger_->msgStream(LogDebug2));
        logger_->msgStream(LogDebug) 
          << me_ << "activity = " << act << std::endl;  
#endif
        isFeas_ = false;
//        std::cout << "  isfeasible is false 1\n";
        return false;
      }
    }
  }
#if SPEW
  logger_->msgStream(LogDebug) 
    << me_ << "all nonlinear constraints feasible." << std::endl;
#endif
  objCutOff = false;
  if (true == oNl_ ) {
    //double alpha = sol->getObjValue();
    // It is important to check against x[.] value and not against
    // sol->getObjValue(). [See e.g. Feasibility Pump]
    double alpha = x[objVar_->getIndex()]; 
    relobj_ = alpha;
    act = minlp_->getObjValue(x, &error);

//    std::cout << "LP value = " << alpha 
//                               << ", NLP activity = " << act 
//                               << std::endl;

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "LP value = " << alpha 
                                 << std::endl
                                 << me_ << "NLP activity = " << act 
                                 << std::endl;

#endif
    if (alpha < act - solAbsTol_) {
      //std::cout << " **************** objective not feasible" << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "objective not feasible" 
                                   << std::endl;
#endif
      isFeas_ = false;
      objCutOff = true;
      return false;
    }
  }
  isFeas_ = true;
  return true;
}


void quasiQGHandler::separate(ConstSolutionPtr sol, NodePtr node_1, RelaxationPtr, CutManager *cutman,
                         SolutionPoolPtr s_pool, Bool *sol_found, 
                         SeparationStatus *status)
{


//  std::cout << "separate\n";
///*
//#ifdef CTMNGR
//  timer_->start();
  //ctMngr_->checkCuts(rel_,sol,node_1);
  //ctMngr_->updateCuts(rel_,sol);
  //ctmngrTime_ += timer_->query();
//  timer_->stop();
//#endif
//*/

  //std::cout << "start separating ";
  // check integer feasibility of sol; must add cuts if is integer feasible
  numvars_ = minlp_->getNumVars();
  VariableConstIterator v_iter;
  VariableType v_type;
  double value;
  const double *x = sol->getPrimal();
  Bool is_int_feas = true; //, isNLPfeas = false, dummy = false;
  Bool is_feas = true;
  Bool repeat_sol=true;
  int i;
  int last_node1_ = last_node_;
  double lastOptVal1 = lastOptVal_;
  SeparationStatus *status1;
  UInt addCuts = 0;
  int depth = node_1->getDepth();
  ConstraintPtr con;
  int error;

/*
  std:: cout << "11  ";
 if (*status == SepaResolve)
   std::cout << "Resolve  ";
 else if (*status == SepaPrune)
    std::cout << "Prune  ";
 else if (*status == SepaContinue)
    std::cout << "Continue  ";
 else
    std::cout << "Other  ";
*/

  double rand_ecp = (double) rand() / ((double) RAND_MAX + 1 );
//  std::cout << "last node number = " << last_node_ << "\n";
//  std::cout << "node number = " << node_1->getId() << "\n";
  if (node_1->getId() == 0 || last_node_ != node_1->getId()) {
    lastOptVal_ = INFINITY;
    last_node_ = node_1->getId();
    for (i=0; i < numvars_; i++){
//      std::cout << " x[" << i << "]=" << x[i] << ", ";
      last_sol_[i]=x[i];
    }
  }
  else {
    for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd();
         ++v_iter) {
          i = (*v_iter)->getIndex();
          value = x[i];
          if (fabs(value-last_sol_[i]) > solAbsTol_) {
            repeat_sol=false;
            break;
          }
      }
    if (!repeat_sol){
      for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd();
           ++v_iter) {
            i = (*v_iter)->getIndex();
            last_sol_[i] = x[i];
      }
    }
    else {
//      logger_->msgStream(LogError) << me_ << "NLP engine status = " 
//                                   << nlpe_->getStatusString() << std::endl;
//      assert(!"quasiQGHandler: Cannot proceed further");
    }
  }

  for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd(); 
       ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type==Binary || v_type==Integer) {
      value = x[(*v_iter)->getIndex()];
      if (fabs(value - floor(value+0.5)) > intTol_) {
        is_int_feas = false;
        //std::cout << " is not int_feas\n";
        break;
      }
    }
  }

  //LPrelSol_ = x;
  //BLPrelSol_ = true;

  
  // TODO: There may be other situations when we want to add cuts with different
  // methods

/*
  std:: cout << "22  ";
 if (*status == SepaResolve)
   std::cout << "Resolve  ";
 else if (*status == SepaPrune)
    std::cout << "Prune  ";
 else if (*status == SepaContinue)
    std::cout << "Continue  ";
 else
    std::cout << "Other  ";
*/
  if (is_int_feas) {
    //std::cout << "  integer feasible  ";
#if SPEW
     logger_->msgStream(LogDebug) 
        << me_ << "solution is integer feasible, may need to add cuts" << std::endl;	  
#endif
    if (!partialFix_) {
      //std::cout << "cutIntSol_\n";
      cuttingPoint_ = true;
      cutIntSol_(sol, s_pool, sol_found, status);
      //std::cout << "end cutIntSol_\n";
    }
    else {
      cuttingPoint_ = true;
      solvePartialNLP_(sol, s_pool, sol_found, status);
    }
  }
  else if ( doACCPM_) {
    UInt nodeid = node_1->getId();
    if (last_node_ != nodeid)
    {
#if SPEW
      logger_->msgStream(LogDebug) 
          << me_ << "accpm is on" << std::endl;
#endif
	 // TODO: Add call to accpm cut method here
	 // TODO: Add controls for how often to call this method (e.g., once
	 // per node, only at a certain depth, etc.
	 // accpmCut_(sol, s_pool, sol_found, status);  
//    std::cout << "go to ACCPM\n";
//      QGNewCut qgnewcut = qgACP_->ACCPM_();
//      ACCutsToRelax_(qgnewcut); 
    }
  } 

#ifdef EXTRACUT


/*
  std:: cout << "33  ";
 if (*status == SepaResolve)
   std::cout << "Resolve  ";
 else if (*status == SepaPrune)
    std::cout << "Prune  ";
 else if (*status == SepaContinue)
    std::cout << "Continue  ";
 else
    std::cout << "Other  ";
*/

/*

  if (ECP_){
    timer_->start();
//  if (node_1->getId() != last_node1_){
    //if ((lastOptVal_ - lastOptVal1)/(std::max(1.0,fabs(lastOptVal1))) > 0.0001){ 
    if (handleQuasiConvex_==true){
      numExLin_++;
      //rand_ecp = 0.0;
      //if (rand_ecp <= betaECP_ * pow(2,-depth) && last_node1_ != node_1->getId() ){ // && (lastOptVal_ - lastOptVal1)/(std::max(1.0,fabs(lastOptVal1))) > 0.0001){
      if (rand_ecp <= betaECP_ * pow(2,-depth) && (lastOptVal_ - lastOptVal1)/(std::max(1.0,fabs(lastOptVal1))) > 0.0001){
//	std::cout << " into ECP  ";
        numExLin_ = 0;
        ++stats_->ecpCalls;
        //xnlp_ = nlpe_->getSolution()->getPrimal();
	//ECPCuts_(sol);
	//addCuts = strCutsQuasi2_(sol,xnlp_);
	//addCuts = strCutsQuasi1_(sol,xnlp_);
	//addCuts = QuasiECP_ContNLP(sol);
	addCuts = solvePointProb_(sol->getPrimal());
	//addCuts = QuasiECP_ContProj(sol);
	//addCuts = QuasiECP_ContNLPLimited(sol);
	if (addCuts == -1){
	  *status = SepaPrune;
	  addCuts = 0;
	}
        stats_->adCuts += addCuts;
        }
      } else {
      //if (numExLin_ == 1) { // && node_1->getDepth() <= 5) {
      //if (depth <= 5 || depth == 15 || depth == 30 || numExLin_ == 2 || depth == 10 || depth == 20 || depth ==15 ||  depth == 25 ){ // || depth == 30 || depth == 35){ //  ||
      //if (depth <= 5 || depth == 10 || depth == 20 || depth ==15 || depth == 25 || depth == 30 || depth == 35){
      if (rand_ecp <= betaECP_ * pow(2,-depth) && (lastOptVal_ - lastOptVal1)/(std::max(1.0,fabs(lastOptVal1))) > 0.0001){
      //if (depth <= 10 || depth == 20 || numExLin_ == ExtLinFr_) {
          ++stats_->ecpCalls;
          numExLin_ = 0;
	  addCuts = ECPCuts_(sol);
	//addCuts = strECPCuts_(sol);
	  stats_->adCuts += addCuts;
//	strCutsCvx_(sol,xnlp_);
//	OAFromPoint_(xnlp_, x, status1);
        } //else numExLin_++;
      
    if (addCuts != 0)
      *status = SepaResolve;
    }
    last_node1_ = node_1->getId();
    lastOptVal_ = rel_->getObjValue(x,&error);
    grdTime_ += timer_->query();
    timer_->stop();
  }
*/  
  if (*status == SepaResolve && addCuts == 0){
//    *status = SepaResolve;
//    std::cout << " 1111111111111111111111111111111  ";
  }

//  else std::cout << " 2  ";

#endif

/*
 if (*status == SepaResolve)
   std::cout << "Resolve  ";
 else if (*status == SepaPrune)
    std::cout << "Prune  ";
 else if (*status == SepaContinue)
    std::cout << "Continue  ";
 else
    std::cout << "Other  ";
  std::cout << " node number = " << node_1->getId() << "\n";
*/

}

void quasiQGHandler::cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                           Bool *sol_found, SeparationStatus *status)
{
  const double *nlp_x;
  double nlpval = INFINITY;
  const double *x = sol->getPrimal();
  double lp_obj = (sol) ? sol->getObjValue() : -INFINITY;
  int ncut_inf=0;
  int ncut_pure=0;
  int ncut_int=0;
  int ncut_alpha=0;
  int num_l1proj=0;
  int ncut_cvx = 0;

  fixInts_(x);
#ifdef DEBUG2
  std::cout << "solveNLP_" << std::endl;
#endif
  solveNLP_();
  unfixInts_();

#ifdef DEBUG2
//  std::cout << "solution of relaxation\n";
//  for (UInt i = 0; i < numvars_; i++)
//    std::cout << "x[" << i << "]=" << x[i] << ", ";
//  std::cout << "\n";
#endif

#ifdef DEBUG2
//  std::cout << "before solving projection" << std::endl;
//  std::cout << "after solving projection" << std::endl;
//  minlp_->write(std::cout);

#endif

//  std::cout << "nlp status cutIntSol_ = " << nlpe_->getStatusString() << "\n";
  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):

    ++(stats_->nlpF);
    updateUb_(s_pool, &nlpval, sol_found);
#ifdef NewPOINTPROBLEM
    if (ECP_){
      addPointToProb_(nlp_x);
      quasiSol_->addSol(nlp_x);
    }
#endif
#if SPEW
     logger_->msgStream(LogDebug) 
        << me_ << "solved fixed NLP to optimality, "
		  << "lp_obj = " << lp_obj 
		  << ", nlpval = " << nlpval << std::endl;
#endif
//    std::cout << " 11OAFromPoint_ ";
    if (lp_obj > nlpval-solAbsTol_ || 
        lp_obj > nlpval-(fabs(nlpval)+1e-10)*solRelTol_){
      *status = SepaPrune;
      break;
    } else {
      nlp_x = nlpe_->getSolution()->getPrimal();
      ncut_cvx = OAFromPoint_(nlp_x, sol, status);
/*
  std:: cout << "44  ";
 if (*status == SepaResolve)
    std::cout << "Resolve  ";
 else if (*status == SepaPrune)
     std::cout << "Prune  ";
 else if (*status == SepaContinue)
     std::cout << "Continue  ";
 else
     std::cout << "Other  ";
*/
      break;
    }
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    nlp_x = nlpe_->getSolution()->getPrimal();
    if (nlpStatus_ == ProvenInfeasible || nlpStatus_ == ProvenLocalInfeasible){
      if (!handleQuasiConvex_)
      { 
        OAFromPoint_(nlp_x, sol, status);         
      } else {
        if(ECP_){
	  ncut_inf = OAFromPointInf_(nlp_x, x, status);
	  stats_->infCuts += ncut_inf;
	  num_cuts_ += ncut_inf;
	  ncut_cvx += ncut_inf;
	  if (ncut_inf==0){
	    ncut_int = ProjCutFromPoint_(sol, s_pool, sol_found, status); 
            stats_->p2Cuts += ncut_int;
            num_cuts_ += ncut_int;
	    if (ncut_int == 0){
	//	std::cout << "nuct_int = " << ncut_int << "\n";
	//      ncut_alpha=alphaCutProj_(x, x_pr_, status);
	//      stats_->looseCuts += ncut_alpha;
	//      num_cuts_ += ncut_alpha;
  	//      std::cout << "nuct_alpha = " << ncut_alpha << "\n";
	    }
		
          }		
	  
        } else {
	  //std::cout << "l2 project\n";
          if (env_->getOptions()->findBool("l2_project")->getValue()==true){
	    ncut_int = ProjCutFromPoint_(sol, s_pool, sol_found, status); 
	    ncut_cvx += ncut_int;
            stats_->p2Cuts += ncut_int;
            num_cuts_ += ncut_int;
	    if (ncut_int == 0){
	//	std::cout << "nuct_int = " << ncut_int << "\n";
	//      ncut_alpha=alphaCutProj_(x, x_pr_, status);
	//      stats_->looseCuts += ncut_alpha;
	//      num_cuts_ += ncut_alpha;
  	//      std::cout << "nuct_alpha = " << ncut_alpha << "\n";
	    }

	    //if (ncut_int == 0)
	    //ncut_inf = OAFromPoint_(nlp_x,sol, status);
/*
	    if (ncut_int == 0) {
	      num_l1proj = l1_project_(x, status);
	      stats_->p1Cuts += num_l1proj;
	      if (num_l1proj == 0){
		ncut_inf = OAFromPointInf_(nlp_x, x, status);
		std::cout  << "ncut_inf = " << ncut_inf << "\n";
		stats_->infCuts += ncut_inf;
		num_cuts_ += ncut_inf;		
		if (ncut_inf == 0) {
		  ncut_pure = pure_project_(x, status);
		  std::cout  << "ncut_pure = " << ncut_pure << "\n";
		  stats_->CoCuts += ncut_pure;
		  num_cuts_ += ncut_pure;
		  if (ncut_pure==0) {
//		    ncut_alpha=alphaCutProj_(x, x_pr_, status);
		    std::cout  << "ncut_alpha = " << ncut_alpha << "\n";
		    stats_->looseCuts += ncut_alpha;
		    num_cuts_ += ncut_alpha;
		  } 
		}
	      }
            }
*/
          } else {
//          if (env_->getOptions()->findBool("l1_project")->getValue()==true){
	    //std::cout << "l1 project\n";
            num_l1proj = l1_project_(x, status);
            stats_->p1Cuts += num_l1proj;
            num_cuts_ += num_l1proj;
	    ncut_cvx += num_l1proj;
 	    if (num_l1proj == 0) {
              ncut_inf = OAFromPointInf_(nlp_x, x, status);
	      stats_->infCuts += ncut_inf;
              num_cuts_ += ncut_inf;
	      ncut_cvx += ncut_inf;
	      if (ncut_inf == 0) {
                ncut_int = ProjCutFromPoint_(sol, s_pool, sol_found, status);
		stats_->p2Cuts += ncut_int;
                num_cuts_ += ncut_int;                
	        ncut_cvx += ncut_int;
              }
            }
          }
        }
      }


    }
//               if (ncut_inf == 0) {
//		 ncut_pure = pure_project_(x, status);
//		 std::cout  << "ncut_pure = " << ncut_pure << "\n";
//		 mylogfile_ << "ncut_pure = " << ncut_pure << "\n";
//  	         stats_->CoCuts += ncut_pure;
//                 num_cuts_ += ncut_pure;
//                 if (ncut_pure==0) {
//                 ncut_alpha=alphaCutProj_(x, x_pr_, status);
//                 std::cout  << "ncut_alpha = " << ncut_alpha << "\n";
//                 mylogfile_ << "ncut_alpha = " << ncut_alpha << "\n";
//                 stats_->looseCuts += ncut_alpha;
//                 num_cuts_ += ncut_alpha;
//                 if (ncut_alpha==0){
//                  no_good(sol->getPrimal());
//		  *status = SepaResolve;
//                }
//              }
//            }
//          }
//        if (num_cuts_ == 0)
//          x_pr_ = proj_nlpe_->getSolution()->getPrimal();
          //updateUb_new_(s_pool, &nlpval, sol_found);
//          *status = SepaPrune;
//        }
//      }
//    } 
//    else {
//      OAFromPoint_(nlp_x, sol, status);         
//    }
/*
  if (num_cuts_ == 0)
  {
  *status = SepaPrune;
  updateUb_new_(s_pool, &nlpval, sol_found);
  }
*/

    // else
    // ProjCutFromPoint_(nlp_x, sol, status);
/*
*/

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
                                 << nlpe_->getStatusString() << std::endl
                                 << me_ << "No cut generated, may cycle!"
                                 << std::endl;
    *status = SepaError;

  }
  if (ncut_cvx != 0)
    *status = SepaResolve;
  //else *status = SepaPrune;

//  delete [] nlp_x;

#ifdef EXTRACUTCVX
  std::cout << "extracutcvx\n";
  if (!handleQuasiConvex_)
  {
    if (s_pool){
      strCutsCvx_(x, s_pool->getBestSolution()->getPrimal());
    } else {
      strCutsCvx_(x, nlpe_->getSolution()->getPrimal());
    }
  }
#endif
}


void quasiQGHandler::fixInts_(const double *x)
{
  VariablePtr v;
  double xval;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd(); 
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5); // round it to integer.
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp_);
      nlpMods_.push(m);
    }
  }
}


void quasiQGHandler::unfixInts_()
{
  Modification *m = 0;
  while(nlpMods_.empty() == false) {
    m = nlpMods_.top();
    m->undoToProblem(minlp_);
    nlpMods_.pop();
    delete m;
  }
}

void quasiQGHandler::copyLPBounds_(std::stack<Modification *> *mods)
{
  VariablePtr v, lpvar;
  VarBoundMod2 *m = 0;
  int i=0;

  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd(); 
       ++vit, ++i) {
    v = *vit;
    lpvar = rel_->getVariable(i);
    m = new VarBoundMod2(v, lpvar->getLb(), lpvar->getUb());
    m->applyToProblem(minlp_);
    mods->push(m);
  }
}


void quasiQGHandler::undoLPBounds_(std::stack<Modification *> *mods)
{
  Modification *m = 0;
  while (mods->empty()==false) {
    m = mods->top();
    m->undoToProblem(minlp_);
    mods->pop();
    delete m;
  }
}

void quasiQGHandler::solvePartialNLP_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                                 Bool *sol_found, SeparationStatus *status)
{
  std::stack<Modification *> mods;
  numvars_ = minlp_->getNumVars();
  const double *x = new double[numvars_];
  VariableConstIterator v_iter;
  VariableType v_type;
  VariablePtr v;
  double value;
  Bool is_int_feas = true;
  double nlpval;
  double lp_obj = (sol) ? sol->getObjValue() : -INFINITY;

  int alphaCut = 0;
  int ncut_int = 0;
  

/*
  std::cout << std::endl;
    for (int i=0; i < numvars_; i++)
    {
//      v = minlp_->getVariable(i);
//      if (v->getType()==Binary || v->getType()==Integer)
      std::cout << "x[" << i+1 << "] = " << sol->getPrimal()[i] << ", ";
    }
    std::cout << std::endl;
*/
  

  copyLPBounds_(&mods);
  solveNLP_();
  nlpval = nlpe_->getSolutionValue();
//  std::cout << " lp_obj = " << lp_obj;
//  std::cout << " nlpval = " << nlpval << "\n";

//  std::cout << "partial nlp status = " << nlpe_->getStatusString() << "\n";
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    x = nlpe_->getSolution()->getPrimal();

    for (v_iter=minlp_->varsBegin(); v_iter!=minlp_->varsEnd();
         ++v_iter) {
      v_type=(*v_iter)->getType();
//      std::cout << "x[" << (*v_iter)->getIndex()+1 << "] = " << value << ", "; 
      if (v_type==Binary || v_type==Integer) {
        value = x[(*v_iter)->getIndex()];
        if (fabs(value-floor(value+0.5)) > intTol_) {
          is_int_feas = false;
//          std::cout << "            ~is_int_feas\n";
          break;
        }
      }
    }
//    std::cout << "\n";
/*
    for (int i=0; i < rel_->getNumVars(); i++)
    {
//      v = minlp_->getVariable(i);
//      if (v->getType()==Binary || v->getType()==Integer)
        std::cout << "x[" << i+1 << "] = " << x[i] << ", ";
    }
    std::cout << std::endl;
*/
    nlpval = nlpe_->getSolutionValue();
//    std::cout << "nlpval = " << nlpval << "  lp_obj = " << lp_obj << "\n";
    if (is_int_feas) {
      updateUb_(s_pool, &nlpval, sol_found);
      if (lp_obj > nlpval-solAbsTol_ ||
          lp_obj > nlpval - (fabs(nlpval)+1e-10)*solRelTol_){
        *status = SepaPrune;
        //std::cout << " *prune\n";
        break;
      } else {
          *status = SepaPrune;
          break;
        }
    } else {
//      std::cout << "integer infeasible 1\n";
      alphaCut = OAFromPoint_(x, sol, status);
      if (alphaCut == 0)
        ncut_int = ProjCutFromPoint_(sol, s_pool, sol_found, status);

      //*status = SepaContinue;
    }
    //rootSol_ = x;
    //BrootSol_ = true;
    // change lb of objective.
    // rel_->changeBound(objVar_, Lower, nlpe_->getSolutionValue());
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    *status = SepaPrune;
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
                                 << nlpe_->getStatusString() << std::endl;
    assert(!"quasiQGHandler: Cannot proceed further");
//    ++(stats_->nlpI);
//    *status = SepaPrune;
//    break;

  }
  undoLPBounds_(&mods);
/*
  if (*status == SepaContinue)
    std::cout << "SepaContinue\n";
  else if (*status==SepaPrune)
    std::cout << "SepaPrune\n";
  else if (*status==SepaResolve)
    std::cout << "SepaResolve\n";
  else std::cout << "oza kharabe\n";
*/

}

void quasiQGHandler::solveNLP_()
{
  //minlp_->write(std::cout);
  nlpe_->clear();
  //minlp_->calculateSize();

  /*
  if (BincSol_) {
    minlp_->setInitialPoint(incSol_);
  } else if (BLPrelSol_) {
      minlp_->setInitialPoint(LPrelSol_);
    } else if (BrootSol_) {
        minlp_->setInitialPoint(rootSol_);
    }
  */  
  nlpe_->load(minlp_);
  nlpStatus_ = nlpe_->solve();
  
  ++(stats_->nlpS);
}

void quasiQGHandler::solveProjection_()
{


  //proj_minlp_->calculateSize(true);
  //HessianOfLagPtr hPtr = (HessianOfLagPtr) new HessianOfLag(proj_minlp_.get());
  //proj_minlp_->setHessian(hPtr);

  /*
  if (BincSol_) {
    proj_minlp_->setInitialPoint(incSol_);
  } else if (BLPrelSol_) {
      proj_minlp_->setInitialPoint(LPrelSol_);
    } else if (BrootSol_) {
        proj_minlp_->setInitialPoint(rootSol_);
    }
  */

  proj_nlpe_->load(proj_minlp_);
  projnlpStatus_ = proj_nlpe_->solve();
  ++(stats_->nlpP);

}
int quasiQGHandler::OAFromPoint_(const double *x, ConstSolutionPtr sol,
                             SeparationStatus *status)
{
  int ncut = OAFromPoint_(x, sol->getPrimal(), status);
  relobj_ = sol->getObjValue();
  return ncut;
}

int quasiQGHandler::OAFromPoint_(const double *x, const double *inf_x,
                             SeparationStatus *status)
{
  //SeparationStatus *dummy_status = SepaContinue;

  /*
   * x is the solution of fixed NLP ==> act = g(x), if  act <= u then the constraint is satisfied.
   * sol is the pointer to the solution of relaxation
   */

  double act=-INFINITY, nlpact = -INFINITY; //lpvio = -INFINITY,lpact=-INFINITY, cuteval = -INFINITY  
  double nlpvio = -INFINITY;
  ConstraintPtr con, newcon;
  double c;
  LinearFunctionPtr lf = LinearFunctionPtr(); // NULL
  FunctionPtr f, f2;
  double *gr = new double[numvars_];
  //LinearCutPtr cut;
  ObjectivePtr o;
  UInt num_cuts = 0;
  int error;
  double lpact=0.0;
  double slack=0.0;
  int ncut = 0;
  *status=SepaContinue;
#if SPEW
    logger_->msgStream(LogDebug) 
        << me_ << "Adding linearizations, inf_x = " << inf_x << std::endl;
    logger_->msgStream(LogDebug)
        << me_ << "number of cuts so far = " << stats_->cuts << std::endl;
#endif

  std::vector<ConstraintPtr> conVect;
  int n = 0;
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it; 
    f = con->getFunction();
    act = con->getActivity(x, &error);
    // g(x) <= 0
    if(con->getUb() < INFINITY) {
        nlpact = con->getActivity(x, &error);  
        nlpvio = nlpact - con->getUb();
        if (!handleQuasiConvex_){ 
          linearAt_(f, act, x, &c, &lf);
          f2 = (FunctionPtr) new Function(lf);
          lpact = f2->eval(inf_x, &error);
	  slack = lpact - con->getUb() + c;
          if ( slack > linCoeffTol_){
            ++ncut;
	    if (ctMngr_){
	      newcon = ctMngr_->addCut(rel_,f2,-INFINITY,con->getUb()-c,true,false);
	    }else{
              newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c, "lnrztn_cut");
              conVect.push_back(newcon);
	    }
	    ++(stats_->cuts);
            ++num_cuts;
            *status = SepaResolve;
	  }
          if (doACCPM_)
          {
//            lf->evalGradient(gr);
//            qgACP_->addCut(gr, con->getUb()-c, true);
          }
        } else { 
	  nlpact = con->getActivity(x, &error);  
          nlpvio = nlpact - con->getUb();
	  if (nlpvio > -solAbsTol_) {
            linearAt_(f, 0.0 , x, &c, &lf);
            f2 = (FunctionPtr) new Function(lf);
            lpact = f2->eval(inf_x, &error);
  	    slack = lpact + c;
            //std::cout << "slack = " << slack << "\n";
            if ( slack > solAbsTol_){ //linCoeffTol_){
              ++ncut;
	      if (ctMngr_)
	        newcon = ctMngr_->addCut(rel_,f2,-INFINITY, -c,true,false);
	      else{
                newcon = rel_->newConstraint(f2, -INFINITY, -c , "lnrztn_cut");
                conVect.push_back(newcon);
	      }
	      ++(stats_->cuts);
              ++num_cuts;
              *status = SepaResolve;
            }
            //Add the new cut to analytic center problem
            if (doACCPM_)
            {
//              lf->evalGradient(gr);
//              qgACP_->addCut(gr, nlpact-c, true);
            }
          }
        }
    }
    // 0 <= g(x)
    if(con->getLb() > -INFINITY) {
        if ( !handleQuasiConvex_) {
          linearAt_(f, act, x, &c, &lf);
          f2 = (FunctionPtr) new Function(lf);
	  lpact = f2->eval(inf_x, &error);
  	  slack = lpact - con->getLb()+ c;
          if ( slack < solAbsTol_){
            ++ncut;
	    if (ctMngr_)
	      newcon = ctMngr_->addCut(rel_,f2,con->getLb()-c,INFINITY,true,false);
	    else{
              newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY, "lnrztn_cut");
              conVect.push_back(newcon);
	    }
	    ++(stats_->cuts);
            if (doACCPM_)
            {
//            lf->evalGradient(gr);
//            qgACP_->addCut(gr, con->getLb()-c, false);
            }
            ++num_cuts; 
            *status = SepaResolve;
          }
        } else  { 
          nlpact = con->getActivity(x, &error);
          nlpvio = con->getLb() - nlpact;
          if (nlpvio > -solAbsTol_) {
            linearAt_(f, 0.0, x, &c, &lf);
            f2 = (FunctionPtr) new Function(lf);
  	    lpact = f2->eval(inf_x, &error);
  	    slack = lpact + c;
            if ( slack > solAbsTol_){
              ++ncut;
	      if (ctMngr_)
	        newcon = ctMngr_->addCut(rel_,f2,-c,INFINITY,true,false);
	      else{
                newcon = rel_->newConstraint(f2, -c, INFINITY, "lnrztn_cut");
                conVect.push_back(newcon);
	      }
	      ++(stats_->cuts);
//            newcon->write(std::cout);
            //Add the new cut to analytic center problem
              if (doACCPM_)
              {
//              lf->evalGradient(gr);
//              qgACP_->addCut(gr, c - nlpact, false);
              }
              ++num_cuts;
              *status = SepaResolve;
            }
          }
        }
    }

  }
  
#ifdef DEBUG2
  std::cout << "objective\n";
#endif

  o = minlp_->getObjective();
  if (oNl_ && o) {
    f = o->getFunction();
    double vio;
    if (inf_x) {
      act = o->eval(inf_x, &error);
      vio = std::max(act-relobj_, 0.);
    } else {
      vio = 1.;
    }
    if (vio > solAbsTol_) {
      linearAt_(f, act, inf_x, &c, &lf);
      lf->addTerm(objVar_, -1.);
      f2 = (FunctionPtr) new Function(lf);
      lpact = f2->eval(inf_x, &error);
      if (ctMngr_)
        newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-1.0*c,true,false);
      else{
        newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut"); 
        conVect.push_back(newcon);
      }
      slack = lpact - newcon->getUb();
      if ( slack > solAbsTol_){
        ++ncut;
      }
      ++(stats_->cuts);
      if (doACCPM_)
      {
//        lf->evalGradient(gr);
//        qgACP_->addCut(gr, -1.0*c, true);
      }

#if SPEW
      logger_->msgStream(LogDebug) << me_ << "New inequality:" << std::endl;
      newcon->write(logger_->msgStream(LogDebug));
      logger_->msgStream(LogDebug) << std::endl;
      logger_->msgStream(LogDebug) << me_ << "obj violation = " 
                                   << vio << std::endl;
#endif
      ++num_cuts;
      *status = SepaResolve;

    }

  }
/*
  o = minlp_->getObjective();
  if (oNl_ && o) {
//    std::cout << "objective\n";
    f = o->getFunction();
#if 0
//	 Jim: Don't think we should be checking violation of lp solution

    if (inf_x) {
      act = o->eval(inf_x, &error);
      std::cout << "act = " << act << "\n";
      vio = std::max(act-sol->getObjValue(), 0.);
    } else {
      vio = 1.;
    }
    if (vio > eTol_) {

#endif
#ifdef DEBUG2
      std::cout << "vio of obj = " << vio <<  "\n";
#endif
//      act = o->eval(x, &error);
      act = o->eval(inf_x, &error);
      
      linearAt_(f, act, inf_x, &c, &lf);
//      linearAt_(f, act, inf_x, &c, &lf);
      lf->addTerm(objVar_, -1.);
      //cut = (LinearCutPtr) new LinearCut(lf, -1.0*c);
      f2 = (FunctionPtr) new Function(lf);
      newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut");
      conVect.push_back(newcon);
      //Add the new cut to analytic center problem
      if (doACCPM_)
      {
        lf->evalGradient(gr);
        qgACP_->addCut(gr, -1.0*c, true);
      }

#if SPEW
      logger_->msgStream(LogDebug) << me_ << "New inequality:" << std::endl;
      newcon->write(logger_->msgStream(LogDebug));
      logger_->msgStream(LogDebug) << std::endl;
      logger_->msgStream(LogDebug) << me_ << "obj violation = " 
                                   << vio << std::endl;
#endif
      ++num_cuts;
      *status = SepaResolve;

//    }

//  }

  }
*/
  double nlpact1;
  int numvio = 0;
  for ( UInt i=0; i < conVect.size(); i++){
    nlpact1 =  conVect[i]->getActivity(inf_x, &error);
    if ( (nlpact1 - conVect[i]->getUb() > solAbsTol_) || (conVect[i]->getLb() - nlpact1 > solAbsTol_ ) )
    {
      ++numvio;
    }
  }
  if (numvio == 0)
    num_cuts=0;
  if (num_cuts == 0 && cuttingPoint_ == true)
  {
    double *x_alpha = new double[numvars_];
    cuttingPoint_ = false;
    for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it)
    { 
      con = *it;
      act = con->getActivity(inf_x, &error);
      if (con->getUb() < INFINITY && act-con->getUb() > solAbsTol_) 
      { 
        //std::cout << "  cutXLP_  <=";
        cutXLP_(x, inf_x, x_alpha, con, true);
        stats_->looseCuts += 1;
        OAFromPoint_(x_alpha, inf_x, status);
      } else if (con->getLb() > -INFINITY && con->getLb()-act > solAbsTol_) 
      {
        //std::cout << "  cutXLP_  >=";
        cutXLP_(x, inf_x, x_alpha, con, false);
        stats_->looseCuts += 1;
        OAFromPoint_(x_alpha, inf_x, status);
      }
    }
    o = minlp_->getObjective();
    if (oNl_) { // && o) {
      cutXLPObj_(x, inf_x, x_alpha, o->getFunction());
      *status = SepaResolve;
    }
  }


#if SPEW
  logger_->msgStream(LogDebug) << "Number of cuts = " << num_cuts << std::endl;
#endif
  //stats_->cuts += num_cuts;
//  if (num_cuts != 0)
//    *status = SepaResolve;
//  else *status = SepaPrune;
//  rel_->write(std::cout);
//  if (numVilations == 0)
//    *status = 

//  std::cout << "ncut = " << ncut << "\n";
//  if (ncut == 0){
//    assert(!"Gradient inequalities did not cut off x_lp\n");
//  } // else
//    std::cout << "\nncut = " << ncut << "\n";
    if (ncut == 0)
    {
      for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it){
        con = *it;
	act = con->getActivity(inf_x,&error);
//	std::cout << std::setprecision(10) << con->getLb() << ",  " << act << ",  " << con->getUb() << "\n";
//	if (act > con->getUb() + solAbsTol_ || act < con->getLb() - solAbsTol_) {
//	  std::cout << "constraint not feasible\n";
//       }
      }     
    }
#ifdef DEBUG2    
  std::cout << "num_cuts = " << num_cuts;
  std::cout << "  end of OAFromPoint_\n";
  if (*status == SepaContinue)
    std::cout << "SepaContinue\n";
  else if (*status==SepaPrune)
    std::cout << "SepaPrune\n";
  else if (*status==SepaResolve)
    std::cout << "SepaResolve\n";
  else std::cout << "oza kharabe\n";
  std::cout << "OAFromPoint finished\n";
#endif
//  delete [] gr;
//  delete [] x_alpha;
  return ncut;
}

int quasiQGHandler::OAFromPointInf_(const double *x, const double *inf_x, 
                            SeparationStatus *status)
{
  //int a=0;
  int ncuts = 0;

  double act=-INFINITY, nlpact = -INFINITY; //lpvio = -INFINITY,lpact=-INFINITY, cuteval = -INFINITY  
  double lpact;
  double nlpvio = -INFINITY;
  ConstraintPtr con, newcon;
  double c;
  LinearFunctionPtr lf = LinearFunctionPtr(); // NULL
  FunctionPtr f, f2;
  double *gr = new double[numvars_];
  //LinearCutPtr cut;
  ObjectivePtr o;
  int error;
//  const double *inf_x = sol->getPrimal();
  

  *status=SepaContinue;

  std::vector<ConstraintPtr> conVect;
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it; 
    f = con->getFunction();
    act = con->getActivity(x, &error);
    // g(x) <= 0
    if(con->getUb() < INFINITY) {
        nlpact = con->getActivity(x, &error);  
        nlpvio = nlpact - con->getUb();
        if (!handleQuasiConvex_){ 
          linearAt_(f, act, x, &c, &lf);
          f2 = (FunctionPtr) new Function(lf);
          lpact = f2->eval(inf_x, &error);
          if (lpact - con->getUb() + c > solAbsTol_) 
          {
	    if (ctMngr_)
	      newcon = ctMngr_->addCut(rel_,f2,-INFINITY,con->getUb()-c,true,false);
	    else{
              newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c, "lnrztn_cut");
              conVect.push_back(newcon);
	    }
	    ++(stats_->cuts);

            if (doACCPM_)
            {
              lf->evalGradient(gr);
//              qgACP_->addCut(gr, con->getUb()-c, true);
            }
            ++ncuts;
            *status = SepaResolve;
          }
        } else { 
	  nlpact = con->getActivity(x, &error);  
          nlpvio = nlpact - con->getUb();
	  if (nlpvio > -solAbsTol_) {
            linearAt_(f, 0.0, x, &c, &lf);
            f2 = (FunctionPtr) new Function(lf); 
            lpact = f2->eval(inf_x, &error);
            if (lpact + c > solAbsTol_)
            {
	      if (ctMngr_)
	        newcon = ctMngr_->addCut(rel_,f2,-INFINITY, -c,true,false);
	      else{
                newcon = rel_->newConstraint(f2, -INFINITY, -c , "lnrztn_cut");
                conVect.push_back(newcon);
	      }
	      ++(stats_->cuts);
            //Add the new cut to analytic center problem
              if (doACCPM_)
              {
                lf->evalGradient(gr);
//                qgACP_->addCut(gr, nlpact-c, true);
              }
              ++ncuts;
              *status = SepaResolve;
            }  
//            std::cout << "num_cuts_ = " << num_cuts_ << "\n";
          }
        }
      
    }
    // 0 <= g(x)
//    std::cout << std::endl;

    if(con->getLb() > -INFINITY) {
        if ( !handleQuasiConvex_) {
          linearAt_(f, act, x, &c, &lf);
          f2 = (FunctionPtr) new Function(lf);
          lpact = f2->eval(inf_x, &error);
          if (lpact - con->getLb() + c < -solAbsTol_)
          {
//         linearAt_(f, act, inf_x, &c, &lf);
	    if (ctMngr_)
	      newcon = ctMngr_->addCut(rel_,f2,con->getLb()-c,INFINITY,true,false);
	    else {
              newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY, "lnrztn_cut");
              conVect.push_back(newcon);
	    }
	    ++(stats_->cuts);
  //          newcon->write(std::cout);
            //Add the new cut to analytic center problem
            if (doACCPM_)
            {
              lf->evalGradient(gr);
//              qgACP_->addCut(gr, con->getLb()-c, false);
            }
            ++ncuts; 
            *status = SepaResolve;
          }
        } else  { 
          nlpact = con->getActivity(x, &error);
//          nlpact = con->getActivity(x, &error);
          nlpvio = con->getLb() - nlpact;
//          std::cout << "nlpvio = " << nlpvio;
          if (nlpvio > -solAbsTol_) {
            linearAt_(f, 0.0, x, &c, &lf);
            f2 = (FunctionPtr) new Function(lf);
            lpact = f2->eval(inf_x, &error);
            if (lpact + c < -solAbsTol_)
            {
	      if (ctMngr_)
	        newcon = ctMngr_->addCut(rel_,f2,-c,INFINITY,true,false);
	      else {
                newcon = rel_->newConstraint(f2, -c, INFINITY, "lnrztn_cut");
                conVect.push_back(newcon);
	      }
	      ++(stats_->cuts);
//            newcon->write(std::cout);
            //Add the new cut to analytic center problem
              if (doACCPM_)
              {
                lf->evalGradient(gr);
//                qgACP_->addCut(gr, c - nlpact, false);
              }
              ++ncuts;
              *status = SepaResolve;
            } 
          }
        }
    }
//    std::cout << std::endl;

  }
  return ncuts;

}

void quasiQGHandler::cutXLP_(const double *x_nlp, const double *x_lp,
                        double *x_alpha, ConstraintPtr con, Bool dir)
{
  cutXLP_(x_nlp, x_lp, x_alpha, con->getFunction(), con->getUb(), con->getLb(), dir);
}

void quasiQGHandler::cutXLP_(const double *x_nlp, const double *x_lp,
                        double *x_alpha, FunctionPtr fn, double ub, double lb, Bool dir)
{
  numvars_ = minlp_->getNumVars();
  double *gr = new double[numvars_];

//  FunctionPtr fn = con->getFunction();
  FunctionPtr fn2;
  LinearFunctionPtr lf;
  ConstraintPtr newcon;

  double alpha_l = 0.0;
  double alpha_u = 1.0;
  Bool stop = false;
  double alpha;
  int error = 0.0;
  double innProd = 0.0;
  double eval_x_alpha = 0.0;
  double eval_x_lp;
  
  double inn_gr_xlp;
  double inn_gr_xalpha;

  double *xdiff = new double[numvars_];
  for (int i = 0; i < numvars_; i++) {
    xdiff[i] = x_lp[i] - x_nlp[i];
  }


  if (dir) {
    eval_x_lp = fn->eval(x_lp, &error)-ub;
    while (!stop) {
      alpha = 0.5 * (alpha_l + alpha_u);
      //std::cout << " alpha = " << alpha;
      std::fill(gr, gr+numvars_, 0.0);
      for (int i = 0; i < numvars_; i++) {
        x_alpha[i] = (1-alpha) * x_lp[i]+alpha * x_nlp[i];
      }
      fn->evalGradient(x_alpha, gr, &error);
      eval_x_alpha = fn->eval(x_alpha, &error)-ub;
      
      if (eval_x_alpha > -eLinTol_) {
//        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
//        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
//        innProd = inn_gr_xlp - inn_gr_xalpha;
        innProd = InnerProduct(gr, xdiff,numvars_);
        //std::cout << ", innProd = " << innProd;
        //std::cout << ", eval_x_alpha = " << eval_x_alpha;
        //std::cout << std::endl;
        if (innProd > -solAbsTol_ || alpha_u - alpha_l < 1e-8) {
//          std::cout << ", stoped\n";
          stop = true;
        }
      }
      if (eval_x_alpha > -solAbsTol_ || innProd < solAbsTol_) {
        if (eval_x_lp - eval_x_alpha > solAbsTol_){        
          alpha_u = alpha;
        } else {
          alpha_l = alpha;
        }
      }  
    }
  } else {
    eval_x_lp = fn->eval(x_lp, &error) - lb;
    while (!stop){
      alpha = 0.5 * (alpha_l + alpha_u);
      std::fill(gr, gr+numvars_, 0.0);
      for (int i = 0; i < numvars_; i++)
      {
        x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
      } 
      fn->evalGradient(x_alpha, gr, &error);
      if (eval_x_alpha < solAbsTol_)
      {
        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
        innProd = inn_gr_xlp - inn_gr_xalpha;
        if (innProd < -eLinTol_)
        {
          stop = true;
        }
      }
        if (eval_x_alpha < -solAbsTol_ || innProd < solAbsTol_)
        {
          if (eval_x_lp - eval_x_alpha > solAbsTol_)
          {
            alpha_u = alpha;
          } else alpha_l = alpha;
        } 

    }
  }
  delete [] xdiff;
}

void quasiQGHandler::cutXLP_(const double *x_nlp, const double *x_lp,
                         ConstraintPtr con, SeparationStatus *status, Bool dir)
{
  double *x_alpha = new double[numvars_];
  numvars_ = minlp_->getNumVars();
  double *gr = new double[numvars_];
//  double *x_diff = new double[numvars_];

  FunctionPtr fn = con->getFunction();
  FunctionPtr fn2;
  LinearFunctionPtr lf;// = LinearFunctionPtr();
  ConstraintPtr newcon;

  double alpha_l = 0.0;
  double alpha_u = 1.0;
  Bool stop = false;
  double alpha;
  int error = 0.0;
  double innProd = 0.0;
  double eval_x_alpha;
  double eval_x_lp;

  double inn_gr_xlp;
  double inn_gr_xalpha;
  double c;
#ifdef DEBUG5
  std::cout << "cutXLP_\n";
  std::cout << "x_lp ******\n";
  write_array(x_lp, minlp_->getNumVars(), "x_lp");
  std::cout << "x_nlp ******\n";
  write_array(x_nlp, minlp_->getNumVars(), "x_nlp");
#endif

  if (dir)
  {
    eval_x_lp= con->getActivity(x_lp,&error)-con->getUb();
#ifdef DEBUG5
    int n = 0;
#endif

    while (!stop)
    {
#ifdef DEBUG5
      n += 1;
#endif
      alpha = 0.5 * (alpha_l + alpha_u);
#ifdef DEBUG5
      std::cout << "alpha_l = " << alpha_l << ", alpha_u = " << alpha_u;
      std::cout << ", alpha = " << alpha << "\n";
#endif
      std::fill(gr, gr+numvars_, 0.0);
      for (int i = 0; i < numvars_; i++)
      {
        x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
//        std::cout << "x_a[" << i << "]=" << x_alpha[i] << ", ";
//        x_diff[i] = x_lp[i] - x_alpha[i];
      }
//      std::cout << "\n";
      fn->evalGradient(x_alpha, gr, &error);
#ifdef DEBUG5
      std::cout << "x_alpha ******\n";
      write_array(x_alpha, minlp_->getNumVars(), "x_alpha");
      std::cout << "gr ***********\n";
      write_array(gr, numvars_, "gr");
#endif
      eval_x_alpha = con->getActivity(x_alpha, &error) - con->getUb();
      if (eval_x_alpha > -eLinTol_)
      {
      inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
      inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
      innProd = inn_gr_xlp - inn_gr_xalpha;
#ifdef DEBUG5
      std::cout << "innProd = " << innProd << "\n";
#endif
      if (innProd > eLinTol_)
      {
        stop = true;
#ifdef DEBUG5
        std::cout << "alpha = " << alpha;
        std::cout << ", eval_xlp = " << eval_x_lp << ", eval_xnlp = " << con->getActivity(x_nlp,&error)-con->getUb() << ", eval_xalpha = " << con->getActivity(x_alpha,&error)-con->getUb()  << "\n"; 
#endif
//        if (!handleQuasiConvex_)
//        {

//        linearAt_(gr, &lf);
//        lf->write(std::cout);
//        linearAt_(fn, x_alpha, &lf);
        linearAt_(fn, 0.0, x_alpha, &c, &lf);
//        lf = linearAt_(fn, x_alpha);
        if (lf) {
          std::cout << "write lf:";
          lf->write(std::cout);
          fn2 = (FunctionPtr) new Function(lf);
          std::cout << "\nfn2 constructed\n";
        } else {
          std::cout << "lf not exists\n";
        }
//        std::cout << "\nwrite lf\n";
//        lf->write(std::cout);
//        std::cout << "\nwrite fn2\n";
//        linearAt_(fn, x_alpha, &fn2);
        if (fn2) 
        {
//          std::cout << "inn_gr_xalpha = " << inn_gr_xalpha << "\n";
//          std::cout << "fn2 exists\n";
//          fn2->write(std::cout);
//          std::cout << "\nlf of fn2\n";
//          fn2->getLinearFunction()->write(std::cout);
//          std::cout << "\n";
//          std::cout << "fn2 exists\n";
//          fn2->write(std::cout);
//          std::cout << "\nalaki\n";
//          newcon = minlp_->newConstraint(fn2, -INFINITY, inn_gr_xalpha, "lnrztn_cut");
	  if (ctMngr_)
	    newcon = ctMngr_->addCut(rel_,fn2,-INFINITY,inn_gr_xalpha,true,false);
	  else {
            newcon = rel_->newConstraint(fn2, -INFINITY, inn_gr_xalpha, "lnrztn");
	  }
	  ++(stats_->cuts);
//          std::cout << "constraint added\n";
        } else {
          std::cout << "no fn2 function\n";
        }
        if (doACCPM_)
        {
//          qgACP_->addCut(gr, inn_gr_xalpha, true);
        }
        *status = SepaResolve;
        //stats_->cuts += 1;
        stats_->looseCuts += 1;
//        } else {
//          linearAt_(fn, eval_x_alpha, x_alpha, &c, &lf);
//          fn2 = (FunctionPtr) new Function(lf);
//          newcon = rel_->newConstraint(fn2, -INFINITY, );
//        }
      }
      }
      if (eval_x_alpha < -eLinTol_ || innProd < eLinTol_)
      {
        if (eval_x_lp - eval_x_alpha > eLinTol_)
        {
          alpha_u = alpha;
        } else alpha_l = alpha;
      }
#ifdef DEBUG5
//      if (n == 10)
//        stop = true;
#endif
    }
  } else {
    eval_x_lp= con->getActivity(x_lp,&error) - con->getUb();
    while (!stop)
    {
      alpha = 0.5 * (alpha_l + alpha_u);
      std::fill(gr, gr+numvars_, 0.0);
      for (int i = 0; i < numvars_; i++)
      {
        x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
//        x_diff[i] = x_lp[i] - x_alpha[i];
      }
      fn->evalGradient(x_alpha, gr, &error);
//      innProd = InnerProduct(gr, x_diff, numvars_);
      if (eval_x_alpha < eLinTol_)
      {
        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
        innProd = inn_gr_xlp - inn_gr_xalpha;
        if (innProd < -eLinTol_)
        {
          stop = true;
          linearAt_(fn, 0.0, x_alpha, &c, &lf);
          fn2 = (FunctionPtr) new Function(lf);
	  if (ctMngr_)
	    newcon = ctMngr_->addCut(rel_,fn2,-INFINITY,inn_gr_xalpha,true,false);
          else {
            newcon = rel_->newConstraint(fn2, -INFINITY, inn_gr_xalpha, "lnrztn_cut");
	  }
	  ++(stats_->cuts);
          if (doACCPM_)
          {
//            qgACP_->addCut(gr, inn_gr_xalpha, true);
          }
          *status = SepaResolve;
          //stats_->cuts += 1;
          stats_->looseCuts += 1;
        }
      }
      else 
      {
        eval_x_alpha = con->getActivity(x_alpha, &error) - con->getUb();
        if (eval_x_lp - eval_x_alpha < -eLinTol_)
        {
          alpha_u = alpha;
        } else alpha_l = alpha;
      }
    }
  
  }
//  delete [] gr;
//  delete [] x_alpha;
}

void quasiQGHandler::cutXLPObj_(const double *x_nlp, const double *x_lp,
                           double *x_alpha, FunctionPtr fn)
{

  FunctionPtr fn2;
  LinearFunctionPtr lf = LinearFunctionPtr();
  ConstraintPtr newcon;
  double c;
  int error = 0;

  double eval_x_lp = fn->eval(x_lp, &error);

/*
  linearAt_(fn, eval_x_lp, x_lp, &c, &lf);
  lf->addTerm(objVar_, -1.);
  fn2 = (FunctionPtr) new Function(lf);
  newcon = rel_->newConstraint(fn2, -INFINITY, -1.0 * c, "objlnrztn_cut");
  
  double act = newcon->getActivity(x_lp, &error);
  std::cout << "nlpact lp= " << eval_x_lp << "\n";
  std::cout << "nlpact nlp= " << fn->eval(x_nlp, &error) << "\n";
  std::cout << "eta = " << x_lp[objVar_->getIndex()] << "\n";
  std::cout << "vio = " << act - newcon->getUb() << "\n";

*/
///*
  numvars_ = minlp_->getNumVars();
/*
  double *gr = new double[numvars_];
  double eta =  x_lp[objVar_->getIndex()];

  double alpha_l = 0.0;
  double alpha_u = 1.0;
  Bool stop = false;
  double alpha;
  double innProd = 0.0;
  double eval_x_alpha;
  double inn_gr_xalpha;
  double inn_gr_xlp;

  while (!stop)
  {
    alpha = 0.5 * (alpha_l + alpha_u);
    std::fill(gr, gr+numvars_, 0.0);
    for (int i = 0; i < numvars_; i++)
    {
      x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
    }
    fn->evalGradient(x_alpha, gr, &error);
    eval_x_alpha = fn->eval(x_alpha, &error);
    inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
    inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
    innProd = inn_gr_xlp - inn_gr_xalpha + eval_x_alpha;
    std::cout << ", innProd +eta= " << innProd - eta;
    if (innProd > eta + eLinTol_)
    {
      std::cout << ", alpha = " << alpha << "\n";
      stop = true;
    } else {
//    if (innProd < eLinTol_)
//    {
      if (eval_x_lp - eval_x_alpha > eLinTol_)
      {
        alpha_u = alpha;
      } else alpha_l = alpha;
    } 
  }
*/
  eval_x_lp = fn->eval(x_lp, &error);
  linearAt_(fn, eval_x_lp, x_lp, &c, &lf);
  lf->addTerm(objVar_, -1.);
  fn2 = (FunctionPtr) new Function(lf);
  if (ctMngr_)
    newcon = ctMngr_->addCut(rel_,fn2,-INFINITY,-1.0*c,true,false);
  else {
    newcon = rel_->newConstraint(fn2, -INFINITY, -1.0*c, "objlnrztn_cut");
  }
  ++(stats_->cuts);
  std::cout << "act of linearization = " << newcon->getActivity(x_lp,&error) - newcon->getUb() << "\n";
//  std::cout << "objective cut\n";
//  newcon->write(std::cout);
//  std::cout << "\n";
//*/
}

int quasiQGHandler::l1_project_(const double *x_OA, SeparationStatus *status)
{
  ConstraintPtr c,newcon;
  FunctionPtr fn;
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  const double *x = new double [l1proj_minlp_->getNumVars()]; 
  VariablePtr v;
  int i=0;
  double lhs = 0.0;
  double lpact = 0.0;
  int error;
  int ncut = 0;

//  mylogfile_ << "before change\n";
//  l1proj_minlp_->write(mylogfile_);
  //std::ofstream myfile0;
  //myfile0.open("mylogfile0.txt",std::ios::app);
  //myfile0 << "\n**************before change\n";
  //l1proj_minlp_->write(myfile0);
  //myfile0.close();

  l1proj_nlpe_->clear();
  for (CCIter it=l1_linConst_.begin(); it!=l1_linConst_.end(); ++it)
  {
    c = *it;
//    std::cout << "before:\n";
//    c->write(std::cout);
    l1proj_minlp_->changeBound(c, floor(x_OA[IntVarID_[i]]+0.5), floor(x_OA[IntVarID_[i]]+0.5));
//    std::cout << "after:\n";
//    c->write(std::cout);
    ++i;
  }
  
  std::ofstream myfile1;
  //myfile1.open("mylogfile1.txt",std::ios::app);
  


//  l1proj_minlp_->removeObjective();
//  l1proj_minlp_->newObjective(l1_proj_, 0, Minimize);
  l1proj_minlp_->calculateSize(true);
  l1proj_minlp_->setNativeDer();
  l1proj_nlpe_->load(l1proj_minlp_);
  l1proj_nlpe_->solve();
  ++(stats_->nlpP);

  //std::cout << "status of l1proj_nlpe = " << l1proj_nlpe_->getStatusString() << "\n";
  //std::cout << std::setprecision(10) << "l1proj opt value = " << l1proj_nlpe_->getSolutionValue() << "\n";
  x = l1proj_nlpe_->getSolution()->getPrimal();
  x_pr_ = l1proj_nlpe_->getSolution()->getPrimal();
  //l1proj_minlp_->write(std::cout);
//  for (int k=0; k < l1proj_minlp_->getNumVars(); ++k)
//  {
//    std::cout << " x[" << k+1 << "]=" << x[k] << ",";
//  }
//  std::cout << "\n";
/*
  if (stats_->p1Cuts==1){
    write_point(x_OA,x,rel_->getNumVars(),l1proj_minlp_->getNumVars());
    myfile1 << "\n**************after change\n";
    l1proj_minlp_->write(myfile1);
  }
  myfile0.close();
//  VariableConstIterator vbeg = minlp_->varsBegin();
 
  
  double a1=1.0;
  double epsi=1e-16;
  i=0;
  for (VariableConstIterator it = minlp_->varsBegin(); it!=minlp_->varsEnd(); ++it)
  {
    v=*it;
    if (v->getType()==Binary || v->getType()==Integer)
    {
      i=v->getIndex();
      if (x[i] < x_OA[i]-epsi)
      {
        lf->addTerm(v,-1.0);
        lhs += -1.0 * x[i];
      } else if (x[i] > x_OA[i]+epsi)
      {
        lf->addTerm(v,1.0);
        lhs += 1.0 * x[i];
      } else if (x[i] <= x_OA[i]+epsi && x[i] >= x_OA[i]-epsi)
      {
        lf->addTerm(v,a1);
        lhs += a1*x[i];
      }
    }
  }
*/
/*
  for (UInt j=0; j<l1_linConst_.size(); ++j)
  {
    i = IntVarID_[j];
    v = minlp_->getVariable(i);
    if (x[i] < x_OA[i]-solAbsTol_)
    {
      lf->addTerm(v, -1.0);
      lhs += -1.0 * x[i];
    } 
    if (x[i] > x_OA[i]+solAbsTol_)
    {
      lf->addTerm(v, 1.0);
      lhs += x[i];
    } 
  }
*/

/*
  j=0;
  for (VariablePtr v = l1IntVars.begin(); v!=l1IntVars.end(); ++v)
  {
    i = IntVarID_[j];
    if (x[i] <= x_OA[i]+solAbsTol_)
    {
      lf->addTerm(v, -1.0*x[i]);
      lhs += -1.0 * x[i];
    } else if (x[i] > x_OA[i] + solAbsTole_)
      {
        lf->addTerm(v, 1.0*x[i]);
        lhs += x[i];
      }
  }
*/
/*
  fn = (FunctionPtr) new Function(lf);
  lpact = fn->eval(x_OA, &error);
  std::cout << std::setprecision(10) << "lpact = " << lpact << ", lhs = " << lhs << "\n";
  mylogfile_ << std::setprecision(10) << "lpact = " << lpact << ", lhs = " << lhs << "\n";
//  newcon = rel_->newConstraint(fn, floor(lhs+0.5)+1, INFINITY, "l1_projection");
  if (lpact - lhs < -solAbsTol_)
  {
//    newcon = rel_->newConstraint(fn, ceil(lhs), INFINITY, "l1_projection");
    newcon = rel_->newConstraint(fn, lhs, INFINITY, "l1_projection");
    newcon->write(std::cout);
    ++ncut;
    *status = SepaResolve;
  }
*/
/*
  for (CCIter it=l1_linConst_.begin(); it!=l1_linConst_.end(); ++it)
  {
    c = *it;
    l1proj_minlp_->changeBound(c, -INFINITY, INFINITY);
    ++i;
  }
*/
  if (stats_->p1Cuts==1) {
//    assert(!"done");
  }
  ncut = OAFromPoint_(x,x_OA, status);
//  ncut=1;
  return ncut; 
}
int quasiQGHandler::ProjCutFromPoint_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, Bool *sol_found,
                                  SeparationStatus *status)
{
  double c = 0.0;
  double constantTerm = 0.0;
  int error = 0;
  num_cuts_ = 0;

  LinearFunctionPtr lf = LinearFunctionPtr(); //Null
  FunctionPtr f2 = (FunctionPtr) new Function(lf);
  FunctionPtr f;

  ConstraintPtr newcon;
  VariableConstIterator vbeg = minlp_->varsBegin();
  VariableConstIterator vend = minlp_->varsEnd();
  VariablePtr v;
  VariableConstIterator v_iter;
  Bool int_feas = false;
  VariableType v_type;
  double value; 
  double nlpval;
  double lpval;
  double varUb;
  double beta = 0.0;
  int ncut=0;

  x_OA_ = sol->getPrimal();
  ProjObjUpdate(x_OA_, constantTerm);
  solveProjection_();
  double *xhat = new double[numvars_];
  ConstraintPtr cons;
  double act;
  Bool isfeas=true;
#if SPEW
     logger_->msgStream(LogDebug) 
        << me_ << "solved projection" << std::endl;
#endif

  switch (projnlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    x_pr_ = proj_nlpe_->getSolution()->getPrimal();
#ifdef NewPOINTPROBLEM
    if (ECP_){
      addPointToProb_(x_pr_);
      quasiSol_->addSol(x_pr_);
    }
#endif
    for (v_iter=minlp_->varsBegin(); v_iter!=minlp_->varsEnd();++v_iter)
    {
      v_type = (*v_iter)->getType();
      if (v_type==Binary || v_type==Integer) {
        value = x_pr_[(*v_iter)->getIndex()];
        if (fabs(value-floor(value+0.5)) > intTol_) {
          int_feas = false;
          break;
        }
      }
    } 
    if (int_feas) {
      //updateUb_(s_pool, &nlpval, sol_found);
    }
    if (env_->getOptions()->findBool("GradLin")->getValue()==false)
    {
      //std::cout << "no gradient *********************\n";
      for (int i =0; i < numvars_; i++)
      {
        v = proj_minlp_->getVariable(i);
        if ( v->getType() == Binary || v->getType() == Integer )
        {
          a_[i] = x_OA_[i] - x_pr_[i] ;
          xhat[i] = x_OA_[i];
          if (fabs(a_[i]) < solAbsTol_){
            if (a_[i] < -1e-12) {
              varUb = v->getUb();
              beta += -a_[i] * varUb;
            }
            a_[i] = 0.0;
          }
        }else {
          a_[i] = 0.0;
          xhat[i]=x_pr_[i];
        }
      }
      isfeas = true;
      for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
         ++it) {
        cons=*it;
        act=cons->getActivity(xhat,&error);
        if (act > cons->getUb() + 1e-6 || act < cons->getLb() - 1e-6) {
          isfeas=false;
          break;
        }
      }
      if (isfeas)
      {
        updateUb_new_(s_pool, &nlpval, xhat, sol_found);
      }
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "projection objective = " << proj_nlpe_->getSolutionValue() + .5 * constantTerm << std::endl;
#endif
    //std::cout << std::setprecision(9) << "projection objective = " << proj_nlpe_->getSolutionValue() + .5 * constantTerm << std::endl;
//      if (proj_nlpe_->getSolutionValue() + .5 * constantTerm > solAbsTol_)
//      {
        lf = (LinearFunctionPtr) new LinearFunction(a_, vbeg, vend, linCoeffTol_); 
        c = InnerProduct(a_, x_pr_, numvars_);
        f2 = (FunctionPtr) new Function(lf);
        lpval = f2->eval(x_OA_,&error);
//        if (lpval-c-beta > solAbsTol_){
	//std::cout << std::setprecision(9) << "********************violation = " << lpval - c << "\n";
        if (lpval-c - beta > solAbsTol_){
	  //std::cout << " cut added ";
          ++ncut;
	  if (ctMngr_)
	    newcon = ctMngr_->addCut(rel_,f2,-INFINITY,c+beta,true,false);
	  else {
            //newcon = rel_->newConstraint(f2, -INFINITY, c + beta, "projlnrztn_cut");
            newcon = rel_->newConstraint(f2, -INFINITY, c+beta , "projlnrztn_cut");
	  }
	  ++(stats_->cuts);
        } //else {std::cout << "no cut added\n";}
        if (doACCPM_)
        {
//        qgACP_->addCut(a_, c, false);
        }
//      }
 
    }else{
      //std::cout << "gradient *********************\n";
	//std::cout << " go to gradlin ";
      ncut = OAFromPoint_(x_pr_,x_OA_, status);
    }
//#ifdef DEBUG4
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
  case (ProvenUnbounded):
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "projNLP engine status = " 
                                 << proj_nlpe_->getStatusString() << std::endl;
    assert(!"quasiQGHandler: Cannot proceed further");
  }

///*


#if SPEW1    
    double * gr = new double [numvars_];
    for (int i = 0; i < n; i++)
      gr[i] = 0.0;
    proj_minlp_->getObjective()->evalGradient(x_pr_, gr, &error);

    for (int i = 0; i < numvars_; i++)
    {
      v = proj_minlp_->getVariable(i);
      if (v->getType() == Binary || v->getType() == Integer) {
        if ( fabs(gr[i] + x_OA_[i] - x_pr_[i]) > linTol)
         std::cout << "Dgr[" << i+1 <<  "] = " << gr[i] + x_OA_[i] - x_pr_[i] << ", ";
        } 
//        std::cout << "Dgr[" << i+1 <<  "] = " << gr_[i] << ", ";
      else
      {
        if (fabs(gr[i]) > 0.000001)
          std::cout << "Cgr[" << i+1 << "] = " << gr[i] << ", ";
      }

    }
//    delete [] gr;
#endif
#if SPEW  
    logger_->msgStream(LogDebug) << me_ << "optimal solutions:" << std::endl;
    for (int i = 0; i < numvars_; i++)
      if (proj_minlp_->getVariable(i)->getType() == Binary || proj_minlp_->getVariable(i)->getType() == Integer)
        logger_->msgStream(LogDebug) << "(" << i+1 << "," << x_OA_[i] << "," << x_pr_[i] << ") "; 
    logger_->msgStream(LogDebug) << std::endl;
#endif
    //c = InnerProduct(a_, x_pr_, n);
//    std::cout << "f2(x) = " << lf->eval(x_OA_) << " c = " << c << std::endl;
#if SPEW
    std::cout.precision(8);
    logger_->msgStream(LogDebug) << me_ << "projection objective value = " << proj_minlp_->getObjValue(x_pr_, &error) + 0.5*constantTerm << std::endl;
    std::cout.precision(4);
#endif
///*
//    if (InnerProduct(x_OA_, a_, n)-c >eLinTol_)
//*/
//#endif

#ifdef DEBUG4
  std::cout << "num_cuts_ = " << num_cuts_ << "\n";
#endif
  if (ncut > 0){ 
    //++stats_->cuts;
    *status = SepaResolve; 
  }
  else
  {
  double funeval; 
  double consact;
/*
  std::cout << "Function evals: \n";
  for (UInt i = 0; i < minlp_->getNumCons(); i++){
    funeval = proj_minlp_->getConstraint(i)->getFunction()->eval(x_OA_, &error);
    consact = funeval - minlp_->getConstraint(i)->getUb();
    if (consact > solAbsTol_){
      std::cout << "Uconst[" << i << "] = " << consact << "\n";
      proj_minlp_->getConstraint(i)->write(std::cout);
    }  
    consact = funeval - proj_minlp_->getConstraint(i)->getLb();
    if (consact < -1.0 * solAbsTol_){
      std::cout << "Bconst[" << i << "] = " << consact << "\n";
      proj_minlp_->getConstraint(i)->write(std::cout);
    }
  }
*/

  //write_point(x_OA_,x_pr_, minlp_->getNumVars());
#if SPEW
 logger_->msgStream(LogDebug2) << me_ << "Function evals: \n";
  for (UInt i = 0; i < minlp_->getNumCons(); i++){
    funeval = minlp_->getConstraint(i)->getFunction()->eval(x_OA_, &error);
    consact = funeval - minlp_->getConstraint(i)->getUb();
    if (consact > solAbsTol_){
      logger_->msgStream(LogDebug2) << "Uconst[" << i << "] = " << consact << "\n";
    }  
    consact = funeval - minlp_->getConstraint(i)->getLb();
    if (consact < -1.0 * solAbsTol_){
      logger_->msgStream(LogDebug2) << "Bconst[" << i << "] = " << consact << "\n";
    }
  }
#endif

//    assert(!"projection did not add any cut\n");
//    *status = SepaPrune;
  }
//  std::cout << "projection cut was added" << std::endl;
//  delete [] init_x;
//  delete [] x_sol;
//  delete [] x;
//  delete [] a;
//  delete [] x_pr; 
//  delete [] x_OA;
//  delete [] gr;
  proj_nlpe_->clear();
  return ncut;
//  std::cout << "arrays deleted" << std::endl;
}

void quasiQGHandler::strCutsCvx_(ConstSolutionPtr sol, const double *xnlp)
{
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  const double *x = sol->getPrimal();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];

  double nlpact0 = 0.0;
  double nlpact1 = 0.0;
  double nlpact2 = 0.0;
  double cnst = 0.0;
  LinearFunctionPtr lf;
  FunctionPtr f,f2;

  double alphal = 0.0;
  double alphau = 1.0;
  double alpha = 0.0;
  double alphaC = 0.0;
  double adAlpha = 0.0;
  Bool stop = false;
  Bool stop1 = false;
  int error = 0;
  double gnorm = 0.0;
  double *dir = new double[numvars];
  int ncut = 0;
//  int n1 = 0;
//  int iter = 0;
//  for (int i=0; i < numvars; i++)
//  {
//    gr[i] = xnlp[i] - x[i];
//  }

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it){
    con = *it;
    std::fill(gr,gr+numvars,0.0);
    con->getFunction()->evalGradient(x,gr,&error);
    nlpact1 = con->getActivity(x,&error);
    if (nlpact1 > solAbsTol_ + con->getUb()){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] + gr[i]/gnorm;
    }
    if (nlpact1 < con->getLb()-solAbsTol_){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] - gr[i]/gnorm;
    }  
  }
  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
//    std::cout << "n1 = " << n1++ << "\n";
    con = *it;
    nlpact1 = con->getActivity(x,&error);
//    nlpact2 = con->getActivity(xnlp, &error); 
    if (nlpact1 > solAbsTol_ + con->getUb())
    {
//      std::cout << "inside if 1\n";
//      std::fill(gr, gr+numvars, 0.0); 
//      con->getFunction()->evalGradient(x,gr,&error);
      alphal = 0.0; alphau = 1.0;  stop = false; stop1 = false;  
//      iter = 0;
/*
      stop1 = true;
      while (stop1==false) // && iter < 5)
      {
//	iter++;
	alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];	
	nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 > con->getUb() + solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
//      std::cout << "end while loop 1\n";
      alphal = alphau - 1.0;
*/
      adAlpha = INFINITY;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alphau * dir[i];
	
        nlpact0 = con->getActivity(xalpha,&error);
      std::cout << "nlpact0 =" << nlpact0 - con->getUb() << "\n";
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
	
        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 > con->getUb() + solAbsTol_ && nlpact0 < nlpact1 - solAbsTol_) {
	  adAlpha = alpha;
	  alphau = alpha;
	} else if (nlpact0 > con->getUb() + solAbsTol_){
  	    alphau = alpha;
	} else if (nlpact0 < con->getUb() - solAbsTol_) {
	    alphal = alpha;
	} else {
	    stop = true; adAlpha = alpha;
   	}
      }
//      std::cout << "end while loop 2\n";
      if (stop == true){
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
      }
      if (adAlpha < INFINITY){
        f = con->getFunction();
        linearAt_(f,f->eval(xalpha,&error),xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error)+cnst -con->getUb() > solAbsTol_){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2, -INFINITY, con->getUb()-cnst,true,false);
	  else {
            newcon = rel_->newConstraint(f2, -INFINITY,con->getUb()-cnst,"ExtraCuts");
	  }
	  ++(stats_->cuts);
	  std::cout << "exxxxx\n";
	  ncut++;
        }
      }
    }

    if (nlpact1 < con->getLb()-solAbsTol_)
    {
//      std::cout << "inside if 2\n";
      alphal = 0.0;
      alphau = 1.0;
      stop = false;
      stop1 = false;
//      std::fill(gr, gr+numvars, 0.0);
//      con->getFunction()->evalGradient(x,gr,&error);
/*
      stop1 = true;
      while (stop1 == false)
      {
        alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_){
          alpha++;
        } else stop1 = true;
      }
      alphal = alphau - 1.0;
*/
	
      adAlpha = INFINITY;
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_ && nlpact0 > nlpact1 + solAbsTol_){
	  adAlpha = alpha;
	  alphau = alpha;
        } else if (nlpact0 < con->getLb() - solAbsTol_) {
	    alphau = alpha;
	} else if (nlpact0 > con->getUb() + solAbsTol_) {
	    alphal = alpha;  
	} else {
	  stop = true;
	  adAlpha = alpha;
	}
      }
      if (stop == true) {
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
      }
      if (adAlpha < INFINITY){
        f = con->getFunction();
        linearAt_(f,f->eval(xalpha,&error),xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error) + cnst - con->getLb() < -solAbsTol_){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2,con->getLb() -cnst,INFINITY,true,false);
	  else {
            newcon = rel_->newConstraint(f2,con->getLb() -cnst, INFINITY, "ExtraCuts");
	  }
	  ++(stats_->cuts);
//      } 
	  std::cout << "exxxxx\n";
 	  ncut++;
        }
      }
    }
  }
//  std::cout << "strengthened ncut = " << ncut << "\n";
//  delete [] x;
  delete [] xalpha;
  delete [] gr;
  delete [] dir;
}

int quasiQGHandler::strCutsQuasi_(ConstSolutionPtr sol, const double *xnlp)
{
  timer_->start();
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  const double *x = sol->getPrimal();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];

  double nlpact0 = 0.0;
  double nlpact1 = 0.0;
  double nlpact2 = 0.0;
  double nlpactm = 0.0;
  double cnst = 0.0;
  LinearFunctionPtr lf;
  FunctionPtr f,f2;

  double alphal = 0.0;
  double alphau = 1.0;
  double alpha = 0.0;
  double alphaC = 0.0;
  double adAlpha = 0.0;
  Bool stop = false;
  Bool stop1 = false;
  int error = 0;
  double gnorm = 0.0;
  double *dir = new double[numvars];
  int ncut = 0;

//  int n1 = 0;
//  int iter = 0;
//  for (int i=0; i < numvars; i++)
//  {
//    dir[i] = xnlp[i] - x[i];
//  }


///////////////////////////////

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it){
    con = *it;
    std::fill(gr,gr+numvars,0.0);
    con->getFunction()->evalGradient(x,gr,&error);
    nlpact1 = con->getActivity(x,&error);
    if (nlpact1 > solAbsTol_ + con->getUb()){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      gnorm = sqrt(gnorm);
      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] + gr[i]/gnorm;
    }
    if (nlpact1 < con->getLb()-solAbsTol_){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] - gr[i]/gnorm;
    }  
  }

//////////////////////////////



  for (int i=0; i < numvars; i++)
    std::cout << "dir[" << i << "]=" << dir[i] << ", ";
  std::cout << "\n\n\n";

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
//    std::cout << "n1 = " << n1++ << "\n";
    con = *it;
//    std::fill(dir,dir+numvars,0.0);
//    con->getFunction()->evalGradient(x,dir,&error);

//    for (int i=0; i < numvars; i++)
//      std::cout << "dir[" << i << "]=" << dir[i] << ", ";
//    std::cout << "\n\n\n";
   
    nlpact1 = con->getActivity(x,&error);
    std::cout << con->getLb() << " " << nlpact1 << " " << con->getUb() << "\n";
//    nlpact2 = con->getActivity(xnlp, &error); 
    if (nlpact1 > solAbsTol_ + con->getUb())
    {
//      std::cout << "inside if 1\n";
//      std::fill(gr, gr+numvars, 0.0); 
//      con->getFunction()->evalGradient(x,gr,&error);
      alphal = 0.0; alphau = 1.0;  stop = false; stop1 = false;  
//      iter = 0;
/*
      stop1 = true;
      while (stop1==false) // && iter < 5)
      {
//	iter++;
	alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];	
	nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 > con->getUb() + solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
//      std::cout << "end while loop 1\n";
      alphal = alphau - 1.0;
*/
      adAlpha = -INFINITY;
      nlpactm = INFINITY;
      alpha = alphau;
      while (stop == false && (alphau - alphal) > 1e-5)
      {
//	alpha = (alphal + alphau) * 0.5;
//	std::fill(xalpha,xalpha+numvars,0.0);
	for (int i = 0; i < numvars; ++i){
          xalpha[i] = x[i] - alpha * dir[i];
//	  std::cout << "xa[" << i << "]=" << xalpha[i] << " ";
	}
//	std::cout << "\n";
//        nlpact0 = con->getActivity(xalpha,&error);
	nlpact0 = con->getFunction()->eval(xalpha, &error);
	std::cout << "nlpact0 = " << nlpact0;
	std::cout << ", alpha = " << alpha << ", adAlpha =" << adAlpha << "\n";
	if (nlpact0 > con->getUb() + solAbsTol_ && nlpact0 < nlpact1 - solAbsTol_) {
	  std::cout << "1 ";
	  if (nlpact0 < nlpactm){
  	    std::cout << "2 ";
  	    adAlpha = alpha;
	    nlpactm = nlpact0;
	  }
	  alphau = alpha;
	} else if (nlpact0 > con->getUb() + solAbsTol_){
	    std::cout << "3 ";
  	    alphau = alpha;
	} else if (nlpact0 < con->getUb() - solAbsTol_) {
	    std::cout << "4 ";
	    alphal = alpha;
	} else {
	    std::cout << "5 ";
	    stop = true; adAlpha = alpha;
   	}
	alpha = (alphal + alphau) * 0.5;
      }
//      std::cout << "end while loop 2\n";
      if (stop == true){
	std::cout << "inside if\n";
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
      }
	std::cout << "adAlpha = " << adAlpha << "\n";
      if (adAlpha > -INFINITY){
        f = con->getFunction();
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error)+cnst > solAbsTol_){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2, -INFINITY, -cnst,true,false);
	  else {
            newcon = rel_->newConstraint(f2, -INFINITY,-cnst,"ExtraCuts");
	  }
	  ++(stats_->cuts);
//	std::cout << "exxxxx\n";
  	  ncut++;
        }
      }
    }

    if (nlpact1 < con->getLb()-solAbsTol_)
    {
//      std::cout << "inside if 2\n";
      alphal = 0.0;
      alphau = 1.0;
      stop = false;
      stop1 = false;
//      std::fill(gr, gr+numvars, 0.0);
//      con->getFunction()->evalGradient(x,gr,&error);
/*
      stop1 = true;
      while (stop1 == false)
      {
        alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_){
          alpha++;
        } else stop1 = true;
      }
      alphal = alphau - 1.0;
*/
      adAlpha = INFINITY;
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_ && nlpact0 > nlpact1 + solAbsTol_){
	  adAlpha = alpha;
	  alphau = alpha;
        } else if (nlpact0 < con->getLb() - solAbsTol_) {
	    alphau = alpha;
	} else if (nlpact0 > con->getUb() + solAbsTol_) {
	    alphal = alpha;  
	} else {
	  stop = true;
	  adAlpha = alpha;
	}
      }
      if (stop == true) {
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
      }
	std::cout << "adAlpha = " << adAlpha << "\n";
      if (adAlpha < INFINITY){
        f = con->getFunction();
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error) + cnst < -solAbsTol_){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2,-cnst,INFINITY,true,false);
	  else {
            newcon = rel_->newConstraint(f2,-cnst, INFINITY, "ExtraCuts");
	  }
	  ++(stats_->cuts);
//      } 
//	std::cout << "exxxxx\n";
	  ncut++;
        }
      }
    }
  }
  std::cout << "strengthened ncut = " << ncut << "\n";

  grdTime_ += timer_->query();
  timer_->stop();
//  delete [] x;
  delete [] xalpha;
  delete [] gr;
  delete [] dir;
  return ncut;
}

int quasiQGHandler::strCutsQuasi2_(ConstSolutionPtr sol, const double *xnlp)
{
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  const double *x = sol->getPrimal();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];

  double nlpact0 = 0.0;
  double nlpact1 = 0.0;
  double nlpactm = 0.0;
  double cnst = 0.0;
  LinearFunctionPtr lf;
  FunctionPtr f,f2;

  double alphal = 0.0;
  double alphau = 1.0;
  double alpha = 0.0;
  double adAlpha = 0.0;
  Bool stop = false;
  int error = 0;
  double gnorm = 0.0;
  double *dir  = new double[numvars];
  double *dir1 = new double[numvars];
  int ncut = 0;


  double e = 0.1;
  double beta1 = 1.0-e;
  double beta2 = 1.0+e;
  double n1;
  double Tol = 0.25;
  double gap = 0.0;
  //double gnorm;

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
    con = *it;
    std::fill(dir,dir+numvars,0.0);
    con->getFunction()->evalGradient(x,dir,&error);
 
    nlpact1 = con->getActivity(x,&error);
    if (nlpact1 > solAbsTol_ + con->getUb())
    {
      alphal = 0.0; alphau = 1.0;  stop = false;  
      adAlpha = -INFINITY;
      nlpactm = INFINITY;
      alpha = alphau;
      n1 = 0;
//      while (stop == false && n1 <= 1000)// && (alphau - alphal) > 1e-5)
//      {

      adAlpha = LineSearch1_(con,x,xalpha,dir);
#ifdef PrEXCUT
      std::cout << "adAlpha = " << adAlpha << "\n";
#endif
        nlpact0 = con->getFunction()->eval(xalpha, &error);
        if (adAlpha > -INFINITY && nlpact0 > con->getUb()-solAbsTol_){
          f = con->getFunction();
          linearAt_(f,0.0,xalpha,&cnst, &lf,dir);
          f2 = (FunctionPtr) new Function(lf);
//        std::cout << "before exxxxx\n";
	  gap = f2->eval(x,&error)+cnst;
	  gnorm = InnerProduct(dir,dir,numvars);
	  gnorm =sqrt(gnorm);
	  gap = gap / gnorm;
          if (gap > Tol){
	    if (ctMngr_)
              newcon = ctMngr_->addCut(rel_,f2, -INFINITY, -cnst,true,false);
	    else {
              newcon = rel_->newConstraint(f2, -INFINITY,-cnst,"ExtraCuts");
	    }
	    ++(stats_->cuts);
//	std::cout << "exxxxx\n";
  	    ncut++;
//          }
        }
      }
    }
    if (nlpact1 < con->getLb()-solAbsTol_)
    {
//      std::cout << "inside if 2\n";
      alphal = 0.0;
      alphau = 1.0;
      stop = false;
//      std::fill(gr, gr+numvars, 0.0);
//      con->getFunction()->evalGradient(x,gr,&error);
/*
      stop1 = true;
      while (stop1 == false)
      {
        alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_){
          alpha++;
        } else stop1 = true;
      }
      alphal = alphau - 1.0;
*/
      adAlpha = INFINITY;
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_ && nlpact0 > nlpact1 + solAbsTol_){
	  adAlpha = alpha;
	  alphau = alpha;
        } else if (nlpact0 < con->getLb() - solAbsTol_) {
	    alphau = alpha;
	} else if (nlpact0 > con->getUb() + solAbsTol_) {
	    alphal = alpha;  
	} else {
	  stop = true;
	  adAlpha = alpha;
	}
      }
      if (stop == true) {
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
      }
	std::cout << "adAlpha = " << adAlpha << "\n";
      if (adAlpha < INFINITY){
        f = con->getFunction();
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error) + cnst < -Tol){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2,-cnst,INFINITY,true,false);
	  else {
            newcon = rel_->newConstraint(f2,-cnst, INFINITY, "ExtraCuts");
	  }
	  ++(stats_->cuts);
//      } 
//	std::cout << "exxxxx\n";
	  ncut++;
        }
      }
    }
  }

#ifdef PrEXCUT
  std::cout << "strengthened ncut = " << ncut << "\n";
#endif
//  delete [] x;
  delete [] xalpha;
  delete [] gr;
  delete [] dir;
  return ncut;
}

int quasiQGHandler::strCutsQuasi1_(ConstSolutionPtr sol, const double *xnlp)
{
  timer_->start();
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  const double *x = sol->getPrimal();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];

  double nlpact0 = 0.0;
  double nlpact1 = 0.0;
  double nlpactm = 0.0;
  double cnst = 0.0;
  LinearFunctionPtr lf;
  FunctionPtr f,f2;

  double alphal = 0.0;
  double alphau = 1.0;
  double alpha = 0.0;
  double adAlpha = 0.0;
  Bool stop = false;
  int error = 0;
  double gnorm = 0.0;
  double *dir  = new double[numvars];
  double *dir1 = new double[numvars];
  int ncut = 0;


  double e = 0.1;
  double beta1 = 1.0-e;
  double beta2 = 1.0+e;
  double n1;
  double Tol = 0.19;
//  int n1 = 0;
//  int iter = 0;
//  for (int i=0; i < numvars; i++)
//  {
//    dir[i] = xnlp[i] - x[i];
//  }


///////////////////////////////

/*
  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it){
    con = *it;
    std::fill(gr,gr+numvars,0.0);
    con->getFunction()->evalGradient(x,gr,&error);
    nlpact1 = con->getActivity(x,&error);
    if (nlpact1 > solAbsTol_ + con->getUb()){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      gnorm = sqrt(gnorm);
      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] + gr[i]/gnorm;
    }
    if (nlpact1 < con->getLb()-solAbsTol_){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] - gr[i]/gnorm;
    }  
  }
*/
//////////////////////////////



//  for (int i=0; i < numvars; i++)
//    std::cout << "dir[" << i << "]=" << dir[i] << ", ";
//  std::cout << "\n\n\n";

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
//    std::cout << "n1 = " << n1++ << "\n";
    con = *it;
    std::fill(dir,dir+numvars,0.0);
    con->getFunction()->evalGradient(x,dir,&error);

//    for (int i=0; i < numvars; i++)
//      std::cout << "dir[" << i << "]=" << dir[i] << ", ";
//    std::cout << "\n\n\n";
   
    nlpact1 = con->getActivity(x,&error);
//    std::cout << con->getLb() << " " << nlpact1 << " " << con->getUb() << "\n";
//    nlpact2 = con->getActivity(xnlp, &error); 
    if (nlpact1 > solAbsTol_ + con->getUb())
    {
//      std::cout << "inside if 1\n";
//      std::fill(gr, gr+numvars, 0.0); 
//      con->getFunction()->evalGradient(x,gr,&error);
      alphal = 0.0; alphau = 1.0;  stop = false;  
//      iter = 0;
/*
      stop1 = true;
      while (stop1==false) // && iter < 5)
      {
//	iter++;
	alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];	
	nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 > con->getUb() + solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
//      std::cout << "end while loop 1\n";
      alphal = alphau - 1.0;
*/
      adAlpha = -INFINITY;
      nlpactm = INFINITY;
      alpha = alphau;
      n1 = 0;
      while (stop == false && n1 <= 1000)// && (alphau - alphal) > 1e-5)
      {
//	alpha = (alphal + alphau) * 0.5;
//	std::fill(xalpha,xalpha+numvars,0.0);
	for (int i = 0; i < numvars; ++i){
          xalpha[i] = x[i] - alpha * dir[i];
	}
	nlpact0 = con->getFunction()->eval(xalpha, &error);
//	std::cout << "nlpact0 = " << nlpact0;
//	std::cout << ", alpha = " << alpha << ", adAlpha =" << adAlpha << "\n";
	if (nlpact0 > con->getUb() + solAbsTol_){// && nlpact0 < nlpact1 - solAbsTol_) {
//	  std::cout << "1 ";
	  std::fill(gr,gr+numvars,0.0);
	  con->getFunction()->evalGradient(xalpha,gr,&error);
	  for (int i = 0; i < numvars; ++i)
	    dir1[i] = x[i] - xalpha[i];

	  if (InnerProduct(dir,gr,numvars) > -solAbsTol_){
	    alpha = alpha * beta2;
	    if (nlpact0 < nlpactm - solAbsTol_ && nlpact0 < nlpact1 - solAbsTol_){
	      adAlpha = alpha;
	      nlpactm = nlpact0;
	    } else n1++;
	  } else {
	    alpha = alpha * beta1;
	    n1++;
	  }
//	} else if (nlpact0 > con->getUb() + solAbsTol_){
//	    std::cout << "3 ";
//  	    alphau = alpha;
	} else if (nlpact0 < con->getUb() - solAbsTol_) {
//	    std::cout << "4 ";
//	    alphal = alpha;
	    alpha = alpha * beta1;
	    n1++;
	} else {
//	    std::cout << "5 ";
	    stop = true; adAlpha = alpha;
   	}
//	alpha = (alphal + alphau) * 0.5;
      }
//      std::cout << "end while loop 2\n";
//      if (stop == true){
//	std::cout << "inside if\n";
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
//      }

#ifdef PrEXCUT
      std::cout << "adAlpha = " << adAlpha << "\n";
#endif
      nlpact0 = con->getFunction()->eval(xalpha, &error);
      if (adAlpha > -INFINITY && nlpact0 > con->getUb()-solAbsTol_){
        f = con->getFunction();
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error)+cnst > Tol){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2, -INFINITY, -cnst,true,false);
	  else {
            newcon = rel_->newConstraint(f2, -INFINITY,-cnst,"ExtraCuts");
	  }
	  ++(stats_->cuts);
//	std::cout << "exxxxx\n";
  	  ncut++;
        }
      }
    }

    if (nlpact1 < con->getLb()-solAbsTol_)
    {
//      std::cout << "inside if 2\n";
      alphal = 0.0;
      alphau = 1.0;
      stop = false;
//      std::fill(gr, gr+numvars, 0.0);
//      con->getFunction()->evalGradient(x,gr,&error);
/*
      stop1 = true;
      while (stop1 == false)
      {
        alpha = alphau;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_){
          alpha++;
        } else stop1 = true;
      }
      alphal = alphau - 1.0;
*/
      adAlpha = INFINITY;
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * gr[i];	
        nlpact0 = con->getActivity(xalpha,&error);
        if (nlpact0 < con->getLb() - solAbsTol_ && nlpact0 > nlpact1 + solAbsTol_){
	  adAlpha = alpha;
	  alphau = alpha;
        } else if (nlpact0 < con->getLb() - solAbsTol_) {
	    alphau = alpha;
	} else if (nlpact0 > con->getUb() + solAbsTol_) {
	    alphal = alpha;  
	} else {
	  stop = true;
	  adAlpha = alpha;
	}
      }
      if (stop == true) {
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
      }
	std::cout << "adAlpha = " << adAlpha << "\n";
      if (adAlpha < INFINITY){
        f = con->getFunction();
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//      std::cout << "before exxxxx\n";
        if (f2->eval(x,&error) + cnst < -Tol){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2,-cnst,INFINITY,true,false);
	  else {
            newcon = rel_->newConstraint(f2,-cnst, INFINITY, "ExtraCuts");
	  }
	  ++(stats_->cuts);
//      } 
//	std::cout << "exxxxx\n";
	  ncut++;
        }
      }
    }
  }

#ifdef PrEXCUT
  std::cout << "strengthened ncut = " << ncut << "\n";
#endif
  grdTime_ += timer_->query();
  timer_->stop();

//  delete [] x;
  delete [] xalpha;
  delete [] gr;
  delete [] dir;
  return ncut;
}

UInt quasiQGHandler::fixFrac(ConstSolutionPtr sol)
{
  const double *x = sol->getPrimal();
  const double *nlpx;
  UInt ncut = 0;
  fixInts_(x);
  solveNLP_();
  unfixInts_();
  
  switch(nlpStatus_){
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    nlpx = nlpe_->getSolution()->getPrimal();
    allLinearAt_(nlpx,x);
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
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
                                 << nlpe_->getStatusString() << std::endl
                                 << me_ << "No cut generated, may cycle!"
                                 << std::endl;
  }
}

void quasiQGHandler::createPointProb_()
{
  //std::cout << "createPointProb_\n";
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  LinearFunctionPtr lf1= (LinearFunctionPtr) new LinearFunction();
  //QuadraticFunctionPtr qfPoint_ = (QuadraticFunctionPtr) new QuadraticFunction();
  FunctionPtr f;
  double a = 0.0;
  numvars_ = minlp_->getNumVars();

  quasiSolutionPtr quasiSol = *(quasiSol_->quasiSolsBegin());
  double *x = quasiSol->getSolution();
  //PointProbPtr = (ProblemPtr) new Problem(lbs,ubs);
  PointProb_ = (ProblemPtr) new Problem();
  VariablePtr v = PointProb_->newVariable(0.0,INFINITY,Continuous);

  lf->addTerm(v,1.0);
  f = (FunctionPtr) new Function(lf);
  PointProb_->newConstraint(f,1.0,1.0);
  
  a = InnerProduct(x,x,numvars_);
  qfPoint_->addTerm(v,v,a);

  f = (FunctionPtr) new Function(lf1,qfPoint_);
  PointProb_->newObjective(f,0.0,Minimize);
//  PointProb_->write(std::cout);
}

void quasiQGHandler::addPointToProb_(const double *x)
{
  quasiSolutionPtr quasiSol;
  double a;
  VariablePtr v1 = PointProb_->newVariable(0.0,INFINITY,Continuous);
  VariablePtr v2;
  LinearFunctionPtr lf= PointProb_->getConstraint(0)->getFunction()->getLinearFunction();
  double *x1;
  int i = 0;
  for (quasiSolutionIterator it=quasiSol_->quasiSolsBegin(); 
	it!=quasiSol_->quasiSolsEnd();++it)
  {
    quasiSol = *it;
    v2 = PointProb_->getVariable(i);
    x1 = quasiSol->getSolution();
    a = InnerProduct(x,x1,numvars_);
    qfPoint_->addTerm(v1,v2,-2.0*a);
    a = 0.0;
    ++i;
  }
  a = 0.0;
  a = InnerProduct(x,x,numvars_);
  qfPoint_->addTerm(v1,v1,a);
  lf->addTerm(v1,1.0);
  PointProb_->changeConstraint(PointProb_->getConstraint(0),lf,1.0,1.0);
//  std::cout << "in addPointToProb_:\n";
//  PointProb_->write(std::cout);
}

void quasiQGHandler::updatePointProb_(const double *x)
{
  LinearFunctionPtr lf = PointProb_->getObjective()->getFunction()->getLinearFunction();//(LinearFunctionPtr) new LinearFunction();
  LinearFunctionPtr lf1 = (LinearFunctionPtr) new LinearFunction();
  int i = 0;
  VariablePtr v;
  double a = 0.0;
  double *x1;
  FunctionPtr f;
  quasiSolutionIterator it1 = quasiSol_->quasiSolsBegin();
//  std::cout << "num of sols = " << quasiSol_->getNum() << "\n";
  x1 = (*it1)->getSolution();
  //for (std::vector<VariablePtr>::iterator it2=PointProb_->varsBegin();
  for (VariableConstIterator it2=PointProb_->varsBegin();
	it2!=PointProb_->varsEnd();++it2)
  {
    v = *it2;
    a = InnerProduct(x,x1,numvars_);
    lf1->addTerm(v,-2.0 * a);
    ++it1;
    ++i;
  }
  lf1 = lf1 + lf;
  f = (FunctionPtr) new Function(lf1,qfPoint_);
  PointProb_->newObjective(f,0.0,Minimize);
}

UInt quasiQGHandler::bisectAddCut_(const double *xlp, const double *xnlp)
{
  UInt ncut = 0;
  int error = 0;
  double lpact;
  ConstraintPtr newcon;
  FunctionPtr f = rel_->getObjective()->getFunction();

  double act = f->eval(xlp,&error);
  double nlpact = f->eval(xnlp,&error);
  LinearFunctionPtr lf;
  FunctionPtr f2;
  double c;
  double nlpvalmid,nlpval1,nlpval2;
  double *xmid = new double[numvars_];
  double *x1 = new double[numvars_];
  double *x2 = new double[numvars_];
  double ub = relobj_;
  int n1 = 0;
  double gap;

  if (act - ub < solAbsTol_)
    return 0;

  for (int i = 0; i < numvars_; ++i){
    x1[i] = xlp[i];
    x2[i] = xnlp[i];
    xmid[i] = 0.5 * x1[i] + 0.5 * x2[i];
  }
  nlpvalmid = f->eval(xmid,&error);
  while (fabs(nlpvalmid - ub) > solAbsTol_ && n1 < 1000){
    //nlpval1 = f->eval(x1,&error);
    //nlpval2 = f->eval(x2,&error);
    if (nlpvalmid < ub - solAbsTol_){
      for (int i = 0; i < numvars_; ++i)
        x2[i] = xmid[i];	
    } else if (nlpvalmid > ub + solAbsTol_){
        for (int i = 0; i < numvars_; ++i)
          x1[i] = xmid[i];
    } else break;
    nlpvalmid = f->eval(xmid,&error);
    ++n1;
  }


  if (act - relobj_ > solAbsTol_){
    linearAt_(f,act,xmid,&c,&lf);
    lf->addTerm(objVar_,-1.0);
    f2 = (FunctionPtr) new Function(lf);
    lpact = f2->eval(xlp,&error);
    if (lpact + c > tolECP_){
      ++ncut;
      ++(stats_->cuts);
      if (ctMngr_)
        newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-c,true,false);
      else
        newcon = rel_->newConstraint(f2,-INFINITY,-c,"objlnrztn_cut");
    } 
  }
  
  return ncut;
}

UInt quasiQGHandler::bisectAddCut_(ConstraintPtr con,const double *xlp, const double *xnlp)
{
  UInt ncut = 0;
  FunctionPtr f = con->getFunction();
  LinearFunctionPtr lf;
  FunctionPtr f2;
  double c;
  double nlpvalmid,nlpval1,nlpval2;
  double *xmid = new double[numvars_];
  double *x1 = new double[numvars_];
  double *x2 = new double[numvars_];
  int error = 0;
  double ub,lb; 
  int n1 = 0;
  ub = con->getUb();
  lb = con->getLb();
  ConstraintPtr newcon;
  double gap;

  nlpval1 = con->getActivity(xlp,&error);
  if (nlpval1 < ub + solAbsTol_ && nlpval1 > lb - solAbsTol_)
    return 0;

  for (int i = 0; i < numvars_; ++i){
    x1[i] = xlp[i];
    x2[i] = xnlp[i];
    xmid[i] = 0.5 * x1[i] + 0.5 * x2[i];
  }
  nlpvalmid = f->eval(xmid,&error);
  if (ub < INFINITY)
  {
    while (fabs(nlpvalmid - ub) > solAbsTol_ && n1 < 1000){
      //nlpval1 = f->eval(x1,&error);
      //nlpval2 = f->eval(x2,&error);
      if (nlpvalmid < ub - solAbsTol_){
        for (int i = 0; i < numvars_; ++i)
	  x2[i] = xmid[i];	
      } else if (nlpvalmid > ub + solAbsTol_){
	  for (int i = 0; i < numvars_; ++i)
	    x1[i] = xmid[i];
      }
      nlpvalmid = f->eval(xmid,&error);
      ++n1;
    }
    if (nlpvalmid > ub - solAbsTol_){
      linearAt_(f,0.0,xmid,&c,&lf);
      f2 = (FunctionPtr) new Function(lf);
      gap = f2->eval(xlp,&error) + c;
      if (gap > tolECP_)
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-c,true,false);
	else {
	  newcon = rel_->newConstraint(f2,-INFINITY,-c,"lnrztn_cut");
        }
	++ncut;
    }
  }
 
  if (lb > -INFINITY)
  {
    while (fabs(nlpvalmid - lb) > solAbsTol_ && n1 < 1000){
      if (nlpvalmid > ub + solAbsTol_){
        for (int i = 0; i < numvars_; ++i)
	  x2[i] = xmid[i];	
      } else if (nlpvalmid < lb - solAbsTol_){
	  for (int i = 0; i < numvars_; ++i)
	    x1[i] = xmid[i];
      }
      nlpvalmid = f->eval(xmid,&error);
      ++n1;
    }
    if (nlpvalmid < lb + solAbsTol_){
      linearAt_(f,0.0,xmid,&c,&lf);
      f2 = (FunctionPtr) new Function(lf);
      gap = f2->eval(xlp,&error) + c;
      if (gap < -tolECP_)
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,-c,INFINITY,true,false);
	else {
	  newcon = rel_->newConstraint(f2,-c,INFINITY,"lnrztn_cut");
        }
	++ncut;
    }
  }
  return ncut;
}

UInt quasiQGHandler::solvePointProb_(const double *xlp)
{
  UInt ncut = 0;
  UInt ncut1;
  const double *x;
  ConstraintPtr con;
  std::cout << "P1 \n";
  updatePointProb_(xlp);
//  PointProb_->write(std::cout);
  pntE_->clear();
  std::cout << "2\n";
  PointProb_->calculateSize(true);
  std::cout << "3\n";
  PointProb_->setNativeDer();

  std::cout << "3-2\n";
  pntE_->load(PointProb_);

  std::cout << "3-3\n";
  pntE_->solve();

  std::cout << "4\n";
  x = pntE_->getSolution()->getPrimal();
  
  std::cout << "5\n";
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end();++it)
  {
    ncut1 = bisectAddCut_(*it,xlp,x);
    ncut += ncut1;
  }
  ncut1 = bisectAddCut_(xlp,x);
  ncut += ncut1;
  pntE_->clear();
  return ncut;
}

UInt quasiQGHandler::QuasiECP_ContNLPLimited(ConstSolutionPtr sol)
{
  UInt num_cut = 0;
  numvars_ = minlp_->getNumVars();
  double nlpval;
  const double *x = sol->getPrimal();
  const double *xnlp;//= new double[numvars_];
  std::stack<Modification *> mods;

  copyLPBounds_(&mods);
  nlpe_->clear();
  nlpe_->setIterationLimit(10);
  nlpe_->load(minlp_);
  nlpStatus_ = nlpe_->solve();
  nlpe_->resetIterationLimit();
  ++(stats_->nlpS);
  nlpval = nlpe_->getSolutionValue();
  xnlp = nlpe_->getSolution()->getPrimal();
  num_cut = allLinearAt_(xnlp,x);

  undoLPBounds_(&mods);
  return num_cut;
}

UInt quasiQGHandler::QuasiECP_ContProj(ConstSolutionPtr sol)
{
  UInt num_cut = 0;
  const double *x = sol->getPrimal();
  const double *xproj; 
  VariablePtr v;
  FunctionPtr f;
  FunctionPtr orjProjObj = proj_minlp_->getObjective()->getFunction();
 
  lf_pure_->clearAll();
  int i = 0;
  for (VariableConstIterator it=proj_minlp_->varsBegin(); it!=proj_minlp_->varsEnd();++it){
    v = *it;
    lf_pure_->addTerm(v,-1.0*x[i]);
    ++i;
  } 
  f = (FunctionPtr) new Function(lf_pure_,qf_pure_);
  proj_nlpe_->clear();
  proj_minlp_->removeObjective();
  proj_minlp_->newObjective(f,0,Minimize);
  proj_minlp_->calculateSize(true);
  proj_minlp_->setNativeDer();

  proj_nlpe_->load(proj_minlp_);
  projnlpStatus_ = proj_nlpe_->solve();
  switch (projnlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    xproj = proj_nlpe_->getSolution()->getPrimal();
    num_cut = allLinearAt_(xproj,x);
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
  case (ProvenUnbounded):
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "Projection did not help" << std::endl;

  }
  proj_nlpe_->clear();  
  proj_minlp_->newObjective(orjProjObj, 0, Minimize);
  proj_minlp_->calculateSize(true);
  proj_minlp_->setNativeDer();
  
  return num_cut;
}

UInt quasiQGHandler::QuasiECP_ContNLP(ConstSolutionPtr sol)
{
  UInt num_cut = 0;
  numvars_ = minlp_->getNumVars();
  double nlpval;
  const double *x = sol->getPrimal();
  const double *xnlp;//= new double[numvars_];
  std::stack<Modification *> mods;

  copyLPBounds_(&mods);
  solveNLP_();
  nlpval = nlpe_->getSolutionValue();
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    xnlp = nlpe_->getSolution()->getPrimal();
    num_cut = allLinearAt_(xnlp,x);
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
    //num_cut = -1;
    break;
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (ProvenUnbounded):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status = " 
				 << nlpe_->getStatusString() 
				 << std::endl;
    //assert(!"NLP engine failes");
    return 0;
  }
  undoLPBounds_(&mods);
  return num_cut;
}

UInt quasiQGHandler::allLinearAt_(const double *x,const double *xlp)
{
  double act,lpact,nlpact,c;
  ConstraintPtr con,newcon;
  FunctionPtr f,f2;
  int error;
  UInt ncut = 0;
  LinearFunctionPtr lf = LinearFunctionPtr();
  ObjectivePtr o = minlp_->getObjective();
  

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it){
    con = *it;
    f = con->getFunction();
    act = con->getActivity(xlp,&error);
    nlpact = con->getActivity(x,&error);

    if (con->getUb() < INFINITY){
      if (act > con->getUb() + solAbsTol_ && nlpact > con->getUb() - solAbsTol_ ){
        linearAt_(f,0.0,x,&c,&lf);
	f2 = (FunctionPtr) new Function(lf);
	lpact = f2->eval(xlp,&error);
	if (lpact + c > tolECP_){
	  ++ncut;
  	  ++(stats_->cuts);
	  if (ctMngr_)
	    newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-c,true,false);
	  else {
	    newcon = rel_->newConstraint(f2,-INFINITY,-c,"lnrztn_cut");
	    //conVect.push_back(newcon);
	  } 
	}
      }
    }
    if (con->getLb() > -INFINITY) {
      if (act < con->getLb() - solAbsTol_ && nlpact < con->getLb() + solAbsTol_) {
        linearAt_(f,0.0,x,&c,&lf);
        f2 = (FunctionPtr) new Function(lf);
        lpact = f2->eval(xlp,&error);
        if (lpact + c < -tolECP_){
          ++ncut;
          ++(stats_->cuts);
	  if (ctMngr_)
	    newcon = ctMngr_->addCut(rel_,f2,-c, INFINITY, true, false);
	  else {
	    newcon = rel_->newConstraint(f2,-c, INFINITY, "lnrztn_cut");
	    //conVect.push_pack(newcon);
	  }
	}
      }
    }
    //}
  }

  if (oNl_ && o){
    double viol;
    f = o->getFunction();
    //act = f->eval(xlp,&error);
    act = o->eval(xlp,&error);
    nlpact = o->eval(x,&error);
    //viol = std::max(act-relobj_,0.0);
    if (act - relobj_ > solAbsTol_){
      linearAt_(f,nlpact,x,&c,&lf);
      lf->addTerm(objVar_,-1.0);
      f2 = (FunctionPtr) new Function(lf);
      lpact = f2->eval(xlp,&error);
      if (lpact + c > tolECP_){
	++ncut;
	++(stats_->cuts);
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-c,true,false);
	else
	  newcon = rel_->newConstraint(f2,-INFINITY,-c,"objlnrztn_cut");
      } 
    } 
  }
  return ncut;
}

int quasiQGHandler::ECPCuts_(ConstSolutionPtr sol)
{
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  double nlpact1 = 0.0;
  int error;
  double cnst;
  //double cnst1;
  double gap=0.0;
  const double *x = sol->getPrimal();
  double *gr = new double[numvars];
  double gnorm = 0.0;

  LinearFunctionPtr lf;
  FunctionPtr f,f2;
  ObjectivePtr o = minlp_->getObjective();
  relobj_ = sol->getObjValue();
  int ncut = 0;
  double Tol = tolECP_;
  
  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it)
  {
    con = *it;
    nlpact1 = con->getActivity(x, &error);
    f = con->getFunction();
    if (nlpact1 > con->getUb() + solAbsTol_){
//      std::cout << "violation = " << nlpact1 - con->getUb() << "\n";
      linearAt_(f, nlpact1, x, &cnst, &lf, gr);//, &cnst1);
//      for (int i = 0; i < numvars; ++i)
//	if (abs(gr[i]) > 1e-6)
// 	  std::cout << " gr[" << i << "]=" << gr[i];
//      std::cout << "\n";
//      lf->write(std::cout);
//      std::cout << "\n";
      gnorm = InnerProduct(gr,gr,numvars);
      gnorm = sqrt(gnorm);
      f2 = (FunctionPtr) new Function(lf);
      gap = f2->eval(x,&error) + cnst - con->getUb();
      if (gap/gnorm > Tol){
#ifdef PrEXCUT
        std::cout << std::setprecision(8) << "gap1 = " << gap << "\n";
#endif
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2, -INFINITY, con->getUb()-cnst,true,false);
	else {
          newcon = rel_->newConstraint(f2, -INFINITY,con->getUb()-cnst,"ECPCuts");
	}
	++(stats_->cuts);
	//newcon->write(std::cout);
	ncut++;
      }
    }
    if (nlpact1 < con->getLb() - solAbsTol_){
      linearAt_(f,nlpact1,x,&cnst,&lf,gr);
      gnorm = InnerProduct(gr,gr,numvars);
      gnorm = sqrt(gnorm);
      f2 = (FunctionPtr) new Function(lf);
      gap = f2->eval(x,&error) + cnst - con->getLb();
      if (gap/gnorm < -Tol){
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2,con->getLb() - cnst,INFINITY,true,false);
	else {
          newcon = rel_->newConstraint(f2,con->getLb() -cnst, INFINITY, "ECPCuts");
	}
//	conVect.push_back(newcon);
	++(stats_->cuts);
	//newcon->write(std::cout);
	ncut++;
      }
    }
  }
#ifdef PrEXCUT
  std::cout << "*************************8objective cuts\n";
#endif
  if (oNl_ && o)
  {
    f = o->getFunction();
    nlpact1 = o->eval(x, &error);
    gap = std::max(nlpact1-relobj_,0.0);
    if (gap > solAbsTol_) {
      linearAt_(f, nlpact1, x, &cnst, &lf,gr);
      lf->addTerm(objVar_, -1.);
      f2 = (FunctionPtr) new Function(lf);
      gnorm = InnerProduct(gr,gr,numvars);
      gnorm = sqrt(gnorm);
      gap = f2->eval(x,&error) + cnst;
#ifdef PrEXCUT
      std::cout << "gap = " << gap << "\n";
#endif
      if (gap/gnorm > Tol){
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-1.0*cnst,true,false);
	else {
          newcon = rel_->newConstraint(f2, -INFINITY, -1.0*cnst, "objlnrztn_cut"); 
	}
//        conVect.push_back(newcon);
	++(stats_->cuts);
	//newcon->write(std::cout);
	ncut++;
      }
    }
  }

#ifdef PrEXCUT
  std::cout << "num ECP cuts = " << ncut << "\n";
#endif

  //delete [] x;
  delete [] gr;
  return ncut;
}

int quasiQGHandler::strECPCuts_(ConstSolutionPtr sol)
{

#ifdef TimerLS
#else
  timer_->start();
#endif
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  double nlpact1 = 0.0;
  int error;
  double cnst;
  double gap=0.0;
  double gnorm;
  double nlpact0 = 0.0;
  double nlpactm = 0.0;

  const double *x = sol->getPrimal();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];
  double *dir  = new double[numvars];
  double *dir1 = new double[numvars];

  LinearFunctionPtr lf;
  FunctionPtr f,f2;
  ObjectivePtr o = minlp_->getObjective();
  relobj_ = sol->getObjValue();
  int ncut = 0;

  double alphau = 1.0;
  double alpha = 0.0;
  double adAlpha = 0.0;
  Bool stop = false;  
  double n1;
  double e = 0.1;
  double beta1 = 1.0-e;
  double beta2 = 1.0+e;
  double Tol = 0.01;
  Bool admit = false;
/*
  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it){
    con = *it;
    std::fill(gr,gr+numvars,0.0);
    con->getFunction()->evalGradient(x,gr,&error);
    nlpact1 = con->getActivity(x,&error);
    if (nlpact1 > solAbsTol_ + con->getUb()){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      gnorm = sqrt(gnorm);
      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] + gr[i]/gnorm;
    }
    if (nlpact1 < con->getLb()-solAbsTol_){
      gnorm = 0.0;
      for (int i=0; i < numvars; i++)
        gnorm = gnorm + gr[i]*gr[i];

      for (int i=0; i < numvars; i++)
        dir[i] = dir[i] - gr[i]/gnorm;
    }  
  }
*/

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it)
  {
    con = *it;
    nlpact1 = con->getActivity(x, &error);
    f = con->getFunction();
    f->evalGradient(x,gr,&error);
    gnorm = sqrt(InnerProduct(gr,gr,numvars));
//    if ((nlpact1 - con->getUb())/gnorm > Tol){
    if ((nlpact1 - con->getUb()) > solAbsTol_){
//      std::cout << "violation = " << nlpact1 - con->getUb() << "\n";

/*
      alphau = 1.0; stop = false;
      adAlpha = -INFINITY; nlpactm = INFINITY;
      alpha = alphau;
      n1 = 0;
      while (stop = false && n1 <= 1000){
	for (int i = 0; i < numvars; ++i){
          xalpha[i] = x[i] - alpha * dir[i];
	}
        nlpact0 = f->eval(xalpha,&error);
        if (nlpact0 > con->getUb() + solAbsTol_){
	  std::fill(gr,gr+numvars,0.0);
	  con->getFunction()->evalGradient(xalpha,gr,&error);
	  for (int i = 0; i < numvars; ++i)
	    dir1[i] = x[i] - xalpha[i];

	  if (InnerProduct(dir1,gr,numvars) > -solAbsTol_){
	    alpha = alpha * beta2;
	    if (nlpact0 < nlpactm && nlpact0 < nlpact1){
	      adAlpha = alpha;
	      nlpactm = nlpact0;
	    } else n1++;
	  } else {
	    alpha = alpha * beta1;
	    n1++;
	  }
        } else if (nlpact0 < con->getUb() - solAbsTol_) {
	    alpha = alpha * beta1;
	    n1++;
	} else {
	  stop = true; adAlpha = alpha;
        }
      }
      alpha = adAlpha;
      for (int i = 0; i < numvars; ++i)
        xalpha[i] = x[i] - alpha * dir[i];
*/
//      std::cout << " ************** gnorm = " << gnorm << "\n";
      LineSearch2_(con,x,xalpha,gr,&admit);
      if (admit == true){
//      std::cout << "11111111111111\n";
#ifdef PrEXCUT
      std::cout << "  adAlpha = " << adAlpha << "\n";
#endif
      nlpact0 = f->eval(xalpha,&error); 
      //linearAt_(f, nlpact0, xalpha, &cnst, &lf);
      //f->evalGradient(xalpha,gr,&error);
      linearAt_(f, nlpact0, xalpha, &cnst, &lf,gr);
      f2 = (FunctionPtr) new Function(lf);
      gap = f2->eval(x,&error) + cnst - con->getUb();
      gnorm = InnerProduct(gr,gr,numvars);
      gnorm = sqrt(gnorm);
 
//      std::cout << "gap / norm - Tol = " << gap/gnorm - Tol << "\n";
      if (gap/gnorm > Tol){
#ifdef PrEXCUT
        std::cout << std::setprecision(8) << "gap1 = " << gap << "\n";
#endif
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2, -INFINITY, con->getUb()-cnst,true,false);
	else{
          newcon = rel_->newConstraint(f2, -INFINITY,con->getUb()-cnst,"ECPCuts");
	}
//	conVect.push_back(newcon);
	++(stats_->cuts);
	//newcon->write(std::cout);
	ncut++;
      }
      }
    }

    if (nlpact1 < con->getLb() - solAbsTol_){


      linearAt_(f,nlpact0,xalpha,&cnst,&lf);
      f2 = (FunctionPtr) new Function(lf);
      if (f2->eval(x,&error) + cnst - con->getLb() < -Tol){
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2,con->getLb() - cnst,INFINITY,true,false);
	else {
          newcon = rel_->newConstraint(f2,con->getLb() -cnst, INFINITY, "ECPCuts");
	}
//	conVect.push_back(newcon);
	++(stats_->cuts);
	//newcon->write(std::cout);
	ncut++;
      }
    }
  }
//  std::cout << "objective cuts\n";
  if (oNl_ && o)
  {
    f = o->getFunction();
    nlpact1 = o->eval(x, &error);
    gap = std::max(nlpact1-relobj_,0.0);
    if (gap > solAbsTol_) {
      linearAt_(f, nlpact1, x, &cnst, &lf);
      lf->addTerm(objVar_, -1.);
      f2 = (FunctionPtr) new Function(lf);
      gap = f2->eval(x,&error);
      if (gap + cnst > Tol){
	if (ctMngr_)
         newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-1.0*cnst,true,false);
	else{
          newcon = rel_->newConstraint(f2, -INFINITY, -1.0*cnst, "objlnrztn_cut"); 
	}
//        conVect.push_back(newcon);
	++(stats_->cuts);
	//newcon->write(std::cout);
	ncut++;
      }
    }
  }

#ifdef PrEXCUT
  std::cout << "num ECP cuts = " << ncut << "\n";
#endif

  //delete [] x;
  delete [] xalpha;
  delete [] gr;
  delete [] dir; 
  delete [] dir1;
#ifdef TimerLS
#else 
  grdTime_ += timer_->query();
  timer_->stop();
#endif 
  return ncut;
}

void quasiQGHandler::ExtraCutsCvx_(const double *x, const double *xnlp)
{
  ConstraintPtr con, newcon;
  int numvars = minlp_->getNumVars();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];
  double *dir = new double[numvars];


  double nlpact0 = 0.0;
  double nlpact1 = 0.0;
  double nlpact2 = 0.0;
  int error;
  double cnst;

  LinearFunctionPtr lf;
  FunctionPtr f,f2;

  double alphal = 0.0;
  double alphau = 1.0;
  double alpha = 0.0;
  double alphaC = 0.0;
  Bool stop = false;
  Bool stop1 = false;
  double adAlpha = 0.0;

  double alpha0 = 0.05;
  double beta   = 0.1;
  double gnorm = 0.0;

  int ncut = 0;
//  int n1 = 0;
  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
//    std::cout << "n1 = " << n1++ << "\n";
    con = *it;
    nlpact1 = con->getActivity(x,&error);
    nlpact2 = con->getActivity(xnlp, &error);

    if (nlpact1 > solAbsTol_ + con->getUb())
    {
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
      gnorm = 0.0;
      for (int i = 0; i < numvars; i++)
	gnorm = gnorm + gr[i] * gr[i];

      gnorm = sqrt(gnorm);
      for (int i = 0; i < numvars; i++)
	dir[i] = dir[i] + gr[i]/gnorm;
    }

    if (nlpact1 < -solAbsTol_ + con->getLb())
    {
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
      gnorm = 0.0;
      for (int i = 0; i < numvars; i++)
	gnorm = gnorm + gr[i] * gr[i];

      gnorm = sqrt(gnorm);
      for (int i = 0; i < numvars; i++)
	dir[i] = dir[i] - gr[i]/gnorm;

    }
  }

//  n1 = 0;
  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
//    std::cout << "n1 = " << n1++ << "\n";
    con = *it;
    alphal = 0.0;
    alphau = 1.0;

    nlpact1 = con->getActivity(x,&error);
    nlpact2 = con->getActivity(xnlp, &error);

//    std::cout << "Lb = " << con->getLb();
//    std::cout << "  nlpact1 = " << nlpact1;
//    std::cout << "  nlpact2 = " << nlpact2;
//    std::cout << "  Ub = " << con->getUb();
    if (nlpact1 > solAbsTol_ + con->getUb() && nlpact2 < solAbsTol_ + con->getUb() )
    {
//      std::cout << "  violated  1\n";
/*
      nlpact0 = -INFINITY;
      while (nlpact0 < -solAbsTol_ + con->getUb() && (alphau - alphal) > 1e-3)
      {
	alpha  = (alphal+alphau)*0.5;
        alphac = 1-alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];
      
        alphau = alpha;
        nlpact0 = con->getActivity(xalpha,&error);
      }
*/
/*
      while ( alphau - alphal > 1e-3 && stop == false)
      {
        alpha = (alphal + alphau) * 0.5;
	alphac = 1.0 - alphac;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];

        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 - con->getUb() > solAbsTol_ )
	{
	  alphau = alpha;
	  adAlpha = alpha;
	} else if (nlpact0 - con->getUb() < -solAbsTol_) {
		alphal = alpha;
	} else stop = true;
      }
      if (stop = false){
	alpha = adAlpha;
	alphac = 1.0 - alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];	
      }
*/
/*
      nlpact0 = INFINITY;
      while (nlpact0 > solAbsTol_ + con->getUb() && (alphau - alphal) > 1e-3)
      {
	alpha0 = alpha;
	alpha = (alphal + alphau)*0.5;
	alphac = 1.0 - alpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];

	alphau = alpha;
        nlpact0 = con->getActivity(xalpha,&error);
      }
      std::cout << "alpha0 = " << alpha0 << "\n";
      alphac = 1-alpha0;
      for (int i = 0; i < numvars; ++i)
        xalpha[i] = alphac * x[i] + alpha0 * xnlp[i];

*/
///////////////////////////////////

      stop = false;
      stop1 = false;
      alphal = 0.0;
      alphau = 1.0;
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
/*
      while (stop1 == false)
      {
        alpha = alphau; 
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];	
        nlpact0 = con->getActivity(xalpha,&error);	  

	if (nlpact0 > con->getUb() + solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
      alphal = alphau - 1;
*/
      stop1 = false;
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 > con->getUb() + solAbsTol_) {
	  alphal = alpha;
	  if (nlpact0 < nlpact1){
	    adAlpha = alpha;
	    stop1 = true;
          }
	} else if (nlpact0 < con->getUb() - solAbsTol_) {
	    alphau = alpha;
	  } else {
	    stop = true;
	  }
//	}
      }


/////////////////////////////

      if (stop == false && stop1 == true){
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
      }
      

//      if (alphau - alphal > 1e-3) {
//	std::cout << " ***************** add extra cuts 1\n";
        f = con->getFunction();
        linearAt_(f,f->eval(xalpha,&error),xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//	std::cout << "act = " << f2->eval(x,&error)+cnst - con->getUb() << "\n";
	if (f2->eval(x,&error)+cnst - con->getUb() > solAbsTol_ && stop1 == true){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2, -INFINITY, con->getUb()-cnst,true,false);
	  else {
            newcon = rel_->newConstraint(f2, -INFINITY,con->getUb()-cnst,"ExtraCuts");
	  }
	  ++(stats_->cuts);
//      }
	  ncut++;
        }
    }

    
    alphal = 0.0;
    alphau = 1.0;
    if (nlpact1 < -solAbsTol_ + con->getLb() && nlpact2 > con->getLb() - solAbsTol_)
    {
/*
      nlpact0 = INFINITY;
      while (nlpact0 > solAbsTol_ + con->getLb() &&  (alphau - alphal) > 1e-3)
      {
	alpha0 = alpha;
	alpha  = (alphal+alphau)*0.5;
        alphac = 1-alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];
      
	alphau = alpha;
        nlpact0 = con->getActivity(xalpha,&error);
//	if (nlpact0 > -solAbsTol_ + con->getLb())
//	  alphal = alpha;
//	if (nlpact0 < con->getLb - solAbsTol_)
//	  alphau = alpha;
	
      }
*/
/*
      stop = false;
      while (alphau - alphal > solAbsTol_ && stop == false)
      {
        alpha = (alphal + alphau) * 0.5;
	alphac = 1.0 - alphac;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];

        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 - con->getLb() < -solAbsTol_ )
	{
	  alphau = alpha;
	  adAlpha = alpha;
	} else if (nlpact0 - con->getLb() > solAbsTol_) {
		alphal = alpha;
	} else stop = true;
      }

      if (stop = false){
	alpha = adAlpha;
  	alphac = 1.0 - alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];	
      }
*/
/*
      std::cout << "alpha0 = " << alpha0 << "\n";
      alphac = 1-alpha0;
      for (int i = 0; i < numvars; ++i)
        xalpha[i] = alphac * x[i] + alpha0 * xnlp[i];
*/
///////////////////////////////////

      alphal = 0.0;
      stop = false;
      alphau = 1.0;
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
      stop1 = false;
/*
      while (stop1 == false)
      {
        alpha = alphau; 
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];	
        nlpact0 = con->getActivity(xalpha,&error);	  

	if (nlpact0 < con->getLb() - solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
      alphal = alphau - 1;
*/
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];

        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 < con->getLb() - solAbsTol_) {
	  alphal = alpha;
	  if (nlpact0 > nlpact1){
  	    adAlpha = alpha;
	    stop1 = true;
          }
	} else if (nlpact0 > con->getLb() + solAbsTol_) {
	    alphau = alpha;
	  } else {
	    stop = true;
	  }
	  
//	}
      }

/////////////////////////////

      if (stop == false && stop1 == true){
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
      }

//      if (alphau - alphal > 1e-3) {
//	std::cout << " ***************** add extra cuts 2\n";
        f = con->getFunction();
        linearAt_(f,f->eval(xalpha,&error),xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
	if (f2->eval(x,&error) + cnst - con->getLb() < -solAbsTol_ && stop1 == true){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2,con->getLb() -cnst,INFINITY,true,false);
	  else {
            newcon = rel_->newConstraint(f2,con->getLb() -cnst, INFINITY, "ExtraCuts");
	  }
	  ++(stats_->cuts);
//      } 
	  ncut++;
	}
    }
  }
  std::cout << "extra cut # = " << ncut << "\n";


  delete [] gr;
  delete [] dir;
  delete [] xalpha; 
}

void quasiQGHandler::ExtraCuts_(const double *x, const double *xnlp)
{

  std::cout << "begin extracuts\n";
  ConstraintPtr con,newcon;
  int numvars = minlp_->getNumVars();
  double *xalpha = new double[numvars];
  double *gr = new double[numvars];
  double *dir = new double[numvars];

  double nlpact1 = 0.0; 
  double nlpact2 = 0.0;
  double nlpact0 = 0.0;
  int error;
  double cnst;

  LinearFunctionPtr lf;
  FunctionPtr f;
  FunctionPtr f2;

  double alphal = 0.0;
  double alphau = 1.0;
  double alpha  = 0.0;
  double alphac = 0.0;
  double adAlpha = 0.0;
  Bool stop = false;
  Bool stop1 = false;

  double alpha0 = 0.05;
  double beta   = 0.1;
  double gnorm = 0.0;
  int ncut = 0;

  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
    con = *it;
    nlpact1 = con->getActivity(x,&error);
    nlpact2 = con->getActivity(xnlp, &error);
    if (nlpact1 > solAbsTol_ + con->getUb())
    {
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
      gnorm = 0.0;
      for (int i = 0; i < numvars; i++)
	gnorm = gnorm + gr[i] * gr[i];

      gnorm = sqrt(gnorm);
      for (int i = 0; i < numvars; i++)
	dir[i] = dir[i] + gr[i]/gnorm;
    }

    if (nlpact1 < -solAbsTol_ + con->getLb())
    {
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
      gnorm = 0.0;
      for (int i = 0; i < numvars; i++)
	gnorm = gnorm + gr[i] * gr[i];
      gnorm = sqrt(gnorm);
      for (int i = 0; i < numvars; i++)
	dir[i] = dir[i] - gr[i]/gnorm;
    }
  }

  int n1 = 0;
  for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
    std::cout << "n1 = " << n1++ << "\n";
    con = *it;
    alphal = 0.0;
    alphau = 1.0;

    nlpact1 = con->getActivity(x,&error);
    nlpact2 = con->getActivity(xnlp, &error);

    if (nlpact1 > solAbsTol_ + con->getUb() && nlpact2 < solAbsTol_ + con->getUb() )
    {

/*
      nlpact0 = -INFINITY;
      while (nlpact0 < -solAbsTol_ + con->getUb() && (alphau - alphal) > 1e-3)
      {
	alpha  = (alphal+alphau)*0.5;
        alphac = 1-alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];
      
        alphau = alpha;
        nlpact0 = con->getActivity(xalpha,&error);
      }
*/
/*
      while ( alphau - alphal > 1e-3 && stop == false)
      {
        alpha = (alphal + alphau) * 0.5;
	alphac = 1.0 - alphac;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];

        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 - con->getUb() > solAbsTol_ )
	{
	  alphau = alpha;
	  adAlpha = alpha;
	} else if (nlpact0 - con->getUb() < -solAbsTol_) {
		alphal = alpha;
	} else stop = true;
      }
      if (stop = false){
	alpha = adAlpha;
	alphac = 1.0 - alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];	
      }
*/
/*
      nlpact0 = INFINITY;
      while (nlpact0 > solAbsTol_ + con->getUb() && (alphau - alphal) > 1e-3)
      {
	alpha0 = alpha;
	alpha = (alphal + alphau)*0.5;
	alphac = 1.0 - alpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];

	alphau = alpha;
        nlpact0 = con->getActivity(xalpha,&error);
      }
      std::cout << "alpha0 = " << alpha0 << "\n";
      alphac = 1-alpha0;
      for (int i = 0; i < numvars; ++i)
        xalpha[i] = alphac * x[i] + alpha0 * xnlp[i];

*/
///////////////////////////////////

      stop = false;
      stop1 = false;
      alphal = 0.0;
      alphau = 1.0;
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
/*
      while (stop1 == false)
      {
        alpha = alphau; 
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];	
        nlpact0 = con->getActivity(xalpha,&error);	  

	if (nlpact0 > con->getUb() + solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
      alphal = alphau - 1;
*/
      stop1 = false;
//      std::cout << "nlpact1 = " << nlpact1 << "\n";
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	alpha = (alphal + alphau) * 0.5;
//	std::cout << "alpha = " << alpha << "\n";
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];

        nlpact0 = con->getActivity(xalpha,&error);
//	std::cout << "nlpact0 = " << nlpact0 << "\n";
	if (nlpact0 > con->getUb() + solAbsTol_ ){ //-solAbsTol_) {
//	  stop1 = true;
	  alphal = alpha;
	  if (nlpact0 > nlpact1){
  	    adAlpha = alpha;
	    stop1 = true;
	  }
	} else if (nlpact0 < con->getUb() - solAbsTol_) {
	    alphau = alpha;
	  } else {
//	    std::cout << "met\n";
	    stop = true;
//	    adAlpha = alpha;
	  }
//	}
      }
	
/////////////////////////////

      if (stop == false && stop1 == true){
//	std::cout << "2222\n";
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpha * dir[i];
      } 
      
//        nlpact0 = con->getActivity(xalpha,&error);
//	std::cout << "nlpact0 = " << nlpact0 << " U = " << con->getUb() << "\n" ;
//      if (stop1 == true || stop == true){
//      if (alphau - alphal > 1e-3) {
//	std::cout << " ***************** add extra cuts 1\n";
        f = con->getFunction();
	cnst = 0.0;
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
//	std::cout << "nlpact0 = " << f->eval(xalpha,&error) - con->getUb() << "\n";
//	std::cout << "slack = " << f2->eval(x,&error) + cnst << "\n";
//	if (f2->eval(x,&error) + cnst > solAbsTol_ && stop1==true){
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2, -INFINITY,-cnst,true,false);
	else {
          newcon = rel_->newConstraint(f2, -INFINITY, -cnst, "ExtraCuts");
	}
	  ++(stats_->cuts);
//      }
	  ncut++;
//        }
//      } 
    }

    
    alphal = 0.0;
    alphau = 1.0;
    if (nlpact1 < -solAbsTol_ + con->getLb() && nlpact2 > con->getLb() - solAbsTol_)
    {
/*
      nlpact0 = INFINITY;
      while (nlpact0 > solAbsTol_ + con->getLb() &&  (alphau - alphal) > 1e-3)
      {
	alpha0 = alpha;
	alpha  = (alphal+alphau)*0.5;
        alphac = 1-alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];
      
	alphau = alpha;
        nlpact0 = con->getActivity(xalpha,&error);
//	if (nlpact0 > -solAbsTol_ + con->getLb())
//	  alphal = alpha;
//	if (nlpact0 < con->getLb - solAbsTol_)
//	  alphau = alpha;
	
      }
*/
/*
      stop = false;
      while (alphau - alphal > solAbsTol_ && stop == false)
      {
        alpha = (alphal + alphau) * 0.5;
	alphac = 1.0 - alphac;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];

        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 - con->getLb() < -solAbsTol_ )
	{
	  alphau = alpha;
	  adAlpha = alpha;
	} else if (nlpact0 - con->getLb() > solAbsTol_) {
		alphal = alpha;
	} else stop = true;
      }

      if (stop = false){
	alpha = adAlpha;
  	alphac = 1.0 - alpha;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = alphac * x[i] + alpha * xnlp[i];	
      }
*/
/*
      std::cout << "alpha0 = " << alpha0 << "\n";
      alphac = 1-alpha0;
      for (int i = 0; i < numvars; ++i)
        xalpha[i] = alphac * x[i] + alpha0 * xnlp[i];
*/
///////////////////////////////////

      stop = false;
      stop1 = false;
      alphal = 0.0;
      alphau = 1.0;
      std::fill(gr, gr+numvars, 0.0);
      con->getFunction()->evalGradient(x,gr,&error);
/*
      while (stop1 == false)
      {
        alpha = alphau; 
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];	
        nlpact0 = con->getActivity(xalpha,&error);	  

	if (nlpact0 < con->getLb() - solAbsTol_)
	  alphau++;
	else stop1 = true;
      }
      alphal = alphau - 1;
*/
      while (stop == false && (alphau - alphal) > 1e-3)
      {
	alpha = (alphal + alphau) * 0.5;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
        nlpact0 = con->getActivity(xalpha,&error);
	if (nlpact0 < con->getLb() - solAbsTol_) {
	  alphal = alpha;
	  if (nlpact0 < nlpact1){
  	    adAlpha = alpha;
	    stop1 = true;
	  }
	} else if (nlpact0 > con->getLb() + solAbsTol_) {
	    alphau = alpha;
	  } else {
	    stop = true;
	  }
	  
//	}
      }

/////////////////////////////

      if (stop == false && stop1 == true){
	alpha = adAlpha;
	for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] + alpha * dir[i];
      }

//      if (alphau - alphal > 1e-3) {
//	std::cout << " ***************** add extra cuts 2\n";
        f = con->getFunction();
        linearAt_(f,0.0,xalpha,&cnst, &lf);
        f2 = (FunctionPtr) new Function(lf);
	if (f2->eval(x,&error) + cnst < -solAbsTol_ && stop1 == true){
	  if (ctMngr_)
            newcon = ctMngr_->addCut(rel_,f2,-cnst, INFINITY,true,false);
	  else{
            newcon = rel_->newConstraint(f2, -cnst, INFINITY, "ExtraCuts");
	  }
	  ++(stats_->cuts);
//      } 
	  ncut++;
	}
    }
  }
  std::cout << "number of extra cutsss = " << ncut << "\n";
  delete [] gr;
  delete [] dir;
  delete [] xalpha;
}

void quasiQGHandler::LineSearch2_(ConstraintPtr con, const double *x,
				    double *xalpha, double *gr, Bool *admit)
{
  int error = 0;
  int numvars = minlp_->getNumVars();
  FunctionPtr f = con->getFunction();
  double ub = con->getUb();
  double act0 = f->eval(x,&error) - ub;
  double *gr1 = new double[numvars];
  //f->evalGradient(x,gr,&error);
  double gnorm = InnerProduct(gr,gr,numvars);
  double alpha = act0 / sqrt(gnorm);
  double viol = 0.0;
  
  double alpham;
  double alphau;
  double alphal = 0.0;

  double act1;
  double act2;
  double actm;
  double *x1 = new double[numvars];
  double *x2 = new double[numvars];
  double *xm = new double[numvars];
  Bool repeat = true;
  double beta1 = 0.95;
  double beta2 = 1.05;
  double K;
  int n1 = 0;
#ifdef PrEXCUT
  std::cout << "\n**********************\n";
#endif

//  std::cout << "\n**********************\n";
  gnorm = sqrt(InnerProduct(gr,gr,numvars));
  double sgnorm = 1.0; //sqrt(gnorm);
//  std::cout << " ************** gnorm = " << gnorm << "\n";
  n1 = 0;
  while (repeat == true && n1 < 1000){
    n1++;
//    std::cout << "1\n";
    for (int i = 0; i < numvars; ++i)
      xalpha[i] = x[i] - alpha * gr[i];
//    std::cout << "2\n";
    act1 = f->eval(xalpha, &error) - ub;
//    std::cout << "3\n";
#ifdef PrEXCUT
  std::cout << std::setprecision(8) << " act1 = " << act1 << " alpha = " << alpha;
#endif

//    std::cout << " 4\n";
    if (act1 < solAbsTol_){
//      std::cout << "5\n";
      repeat = false;
    } else {
//     std::cout << "6\n";
#ifdef TimerLS
      timer_->start();
#endif
      std::fill(gr1,gr1+numvars,0.0);
      f->evalGradient(xalpha,gr1,&error);

#ifdef TimerLS
      grdTime_ += timer_->query();
#endif
#ifdef PrEXCUT
      std::cout << "timer = " << timer_->query() << "\n";
#endif
      act1 = f->eval(xalpha,&error) - ub;
      viol = act1 + alpha * InnerProduct(gr1,gr,numvars);
//      std::cout << "viol = " << viol;
      if ( viol/sgnorm > solAbsTol_){
	alpha = alpha * beta2;
//        std::cout << " 6- 1\n";
      } else {
	alpha = alpha * beta1;
//        std::cout << " 6- 2\n";
      }
    }
//    std::cout << "7\n";
  }
  if (repeat == true && act1 > -solAbsTol_){
//    std::cout << "22222222222\n";
    *admit = true;
    for (int i = 0; i < numvars; ++i)
      gr[i] = gr1[i];
    delete [] gr1;
    delete [] x1;
    delete [] x2;
    delete [] xm;
    return;
  } else if (repeat == true){
//    std::cout << "3333333333\n";
    *admit = false;
    delete [] gr1;
    delete [] x1;
    delete [] x2;
    delete [] xm;
    return;
  }
#ifdef PrEXCUT
  std::cout << " act1 = " << act1 << " ";
  std::cout << std::setprecision(8) << "  grdTime = " << grdTime_ << "\n";
#endif
//  std::cout << "8\n";
  if (act1 > -solAbsTol_){
//    std::cout << "9\n";
//    for (int i = 0; i < numvars; ++i)
//      xalpha[i] = x[i] - alpha * gr[i];
#ifdef TimerLS
    timer_->start();
#endif
    std::fill(gr,gr+numvars,0.0);
    f->evalGradient(xalpha,gr,&error); 
#ifdef TimerLS
    grdTime_ += timer_->query();
#endif

//    std::cout << "44444444444\n";
    *admit = true;
    delete [] gr1;
    delete [] x1;
    delete [] x2;
    delete [] xm;
    return;
  } else {
//    std::cout << "10\n";
    alphau = alpha;
    repeat = true;
    for (int i = 0; i < numvars; ++i){
      x1[i] = x[i];
      x2[i] = x[i] - alphau * gr[i];
    }
    //std::cout << "before while  ";
    act1 = f->eval(x1,&error) - ub;
    act2 = f->eval(x2,&error) - ub;

    n1 = 0;
    while (true == repeat && abs(alphal - alphau) > 1e-3){
//      std::cout << "11  alphal = " << alphal << " alphau " << alphau << "\n";
//      std::cout << std::setprecision(8) << " act1 = " << act1 << " act2 = " << act2 << " ";
      K = (act2 - act1)/(alphau - alphal);
      alpham = - act1 / K + alphal;
//      std::cout << " alpham = " << alpham << " ";
      for (int i = 0; i < numvars; ++i)
        xm[i] = x[i] - alpham * gr[i];

      actm = f->eval(xm,&error) - ub;
//      std::cout << " actm = " << actm << "\n";
      if (actm < -solAbsTol_){
        alphau = alpham;
        for (int i = 0; i < numvars; ++i)
          x2[i] = x[i] - alphau * gr[i];

        act2 = actm;
      } else if (actm > solAbsTol_) {
        alphal = alpham;
        for (int i = 0; i < numvars; ++i)
          x1[i] = x[i] - alphal * gr[i];
        act1 = actm;
      } else {
        repeat = false;
        for (int i = 0; i < numvars; ++i)
          xalpha[i] = x[i] - alpham * gr[i];
      }
#ifdef PrEXCUT
      std::cout << "  alpham = " << alpham << " actm = " << actm;
#endif

    }
    if (repeat == true && actm > -solAbsTol_){
      *admit = true;
      for (int i = 0; i < numvars; ++i)
	gr[i] = gr1[i];

//      std::cout << "5555555555555\n";
      delete [] gr1;
      delete [] x1;
      delete [] x2;
      delete [] xm;
      return;
    } else if (repeat == true){
//      std::cout << "6666666666\n";
      *admit = false;
      delete [] gr1;
      delete [] x1;
      delete [] x2;
      delete [] xm;
      return;
    }
  }
#ifdef PrEXCUT
  std::cout << "\n**********************\n";
#endif

#ifdef TimerLS
  timer_->stop();
#endif
  for (int i = 0; i < numvars; ++i)
    gr[i] = gr1[i];

  delete [] gr1;
  delete [] x1;
  delete [] x2;
  delete [] xm;

}

double quasiQGHandler::LineSearch1_(ConstraintPtr con, const double *x,
				    double *xalpha, double *gr)
{
  int error = 0;
  int numvars = minlp_->getNumVars();
  FunctionPtr f = con->getFunction();
  double ub = con->getUb();
  double act0 = f->eval(x,&error) - ub;
//  double *gr  = new double[numvars];
  double *gr1 = new double[numvars];
  f->evalGradient(x,gr,&error);
  double gnorm = InnerProduct(gr,gr,numvars);
  double alphal = 0.0;
  double alphau;
  //double alpha = act0 / sqrt(gnorm);
  double alpha = 10.0;
  double alpham;

  double act1;
  double act2;
  double actm;
  double *x1 = new double[numvars];
  double *x2 = new double[numvars];
  double *xm = new double[numvars];

//  double xalpha = new double[numvars];
  
  Bool repeat = true;
  double beta1 = 0.9;
  double beta2 = 1.1;
  double K;
  
  while (repeat == true){
    for (int i = 0; i < numvars; ++i)
      xalpha[i] = x[i] - alpha * gr[i];
    act1 = f->eval(xalpha, &error) - ub;
    if (act1 < solAbsTol_){
      repeat = false;
    } else {
//      std::fill(gr1,gr1+numvars,0.0);
      alpha = alpha * beta2;
    }
  }
  std::cout << "alphau = " << alpha << "\n";
  alphau = alpha;
  alphal = 0.0;
  if (act1 > -solAbsTol_){
//    for (int i = 0; i < numvars; ++i)
//      xalpha[i] = x[i] - alpha * gr[i];
    std::fill(gr,gr+numvars,0.0);
    f->evalGradient(xalpha,gr,&error); 
    delete [] gr1;
    return alpha;
  } 
  repeat = true;
  for (int i = 0; i < numvars; ++i){
    x1[i] = x[i];
    x2[i] = x[i] - alphau * gr[i];
  }
  while (true == repeat){
    act1 = f->eval(x1,&error) - ub;
    act2 = f->eval(x2,&error) - ub;
    K = (act1 - act2)/(alphal - alphau);
    alpham = (K*alphal - act1)/K;
    for (int i = 0; i < numvars; ++i)
      xm[i] = x[i] - alpham * gr[i];

    actm = f->eval(xm,&error) - ub;
    if (actm < -solAbsTol_){
      alphau = alpham;
      for (int i = 0; i < numvars; ++i)
        x2[i] = x[i] - alphau * gr[i];
    } else if (actm > solAbsTol_) {
      alphal = alpham;
      for (int i = 0; i < numvars; ++i)
        x1[i] = x[i] - alphal * gr[i];
    } else {
      repeat = false;
      for (int i = 0; i < numvars; ++i)
        xalpha[i] = x[i] - alpham * gr[i];
      return alpham;
    }
  }
}

double quasiQGHandler::LineSearch_(ConstraintPtr con, const double *x, 
				   double *xalpha, double *gr)
{
  double alpha0 = 0.0;
  double alpham = 100.0;
  double alpha = 100.0;
  double alphai = 100.0;
  Bool repeat = true;
  double alpha2 = alpha0;
  double alpha1 = alphai;
  
  double c1 = 1e-4;
  double c2 = 0.9;
  int numvars = minlp_->getNumVars();
  int error= 0.0;
  FunctionPtr f = con->getFunction();
  double ub = con->getUb();
  double act = 0.0;
  double act1= 0.0;
  double act0 = f->eval(x,&error) - ub;
  double gnorm = InnerProduct(gr,gr,numvars);
  double *gr1 = new double[numvars];
  double phi_p_i = 0.0;

  std::cout << "beginning of LineSearch_\n";
  while (repeat==true) {
    for (int i = 0; i < numvars; ++i) 
      xalpha[i] = x[i] - alphai * gr[i];
    act = f->eval(xalpha, &error) - ub;
//    std::cout << "act = " << act << " ";
//    std::cout << "gnorm = " << gnorm << " ";
//    std::cout << std::setprecision(8) << "ac0 = " << act0 << " ";
    if (act > act0 - c1 * alphai * gnorm + solAbsTol_){
      std::cout << "\nbeginning of zoom_ 1\n";
      alpha = zoom_(con,alpha1,alpha2,act0,gnorm,x,gr);
      std::cout << "end of zoom_ 1\n\n\n";
      repeat = false;
    }
    std::fill(gr1,gr1+numvars,0.0);
    f->evalGradient(xalpha,gr1,&error);
    phi_p_i = - InnerProduct(gr,gr1,numvars);
//    std::cout << "phi_p_i = " << phi_p_i << " ";
    //act1 = abs(phi_p_i);
    if (abs(phi_p_i) < c2 * gnorm + solAbsTol_){
      std::cout << "1 ";
      repeat = false;
      alpha = alphai;
    }
    if (phi_p_i >-solAbsTol_){
      std::cout << "\nbeginning of zoom_ 2\n";
      alpha = zoom_(con,alpha1,alpha2,act0,gnorm,x,gr);
      std::cout << "end of zoom_ 2\n\n\n";
      repeat = false;
    }
    alpha2 = alphai;
    alphai = (alphai + alpham)/2;
    alpha1 = alphai;
    std::cout << " alphai = " << alphai;
  }
  std::cout << "end of LineSearch_\n";
  return alpha;
}

double quasiQGHandler::zoom_(ConstraintPtr con, double alpha1, double alpha2, double fval, double gnorm,
			     const double *x, double *gr)
{


  int numvars = minlp_->getNumVars();
  double *xalpha1 = new double[numvars];
  double *xalpha2 = new double[numvars];
  double *xalphaj = new double[numvars];
  double *grj = new double[numvars];
  double *xalpha = new double[numvars];
  double actj = 0.0;
  FunctionPtr f = con->getFunction();
  double ub = con->getUb();
  int error = 0.0;
  double c1 = 1e-4;
  double c2 = 0.9;
  

  for (int i = 0; i < numvars; ++i){
    xalpha2[i] = x[i] - alpha2 * gr[i];
    xalpha1[i] = x[i] - alpha1 * gr[i];
  }

  double act2 = f->eval(xalpha2,&error) - ub;
  double act1 = f->eval(xalpha1,&error) - ub;
  double phi_p_j;

  Bool repeat = true;
  double alpha;
 
  while (repeat == true){
    double alphaj = (alpha1 + alpha2)/2;
    for (int i = 0; i < numvars; ++i)
      xalpha[i] = x[i] - alphaj * gr[i];
    actj = f->eval(xalpha,&error) - ub;
    if (actj > fval - c1 * alphaj * gnorm + solAbsTol_ || actj > act2){
      alpha1 = alphaj;
      for (int i = 0; i < numvars; ++i){
        xalpha1[i] = x[i] - alpha1 * gr[i];
      }
    } else {
      std::fill(grj,grj+numvars,0.0);
      f->evalGradient(xalphaj,grj,&error);
      phi_p_j = - InnerProduct(grj,gr,numvars);
      if (abs(phi_p_j) <  c2 * gnorm + solAbsTol_){
        alpha = alphaj;
 	repeat = false;
      } 
      if (phi_p_j*(alpha1-alpha2) > -solAbsTol_){
	alpha1 = alpha2;
        for (int i = 0; i < numvars; ++i){
          xalpha1[i] = x[i] - alpha1 * gr[i]; 
         }
      }
      alpha2 = alphaj;
      for (int i = 0; i < numvars; ++i){
        xalpha2[i] = x[i] - alpha2 * gr[i];
      }
    }
  }

}

int quasiQGHandler::alphaCutProj_(const double *x_OA, const double *x_pr, SeparationStatus *status)
{
  
  ConstraintPtr con;
  double act;
  int error;
  numvars_=minlp_->getNumVars();
  double *x_alpha = new double[numvars_];
  int ncut = 0;
  LinearFunctionPtr lf;
  FunctionPtr f;
  FunctionPtr f2;
  double lpact, nlpact, c;
  ConstraintPtr newcon;
  double lpeval;
  std::cout << "alphaCut begins\n";
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it)
  {
    std::cout << "const id = " << (*it)->getId() << ": ";
    con=*it;
    act = con->getActivity(x_pr, &error);
    std::cout << con->getLb() << ", " << act << ", " << con->getUb() << "\n";
    if (con->getUb() < INFINITY && act-con->getUb() > -solAbsTol_)
    {
      std::cout << "constraints <= \n";
      std::fill(x_alpha,x_alpha+numvars_,0.0);
//      lpeval = con->getActivity(x_pr_, &error);
//      if (lpeval-con->getUb()>solAbsTol_){
      cutXLP_(x_pr, x_OA, x_alpha, con, true);
      f = con->getFunction();
      linearAt_(f, 0.0 , x_alpha, &c, &lf);
      f2 = (FunctionPtr) new Function(lf);
      lpact = f2->eval(x_OA, &error);
      //nlpact = con->getActivity(x_OA, &error);
      if (lpact + c > solAbsTol_ )
      {
	if (ctMngr_)
          newcon = ctMngr_->addCut(rel_,f2,-INFINITY,-c,true,false);
	else{
          newcon = rel_->newConstraint(f2, -INFINITY, -c, "alphaCut");
	}
	++(stats_->cuts);
        ++ncut;
      }
//      }
    }
    if (con->getLb() > -INFINITY && act-con->getLb() < solAbsTol_)
    {
      std::cout << "constraints >= \n";
      std::fill(x_alpha,x_alpha+numvars_,0.0);
//      lpeval = con->getActivity(x_pr_, &error);
//      if (lpeval-con->getLb()<-solAbsTol_){
      cutXLP_(x_pr, x_OA, x_alpha, con, false);
      f=con->getFunction();
      linearAt_(f, 0.0, x_alpha, &c, &lf);
      f2=(FunctionPtr) new Function(lf);
      lpact=f2->eval(x_OA,&error);
      //nlpact=con->getActivity(x_OA,&error);
      if (lpact + c < -solAbsTol_){
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,-c,INFINITY,true,false);
	else{
          newcon = rel_->newConstraint(f2, -c, INFINITY, "alphaCut");
	}
	++(stats_->cuts);
        ++ncut;
      }
      //      OAFromPoint_(x_alpha, x_OA, status);
//      }
    }
  }
  if (ncut > 0)
    *status = SepaResolve;
  return ncut;
}

int quasiQGHandler::pure_project_(const double *x_OA, SeparationStatus *status)
{

//  std::cout << "start doing pure project\n";
  FunctionPtr orjProjObj = proj_minlp_->getObjective()->getFunction();
  LinearFunctionPtr lf_cut;
  FunctionPtr f2;
  ConstraintPtr newcon;

  VariableConstIterator vbeg = minlp_->varsBegin();
  VariableConstIterator vend = minlp_->varsEnd();
  double optval;
 
  lf_pure_->clearAll();
  FunctionPtr f;
  VariablePtr v;
  int error;
  double c;
  int ncut=0;
  double lpact;

  for (int i = 0; i <numvars_ ; i++)
  {
    v = proj_minlp_->getVariable(i); 
    lf_pure_->addTerm(v, -1.0 * x_OA[i]);
  }
  f = (FunctionPtr) new Function(lf_pure_, qf_pure_); 
//  std::cout << "pure projection objective\n";
//  f->write(std::cout);
  proj_nlpe_->clear();
  proj_minlp_->removeObjective();
  proj_minlp_->newObjective(f, 0, Minimize);
  proj_minlp_->calculateSize(true);
  proj_minlp_->setNativeDer();
//  proj_minlp_->setHessian();
  
  proj_nlpe_->load(proj_minlp_);
  projnlpStatus_ = proj_nlpe_->solve();
  ++(stats_->nlpP);

  switch (projnlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    x_pr_ = proj_nlpe_->getSolution()->getPrimal();
    proj_minlp_->getObjective()->evalGradient(x_pr_, a_, &error);
    optval = proj_nlpe_->getSolutionValue() + .5 * InnerProduct(x_OA, x_OA, numvars_);

    if (optval > solAbsTol_)
    {
      for (int i = 0; i < numvars_; i++){
        if (fabs(a_[i]) < solAbsTol_){
          a_[i] = 0;
        }
      }
      lf_cut = (LinearFunctionPtr) new LinearFunction(a_, vbeg, vend, linCoeffTol_ );
      c = InnerProduct(a_, x_pr_, numvars_);
      f2 = (FunctionPtr) new Function(lf_cut);
    
/*
    std::cout << "pure projection optimal value = " << proj_nlpe_->getSolutionValue() + .5 * InnerProduct(x_OA, x_OA, n) << std::endl;
   newcon = rel_->newConstraint(f2, c, INFINITY, "pureprojlnrztn_cut");
    for (UInt i = 0; i < n; i++)
        std::cout << "(" << i+1 << "," << x_OA_[i] << "," << x_pr_[i] << ") "; 
    std::cout << std::endl;
    std::cout << "pure projection:\n";
    newcon->write(std::cout);
*/
      lpact = f2->eval(x_OA, &error);
      if (lpact - c < -solAbsTol_) {
	if (ctMngr_)
	  newcon = ctMngr_->addCut(rel_,f2,c,INFINITY,true,false);
	else{
          newcon = rel_->newConstraint(f2, c, INFINITY, "pureprojlnrztn_cut");
	}
	++(stats_->cuts);
        if (doACCPM_)
        {
//          qgACP_->addCut(a_, c, false);
        }
        ncut += 1;
        *status = SepaResolve;
      }
    } 
  break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
  case (ProvenUnbounded):
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "projNLP engine status = " 
                                 << nlpStatus_ << std::endl;
    assert(!"quasiQGHandler: Cannot proceed further");
  }
   
  proj_nlpe_->clear();
  proj_minlp_->newObjective(orjProjObj, 0, Minimize);
  proj_minlp_->calculateSize(true);
  proj_minlp_->setNativeDer();
  //proj_minlp_->calculateSize(true);
  //HessianOfLagPtr hPtr = (HessianOfLagPtr) new HessianOfLag(proj_minlp_.get());
  //proj_minlp_->setHessian(hPtr);
//  proj_minlp_->setQuadraticHessian();
  
  return ncut;

}

void quasiQGHandler::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf, double *a, double *k)
{
  int n = minlp_->getNumVars();
//  double *a = new double[n];
  VariableConstIterator vbeg = minlp_->varsBegin();
  VariableConstIterator vend = minlp_->varsEnd();
  int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_); 
  *k = InnerProduct(x,a,n);
  *c  = fval - *k;
//  delete [] a;
}


void quasiQGHandler::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf, double *a)
{
  int n = minlp_->getNumVars();
//  double *a = new double[n];
  VariableConstIterator vbeg = minlp_->varsBegin();
  VariableConstIterator vend = minlp_->varsEnd();
  int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_); 
  *c  = fval - InnerProduct(x, a, n);
//  delete [] a;
}


void quasiQGHandler::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf)
{
  int n = minlp_->getNumVars();
  double *a = new double[n];
  VariableConstIterator vbeg = minlp_->varsBegin();
  VariableConstIterator vend = minlp_->varsEnd();
  int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_); 
  *c  = fval - InnerProduct(x, a, n);
  delete [] a;
}


void quasiQGHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval, 
                          Bool *sol_found)
{
  double     val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();

  //incSol_ = s_pool->getBestSolution()->getPrimal();
  //BincSol_ = true;
  

  if (val <= bestval) { // - solAbsTol_) {
    const double *x = nlpe_->getSolution()->getPrimal();
#if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "new solution found, value = " << val << std::endl;
#endif
    s_pool->addSolution(x, val);
    *sol_found = true;
  } else {
#if SPEW
    logger_->msgStream(LogDebug) 
      << std::setprecision(10)
      << me_ << "new solution is suboptimal, value = " << val 
      << " best value = " << bestval << std::endl;
#endif
  }
  *nlpval = val;
}

void quasiQGHandler::updateUb_new_(SolutionPoolPtr s_pool, double *nlpval,
                              double *xhat, Bool *sol_found)
{
//  double val = e->getSolutionValue();
  //const double *x1 = new double[numvars_];
  std::cout << "proj_nlpe_= " << proj_nlpe_->getStatusString() << "\n";
  int error = 0;
  double val = minlp_->getObjValue(xhat, &error);
  std::cout << "val = " << val << "\n";
  double bestval = s_pool->getBestSolutionValue();

  if (val <= bestval){ 
    //const double *x1 = proj_nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(xhat, val);
    *sol_found = true;
  } else {
#if SPEW
    logger_->msgStream(LogDebug) 
      << std::setprecision(10)
      << me_ << "new solution is suboptimal, value = " << val 
      << " best value = " << bestval << std::endl;
#endif
  }
  *nlpval = val;
}

void quasiQGHandler::ProjObjUpdate(const double * x1, double& CTerm)
{
  VariablePtr v;
  CTerm = 0.0;
  proj_nlpe_->clear();
  FunctionPtr projObj;
  proj_qf_ = proj_minlp_->getObjective()->getFunction()->getQuadraticFunction();
  LinearFunctionPtr lf_ = (LinearFunctionPtr) new LinearFunction();

//  double funeval; 
//  double consact;
//  int error;
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Function evals: \n";
  for (UInt i = 0; i < minlp_->getNumCons(); i++){
    funeval = minlp_->getConstraint(i)->getFunction()->eval(x_OA_, &error);
    consact = funeval - minlp_->getConstraint(i)->getUb();
    if (consact > solAbsTol_){
      logger_->msgStream(LogDebug) << "Uconst[" << i << "] = " << consact << "\n";
    }  
    consact = funeval - minlp_->getConstraint(i)->getLb();
    if (consact < -1.0 * solAbsTol_){
      logger_->msgStream(LogDebug) << "Bconst[" << i << "] = " << consact << "\n";
    }
  }
#endif
  
///*
//  std::cout << "update projection" << std::endl;
//  for (UInt j = 0; j < proj_minlp_->getNumVars(); j++)
//    std::cout << "(" << j+1 << "," << x1[j] << ") ";

  double xval;
  //proj_lf_->clearAll();
//  proj_minlp_->write(std::cout);
  int i=0;
  for (i = 0; i < numvars_; i++)
  {
    v = proj_minlp_->getVariable(i);
    if (v->getType() == Binary || v->getType() == Integer)
    {
//      if (fabs(x1[i] > tol_)){
        xval = x1[i];
        xval = floor(xval+0.5);
        lf_->addTerm(v, -1.0*constCoef_*xval);
        CTerm += xval * xval * constCoef_;
//      }
    }
  }

  projObj = (FunctionPtr) new Function(lf_, proj_qf_);
  proj_minlp_->newObjective(projObj,0,Minimize);
  proj_minlp_->calculateSize(true);
//  std::cout << "after update\n";
//  proj_minlp_->write(std::cout);

}

void quasiQGHandler::readSol_()
{
  FILE *file = fopen("solution.txt", "r");
  UInt i = 0;

  if (file != NULL)
  {
    char line [128];
    while (fgets (line, sizeof line, file) != NULL)
    {
      x_star[i] = strtod(line, NULL);
      i++;
    }
    fclose( file );
//    file.close();
  }
}

void quasiQGHandler::ACCutsToRelax_(QGNewCut qgnewcut)
{
  std::vector<double*> gr = qgnewcut.coef;
  std::vector<double> c = qgnewcut.rhs;
  ConstraintPtr newcon;
  //int n = qgnewcut.n;
  int n1 = rel_->getNumVars();
 
  VariableConstIterator vbeg = rel_->varsBegin();
  VariableConstIterator vend = rel_->varsEnd();
  double *a = new double[n1];

  LinearFunctionPtr lf;
  FunctionPtr f;
  
  for (UInt i = 0; i < c.size(); i++)
  {
    a = gr[i];
    lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_);
    f = (FunctionPtr) new Function(lf);
    if (ctMngr_)
      newcon = ctMngr_->addCut(rel_,f,-INFINITY,c[i],true,false);
    else {
      newcon = rel_->newConstraint(f, -INFINITY, c[i]);
    }
    ++(stats_->cuts);
  }

  ConstraintPtr con = rel_->getConstraint(ObjId_);
//  if (con->getUb() > qgnewcut.newUb + eTol_)
//    rel_->changeBound(con, -INFINITY, qgnewcut.newUb);
  
}

void quasiQGHandler::no_good(const double *x)
{
  int numvars = proj_minlp_->getNumVars();
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;
  VariablePtr v;
  ConstraintPtr c;
  int lhs = 1;

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Adding no-good cut\n";
#endif
  std::cout << " **********************   Adding no-good cut  ************************** \n";
  std::cout << "numvars = " << numvars << "\n";
  for (int i = 0; i < numvars; i++)
  {
    v = minlp_->getVariable(i);
    if (v->getType() == Binary || v->getType()==Integer)
    { 
      if (x[i] < solAbsTol_)
      {
        lf->addTerm(v, 1.0);
      } else if (x[i] > 1-solAbsTol_)
        {
          lf->addTerm(v, -1.0);
          lhs += -1.0;
        }
    }
  }
  f = (FunctionPtr) new Function(lf);
//  std::cout << "no good function = ";
  f->write(std::cout);
  std::cout << "\n";
  std::cout << "no good lhs = " << lhs << "\n";
  if (ctMngr_)
    c = ctMngr_->addCut(rel_,f,lhs,INFINITY,true,false);
  else{
    c = rel_->newConstraint(f, lhs, INFINITY, "no_good");
  }
  ++stats_->cuts;
}

/*
quasiACProblem::quasiACProblem()
: nlNewCons_(0),
  nlNewVars_(0),
  eTol_(1e-4),
  linCoeffTol_(1e-5),
  quasiACProblem_(ProblemPtr())
{

}

quasiACProblem::quasiACProblem(EnvPtr env, EnginePtr ACnlpe, RelaxationPtr ACrel, Bool QC, std::vector<ConstraintPtr> nlCons)
: nlNewCons_(0),
  nlNewVars_(0),
  eTol_(1e-4),
  linCoeffTol_(1e-5),
  quasiACProblem_(),
  ACrel_(ACrel),
  nlCons_(nlCons),
  handleQuasiConvex_(QC),
  AC_Engine_(ACnlpe),
  AC_EngStatus_(EngineUnknownStatus)
{
  ///Is this correct to rel_ instead of minlp_?
//  logger_ = (LoggerPtr) new Logger(LogDebug2);
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("qgAC_log_level")->getValue());

#if SPEW
  logger_->msgStream(LogDebug) << me_ << " Relaxation problem passed to ACCPM\n";
//  ACrel_->write(logger_->msgStream(LogDebug));
#endif
  quasiACProblem_ = (ProblemPtr) new Problem();
  quasiACProblem_->newVariables(ACrel_->varsBegin(), ACrel_->varsEnd());

  ConstraintPtr c, newcon;

  LinearFunctionPtr lf1;
  FunctionPtr f1;
  VariablePtr v;
  lf1 = ACrel_->getObjective()->getFunction()->getLinearFunction();

  v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous);
  nlNewVars_.push_back(v);
  lf1->addTerm(v, 1.0);

  f1 = (FunctionPtr) new Function(lf1);
  ///TODO How do I get the lower and upper bouns of the relaxation problem
  c = quasiACProblem_->newConstraint(f1, -INFINITY, INFINITY);
  
//  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
//    ++it) {
//    c = *it;
//    if (c->getFunctionType()==Constant || c->getFunctionType() == Linear) {
//      newcon = quasiACProblem_->newConstraint(c->getFunction(),c->getLb(), c->getUb());
//      consAC_.push_back(newcon);
//    }
//  }
  for (UInt i = 0; i < ACrel_->getNumCons(); i++)
  {
    c = ACrel_->getConstraint(i);
    newcon = quasiACProblem_->newConstraint(c->getFunction(), c->getLb(), c->getUb(),c->getName());
  }
  
  ACLogBar_ = (CGraphPtr) new CGraph();
  createLogBarrier_();    
}

quasiACProblem::~quasiACProblem()
{
}

void quasiACProblem::createLogBarrier_()
{
 
  CNodeVector AC_Nodes_vars;
//  CNodeVector AC_Nodes_logs;
  CNode *cnode0, *cnode1, *cnode2, *cnode3;
  LinearFunctionPtr lf1;
  QuadraticFunctionPtr qf1;
//  FunctionPtr f;
//  FunctionPtr fobj;

  double lb;
  double ub;

  //VariableConstIterator v_iter;
  VariablePtr v;

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "ACCPM is creating Log Barrier function\n"; 
#endif
//  for (v_iter = quasiACProblem_->varsBegin(); v_iter != quasiACProblem_->varsEnd(); v_iter++)
  for (UInt i = 0; i < nlNewVars_.size(); i++)
  {
    v = nlNewVars_[i];
    ub = v->getUb();
    lb = v->getLb();
    if (ub < INFINITY && ub != 0.0)
    {
      cnode0 = ACLogBar_->newNode(ub);
      cnode1 = ACLogBar_->newNode(v);
      AC_Nodes_vars.push_back(cnode1);
      cnode2 = ACLogBar_->newNode(OpMinus, cnode0, cnode1);
      finalNode_ = ACLogBar_->newNode(OpLog, cnode2, NULL);
      AC_Nodes_logs_.push_back(cnode3);
    } else if (ub < INFINITY)
           {
             cnode1 = ACLogBar_->newNode(v);
             finalNode_ = ACLogBar_->newNode(OpLog, cnode1, NULL);
           }
    if (lb > -INFINITY && lb != 0.0)
    {
      cnode0 = ACLogBar_->newNode(lb);
      cnode1 = ACLogBar_->newNode(v);
      //AC_Nodes_vars.push_back(cnode1);
      cnode2 = ACLogBar_->newNode(OpMinus, cnode1, cnode0);
      finalNode_ = ACLogBar_->newNode(OpLog, cnode2, NULL);
      AC_Nodes_logs_.push_back(finalNode_);
    } else if (lb > -INFINITY)
           {
             cnode1 = ACLogBar_->newNode(v);
             finalNode_ = ACLogBar_->newNode(OpLog, cnode1, NULL);
             AC_Nodes_logs_.push_back(finalNode_);
           }
   
  }
*/
 
/*
  std::cout << "size of vector = " << AC_Nodes_logs_.size() << "\n";
  if (AC_Nodes_logs_.size() < 2)
  {
#if SPEW 
    logger_->msgStream(LogDebug) << me_ << "log vector has only one node\n";
#endif    
    finalNode_ = AC_Nodes_logs_[0];
    ACLogBar_->setOut(finalNode_);
  
  } else {
    finalNode_ = ACLogBar_->newNode(OpSumList, &AC_Nodes_logs_[0], AC_Nodes_logs_.size());  
    ACLogBar_->setOut(finalNode_);
  }
*/

/*
  ACLogBar_->setOut(finalNode_);
  ACLogBar_->finalize();

  ACObjFunc_ = (FunctionPtr) new Function(ACLogBar_);
#if SPEW
  logger_->msgStream(LogDebug) << me_ << " Log Barrier function\n";
  ACObjFunc_->write(logger_->msgStream(LogDebug)); 
#endif
  
  quasiACProblem_->newObjective(ACObjFunc_, 0.0, Minimize, "ACObj");

  quasiACProblem_->calculateSize();  
}

void quasiACProblem::ACUpdate_(const double *x)
{

#if SPEW
  logger_->msgStream(LogDebug) << me_ << " update AC problem\n";
#endif
  ConstraintPtr c;
  double act = - INFINITY;
  int error;
  int numvars = ACrel_->getNumVars();
  LinearFunctionPtr lf = LinearFunctionPtr();
  FunctionPtr f, f2;
  ConstraintPtr newcon;
  VariablePtr v;
  double rhs;
//  double *a = new double[numvars];

  double *x1 = new double[numvars];
  double *grd = new double[numvars];

  double objvalue = ACrel_->getObjValue(x, &error);
  AC_Engine_->clear();
  for (int i = 0; i < numvars; i++)
    x1[i] = x[i];
  
  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it)
  {
    c = *it;
    act = c->getActivity(x1, &error);
    if ((c->getUb() < INFINITY && std::max(act - c->getUb(),0.)>eTol_) || (c->getLb() > -INFINITY && std::max(c->getLb()-act,0.)>eTol_))  {
      /// add feasibility cut
      if (!handleQuasiConvex_){
        f = c->getFunction();
        linearAt_(f, act, x1, &rhs, &lf);
*/

        /* add the constraint to the relaxation rel_ */
        //f2 = (FunctionPtr) new Function(lf);
        //newcon = ACrel_->newConstraint(f2, -INFINITY, rhs, "feasCut");
        /* *** */
/*
        v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous, "S");
        nlNewVars_.push_back(v);
        lf->addTerm(v, 1.0);
        f2 = (FunctionPtr) new Function(lf);
        newcon = quasiACProblem_->newConstraint(f2, rhs, rhs, "feasCut");
        consAC_.push_back(newcon);

        updateLogBar_(v);
      } else {
        act = c->getActivity(x1, &error);
        /// add feasibility cut
        f = c->getFunction();
        linearAt_(f, act, x1, &rhs, &lf);
        f->evalGradient(x1, grd, &error);
        rhs = InnerProduct(x1, grd, numvars);
*/
        /* add the constraint to the relaxation rel_ */
        //f2 = (FunctionPtr) new Function(lf);
        //newcon = ACrel_->newConstraint(f2, -INFINITY, rhs, "feasCut");
        /* *** */
/*
        v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous, "S");
        nlNewVars_.push_back(v);
        lf->addTerm(v, 1.0);
        f2 = (FunctionPtr) new Function(lf);
        newcon = quasiACProblem_->newConstraint(f2, rhs, rhs, "feasCut");
        consAC_.push_back(newcon);
      }
    } 

  }
      /// add optimality cut
      /// updateUb_(s_pool, &objvalue, sol_found); 
      objvalue = ACrel_->getObjValue(x, &error);
//      lf->addTerm(objVar_, 1.0);
//      v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous, "S");
//      nlNewVars_.push_back(v);
      ConstraintPtr con;
      con = quasiACProblem_->getConstraint(0);
      if (ACrel_->getObjective()->getObjectiveType() == Minimize){
//        lf->addTerm(v, 1.0);
        if (objvalue < con->getUb()-eTol_)
        {
          quasiACProblem_->changeBound(con, con->getLb(), objvalue);   /// Change the upper bound to the new found upper bound for Min problem
//          linearAt_(con->getFunction(),con->getActivity(x1, &error), x, &rhs, &lf);
          newObjBound = objvalue;
        }
      }
      else {
//        lf->addTerm(v, -1.0);
        if (objvalue > con->getLb()+eTol_)
          quasiACProblem_->changeBound(con, objvalue, con->getUb());  /// Change the lower bound to the new found lower bound for Max problem
      }
      //newcon = quasiACProblem_->newConstraint(f2, objvalue, objvalue, "optCut");
      //consAC_.push_back(newcon);
*/
      /* add the constraint to the relaxation rel_ */
      // newcon = rel_->newConstraint(f2, objvalue, objvalue, "feasCut");
      //objVar_->setUb(objvalue);
      /* *** */
//    }
//  }
/*
    else if (c->getLb() > -INFINITY) {
      act = c->getActivity(x1, &error);
      if (std::max(c->getLb()-act,0.)>eTol_){
        /// add feasibility cut
        if (!handleQuasiConvex_){
          f = c->getFunction();
          linearAt_(f, act, x1, &rhs, &lf);
          v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous, "S");
          lf->addTerm(v, 1.0);
          f2 = (FunctionPtr) new Function(lf);
          quasiACProblem_->newConstraint(f2, rhs, rhs, "feasCut");
          updateLogBar_(v);
        } else {
          act = c->getActivity(x1, &error);
          /// add feasibility cut
          f = c->getFunction();
          linearAt_(f, act, x1, &rhs, &lf);
          f->evalGradient(x1, grd, &error);
          rhs = InnerProduct(x1, grd, numvars);
          v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous, "S");
          lf->addTerm(v, 1.0);
          f2 = (FunctionPtr) new Function(lf);
          quasiACProblem_->newConstraint(f2, rhs, rhs, "feasCut");
          updateLogBar_(v);
        }
      } else {
        /// add optimality cut
        updateUb_(s_pool, &objvalue, sol_found); 
        lf->addTerm(objVar_, 1.0);
        v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous, "S");
        if (minlp_->getObjective()->getObjectiveType() == Minimize)
          lf->addTerm(v, 1.0);
        else
          lf->addTerm(v, -1.0);
        quasiACProblem_->newConstraint(f2, objvalue, objvalue, "optCut");
        updateLogBar_(v);
      }

    }
  }
*/
//}
/*
void quasiACProblem::updateLogBar_(VariablePtr v)
{ 
#if SPEW
  logger_->msgStream(LogDebug) << me_ << " updating Log Barrier\n";
#endif

  CNode *cnode0, *cnode1, *cnode2, *cnode3, *cnode4;
  double lb, ub;
  FunctionPtr fobj;
  LinearFunctionPtr lf1;
  QuadraticFunctionPtr qf1;

  ub = v->getUb();
  lb = v->getLb();

  if (ub < INFINITY){
    cnode0 = ACLogBar_->newNode(ub);
    cnode1 = ACLogBar_->newNode(v);
    cnode2 = ACLogBar_->newNode(OpMinus, cnode0, cnode1);
    cnode3 = ACLogBar_->newNode(OpLog, cnode2, NULL);
    AC_Nodes_logs_.push_back(cnode3);
  } 
 
  
  if (lb > -INFINITY)
  { 
    if (lb != 0.0)
    {
      cnode0 = ACLogBar_->newNode(lb);
      cnode1 = ACLogBar_->newNode(v);
      cnode2 = ACLogBar_->newNode(OpMinus, cnode1, cnode0);
      cnode4 = ACLogBar_->newNode(OpLog, cnode2, NULL);
      AC_Nodes_logs_.push_back(cnode4);  
    } else 
      {
        cnode1 = ACLogBar_->newNode(v);
        cnode4 = ACLogBar_->newNode(OpLog, cnode1, NULL);
        AC_Nodes_logs_.push_back(cnode4);
      }
  } 
  

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "number of log terms = " << AC_Nodes_logs_.size() << "\n";
#endif
//  finalNode_ = ACLogBar_->newNode(OpSumList, &AC_Nodes_logs_[0], AC_Nodes_logs_.size());  
  finalNode_ = ACLogBar_->newNode(OpPlus, finalNode_, cnode4);
  ACLogBar_->setOut(finalNode_);
  ACLogBar_->finalize();

  ACObjFunc_ = (FunctionPtr) new Function(lf1, qf1, ACLogBar_);

  quasiACProblem_->calculateSize();   
}
*/
/*
void quasiACProblem::createACP_()
{

  ///Is this correct to rel_ instead of minlp_?
  quasiACProblem_->newVariables(rel_->varsBegin(), rel_->varsEnd());

  ConstraintPtr c, newcon;
  
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
    ++it) {
    c = *it;
    if (c->getFunctionType()==Constant || c->getFunctionType() == Linear) {
      newcon = quasiACProblem_->newConstraint(c->getFunction(),c->getLb(), c->getUb());
      consAC_.push_back(newcon);
    }
  }
  
  createLogBarrier_();  
}
*/

/*
void quasiACProblem::addCut(double *coef, double ub, Bool IneDir)
{

#if SPEW
  logger_->msgStream(LogDebug) << me_ << " adding cuts to ACP\n";
#endif

  VariableConstIterator vbeg = ACrel_->varsBegin();
  VariableConstIterator vend = ACrel_->varsEnd();
  //ConstraintPtr newcon;
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction(coef, vbeg, vend,linCoeffTol_) ;

  VariablePtr v;
  
  v = quasiACProblem_->newVariable(0.0, INFINITY, Continuous);
  nlNewVars_.push_back(v);
  if (IneDir == true)
    lf->addTerm(v,1.0);
  else
    lf->addTerm(v,-1.0);
  FunctionPtr f = (FunctionPtr) new Function(lf);
  consAC_.push_back(quasiACProblem_->newConstraint(f, ub, ub));
  
  updateLogBar_(v);
}

void quasiACProblem::updateObjConst(double ub)
{
#if SPEW
  logger_->msgStream(LogDebug) << me_ << " updating upper bound of objective\n";
#endif

  ConstraintPtr cons;

  cons = quasiACProblem_->getConstraint(0);
  quasiACProblem_->changeBound(cons, ub, ub);
}

void quasiACProblem::solvequasiACProblem_()
{
#if SPEW
  logger_->msgStream(LogDebug) << me_ << " solving ACP\n";
#endif

  ObjectivePtr o;
  o = quasiACProblem_->getObjective();
  //AC_Engine_->clear();
  if (!o){
    std::cout << "AC has no objective" << std::endl;
  } else {
    quasiACProblem_->removeObjective();
  }

  quasiACProblem_->newObjective(ACObjFunc_, 0.0, Maximize, "ACObj");
  

  //quasiACProblem_->calculateSize(true);  
  //JacobianPtr jPtr = (JacobianPtr) new Jacobian(consAC_, quasiACProblem_->getNumVars());
  //HessianOfLagPtr hPtr = (HessianOfLagPtr) new HessianOfLag(quasiACProblem_.get());
  //quasiACProblem_->setJacobian(jPtr);
  //quasiACProblem_->setHessian(hPtr);
  quasiACProblem_->setNativeDer();
  
  AC_Engine_->load(quasiACProblem_);
  AC_EngStatus_ = AC_Engine_->solve();
  
}
*/
/*
QGNewCut quasiACProblem::ACCPM_()
{
#if SPEW
  logger_->msgStream(LogDebug) << me_ << " passing cuts to relaxation\n";
#endif
  const double *solAC;  
  numACCPMSolve_ = 2; 
  QGNewCut qgnewcut;

  for (int i = 0; i < numACCPMSolve_; i++)
  {
    solvequasiACProblem_();
    solAC = new double[quasiACProblem_->getNumVars()];
    AC_Engine_->getSolution()->getPrimal();
    ACUpdate_(solAC);
    
  }
  //qgnewcut.coef = gr_;
  //qgnewcut.rhs = rhs_;
  for (UInt i = 0; i < rhs_.size(); i++)
  {
    qgnewcut.coef.push_back(gr_[i]);
    //qgnewcut.rhs.push_back(rhs_[i]);
  }
  qgnewcut.newUb = newObjBound; 
  return qgnewcut;
}
*/

/*
void quasiACProblem::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf)
{
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "linearizing over points\n";
#endif

  int n = ACrel_->getNumVars();
  double *a = new double[n];
  VariableConstIterator vbeg = ACrel_->varsBegin();
  VariableConstIterator vend = ACrel_->varsEnd();
  int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_); 
  *c  = fval - InnerProduct(x, a, n);

  gr_.push_back(a);
  //rhs_.push_back(*c);
  delete [] a;
}
*/


quasiSolution::quasiSolution()
  : size_(0),
    x_(0)
{
}

quasiSolution::quasiSolution(int num, const double *x)
  : size_(num)
{
  x_ = new double[num];
  for (int i = 0; i < num; ++i)
    x_[i] = x[i];
}

quasiSolution::~quasiSolution()
{
  delete [] x_;
}

double * quasiSolution::getSolution()
{
//  for (int i = 0; i < size_;++i)
//    std::cout << "x[" << i << "]=" << x_[i] << ",";
//  std::cout << "\n";
  return x_;
}

quasiSolutionPool::quasiSolutionPool()
  : size_(0)
{
}

quasiSolutionPool::quasiSolutionPool(int size)
  : size_(size)
{
}

quasiSolutionPool::~quasiSolutionPool()
{
}
void quasiSolutionPool::addSol(const double *x)
{
  quasiSolutionPtr quasiSol = (quasiSolutionPtr) new 
		quasiSolution(size_,x);

  quasiSol_.push_back(quasiSol);
}

void quasiQGHandler::writeStats(std::ostream &out) const
{

  if (partialFix_){
    out
      << "QG Handler: number of nlps solved     = " << stats_->nlpS << std::endl
      << "QG Handler: number of infeasible nlps = " << stats_->nlpI << std::endl
      << "QG Handler: number of feasible nlps   = " << stats_->nlpF << std::endl
      << "QG Handler: number of cuts added      = " << stats_->cuts << std::endl
      << "QG Handler: number of loose cuts      = " << stats_->looseCuts << std::endl
      << "QG Handler: number of additional cuts = " << stats_->adCuts << std::endl
      << "QG Handler: time of taking gradients  = " << grdTime_ << std::endl;
//      << "QG Handler: time of CutManager        = " << ctmngrTime_ << std::endl;
  } else if (!handleQuasiConvex_){
      out
        << "QG Handler: number of nlps solved     = " << stats_->nlpS << std::endl
        << "QG Handler: number of infeasible nlps = " << stats_->nlpI << std::endl
        << "QG Handler: number of feasible nlps   = " << stats_->nlpF << std::endl
        << "QG Handler: number of cuts added      = " << stats_->cuts << std::endl
        << "QG Handler: number of additional cuts = " << stats_->adCuts << std::endl
        << "QG Handler: time of taking gradients  = " << grdTime_ << std::endl;
//	<< "QG Handler: time of CutManager        = " << ctmngrTime_ << std::endl;
    } else {
      out
      << "QG Handler: number of nlps solved       = " << stats_->nlpS << std::endl
      << "QG Handler: number of projection solved = " << stats_->nlpP << std::endl
      << "QG Handler: number of infeasible nlps   = " << stats_->nlpI << std::endl
      << "QG Handler: number of feasible nlps     = " << stats_->nlpF << std::endl
      << "QG Handler: number of cuts added        = " << stats_->cuts << std::endl
      << "QG Handler: number of additional cuts	  = " << stats_->adCuts << std::endl
      << "QG Handler: time of taking gradients    = " << grdTime_ << std::endl;
//      << "QG Handler: time of CutManager          = " << ctmngrTime_ << std::endl;
    }
}


std::string quasiQGHandler::getName() const
{
  return "quasiQG Handler (quasi Quesada-Grossmann)";
}

void write_cut(double *gr, double rhs, double gap, int numvars)
{
  std::ofstream myfile;
  myfile.open ("Gradients.txt",std::ios::app);
  for (int i = 0; i < numvars; i++)
    myfile << gr[i] << ",";
  myfile << rhs << "," << gap << std::endl;

  myfile.close();
}
void write_point(const double *xo, const double *xp, int numvars)
{
  std::ofstream myfile0;
  std::ofstream myfile1;
  myfile1.open ("xpr.txt",std::ios::app);
  myfile0.open ("xOA.txt",std::ios::app);
//  if (a==1)
//  {
    for (int i = 0; i < numvars; i++)
      myfile1 << xp[i] << ",";
    myfile1 << std::endl;
//  } else if (a==0) {
    for (int i = 0; i < numvars; i++)
      myfile0 << xo[i] << ",";
    myfile0 << std::endl;
//  }
  myfile0.close();
  myfile1.close();
}

void write_point(const double *xo, const double *xp, int numvars0, int numvars1)
{
  std::ofstream myfile0;
  std::ofstream myfile1;
  myfile1.open ("xpr1.txt",std::ios::app);
  myfile0.open ("xOA1.txt",std::ios::app);
//  if (a==1)
//  {
    for (int i = 0; i < numvars1; i++)
      myfile1  << i+1 << "," << std::setprecision(10) << xp[i] << "\n";
    myfile1 << std::endl;
//  } else if (a==0) {
    for (int i = 0; i < numvars0; i++)
      myfile0 << i+1 << "," << std::setprecision(10)<< xo[i] << "\n";
    myfile0 << std::endl;
//  }
  myfile0.close();
  myfile1.close();
}
