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
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  extraLin_(0),
  maxVioPer_(0),
  maxDist_(0),
  //lpdist_(0),
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
  stats_->fracCuts = 0;
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
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

  // user input for root linearization schemes
  // //MS: names of the schemes
  maxVioPer_ = env_->getOptions()->findDouble("maxVioPer")->getValue();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  bool rg1 = env_->getOptions()->findBool("root_genLinScheme1")->getValue();
  double rg2 = env_->getOptions()->findDouble("root_linGenScheme2_per")->getValue(); //MS: change name in Environment
  double rs1 = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  double rs2Per = env_->getOptions()->findDouble("root_linScheme2_per")->getValue();
  
  if (*isInf == false && nlCons_.size() > 0) {
    if (rs1 || rs2Per ||  rs3_ || rg1 || rg2) {
      extraLin_ = new Linearizations(env_, rel_, minlp_, nlCons_);
      //if (rs3_ || rg1 || rg2 || maxVioPer_) {
      if (rs3_ || rg1 || rg2) {
        extraLin_->setNlpEngine(nlpe_->emptyCopy());        
        extraLin_->findCenter();
        //if (maxVioPer_) {
          //solC_ = extraLin_->getCenter();
          //if (solC_ == 0) {
            //maxVioPer_ = 0;
          //}
        //}
      } 
      if (rs1 || rs2Per || rs3_ || rg1 || rg2) {
        extraLin_->rootLinearizations(nlpe_->getSolution()->getPrimal());
      }
    } 
      //else if (maxVioPer_) {
      //findCenter_();
    //}
  } else {
    rs3_ = 0;
    maxVioPer_ = 0;
  }
  return;
}


void QGHandlerAdvance::solveCenterNLP_(EngineStatus nlpStatus, ConstSolutionPtr sol)
{ 
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    if (solC_) {
      delete [] solC_;
      solC_ = 0;
    }
    //std::cout << "Center " << std::setprecision(6) << nlpe_->getSolution()->getObjValue();
    //exit(1);
    if (fabs(sol->getObjValue()) > solAbsTol_) {
      const double *temp = sol->getPrimal();
      solC_ = new double[minlp_->getNumVars()];
      std::copy(temp, temp+minlp_->getNumVars(), solC_);
    }
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
      << nlpe_->getStatusString() << std::endl;
    break;
  }

  return;
}

void QGHandlerAdvance::findCenter_()
{
  // Center is found if the feasible region is compact and has
  // non-empty inetrior. Otherwise, not.
  double lb, ub;
  EnginePtr nlpe = nlpe_->emptyCopy();
  bool NLPFeas = false;
  VariablePtr vPtr, v;
  ConstraintPtr con;
  FunctionType fType;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone();
  //inst_C->write(std::cout);
  //UInt numVars = minlp_->getNumVars();
  double *sol1;

  FunctionPtr fnewc;
  LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
  vPtr = inst_C->newVariable(-INFINITY, 0, Continuous, "eta", VarHand);
  vPtr->setFunType_(Nonlinear);
  inst_C->removeObjective();
  lfc->addTerm(vPtr, 1.0);
  fnewc = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(fnewc, 0.0, Minimize);

  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
    if (fType == Constant || fType == Linear) {
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

  //inst_C->write(std::cout);
  inst_C->prepareForSolve();
  nlpe->load(inst_C);
  EngineStatus nlpStatus = nlpe->solve();
  solveCenterNLP_(nlpStatus, nlpe->getSolution());

  if (solC_) {
    NLPFeas = true;
    sol1 = new double[minlp_->getNumVars()];
    std::copy(solC_, solC_+minlp_->getNumVars(), sol1);
  }
//else {
    //return;
  //}
 // Solving more restricted proiblem to find center 
  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
    if (fType == Constant) {
      inst_C->markDelete(con);
      continue;
    } else if (fType == Linear)  {
      if (lb != -INFINITY && ub != INFINITY) {
        if (lb == ub) {
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
      } else {
        inst_C->markDelete(con);
        continue;
      }
    } else {
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
  inst_C->delMarkedCons();

  for (VariableConstIterator vit=inst_C->varsBegin(); vit!=inst_C->varsEnd()-1;
       ++vit) {
    v = *vit;
    lb = v->getLb(), ub = v->getUb();
    if (lb == ub) {
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

  nlpe->clear();
  nlpe->load(inst_C);
  nlpStatus = nlpe->solve();
  solveCenterNLP_(nlpStatus, nlpe->getSolution());

  //inst_C->write(std::cout);
  if (solC_ == 0) {
    if (NLPFeas) {
      solC_ = new double[minlp_->getNumVars()];
      std::copy(sol1, sol1 + minlp_->getNumVars(), solC_);
    }
  }

  if (NLPFeas) {
    delete [] sol1;
    sol1 = 0;
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
		//exit(0);  
	//}

  *status = SepaContinue;

  if (rs3_ && node->getId() == 0) {
    rs3_ = false;
    extraLin_->rootLinScheme3(lpe_, objVar_, status);
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
     //if ((maxVioPer_ > 0) && (int(node->getDepth()) < depth_)) 
     if (maxVioPer_ > 0) {
      maxVio_(x, node, sol_found, s_pool, cutMan, status); // maxViolation
    } 
  }
  return;
}


void QGHandlerAdvance::ESHTypeCut_(const double *lpx, CutManager *cutMan,
                             ConstraintPtr con)
{
  int error = 0;
  double *x, *xl, *xu;
  bool ptChanged = true;
  FunctionPtr f = con->getFunction();
  UInt numVars =  minlp_->getNumVars(), repPt = 0;
  double cUb = con->getUb(), nlpact, repSol = INFINITY,
         lambda1 = 0.5, lambda2 = 0.5;
  
  x  = new double[numVars];
  xl = new double[numVars];
  xu = new double[numVars];
  std::copy(lpx,lpx+numVars,xu);
  std::copy(solC_,solC_+numVars,xl);
  

  while (true) {
    for (UInt i = 0 ; i < numVars; ++i) {
      x[i] = lambda1*xl[i] + lambda2*xu[i];
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
    if (nlpact == repSol) {
      ++repPt;    
    } else {
      repPt = 0;
      repSol = nlpact;
    }
    if (repSol == 5) {
      break;    
    }
  }

  if (ptChanged) {
    double c;
    std::stringstream sstm;
    LinearFunctionPtr lf = 0;
    linearAt_(f, nlpact, x, &c, &lf, &error);
    if (error == 0) { 
      ++(stats_->cuts);
      sstm << "_qgCut_" << stats_->cuts;
      f = (FunctionPtr) new Function(lf);
      rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
    }
  } else {
    nlpact =  con->getActivity(lpx, &error);
    ECPTypeCut_(lpx, cutMan, con, nlpact);
  }

  
  delete [] x;
  delete [] xl;
  delete [] xu;
  return;
}


void QGHandlerAdvance::maxVio_(const double *x, NodePtr node, bool *sol_found,
                               SolutionPoolPtr s_pool, CutManager *cutMan,
                               SeparationStatus *status)
{
  int error = 0; 
  ConstraintPtr c;
  double act, cUb, vio = 0.0;
  UInt  temp = stats_->cuts, nodeId = node->getId();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    c = *it; 
    act = c->getActivity(x, &error);
    if (error == 0) { 
      cUb = c->getUb();
      if (act > cUb+solAbsTol_ && (cUb == 0 || 
                                   act > cUb + fabs(cUb)*solRelTol_)) {
        //std::cout << "node " << node->getId() << " vio " << act -cUb << "\n";
        if (fabs(cUb) > solAbsTol_) {
          vio = 100*(act - cUb)/fabs(cUb);     
        } else {
          vio = 100*(act - cUb); 
        }
        if (vio >= maxVioPer_) {
          ECPTypeCut_(x, cutMan, c, act);
          //ESHTypeCut_(x, cutMan, c);
        }
      }
    }
  }
  stats_->fracCuts = stats_->fracCuts + (stats_->cuts - temp);

  if ((temp < stats_->cuts) && (nodeId != UInt(lastNodeId_))) {
    *status = SepaResolve;
  }
  
  //std::cout << "Node " << node->getId() << " depth " << node->getDepth() 
    //<< " vio val " << val << " bnd " << bnd << " max vio % " << max << "\n";
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
	////exit(1);

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
	////exit(1);
	//nlpe->load(inst_C);
	//shortestNlpStatus_ = nlpe->solve();

	//switch(shortestNlpStatus_) {
	//case (ProvenOptimal):
	//case (ProvenLocalOptimal):
		//lpdist_ = nlpe->getSolution()->getObjValue();
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
      //sstm << "_qgCut_" << stats_->fracCuts;
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
    << me_ << "number of cuts added                        = " 
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
