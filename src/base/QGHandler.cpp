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
  nlpe1_(EnginePtr()),
  lpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rootLinScheme3_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  solC_(NULL),
  solNLP_(NULL),
  stats_(0)
{
  rScheme1Para_ = env_->getOptions()->findInt("root_LinScheme1")->getValue();
  rScheme3Para_ = env_->getOptions()->findInt("root_LinScheme3")->getValue();
  rScheme4Para_ = env_->getOptions()->findInt("root_LinScheme4")->getValue();
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
  nlpe1_(EnginePtr()),
  lpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rootLinScheme3_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  solC_(NULL),
  solNLP_(NULL)
{
  rScheme1Para_ = env_->getOptions()->findInt("root_LinScheme1")->getValue();
  rScheme3Para_ = env_->getOptions()->findInt("root_LinScheme3")->getValue();
  rScheme4Para_ = env_->getOptions()->findInt("root_LinScheme4")->getValue();
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
      return newcon->getIndex();
      sstm.str("");
    }
  }	else {
    logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
      " is not defined at this point." << std::endl;
  }
  return 0;
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

bool QGHandler::diffFunVarVal_(const double *x, FunctionPtr f)
{
  UInt idx; 
  for(VariableSet::iterator vit=f->varsBegin(); vit!=f->varsEnd(); ++vit) {
    idx = (*vit)->getIndex();
    if ((fabs(x[idx]-solNLP_[idx]) > solAbsTol_) &&
          (solNLP_[idx] != 0 || fabs(x[idx]-solNLP_[idx]) > fabs(solNLP_[idx])*solRelTol_)) {
      return true;
      break;          
    }
  }
  return false;
}

void QGHandler::addInitLinearX_(const double *x, bool isSecNLP)
{ 
  int error=0;
  bool addCut;
  UInt newcuts = 0; //MS: for testing
  FunctionPtr f;
  double c, act, cUb;
  std::stringstream sstm;
  ConstraintPtr con;
  //ConstraintPtr newcon;
  LinearFunctionPtr lf = LinearFunctionPtr();
  //std::cout << "isSecNLP " << isSecNLP << std::endl;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      if (isSecNLP) {
        addCut = diffFunVarVal_(x, f);
        // Add cut only if the new point is different than the previous point
        if (addCut == false) {
          continue;
        }
        ++newcuts;
      }
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
        //if (rootLinScheme3_ && twoVarsCon_(con)) { //MS: for scheme 4?
          ////act = con->getActivity(x, &error);
          ////if ((fabs(act-cUb) <= solAbsTol_) ||
              ////(cUb!=0 && (fabs(act-cUb) <= fabs(cUb)*solRelTol_))) {
            //rootScheme4_(x, con); //MS: try applying for all cons
            ////rootScheme3_(x, con, lf);
          ////MS: same technique on the LP solution as well
          ////}
        //}
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
      if (isSecNLP) {
        addCut = diffFunVarVal_(x, f);
        // Add cut only if the new point is different than the previous point
        if (addCut == false) {
          std::cout << "No. of warmstart cuts " << newcuts << std::endl; // testing
          return;
        }
        ++newcuts;
      }
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
        
  if (isSecNLP) {
    std::cout << "No. of warmstart cuts " << newcuts << std::endl;
  }
  return;
}


void QGHandler::alphaSelect_(VariablePtr nVar, VariablePtr lVar, double d1, double d2, 
                             const double * nlpx, double &minA, double &maxA)
{
  double l1, l2, u1, u2;
  UInt vnIdx = nVar->getIndex(), vlIdx = lVar->getIndex();
  if (d1 > 0) {
    l1 = (nVar->getLb() - nlpx[vnIdx])/fabs(d1);
    u1 = (nVar->getUb() - nlpx[vnIdx])/fabs(d1);
  } else {
    l1 = (nlpx[vnIdx]-nVar->getUb())/fabs(d1);
    u1 = (nlpx[vnIdx]-nVar->getLb())/fabs(d1);
  } 
  if (d2 > 0) {
    l2 = (lVar->getLb() - nlpx[vlIdx])/fabs(d2);
    u2 = (lVar->getUb() - nlpx[vlIdx])/fabs(d2);
  } else {
    l2 = (nlpx[vlIdx]-lVar->getUb())/fabs(d2);
    u2 = (nlpx[vlIdx]-lVar->getLb())/fabs(d2);
  }

  minA = std::max(l1, l2);
  maxA = std::min(u1, u2);
  //minA = std::max(minA, -3.0);
  //maxA = std::min(maxA, 3.0);
}

// MS: consistently name the schemes
void QGHandler::rootScheme3_(const double *nlpx, ConstraintPtr con,
                              LinearFunctionPtr lf)
{
  int error = 0;
  UInt vnIdx, vlIdx, newConId;
  VariablePtr nVar, lVar;
  NonlinearFunctionPtr nlf;
  UInt n = minlp_->getNumVars();
  double c1, c2, d1, d2, alpha, minA, maxA;
  double linTermCoeff = con->getLinearFunction()->termsBegin()->second;
  if (lf->getNumTerms() == 2) {
    nlf = con->getNonlinearFunction();
    nVar = (*(nlf->varsBegin())); // var in nonlinear term
    lVar = (con->getLinearFunction()->termsBegin()->first); // var in lin term
    vnIdx = nVar->getIndex(), vlIdx = lVar->getIndex();
    VariableGroupConstIterator it = lf->termsBegin();
    if (it->first->getIndex() == vnIdx) {
      c1 = it->second;
      ++it;
      c2 = it->second;
    } else {
      c2 = it->second;
      ++it;
      c1 = it->second;   
    }
    double* npt = new double[n];
    std::fill(npt, npt+n, 0.);
    if (c1*c2 != 0) {
      d1 = fabs(nlpx[vnIdx]) + 1;
      d2 =  (-c1* d1)/c2;
      d1 = d1/sqrt(d1*d1 + d2*d2), d2 = d2/sqrt(d1*d1 + d2*d2); // unit vectors
      alphaSelect_(nVar, lVar, d1, d2, nlpx, minA, maxA);
      double k = fabs(minA)/rScheme3Para_; //MS: make this division by value also a parameter
      alpha = minA;
      while (alpha < 0) {
        npt[vnIdx] = nlpx[vnIdx] + alpha*d1; 
        addNewCut_(npt, vlIdx, con, linTermCoeff, error, newConId, nlf);
        alpha = alpha + k;
      }
      k = fabs(maxA)/rScheme3Para_;
      alpha = maxA;
      while (alpha > 0) {
        npt[vnIdx] = nlpx[vnIdx] + alpha*d1; 
        addNewCut_(npt, vlIdx, con, linTermCoeff, error, newConId, nlf);
        alpha = alpha - k;
      }
    }
    delete [] npt;
  }
}

//MS: variation of rootLinScheme3 to add cut even for inactive cons
void QGHandler::rootScheme4_(const double *nlpx, ConstraintPtr con)
{
  int error = 0;
  UInt vnIdx, vlIdx, newConId;
  VariablePtr nVar, lVar;
  NonlinearFunctionPtr nlf;
  UInt n = minlp_->getNumVars();
  double alpha, minA, maxA;
  double linTermCoeff = con->getLinearFunction()->termsBegin()->second;
  //if (lf->getNumTerms() == 2) {
  nlf = con->getNonlinearFunction();
  nVar = (*(nlf->varsBegin())); // var in nonlinear term
  lVar = (con->getLinearFunction()->termsBegin()->first); // var in lin term
  vnIdx = nVar->getIndex(), vlIdx = lVar->getIndex();
  double* npt = new double[n];
  std::fill(npt, npt+n, 0.);
  minA = nlpx[vnIdx] - nVar->getLb();
  maxA = nVar->getLb() - nlpx[vnIdx];
  double k = fabs(minA)/rScheme4Para_; //MS: make this division by value also a parameter
  alpha = nVar->getLb();
  while (alpha < nlpx[vnIdx]) {
    npt[vnIdx] = nlpx[vnIdx] + alpha; 
    addNewCut_(npt, vlIdx, con, linTermCoeff, error, newConId, nlf);
    alpha = alpha + k;
  }
  k = fabs(maxA)/rScheme4Para_;
  alpha = nVar->getUb();
  while (alpha > nlpx[vnIdx]) {
    npt[vnIdx] = nlpx[vnIdx] + alpha; 
    addNewCut_(npt, vlIdx, con, linTermCoeff, error, newConId, nlf);
    alpha = alpha - k;
  }
  delete [] npt;
  //}
}

void QGHandler::setLpEngine(EnginePtr lpe)
{
  lpe_ = lpe->emptyCopy();
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


void QGHandler::initLinear_(bool *isInf, bool isSecNLP)
{
  *isInf = false;
  const double *x;
  
  //nlpe_->load(minlp_); // loading original problem to NLP engine
  solveNLP_();
  //std::cout << "minlp_ \n";
  //minlp_->write(std::cout);
  
  if (isSecNLP == false) {
    solNLP_ =  nlpe_->getSolution()->getPrimal();  
  }
  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    x = nlpe_->getSolution()->getPrimal();
    addInitLinearX_(x, isSecNLP);
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    x = nlpe_->getSolution()->getPrimal();
    addInitLinearX_(x, isSecNLP);
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
  double x1 = xc[j], y1 = yc[j], x2 = xc[k], y2 = xc[k], x, y;

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

//MS: make sure that this routine is called when constraints has exactly two
//variables
void QGHandler::rootLinearizations_()
{
  //MS: make these schemes option based
  //rootLinScheme1_();  // 2 var constraints linearizations
  rootLinScheme2_();  // warm starting NLP from LP solution
  //rootLinScheme3_();  // add linearizations near root NLP solution
  // Just set a parameter for scheme 3
  //rootLinScheme3_ = true;
  //if (rootLinScheme3_) {
    //bool isInf;
    //findCenter_(&isInf);
    //if (isInf == true) {
      //rootLinScheme3_ = false;    
    //} 
  //}
}

void QGHandler::rootLinScheme2_()
{
  bool isInf;
  lpe_->load(rel_);
  lpe_->solve();
  //nlpe_->setWarmStartPt(NULL);
  minlp_->setInitialPoint(lpe_->getSolution()->getPrimal());
  //nlpe_->loadFromWarmStart(lpe_->getWarmStartCopy());
  initLinear_(&isInf, 1);
}

bool QGHandler::twoVarsCon_(ConstraintPtr con)
{
  UInt vnIdx, vlIdx = 0;
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf;
  lf = con->getLinearFunction();
  nlf = con->getNonlinearFunction();
  vnIdx = nlf->numVars();
  if (vnIdx != 1) {
    return false;        
  }
  if (lf) {
    for(VariableGroupConstIterator vit = lf->termsBegin(); vit != lf->termsEnd(); ++vit) {
      if (fabs(vit->second) > 1e-6) {
        ++vlIdx;          
      }
      if (vlIdx > 1) {
        return false;
      }
    }
  }
  if (vnIdx+vlIdx != 2) {
    return false;    
  }
  return true;
}


void QGHandler::rootLinScheme1_()
{
  //MS: check this functionality once
  FunctionPtr f;
  VariablePtr vnl, vl;
  bool shouldCont;
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf;
  ConstraintPtr con, newcon;
  UInt vnIdx, vlIdx, newConId, numCuts;
  int error = 0, n = rel_->getNumVars(), i;
  double act, cUb, linTermCoeff, y1, y2, maxVio;
  double *b1 = new double[n];
  double iP[2]; //intersection point
  std::vector<UInt > newConsId;
  std::vector<double > linVioVal, xc, yc; //xc - nonlinear var, yc: lin var
  std::fill(b1, b1+n, 0.);
 
  std::cout << "No. of nonlinear cons and root lins added: " << nlCons_.size(); 
  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    numCuts = 0;
    shouldCont = twoVarsCon_(con); //MS: pass directly lf or nlf
    // constraints with extactly two vars, one in nonlinear and one in linear
    // part, is considered
    //lf = con->getLinearFunction();
    nlf = con->getNonlinearFunction();
    //vnIdx = nlf->numVars();
    //if (vnIdx != 1) {
      //continue;        
    //}
    //vlIdx = 0;
    //if (lf) {
      //for(VariableGroupConstIterator vit = lf->termsBegin(); vit != lf->termsEnd(); ++vit) {
        //if (fabs(vit->second) > 1e-6) {
          //++vlIdx;          
        //}
        //if (vlIdx > 1) {
          //shouldCont = false;
          //break;        
        //}
      //}
    //}
    //if (shouldCont == false || (vnIdx+vlIdx != 2) ) {
      //continue;    
    //}

    if (shouldCont == false) {
      continue;    
    }
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
    numCuts++;
    if (shouldCont) {
      y1 = b1[vlIdx];
      newConsId.push_back(newConId); // MS: Add cutmanager later
    } else {
      continue;    
    }

    // upper bound of var in nonlinear cons
    b1[vnIdx] = vnl->getUb();
    shouldCont = addNewCut_(b1, vlIdx, con, linTermCoeff, error, newConId, nlf);
    numCuts++;
    if (shouldCont) {
      y2 = b1[vlIdx];
      newConsId.push_back(newConId); // MS: Add cutmanager later
    } else {
      newConsId.clear();
      continue;    
    }   
    // Find intersection point and its location. In iPtAt index 0 is for
    // nonlinear and 1 is for linear var
    findIntersectPt(newConsId, vl, vnl, iP, shouldCont);
    if (shouldCont == false) {
      newConsId.clear();
      continue;    
    }

    // populate points in an order and the violation value at the points
    xc.push_back(vnl->getLb()), yc.push_back(y1), linVioVal.push_back(0);
        
    b1[vnIdx] = iP[0], b1[vlIdx] = iP[1];;
    act = getVio_(b1, con, error);
    if (error != 0) {
      newConsId.clear();
      xc.clear(), yc.clear();
      continue;    
    }
    xc.push_back(iP[0]), yc.push_back(iP[1]), linVioVal.push_back(act);
    xc.push_back(vnl->getUb()), yc.push_back(y2), linVioVal.push_back(0);

    i = 1; // starting from intersection point
    for (UInt k = 0; k < rScheme1Para_; ++k) {
      //add a new cut at the point indexed i
      b1[vnIdx] = xc[i];
      shouldCont = addNewCut_(b1, vlIdx, con, linTermCoeff, error, newConId, nlf);
      numCuts++;
      if (shouldCont) {
        newcon = rel_->getConstraint(newConId);
        cUb = newcon->getUb();
      } else {
        break;    
      }
      
      // Move right and determine first point that satisfy the new cons
      for (UInt j = i+1; j < xc.size(); ) {
        b1[vnIdx] = xc[j], b1[vlIdx] = yc[j];
        act = newcon->getActivity(b1, &error);
        if (error == 0) {
          if ((act < cUb + solAbsTol_) ||
              (cUb =! 0 && act < cUb + fabs(cUb)*solRelTol_)) {
              //insert new point just before index j
            insertNewPt_(j, j-1, xc, yc, newcon, vl, vnl, shouldCont); 
            b1[vnIdx] = xc[j], b1[vlIdx] = yc[j];
            act = getVio_(b1, con, error);
            if (error != 0) {
              shouldCont = 0;    
            }
            break;
          } else {
            // delete if violated
            xc.erase(xc.begin() + j), yc.erase(yc.begin() + j);        
          }  
        }
      }
      if (shouldCont == false) {
        break;
      }
      for (int j = i-1; j >= 0; --j) {
        b1[vnIdx] = xc[j], b1[vlIdx] = yc[j];
        act = newcon->getActivity(b1, &error);
        if (error == 0) {
          if ((act < cUb + solAbsTol_) ||
              (cUb =! 0 && act < cUb + fabs(cUb)*solRelTol_)) {
            insertNewPt_(j+1, j, xc, yc, newcon, vl, vnl, shouldCont); 
            b1[vnIdx] = xc[j+1], b1[vlIdx] = yc[j+1];
            act = getVio_(b1, con, error);
            if (error != 0) {
              shouldCont = 0;    
            }
            xc.erase(xc.begin()+j+2), yc.erase(yc.begin()+j+2);        
            break;
          }  else {
            // delete if violated
            xc.erase(xc.begin() + j), yc.erase(yc.begin() + j);        
          }  
        }
      }
      if (shouldCont == false) {
        break;
      }
      maxVio = *(std::max_element(linVioVal.begin(), linVioVal.end()));
      cUb = con->getUb();
      if ((maxVio < cUb + solAbsTol_) ||
              (cUb =! 0 && maxVio < cUb + fabs(cUb)*solRelTol_) ) { 
        break;      
      }
      i = std::max_element(linVioVal.begin(), linVioVal.end())-linVioVal.begin();     
    }
    std::cout << " " << numCuts;
    xc.clear();
    yc.clear();
    b1[vnIdx] = 0;
    b1[vlIdx] = 0;
    linVioVal.clear();
    newConsId.clear();
  }

  std::cout << std::endl;
  delete [] b1;
  return;
}


double QGHandler::getVio_(double *b1, ConstraintPtr con, int & error)
{
  double act = INFINITY;  
  act = con->getActivity(b1, &error);
  return act; 
}


void QGHandler::findIntersectPt(std::vector<UInt > newConsId, VariablePtr vl, VariablePtr vnl, double * iP, bool & shouldCont)
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
   shouldCont = false; 
  }
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


void QGHandler::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con, CutManager*,
                        SeparationStatus *status)
{
  int error=0;
  //ConstraintPtr newcon;
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
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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
  nlpe_->load(minlp_); // loading original problem to NLP engine
  //rootLinScheme3_ = true; // set from environment option: MS: for scheme  4???
  initLinear_(isInf, 0);
  rootLinearizations_();
  return;
}


void QGHandler::separate(ConstSolutionPtr sol, NodePtr, RelaxationPtr rel,
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
