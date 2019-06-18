// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/** 
 * \file OAHandler.cpp
 * \Briefly define a handler for Outer Approximation algorithm.
 * \Authors Meenarli Sharma and Prashant Palkar, IIT Bombay
 */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "Constraint.h"
#include "Engine.h"
#include "MILPEngine.h"
#include "Environment.h"
#include "Function.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "OAHandler.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include "QuadraticFunction.h"

//#define SPEW 0

using namespace Minotaur;
//MS: there is a tolerance move it to main()
typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string OAHandler::me_ = "OAHandler: ";

OAHandler::OAHandler()
: env_(EnvPtr()),      
  minlp_(ProblemPtr()),
  timer_(0),                    // NULL
  nlCons_(0),
  nlpe_(EnginePtr()),
  milpe_(MILPEnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  stats_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


OAHandler::OAHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe, MILPEnginePtr milpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  milpe_(milpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0)
{
  timer_ = env->getNewTimer();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_   = new OAStats();
  stats_->milpS = 0;
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->milpIL = 0;
  stats_->cuts = 0;
}


OAHandler::~OAHandler()
{ 
  if (stats_) {
    delete stats_;
  }

  if (timer_) {
    delete timer_;
  }
  //env_.reset();
  env_ = 0;
  //rel_.reset();
  //minlp_.reset();
  rel_ = 0;
  minlp_ = 0;
  if (logger_){
    delete logger_;
  }
}


void OAHandler::addInitLinearX_(const double *x)
{ 
  int error=0;
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con;
  LinearFunctionPtr lf = LinearFunctionPtr();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) { 
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error); 
      if (error == 0) {
        cUb = con->getUb(); 
        ++(stats_->cuts);
        sstm << "_OAcut_" << stats_->cuts << "_AtRoot";
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      }
    }	else {
      logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
        " is not defined at this point" << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "constraint " <<
        con->getName() << " is not defined at this point." << std::endl;
#endif
    } 
  }

  if (oNl_) {
    error = 0;  
    ObjectivePtr o = minlp_->getObjective();
    act = o->eval(x, &error);
    if (error==0) {
      ++(stats_->cuts);
      sstm << "_OAObjcut_" << stats_->cuts << "_AtRoot";
      f = o->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      }
    }	else {
      logger_->msgStream(LogError) << me_ <<
        "Objective not defined at this point" << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<
        "Objective not defined at this point" << std::endl;
#endif
    }
  }
  return;
}


void OAHandler::cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan, 
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  double nlpval = INFINITY;
  const double *lpx = sol->getPrimal(), *nlpx;
  relobj_ = (sol) ? sol->getObjValue() : -INFINITY;

  fixInts_(lpx);           // Fix integer variables
  solveNLP_();
  unfixInts_();            // Unfix integer variables
  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    updateUb_(s_pool, &nlpval, sol_found); 
    if ((relobj_ >= nlpval-objATol_) ||
        (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRTol_))) {
        *status = SepaPrune;
        break;
    } else {
      relobj_= sol->getObjValue();
      nlpx = nlpe_->getSolution()->getPrimal();
      cutToCons_(nlpx, lpx, cutMan, status);
      cutToObj_(nlpx, lpx, cutMan, status);
      break;
    }
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    nlpx = nlpe_->getSolution()->getPrimal();
    cutToCons_(nlpx, lpx, cutMan, status);
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    consCutAtLpSol_(lpx, cutMan, status);
    break;
  case (FailedFeas):
  case (EngineError):
  case (FailedInfeas):
  case (ProvenUnbounded):
  case (ProvenFailedCQFeas):
  case (EngineUnknownStatus):
  case (ProvenFailedCQInfeas):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status = " 
      << nlpe_->getStatusString() << std::endl; 
    logger_->msgStream(LogError)<< me_ << "No cut generated, may cycle!" 
      << std::endl;
    *status = SepaError;
  }

 if (*status == SepaContinue) {
   *status = SepaPrune;
 }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "NLP solve status = " 
    << nlpe_->getStatusString() << std::endl;
  logger_->msgStream(LogDebug) << me_ << "separation status = " << *status << std::endl;
#endif
  return;
}


void OAHandler::fixInts_(const double *x)
{
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd(); 
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5); 
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp_);
      nlpMods_.push(m);
    }
  }
  return;
}

void OAHandler::solveMILP(double* objfLb, ConstSolutionPtr* sol, 
                          SolutionPoolPtr, CutManager*)
{

  milpe_->load(rel_);         //remove double loading in iteration 1!
  EngineStatus lpStatus = milpe_->solve();
  ++(stats_->milpS);
  switch (lpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    *sol = milpe_->getSolution();
    (*objfLb) = (*sol)->getObjValue();
    break;
  case (EngineIterationLimit): // MS: take care of this here.
    ++(stats_->milpIL);
    *sol = milpe_->getSolution();
    (*objfLb) = (*sol)->getObjValue();
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    logger_->msgStream(LogError) << me_ << "MILP engine status at root= " 
      << lpStatus << std::endl;
    assert(!"In OAHandler: MILP infeasible. Check error log.");
    break;
  case (ProvenUnbounded):
  //case (EngineIterationLimit): // MS: take care of this.
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "MILP engine status= " 
      << lpStatus << std::endl;
    assert(!"In OAHandler: stopped. Check error log.");
    break;
  }
  return;
}

void OAHandler::initLinear_(bool *isInf)
{
  *isInf = false;
  const double *x;
  
  nlpe_->load(minlp_);
  solveNLP_();
  
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    x = nlpe_->getSolution()->getPrimal();
    addInitLinearX_(x); 
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    x = nlpe_->getSolution()->getPrimal();
    addInitLinearX_(x); 
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    *isInf = true;
    break;
  case (FailedFeas):
  case (EngineError):
  case (FailedInfeas):
  case (ProvenUnbounded):
  case (ProvenFailedCQFeas):
  case (EngineUnknownStatus):
  case (ProvenFailedCQInfeas):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status at root= " 
      << nlpStatus_ << std::endl;
    assert(!"In OAHandler: stopped at root. Check error log.");
    break;
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = " 
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}


bool OAHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                           double &)
{
  int error=0;
  FunctionPtr f;
  double act, cUb;
  ConstraintPtr c;
  const double *x = sol->getPrimal();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) { 
    c = *it;
    act = c->getActivity(x, &error);
    if (error==0) {
      cUb = c->getUb();
      if (act > cUb + solAbsTol_ && 
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "constraint " <<
          c->getName() << " violated with violation = " << act - cUb <<
          std::endl;
#endif
        return false;     
      } 
    } else {
      logger_->msgStream(LogError) << me_ << c->getName() <<
        "constraint not defined at this point."<< std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "constraint " << c->getName() <<
        " not defined at this point." << std::endl;
#endif
      return false;
    }
  }
 
  if (oNl_) {
    error = 0;
    relobj_ = x[objVar_->getIndex()];
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if (act > relobj_ + solAbsTol_ && 
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "objective violated with "
          << "violation = " << act - relobj_ << std::endl;
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ 
        <<" Objective not defined at this point"<< std::endl;
      return false;
    }
  }

  return true;
}


void OAHandler::linearizeObj_()
{
  ObjectivePtr o = minlp_->getObjective();
  FunctionType fType = o->getFunctionType();
  if (!o) {
    assert(!"need objective in QG!");
  } else if (fType != Linear && fType != Constant) {
    oNl_ = true;
    FunctionPtr f;
    std::string name = "eta";
    ObjectiveType objType = o->getObjectiveType();
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    VariablePtr vPtr = rel_->newVariable(-INFINITY, INFINITY, Continuous,
                                         name, VarHand);
    assert(objType == Minimize);
    rel_->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel_->newObjective(f, 0.0, objType);
    objVar_ = vPtr;
  } 
  return;
}


void OAHandler::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf, int *error)
{

  int n = rel_->getNumVars();
  double *a = new double[n];
  VariableConstIterator vbeg = rel_->varsBegin(), vend = rel_->varsEnd();
  const double linCoeffTol =
    env_->getOptions()->findDouble("conCoeff_tol")->getValue();

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, error);

  if (*error==0) {
    *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol); 
    *c  = fval - InnerProduct(x, a, minlp_->getNumVars());
  } else {
    logger_->msgStream(LogError) << me_ <<"gradient not defined at this point."
      << std::endl;
#if SPEW
    logger_->msgStream(LogDebug) << me_ <<"gradient not defined at this point."
      << std::endl;
#endif
  }
  delete [] a;
  return;
}


void OAHandler::cutToCons_(const double *nlpx, const double *lpx,
                             CutManager *cutman, SeparationStatus *status)
{
  int error=0;
  ConstraintPtr con;
  double nlpact, cUb;


  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if (nlpact > cUb + solAbsTol_ &&
          (cUb == 0 || nlpact > (cUb+fabs(cUb)*solRelTol_))) {
        addCut_(nlpx, lpx, con, cutman, status);
      } else {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() << " feasible at LP solution. No OA cut to be added." << std::endl;
#endif
      }
    } else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() <<
        " not defined at this point." << std::endl;
#endif
    }
  }
  return;
}
 

void OAHandler::objCutAtLpSol_(const double *lpx, CutManager *,
                                  SeparationStatus *status)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    double c, vio, act;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      vio = std::max(act-relobj_, 0.0);
      if (vio > solAbsTol_ && (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
          f = o->getFunction();
          LinearFunctionPtr lf = LinearFunctionPtr();
          linearAt_(f, act, lpx, &c, &lf, &error);
          if (error == 0) {
            ++(stats_->cuts);
            sstm << "_OAObjcut_" << stats_->cuts;
            lf->addTerm(objVar_, -1.0);
            *status = SepaResolve;
            f = (FunctionPtr) new Function(lf);
            rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
          }
        }
    }	else {
      logger_->msgStream(LogError) << me_
        << " objective not defined at this solution point." << std::endl;
    }
  }
  return;
}


void OAHandler::consCutAtLpSol_(const double *lpx, CutManager *,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  ConstraintPtr con;
  double c, nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    f = con->getFunction();
    lf = LinearFunctionPtr();
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if (nlpact > cUb + solAbsTol_ &&
          (cUb == 0 || nlpact > (cUb+fabs(cUb)*solRelTol_))) {
        linearAt_(f, nlpact, lpx, &c, &lf, &error);
        if (error==0) {
            ++(stats_->cuts);
            sstm << "_OAcut_" << stats_->cuts;
            *status = SepaResolve;
            f = (FunctionPtr) new Function(lf);
            rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
            return;
          //}
        }
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
    }
  }
  return;
}


void OAHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status)
{
  int error=0;
  std::stringstream sstm;
  double c, lpvio, act, cUb;
  FunctionPtr f = con->getFunction();
  LinearFunctionPtr lf = LinearFunctionPtr(); 

  act = con->getActivity(nlpx, &error); 
  if (error == 0) {    
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error==0) { 
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if (lpvio>solAbsTol_ && ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "linearization of constraint "
          << con->getName() << " violated at LP solution with violation = " <<
          lpvio << ". OA cut added." << std::endl;
#endif
        ++(stats_->cuts);
        sstm << "_OAcut_" << stats_->cuts;
        *status = SepaResolve;
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        return;
      }
    }
  }	else {
    logger_->msgStream(LogError) << me_ << " constraint not defined at"
      << " this point. "<<  std::endl;
#if SPEW
          logger_->msgStream(LogDebug) << me_ << " constraint " <<
            con->getName() << " not defined at this point." << std::endl;
#endif
  }
  return;
}
  

void OAHandler::cutToObj_(const double *nlpx, const double *lpx,
                            CutManager *, SeparationStatus *status)
{
  if (oNl_) {
    int error=0;
    FunctionPtr f;
    double c, vio, act;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();
    
    act = o->eval(lpx, &error);
    if (error == 0) {
      vio = std::max(act-relobj_, 0.0);
      if (vio > solAbsTol_ && (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " objective violated at LP "
          << " solution with violation = " << vio << std::endl;
#endif
        act = o->eval(nlpx, &error);
        if (error == 0) {
          f = o->getFunction();
          LinearFunctionPtr lf = LinearFunctionPtr(); 
          linearAt_(f, act, nlpx, &c, &lf, &error);
          if (error == 0) {
            vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if (vio > solAbsTol_ && ((relobj_-c)==0 
                                     || vio > fabs(relobj_-c)*solRelTol_)) { 
#if SPEW
              logger_->msgStream(LogDebug) << me_ << "linearization of "
                "objective violated at LP solution with violation = " <<
                vio << ". OA cut added." << std::endl;
#endif
              ++(stats_->cuts);
              sstm << "_OAObjcut_" << stats_->cuts;
              lf->addTerm(objVar_, -1.0);
              *status = SepaResolve;
              f = (FunctionPtr) new Function(lf);
              rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
            }
          }
        }
      }  else {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " objective feasible at LP "
          << " solution. No OA cut to be added." << std::endl;
#endif
      }
    }	else {
      logger_->msgStream(LogError) << me_
        << " objective not defined at this solution point." << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " objective not defined at this "
        << " point." << std::endl;
#endif
    }  
  }
  return;
}


void OAHandler::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void OAHandler::relax_(bool *isInf)
{
  ConstraintPtr c;
  FunctionType fType;
  
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
       ++it) {
    c = *it;
    fType = c->getFunctionType();
    if (fType !=Constant && fType != Linear) {
      nlCons_.push_back(c);
    }
  }
 
  linearizeObj_();
  initLinear_(isInf);
  return;
}


void OAHandler::separate(ConstSolutionPtr sol, NodePtr, RelaxationPtr,
                         CutManager *cutMan, SolutionPoolPtr s_pool,
                         ModVector &, ModVector &, bool *sol_found,
                         SeparationStatus *status)
{      
  //double val;
  //VariableType v_type;
  //VariableConstIterator v_iter;
  //const double *x = sol->getPrimal();

  //*status = SepaContinue;
  //for (v_iter = rel->varsBegin(); v_iter != rel->varsEnd(); ++v_iter) {
    //v_type = (*v_iter)->getType();
    //if (v_type == Binary || v_type == Integer) {
      //val = x[(*v_iter)->getIndex()];
      //if (fabs(val - floor(val+0.5)) > intTol_) {
//#if SPEW
        //logger_->msgStream(LogDebug) << me_ << "variable " <<
          //(*v_iter)->getName() << " has fractional value = " << val <<
          //std::endl;
//#endif
        //return;
      //}
    //}
  //}

  cutIntSol_(sol, cutMan, s_pool, sol_found, status);
  return;
}


void OAHandler::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  return;
}


void OAHandler::unfixInts_()
{
  Modification *m = 0;
  while(nlpMods_.empty() == false) {
    m = nlpMods_.top();
    m->undoToProblem(minlp_);
    nlpMods_.pop();
    delete m;
  }
  return;
}


void OAHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval, 
                          bool *sol_found)
{
  //MS: solution is added to the pool only if better than incumbent
  double val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();

  if ((bestval - objATol_ > val) ||
        (bestval != 0 && (bestval - fabs(bestval)*objRTol_) > val)) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, val);
    *sol_found = true;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "new solution found, value = " 
      << val << std::endl;
#endif
  }
  //objVar_->setUb_(bestval-objATol_);  
  *nlpval = val;
  return;
}

void OAHandler::writeStats(std::ostream &out) const
{
  out
    << me_ << "number of nlps solved                          = " 
    << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps                      = " 
    << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps                        = " 
    << stats_->nlpF << std::endl
    << me_ << "number of nlps hit engine iterations limit     = " 
    << stats_->nlpIL << std::endl
    << me_ << "number of milps solved                         = " 
    << stats_->milpS << std::endl
    << me_ << "number of milps hit engine iterations limit    = " 
    << stats_->milpIL << std::endl
    << me_ << "number of cuts added                           = " 
    << stats_->cuts << std::endl;
  return;
}


std::string OAHandler::getName() const
{
  return "OA Handler (Outer-approximation)";
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
