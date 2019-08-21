// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/** 
 * \file QGHandler.cpp
 * \Briefly define a handler for the textbook type Quesada-Grossmann
 * Algorithm.
 * \Authors Ashutosh Mahajan and Meenarli Sharma, Indian Institute of
 * Technology Bombay
 */


#include <cmath>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "Linearizations.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "QGHandler.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string QGHandler::me_ = "QGHandler: ";


QGHandler::QGHandler()
: env_(EnvPtr()),
  minlp_(ProblemPtr()),
  nlCons_(0),
  nlpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  //newCon_(ConstraintPtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  extraLin_(0),
  stats_(0)
{
  rs1_ = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  rs2Per_ = env_->getOptions()->findDouble("root_linScheme2_per")->getValue();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
}


QGHandler::QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  //newCon_(ConstraintPtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  extraLin_(LinearizationsPtr())
{
  //MS: better option names
  rs1_ = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  rs2Per_ = env_->getOptions()->findDouble("root_linScheme2_per")->getValue();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->cuts = 0;
}


QGHandler::~QGHandler()
{ 
  if (stats_) {
    delete stats_;
  }
  nlCons_.clear();

  if (extraLin_) {
    delete extraLin_;  
  }
  env_ = 0;
  rel_ = 0;
  minlp_ = 0;
  extraLin_ = 0;  
  nlCons_.clear();
}


void QGHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status, double act)
{
  int error=0;
  double c, lpvio, cUb;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  FunctionPtr f = con->getFunction();
  LinearFunctionPtr lf = LinearFunctionPtr();

  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error==0) {
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if ((lpvio > solAbsTol_) &&
          ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " linearization of constraint "
          << con->getName() << " violated at LP solution with violation = " <<
          lpvio << std::endl;
#endif
        *status = SepaResolve;
      }
      ++(stats_->cuts);
      sstm << "_OAcut_" << stats_->cuts;
      f = (FunctionPtr) new Function(lf);
      rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      return;
    }
  }	else {
    logger_->msgStream(LogError) << me_ << " constraint is not defined at"
      << " this point. "<<  std::endl;
#if SPEW
          logger_->msgStream(LogDebug) << me_ << " constraint " <<
            con->getName() << " is not defined at this point." << std::endl;
#endif
  }
  return;
}


void QGHandler::addInitLinearX_(const double *x)
{ 
  int error = 0;
  FunctionPtr f;
  double c, act, cUb;
  ConstraintPtr con;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        cUb = con->getUb();
        ++(stats_->cuts);
        sstm << "_OACut_" << stats_->cuts << "_AtRoot";
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon->write(std::cout);
        sstm.str("");
      }
    }	else {
      logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
        " is not defined at this point." << std::endl;
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
      sstm << "_OAObjCut_" << stats_->cuts << "_AtRoot";
      f = o->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        //newcon->write(std::cout);
      }
    }	else {
      logger_->msgStream(LogError) << me_ <<
        "Objective is not defined at this point." << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<
        "Objective is not defined at this point." << std::endl;
#endif
    }
  }
  return;
}


void QGHandler::cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan,
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
    nlpx = nlpe_->getSolution()->getPrimal();
    cutToCons_(nlpx, lpx, cutMan, status);
    cutToObj_(nlpx, lpx, cutMan, status);
    if ((relobj_ >= nlpval-objATol_) ||
        (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRTol_))) {
      //if (oNl_) {
        //objVar_->setUb_(nlpval-objATol_);
      //}
      //else {
        //newCon_->setUB(nlpval-objATol_);      
      //}
      *status = SepaPrune;
    }
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    nlpx = nlpe_->getSolution()->getPrimal();
    cutToConsInf_(nlpx, lpx, cutMan, status);
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    cutsAtLpSol_(lpx, cutMan, status);
    objCutAtLpSol_(lpx, cutMan, status);
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

  /* This happens when the NLP solution is also feasible to original problem 
   * but there is some tolerance differences
  */
  if (*status == SepaContinue) {
    *status = SepaPrune;
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "NLP solve status = "
    << nlpe_->getStatusString() << " and separation status = " << *status <<
    std::endl;
#endif
  return;
}


void QGHandler::cutsAtLpSol_(const double *lpx, CutManager *,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  ConstraintPtr con;
  double c, act, cUb;
  LinearFunctionPtr lf;
  //ConstraintPtr newcon;
  std::stringstream sstm;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    f = con->getFunction();
    lf = LinearFunctionPtr();
    act =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb+fabs(cUb)*solRelTol_)) {
        linearAt_(f, act, lpx, &c, &lf, &error);
        if (error == 0) {
          ++(stats_->cuts);
          *status = SepaResolve;
          sstm << "_OACut_" << stats_->cuts;
          f = (FunctionPtr) new Function(lf);
          rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          return;
        }
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint is not defined at" <<
        " this point. "<<  std::endl;
    }
  }

  if (oNl_) {
    error = 0;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      if ((act > relobj_ + solAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
          FunctionPtr f1 = o->getFunction();
          LinearFunctionPtr lf1 = LinearFunctionPtr();
          linearAt_(f1, act, lpx, &c, &lf1, &error);
          if (error == 0) {
            ++(stats_->cuts);
            *status = SepaResolve;
            sstm << "_OAObjCut_" << stats_->cuts;
            f1 = (FunctionPtr) new Function(lf1);
            rel_->newConstraint(f1, -INFINITY, cUb-c, sstm.str());
            //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          }
      }
    }	else {
      logger_->msgStream(LogError) << me_
        << " objective is not defined at this solution point." << std::endl;
    }
  }
  return;
}


void QGHandler::cutToCons_(const double *nlpx, const double *lpx, CutManager *cutman,
                           SeparationStatus *status)
{
  int error=0;
  ConstraintPtr con;
  double nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    nlpact =  con->getActivity(nlpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((fabs(nlpact-cUb) <= solAbsTol_) ||
        (cUb != 0 && (fabs(nlpact-cUb) <= fabs(cUb)*solRelTol_))) {
        addCut_(nlpx, lpx, con, cutman, status, nlpact);
      } else {
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() 
        << " is inactive at NLP solution. No OA cut to be added." 
        << std::endl;
#endif
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint is not defined at" <<
        " this point. "<<  std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() <<
        " is not defined at this point." << std::endl;
#endif
    }
  }
  return;
}


void QGHandler::cutToConsInf_(const double *nlpx, const double *lpx, CutManager *cutman,
                              SeparationStatus *status)
{
  int error=0;
  ConstraintPtr con;
  double nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    nlpact =  con->getActivity(nlpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((nlpact >= cUb - solAbsTol_) &&
        (cUb == 0 || (nlpact >= cUb - fabs(cUb)*solRelTol_))) {
        addCut_(nlpx, lpx, con, cutman, status, nlpact);
      } else {
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName()
        << " is inactive at NLP solution. No OA cut to be added." 
        << std::endl;
#endif
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint is not defined at" <<
        " this point. "<<  std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() <<
        " is not defined at this point." << std::endl;
#endif
    }
  }
  return;
}
  

void QGHandler::cutToObj_(const double *nlpx, const double *lpx,
                            CutManager *, SeparationStatus *status)
{
  if (oNl_) {
    int error=0;
    FunctionPtr f;
    double c, vio, act;
    //ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();
    
    act = o->eval(nlpx, &error);
    if (error == 0) {
      f = o->getFunction();
      LinearFunctionPtr lf = LinearFunctionPtr(); 
      linearAt_(f, act, nlpx, &c, &lf, &error);
      if (error == 0) {
        vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
        if ((vio > solAbsTol_) && ((relobj_-c) == 0
                                 || vio > fabs(relobj_-c)*solRelTol_)) {
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "linearization of "
            "objective violated at LP solution with violation = " <<
            vio << ". OA cut added." << std::endl;
#endif
          *status = SepaResolve;
        }
        ++(stats_->cuts);
        sstm << "_OAObjcut_" << stats_->cuts;
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
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



void QGHandler::fixInts_(const double *x)
{
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd();
       ++vit) {
    v = *vit;
    if (v->getType() == Binary || v->getType() == Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5);
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp_);
      nlpMods_.push(m);
    }
  }
  return;
}


void QGHandler::initLinear_(bool *isInf)
{
  *isInf = false;
  const double *x;
  
  solveNLP_();
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    x = nlpe_->getSolution()->getPrimal();
    //solNLP_ = nlpe_->getSolution()->getPrimal();
    //objNLP_ = nlpe_->getSolution()->getObjValue();
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
    logger_->msgStream(LogError) << me_ << "NLP engine status at root = "
      << nlpStatus_ << std::endl;
    assert(!"In QGHandler: stopped at root. Check error log.");
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = " 
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}


bool QGHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                           double &)
{
  int error=0;
  double act, cUb;
  ConstraintPtr c;
  const double *x = sol->getPrimal();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    c = *it;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "constraint " <<
          c->getName() << " violated with violation = " << act - cUb <<
          std::endl;
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ << c->getName() <<
        " constraint is not defined at this point."<< std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "constraint " << c->getName() <<
        " is not defined at this point." << std::endl;
#endif
      return false;
    }
  }

  if (oNl_) {
    error = 0;
    relobj_ = x[objVar_->getIndex()];
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if ((act > relobj_ + solAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "objective is violated with "
          << "violation = " << act - relobj_ << std::endl;
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_
        <<"objective not defined at this point."<< std::endl;
      return false;
    }
  }
  return true;
}


void QGHandler::linearAt_(FunctionPtr f, double fval, const double *x,
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
    logger_->msgStream(LogError) << me_ 
      << "gradient is not defined at this point." << std::endl;
#if SPEW
    logger_->msgStream(LogDebug) << me_ 
      << " gradient is not defined at this point." << std::endl;
#endif
  }
  delete [] a;
  return;
}


void QGHandler::linearizeObj_()
{
  std::stringstream sstm;
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
  // Objective bound constraint
  //else {
    //double bound = 0;
    //LinearFunctionPtr lf = o->getLinearFunction();
    //if (lf) {
      //LinearFunctionPtr lfnew = (LinearFunctionPtr) new LinearFunction();
      //for (VariableGroupConstIterator lit=lf->termsBegin();
           //lit!=lf->termsEnd(); ++lit) {
        //lfnew->addTerm(rel_->getVariable(lit->first->getIndex()), lit->second);
        //if (lit->second < 0) {
          //bound = bound + lit->first->getLb();        
        //} else {
          //bound = bound + lit->first->getUb();        
        //}
      //}
      //++(stats_->cuts);
      //sstm << "_ObjBound_" << stats_->cuts;
      //FunctionPtr fnewc = (FunctionPtr) new Function(lfnew);
      //newCon_ = rel_->newConstraint(fnewc, -INFINITY, 
                                   //bound-o->getConstant(), sstm.str());
    //}
  //}
  return;
}


void QGHandler::objCutAtLpSol_(const double *lpx, CutManager *,
                                  SeparationStatus *status)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    double c, vio, act;
    //ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      vio = std::max(act-relobj_, 0.0);
      if ((vio > solAbsTol_) &&
          (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
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
            //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
          }
        }
    }	else {
      logger_->msgStream(LogError) << me_
        << " objective not defined at this solution point." << std::endl;
    }
  }
  return;
}

void QGHandler::relax_(bool *isInf)
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
  nlpe_->load(minlp_); // loading original problem to NLP engine
  initLinear_(isInf);

  if (*isInf == false && (rs1_ + rs2Per_ + rs3_ > 0)) {
    extraLin_ = new Linearizations(env_, nlpe_, rel_, minlp_);
    extraLin_->rootLinearizations(nlCons_, nlpe_->getSolution()->getPrimal());
  }
  return;
}


void QGHandler::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandler::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void QGHandler::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandler::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                         CutManager *cutMan, SolutionPoolPtr s_pool,
                         ModVector &, ModVector &, bool *sol_found,
                         SeparationStatus *status)
{
  double val;
  VariableType v_type;
  VariableConstIterator v_iter;
  const double *x = sol->getPrimal();

  *status = SepaContinue;
  if (node->getId() == 0 && rs3_ > 0) {
    UInt cutsCount = stats_->cuts;
    extraLin_->rootLinScheme3(sol, cutMan, s_pool, sol_found, status);
    if (*status != SepaPrune && *status != SepaError) {
      *status = SepaResolve;
    }
    rs3_ = 0;
    std::cout << "No. of root lins added (Scheme 3): " 
      << stats_->cuts - cutsCount << std::endl;
    return;
  }
  
  for (v_iter = rel->varsBegin(); v_iter != rel->varsEnd(); ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type == Binary || v_type == Integer) {
      val = x[(*v_iter)->getIndex()];
      if (fabs(val - floor(val+0.5)) > intTol_) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "variable " <<
          (*v_iter)->getName() << " has fractional value = " << val <<
          std::endl;
#endif
        return;
      }
    }
  }

  //std::cout << "Integer feasible at node" << node->getId() << std::endl;
  cutIntSol_(sol, cutMan, s_pool, sol_found, status);
  return;
}


void QGHandler::setLpEngine(EnginePtr lpe)
{
  if (rs3_ > 0) {
    extraLin_->setLpEngine(lpe);
  }
}


void QGHandler::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  return;
}


void QGHandler::unfixInts_()
{
  Modification *m = 0;
  while (nlpMods_.empty() == false) {
    m = nlpMods_.top();
    m->undoToProblem(minlp_);
    nlpMods_.pop();
    delete m;
  }
  return;
}


void QGHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval,
                          bool *sol_found)
{
  double val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();

  if ((bestval - objATol_ > val) ||
        (bestval != 0 && (bestval - fabs(bestval)*objRTol_) > val)) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, val);
    *sol_found = true;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "Better solution found, value = "
      << val << std::endl;
#endif
  }
  *nlpval = val;
  return;
}


void QGHandler::writeStats(std::ostream &out) const
{
  out
    << me_ << "number of nlps solved                       = "
    << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps                   = " 
    << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps                     = " 
    << stats_->nlpF << std::endl
    << me_ << "number of nlps hit engine iterations limit  = " 
    << stats_->nlpIL << std::endl
    << me_ << "number of cuts added                        = " 
    << stats_->cuts << std::endl;
  return;
}


std::string QGHandler::getName() const
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
