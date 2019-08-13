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
  nlpe1_(EnginePtr()),
  lpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  solC_(NULL),
  stats_(0)
{
  rs1_ = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  rs2Per_ = env_->getOptions()->findDouble("root_linScheme2_per")->getValue();
  rs2NbhSize_ = env_->getOptions()->findDouble("root_linScheme2_nbhSize")->getValue();
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
  nlpe1_(EnginePtr()),
  lpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  solC_(NULL)
{
  //MS: set the option for root_LinSchemes
  rs1_ = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  rs2Per_ = env_->getOptions()->findDouble("root_linScheme2_per")->getValue();
  rs2NbhSize_ = env_->getOptions()->findDouble("root_linScheme2_nbhSize")->getValue();
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
  if (solC_) {
    delete [] solC_;
  }
  env_ = 0;
  rel_ = 0;
  minlp_ = 0;
  nlCons_.clear();
}


void QGHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status)
{
  int error=0;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  double c, lpvio, act, cUb;
  FunctionPtr f = con->getFunction();
  LinearFunctionPtr lf = 0;

  act = con->getActivity(nlpx, &error);
  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error==0) {
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if ((lpvio > solAbsTol_) &&
          ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
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
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        return;
      } else {
        delete lf;
        lf = 0;
      }
    }
  } else {
    logger_->msgStream(LogError) << me_ << " constraint not defined at"
      << " this point. "<<  std::endl;
#if SPEW
          logger_->msgStream(LogDebug) << me_ << " constraint " <<
            con->getName() << " not defined at this point." << std::endl;
#endif
  }
  return;
}

UInt QGHandler::addCutAtRoot_(double *x, ConstraintPtr con)
{
  int error = 0; 
  FunctionPtr f;
  double c, act, cUb;
  ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = LinearFunctionPtr();

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
      return newcon->getIndex();
      sstm.str("");
    }
  }	else {
    logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
      " is not defined at this point." << std::endl;
  }
  return 0;
}


void QGHandler::addInitLinearX_(const double *x)
{ 
  int error = 0;
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con;
  //ConstraintPtr newcon;
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
        sstm << "_OAcut_" << stats_->cuts << "_AtRoot";
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
      sstm << "_OAObjcut_" << stats_->cuts << "_AtRoot";
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
        "Objective not defined at this point." << std::endl;
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<
        "Objective not defined at this point." << std::endl;
#endif
    }
  }
  return;
}


//MS: for rootLinScheme3_ for all constraints
void QGHandler::addEshAtRoot_(const double *lpx, double* x, ConstraintPtr con)
{
  int error = 0;
  bool lsPtFound;
  std::stringstream sstm;
  //ConstraintPtr newcon;
  LinearFunctionPtr lf = 0;
  FunctionPtr f = con->getFunction();
  double c, nlpact, cUb = con->getUb();
  
  lsPtFound = lineSearchPt_(x, solC_, lpx, con, nlpact);

  if (lsPtFound) {
    if (error == 0) {
      linearAt_(f, nlpact, x, &c, &lf, &error);
    }	else {
      return;
    }
  } else {
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      linearAt_(f, nlpact, lpx, &c, &lf, &error);
    }	else {
      return;
    }
  }
  if (error==0) {
    ++(stats_->cuts);
    sstm << "_OAcut_" << stats_->cuts;
    f = (FunctionPtr) new Function(lf);
    rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
    //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
    //newcon->write(std::cout);
  }
  
  //if (oNl_) {
    //ObjectivePtr o = minlp_->getObjective();
    
    //nlpact = o->eval(lpx, &error);
    //if (error == 0) {
      //vio = std::max(nlpact-relobj_, 0.0);
      //if ((vio > solAbsTol_)
        //&& (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
        //nlpact = o->eval(x, &error);
        //if (error == 0) {
          //f = o->getFunction();
          //lf = LinearFunctionPtr(); 
          //linearAt_(f, nlpact, x, &c, &lf, &error);
          //if (error == 0) {
            //vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            //if ((vio > solAbsTol_) && ((relobj_-c) == 0
                                     //|| vio > fabs(relobj_-c)*solRelTol_)) {
              //++(stats_->cuts);
              //sstm << "_OAObjcut_" << stats_->cuts;
              //lf->addTerm(objVar_, -1.0);
              //f = (FunctionPtr) new Function(lf);
              //rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              ////newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              ////newcon->write(std::cout);
            //}
          //}
        //}
      //}  else {

      //}
    //}	else {

    //}
  //}
  return;
}


bool QGHandler::addNewCut_(double *b1, UInt lVarIdx, ConstraintPtr con,
                           double lVarCoeff, double nVarCoeff, UInt &newConId)
{
  double act; 
  int error = 0;
  QuadraticFunctionPtr qf = con->getQuadraticFunction();
  NonlinearFunctionPtr nlf = con->getNonlinearFunction();
  if (nlf) {
    act = nlf->eval(b1, &error); 
  } else if (qf) {
    act = qf->eval(b1); 
  }
  if (error == 0) {
    b1[lVarIdx] = (con->getUb()- act - nVarCoeff)/lVarCoeff;    
    newConId = addCutAtRoot_(b1, con);
    if (newConId != 0) {
      return true;
    } 
  }
  return false;
}


void QGHandler::consCutAtLpSol_(const double *lpx, CutManager *,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  ConstraintPtr con;
  //ConstraintPtr newcon;
  double c, nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    f = con->getFunction();
    lf = LinearFunctionPtr();
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((nlpact > cUb + solAbsTol_) &&
          (cUb == 0 || nlpact > cUb+fabs(cUb)*solRelTol_)) {
        linearAt_(f, nlpact, lpx, &c, &lf, &error);
        if (error==0) {
          ++(stats_->cuts);
          sstm << "_OAcut_" << stats_->cuts;
          *status = SepaResolve;
          f = (FunctionPtr) new Function(lf);
          rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          return;
        }
      }
    }	else {
      logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        " this point. "<<  std::endl;
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
    if ((relobj_ >= nlpval-objATol_) ||
        (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRTol_))) {
        *status = SepaPrune;
        break;
    } else {
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
    << nlpe_->getStatusString() << " and separation status = " << *status <<
    std::endl;
#endif
  return;
}


void QGHandler::cutToCons_(const double *nlpx, const double *lpx,
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
      if ((nlpact > cUb + solAbsTol_) &&
          (cUb == 0 || nlpact > cUb+fabs(cUb)*solRelTol_)) {
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
    
    act = o->eval(lpx, &error);
    if (error == 0) {
      vio = std::max(act-relobj_, 0.0);
      if ((vio > solAbsTol_)
        && (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
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
            if ((vio > solAbsTol_) && ((relobj_-c) == 0
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
              //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
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


void QGHandler::findCenter_(bool &noCenter)
{
  //Find center
  double act;
  VariablePtr vPtr;
  ConstraintPtr con;
  FunctionType fType;
  const double *dtemp;
  EngineStatus nlpStatus;
  ProblemPtr inst_C = minlp_->clone();

  FunctionPtr fnewc;
  std::string name = "eta";
  LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
  vPtr = inst_C->newVariable(-INFINITY, INFINITY, Continuous,
                                       name, VarHand);
  inst_C->removeObjective();
  lfc->addTerm(vPtr, 1.0);
  fnewc = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(fnewc, 0.0, Minimize);
  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     ++it) {
    con = *it;
    fType = con->getFunctionType();
    if (fType == Constant || fType == Linear) {
      continue;
    }
    lfc = con->getLinearFunction();
    if (lfc) {
      lfc->addTerm(vPtr, -1.0);
    } else {
      lfc = (LinearFunctionPtr) new LinearFunction();
      lfc->addTerm(vPtr, -1.0);
    }
    inst_C->changeConstraint(con, lfc, con->getLb(), con->getUb());
  }
  inst_C->resetInitialPoint(1);
  
  nlpe1_ = nlpe_->emptyCopy(); //Engine for modified problem
  nlpe1_->load(inst_C);
  nlpStatus = nlpe1_->solve();
 
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    dtemp = nlpe1_->getSolution()->getPrimal();
    if (solC_) {
      delete solC_;
    }
    solC_ = new double[minlp_->getNumVars()];
    std::copy(dtemp, dtemp+minlp_->getNumVars(), solC_);
    act = nlpe1_->getSolution()->getObjValue();
    if (act >= 0) {
      noCenter = true;
    } else {
      std::cout << "Center NLP Status " << nlpe1_->getStatusString() 
        << " and obj value " << act << std::endl;
    }
    ++(stats_->nlpF);
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    solC_ = NULL;
    noCenter = true;
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
      << nlpe1_->getStatusString() << std::endl;
    solC_ = NULL;
    noCenter = true;
    break;
  }
  delete nlpe1_;
  nlpe1_ = 0;
  delete inst_C;
  inst_C = 0;
  return;
}


bool QGHandler::findIntersectPt_(std::vector<UInt > newConsId, VariablePtr vl,
                                VariablePtr vnl, double * iP)
{
  ConstraintPtr con;
  LinearFunctionPtr lf;
  con = rel_->getConstraint(newConsId[0]);
  lf = con->getLinearFunction();
  //std::cout << "linear and nonlinear  pointer " << vl << " " << vnl << std::endl;
  double a = lf->getWeight(vl);
  double b = lf->getWeight(vnl);
  double e = con->getUb();
  
  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  double c = lf->getWeight(vl);
  double d = lf->getWeight(vnl);
  double f = con->getUb();

  /* we solve the linear system
   * ax+by=e
   * cx+dy=f
   * where, x is iP[1] and y is iP[0]
   */
  double determinant = a*d - b*c; 
  if(determinant != 0) {
    iP[1] = (e*d - b*f)/determinant;
    iP[0] = (a*f - e*c)/determinant;
  } else {
    std::cout << "Cramer equations system: determinant is zero\n"
              "there are either no solutions or many solutions exist.\n";
    return false; 
  }
  return true;
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
    assert(!"In QGHandler: stopped at root. Check error log.");
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = " 
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}


void QGHandler::insertNewPt_(UInt j, UInt k, std::vector<double > & xc,
                             std::vector<double> & yc, ConstraintPtr con,
                             VariablePtr vl, VariablePtr vnl, bool & shouldCont)
{
  LinearFunctionPtr lf;
  double f = con->getUb();
  lf = con->getLinearFunction();
  double d = lf->getWeight(vl);
  double c = lf->getWeight(vnl);
  double x1 = xc[j], y1 = yc[j], x2 = xc[k], y2 = yc[k], x, y;

  // point of intersection of newcon with the lin from j and j-1
  double a = y1-y2;
  double b = x2-x1;
  double e = y1*(x2-x1) - x1*(y2-y1);
  double determinant = a*d - b*c; 
  if(determinant != 0) {
    x = (e*d - b*f)/determinant;
    y = (a*f - e*c)/determinant;
    xc.insert(xc.begin()+j,x);
    yc.insert(yc.begin()+j,y);
  } else {
    std::cout << "Cramer equations system: determinant is zero\n"
              "there are either no solutions or many solutions exist.\n"; 
    shouldCont = false;
  }
}


bool QGHandler::isFeas_(ConstSolutionPtr sol)
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
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ << c->getName() <<
        " constraint not defined at this point."<< std::endl;
      return false;
    }
  }
  return true;
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
        " constraint not defined at this point."<< std::endl;
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
      if ((act > relobj_ + solAbsTol_) &&
          (relobj_ == 0 || (act > relobj_ + fabs(relobj_)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "objective violated with "
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
    assert(objType == Minimize);
    rel_->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel_->newObjective(f, 0.0, objType);
    objVar_ = vPtr;
  }
  return;
}


//MS: rootLinScheme3_ - all ESH - working
bool QGHandler::lineSearchPt_(double* x, const double* l, const double* u,
                              ConstraintPtr con, double &nlpact)
{
  int error = 0;
  bool ptFound = true;
  double cUb = con->getUb();
  FunctionPtr f = con->getFunction();
  UInt numVars =  minlp_->getNumVars();
  
  double* xl = new double[numVars];
  double* xu = new double[numVars];
  std::copy(l,l+numVars,xl);
  std::copy(u,u+numVars,xu);
  //std::cout << "xl x xu " << std::endl;
  while (true) {
    for (UInt i = 0 ; i < minlp_->getNumVars(); ++i) {
      x[i] = 0.5*(xl[i] + xu[i]);
      //std::cout << xl[i] << " " << x[i] << " " << xu[i] << std::endl;
    }
    nlpact = f->eval(x, &error);
    //std::cout << "cons val at new point and ub " << nlpact << " " << cUb << "\n";
    if (error == 0) {
      if (nlpact>cUb+solAbsTol_ && (cUb==0 ||  nlpact > cUb+fabs(cUb)*solRelTol_)) {
        std::copy(x,x+numVars,xu);
        //std::cout << "change upper bound \n";
      } else {
        if (fabs(cUb-nlpact) <= solAbsTol_ || (cUb!=0 && fabs(cUb-nlpact) <= fabs(cUb)*solRelTol_)) {
          break;
        } else {
          //std::cout << "change lower bound \n";
          std::copy(x,x+numVars,xl);
        }
      }
    } else {
      //MS: think later what can be done here
      ptFound = false;
      break;
    }
  }

  delete [] xl;
  delete [] xu;
  return ptFound;
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

  if (*isInf == false) {
    //Default values of these parameters
    if ((rs1_ + rs2Per_ + rs3_) > 0) {
      rootLinearizations_();
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


void QGHandler::rootLinearizations_()
{
  bool shouldCont;
  ConstraintPtr con;
  double lVarCoeff, nVarCoeff;
  UInt nVarIdx, lVarIdx, numRS1Lin = 0, numRS2Lin = 0;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    lVarIdx = 0;
    lVarCoeff = 0;
    nVarCoeff = 0;
    // constraints with only one var in the nonlinear part
    shouldCont = twoVarsCon_(con, lVarCoeff, lVarIdx, nVarIdx, nVarCoeff); 
    if (shouldCont == false) {
      continue;    
    } else {
      if (rs1_ > 0) {
        rootLinScheme1_(con, lVarCoeff, lVarIdx, nVarIdx, nVarCoeff, numRS1Lin);
      }
      if (rs2Per_ > 0) { // there is a default neighborhood
        rootLinScheme2_(con, lVarCoeff, lVarIdx, nVarIdx, numRS2Lin);  
      }    
    }
  }
  std::cout << "No. of nonlinear cons, root lin Scheme 1, Scheme 2 added: "
    << nlCons_.size() << " " << numRS1Lin << " " << numRS2Lin << std::endl; 

  if (rs3_ > 0) {
    if (nlCons_.size() > 0) {
      bool noCenter = false;
      findCenter_(noCenter);
      if (noCenter == true) {
        //std::cout << "No center is found\n";
        rs3_ = 0;
      }   
    } else {
      rs3_ = 0;
    }
  }
  return;
}


void QGHandler::rootLinScheme1_(ConstraintPtr con, double lVarCoeff,
                            UInt lVarIdx, UInt nVarIdx, double nVarCoeff,
                            UInt &numRS1Lin)
{
  double iP[2]; // intersection point
  UInt newConId;
  bool shouldCont;
  ConstraintPtr newcon;
  std::vector<UInt > newConsId;
  VariablePtr vnl = NULL, vl = NULL;
  std::vector<double > linVioVal, xc, yc; // xc and yc  nonlinear and lin var
  int i, error = 0, n = rel_->getNumVars();
  double *b1 = new double[n];
  double act, cUb, y1, y2, maxVio, stopCond, consUb; 

  std::fill(b1, b1+n, 0.);
  vl = rel_->getVariable(lVarIdx);
  vnl = rel_->getVariable(nVarIdx);
  b1[nVarIdx] = vnl->getLb();
  shouldCont = addNewCut_(b1, lVarIdx, con, lVarCoeff, nVarCoeff, newConId);
  if (shouldCont) {
    ++numRS1Lin;
    y1 = b1[lVarIdx];
    newConsId.push_back(newConId); 
  } else {
    delete [] b1;
    return;    
  }

  // upper bound of var in nonlinear cons
  b1[nVarIdx] = vnl->getUb();
  shouldCont = addNewCut_(b1, lVarIdx, con, lVarCoeff, nVarCoeff, newConId);
  if (shouldCont) {
    ++numRS1Lin;
    y2 = b1[lVarIdx];
    newConsId.push_back(newConId);
  } else {
    delete [] b1;
    return;    
  }  

  shouldCont = findIntersectPt_(newConsId, vl, vnl, iP);
  if (shouldCont == false) {
    delete [] b1;
    return;    
  }

  // populate points and their violation in cyclic order
  xc.push_back(vnl->getLb());
  yc.push_back(y1);
  linVioVal.push_back(0);
      
  b1[nVarIdx] = iP[0], b1[lVarIdx] = iP[1];
  act = con->getActivity(b1, &error);
  if (error != 0) {
    delete [] b1;
    return;    
  }  
  consUb = con->getUb();
  act = std::max(act-consUb, 0.0); 
  xc.push_back(iP[0]), yc.push_back(iP[1]), linVioVal.push_back(act);
  xc.push_back(vnl->getUb()), yc.push_back(y2), linVioVal.push_back(0);

  i = 1; // starting from intersection point
  if (fabs(consUb) > solAbsTol_) { 
    stopCond = consUb*rs1_/100;    
  } else {
    stopCond = linVioVal[i]*rs1_/100;    
  }
  maxVio = linVioVal[i];
  while (maxVio >= stopCond) { 
    //add a new cut at the point indexed i
    b1[nVarIdx] = xc[i];
    shouldCont = addNewCut_(b1, lVarIdx, con, lVarCoeff, nVarCoeff, newConId);
    if (shouldCont) {
      ++numRS1Lin;
      newcon = rel_->getConstraint(newConId);
      cUb = newcon->getUb();
    } else {
      break;
    }
    
    // Move right and determine first point that satisfy the newcon
    for (UInt j = i+1; j < xc.size(); ) {
      b1[nVarIdx] = xc[j], b1[lVarIdx] = yc[j];
      act = newcon->getActivity(b1, &error);
      if (error == 0) {
        if ((act < cUb + solAbsTol_) ||
            (cUb =! 0 && act < cUb + fabs(cUb)*solRelTol_)) {
            //insert new point just before index j
          insertNewPt_(j, j-1, xc, yc, newcon, vl, vnl, shouldCont); 
          b1[nVarIdx] = xc[j], b1[lVarIdx] = yc[j];
          act = con->getActivity(b1, &error);
          if (error != 0) {
            shouldCont = false;    
          } else {
            act = std::max(act-consUb, 0.0); 
            linVioVal.insert(linVioVal.begin()+j,act);
          }
          break;
        } else {
          // delete point if violates newcon
          xc.erase(xc.begin() + j);
          yc.erase(yc.begin() + j);        
          linVioVal.erase(linVioVal.begin() + j);        
        }  
      }
    }
    if (shouldCont == false) {
      break;
    }
    for (int j = i-1; j >= 0; --j) {
      b1[nVarIdx] = xc[j], b1[lVarIdx] = yc[j];
      act = newcon->getActivity(b1, &error);
      if (error == 0) {
        if ((act < cUb + solAbsTol_) ||
            (cUb =! 0 && act < cUb + fabs(cUb)*solRelTol_)) {
          insertNewPt_(j+1, j, xc, yc, newcon, vl, vnl, shouldCont); 
          b1[nVarIdx] = xc[j+1], b1[lVarIdx] = yc[j+1];
          act = con->getActivity(b1, &error);
          if (error != 0) {
            shouldCont = false;    
          } else {
            act = std::max(act-consUb, 0.0); 
            linVioVal.insert(linVioVal.begin()+j+1,act);
            xc.erase(xc.begin()+j+2);
            yc.erase(yc.begin()+j+2);        
            linVioVal.erase(linVioVal.begin()+j+2);        
          }
          break;
        }  else {
          // delete point if violates newcon
          xc.erase(xc.begin() + j);
          yc.erase(yc.begin() + j);        
          linVioVal.erase(linVioVal.begin() + j);        
        }  
      }
    }
    if (shouldCont == false) {
      break;
    }
    maxVio = *(std::max_element(linVioVal.begin(), linVioVal.end()));
    if ((maxVio < solAbsTol_) || 
        (consUb!=0 && maxVio < fabs(consUb)*solRelTol_ )) { 
      break;
    }
    i = std::max_element(linVioVal.begin(), linVioVal.end())-linVioVal.begin();     
  }
  delete [] b1;
  return;
}


void QGHandler::rootLinScheme2_(ConstraintPtr con, double lVarCoeff,
                                UInt lVarIdx, UInt nVarIdx, UInt &numRS2Lin)
{
  int error = 0;
  FunctionPtr f;
  VariablePtr vnl;
  UInt n = minlp_->getNumVars();
  double alpha, lastSlope, delta, nlpSlope, nbhSize;
  const  double *nlpx = nlpe_->getSolution()->getPrimal();
  
  vnl = rel_->getVariable(nVarIdx);
  
  double* npt = new double[n];
  std::fill(npt, npt+n, 0.);
  
  double *grad = new double[n];
  std::fill(grad, grad+n, 0.);
  
  f = con->getFunction();
  f->evalGradient(nlpx, grad, &error);
  
  nlpSlope = -1*(grad[nVarIdx]/lVarCoeff);
  lastSlope = nlpSlope;
  
  nbhSize = std::max(vnl->getLb(), nlpx[nVarIdx] - rs2NbhSize_);    
  if (nlpx[nVarIdx] - nbhSize >= 1) {
    delta = 0.5;  
  } else {
    delta = nlpx[nVarIdx] - nbhSize;  
  }

  alpha = nlpx[nVarIdx] - delta;
     
  if (delta != 0) {
    while (alpha >= nbhSize) {
      grad[nVarIdx] = 0;
      grad[lVarIdx] = 0;
      rScheme2Cut_(con, alpha, delta, lVarCoeff, lastSlope, nVarIdx, npt, grad,
                   numRS2Lin);
      alpha = alpha - delta;
    }
  }
  
  nbhSize = std::min(vnl->getUb(), nlpx[nVarIdx] + rs2NbhSize_);
  if (nbhSize - nlpx[nVarIdx] >= 1) {
    delta = 0.5;  
  } else {
    delta = nbhSize - nlpx[nVarIdx];  
  }

  lastSlope = nlpSlope;
  alpha = nlpx[nVarIdx] + delta;
  if (delta != 0) {
    while (alpha <= nbhSize) {
      grad[nVarIdx] = 0;
      grad[lVarIdx] = 0;
      rScheme2Cut_(con, alpha, delta, lVarCoeff, lastSlope, nVarIdx, npt, grad,
                   numRS2Lin);
      alpha = alpha + delta;
    }
  }
  delete [] grad;
  delete [] npt;
  return;
}


void QGHandler::rScheme2Cut_(ConstraintPtr con, double &alpha, double &delta,
                                double lVarCoeff, double &lastSlope,
                                UInt nVarIdx, double * npt, double * grad,
                                UInt &numRS2Lin)
{
  int error = 0;
  double newSlope;
  npt[nVarIdx] = alpha; 
  FunctionPtr f = con->getFunction();
  f->evalGradient(npt, grad, &error);
  if (error != 0) {
    return;
  } 
  newSlope = -1*(grad[nVarIdx]/lVarCoeff); 
  if ((lastSlope == 0 && newSlope == 0) ||
      (fabs((newSlope-lastSlope)/lastSlope)*100 <  rs2Per_)) {
    delta = 2*delta;
    return;
  }
  lastSlope = newSlope;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = LinearFunctionPtr();
  VariableConstIterator vbeg = rel_->varsBegin(), vend = rel_->varsEnd();
  const double linCoeffTol =
    env_->getOptions()->findDouble("conCoeff_tol")->getValue();
  double c, cUb = con->getUb(), act = con->getActivity(npt, &error);
  
  lf = (LinearFunctionPtr) new LinearFunction(grad, vbeg, vend, linCoeffTol);
  c  = act - InnerProduct(npt, grad, minlp_->getNumVars());

  ++(stats_->cuts);
  sstm << "_OAcut_" << stats_->cuts << "_AtRoot";
  f = (FunctionPtr) new Function(lf);
  rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
  numRS2Lin++;
  //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
  //newcon->write(std::cout);
  return;
}


//MS: add esh all from LP solution - working
void QGHandler::rootLinScheme3_(ConstSolutionPtr sol, CutManager * cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  UInt oldCuts;
  int error = 0;
  FunctionPtr f;
  ObjectivePtr o;
  const double *lpx;
  ConstraintPtr con;
  VariableType v_type;
  LinearFunctionPtr lf;
  ConstSolutionPtr lpSol;
  std::stringstream sstm;
  EngineStatus engineStatus;
  bool should_prune = false, isFrac;
  double c, val, act, cUb, bestVal, vio;
  double *x = new double[minlp_->getNumVars()];
  //ConstraintPtr newcon;
 
  // load relaxation to the lp engine 
  //lpe_->load(rel_);
  //Check if new constraints are added
 
  lpSol = sol;
  lpx = lpSol->getPrimal();
 //MS: check if this works 
  //std::cout << "number of cuts initially " << stats_->cuts << std::endl;
  oldCuts = stats_->cuts;
  for (UInt i = 1; i <= rs3_; ++i) {
    isFrac = false;
    //std::cout << "Iteration and LB " << i+1 << " " << std::setprecision(6) 
      //<< lpSol->getObjValue() << std::endl;
    for (VariableConstIterator v_iter = rel_->varsBegin(); 
         v_iter != rel_->varsEnd(); ++v_iter) {
      v_type = (*v_iter)->getType();
      if (v_type == Binary || v_type == Integer) {
        val = lpx[(*v_iter)->getIndex()];
        if (fabs(val - floor(val+0.5)) > intTol_) {
          isFrac = true;
          break;
        }
      }
    } // for
    //MS: make sure adding objective here
    if (isFrac) {
      for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it) {
        con = *it;
        cUb = con->getUb();
        act = con->getActivity(lpx, &error);
        if (error == 0) {
          if ((act > cUb + solAbsTol_) &&
              (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
            addEshAtRoot_(lpx, x, con); 
          }
        } else {
          logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName()
            << " is not defined at this point." << std::endl;
        }
      }
    } else {
      if (isFeas_(lpSol)) {
        val = lpSol->getObjValue();
        bestVal = s_pool->getBestSolutionValue();
        if ((bestVal - objATol_ > val) ||
            (bestVal != 0 && (bestVal - fabs(bestVal)*objRTol_) > val)) {
          s_pool->addSolution(lpSol);
          *sol_found = true;
          *status = SepaPrune;
          break;
          //delete [] x;
          //return;
        }
      } else {
        //check nonlinear cons feasibility if feasible stop else solve NLP and
        //add cuts      
        cutIntSol_(lpSol, cutMan, s_pool, sol_found, status);
        if (*status == SepaPrune || *status == SepaError) {
          //MS: take care of this later
          //delete [] x;
          //return;          
          break;
        }
      }
    }

    if (oNl_) {
      error = 0;
      o = minlp_->getObjective();
      act = o->eval(lpx, &error);
      relobj_ = (lpSol) ? lpSol->getObjValue() : -INFINITY;
      if (error == 0) {
        vio = std::max(act-relobj_, 0.0);
        if ((vio > solAbsTol_)
          && (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
          f = o->getFunction();
          lf = LinearFunctionPtr(); 
          linearAt_(f, act, lpx, &c, &lf, &error);
          if (error == 0) {
            ++(stats_->cuts);
            sstm << "_OAObjcut_" << stats_->cuts;
            lf->addTerm(objVar_, -1.0);
            //status = SepaResolve;
            f = (FunctionPtr) new Function(lf);
            rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
            //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
          }
        } else {
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
    //if (i == rs3_ || stats_->cuts == oldCuts)  
    if (stats_->cuts == oldCuts) {
      //delete [] x;
      break;    
    }
    oldCuts = stats_->cuts;    
    //std::cout << "number of cuts " << stats_->cuts << std::endl;
    lpe_->solve();
    engineStatus = lpe_->getStatus();
    should_prune = shouldPrune_(engineStatus);
    if (should_prune) {
      //delete [] x;
      break;    
    }
    lpSol = lpe_->getSolution();
    lpx = lpSol->getPrimal();
  }
  delete [] x;
  return;
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
    rootLinScheme3_(sol, cutMan, s_pool, sol_found, status);
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
  lpe_ = lpe;
}


bool QGHandler::shouldPrune_(EngineStatus eStatus)
{
  bool should_prune = false;
  switch (eStatus) {
   case (FailedInfeas):
     logger_->msgStream(LogInfo) << me_ << "failed to converge "
     << "(infeasible) in root" << std::endl;
     should_prune = true;
     break;
   case (ProvenFailedCQInfeas):
     logger_->msgStream(LogInfo) << me_ << "constraint qualification "
                                        << "violated in root "
                                        << std::endl;
   case (ProvenInfeasible):
   case (ProvenLocalInfeasible):
     should_prune = true;
     break;
   case (ProvenObjectiveCutOff):
     should_prune = true;
     break;
   case (ProvenUnbounded):
     should_prune = false;
     logger_->msgStream(LogDebug2) << me_ << "problem relaxation is "
                                   << "unbounded!" << std::endl;
     assert(!"Relaxation unbounded."); 
     break;
   case (FailedFeas):
     logger_->msgStream(LogInfo) << me_ << "Failed to converge " 
                                 << "(feasible) in root " << std::endl;
     break;
   case (ProvenFailedCQFeas):
     logger_->msgStream(LogInfo) << me_ << "constraint qualification "
                                 << "violated in root" << std::endl;
     break;
   case (EngineIterationLimit):
     logger_->msgStream(LogInfo) << me_ << "engine hit iteration limit, "
                                 << "continuing in root" << std::endl;
     // continue with this node by following ProvenLocalOptimal case.
   case (ProvenLocalOptimal):
   case (ProvenOptimal):
     break;
   case (EngineError):
     should_prune = true;
     break;
   default:
     break;
  }

  return should_prune;
}


void QGHandler::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  return;
}


bool QGHandler::twoVarsCon_(ConstraintPtr con, double &lVarCoeff,
                            UInt & lVarIdx, UInt & nVarIdx, double &nVarCoeff)
{
  double coeff;
  bool foundVar = false;
  UInt nlTerms = 0, qTerms = 0, idx;
  LinearFunctionPtr lf = con->getLinearFunction();
  QuadraticFunctionPtr qf = con->getQuadraticFunction();
  NonlinearFunctionPtr nlf = con->getNonlinearFunction();
  const double linCoeffTol =
    env_->getOptions()->findDouble("conCoeff_tol")->getValue();
  if (nlf) {
    nlTerms = nlf->numVars();
    if (nlTerms > 1) {
      return false;    
    }
    nVarIdx= (*(nlf->varsBegin()))->getIndex(); //index of var in nonlinear term
  }
  if (qf) {
    qTerms = qf->getNumVars();  
    if (qTerms > 1) {
      return false;    
    }
    nVarIdx= ((qf->varsBegin())->first)->getIndex(); 
  }
  if (qTerms + nlTerms != 1) {
    return false;  
  }
  if (lf) {
    for(VariableGroupConstIterator vit = lf->termsBegin();
        vit != lf->termsEnd(); ++vit) {
      coeff = vit->second;
      idx = (vit->first)->getIndex();
      if (idx == nVarIdx) {
        nVarCoeff = nVarCoeff + coeff;
        continue;      
      }
      if (fabs(vit->second) > linCoeffTol && foundVar == false) {
        lVarCoeff = vit->second;
        lVarIdx = (vit->first)->getIndex();
        foundVar = true;
      }
    }
  }
  if (foundVar) {
    return true;
  }
  return false;
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
