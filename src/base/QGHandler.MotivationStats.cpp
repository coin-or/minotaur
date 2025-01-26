// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/** 
 * \file QGHandler.cpp
 * \Briefly define a handler for the textbook type Quesada-Grossmann
 * Algorithm. All motivations of the linearization schemes.
 * \Authors Ashutosh Mahajan and Meenarli Sharma, Indian Institute of
 * Technology Bombay
 */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MinotaurConfig.h"

#include "CGraph.h"
#include "CNode.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
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

QGHandler::QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  inactiveCutsAtRoot_(0),
  lastNodeId_(-1),
  intNodes_(0),
  fracNodes_(0),
  solNLP_(NULL),
  nlpObjVal_(-INFINITY),
  lpDist_(INFINITY),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new QGStats();
  stats_->inactiveCuts = 0;
  stats_->cutsAtRoot = 0;
  stats_->cuts = 0;
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;

  vioInt_ = new vioInterval();
  vioInt_->zero = 0;
  vioInt_->first = 0;
  vioInt_->second = 0;
  vioInt_->third = 0;
  vioInt_->fourth = 0;
  vioInt_->fifth = 0;
  vioInt_->all = 0;
}


QGHandler::~QGHandler()
{ 
  if (stats_) {
    delete stats_;
  }

  if (vioInt_) {
    delete vioInt_;
  }
  if (solNLP_) {
    delete [] solNLP_;
  }
  env_ = 0;
  rel_ = 0;
  minlp_ = 0;
  nlCons_.clear();
}

// 1 if contraint either has no linear part of linear part that have some
// variables that are not there in the nonlinear part
bool QGHandler::isLinPart_(ConstraintPtr con)
{
  VariablePtr var;
  LinearFunctionPtr lf = con->getLinearFunction();
  QuadraticFunctionPtr qf = con->getQuadraticFunction();
  NonlinearFunctionPtr nlf = con->getNonlinearFunction();
  
  if (lf) {
    for(VariableGroupConstIterator vit = lf->termsBegin();
        vit != lf->termsEnd(); ++vit) {
      var = vit->first;
      if (nlf && qf) {
        if (!(qf->hasVar(var)) && !(nlf->hasVar(var))) {
          return true;
        }
      } else if (nlf) {
         if (!(nlf->hasVar(var))) {
          return true;
        }     
      } else {
          if (!(qf->hasVar(var))) {
          return true;
        }    
      }
    }
  }
  return false;
}

void QGHandler::addInitLinearX_(const double *x)
{ 
  int error=0;
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con;
  //ConstraintPtr newcon;
  LinearFunctionPtr lf = LinearFunctionPtr();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        cUb = con->getUb();
        if (cUb-act < solAbsTol_ || 
            (cUb!=0 && cUb-act < fabs(cUb)*solRelTol_)) {
          // cons inactive at nlp sol and does not have a linear part
          if (!(isLinPart_(con))) {
            ++(stats_->inactiveCuts);
          }
        }
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

  if (stats_->cuts > 0) {
    inactiveCutsAtRoot_ = stats_->inactiveCuts;
    stats_->inactiveCuts = 0;
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

  stats_->cutsAtRoot = stats_->cuts;
  return;
}


void QGHandler::cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  const double *lpx = sol->getPrimal();
  relobj_ = (sol) ? sol->getObjValue() : -INFINITY;

  fixInts_(lpx);           // Fix integer variables
  solveNLP_();
  unfixInts_();            // Unfix integer variables
  
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
    cutsAtLpSol_(lpx, cutMan, status);
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

 if (*status == SepaContinue) {
   // happens due to tolerance. No linearizations are generated so prune the node
   *status = SepaPrune;
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
  
  nlpe_->load(minlp_);
  solveNLP_();
 
  rootNLPStatus_ = nlpStatus_;

  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      ++(stats_->nlpF);
      x = nlpe_->getSolution()->getPrimal();
      solNLP_ = new double[minlp_->getNumVars()];
      std::copy(x, x+minlp_->getNumVars(), solNLP_);
      relobj_ = nlpe_->getSolution()->getObjValue();
      nlpObjVal_ = nlpe_->getSolution()->getObjValue();
      addInitLinearX_(x);
    }
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
    assert(!"In QGHandler: stopped at root. Check error log.");
    break;
  }
  return;
}


bool QGHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
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


void QGHandler::linearizeObj_()
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
    objVar_ = vPtr;
    assert(objType == Minimize);
    rel_->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel_->newObjective(f, 0.0, objType);
  }
  return;
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
    logger_->msgStream(LogError) << me_ <<"gradient not defined at this point."
      << std::endl;
  }
  delete [] a;
  return;
}


void QGHandler::cutToCons_(const double *nlpx, const double *lpx,
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


void QGHandler::cutsAtLpSol_(const double *lpx, CutManager *,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  ConstraintPtr con;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  //ConstraintPtr newcon;
  double c, lpvio, act, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    lf = 0;
    con = *it;
    f = con->getFunction();
    act =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb+fabs(cUb)*solRelTol_)) {
        linearAt_(f, act, lpx, &c, &lf, &error);
        if (error == 0) {
          lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
          if ((lpvio > solAbsTol_) && ((cUb-c)==0 ||
                                   (lpvio>fabs(cUb-c)*solRelTol_))) {

            if (fabs(cUb-act) > solAbsTol_ && 
                (cUb ==0 || fabs(cUb-act) > fabs(cUb)*solRelTol_)) {
              // cons inactive at nlp sol and does not have a linear part
              if (!(isLinPart_(con))) {
                ++(stats_->inactiveCuts);
              }
            }
            ++(stats_->cuts);
            *status = SepaResolve;
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
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
    }
  }

  if (oNl_) {
    error = 0;
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


void QGHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status)
{
  int error=0;
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
      if (fabs(cUb-act) > solAbsTol_ && 
          (cUb ==0 || fabs(cUb-act) > fabs(cUb)*solRelTol_)) {
        // cons inactive at nlp sol and does not have a linear part
        if (!(isLinPart_(con))) {
          ++(stats_->inactiveCuts);
        }
      }
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
  

void QGHandler::cutToObj_(const double *nlpx, const double *lpx,
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


void QGHandler::relax_(bool *isInf)
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
  return;
}

void QGHandler::shortestDist_(ConstSolutionPtr sol)
{
  EnginePtr nlpe = nlpe_->emptyCopy(); //Engine for modified problem
  ProblemPtr inst_C = minlp_->clone();
  UInt n = minlp_->getNumVars();
  std::vector<CNode *> temp;
  CNode * n1, * n2, * nf;
  VariablePtr v;
  const double *x = sol->getPrimal();
  EngineStatus nlpStatus;
  //inst_C->write(std::cout);
  //exit(1);

  CGraphPtr t = (CGraphPtr) new CGraph();
  ObjectivePtr o = inst_C->getObjective();
  ObjectiveType objType = o->getObjectiveType();
    
  for (VariableConstIterator v_iter=inst_C->varsBegin(); 
      v_iter!=inst_C->varsEnd(); ++v_iter) {
    v = *v_iter;
    v->setFunType_(Nonlinear);
    n1 = t->newNode(v);
    n2 = t->newNode(x[v->getIndex()]);
    nf = t->newNode(Minotaur::OpMinus, n1, n2);
    n2 = t->newNode(2);
    n1 = t->newNode(Minotaur::OpPowK, nf, n2);
    temp.push_back(n1);
  }
  CNode **childr = new Minotaur::CNode *[n];
  for (UInt i = 0; i < n; ++i) {
    childr[i]=temp[i];
  }
  nf = t->newNode(Minotaur::OpSumList,childr, n);
  n1 = t->newNode(Minotaur::OpSqrt, nf, 0);
  t->setOut(n1);
  t->finalize();
  
  inst_C->removeObjective();
  FunctionPtr f = (FunctionPtr) new Function(t);
  inst_C->newObjective(f, 0.0, objType);
  inst_C->write(std::cout);
  //exit(1);
  //inst_C->prepareForSolve();
  nlpe->load(inst_C);
  nlpStatus = nlpe->solve();

  sepNLPStatus_ = nlpStatus;

  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    lpDist_ = nlpe->getSolution()->getObjValue();
    break;
  case (EngineIterationLimit):
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
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
      << nlpe->getStatusString() << std::endl;
    std::cout << "Shortest distance of LP sol from feasible region couldn't find" << std::endl;
    break;
  }
  
  //for (UInt i = 0; i < n; ++i) {
    //delete tempN[i];
  //}
  delete nlpe;
  nlpe = 0;
  delete inst_C;
  inst_C = 0;
  delete [] childr;
  tempN.clear();
  return;
}


void QGHandler::rootLPVio_(ConstSolutionPtr sol)
{
  int error = 0;
  ConstraintPtr c;
  double act, cUb, vioPer;
  const double *x = sol->getPrimal();
  rootMaxVio_ = -INFINITY, rootMinVio_ = INFINITY, rootAvgVio_ = 0;

  if (nlCons_.size() == 0) {
    return;
  } else {
    for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
      c = *it;
      act = c->getActivity(x, &error);
      if (error == 0) {
        cUb = c->getUb();
        if ((act > cUb + solAbsTol_) &&
            (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
          if (cUb != 0) {
            vioPer = 100*(act - cUb)/fabs(cUb);          
          } else {
            vioPer = act;          
          }
          if (vioPer < rootMinVio_) {
            rootMinVio_= vioPer;
          }
          if (vioPer > rootMaxVio_) {
            rootMaxVio_= vioPer;        
          }
          rootAvgVio_= rootAvgVio_ + vioPer;
        }
      }	
    }
    rootAvgVio_= rootAvgVio_/nlCons_.size();
  }

  rootObjVio_ = INFINITY;
  if (oNl_) {
    error = 0;
    relobj_ = sol->getObjValue();
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if ((act > relobj_ + solAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
        if (relobj_ != 0) {
          vioPer = 100*(act - relobj_)/fabs(relobj_);          
        } else {
          vioPer = act;          
        }
        rootObjVio_ = vioPer;
      }
    }	
  }
  return;
}

void QGHandler::consVio_(ConstSolutionPtr sol)
{
  if (nlCons_.size() == 0) {
    return;  
  }

  int error = 0, numVio = 0;
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
        ++numVio;
      }
    }
  }
  if (oNl_) {
    error = 0;
    relobj_ = sol->getObjValue();
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if ((act > relobj_ + solAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
        ++numVio;
      }
      numVio = 100*numVio/(1+nlCons_.size());
    } else {
      numVio = 100*numVio/nlCons_.size();
    }
  } else {
    numVio = 100*numVio/nlCons_.size();
  }

  if (numVio == 0) {
    ++vioInt_->zero;  
  } else if (numVio > 0 && numVio < 20) {
    ++vioInt_->first;  
  } else if (numVio >= 20 && numVio < 40) {
    ++vioInt_->second;  
  } else if (numVio >= 40 && numVio < 60) {
    ++vioInt_->third;  
  } else if (numVio >= 60 && numVio < 80) {
    ++vioInt_->fourth;  
  } else if (numVio >= 80 && numVio < 100) {
    ++vioInt_->fifth;  
  } else {
    ++vioInt_->all;  
  }
  return;  
}


void QGHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                         CutManager *cutMan, SolutionPoolPtr s_pool,
                         ModVector &, ModVector &, bool *sol_found,
                         SeparationStatus *status)
{
  double val;
  VariablePtr var;
  VariableType v_type;
  const double *x = sol->getPrimal();

  if (node->getId() == 0) {
    shortestDist_(sol);
    rootLPVio_(sol);  
  }

  *status = SepaContinue;
  for (VariableConstIterator v_iter = rel->varsBegin();
       v_iter != rel->varsEnd(); ++v_iter) {
    var = *v_iter;
    v_type = var->getType();
    if (v_type == Binary || v_type == Integer) {
      val = x[var->getIndex()];
      if (fabs(val - floor(val+0.5)) > intTol_) {
        if (node->getId() != lastNodeId_) {
          ++fracNodes_;
          lastNodeId_ = node->getId();
          consVio_(sol);
        }
        return;
      }
    }
  }
  if (node->getId() != lastNodeId_) {
    ++intNodes_;
    lastNodeId_ = node->getId();
  }
  cutIntSol_(sol, cutMan, s_pool, sol_found, status);
  return;
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

  while(nlpMods_.empty() == false) {
    m = nlpMods_.top();
    m->undoToProblem(minlp_);
    nlpMods_.pop();
    delete m;
  }
  return;
}


void QGHandler::updateUb_(SolutionPoolPtr s_pool, double nlpval,
                          bool *sol_found)
{
  double bestval = s_pool->getBestSolutionValue();

  if ((bestval - objATol_ > nlpval) ||
        (bestval != 0 && (bestval - fabs(bestval)*objRTol_ > nlpval))) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, nlpval);
    *sol_found = true;
  }
  return;
}


void QGHandler::writeStats(std::ostream &out) const
{
  VariablePtr v;
  UInt intr = 0, bin = 0, iInt = 0, iBin = 0, cont = 0;

  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd();
       ++vit) {
    v = *vit;
    if (v->getType() == Binary ) {
      ++bin;
    } else if (v->getType() == Integer) {
      ++intr;
    } else if (v->getType() == ImplBin) {
      ++iBin;
    } else if (v->getType() == ImplInt) {
      ++iInt;
    } else {
      ++cont;
    }
  }

  std::cout <<"\nProb type, no. of bin, int, implBin, implInt, cont vars, nl cons; obj nonlinear?; " 
    << getProblemTypeString(minlp_->findType()) << " ; " << bin << " ; " 
    << intr << " ; " << iBin << " ; " << iInt << " ; " 
    << cont << " ; " << nlCons_.size() << " ; " << oNl_ << std::endl;

  std::cout << "\nRoot and shortest dist NLP solve status and shortest distance; " 
    << rootNLPStatus_ << " ; " << sepNLPStatus_ << " ; " 
    << std::setprecision(6) << lpDist_ << std::endl;

  std::cout << "\nRoot LP per cons vio, min ; max ; avg ; obj vio; " 
    << std::setprecision(6) << rootMinVio_ << " ; " << rootMaxVio_ 
    << " ; " << rootAvgVio_ << " ; " << rootObjVio_ << std::endl;        

    std::cout << "\nNo. of total cuts, cuts at root, inactive cuts at root and rest of " << 
    "the tree; " << stats_->cuts << " ; " << stats_->cutsAtRoot 
    << " ; " << inactiveCutsAtRoot_ << " ; " << stats_->inactiveCuts << std::endl;

    std::cout << "\nNo. of Frac and int nodes; " 
    << std::setprecision(0) << fracNodes_ << " ; " << intNodes_ << std::endl;

  if (nlCons_.size() != 0 || oNl_) {
    UInt check = vioInt_->zero + vioInt_->first + vioInt_->second + vioInt_->third 
      + vioInt_->fourth + vioInt_->fifth + vioInt_->all ;
    if (check == fracNodes_) {

      double i0 = 100*vioInt_->zero/fracNodes_, i1 = 100*vioInt_->first/fracNodes_,
            i2 = 100*vioInt_->second/fracNodes_, i3 = 100*vioInt_->third/fracNodes_,
           i4 = 100*vioInt_->fourth/fracNodes_, i5 = 100*vioInt_->fifth/fracNodes_,
          iall = 100*vioInt_->all/fracNodes_;


      std::cout << "\nCheck: passed, con/obj vio brackets in frac nodes; " << i0 << " ; " << i1 << " ; " << i2 
        << " ; " << i3 << " ; " << i4 << " ; " << i5 << " ; " << iall << std::endl;

    } else {
      std::cout << "\nCheck: failed" << std::endl;
    }
  } else {
    std::cout << "\nCheck: passed" << std::endl;
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
    << me_ << "number of cuts added                        = " 
    << stats_->cuts << std::endl;
  std::cout << std::endl;

  return;
}


std::string QGHandler::getName() const
{
  return "QG Handler (Quesada-Grossmann)";
}

