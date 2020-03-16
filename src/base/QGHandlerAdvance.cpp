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
#include "CGraph.h"
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
#include "Timer.h"
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
  solC_(0),
  objVar_(0),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  extraLin_(0),
  rs3_(0),
  maxVioPer_(0),
  objVioMul_(0),
  consDual_(0),
  //lpdist_(-1),
  lastNodeId_(-1)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new QGStats();
  stats_->cuts = 0;
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
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
  } else {
    if (solC_) {
      delete [] solC_;
      solC_ = 0;
    }
  }

  env_ = 0;
  lpe_ = 0;
  rel_ = 0;
  nlpe_ = 0;
  minlp_ = 0;
  extraLin_ = 0;  
  nlCons_.clear();
  consDual_.clear();
}


void QGHandlerAdvance::dualBasedCons_(ConstSolutionPtr sol)
{
  //// Dual based scheme
  //double K = 0.5; // MS: can be parametrized.
  //highDualCons_.clear();
  //double act, maxDual = -INFINITY;
  //const double * consDual = sol->getDualOfCons();

  //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    //act = consDual[(*it)->getIndex()];
    //if (act > maxDual) {
      //maxDual = act;    
    //}
  //}

  //if (maxDual > 0) {
    //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
      //act = consDual[(*it)->getIndex()];
      //if (act >= K*maxDual) {
        //highDualCons_.push_back(*it);
      //}
    //}
  //}

  ////  Score based scheme
  double lambda1 = 0.05, lambda2 = 0.95;
  const double * consDual = sol->getDualOfCons();
  //consDual_.clear();
  //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    //consDual_.push_back(consDual[(*it)->getIndex()]);
  for (UInt i = 0; i < nlCons_.size(); ++i) {
    consDual_[i] = lambda1*consDual_[i] + 
      lambda2*(consDual[(nlCons_[i])->getIndex()]);
  }

  return;
}

void QGHandlerAdvance::addInitLinearX_(ConstSolutionPtr sol)
{ 
  int error = 0;
  FunctionPtr f;
  ConstraintPtr con;
  double c, act, cUb;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  const double *x = sol->getPrimal();
  //ConstraintPtr newcon;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        cUb = con->getUb();
        ++(stats_->cuts);
        sstm << "_qgCutRoot_" << stats_->cuts;
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
      ++(stats_->cuts);
      sstm << "_qgObjCutRoot_" << stats_->cuts;
      f = o->getFunction();
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


void QGHandlerAdvance::fixInts_(const double *x)
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


void QGHandlerAdvance::initLinear_(bool *isInf)
{
  *isInf = false;
  
  nlpe_->load(minlp_);
  solveNLP_();
  
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    addInitLinearX_(nlpe_->getSolution());
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    addInitLinearX_(nlpe_->getSolution());
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
    std::string name = "dummy_obj_var";
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
  int error = 0;
  double act, cUb;
  ConstraintPtr con;
  UInt temp = stats_->cuts;
  
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act =  con->getActivity(lpx, &error);
    if (error == 0) { 
      cUb = con->getUb();
      if ((act > cUb + solAbsTol_) &&
        (cUb == 0 || act > cUb+fabs(cUb)*solRelTol_)) {
        ECPTypeCut_(lpx, cutMan, con, act);
      }
    }
  }
  
  if (temp < stats_->cuts) {
    *status = SepaResolve;
  }
  return;
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
      //std::cout << relobj_ << " " << lpvio << "\n";
      if (maxVioPer_) {
        if ((fabs(relobj_) < solAbsTol_) || (fabs(lpvio) < solAbsTol_) || 
            ((lpvio-relobj_)*100/fabs(relobj_) < objVioMul_)) {
          return;
        }
        //std::cout << "lpvio " << lpvio << " relobj_ " << relobj_ << "\n";
      }
      if ((lpvio > solAbsTol_) &&
          (relobj_ == 0 || lpvio > fabs(relobj_)*solRelTol_)) {
          lf = 0;
          f = o->getFunction();
          linearAt_(f, act, lpx, &c, &lf, &error);
          if (error == 0) {
            lpvio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((lpvio > solAbsTol_) && ((relobj_-c) == 0
                                       || lpvio > fabs(relobj_-c)*solRelTol_)) {
              ++(stats_->cuts);
              *status = SepaResolve;
              lf->addTerm(objVar_, -1.0);
              f = (FunctionPtr) new Function(lf);
              sstm << "_qgObjCut_" << stats_->cuts;
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
        ++(stats_->cuts);
        sstm << "_qgCut_" << stats_->cuts;
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
              ++(stats_->cuts);
              sstm << "_qgObjCut_" << stats_->cuts;
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
  
  for (ConstraintConstIterator it = minlp_->consBegin();
       it != minlp_->consEnd(); ++it) {
    c = *it;
    fType = c->getFunctionType();
    if (fType != Constant && fType != Linear) {
      nlCons_.push_back(c);
    }
  }
 
  linearizeObj_();
  initLinear_(isInf);

  bool temp1 = 0, temp2 = 0;
  UInt nlCons = nlCons_.size();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  maxVioPer_ = env_->getOptions()->findDouble("maxVioPer")->getValue();
  objVioMul_ = env_->getOptions()->findDouble("objVioMul")->getValue();
  cutMethod_ = env_->getOptions()->findString("cutMethod")->getValue();
  bool rg1 = env_->getOptions()->findBool("root_linGenScheme1")->getValue();
  double rs1 = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  double rs2Per = env_->getOptions()->findDouble("root_linScheme2")->getValue();
  double rg2 = env_->getOptions()->findDouble("root_linGenScheme2_per")->getValue(); //MS: change name in Environment
      
  if (rs3_ && nlCons > 0) {
    temp1 = 1;
  }
      
  if (maxVioPer_ && (cutMethod_ == "esh") && (nlCons > 0)) {
    temp2 = 1;
  }

  if (*isInf == false && ((nlCons_.size() > 0) || oNl_)) {
    if (rs1 || rs2Per ||  rs3_ || rg1 || rg2) {
      extraLin_ = new Linearizations(env_, rel_, minlp_, nlCons_, objVar_,
                                     nlpe_->getSolution());
      if (temp1 || temp2 || rg1 || rg2) {
        extraLin_->setNlpEngine(nlpe_->emptyCopy());        
        extraLin_->findCenter();
        if (temp2) {
          solC_ = extraLin_->getCenter();
          if (solC_ == 0) {
            maxVioPer_ = 0;
          }
        }
      } 
      if (rs1 || rs2Per || rg1 || rg2) {
        extraLin_->rootLinearizations();
      }
    } else if (temp2) {
      findCenter_();
      if (solC_ == 0) {
        maxVioPer_ = 0;
      } else {
        const double *nlpx = nlpe_->getSolution()->getPrimal();
        double l1 = 0.5, l2 = 0.5; //MS: can be parametrized.
        for (UInt i = 0 ; i < minlp_->getNumVars(); ++i) {
          solC_[i] = l1*nlpx[i] + l2*solC_[i];
        }
      }
    }
  } else {
    rs3_ = 0;
  }
   
 //// For dual multiplier based maxvio rule and score based rule
 //// Also make appropriate changes in the updateUb_()
  if (maxVioPer_ && (nlCons > 0)) {
    consDual_.resize(nlCons, 0);
    dualBasedCons_(nlpe_->getSolution());
  }
  return;
}


void QGHandlerAdvance::solveCenterNLP_(EnginePtr nlpe)
{ 
  if (solC_) {
    delete [] solC_;
    solC_ = 0;
  }
  EngineStatus nlpStatus = nlpe->solve();

  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      //std::cout << "Center " << std::setprecision(6) << nlpe_->getSolution()->getObjValue() << "\n";
      //exit(1);
      UInt numVars = minlp_->getNumVars();
      const double *temp = nlpe->getSolution()->getPrimal();
      solC_ = new double[numVars];
      std::copy(temp, temp+numVars, solC_);
    }
    break;
  case (ProvenUnbounded):
    break;
  case (EngineIterationLimit):
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
  case (FailedFeas):
  case (EngineError):
  case (FailedInfeas):
  case (ProvenFailedCQFeas):
  case (EngineUnknownStatus):
  case (ProvenFailedCQInfeas):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status = "
      << nlpe_->getStatusString() << std::endl;
    break;
  }

  return;
}


void QGHandlerAdvance::findCenter_()
{
  double lb, ub;
  FunctionPtr fnewc;
  ConstraintPtr con;
  FunctionType fType;
  VariablePtr vPtr, v;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone(env_);
  EnginePtr nlpe = nlpe_->emptyCopy();
  LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
  
  // Modify objective 
  vPtr = inst_C->newVariable(-INFINITY, 0, Continuous, "eta", VarHand);
  vPtr->setFunType_(Nonlinear);
  inst_C->removeObjective();
  lfc->addTerm(vPtr, 1.0);
  fnewc = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(fnewc, 0.0, Minimize);


  // Solving more restricted proiblem - consider only inequality constraints
  // including variable bounds - to find center 
  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
    //if (fType == Constant) {
      //inst_C->markDelete(con);
      //continue;
    //} else 
    if (fType == Linear)  {
      continue;
    } else {
      if (con->getLinearFunction()) {
        lfc = con->getLinearFunction()->clone();
        lfc->addTerm(vPtr, -1.0);
      } else {
        lfc = (LinearFunctionPtr) new LinearFunction();
        lfc->addTerm(vPtr, -1.0);
      }
    }
    inst_C->changeConstraint(con, lfc, lb, ub);
  }

  inst_C->prepareForSolve();
  nlpe->load(inst_C);
  solveCenterNLP_(nlpe);
  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);

  if (nlpe->getStatusString() == "ProvenUnbounded") {
    logger_->msgStream(LogDebug) << me_ 
      << " Problem for finding center is unbounded." <<
     " Solving a restricted problem." << std::endl;

    for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
         ++it) {
      con = *it;
      lb = con->getLb();
      ub = con->getUb();
      fType = con->getFunctionType();
      if (fType == Linear)  {
        if (lb != -INFINITY && ub != INFINITY) {
          if (fabs(lb-ub) <= solAbsTol_) {
            continue;       
          }
          cp.push_back(con);
          inst_C->markDelete(con);
          continue;
        } else if (lb != -INFINITY) {
          ub = INFINITY;
          lfc = con->getLinearFunction()->clone();
          lfc->addTerm(vPtr, 1.0);
        } else if (ub != INFINITY ) {
          lb = -INFINITY;
          lfc = con->getLinearFunction()->clone();
          lfc->addTerm(vPtr, -1.0);
        } 
      }  else {
        continue;      
      }
      inst_C->changeConstraint(con, lfc, lb, ub);
    }

    for (UInt i = 0; i < cp.size(); ++i) {
      con = cp[i];
      lb = con->getLb(), ub = con->getUb();
      lfc = con->getLinearFunction()->clone();
      lfc->addTerm(vPtr, 1.0);
      fnewc = (FunctionPtr) new Function(lfc);
      inst_C->newConstraint(fnewc, lb, INFINITY);

      lfc = con->getLinearFunction()->clone();
      lfc->addTerm(vPtr, -1.0);
      fnewc = (FunctionPtr) new Function(lfc);
      inst_C->newConstraint(fnewc, -INFINITY, ub);
    }
    cp.clear();

    for (VariableConstIterator vit=inst_C->varsBegin(); vit!=inst_C->varsEnd()-1;
         ++vit) {
      v = *vit;
      lb = v->getLb(), ub = v->getUb();
      if (fabs(lb-ub) <= solAbsTol_) {
        continue;
      }

      if (lb != -INFINITY) {
        lfc = (LinearFunctionPtr) new LinearFunction();
        lfc->addTerm(vPtr, 1.0);
        lfc->addTerm(v, 1.0);
        fnewc = (FunctionPtr) new Function(lfc);
        inst_C->newConstraint(fnewc, lb, INFINITY);
      }
      
      if (ub != INFINITY) {
        lfc = (LinearFunctionPtr) new LinearFunction();
        lfc->addTerm(vPtr, -1.0);
        lfc->addTerm(v, 1.0);
        fnewc = (FunctionPtr) new Function(lfc);
        inst_C->newConstraint(fnewc, -INFINITY, ub);
      }
    }
    //inst_C->write(std::cout);

    inst_C->prepareForSolve();  
    nlpe->load(inst_C);
    solveCenterNLP_(nlpe);
  
    if (solC_) {
      if (fabs(nlpe->getSolution()->getObjValue()) <= solAbsTol_) {
        delete [] solC_;
        solC_ = 0;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "NLP engine status for restricted center problem = "
        << nlpe->getStatusString() << std::endl;
    }
  } else {
    if (solC_) {
      if (fabs(nlpe->getSolution()->getObjValue()) <= solAbsTol_) {
        delete [] solC_;
        solC_ = 0;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "NLP engine status for center problem = "
        << nlpe->getStatusString() << std::endl;
    }
  }

  delete nlpe;
  nlpe = 0;
  delete inst_C;
  inst_C = 0;
  return;
}


void QGHandlerAdvance::separate(ConstSolutionPtr sol, NodePtr node,
                                RelaxationPtr, CutManager *cutMan,
                                SolutionPoolPtr s_pool, ModVector &,
                                ModVector &, bool *sol_found,
                                SeparationStatus *status)
{
  bool isIntFeas;
  const double *x = sol->getPrimal();

  //if (node->getId() == 0) {
    //shortestDist_(sol);
  //} else {
    //std::cout << "Shortest dist NLP solve status and shortest distance ; " 
      //<< shortestNlpStatus_ << " ; " << std::setprecision(6) << lpdist_ << std::endl;
    //exit(1);
  //}


  *status = SepaContinue;

  if (rs3_ && node->getId() == 0) {
    rs3_ = 0;
    extraLin_->rootLinScheme3(lpe_, status);
    if (*status == SepaResolve) {
      return;
    }
  }

  isIntFeas = isIntFeas_(x);
  if (isIntFeas) {
    fixInts_(x);            // Fix integer variables
    relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
    solveNLP_();            // solve NLP
    undoMods_();            // Unfix integer variables
    cutIntSol_(x, cutMan, s_pool, sol_found, status);
  } else {
     if (maxVioPer_ > 0) {
       relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
       maxVio_(sol, node, cutMan, status); // maxViolation
    } 
  }
  return;
}

void QGHandlerAdvance::ESHTypeCut_(const double *x, CutManager *cutMan)
{
  UInt i = 0;
  int error = 0;
  ConstraintPtr c;
  double act, cUb;
  std::vector<UInt > consToLin; // cons to add linearizations
  bool active = false, vio = false, ptFound;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it, ++i) {
    c = *it;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        if (!vio) {
          vio = true;
          if (consToLin.size() != 0) {
            consToLin.clear();
          }
        }
        consToLin.push_back(i);
      } else if ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_)) {
        if (!vio) {
          if (!active) {
            active = true;         
          }
          consToLin.push_back(i);
        }
      }
    }
  }
  
  if (vio) {
    double* xnew = new double[minlp_->getNumVars()];
    ptFound = boundaryPtForCons_(xnew, x, consToLin);
    if (ptFound) {
      genLin_(xnew, consToLin, cutMan);
    }
    delete [] xnew;
  } else if (active) {
    genLin_(x, consToLin, cutMan);
  }
  return;
}


void QGHandlerAdvance::genLin_(const double *x, std::vector<UInt > vioCons,
                               CutManager *cutMan)
{
  int error;
  FunctionPtr f;
  ConstraintPtr con;
  double c, cUb, act;
  LinearFunctionPtr lf;
  std::stringstream sstm;

  for (UInt i = 0; i < vioCons.size(); ++i) {
    error = 0;
    con = nlCons_[vioCons[i]];
    act = con->getActivity(x, &error);
    if (error == 0) {
      lf = 0;
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        ++(stats_->cuts);
        cUb = con->getUb();
        f = (FunctionPtr) new Function(lf);
        sstm << "_qgCut_" << stats_->cuts;
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      } 
    }
  }

  if (oNl_) {
    SeparationStatus s = SepaContinue;
    objCutAtLpSol_(x, cutMan, &s);
  }
  return;
}


bool QGHandlerAdvance::boundaryPtForCons_(double* xnew, const double *xOut,
                                     std::vector<UInt > &vioCons)
{
  ConstraintPtr con;
  int error = 0, repPt = 0;
  bool firstVio, firstActive;
  UInt numVars =  minlp_->getNumVars(); 

  double* xl = new double[numVars];
  double* xu = new double[numVars];
  double act, cUb, repSol, oldSol = INFINITY, lambdaIn = 0.5, lambdaOut = 0.5;

  std::copy(xOut, xOut+numVars, xu);
  std::copy(solC_, solC_+numVars, xl);
    
  while (true) {
    for (UInt i = 0 ; i < numVars; ++i) {
      xnew[i] = lambdaIn*xl[i] + lambdaOut*xu[i];
    }
    
    repSol = 0;
    firstVio = false, firstActive = false;

    for (UInt k = 0; k < vioCons.size(); ) {
      con = nlCons_[vioCons[k]];
      act = con->getActivity(xnew, &error);
      if (error != 0) {
        delete [] xl;
        delete [] xu;
        return false;
      }
      cUb = con->getUb();
      repSol = repSol + act;
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) { // violated
        if (!firstVio) {
          firstVio = true;
          if (k != 0) {
            vioCons.erase(vioCons.begin(), vioCons.begin() + k);
            k = 0;
          }
        }
        ++k;
      } else if ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_)) { // active
        if (firstVio) {
          vioCons.erase(vioCons.begin() + k);
        } else {
          if (!firstActive) {
            firstActive = true;
            if (k != 0) {
              vioCons.erase(vioCons.begin(),vioCons.begin() + k);
              k = 0;
            }
          }
          ++k;
        }
      } else {
        if (firstVio || firstActive) {
           vioCons.erase(vioCons.begin() + k);
        } else {
           ++k;
        }
      }
    }
    
    if (fabs(repSol-oldSol) <= solAbsTol_) {
      ++repPt;
    } else {
      repPt = 0;
      oldSol = repSol;
    }

    if (repPt == 10) {
      firstVio = false;
      firstActive = true;
    }

    if (!firstVio) {
      if (!firstActive) {
        std::copy(xnew,xnew+numVars,xl);
      } else {
        delete [] xl;
        delete [] xu;
        return true;
      }
    } else {
      std::copy(xnew,xnew+numVars,xu);
    } 
  }
  
  return false;
}


//void QGHandlerAdvance::ESHTypeCut_(const double *lpx, CutManager *cutMan,
                             //ConstraintPtr con)
//{
  //int error = 0;
  //double *x, *xl, *xu;
  //bool ptChanged = true;
  //FunctionPtr f = con->getFunction();
  //UInt numVars =  minlp_->getNumVars(), repPt = 0;
  //double cUb = con->getUb(), nlpact, repSol = INFINITY,
         //lambda1 = 0.5, lambda2 = 0.5;
  
  //x  = new double[numVars];
  //xl = new double[numVars];
  //xu = new double[numVars];
  //std::copy(lpx,lpx+numVars,xu);
  //std::copy(solC_,solC_+numVars,xl);
  

  //while (true) {
    //for (UInt i = 0 ; i < numVars; ++i) {
      //x[i] = lambda1*xl[i] + lambda2*xu[i];
    //}
    //nlpact = f->eval(x, &error);
    //if (error == 0) {
      //if (nlpact > cUb+solAbsTol_ && (cUb == 0 ||
                                  //nlpact > cUb+fabs(cUb)*solRelTol_)) {
        //std::copy(x,x+numVars,xu);
      //} else {
        //if (fabs(cUb-nlpact) <= solAbsTol_ ||
            //(cUb!=0 && fabs(cUb-nlpact) <= fabs(cUb)*solRelTol_)) {
            //break;
        //} else {
          //std::copy(x,x+numVars,xl);
        //}
      //}
    //} else {
      //ptChanged = false;
      //break;
    //}
    //if (nlpact == repSol) {
      //++repPt;    
    //} else {
      //repPt = 0;
      //repSol = nlpact;
    //}
    //if (repPt == 10) {
      //break;    
    //}
  //}

  //SeparationStatus s = SepaContinue;
  //if (ptChanged) {
    //double c;
    //std::stringstream sstm;
    //LinearFunctionPtr lf = 0;
    //linearAt_(f, nlpact, x, &c, &lf, &error);
    //if (error == 0) { 
      //++(stats_->cuts);
      //sstm << "_qgCut_" << stats_->cuts;
      //f = (FunctionPtr) new Function(lf);
      //rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      ////newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //// Cut to nonlinear objective
      //objCutAtLpSol_(x, cutMan, &s);
    //}
  //} else {
    //nlpact =  con->getActivity(lpx, &error);
    //ECPTypeCut_(lpx, cutMan, con, nlpact);
   
    //// Cut to nonlinear objective
    //objCutAtLpSol_(lpx, cutMan, &s);
  //}

  
  //delete [] x;
  //delete [] xl;
  //delete [] xu;
  //return;
//}


//void QGHandlerAdvance::maxVio_(ConstSolutionPtr sol, NodePtr node,
                               //CutManager *cutMan,
                               //SeparationStatus *status)
//{
  //int error = 0; 
  //ConstraintPtr c;
  //double act, cUb, vio = 0.0;
  //const double *x = sol->getPrimal();
  //UInt  temp = stats_->cuts, nodeId = node->getId();

  //if (cutMethod_ == "ecp" || (nlCons_.size() == 0 && oNl_)) {
    ////for (CCIter it=highDualCons_.begin(); it!=highDualCons_.end(); ++it) {
    //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
      //c = *it; 
      //act = c->getActivity(x, &error);
      //if (error == 0) { 
        //cUb = c->getUb();
        //if (act > cUb+solAbsTol_ && (cUb == 0 || 
                                     //act > cUb + fabs(cUb)*solRelTol_)) {
          ////std::cout << "node " << node->getId() << " vio " << act -cUb << "\n";
          //if (fabs(cUb) > solAbsTol_) {
            //vio = 100*(act - cUb)/fabs(cUb);     
          //} else {
            //vio = 100*(act - cUb); 
          //}
          ////std::cout << vio << "\n";
          //if (vio >= maxVioPer_) {
            //ECPTypeCut_(x, cutMan, c, act);
          //}
        //}
      //}
    //}

    //if (oNl_) {
      //SeparationStatus s = SepaContinue;
      //objCutAtLpSol_(x, cutMan, &s);
    //}
  //} else if (cutMethod_ == "esh") {
     ////for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    //for (CCIter it=highDualCons_.begin(); it!=highDualCons_.end(); ++it) {
      //c = *it; 
      //act = c->getActivity(x, &error);
      //if (error == 0) { 
        //cUb = c->getUb();
        //if (act > cUb+solAbsTol_ && (cUb == 0 || 
                                     //act > cUb + fabs(cUb)*solRelTol_)) {
          ////std::cout << "node " << node->getId() << " vio " << act -cUb << "\n";
          //if (fabs(cUb) > solAbsTol_) {
            //vio = 100*(act - cUb)/fabs(cUb);     
          //} else {
            //vio = 100*(act - cUb); 
          //}
          ////std::cout << vio << "\n";
          //if (vio >= maxVioPer_) {
            //ESHTypeCut_(x, cutMan);
            //break;
          //}
        //}
      //}
    //}
  //}
  
  //stats_->fracCuts = stats_->fracCuts + (stats_->cuts - temp);

  //if ((temp < stats_->cuts) && (nodeId != UInt(lastNodeId_))) {
    //*status = SepaResolve;
  //}
  
  ////std::cout << "Node " << node->getId() << " depth " << node->getDepth() 
    ////<< " vio val " << val << " bnd " << bnd << " max vio % " << max << "\n";
  //lastNodeId_ = nodeId;
  //return;
//}


//// Score based rule
void QGHandlerAdvance::maxVio_(ConstSolutionPtr sol, NodePtr node,
                               CutManager *cutMan, SeparationStatus *status)
{
  int error = 0;
  ConstraintPtr c;
  UInt temp = stats_->cuts;
  std::vector<double > consAct;
  const double *x = sol->getPrimal();
  double act, cUb, vio, totScore = 0, parentScore; 
  UInt i = 0, vioConsNum = 0, nodeId = node->getId();

  if (nlCons_.size() > 0) {
    for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it, ++i) {
      c = *it;
      act = c->getActivity(x, &error);
      if (error == 0) {
        cUb = c->getUb();
        vio = act - cUb;
        if (cutMethod_ == "ecp") {
          consAct.push_back(act);
        }
        if ((vio > solAbsTol_) &&
            (cUb == 0 || vio > fabs(cUb)*solRelTol_)) {
          ++vioConsNum;
          vio = act - cUb;
          if (fabs(cUb) > solAbsTol_) {
            totScore = totScore + vio*consDual_[i] + vio/fabs(cUb);
            //std::cout << "act = "<< act << " vio = " << vio << " dual = " << consDual_[i] << " cub = " << cUb << std::endl;
          } else {
            totScore = totScore + vio*consDual_[i] + vio;
          }
        }
      } else {
        if (cutMethod_ == "ecp") {
          consAct.push_back(INFINITY);
        }
      }
    }

    if (vioConsNum > 0) {
      totScore = totScore/vioConsNum;
      node->setVioVal(totScore);
    } else {
      node->setVioVal(totScore);
      return;
    }

    if (nodeId > 0 && int(nodeId) != lastNodeId_) {
      parentScore = node->getParent()->getVioVal();
      if (parentScore < INFINITY && totScore < INFINITY) {
        if (fabs(parentScore) > 1e-3 && fabs(totScore) > 1e-2 
            && fabs(totScore) > (maxVioPer_*fabs(parentScore))) { //MS: here maxVioPer_ is in times (0.5, 1, 2, 5,..)
          //std::cout << std::setprecision(6) << "node, score, and parent's score "<< nodeId << " " << totScore << " " << parentScore << "\n";
          if (cutMethod_ == "ecp") {
            i = 0;
            for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it, ++i) {
              c = *it;
              act = consAct[i];
              if (act == INFINITY) {
                continue;
              }
              cUb = c->getUb();
              vio = act - cUb;
              if ((vio > solAbsTol_) &&
                    (cUb == 0 || vio > fabs(cUb)*solRelTol_)) {
                ECPTypeCut_(x, cutMan, c, act);
              }
            }

            if (oNl_) {
              SeparationStatus s = SepaContinue;
              objCutAtLpSol_(x, cutMan, &s);
            }
          } else if (cutMethod_ == "esh") {
            ESHTypeCut_(x, cutMan);
          }
        }
      }
    }
  } else if (oNl_) {
    SeparationStatus s = SepaContinue;
    objCutAtLpSol_(x, cutMan, &s);
  }

  stats_->fracCuts = stats_->fracCuts + (stats_->cuts - temp);

  if ((temp < stats_->cuts) && (nodeId != UInt(lastNodeId_))) {
    *status = SepaResolve;
  }

  lastNodeId_ = nodeId;
  return;
}


//void QGHandlerAdvance::shortestDist_(ConstSolutionPtr sol)
//{
  //EnginePtr nlpe = nlpe_->emptyCopy(); //Engine for modified problem
  //ProblemPtr inst_C = minlp_->clone();
  //UInt n = minlp_->getNumVars();
  //std::vector<CNode *> temp;
  //CNode * n1, * n2, * nf;
  //VariablePtr v;
  //const double *x = sol->getPrimal();
  ////std::cout << "Problem before construction\n";
  ////inst_C->write(std::cout);

  //CGraphPtr t = (CGraphPtr) new CGraph();
    
  //for (VariableConstIterator v_iter=inst_C->varsBegin(); 
      //v_iter!=inst_C->varsEnd(); ++v_iter) {
    //v = *v_iter;
    //v->setFunType_(Nonlinear);
    //n1 = t->newNode(v);
    //n2 = t->newNode(x[v->getIndex()]);
    //nf = t->newNode(Minotaur::OpMinus, n1, n2);
    //n2 = t->newNode(2);
    //n1 = t->newNode(Minotaur::OpPowK, nf, n2);
    //temp.push_back(n1);
  //}  
  //CNode **childr = new Minotaur::CNode *[n];
  //for (UInt i = 0; i < n; ++i) {
    //childr[i]=temp[i];
  //}  
  //nf = t->newNode(Minotaur::OpSumList,childr, n);
  ////n1 = t->newNode(Minotaur::OpSqrt, nf, 0);
  ////t->setOut(n1);
  //t->setOut(nf);
  //t->finalize();
  
  //inst_C->removeObjective();
  //FunctionPtr f = (FunctionPtr) new Function(t);
  //inst_C->newObjective(f, 0.0, Minimize);
  //inst_C->prepareForSolve();
  //std::cout << "Shortest distance NLP \n";
  ////inst_C->write(std::cout);
  //nlpe->load(inst_C);
  //shortestNlpStatus_ = nlpe->solve();

  //switch(shortestNlpStatus_) {
  //case (ProvenOptimal):
  //case (ProvenLocalOptimal):
    //lpdist_ = sqrt(nlpe->getSolution()->getObjValue());
    //break;
  //case (EngineIterationLimit):
  //case (ProvenInfeasible):
  //case (ProvenLocalInfeasible): 
  //case (ProvenObjectiveCutOff):
    //break;
  //case (FailedFeas):
  //case (EngineError):
  //case (FailedInfeas):
  //case (ProvenUnbounded):
  //case (ProvenFailedCQFeas):
  //case (EngineUnknownStatus):
  //case (ProvenFailedCQInfeas):
  //default:
    //logger_->msgStream(LogError) << me_ << "NLP engine status = "
      //<< nlpe->getStatusString() << std::endl;
    //std::cout << "Shortest distance of LP sol from feasible region couldn't find" << std::endl;
    //break;
  //}  
  
  //delete nlpe;
  //nlpe = 0;
  //delete inst_C;
  //inst_C = 0;
  //delete [] childr;
  //temp.clear();
  //return;
//}


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


void QGHandlerAdvance::ECPTypeCut_(const double *lpx, CutManager *, ConstraintPtr con, double act)
{
  int error = 0;
  std::stringstream sstm;
  //ConstraintPtr newcon;
  LinearFunctionPtr lf = 0;
  
  double c, cUb, lpvio;
  FunctionPtr f = con->getFunction();
  
  linearAt_(f, act, lpx, &c, &lf, &error);
  
  if (error==0) {
    cUb = con->getUb();
    lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
    if ((lpvio > solAbsTol_) && ((cUb-c)==0 ||
                             (lpvio>fabs(cUb-c)*solRelTol_))) {
      ++(stats_->cuts);
      sstm << "_qgCut_" << stats_->cuts;
      f = (FunctionPtr) new Function(lf);
      rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      return;
    } else {
      delete lf;
      lf = 0;
    }
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
    *sol_found = true;
    
    if (maxVioPer_ && (nlCons_.size() > 0)) {
      dualBasedCons_(nlpe_->getSolution());
    }
  }
  return;
}


void QGHandlerAdvance::writeStats(std::ostream &out) const
{
  if (extraLin_ != 0) {
    extraLin_->writeStats(out);  
  } 

	//std::cout << "Shortest dist NLP solve status and shortest distance ; " 
			//<< shortestNlpStatus_ << " ; " << std::setprecision(6) << lpdist_ << std::endl;

  out
    << me_ << "number of nlps solved                       = "
    << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps                   = " 
    << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps                     = " 
    << stats_->nlpF << std::endl
    << me_ << "number of nlps hit engine iterations limit  = " 
    << stats_->nlpIL << std::endl
    << me_ << "number of frac cuts added                   = " 
    << stats_->fracCuts << std::endl
    << me_ << "number of total cuts added                  = " 
    << stats_->cuts << std::endl;
  return;
}


std::string QGHandlerAdvance::getName() const
{
  return "QGAdv Handler (Quesada-Grossmann)";
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
