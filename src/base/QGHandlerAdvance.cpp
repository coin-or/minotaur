//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file QGHandlerAdvance.cpp
 * \Briefly define a handler to enhance  Quesada-Grossmann
 * Algorithm by using schemes for adding extra linearization at different
 * node of the branch-and-bound tree
 * \Author Meenarli Sharma, Indian Institute of Technology Bombay
 */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MinotaurConfig.h"

#include "Branch.h"
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
#include "QGHandlerAdvance.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string QGHandlerAdvance::me_ = "QGHandlerAdvance: ";

QGHandlerAdvance::QGHandlerAdvance(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  lpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  extraLin_(0),
  lastNodeId_(0),
  lastNodeIdLbUb_(0),
  maxVioVal_(0),
  maxVioPer_(0),
  lastLb_(-INFINITY),
  resolve_(1),
  solC_(NULL)
{
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
  stats_->qgCuts = 0;
  stats_->fracCuts = 0;
}


QGHandlerAdvance::~QGHandlerAdvance()
{ 
  if (stats_) {
    delete stats_;
  }

  nlCons_.clear();
  undoMods_();
 
  if (extraLin_) {
    delete extraLin_;  
  }
  env_ = 0;
  lpe_ = 0;
  rel_ = 0;
  nlpe_ = 0;
  minlp_ = 0;
  extraLin_ = 0;  
  nlCons_.clear();
}


void QGHandlerAdvance::addInitLinearX_(const double *x)
{ 
  int error = 0;
  FunctionPtr f;
  ConstraintPtr con;
  double c, act, cUb;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  //ConstraintPtr newcon;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        cUb = con->getUb();
        ++(stats_->qgCuts);
        sstm << "_qgCutRoot_" << stats_->qgCuts;
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      }
    }	else {
      logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
        " is not defined at this point." << std::endl;
    }
  }

  if (oNl_) {
    error = 0;
    ObjectivePtr o = minlp_->getObjective();
    act = o->eval(x, &error);
    if (error==0) {
      ++(stats_->qgCuts);
      f = o->getFunction();
      sstm << "_qgObjCutRoot_" << stats_->qgCuts;
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      }
    }	else {
      logger_->msgStream(LogError) << me_ <<
        "Objective not defined at this point." << std::endl;
    }
  }
  return;
}


void QGHandlerAdvance::cutAtNlpSol_(CutManager *cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      ++(stats_->nlpF);
      const double * nlpx = nlpe_->getSolution()->getPrimal();
      cutToConsAtFrac_(nlpx, cutMan, status);
      //cutToObjAtFrac_(nlpx, cutMan, status);
      if (isIntFeas_(nlpx)) { 
        double nlpval = nlpe_->getSolutionValue();
        updateUb_(s_pool, nlpval, sol_found);
        if ((relobj_ >= nlpval-objATol_) ||
            (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRTol_))) {
          *status = SepaPrune;
        }
      }
    }
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    {  
      ++(stats_->nlpI);
      const double * nlpx = nlpe_->getSolution()->getPrimal();
      cutToConsAtFrac_(nlpx, cutMan, status);
      break;
    }
  case (EngineIterationLimit):
    {
      ++(stats_->nlpIL);
      const double * nlpx = nlpe_->getSolution()->getPrimal();
      consCutsAtLpSol_(nlpx, cutMan, status);
      //objCutAtLpSol_(nlpx, cutMan, status);
      *status = SepaContinue;
      break;
    }
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
  }
  return;
}


void QGHandlerAdvance::cutIntSol_(const double *lpx, CutManager *cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      ++(stats_->nlpF);
      double nlpval = nlpe_->getSolutionValue();
      updateUb_(s_pool, nlpval, sol_found);
      if ((relobj_ >= nlpval-objATol_) ||
          (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRTol_))) {
          *status = SepaPrune;
      } else {
        const double * nlpx = nlpe_->getSolution()->getPrimal();
        cutToObj_(nlpx, lpx, cutMan, status);
        cutToCons_(nlpx, lpx, cutMan, status);
      }
    }
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    {
      ++(stats_->nlpI);
      const double * nlpx = nlpe_->getSolution()->getPrimal();
      cutToCons_(nlpx, lpx, cutMan, status);
    }
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    objCutAtLpSol_(lpx, cutMan, status);
    consCutsAtLpSol_(lpx, cutMan, status);
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
    break;
  }

  // happens due to tolerance miss-match among different solvers
  if (*status == SepaContinue) {
    *status = SepaPrune;
  }
  return;
}


void QGHandlerAdvance::fixSomeInts_(const double *x)
{
  // Fix those integer variables that are integer feasible
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd();
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = x[v->getIndex()];
      if (fabs(xval - floor(xval+0.5)) < intTol_) {
        xval = floor(xval + 0.5);
        m = new VarBoundMod2(v, xval, xval);
        m->applyToProblem(minlp_);
        nlpMods_.push(m);
      }
    }
  }
  return;
}


void QGHandlerAdvance::fixInts_(const double *x)
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


void QGHandlerAdvance::initLinear_(bool *isInf)
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
    assert(!"In QGHandlerAdvance: stopped at root. Check error log.");
    break;
  }
  return;
}


bool QGHandlerAdvance::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                           double &)
{
  int error = 0;
  double act, cUb;
  ConstraintPtr c;
  const double *x = sol->getPrimal();

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    c = *it;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ << c->getName() <<
        " constraint not defined at this point."<< std::endl;
      return false;
    }
  }

  if (oNl_) {
    error = 0;
    relobj_ = sol->getObjValue();
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if ((act > relobj_ + solAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
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


void QGHandlerAdvance::linearizeObj_()
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
    
    objVar_ = vPtr;
    rel_->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel_->newObjective(f, 0.0, objType);
  }
  return;
}


void QGHandlerAdvance::linearAt_(FunctionPtr f, double fval, const double *x,
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
  }
  delete [] a;
  return;
}


void QGHandlerAdvance::cutToCons_(const double *nlpx, const double *lpx,
                             CutManager *cutman, SeparationStatus *status)
{
  int error = 0;
  ConstraintPtr con;
  double nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((nlpact > cUb + solAbsTol_) &&
          (cUb == 0 || nlpact > cUb+fabs(cUb)*solRelTol_)) {
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
    }
  }
  return;
}


void QGHandlerAdvance::consCutsAtLpSol_(const double *lpx, CutManager *cutMan,
                             SeparationStatus *status)
{
  UInt temp = stats_->qgCuts;
  for ( UInt i = 0; i < nlCons_.size(); ++i) {
    ECPTypeCut_(lpx, cutMan, i);
  }
  if (temp < stats_->qgCuts) {
    *status = SepaResolve;
  }
}


void QGHandlerAdvance::objCutAtLpSol_(const double *lpx, CutManager *,
                                  SeparationStatus *status)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    LinearFunctionPtr lf;
    double c, lpvio, act;
    //ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      lpvio = std::max(act-relobj_, 0.0);
      if ((lpvio > solAbsTol_) &&
          (relobj_ == 0 || lpvio > fabs(relobj_)*solRelTol_)) {
          lf = 0;
          f = o->getFunction();
          linearAt_(f, act, lpx, &c, &lf, &error);
          if (error == 0) {
            lpvio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((lpvio > solAbsTol_) && ((relobj_-c) == 0
                                       || lpvio > fabs(relobj_-c)*solRelTol_)) {
              ++(stats_->qgCuts);
              *status = SepaResolve;
              lf->addTerm(objVar_, -1.0);
              f = (FunctionPtr) new Function(lf);
              sstm << "_qgObjCut_" << stats_->qgCuts;
              rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
            } else {
              delete lf;
              lf = 0;
            }
          }
        }
    }	else {
      logger_->msgStream(LogError) << me_
        << " objective not defined at this solution point." << std::endl;
    }
  }
  return;
}


void QGHandlerAdvance::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status)
{
  int error = 0;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  double c, lpvio, act, cUb;
  FunctionPtr f = con->getFunction();

  act = con->getActivity(nlpx, &error);

  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error == 0) {
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if ((lpvio > solAbsTol_) &&
          ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
        ++(stats_->qgCuts);
        sstm << "_qgCut_" << stats_->qgCuts;
        *status = SepaResolve;
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        return;
      } else {
        delete lf;
        lf = 0;
      }
    }
  }	else {
    logger_->msgStream(LogError) << me_ << " constraint not defined at"
      << " this point. "<<  std::endl;
  }
  return;
}
  

void QGHandlerAdvance::cutToObj_(const double *nlpx, const double *lpx,
                            CutManager *, SeparationStatus *status)
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
      if ((vio > solAbsTol_)
        && (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
        act = o->eval(nlpx, &error);
        if (error == 0) {
          f = o->getFunction();
          LinearFunctionPtr lf = 0; 
          linearAt_(f, act, nlpx, &c, &lf, &error);
          if (error == 0) {
            vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((vio > solAbsTol_) && ((relobj_-c) == 0
                                     || vio > fabs(relobj_-c)*solRelTol_)) {
              ++(stats_->qgCuts);
              sstm << "_qgObjCut_" << stats_->qgCuts;
              lf->addTerm(objVar_, -1.0);
              *status = SepaResolve;
              f = (FunctionPtr) new Function(lf);
              //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
            } else {
              delete lf;
              lf = 0;
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
    }
  }
  return;
}


void QGHandlerAdvance::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandlerAdvance::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void QGHandlerAdvance::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandlerAdvance::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandlerAdvance::relax_(bool *isInf)
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

  // user input for root linearization schemes
  // //MS: names of the schemes
  maxVioPer_ = env_->getOptions()->findDouble("maxVioPer")->getValue();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  //int rs3 = env_->getOptions()->findInt("root_linScheme3")->getValue();
  bool rg1 = env_->getOptions()->findBool("root_genLinScheme1")->getValue();
  bool rg2 = env_->getOptions()->findBool("root_genLinScheme2")->getValue();
  double rs1 = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  double rs2Per = env_->getOptions()->findDouble("root_linScheme2_per")->getValue();
  
  if (*isInf == false && nlCons_.size() > 0) {
    if (rs1 || rs2Per ||  rs3_ || rg1 || rg2 || maxVioPer_) {
      extraLin_ = new Linearizations(env_, rel_, minlp_, nlCons_);
      if (rs3_ || rg1 || rg2 || maxVioPer_) {
        extraLin_->setNlpEngine(nlpe_->emptyCopy());        
        extraLin_->findCenter();
        if (maxVioPer_) {
          solC_ = extraLin_->getCenter();
          if (solC_ == NULL) {
            maxVioPer_ = 0;
          }
        }
      }
      if (rs1 || rs2Per || rs3_ || rg1 || rg2) {
        extraLin_->rootLinearizations(nlpe_->getSolution()->getPrimal());
      }
    }
  } else {
    maxVioPer_ = 0;  
  }
  return;
}


void QGHandlerAdvance::separate(ConstSolutionPtr sol, NodePtr node,
                                RelaxationPtr, CutManager *cutMan,
                                SolutionPoolPtr s_pool, ModVector &,
                                ModVector &, bool *sol_found,
                                SeparationStatus *status)
{
  const double *x = sol->getPrimal();
  *status = SepaContinue;

  if (rs3_ && node->getId() == 0) {
    rs3_ = false;
    extraLin_->rootLinScheme3(sol->getPrimal(), lpe_, status);
    if (*status == SepaResolve) {
      return;    
    }
  }

  if (!(isIntFeas_(x))) {
    if (maxVioPer_ > 0) {
      //MS: parameter tunning for depth
      //if (maxVioPer > 0 && node->getDepth() < 10) 
      relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
      maxVio_(x, node, sol_found, s_pool, cutMan, status); // maxViolation
      //lbub_(x, node, sol_found, s_pool, cutMan, status); // bounds based schemes
      //kktCondBasedScheme_(sol, node, cutMan, s_pool, sol_found, status); // dual based scheme
    }
  } else {
    // solve NLP with integer vars fixed
    fixInts_(x);            // Fix integer variables
    relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
    solveNLP_();            // solve NLP
    undoMods_();            // Unfix integer variables
    cutIntSol_(x, cutMan, s_pool, sol_found, status);
  }
  return;
}

void QGHandlerAdvance::lbub_(const double *x, NodePtr node, bool *sol_found,  
                           SolutionPoolPtr s_pool, CutManager *cutMan,
                         SeparationStatus *status)
{
  double bestUb = s_pool->getBestSolutionValue();
  if (bestUb == INFINITY) {
    UInt nIndex = node->getId();
    double nodeLb = node->getLb(), temp;
    if (nIndex == 0) {
      lastLb_ = nodeLb;
      lastNodeIdLbUb_ = nIndex;          
      return;
      //MS: as of now lastNodeId_ used here and maxVio_ are different, see if they are to be kept same or different
    } else {
      if (node->getParent()->getId() != lastNodeIdLbUb_) {
        lastLb_ = node->getParent()->getLb();
      }
    }
    if (nIndex != lastNodeIdLbUb_) {
      // resolve only once in a node at which cuts are added
      resolve_ = 1;          
    } else {
      resolve_ = 0;          
    }
    //MS: change parameter here and everywhere required
    if (nodeLb >= (maxVioPer_/100)*lastLb_) {
      // ECP
      temp = stats_->qgCuts; 
      for ( UInt i = 0; i < nlCons_.size(); ++i) {
        ECPTypeCut_(x, cutMan, i);
      }
      if (temp < stats_->qgCuts) {
        ++(stats_->fracCuts);
        if (!resolve_) {
          if (*status != SepaPrune) {
            *status = SepaContinue;      
          }      
        }
      }

      // restricted NLP
      //
      // ESH
    }
    lastNodeIdLbUb_ = nIndex;          
  }
  return;
}

// MS: remove all the arguments that are not needed
void QGHandlerAdvance::kktCondBasedScheme_(ConstSolutionPtr sol, NodePtr,
                         CutManager *, SolutionPoolPtr,
                         bool *, SeparationStatus *)
{
  int i = 0;
  size_t found;
  ConstraintPtr con;
  LinearFunctionPtr lf;
  std::string consName, findStr = "qgCut"; 
  const double * consDual = sol->getDualOfCons();
  double gradSize  = 0, maxVal = -INFINITY, conVal;

  for (ConstraintConstIterator it=rel_->consBegin();
       it!= rel_->consEnd(); ++it, ++i) {
    if (fabs(consDual[i]) > solAbsTol_) {
      con = *it;
      consName = con->getName();
      found = consName.find(findStr); 
      if (found != std::string::npos)  {
        // QG cuts
        lf = con->getLinearFunction();
        if (lf) {
          for(VariableGroupConstIterator vit = lf->termsBegin();
              vit != lf->termsEnd(); ++vit) {
            gradSize = gradSize + (vit->second)*(vit->second);
          }
        }
      }
      conVal = sqrt(gradSize)*fabs(consDual[i]);
    }
    if (conVal > maxVal) {
      maxVal = conVal;    
    }
    // If maxVal is large then sensitive..how large to be decided
    //
    // What cuts to be added....
    // 1. restricted NLP
    // 2. ECP
    // 3. ESH...

  }
}


void QGHandlerAdvance::restrictedNLP_(NodePtr node, bool *sol_found,  
                           SolutionPoolPtr s_pool, CutManager *cutMan,
                         SeparationStatus *status)
{
  BranchPtr bPtr;
  ModificationPtr mod;
  VarBoundModPtr m = 0;
  //VarBoundMod2 *m2 = 0;
  std::stack<NodePtr> modNodes;
  NodePtr p_node = node->getParent();
  ModificationConstIterator mod_iter;
  
  modNodes.push(node);
  p_node = p_node->getParent();
  while (p_node) {
    modNodes.push(p_node);
    p_node = p_node->getParent();
  }

  // starting from the top, put in modifications made at each node to the
  // engine
  while (!modNodes.empty()) {
    p_node = modNodes.top();
    bPtr = p_node->getBranch();
    if (bPtr) {
      for (mod_iter=bPtr->rModsBegin(); mod_iter!=bPtr->rModsEnd(); 
          ++mod_iter) {
        mod = *mod_iter;
        m = dynamic_cast <VarBoundModPtr> (mod);
        findMods_(m);
        //undoMod_(nlpMods, m);
      }
      // now apply any other mods that were added while processing it.
      for (mod_iter=p_node->modsrBegin(); mod_iter!=p_node->modsrEnd(); ++mod_iter) {
        mod = *mod_iter;
        m = dynamic_cast <VarBoundModPtr> (mod);
        findMods_(m);
        //undoMod_(nlpMods, m);
      }
    }
    modNodes.pop();
  }
  
  solveNLP_();
  //Unfix changes
  undoMods_();

  //MS: use of sol_found here ????
  cutAtNlpSol_(cutMan, s_pool, sol_found, status);
  return;
}


void QGHandlerAdvance::findMods_(VarBoundModPtr m)
{
  //converting modifications from relaxation to original problem
  VarBoundMod *m1 = 0;
  VariablePtr var = m->getVar();
  BoundType boundType= m->getLU();
  
  if (var->getName() != "eta") {
    var = minlp_->getVariable(m->getVar()->getIndex());
    if (boundType == Lower) {
      m1 = new VarBoundMod(var, Lower, m->getNewVal());
    } else if (boundType == Upper) {
      m1 = new VarBoundMod(var, Upper, m->getNewVal());
    } else {
      assert(!"In QGHandlerAdvance: unknown bound type.");        
    }
    m1->applyToProblem(minlp_);
    nlpMods_.push(m1);
  }
  return;
}
  

void QGHandlerAdvance::cutToObjAtFrac_(const double *nlpx, CutManager *,
                         SeparationStatus *status)
{
  if (oNl_) {
    int error=0;
    FunctionPtr f;
    double c, act;
    //ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();
    
    act = o->eval(nlpx, &error);
    if (error == 0) {
      f = o->getFunction();
      LinearFunctionPtr lf = LinearFunctionPtr(); 
      linearAt_(f, act, nlpx, &c, &lf, &error);
      if (error == 0) {
        ++(stats_->fracCuts);
        sstm << "_fracObjCut_" << stats_->fracCuts;
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        if (resolve_) {
          *status = SepaResolve;
        }
        //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      }
    }
  }
  return;
}

void QGHandlerAdvance::cutToConsAtFrac_(const double *nlpx, CutManager *cutman,
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
      if ((fabs(nlpact- cUb) <= solAbsTol_) ||
          (cUb != 0 && fabs(nlpact - cUb) <= fabs(cUb)*solRelTol_)) {
        //MS: add cuts to only active constraint at NLP
        addCutAtFrac_(nlpx, con, cutman, status);
      }
    }
  }
  return;
}

void QGHandlerAdvance::addCutAtFrac_(const double *nlpx,
                        ConstraintPtr con, CutManager*,
                         SeparationStatus *status)
{
  int error=0;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  double c, act, cUb;
  FunctionPtr f = con->getFunction();
  LinearFunctionPtr lf = LinearFunctionPtr();

  act = con->getActivity(nlpx, &error);
  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error==0) {
      cUb = con->getUb();
      ++(stats_->fracCuts);
      sstm << "_fracCut_" << stats_->fracCuts;
      f = (FunctionPtr) new Function(lf);
      rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      if (resolve_) {
        *status = SepaResolve;
      }
      //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
    }
  }
  return;
}

bool QGHandlerAdvance::isIntFeas_(const double* x)
{
  double val;
  VariablePtr var;
  VariableType v_type;
  VariableConstIterator v_iter;

  for (v_iter = rel_->varsBegin(); v_iter != rel_->varsEnd(); ++v_iter) {
    var = *v_iter;
    v_type = var->getType();
    if (v_type == Binary || v_type == Integer) {
      val = x[var->getIndex()];
      if (fabs(val - floor(val+0.5)) > intTol_) {
        return false;
      }
    }
  }
  return true;
}


void QGHandlerAdvance::maxVio_(const double *x, NodePtr node, bool *sol_found,
                               SolutionPoolPtr s_pool, CutManager *cutMan,
                               SeparationStatus *status)
{
  UInt conIndex, nIndex = node->getId(), temp;
  double consMaxVio = maxVioConsIdx_(x, conIndex);

  // MS: associated violation with node*
  if ((nIndex == 0) || (nIndex != lastNodeId_
      && node->getParent()->getId() != lastNodeId_)) {
    lastNodeId_ = nIndex;
    maxVioVal_ = consMaxVio;
    return;
  }

  if (nIndex != lastNodeId_) {
    // resolve only once in a node at which cuts are added (tunning)
    resolve_ = 1;          
  } else {
    resolve_ = 0;
  }

  if (consMaxVio >  (maxVioPer_/100) * maxVioVal_) {
    maxVioVal_ = consMaxVio; 
    //// ECP
    temp = stats_->qgCuts; 
    ECPTypeCut_(x, cutMan, conIndex); // ECP
    if (temp < stats_->qgCuts) {
      ++(stats_->fracCuts);
      if (!resolve_) {
        if (*status != SepaPrune) {
          *status = SepaContinue;      
        }      
      }
    }
    //objCutAtLpSol_(x, cutMan, status);         //MS: required??

    //// NLP
    //temp = stats_->qgCuts; 
    
    //// 1. restricted NLP
    //restrictedNLP_(node, sol_found, s_pool, cutMan, status); // Restricted NLP
    ////2. Fixed NLP solve 
    //fixSomeInts_(x);
    //solveNLP_();
    //undoMods_();
    //cutAtNlpSol_(cutMan, s_pool, sol_found, status);
    
    //if (temp < stats_->qgCuts) {
      //stats_->fracCuts = stats_->fracCuts + stats_->qgCuts - temp;
      //if (!resolve_) {
        //if (*status != SepaPrune) {
          //*status = SepaContinue;      
        //}      
      //} 
    //}
    ////// 3. First fixed, if not found then restricted NLP
    //else {
      //restrictedNLP_(node, sol_found, s_pool, cutMan, status); // Restricted NLP
    //}
    
    //// ESH
    //ESHTypeCut_(x, cutMan, conIndex); // ESH standard way
    //temp = stats_->qgCuts; 
    //if (temp < stats_->qgCuts) {
      //++(stats_->fracCuts);
      //if (!resolve_) {
        //if (*status != SepaPrune) {
          //*status = SepaContinue;      
        //}      
      //}
    //}
  }
  lastNodeId_ = nIndex;
  return;
}


void QGHandlerAdvance::ESHTypeCut_(const double *lpx, CutManager *,
                             int index)
{
  int error = 0;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  bool ptChanged = true;
  
  CCIter it = nlCons_.begin() + index;
  ConstraintPtr con = *it;
  //ConstraintPtr newcon;
  FunctionPtr f = con->getFunction();
  double c, nlpact, cUb = con->getUb();
  
  UInt numVars =  minlp_->getNumVars();
  double *x = new double[numVars];
  double *xl = new double[numVars];
  double *xu = new double[numVars];
  std::copy(solC_,solC_+numVars,xl);
  std::copy(lpx,lpx+numVars,xu);
  
  while (true) {
    for (UInt i = 0 ; i < numVars; ++i) {
      x[i] = 0.5*(xl[i] + xu[i]);
    }
    nlpact = f->eval(x, &error);
    if (error == 0) {
      if (nlpact > cUb+solAbsTol_ && (cUb == 0 ||
                                  nlpact > cUb+fabs(cUb)*solRelTol_)) {
        std::copy(x,x+numVars,xu);
      } else {
        if (fabs(cUb-nlpact) <= solAbsTol_ || 
            (cUb!=0 && fabs(cUb-nlpact) <= fabs(cUb)*solRelTol_)) {
            break;
        } else {
          std::copy(x,x+numVars,xl);
        }
      }
    } else {
      //MS: think later what can be done here
      ptChanged = false;
      break;    
    }
  }

  delete [] xl;
  delete [] xu;

  lf = 0;
  if (ptChanged) {
    nlpact =  con->getActivity(x, &error);
    if (error == 0) {
      linearAt_(f, nlpact, x, &c, &lf, &error);
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
      delete [] x;
      return;
    }
  } else {
    delete [] x;
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      linearAt_(f, nlpact, lpx, &c, &lf, &error);
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
      delete [] x;
      return;
    }
  }

  if (error == 0) {
    ++(stats_->qgCuts); 
    sstm << "_qgCut_" << stats_->qgCuts;
    f = (FunctionPtr) new Function(lf);
    rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
    //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
  }
  delete [] x;
  return;
}


double QGHandlerAdvance::maxVioConsIdx_(const double *x, UInt &index)
{
  ConstraintPtr c;
  int error=0, i = 0;
  double act, cUb, vio = 0.0, max = -INFINITY;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it, ++i) {
    c = *it;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if (act > cUb+solAbsTol_ && (cUb == 0 ||
                                   act > cUb + fabs(cUb)*solRelTol_)) {
        //if (fabs(cUb) > solAbsTol_ && fabs(cUb) > solRelTol_) {
        if (fabs(cUb) > solAbsTol_) {
          vio = 100*(act - cUb)/fabs(cUb);      
        } else {
          vio = act - cUb;
        }
        if (vio > max) {
          max = vio;          
          index = i; 
        }
      }      
    }
  }
  return max;
}


void QGHandlerAdvance::ECPTypeCut_(const double *lpx, CutManager *, int i)
{
  int error = 0;
  std::stringstream sstm;
  //ConstraintPtr newcon;
  LinearFunctionPtr lf = 0;
  
  ConstraintPtr con = *(nlCons_.begin() + i);
  FunctionPtr f = con->getFunction();
  double c, cUb, lpvio, act =  con->getActivity(lpx, &error);
  
  if (error == 0) {
    cUb = con->getUb();
    if ((act > cUb + solAbsTol_) &&
        (cUb == 0 || act > cUb+fabs(cUb)*solRelTol_)) {
      linearAt_(f, act, lpx, &c, &lf, &error);
      if (error==0) {
        lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
        if ((lpvio > solAbsTol_) && ((cUb-c)==0 ||
                                 (lpvio>fabs(cUb-c)*solRelTol_))) {
          ++(stats_->qgCuts);
          sstm << "_qgCut_" << stats_->qgCuts;
          f = (FunctionPtr) new Function(lf);
          rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          return;
        } else {
          delete lf;
          lf = 0;
        }
      }
    }
  }	else {
    logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
      " this point. "<<  std::endl;
  }

  return;
}


void QGHandlerAdvance::solveNLP_()
{
  ++(stats_->nlpS);
  nlpStatus_ = nlpe_->solve();
  return;
}


void QGHandlerAdvance::undoMods_()
{
  ModificationPtr m = 0;
  while(nlpMods_.empty() == false) {
    m = nlpMods_.top();
    m->undoToProblem(minlp_);
    nlpMods_.pop();
    delete m;
  }
  return;
}


void QGHandlerAdvance::updateUb_(SolutionPoolPtr s_pool, double nlpval,
                          bool *sol_found)
{
  double bestval = s_pool->getBestSolutionValue();

  if ((bestval - objATol_ > nlpval) ||
        (bestval != 0 && (bestval - fabs(bestval)*objRTol_ > nlpval))) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, nlpval);
    // modify solC_ whenever a new point is found
    //for (UInt i = 0 ; i < minlp_->getNumVars(); ++i) {
      //solC_[i] = 0.5*(solC_[i] + x[i]);
    //}
    *sol_found = true;
  }
  return;
}


void QGHandlerAdvance::writeStats(std::ostream &out) const
{
  if (extraLin_ != 0) {
    extraLin_->writeStats(out);  
  } 

  out
    << me_ << "number of nlps solved                       = "
    << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps                   = " 
    << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps                     = " 
    << stats_->nlpF << std::endl
    << me_ << "number of nlps hit engine iterations limit  = " 
    << stats_->nlpIL << std::endl
    << me_ << "number of cuts added at frac nodes          = " 
    << stats_->fracCuts << std::endl
    << me_ << "total number of cuts added                  = " 
    << stats_->qgCuts << std::endl;
  return;
}


std::string QGHandlerAdvance::getName() const
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
