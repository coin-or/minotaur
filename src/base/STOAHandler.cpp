// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/** 
 * \file STOAHandler.cpp
 * \Briefly define a handler for the textbook type Quesada-Grossmann
 * Algorithm.
 * \Authors Meenarli Sharma and Prashant Palkar, Indian Institute of
 * Technology Bombay 
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
//#include "LPEngine.h"
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
#include "STOAHandler.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include "QuadraticFunction.h"

#include<omp.h>
//#define SPEW 1
#define PRINT 0

using namespace Minotaur;
typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string STOAHandler::me_ = "STOAHandler: ";


STOAHandler::STOAHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe,
                         MILPEnginePtr milpe, SolutionPoolPtr solPool)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  milpe_(milpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  solPool_(solPool),
  //relobj_(0.0),
  numCalls_(0),
  cbtime_(0)
{
  timer_ = env->getNewTimer();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  npATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  npRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_   = new STOAStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->cuts = 0;
}


STOAHandler::~STOAHandler()
{ 
  if (stats_) {
    delete stats_;
  }

  if (timer_) {
    delete timer_;
  }
  env_ = 0;
  rel_ = 0;
  nlpe_ = 0;
  minlp_ = 0;
}


void printx(double *x, UInt size) {
  for (UInt i=0; i < size; i++) {
    std::cout << i+1 << " " << x[i] << "\n";
  }
}


void printx(const double *x, UInt size) {
  for (UInt i=0; i < size; i++) {
    std::cout << i+1 << " " << x[i] << "\n";
  }
}


void STOAHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, double* rhs,
                             std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff)
{
  int error = 0;
  LinearFunctionPtr lf = 0; 
  double c, lpvio, act, cUb;
  FunctionPtr f = con->getFunction();

  act = con->getActivity(nlpx, &error); 
  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error==0) { 
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if ((lpvio > solAbsTol_) &&
          ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
        ++(stats_->cuts);
        *rhs = cUb-c;
        for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
       ++it) {
          (*varIdx).push_back(it->first->getIndex());
          (*varCoeff).push_back(it->second);
        }
        //MS: not updating rel_ with new linearizations.
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

void STOAHandler::addInitLinearX_(const double *x)
{ 
  int error=0;
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con;
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
        sstm << "_STOACut_" << stats_->cuts << "_AtRoot";
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
      sstm << "_STOAObjCut_" << stats_->cuts << "_AtRoot";
      f = o->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      }
    }	else {
      logger_->msgStream(LogError) << me_ <<
        "Objective is not defined at this point" << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<
        "Objective is not defined at this point" << std::endl;
#endif
    }
  }
  return;
}


//bool STOAHandler::fixedNLP(const double *lpx, const double *)
//{
  //numCalls_++;
  //newUb_ = INFINITY;

  //fixInts_(lpx);           // Fix integer variables
  //solveNLP_();
  //unfixInts_();             // Unfix integer variables

  //switch(nlpStatus_) {
  //case (ProvenOptimal):
  //case (ProvenLocalOptimal):
    //++(stats_->nlpF);
    //newUb_ = nlpe_->getSolutionValue();
    //solPool_->addSolution(nlpe_->getSolution());
    //nlpe_->getSolution()->getPrimal();
    //return true;
    //break;
  //case (ProvenInfeasible):
  //case (ProvenLocalInfeasible): 
  //case (ProvenObjectiveCutOff):
    //++(stats_->nlpI);
    //return false;
    //break;
  //case (EngineIterationLimit):
    //++(stats_->nlpIL);
    //return false;
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
      //<< nlpe_->getStatusString() << std::endl;
    //logger_->msgStream(LogError)<< me_ << "No cut generated, may cycle!"
      //<< std::endl;
    //return false;
  //}
  //return true;
//}


bool STOAHandler::fixedNLP(const double *lpx, EnginePtr &nlpe)
{
  numCalls_++;
  //MS: Verify before using newUb_
  newUb_ = INFINITY;
  EngineStatus nlpStatus;
  std::stack<Modification *> nlpMods;
  ProblemPtr minlp = minlp_->clone();
  nlpe = nlpe_->emptyCopy();

  fixInts_(lpx, minlp, nlpMods);           // Fix integer variables
  nlpe->load(minlp);
  nlpStatus = nlpe->solve();
  ++(stats_->nlpS);
  unfixInts_(minlp, nlpMods);             // Unfix integer variables

  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    newUb_ = nlpe->getSolutionValue();
    solPool_->addSolution(nlpe->getSolution());
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible):
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
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
    return false;
  }
  return true;
}


void STOAHandler::OACutToObj(const double *lpx, double* rhs,
                             std::vector<UInt> *varIdx,
                             std::vector<double>* varCoeff, double ub, EnginePtr nlpe)
{
  EngineStatus nlpStatus = nlpe->getStatus();
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      const double * nlpx = nlpe->getSolution()->getPrimal();
      cutToObj_(nlpx, lpx, rhs, varIdx, varCoeff, ub);
    }
    break;
  case (EngineIterationLimit):
    objCutAtLpSol_(lpx, rhs, varIdx, varCoeff, ub);
    break;
  default:
    break;
  }
}


void STOAHandler::OACutToCons(const double *lpx, ConstraintPtr con,
                              double* rhs, std::vector<UInt> *varIdx,
                              std::vector<double>* varCoeff, EnginePtr nlpe)
{
  EngineStatus nlpStatus = nlpe->getStatus();
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    {
      const double * nlpx = nlpe->getSolution()->getPrimal();
      cutToCons_(con, nlpx, lpx, rhs, varIdx, varCoeff);
    }
    break;
  case (EngineIterationLimit):
    consCutAtLpSol_(con, lpx, rhs, varIdx, varCoeff);
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
    break;
  }
  return;
}


void STOAHandler::fixInts_(const double *x, ProblemPtr minlp, std::stack<Modification *> nlpMods)
{
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp->varsBegin(); vit!=minlp->varsEnd();
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5); 
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp);
      nlpMods.push(m);
    }
  }
  return;
}

void STOAHandler::solveMILP(double* objfLb, ConstSolutionPtr* sol, 
                          SolutionPoolPtr, CutManager*)
{
  milpe_->load(rel_);         //link directly to CPLEX (already loading in NodeRelaxer, remove initial double loading!)
  EngineStatus lpStatus = milpe_->solve();
  switch (lpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    *sol = milpe_->getSolution();
    (*objfLb) = (*sol)->getObjValue();
    break;
  case (EngineIterationLimit): // MS: take care of this.
    *sol = milpe_->getSolution();
    (*objfLb) = (*sol)->getObjValue();
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    logger_->msgStream(LogError) << me_ << "MILP engine status at root= " 
      << lpStatus << std::endl;
    assert(!"In STOAHandler: MILP infeasible. Check error log.");
    break;
  case (ProvenUnbounded):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "MILP engine status= " 
      << lpStatus << std::endl;
    assert(!"In STOAHandler: stopped. Check error log.");
    break;
  }
  return;
}


void STOAHandler::initLinear_(bool *isInf)
{
  *isInf = false;
  const double *x;
  nlpe_->load(minlp_);
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  //solveNLP_();
 
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
    logger_->msgStream(LogError) << me_ << "NLP engine status at root = "
      << nlpStatus_ << std::endl;
    assert(!"In STOAHandler: stopped at root. Check error log.");
    break;
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = "
    << nlpe_->getStatusString() << std::endl;
#endif
#if PRINT
  std::cout << "\nin lstoa: orig minlp\n";
  minlp_->write(std::cout);
  nlpe_->getSolution()->writePrimal(std::cout);
  std::cout << "\nin lstoa: rel\n";
  rel_->write(std::cout);
#endif
 
  return;
}


//bool STOAHandler::isFeas(const double *x)
//{
  //int error = 0;
  //double act, cUb;
  //ConstraintPtr c;

  //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) { 
    //c = *it;
    //act = c->getActivity(x, &error);
    //if (error==0) {
      //cUb = c->getUb();
      //if ((act > cUb + solAbsTol_) &&
          //(cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        //return false;     
      //} 
    //} else {
      //logger_->msgStream(LogError) << me_ << c->getName() <<
        //"constraint is not defined at this point."<< std::endl;
      //return false;
    //}
  //}
 
  //if (oNl_) {
    //error = 0;
    //relobj_ = x[objVar_->getIndex()];
    //act = minlp_->getObjValue(x, &error);
    //if (error == 0) {
      //if ((act > relobj_ + solAbsTol_) &&
          //(relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
        //return false;
      //}
    //}	else {
      //logger_->msgStream(LogError) << me_
        //<<"objective not defined at this point."<< std::endl;
      //return false;
    //}
  //}
  //return true;
//}


void STOAHandler::linearizeObj_()
{
  ObjectivePtr o = minlp_->getObjective();
  FunctionType fType = o->getFunctionType();
  if (!o) {
    assert(!"no objective function!");
  } else if (fType != Linear && fType != Constant) {
    oNl_ = true;
    FunctionPtr f;
    std::string name = "eta";
    ObjectiveType objType = o->getObjectiveType();
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    VariablePtr vPtr = rel_->newVariable(-INFINITY, INFINITY, Continuous,
                                         name, VarHand);
    objVar_ = vPtr;
    assert(objType == Minimize);
    rel_->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel_->newObjective(f, 0.0, objType);
  }
  return;
}


void STOAHandler::linearAt_(FunctionPtr f, double fval, const double *x,
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


void STOAHandler::cutToCons_(ConstraintPtr con, const double *nlpx,
                             const double *lpx, double* rhs,
                             std::vector<UInt> *varIdx,
                             std::vector<double>* varCoeff)
{
  int error = 0;
  double act, cUb;

  act =  con->getActivity(lpx, &error);
  if (error == 0) {
    cUb = con->getUb();
    if ((act > cUb + solAbsTol_) &&
        (cUb == 0 || act > cUb+fabs(cUb)*solRelTol_)) {
      addCut_(nlpx, lpx, con, rhs, varIdx, varCoeff);
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
  return;
}
 

void STOAHandler::objCutAtLpSol_(const double *lpx, double* rhs,
                             std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff, double relobj)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    double c, act, lpvio;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      if ((act > relobj + solAbsTol_) &&
          (relobj == 0 || (act > relobj + fabs(relobj)*solRelTol_))) {
        f = o->getFunction();
        LinearFunctionPtr lf = 0;
        linearAt_(f, act, lpx, &c, &lf, &error);
        if (error == 0) {
          lpvio = std::max(lf->eval(lpx)-relobj+c, 0.0);
          if ((lpvio > solAbsTol_) &&
            ((relobj-c) == 0 || (lpvio>fabs(relobj-c)*solRelTol_))) {
            ++(stats_->cuts);
            *rhs = -1.0*c;
            for (VariableGroupConstIterator it=lf->termsBegin(); 
                 it!=lf->termsEnd();
                 ++it) {
              (*varIdx).push_back(it->first->getIndex());
              (*varCoeff).push_back(it->first->getIndex());
            }
            (*varIdx).push_back(objVar_->getIndex());
            (*varCoeff).push_back(-1.0);
          } else {
            delete lf;
            lf = 0;          
          }
        }
      }
    }	else {
      logger_->msgStream(LogError) << me_
        << "objective is not defined at this solution point." << std::endl;
    }
  }
  return;
}


void STOAHandler::consCutAtLpSol_(ConstraintPtr con, const double *lpx,
                                  double* rhs, std::vector<UInt> *varIdx,
                                  std::vector<double>* varCoeff)
{
  int error = 0;
  LinearFunctionPtr lf = 0;
  double c, nlpact, cUb, lpvio;
  FunctionPtr f = con->getFunction();

  nlpact =  con->getActivity(lpx, &error);
  if (error == 0) {
    cUb = con->getUb();
    if ((nlpact > cUb + solAbsTol_) &&
        (cUb == 0 || nlpact > cUb+fabs(cUb)*solRelTol_)) {
      linearAt_(f, nlpact, lpx, &c, &lf, &error);
      if (error == 0) {
        lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
        if ((lpvio > solAbsTol_) &&
            ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
          ++(stats_->cuts);
          *rhs = cUb-c;
          for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
               ++it) {
            (*varIdx).push_back(it->first->getIndex());
            (*varCoeff).push_back(it->second);
          }
          return;
        } else {
          delete lf;
          lf = 0;        
        }
      }
    }
  }	else {
    logger_->msgStream(LogError) << me_ << " constraint is not defined at" <<
      " this point. "<<  std::endl;
  }
  return;
}


double STOAHandler::newUb(std::vector<UInt> *varIdx,
                        std::vector<double>* varVal)
{
  int i =0;
  double val = nlpe_->getSolutionValue();
  const double *x = nlpe_->getSolution()->getPrimal();
  for (VariableConstIterator v = minlp_->varsBegin(); v != minlp_->varsEnd();
       ++v, ++i) {
    (*varIdx).push_back((*v)->getIndex());
    (*varVal).push_back(x[i]);
  }
  if (oNl_) {
    (*varIdx).push_back(objVar_->getIndex());
    (*varVal).push_back(val);
  }
  //MS: check how the auxiliarly var is stored in x and at what position.
  return newUb_;
}


void STOAHandler::cutToObj_(const double *nlpx, const double *lpx,
                            double* rhs, std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff, double relobj)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    double c, vio, act;
    ObjectivePtr o = minlp_->getObjective();
    
    act = o->eval(lpx, &error);
    if (error == 0) {
      vio = std::max(act-relobj, 0.0);
      if ((vio > solAbsTol_)
        && (relobj == 0 || vio > fabs(relobj)*solRelTol_)) {
        act = o->eval(nlpx, &error);
        if (error == 0) {
          f = o->getFunction();
          LinearFunctionPtr lf = 0; 
          linearAt_(f, act, nlpx, &c, &lf, &error);
          if (error == 0) {
            vio = std::max(c+lf->eval(lpx)-relobj, 0.0);
            if ((vio > solAbsTol_) && ((relobj-c) == 0
                                     || vio > fabs(relobj-c)*solRelTol_)) {
              ++(stats_->cuts);
              *rhs = -1.0*c;
              for (VariableGroupConstIterator it=lf->termsBegin();
                   it!=lf->termsEnd();
                   ++it) {
                (*varIdx).push_back(it->first->getIndex());
                (*varCoeff).push_back(it->second);
              }
              (*varIdx).push_back(objVar_->getIndex());
              (*varCoeff).push_back(-1.0);
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


void STOAHandler::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void STOAHandler::relax_(bool *isInf)
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


//void STOAHandler::solveNLP_()
//{
  //nlpStatus_ = nlpe_->solve();
  //++(stats_->nlpS);
  //return;
//}


void STOAHandler::unfixInts_(ProblemPtr minlp, std::stack<Modification *> nlpMods)
{
  Modification *m = 0;
  while(nlpMods.empty() == false) {
    m = nlpMods.top();
    m->undoToProblem(minlp);
    nlpMods.pop();
    delete m;
  }
  return;
}


void STOAHandler::writeStats(std::ostream &out) const
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
    << me_ << "number of cuts added                           = "
    << stats_->cuts << std::endl;
  return;
}


std::string STOAHandler::getName() const
{
  return "STOA Handler (Single Tree Outer-approximation)";
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
