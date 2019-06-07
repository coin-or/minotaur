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
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  stats_(0)
{
  numCuts_ = env_->getOptions()->findInt("cut_depth")->getValue();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger(LogInfo);
}


QGHandler::QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0)
{
  numCuts_ = env_->getOptions()->findInt("cut_depth")->getValue();
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
}

UInt QGHandler::addCutAtRoot_(double *x, ConstraintPtr con, int & error)
{ 
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
      rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //++numCutCount_;
      return newcon->getIndex();
      sstm.str("");
    }
  }	else {
    logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
      " is not defined at this point." << std::endl;
    return 0;
  }
}

bool QGHandler::addNewCut_(double *b1, UInt vlIdx, ConstraintPtr con, double linTermCoeff, int &error, UInt &newConId, NonlinearFunctionPtr nlf)
{
  double act; 
  act = nlf->eval(b1, &error); 
  if (error == 0 && linTermCoeff != 0) {
    b1[vlIdx] = (con->getUb()- act)/linTermCoeff;    
    newConId = addCutAtRoot_(b1, con, error);
    return true;
  }
  return false;
}


void QGHandler::addInitLinearX_(const double *x)
{ 
  int error=0;
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con, newcon;
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
        newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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
    //objCutAtLpSol_(lpx, cutMan, status);
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


void QGHandler::fixInts_(const double *x)
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


void QGHandler::initLinear_(bool *isInf)
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
    assert(!"In QGHandler: stopped at root. Check error log.");
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = " 
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}

void QGHandler::addNewBoundaryPt_(char linLoc, std::vector<double>& xNew, std::vector<double>& yNew, ConstraintPtr con, VariablePtr vl, VariablePtr vnl)
{
  LinearFunctionPtr lf;
  double nVarVal, lVarVal, xc, yc;
  double lbN = vnl->getLb(), lbL = vl->getLb(), ubL = vl->getUb(), ubN = vnl->getUb();
  lf = con->getLinearFunction();
  nVarVal = lf->getWeight(vnl);
  lVarVal = lf->getWeight(vl);

  if (linLoc == 'b') {
    // find bottom point. This point has x[1] = vl->getLb()
    yc = lbL;
    xc = findLinVarVal_(lbL, lVarVal, nVarVal, con->getUb(), lbN);  
  } else {
    yc = ubL;
    xc = findLinVarVal_(ubL, lVarVal, nVarVal, con->getUb(), ubN);  
    // otherwise top boundary
    // find top point. This point has x[1] = vl->getUb()
    xNew.push_back(xc);
    yNew.push_back(yc);
  }

}
void QGHandler::insertNewPt_(double x1, double y1, double x2, double y2, std::vector<double >& xNew, std::vector<double >& yNew,ConstraintPtr con, VariablePtr vl, VariablePtr vnl)
{
  LinearFunctionPtr lf;
  lf = con->getLinearFunction();
  double c = lf->getWeight(vnl);
  double d = lf->getWeight(vl);
  double f = con->getUb();
  double x, y;

  double a = y1-y2;
  double b = x2-x2;
  double e = y1*(x2-x1) - x1*(y2-y1);
  double determinant = a*d - b*c; 
  if(determinant != 0) {
    x = (e*d - b*f)/determinant;
    y = (a*f - e*c)/determinant;
    xNew.push_back(x);
    yNew.push_back(y);
  } else {
    std::cout << "Cramer equations system: determinant is zero\n"
              "there are either no solutions or many solutions exist.\n"; 
  }
}

//MS: make sure that this routine is called when constraints has exactly two
//variables
void QGHandler::rootLinearizations(CutManager *)
{
  char lTemp;
  FunctionPtr f;
  VariablePtr vnl, vl;
  bool shouldCont;
  NonlinearFunctionPtr nlf;
  ConstraintPtr con, newcon;
  UInt vnIdx, vlIdx, newConId, i, j, ptCount;
  int error = 0, iPtAt, n = rel_->getNumVars();
  double act, cUb, linTermCoeff;
  double *b1 = new double[n];
  double iP[2]; //intersection point
  std::vector<UInt > newConsId;
  std::vector<double > linVioVal, xNew, yNew, xc, yc;
  std::vector<char > linPtsLoc;
  std::vector<UInt > ptScore; // 1 for deleion and 2 for insertion, 3 for both, 4 for boundary point and 0 for none
  std::fill(b1, b1+n, 0.);
  
  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    //numCutCount_ = 0; //MS: need not be a private var
    nlf = con->getNonlinearFunction();
    vnIdx= (*(nlf->varsBegin()))->getIndex(); // index of var in nonlinear term
    vlIdx = (con->getLinearFunction()->termsBegin()->first)->getIndex(); // index of var in lin term
    linTermCoeff = con->getLinearFunction()->termsBegin()->second;
    vl = rel_->getVariable(vlIdx);
    vnl = rel_->getVariable(vnIdx);
    //con->write(std::cout);
    //std::cout << "Lin var and nonlin var pointer - cons1 " << vl << " " << vnl << std::endl;
    // lower bound of var in nonlinear cons
    b1[vnIdx] = vnl->getLb();
    shouldCont = addNewCut_(b1, vlIdx, con, linTermCoeff, error, newConId, nlf);
    if (shouldCont) {
      newConsId.push_back(newConId); // MS: Add cutmanager later
    } else {
      continue;    
    }
    // upper bound of var in nonlinear cons
    b1[vnIdx] = vnl->getUb();
    shouldCont = addNewCut_(b1, vlIdx, con, linTermCoeff, error, newConId, nlf);
    if (shouldCont) {
      newConsId.push_back(newConId); // MS: Add cutmanager later
    } else {
      newConsId.clear();
      continue;    
    }   
    // Find intersection point and its location. In iPtAt index 0 is for
    // nonlinear and 1 is for linear var
    iPtAt = findIntersectPt(newConsId, vl, vnl, iP);

    // If the intersection point is strictly inside the box (obtained by bound
    // constraints) then add. Otherwise, intersection point gets added in one
    // of the case, provided it is not outside the box.
    switch(iPtAt) {
    case (1):
        linPtsLoc.push_back('i');
        xc.push_back(iP[0]);
        yc.push_back(iP[1]);
    case (0):
      {
        bottomEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        rightEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        topEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        leftEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }
      break;
    case (2):
      {
        rightEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        topEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        leftEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }
      break;
    case (3):
      {
        bottomEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        rightEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        leftEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }
      break;
    case (4):
      {
        bottomEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        rightEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        topEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }   
      break;
    case (5):
      {
        rightEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        topEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }      
      break;
    case (6):
      {
        bottomEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        rightEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }   
      break;
    case (7):
      {
        bottomEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        topEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        leftEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }   
      break;
    case (8):
      {
        topEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        leftEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }     
      break;
    case (9):
      {
        bottomEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
        leftEdgePts_(newConsId, vl, vnl, xc, yc, linPtsLoc);
      }    
      break;
    default:
      break;
    }

    ////add linearization at intersection point
    b1[vnIdx] = iP[0];
    shouldCont = addNewCut_(b1, vlIdx, con, linTermCoeff, error, newConId, nlf);
    if (shouldCont) {
      newcon = rel_->getConstraint(newConId);
      //cUb = newcon->getUb();
    } else {
      newConsId.clear();
      continue;    
    }

    markPoints_(vnIdx, vlIdx, b1, error, xc, yc, linPtsLoc, ptScore,newcon);
    if (error != 0) {
      newConsId.clear();
      linPtsLoc.clear();
      linVioVal.clear();
      xc.clear();
      yc.clear();
      b1[vnIdx] = 0;
      b1[vlIdx] = 0;
      continue;    
    }

    ////// Store new points
    //for (i =0; i < linPtsLoc.size()-1; ++i) {
      //if (ptScore[i] == 4) {
        //if (linPtsLoc[i] == 'b' || linPtsLoc[i] == 't') {
          //addNewBoundaryPt_(linPtsLoc[i], xNew, yNew, newcon, vl, vnl);        
        //}      
      //} else if (ptScore[i] == 3 || ptScore[i] == 2) {
        //insertNewPt_(xc[i], yc[i], xc[i+1], yc[i+1], xNew, yNew, newcon, vl, vnl);
      //}
    //}
    //////For last entry
   //if (ptScore[i] == 4) {
     //if (linPtsLoc[i] == 'b' || linPtsLoc[i] == 't') {
       //addNewBoundaryPt_(linPtsLoc[i], xNew, yNew, newcon, vl, vnl);        
     //}      
   //} else if (ptScore[i] == 3 || ptScore[i] == 2) {
     //insertNewPt_(xc[i], yc[i], xc[0], yc[0], xNew, yNew, newcon, vl, vnl);
   //}  
   
   //linVioVal.resize(linPtsLoc.size());
    ////insert and delete      
    //ptCount = 0, j = 0, i = 0;
    //while (ptCount < ptScore.size()) {
      //if (ptScore[ptCount] == 1) {
        //linPtsLoc.erase(linPtsLoc.begin() + i);
        //linVioVal.erase(linVioVal.begin() + i);
        //xc.erase(xc.begin() + i);
        //yc.erase(yc.begin() + i);
      //} else if (ptScore[ptCount] == 2) {
        //act = getVio_(b1, xc[i], yc[i], con, vlIdx, vnIdx);
        //linVioVal[i] = act;        
        //if (i == linPtsLoc.size()-1) {
          //xc.push_back(xNew[j]);
          //yc.push_back(yNew[j]);
          //linPtsLoc.push_back('i');
          //act = getVio_(b1, xNew[j], yNew[j], con, vlIdx, vnIdx);
          //linVioVal.push_back(act);        
        //} else {
          //linPtsLoc.insert(linPtsLoc.begin()+i+1,'i');
          //xc.insert(xc.begin()+i+1,xNew[j]);
          //yc.insert(yc.begin()+i+1,yNew[j]);
          //act = getVio_(b1, xNew[j], yNew[j], con, vlIdx, vnIdx);
          //linVioVal.insert(linVioVal.begin()+i+1,act);
          //++j, i = i + 2;
        //}    
      //} else if (ptScore[ptCount] == 3 || ptScore[ptCount] == 4) {
          //linPtsLoc[i] = 'i';        
          //xc[i] = xNew[j];
          //yc[i] = yNew[j];
          //act = getVio_(b1, xNew[j], yNew[j], con, vlIdx, vnIdx);
          //linVioVal[i] = act;        
        //++j, ++i;
      //} else {
        //act = getVio_(b1, xc[i], yc[i], con, vlIdx, vnIdx);
        //linVioVal[i] = act;        
        //++i;
      //}
      //++ptCount;      
    //}
     
    //xNew.clear(); 
    //yNew.clear(); 
    //ptScore.clear();
    ////// Add 5 rounds of cuts based on violation value
    //UInt maxVioIdx;
    //double maxVio;
    //for (UInt k =0; k < numCuts_; ++k) {
      //// Find max vio point
      //maxVio = *(std::max_element(linVioVal.begin(), linVioVal.end()));
      //if (maxVio < con->getUb() + solAbsTol_) { //MS: check if this tol is correct
        //break;      
      //}
      //maxVioIdx = std::max_element(linVioVal.begin(), linVioVal.end())-linVioVal.begin();
      //x = xc[maxVioIdx];
      //y = yc[maxVioIdx];
      //// Add cut to the point with max Vio (MS: add a function for this part
      //// later)
      //b1[vnIdx] = x;
      //act = nlf->eval(b1, &error); 
      //if (error == 0 && linTermCoeff != 0) {
        //b1[vlIdx] = con->getUb()- act;    
        //newConId = addCutAtRoot_(b1, con);
        //newcon = rel_->getConstraint(newConId);
        //cUb = newcon->getUb();
      //} else {
        //exit(1);      
      //}

      ////// Update linPts and add vio of new points
    
      //ptCount = 0, i = 0, x = xc[i], y = yc[i], lTemp = linPtsLoc[i];
      //ptScore.resize(linPtsLoc.size());
      //b1[vnIdx] = x;
      //b1[vlIdx] = y;
      //act = newcon->getActivity(b1, &error);
      //if (error == 0) {
        //if ((act > cUb + solAbsTol_) &&
          //(cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
          //isVio = 1;
          //if (lTemp == 'b' || lTemp == 't') {
            //ptScore[ptCount] = 4;
          //}
          //ptScore[ptCount] = 1;
        //} else {
          //isVio = 0;
          //ptScore[ptCount] = 0;
        //}
      //}  
     //firstVio = isVio;
     //++i;
     //for (; i < linPtsLoc.size()-1; ++i) {
      //++ptCount;
      //ptScore[ptCount] = 0;
      //x = xc[i], y = yc[i], lTemp = linPtsLoc[i];
      //b1[vnIdx] = x;
      //b1[vlIdx] = y;
      //act = newcon->getActivity(b1, &error);
      //if (error == 0) {
        //if ((act > cUb + solAbsTol_) &&
          //(cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
          //if (lTemp == 'b' || lTemp == 't') {
            //ptScore[ptCount] = 4;
          //} else {
            //if (isVio == 0) {
              //ptScore[ptCount-1] = ptScore[ptCount-1] + 2;
            //}
            //ptScore[ptCount] = ptScore[ptCount] + 1;
          //}
          //isVio = 1;
        //} else {
          //if (isVio != 0) {
            //ptScore[ptCount-1] = ptScore[ptCount-1] + 2;
          //}
          //isVio = 0;
        //}
      //}   
    //}  
    //++ptCount;
    //ptScore[ptCount] = 0;
    //x = xc[i], y = yc[i], lTemp = linPtsLoc[i];
    //b1[vnIdx] = x;
    //b1[vlIdx] = y;
    //act = newcon->getActivity(b1, &error);
    //if (error == 0) {
      //if ((act > cUb + solAbsTol_) &&
        //(cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        //if (lTemp == 'b' || lTemp == 't') {
          //ptScore[ptCount] = 4;
        //} else {
          //if (firstVio == 0) {
            //ptScore[ptCount] = 2;
          //}
          //ptScore[ptCount] = 1;
        //}
      //} else {
        //if (firstVio != 0) {
          //ptScore[ptCount] = 2;  
        //}
      //}
    //}
    
    ////// Store new points
    //for (i =0; i < linPtsLoc.size()-1; ++i) {
      //if (ptScore[i] == 4) {
        //if (linPtsLoc[i] == 'b' || linPtsLoc[i] == 't') {
          //addNewBoundaryPt_(linPtsLoc[i], xNew, yNew, newcon, vl, vnl);        
        //}      
      //} else if (ptScore[i] == 3 || ptScore[i] == 2) {
        //insertNewPt_(xc[i], yc[i], xc[i+1], yc[i+1], xNew, yNew, newcon, vl, vnl);
      //}
    //}
    //////For last entry
 
     //if (ptScore[i] == 4) {
       //if (linPtsLoc[i] == 'b' || linPtsLoc[i] == 't') {
       //addNewBoundaryPt_(linPtsLoc[i], xNew, yNew, newcon, vl, vnl);        
     //}      
   //} else if (ptScore[i] == 3 || ptScore[i] == 2) {
     //insertNewPt_(xc[i], yc[i], xc[0], yc[0], xNew, yNew, newcon, vl, vnl);
   //}  
          
      //////insert and delete      
     //ptCount = 0, j = 0, i = 0;
     //while (ptCount < ptScore.size()) {
      //if (ptScore[ptCount] == 1) {
        //linPtsLoc.erase(linPtsLoc.begin() + i);
        //linVioVal.erase(linVioVal.begin() + i);
        //xc.erase(xc.begin() + i);
        //yc.erase(yc.begin() + i);
      //} else if (ptScore[ptCount] == 2) {
        //act = getVio_(b1, xc[i], yc[i], con, vlIdx, vnIdx);
        //linVioVal[i] = act;        
        //if (i == linPtsLoc.size()-1) {
          //xc.push_back(xNew[j]);
          //yc.push_back(yNew[j]);
          //linPtsLoc.push_back('i');
          //act = getVio_(b1, xNew[j], yNew[j], con, vlIdx, vnIdx);
          //linVioVal.push_back(act);        
        //} else {
          //linPtsLoc.insert(linPtsLoc.begin()+i+1,'i');
          //xc.insert(xc.begin()+i+1,xNew[j]);
          //yc.insert(yc.begin()+i+1,yNew[j]);
          //act = getVio_(b1, xNew[j], yNew[j], con, vlIdx, vnIdx);
          //linVioVal.insert(linVioVal.begin()+i+1,act);
          //++j, i = i + 2;
        //}    
      //} else if (ptScore[ptCount] == 3 || ptScore[ptCount] == 4) {
          //linPtsLoc[i] = 'i';        
          //xc[i] = xNew[j];
          //yc[i] = yNew[j];
          //act = getVio_(b1, xNew[j], yNew[j], con, vlIdx, vnIdx);
          //linVioVal[i] = act;        
        //++j, ++i;
      //} else {
        //act = getVio_(b1, xc[i], yc[i], con, vlIdx, vnIdx);
        //linVioVal[i] = act;
        //++i;
      //}
      //++ptCount;      
    //}
    
    //xNew.clear(); 
    //yNew.clear(); 
    //ptScore.clear();
    //} // for loop end

   
    //std::cout << "stats_->cuts " << numCutCount_ << std::endl;  //MS: make
    //use of stats_cut
    newConsId.clear();
    linPtsLoc.clear();
    linVioVal.clear();
    xc.clear();
    yc.clear();
    b1[vnIdx] = 0;
    b1[vlIdx] = 0;
  }

  delete [] b1;
  return;
}

void QGHandler::markPoints_(UInt vnIdx, UInt vlIdx, double * b1, int & error, std::vector<double > xc, std::vector<double >yc, std::vector<char > linPtsLoc, std::vector<UInt > & ptScore, ConstraintPtr newcon)
{
  double act;
  bool isVio, firstVio;
  UInt ptCount = 0, i = 0;
  double x = xc[i], y = yc[i], cUb = newcon->getUb();
  char lTemp = linPtsLoc[i];
  ptScore.resize(linPtsLoc.size());
  b1[vnIdx] = x, b1[vlIdx] = y;
  act = newcon->getActivity(b1, &error);
  if (error == 0) {
    if ((act > cUb + solAbsTol_) &&
      (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
      isVio = 1;
      if (lTemp == 'b' || lTemp == 't') {
        ptScore[ptCount] = 4;
      }
      ptScore[ptCount] = 1;
    } else {
      isVio = 0;
      ptScore[ptCount] = 0;
    }
  } else {
    return;  
  } 
  firstVio = isVio;
  ++i;
  ////ptScore = 0 nothing, 1 only delete, 2 only insert, 3 delete and insert, 4 boundary to be deleted and new to be inserted 
  for (; i < linPtsLoc.size()-1; ++i) {
    ++ptCount;
    ptScore[ptCount] = 0;
    x = xc[i], y = yc[i], lTemp = linPtsLoc[i];
    b1[vnIdx] = x;
    b1[vlIdx] = y;
    act = newcon->getActivity(b1, &error);
    if (error == 0) {
      if ((act > cUb + solAbsTol_) &&
        (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        if (lTemp == 'b' || lTemp == 't') {
          ptScore[ptCount] = 4;
        } else {
          if (isVio == 0) {
            ptScore[ptCount-1] = ptScore[ptCount-1] + 2;
          }
          ptScore[ptCount] = ptScore[ptCount] + 1;
        }
        isVio = 1;
      } else {
        if (isVio != 0) {
          ptScore[ptCount-1] = ptScore[ptCount-1] + 2;
        }
        isVio = 0;
      }
    } else {
      return;    
    }
  }
  ++ptCount;
  ptScore[ptCount] = 0;
  x = xc[i], y = yc[i], lTemp = linPtsLoc[i];
  b1[vnIdx] = x;
  b1[vlIdx] = y;
  act = newcon->getActivity(b1, &error);
  if (error == 0) {
    if ((act > cUb + solAbsTol_) &&
      (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
      if (lTemp == 'b' || lTemp == 't') {
        ptScore[ptCount] = 4;
      } else {
        if (firstVio == 0) {
          ptScore[ptCount] = 2;
        }
        ptScore[ptCount] = 1;
      }
    } else {
      if (firstVio != 0) {
        ptScore[ptCount] = 2;  
      }
    }
  } else {
    return;  
  }
}


double QGHandler::getVio_(double *b1, double x, double y, ConstraintPtr con, UInt vlIdx, UInt vnIdx)
{
  int error = 0;
  double act = INFINITY;  
  b1[vnIdx] = x;
  b1[vlIdx] = y;
  act = con->getActivity(b1, &error);
  if (error == 0) {
    return act; // MS: act can be errored based on else of above if
  }
}

void QGHandler::bottomEdgePts_(std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl, std::vector<double > & xc, std::vector<double > & yc, std::vector<char> & linPtsLoc)
{
  ConstraintPtr con;
  LinearFunctionPtr lf;
  bool addP1 = false, addP2 = false;
  double nVarVal0, lVarVal0, nVarVal1, lVarVal1;
  double lbN = vnl->getLb(), lbL = vl->getLb(), ubN = vnl->getUb();
  double p1[2], p2[2];
  
  con = rel_->getConstraint(newConsId[0]);
  lf = con->getLinearFunction();
  nVarVal0 = lf->getWeight(vnl);
  lVarVal0 = lf->getWeight(vl);
  
  p1[1] = lbL;
  p1[0] = findLinVarVal_(lbL, lVarVal0, nVarVal0, con->getUb(), lbN);  
  
  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  nVarVal1 = lf->getWeight(vnl);
  lVarVal1 = lf->getWeight(vl);
  
  p2[1] = lbL;
  p2[0] = findLinVarVal_(lbL, lVarVal1, nVarVal1, con->getUb(), lbN);  
  
  findExtrmPts_(addP1, addP2, p1, p1[0], p2, p2[0], lbN, ubN, newConsId, vl, vnl);
  addExtrmPtsBR_(addP1, addP2, p1, p1[0], p2, p2[0], linPtsLoc, xc, yc, 'b');
  return;
}

void QGHandler::topEdgePts_(std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl, std::vector<double > & xc, std::vector<double > & yc, std::vector<char> & linPtsLoc)
{
  ConstraintPtr con;
  LinearFunctionPtr lf;
  bool addP1 = false, addP2 = false;
  double nVarVal0, lVarVal0, nVarVal1, lVarVal1;
  double lbN = vnl->getLb(), ubN = vnl->getUb(), ubL = vl->getUb();
  double p1[2], p2[2];

   
  con = rel_->getConstraint(newConsId[0]);
  lf = con->getLinearFunction();
  nVarVal0 = lf->getWeight(vnl);
  lVarVal0 = lf->getWeight(vl);
  p1[1] = ubL;
  p1[0] = findLinVarVal_(ubL, lVarVal0, nVarVal0, con->getUb(), ubN);  
    
  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  nVarVal1 = lf->getWeight(vnl);
  lVarVal1 = lf->getWeight(vl);
  p2[1] = ubL;
  p2[0] = findLinVarVal_(ubL, lVarVal1, nVarVal1, con->getUb(), ubN);  
  
  findExtrmPts_(addP1, addP2, p1, p1[0], p2, p2[0], lbN, ubN, newConsId, vl, vnl);
  addExtrmPtsTL_(addP1, addP2, p1, p1[0], p2, p2[0], linPtsLoc, xc, yc, 't');

  return;
}
//MS: later combine top and bottom, and left and right

void QGHandler::leftEdgePts_(std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl, std::vector<double > & xc, std::vector<double > & yc, std::vector<char> & linPtsLoc)
{
  ConstraintPtr con;
  LinearFunctionPtr lf;
  bool addP1 = false, addP2 = false;
  double nVarVal0, lVarVal0, nVarVal1, lVarVal1;
  double lbN = vnl->getLb(), lbL = vl->getLb(), ubL = vl->getUb();
  double p1[2], p2[2];
  
  con = rel_->getConstraint(newConsId[0]);
  lf = con->getLinearFunction();
  nVarVal0 = lf->getWeight(vnl);
  lVarVal0 = lf->getWeight(vl);
  
  p1[0] = lbN;
  p1[1] = findLinVarVal_(lbN, nVarVal0, lVarVal0, con->getUb(), ubL);  
  
  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  nVarVal1 = lf->getWeight(vnl);
  lVarVal1 = lf->getWeight(vl);
  p2[0] = lbN;
  p2[1] = findLinVarVal_(lbN, nVarVal1, lVarVal1, con->getUb(), ubL);  
  
  findExtrmPts_(addP1, addP2, p1, p1[1], p2, p2[1], lbL, ubL, newConsId, vl, vnl);
  addExtrmPtsTL_(addP1, addP2, p1, p1[1], p2, p2[1], linPtsLoc, xc, yc, 'l');

  return;
}

void QGHandler::rightEdgePts_(std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl, std::vector<double > & xc, std::vector<double > & yc, std::vector<char > & linPtsLoc)
{
  ConstraintPtr con;
  LinearFunctionPtr lf;
  bool addP1 = false, addP2 = false;
  double nVarVal0, lVarVal0, nVarVal1, lVarVal1;
  double ubN = vnl->getUb(), lbL = vl->getLb(), ubL = vl->getUb();
 
  con = rel_->getConstraint(newConsId[0]);
  lf = con->getLinearFunction();
  nVarVal0 = lf->getWeight(vnl);
  lVarVal0 = lf->getWeight(vl);
  double p1[2], p2[2];
  
  p1[0] = ubN;
  p1[1] = findLinVarVal_(ubN, nVarVal0, lVarVal0, con->getUb(), lbL);  
    
  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  nVarVal1 = lf->getWeight(vnl);
  lVarVal1 = lf->getWeight(vl);
  
  p2[0] = ubN;
  p2[1] = findLinVarVal_(ubN, nVarVal1, lVarVal1, con->getUb(), lbL);  
  
  findExtrmPts_(addP1, addP2, p1, p1[1], p2, p2[1], lbL, ubL, newConsId, vl, vnl);
  addExtrmPtsBR_(addP1, addP2, p1, p1[1], p2, p2[1], linPtsLoc, xc, yc, 'r');
  
  return;
}


void QGHandler::findExtrmPts_(bool &addP1, bool &addP2, double * x, double c1, double * y, double c2, double lb, double ub, std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl)
{
  ConstraintPtr con;
  LinearFunctionPtr lf;
  double nVarVal0, lVarVal0, nVarVal1, lVarVal1, cUb0, cUb1;
  
  con = rel_->getConstraint(newConsId[0]);
  lf = con->getLinearFunction();
  nVarVal0 = lf->getWeight(vnl);
  lVarVal0 = lf->getWeight(vl);
  cUb0 = con->getUb();

  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  nVarVal1 = lf->getWeight(vnl);
  lVarVal1 = lf->getWeight(vl);
  cUb1 = con->getUb();


  double conVio;
  if (c1 >= lb && c1 <= ub) {
      conVio = std::max(nVarVal1*x[0]+lVarVal1*x[1]-cUb1, 0.0); 
      if ( ((conVio < solAbsTol_) || (cUb1 != 0 && conVio < fabs(cUb1)*solRelTol_))) {
        addP1 = true;
      }
  }

  if (c2 >= lb && c2 <= ub) {
      conVio = std::max(nVarVal0*y[0]+lVarVal0*y[1]-cUb0, 0.0); 
      if ( ((conVio < solAbsTol_) || (cUb0 != 0 && conVio < fabs(cUb0)*solRelTol_))) {
        addP2 = true;
      }
  }
  return;
}

void QGHandler::addExtrmPtsBR_(bool addP1, bool addP2, double * p1, double c1, double * p2, double c2, std::vector<char > & linPtsLoc, std::vector<double> &xc, std::vector<double> &yc, char loc)
{
  if (addP1 && addP2) {
    if (c1 < c2) {
      linPtsLoc.push_back(loc);
      xc.push_back(p1[0]);
      yc.push_back(p1[1]);
      
      xc.push_back(p2[0]);
      yc.push_back(p2[1]);
    } else if (c1 > c2) {
      linPtsLoc.push_back(loc);
     
      xc.push_back(p2[0]);
      yc.push_back(p2[1]);   
      
      xc.push_back(p1[0]);
      yc.push_back(p1[1]);
    } else {
      linPtsLoc.push_back(loc);
      xc.push_back(p1[0]);
      yc.push_back(p1[1]);
    } 
  } else if (addP1) {
    linPtsLoc.push_back(loc);
    xc.push_back(p1[0]);
    yc.push_back(p1[1]);
  } else if (addP2) {
    linPtsLoc.push_back(loc);
    xc.push_back(p2[0]);
    yc.push_back(p2[1]); 
  }
}

void QGHandler::addExtrmPtsTL_(bool addP1, bool addP2, double * p1, double c1, double * p2, double c2, std::vector<char > & linPtsLoc, std::vector<double> &xc, std::vector<double> &yc, char loc)
{
  if (addP1 && addP2) {
    if (c1 < c2) {
      linPtsLoc.push_back(loc);
      xc.push_back(p2[0]);
      yc.push_back(p2[1]);   
      
      xc.push_back(p1[0]);
      yc.push_back(p1[1]);
    } else if (c1 > c2) {
      linPtsLoc.push_back(loc);
      xc.push_back(p1[0]);
      yc.push_back(p1[1]);
      
      xc.push_back(p2[0]);
      yc.push_back(p2[1]);  
    } else {
      linPtsLoc.push_back(loc);
      xc.push_back(p1[0]);
      yc.push_back(p1[1]);
    } 
  } else if (addP1) {
    linPtsLoc.push_back(loc);
    xc.push_back(p1[0]);
    yc.push_back(p1[1]);
  } else if (addP2) {
    linPtsLoc.push_back(loc);
    xc.push_back(p2[0]);
    yc.push_back(p2[1]);
  }
}
double QGHandler::findLinVarVal_(double x, double a, double b, double c, double d)
{
  /* solving for y
   * a*x + b*y = c 
   */
  double p;
  // Points on bottom edge of the box
  if (fabs(b) > solAbsTol_) {
    p = (c - a*x)/b;
  } else {
    p = d;
  }
  return p;
}



int QGHandler::findIntersectPt(std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl, double * iP)
{
  ConstraintPtr con;
  con = rel_->getConstraint(newConsId[0]);
  LinearFunctionPtr lf;
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
  }

  //MS: remove cases that cannot happen.
  if ((iP[0] > vnl->getLb() && iP[0] < vnl->getUb())) {
    if ((iP[1] > vl->getLb() && iP[1] < vl->getUb())) {
      return 1; // inside the box
    } else if (iP[1] == vl->getLb()) {
      return 2; // bottomEdge of the box
    } else if (iP[1] == vl->getUb()) {
      return 3; // topEdge of the box
    }
  } else if ( iP[0] == vnl->getLb()) {
    if ((iP[1] > vl->getLb() && iP[1] < vl->getUb())) {
      return 4; // leftEdge edge of the box
    } else if (iP[1] == vl->getLb()) {
      return 5;  // bottomLeftCorner of the box
    } else if (iP[1] == vl->getUb()) {
      return 6;  // topLeftCorner of the box
    }    
  } else if ( iP[0] == vnl->getUb() ) {
    if ((iP[1] > vl->getLb() && iP[1] < vl->getUb())) {
      return 7; // rightEdge of the box
    } else if (iP[1] == vl->getLb()) {
      return 8; // bottomRightCorner of the box
    } else if (iP[1] == vl->getUb()) {
      return 9; // topRightCorner of the box
    } 
  }
  return 0; // outside of the box
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
    //std::cout << "id x and grad " << std::endl;
  //for (UInt i = 0; i< n; ++i) {
    //std::cout << i << " " << x[i] << " " << a[i] << std::endl;
  //}
  
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
 

void QGHandler::objCutAtLpSol_(const double *lpx, CutManager *,
                                  SeparationStatus *status)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    double c, vio, act;
    ConstraintPtr newcon;
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
            newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
          }
        }
    }	else {
      logger_->msgStream(LogError) << me_
        << " objective not defined at this solution point." << std::endl;
    }
  }
  return;
}


void QGHandler::consCutAtLpSol_(const double *lpx, CutManager *,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  ConstraintPtr con, newcon;
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
          newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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


void QGHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
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
  

void QGHandler::cutToObj_(const double *nlpx, const double *lpx,
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
  //std::cout << "At node" << node->getId() << " LB " << node->getLb() << std::endl;
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


void QGHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval,
                          bool *sol_found)
{
  double val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();

  if ((bestval - val > objATol_) &&
        (bestval == 0 || (bestval - val >= fabs(bestval)*objRTol_))) {
  //if (val <= bestval) 
    //MS: adding only better solution to pool
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, val);
    *sol_found = true;
    //std::cout << "upper bound found " << val << std::endl;
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
    << stats_->nlpF << std::endl
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
