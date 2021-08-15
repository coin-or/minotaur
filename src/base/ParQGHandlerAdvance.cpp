//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file ParQGHandlerAdvance.cpp
 * \Briefly define a handler for the textbook type Quesada-Grossmann
 * Algorithm.
 * \Authors Meenarli Sharma, Prashant Palkar, Ashutosh Mahajan, Indian Institute of
 * Technology Bombay
 */

#if USE_OPENMP
#include <omp.h>
#endif
#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "Constraint.h"
#include "Cut.h"
#include "CutManager.h"
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
#include "ParQGHandlerAdvance.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include "QuadraticFunction.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string ParQGHandlerAdvance::me_ = "ParQGHandler: ";


ParQGHandlerAdvance::ParQGHandlerAdvance(EnvPtr env, ProblemPtr minlp,
                                         EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  solC_(0),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  extraLin_(0),
  maxVioPer_(0),
  objVioMul_(0),
  nodeDep_(0),
  consDual_(0),
  cutMethod_("ecp"),
  lastNodeId_(-1)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new ParQGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->cuts = 0;
  stats_->rcuts = 0;
  stats_->fracCuts = 0;
}


ParQGHandlerAdvance::~ParQGHandlerAdvance()
{
  if (stats_) {
    delete stats_;
  }
  
  if (extraLin_) {
    delete extraLin_;  
    extraLin_ = 0;  
  } else {
    if (solC_) {
      delete [] solC_;
      solC_ = 0;
    }
  }

  env_ = 0;
  rel_ = 0;
  nlpe_ = 0;
  minlp_ = 0;
  unfixInts_();
}


void ParQGHandlerAdvance::dualBasedCons_(ConstSolutionPtr sol)
{
  double lambda1 = 0.05, lambda2 = 0.95;
  const double * consDual = sol->getDualOfCons();
  
  for (UInt i = 0; i < nlCons_.size(); ++i) {
    (consDual_)[i] = lambda1*((consDual_)[i]) + 
      lambda2*(consDual[(nlCons_[i])->getIndex()]);
  }

  return;
}


void ParQGHandlerAdvance::addInitLinearX_(const double *x)
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
        sstm << "_qgCutRoot_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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
      sstm << "_qgObjCutRoot_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
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
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<
        "Objective not defined at this point." << std::endl;
#endif
    }
  }
  return;
}


void ParQGHandlerAdvance::cutIntSol_(const double *lpx, CutManager *cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      ++(stats_->nlpF);
      double nlpval = nlpe_->getSolutionValue();
#pragma omp critical (solPool)
      {
        updateUb_(s_pool, nlpval, sol_found);
      }
      if ((relobj_ >= nlpval-objAbsTol_) ||
          (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRelTol_))) {
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
    objCutAtLpSol_(lpx, cutMan, status, 0);
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
  }

  // happens due to tolerance miss-match among different solvers
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


void ParQGHandlerAdvance::fixInts_(const double *x)
{
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd();
       ++vit) {
    v = *vit;
    switch (v->getType()) {
    case Binary:
    case Integer:
    case ImplBin:
    case ImplInt:
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5);
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp_);
      nlpMods_.push(m);
      break;
    default:
      break;
    }
  }
  return;
}


void ParQGHandlerAdvance::loadProbToEngine()
{
  nlpe_->load(minlp_);
}


void ParQGHandlerAdvance::initLinear_(bool *isInf)
{
  *isInf = false;
  const double *x;
  
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
    assert(!"In ParQGHandlerAdvance: stopped at root. Check error log.");
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = "
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}


bool ParQGHandlerAdvance::isFeasible(ConstSolutionPtr sol, RelaxationPtr,
                                     bool &, double &)
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
        "constraint not defined at this point."<< std::endl;
      return false;
    }
  }

  if (oNl_) {
    error = 0;
    relobj_ = sol->getObjValue();
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if ((act > relobj_ + objAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*objRelTol_))) {
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


void ParQGHandlerAdvance::setObjVar()
{
  ObjectivePtr o = minlp_->getObjective();
  FunctionType fType = o->getFunctionType();
  if (!o) {
    assert(!"need objective in QG!");
  } else if (fType != Linear && fType != Constant) {
    oNl_ = true;
    VariablePtr vPtr;
    for (VariableConstIterator viter=rel_->varsBegin(); viter!=rel_->varsEnd();
         ++viter) {
      vPtr = *viter;
      if (vPtr->getName() == "eta") {
        assert(o->getObjectiveType()==Minimize);
        objVar_ = vPtr;
        break;
      }
    }
  }
  return;
}


void ParQGHandlerAdvance::linearizeObj_()
{
  ObjectivePtr o = minlp_->getObjective();
  FunctionType fType = o->getFunctionType();
  if (!o) {
    assert(!"need objective in QG!");
  } else if (fType != Linear && fType != Constant) {
    oNl_ = true;
    FunctionPtr f;
    VariablePtr vPtr;
    ObjectiveType objType = o->getObjectiveType();
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    for (VariableConstIterator viter=rel_->varsBegin(); viter!=rel_->varsEnd();
         ++viter) {
      vPtr = *viter;
      if (vPtr->getName() == "eta") {
        assert(o->getObjectiveType()==Minimize);
        rel_->removeObjective();
        lf->addTerm(vPtr, 1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newObjective(f, 0.0, objType);
        objVar_ = vPtr;
        break;
      }
    }
  }
  return;
}


void ParQGHandlerAdvance::linearAt_(FunctionPtr f, double fval, const double *x,
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


void ParQGHandlerAdvance::cutToCons_(const double *nlpx, const double *lpx,
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
      logger_->msgStream(LogError) << me_ << "Constraint not defined at"
        << " this point. "<<  std::endl;
    }
  }
  return;
}


void ParQGHandlerAdvance::consCutsAtLpSol_(const double *lpx, CutManager *cutMan,
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


void ParQGHandlerAdvance::objCutAtLpSol_(const double *lpx, CutManager * cutman,
                                  SeparationStatus *status, bool fracNode)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    LinearFunctionPtr lf;
    double c, lpvio, act;
    ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      lpvio = std::max(act-relobj_, 0.0);
      if ((fracNode == 1) && (maxVioPer_ > 0) && (lpvio > objAbsTol_)) {
        if (fabs(relobj_) < objAbsTol_) {
          if (lpvio < objVioMul_) {
            return;
          }
        } else {
           if ((lpvio/fabs(relobj_)) < objVioMul_) {
            return;
          }       
        }
      }
      if ((lpvio > solAbsTol_) &&
          (relobj_ == 0 || lpvio > fabs(relobj_)*solRelTol_)) {
          lf = 0;
          f = o->getFunction();
          linearAt_(f, act, lpx, &c, &lf, &error);
          if (error == 0) {
            lpvio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((lpvio > solAbsTol_) && ((relobj_ -c == 0) || (lpvio > fabs(relobj_ - c)*solRelTol_))) {
              ++(stats_->cuts);
              *status = SepaResolve;
              lf->addTerm(objVar_, -1.0);
              f = (FunctionPtr) new Function(lf);
              sstm << "_qgObjCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
              newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                            -1.0*c, false,false);
              cut->setCons(newcon);
              cutman->addCutToPool(cut);
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


void ParQGHandlerAdvance::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager *cutman,
                        SeparationStatus *status)
{
  int error=0;
  ConstraintPtr newcon;
  std::stringstream sstm;
  double c, lpvio, act, cUb;
  FunctionPtr f = con->getFunction();
  LinearFunctionPtr lf = LinearFunctionPtr();

  act = con->getActivity(nlpx, &error);
  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error == 0) {
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if ((lpvio>solAbsTol_) &&
          ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
        ++(stats_->cuts);
        sstm << "_qgCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
        *status = SepaResolve;
        f = (FunctionPtr) new Function(lf);
        newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                      cUb-c, false,false);
        cut->setCons(newcon);
        cutman->addCutToPool(cut);
        return;
      } else {
        delete lf;
        lf = 0;
      }
    }
  }	else {
    logger_->msgStream(LogError) << me_ << "Constraint not defined at"
      << " this point. "<<  std::endl;
  }
  return;
}
 

void ParQGHandlerAdvance::cutToObj_(const double *nlpx, const double *lpx,
                            CutManager *cutman, SeparationStatus *status)
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
      if ((vio > solAbsTol_)
        && (relobj_ == 0 || vio > fabs(relobj_)*objRelTol_)) {
        act = o->eval(nlpx, &error);
        if (error == 0) {
          f = o->getFunction();
          LinearFunctionPtr lf = 0;
          linearAt_(f, act, nlpx, &c, &lf, &error);
          if (error == 0) {
            vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((vio > solAbsTol_) && ((relobj_ - c == 0)
                                     || (vio > fabs(relobj_ - c)*solRelTol_))) {
              ++(stats_->cuts);
              sstm << "_qgObjCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
              lf->addTerm(objVar_, -1.0);
              *status = SepaResolve;
              f = (FunctionPtr) new Function(lf);
              newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              CutPtr cut = (CutPtr) new Cut(rel_->getNumVars(),f, -INFINITY,
                                            -1.0*c, false,false);
              cut->setCons(newcon);
              cutman->addCutToPool(cut);
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


void ParQGHandlerAdvance::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}


void ParQGHandlerAdvance::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void ParQGHandlerAdvance::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void ParQGHandlerAdvance::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void ParQGHandlerAdvance::nlCons()
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
}


void ParQGHandlerAdvance::setRelaxation(RelaxationPtr rel)
{
  rel_ = rel;
}


void ParQGHandlerAdvance::relax_(bool *isInf)
{
  nlCons();
  linearizeObj_();
  initLinear_(isInf);
 
  if (*isInf) {
    return;
  }
 
  UInt numNlCons = nlCons_.size();
 
  if ((numNlCons > 0) || oNl_) {
    if (nlCons_.size() > 0) {
      nodeDep_ = 10;    
    } else {
      nodeDep_ = 5;    
    } 
    
    if (env_->getOptions()->findBool("extraCuts")->getValue()) {
      bool uniS = false;
      maxVioPer_ = env_->getOptions()->findDouble("maxVioPer")->getValue();
      double rs1 = env_->getOptions()->findDouble("root_linScheme1")->getValue();
      double rg2 = env_->getOptions()->findDouble("root_linGenScheme2_per")->getValue(); 
     
      if (rs1 || rg2) {
        extraLin_ = new Linearizations(env_, rel_, minlp_, nlCons_, objVar_,
                                       nlpe_->getSolution());
        if (rs1) {
          uniS = extraLin_->rootLinearizationsUniS();
        }

        if (uniS) {
          cutMethod_ = "ecp";
        } else {
          if (rg2) {
            cutMethod_ = env_->getOptions()->findString("cutMethod")->getValue();
            extraLin_->setNlpEngine(nlpe_->emptyCopy());        
            extraLin_->findCenter();
            solC_ = extraLin_->getCenter();
            if (solC_ == 0) {
              maxVioPer_ = 0;
            }
            extraLin_->rootLinearizationsGen();
          }
        }
        stats_->rcuts = extraLin_->getStats()->rs1Cuts + extraLin_->getStats()->rgs2Cuts;
      } 
      
      if (rg2 == 0 && maxVioPer_ && (cutMethod_ == "esh") && (numNlCons > 0)) {
        findCenter_();
        if (solC_ == 0) {
          maxVioPer_ = 0;
        } else {
          const double *nlpx = nlpe_->getSolution()->getPrimal();
          double l1 = 0.5, l2 = 0.5; 
          for (UInt i = 0 ; i < minlp_->getNumVars(); ++i) {
            solC_[i] = l1*nlpx[i] + l2*solC_[i];
          }
        }
      }

      if (maxVioPer_ && (numNlCons > 0)) {
        (consDual_).resize(numNlCons, 0);
        dualBasedCons_(nlpe_->getSolution());
      }
    }
  }

  return;
}


void ParQGHandlerAdvance::solveCenterNLP_(EnginePtr nlpe)
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


void ParQGHandlerAdvance::findCenter_()
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


void ParQGHandlerAdvance::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr,
                         CutManager *cutMan, SolutionPoolPtr s_pool,
                         ModVector &, ModVector &, bool *sol_found,
                         SeparationStatus *status)
{
  const double *x = sol->getPrimal();
 
  if ((node->getId() == 0) && oNl_ && maxVioPer_) {
    int error = 0;
    double act, lpvio;
       
    relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
    ObjectivePtr o = minlp_->getObjective();
    act = o->eval(x, &error);
    objVioMul_ = maxVioPer_;
    if (error == 0) {
      lpvio = std::max(act-relobj_, 0.0);
      if (fabs(relobj_) > solAbsTol_) {
        objVioMul_ = lpvio/fabs(relobj_);
      } else {
        objVioMul_ = lpvio;
      }
      if (nlCons_.size() == 0 && oNl_) {
        if (objVioMul_ > 1000) {
          nodeDep_ = floor(nodeDep_/2);
        } else if (objVioMul_ < 0.5) {
          nodeDep_ = nodeDep_*2;   
          objVioMul_ = 2*(objVioMul_);        
        }
      } else {
        if (objVioMul_ < 0.5) {
          objVioMul_ = 2*(objVioMul_);        
        }    
      }
    }
  }

  *status = SepaContinue;

  bool isIntFeas = isIntFeas_(x);
  if (isIntFeas) {
    fixInts_(x);            // Fix integer variables
    relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
# pragma omp critical (fixedNLPSolve)
    {
      solveNLP_();            // solve fixed NLP
    }
    unfixInts_();            // Unfix integer variables
    cutIntSol_(x, cutMan, s_pool, sol_found, status);
  } else {
     if (maxVioPer_) {
       relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
       maxVio_(sol, node, cutMan, status); // maxViolation
    } 
  }
  return;
}


void ParQGHandlerAdvance::ESHTypeCut_(const double *x, CutManager *cutMan)
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


void ParQGHandlerAdvance::genLin_(const double *x, std::vector<UInt > vioCons,
                               CutManager *cutman)
{
  int error;
  FunctionPtr f;
  double c, cUb, act;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  ConstraintPtr con, newcon;

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
        sstm << "_qgCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
        newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                      cUb-c, false,false);
        cut->setCons(newcon);
        cutman->addCutToPool(cut);
        sstm.str("");
      } 
    }
  }

  if (oNl_) {
    SeparationStatus s = SepaContinue;
    objCutAtLpSol_(x, cutman, &s, 1);
  }
  return;
}


bool ParQGHandlerAdvance::boundaryPtForCons_(double* xnew, const double *xOut,
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


void ParQGHandlerAdvance::maxVio_(ConstSolutionPtr sol, NodePtr node,
                               CutManager *cutMan, SeparationStatus *status)
{
  int error = 0;
  ConstraintPtr c;
  std::vector<double > consAct;
  const double *x = sol->getPrimal();
  double act, cUb, vio, totScore = 0, parentScore, incr; 
  UInt i = 0, vioConsNum = 0, nodeId = node->getId(), temp = stats_->cuts;

  if (node->getDepth() >= nodeDep_ && stats_->fracCuts > 0) {
    return;  
  } 

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
        if (fabs(totScore) >= (maxVioPer_* fabs(parentScore + .001)/100)) { 
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
              objCutAtLpSol_(x, cutMan, &s, 1);
            }
          } else if (cutMethod_ == "esh") {
            ESHTypeCut_(x, cutMan);
          }
        }
        if (parentScore > solAbsTol_) {
          incr = std::max(maxVioPer_, (totScore/(parentScore+0.001)));
          if (incr > maxVioPer_) {
# pragma omp critical (maxVioParam)
            maxVioPer_ = ((maxVioPer_ + incr)/2);
          }
        }
      }
    }
  } else if (oNl_) {
    SeparationStatus s = SepaContinue;
    objCutAtLpSol_(x, cutMan, &s, 1);
  }

  stats_->fracCuts = stats_->fracCuts + (stats_->cuts - temp);

  if ((temp < stats_->cuts) && (nodeId != UInt(lastNodeId_))) {
    *status = SepaResolve;
  }

  lastNodeId_ = nodeId;
  return;
}


bool ParQGHandlerAdvance::isIntFeas_(const double* x)
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


void ParQGHandlerAdvance::ECPTypeCut_(const double *lpx, CutManager *cutman, ConstraintPtr con, double act)
{
  int error = 0;
  std::stringstream sstm;
  ConstraintPtr newcon;
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
      sstm << "_qgCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
      f = (FunctionPtr) new Function(lf);
      newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                    cUb-c, false,false);
      cut->setCons(newcon);
      cutman->addCutToPool(cut);
      return;
    } else {
      delete lf;
      lf = 0;
    }
  }

  return;
}


void ParQGHandlerAdvance::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  return;
}


void ParQGHandlerAdvance::unfixInts_()
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


void ParQGHandlerAdvance::updateUb_(SolutionPoolPtr s_pool, double nlpval,
                          bool *sol_found)
{
  double bestval = s_pool->getBestSolutionValue();
  if ((bestval - objAbsTol_ > nlpval) ||
        (bestval != 0 && (bestval - fabs(bestval)*objRelTol_ > nlpval))) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, nlpval);
    *sol_found = true;

    if ((maxVioPer_ > 0) && (nlCons_.size() > 0)) {
      dualBasedCons_(nlpe_->getSolution());
    }
  }
  return;
}


void ParQGHandlerAdvance::writeStats(std::ostream &out) const
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
    << me_ << "number of frac cuts added                   = " 
    << stats_->fracCuts << std::endl
    << me_ << "number of cuts added                        = " 
    << stats_->cuts << std::endl;
  return;
}


std::string ParQGHandlerAdvance::getName() const
{
  return "ParQGHandlerAdvance (Quesada-Grossmann)";
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
