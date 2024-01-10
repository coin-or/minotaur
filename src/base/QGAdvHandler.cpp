//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file QGAdvHandler.cpp
 * \Briefly define a handler for the textbook type Quesada-Grossmann
 * Algorithm.
 * \Authors Meenarli Sharma, Indian Institute of Technology Bombay
 */
/////MS: lots of changes for tolerances and Impl bin
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
#include "Environment.h"
#include "Function.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "QGAdvHandler.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string QGAdvHandler::me_ = "QGAdvHandler: ";


QGAdvHandler::QGAdvHandler()
: env_(EnvPtr()),
  minlp_(ProblemPtr()),
  nlCons_(0),
  nlpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  stats_(0),
  prStatus_(0),
  pcPtr_(PerspCutHandlerPtr())
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  npATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  npRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger(LogInfo);
}


QGAdvHandler::QGAdvHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  prStatus_(0),
  pcPtr_(PerspCutHandlerPtr())
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  npATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  npRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->cuts = 0;
}
 

QGAdvHandler::QGAdvHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe,
                     PerspCutHandlerPtr pcPtr)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  prStatus_(0),
  pcPtr_(pcPtr)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();
  stats_ = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->cuts = 0;
}


QGAdvHandler::~QGAdvHandler()
{
  if (stats_) {
    delete stats_;
  }
  nlCons_.clear();
}


void QGAdvHandler::addInitLinearX_(const double *x)
{
  int error = 0;
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con, newcon;
  LinearFunctionPtr lf = LinearFunctionPtr();

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
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
        newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint" <<  con->getName() <<
        " is not defined at this point." << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " <<
        con->getName() << " is not defined at this point." << std::endl;
#endif
    }
  }

  if (prStatus_) {
    pcPtr_->oriFeasPt(rel_, x);
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
        newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      }
    }	else {
      logger_->msgStream(LogError) << me_ <<
        "Objective not defined at this point." << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<
        "Objective not defined at this point." << std::endl;
#endif
    }
  }
  return;
}


void QGAdvHandler::cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  SeparationStatus pcStatus;
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
    if ((relobj_ >= nlpval-npATol_) ||
        (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*npRTol_))) {
        *status = SepaPrune;
        break;
    } else {
      nlpx = nlpe_->getSolution()->getPrimal();
      oaCutToCons_(nlpx, lpx, cutMan, status);
      oaCutToObj_(nlpx, lpx, cutMan, status);
      if (prStatus_) {
        pcPtr_->atIntPt(rel_, nlpx, lpx, &pcStatus, cutMan);
      }
      break;
    }
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible):
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    nlpx = nlpe_->getSolution()->getPrimal();
    oaCutToCons_(nlpx, lpx, cutMan, status);
    if (prStatus_) {
      pcPtr_->atIntPt(rel_, nlpx, lpx, &pcStatus, cutMan);
    }
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    nlpx = nlpe_->getSolution()->getPrimal();
    oaCutEngLim_(lpx, cutMan, status); //MS: how about taking a convex combination here?
    if (prStatus_) {
      //MS: add here to PR for a point not two points
      //pcPtr_->pcutAtInt(rel_, nlpx, lpx, &pcStatus, cutMan);
    }
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
   
  if (pcStatus == SepaResolve) {
    *status = SepaResolve; //MS: error in PRCutHandler
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << " NLP solve status = "
    << nlpe_->getStatusString() << " and separation status = " << *status <<
    std::endl;
#endif
  return;
}


void QGAdvHandler::oaCutEngLim_(const double *lpx, CutManager *,
                             SeparationStatus *status)
{
  int error = 0;
  FunctionPtr f;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  ConstraintPtr con, newcon;
  double c, lpvio, nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    f = con->getFunction();
    lf = LinearFunctionPtr();
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if (nlpact > cUb + solAbsTol_ ||
          (cUb != 0 && nlpact > (cUb+fabs(cUb)*solRelTol_))) {
        linearAt_(f, nlpact, lpx, &c, &lf, &error);
        if (error==0) {
          lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
          if (lpvio>solAbsTol_ || ((cUb-c)!=0 &&
                                   (lpvio>fabs(cUb-c)*solRelTol_))) {
            ++(stats_->cuts);            
            sstm << "_OAcut_" << stats_->cuts;
            *status = SepaResolve;
            f = (FunctionPtr) new Function(lf);
            newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
            return;
          }
        }
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
    }
  }
  return;
}


void QGAdvHandler::fixInts_(const double *x)
{
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd();
       ++vit) {
    v = *vit;
    if (v->getType() == Binary || v->getType() == Integer ||
        v->getType() == ImplBin || v->getType() == ImplInt) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5);
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp_);
      nlpMods_.push(m);
    }
  }
  return;
}


void QGAdvHandler::initLinear_(bool *isInf)
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
    assert(!"In QGAdvHandler: stopped at root. Check error log.");
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = "
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}


bool QGAdvHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                           double &)
{
  int error = 0;
  FunctionPtr f;
  double act, cUb;
  ConstraintPtr con;
  const double *x = sol->getPrimal();

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      cUb = con->getUb();
      if (act > cUb + solAbsTol_ ||
          (cUb != 0 && act > cUb + fabs(cUb)*solRelTol_)) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " constraint " <<
          con->getName() << " violated with violation = " << act - cUb <<
          std::endl;
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ << con->getName() <<
        " constraint not defined at this point."<< std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName()
        << " not defined at this point." << std::endl;
#endif
      return false;
    }
  }

  if (oNl_) {
    error = 0;
    relobj_ = x[objVar_->getIndex()];
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if (act > relobj_ + solAbsTol_ ||
          (relobj_ != 0 && (act > relobj_ + fabs(relobj_)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " objective violated with "
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

  if (prStatus_) {
    double inf_meas = 0.0;
    bool should_prune = false;
    return pcPtr_->isFeasible(sol, rel_, should_prune, inf_meas);
  }
  return true;
}


void QGAdvHandler::linearizeObj_()
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


void QGAdvHandler::linearAt_(FunctionPtr f, double fval, const double *x,
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


void QGAdvHandler::oaCutToCons_(const double *nlpx, const double *lpx,
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
      if (nlpact > cUb + solAbsTol_ ||
          (cUb != 0 && nlpact > (cUb+fabs(cUb)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " constraint " <<
          con->getName() << " violated at LP solution with violation = " <<
          nlpact - cUb << std::endl;
#endif
        addCut_(nlpx, lpx, con, cutman, status);
      } else {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() <<
          " feasible at LP solution. No OA cut to be added." << std::endl;
#endif
      }
    }	else {
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


void QGAdvHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status)
{
  int error = 0;
  ConstraintPtr newcon;
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
      if (lpvio>solAbsTol_ || ((cUb-c)!=0 && (lpvio>fabs(cUb-c)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "i linearization of constraint "
          << con->getName() << " violated at LP solution with violation = " <<
          lpvio << ". OA cut added." << std::endl;
#endif
        ++(stats_->cuts);
        sstm << "_OAcut_" << stats_->cuts;
        *status = SepaResolve;
        f = (FunctionPtr) new Function(lf);
        newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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


void QGAdvHandler::oaCutToObj_(const double *nlpx, const double *lpx,
                            CutManager *, SeparationStatus *status)
{
  if (oNl_) {
    int error=0;
    FunctionPtr f;
    double c, vio, act;
    ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();
    
    act = o->eval(lpx, &error);
    if (error == 0) {
      vio = std::max(act-relobj_, 0.0);
      if (vio > solAbsTol_ || (relobj_ != 0 && vio > fabs(relobj_)*solRelTol_)) {
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
            if (vio > solAbsTol_ || ((relobj_-c)!=0
                                     && vio > fabs(relobj_-c)*solRelTol_)) {
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
              newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
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


void QGAdvHandler::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}


void QGAdvHandler::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void QGAdvHandler::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{

  //Does nothing
}


void QGAdvHandler::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void QGAdvHandler::relax_(bool *isInf)
{
  ConstraintPtr c;
  FunctionType fType;
  
  if (pcPtr_ != NULL && pcPtr_->getStatus() == true) {
    prStatus_ = true;
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "is perspective reformulation "
    << "amenable = " << prStatus_ << std::endl;
#endif

  if (!prStatus_) {
    for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd();
         ++it) {
      c = *it;
      fType = c->getFunctionType();
      if (fType != Constant && fType != Linear) {
        nlCons_.push_back(c);
      }
    }
  } else {
    std::vector<UInt> prConsId;
    std::vector<ConstraintPtr> prCons = pcPtr_->getPRCons();
    for (UInt i = 0; i < prCons.size(); ++i) {
      prConsId.push_back(prCons[i]->getId());
    }
    for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd();
         ++it) {
      c = *it;
      fType = c->getFunctionType();
      if (fType != Constant && fType != Linear) {
        if (std::find(prConsId.begin(), prConsId.end(),
                      c->getId()) == prConsId.end()) {
          nlCons_.push_back(c);
        }
      }
    }
  }

  linearizeObj_();
  initLinear_(isInf);
  return;
}


void QGAdvHandler::separate(ConstSolutionPtr sol, NodePtr , RelaxationPtr rel,
                         CutManager *cutMan, SolutionPoolPtr s_pool,
                         ModVector &, ModVector &, bool *sol_found,
                         SeparationStatus *status)
{
  double val;
  VariableType v_type;
  VariableConstIterator v_iter;
  const double *x = sol->getPrimal();

  for (v_iter = rel->varsBegin(); v_iter != rel->varsEnd(); ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type == Binary || v_type == Integer || v_type == ImplBin ||
        v_type == ImplInt) {
      val = x[(*v_iter)->getIndex()];
      if (fabs(val - floor(val+0.5)) > intTol_) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << " variable " <<
          (*v_iter)->getName() << " has fractional value = " << val <<
          std::endl;
#endif
        return;
      }
    }
  }

  if (nlCons_.size() == 0) {
    *status = SepaPrune; //ms: WHAT  ABOUT NONLINEAR OBJECTIVE
  } else {
    *status = SepaContinue;
  }
    
  cutIntSol_(sol, cutMan, s_pool, sol_found, status);
  return;
}


void QGAdvHandler::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  return;
}


void QGAdvHandler::unfixInts_()
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


void QGAdvHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval,
                          bool *sol_found)
{
  double val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();

  if (val <= bestval) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, val);
    *sol_found = true;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "new solution found, value = "
      << val << std::endl;
#endif
  }
  *nlpval = val;
  return;
}


void QGAdvHandler::writeStats(std::ostream &out) const
{
  out
    << me_ << "number of nlps solved                          = "
    << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps                      = " 
    << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps                        = " 
    << stats_->nlpF << std::endl
    << me_ << "number of nlps hit engine iterations limit     = " 
    << stats_->nlpF << std::endl
    << me_ << "number of cuts added                           = " 
    << stats_->cuts << std::endl;
  return;
}


std::string QGAdvHandler::getName() const
{
  return "QGAdvHandler (Quesada-Grossmann)";
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
