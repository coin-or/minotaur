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
#include <iomanip>
#include <iostream>
#include <fstream>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "ConBoundMod.h"
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
#include "QuadraticFunction.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string QGHandler::me_ = "QGHandler: ";

QGHandler::QGHandler()
: env_(EnvPtr()),      
  minlp_(ProblemPtr()),
  nlCons_(0),
  nlpe_(EnginePtr()),
  nlpStatus_(EngineUnknownStatus),
  nlpWs_(WarmStartPtr()),
  objFun_(FunctionPtr()),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  stats_(0),
  cpr_(0),
  bpr_(0),
  spr_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger(LogInfo);
}

QGHandler::QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe) 
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  nlpWs_(WarmStartPtr()),
  objFun_(FunctionPtr()),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  cpr_(0),
  bpr_(0),
  spr_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_   = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->cuts = 0;
}
 

QGHandler::QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe,
            std::vector<ConstConstraintPtr> cpr, 
            std::vector<ConstVariablePtr> bpr, std::vector<char> spr)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  nlpWs_(WarmStartPtr()),
  objFun_(FunctionPtr()),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  cpr_(cpr),
  bpr_(bpr),
  spr_(spr)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_   = new QGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->cuts = 0;
}


QGHandler::~QGHandler()
{ 
  if (stats_) {
    delete stats_;
  }
  env_.reset();
  rel_.reset();
  nlpe_.reset();
  nlpWs_.reset();
  minlp_.reset();
  objFun_.reset();
  objVar_.reset();
}


void QGHandler::addInitLinearX_(const double *x)
{ 
  double c;
  int error=0;
  double act = 0;
  ObjectivePtr o;
  FunctionPtr f, f2;
  ConstraintPtr con, newcon;
  LinearFunctionPtr lf = LinearFunctionPtr();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) { 
    con = *it;
    act = con->getActivity(x, &error);
    if(error==0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf);
      f2 = (FunctionPtr) new Function(lf);
      if (con->getUb() < INFINITY) {
        newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c,
                                     "lnrztn_cut");
        ++(stats_->cuts);
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "initial constr. cut: ";
        newcon->write(logger_->msgStream(LogDebug));
#endif
      }

      if (con->getLb() > -INFINITY) {
        newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY,
                                     "lnrztn_cut");
        ++(stats_->cuts);  

#if SPEW
        logger_->msgStream(LogDebug) << me_ << "initial constr. cut: ";
        newcon->write(logger_->msgStream(LogDebug));
#endif
      }
    }	else {
      logger_->msgStream(LogError) << me_ 
        << "Constraint is not defined at this point" << std::endl;
    } 
  }
  error=0;  
  o = minlp_->getObjective();
  if (oNl_ && o) {
    f = o->getFunction();
    act = o->eval(x, &error);
    if (error==0) {
      linearAt_(f, act, x, &c, &lf);
      lf->addTerm(objVar_, -1.0);
      f2 = (FunctionPtr) new Function(lf);
      newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut");
      ++(stats_->cuts);
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "initial obj cut: " << std::endl
        << std::setprecision(9);
      newcon->write(logger_->msgStream(LogDebug));
#endif
    }	else {
      logger_->msgStream(LogError) << me_ 
        << "Objective not defined at this point" << std::endl;
    }
  } else {
    logger_->msgStream(LogDebug) << "QG Handler: Problem does not have a " 
      << "nonlinear objective." << std::endl;
  }
  return;
}


void QGHandler::cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                           bool *sol_found, SeparationStatus *status)
{
  UInt numCuts = 0;
  const double *nlp_x;
  double nlpval = INFINITY;
  const double *x = sol->getPrimal();
  double lp_obj = (sol) ? sol->getObjValue() : -INFINITY;
  fixInts_(x);

  ///MS: Modifying the PR amenable constraint when associated
  // binary variable takes value 0
  if (cpr_.size() > 0) {
    VariablePtr v;
    UInt binindex;
    
    for (UInt i= 0; i < cpr_.size() ; ++i) {
      binindex = bpr_[i]->getIndex();
      if ((x[binindex] < intTol_) && (spr_[i] == '1')) {
        fixCons_(cpr_[i]);
      }
    } 
    solveNLP_();
    unfixCons_();
  } else {
    solveNLP_();
  }

  //solveNLP_();
  
  unfixInts_();
  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    updateUb_(s_pool, &nlpval, sol_found); 
#if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "solved fixed NLP to optimality, "
      << "lp_obj = " << lp_obj 
      << ", nlpval = " << nlpval << std::endl;
#endif
    if (lp_obj > nlpval-solAbsTol_ && 
        lp_obj > nlpval-(fabs(nlpval))*solRelTol_){

#if SPEW
      logger_->msgStream(LogDebug) 
        << me_ << "Pruned" << std::endl;
#endif         
      *status = SepaPrune;
      break;
    } else {
      relobj_=sol->getObjValue();
      nlp_x = nlpe_->getSolution()->getPrimal();
#if SPEW
      logger_->msgStream(LogDebug) 
        << me_ << "Not Pruned in OA" << std::endl;
#endif
      numCuts = OAFromPoint_(nlp_x, sol->getPrimal(), status);
#if SPEW
      logger_->msgStream(LogDebug) 
        << me_ << "number of cuts from cutInt" << std::endl;
#endif 
      break;
    }
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    nlp_x = nlpe_->getSolution()->getPrimal();
    relobj_ = sol->getObjValue();
    numCuts = OAFromPointInf_(nlp_x, sol->getPrimal(), status);         
    break;
  case (ProvenUnbounded):
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status = " 
  << nlpe_->getStatusString() << std::endl
  << me_ << "No cut generated, may cycle!"
  << std::endl;
    *status = SepaError;
  }

  //MS: check if this the only way to do this.
  if (numCuts != 0) {
    *status = SepaResolve;
  } else {
#if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "number of cuts from OAFromPoint_ is 0" << std::endl;
#endif 
    *status = SepaPrune;
  }
  return;
}


void QGHandler::fixCons_(ConstConstraintPtr cp)
{
  double cu = cp->getUb(), cl = cp->getLb();
  ConstraintPtr c = minlp_->getConstraint(cp->getIndex());
  ConBoundMod *m1 = 0;
  if ((cu != INFINITY) || (cl != -INFINITY)) {  
    if (cu == 0) {
      m1 = new ConBoundMod(c, Upper, (cu + 0.1));
    } else if (cl == 0) {
      m1 = new ConBoundMod(c, Lower,(cl - 0.1));
    } 
    m1->applyToProblem(minlp_);
    prMods_.push(m1);
  } else {
 #if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "Constraint does not have any bounds" << std::endl;
#endif  
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
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    *isInf = true;
    break;
  case (ProvenUnbounded):
  case (EngineIterationLimit):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "NLP engine status = " 
      << nlpStatus_ << std::endl;
    assert(!"QGHandler: Cannot proceed further");
  }
  return;
}


bool QGHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, 
                           bool &, double & )
{
  double act;
  int error=0;
  FunctionPtr f;
  ConstraintPtr c;
  const double *x = sol->getPrimal();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) { 
    c = *it;
    act = c->getActivity(x, &error);
    if (error==0) {
      if((act > c->getUb() + solAbsTol_ && 
           act > c->getUb() + (fabs(c->getUb())*solRelTol_))  ||
          (act < c->getLb() - solAbsTol_ && 
            act < c->getLb() - (fabs(c->getLb())*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "constraint not feasible" 
          << std::endl << me_;
        c->write(logger_->msgStream(LogDebug));
        logger_->msgStream(LogDebug) << me_ << "activity = " << act << std::endl;  
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ 
        << "Constraint not defined at this point"<< std::endl;
      return false;
    }
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ 
    << "all nonlinear constraints feasible." << std::endl;
#endif
  error=0;
  if (true == oNl_) {
    double alpha = x[objVar_->getIndex()]; 
    relobj_ = alpha;
    act = minlp_->getObjValue(x, &error);
    if (error==0) {
      if (alpha < act - solAbsTol_ && alpha < act -(fabs(act)*solRelTol_)) {
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "objective not feasible"
          << std::endl;
#endif
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ 
        <<" Objective not defined at this point"<< std::endl;
      return false;
    }
  }
  logger_->msgStream(LogDebug) << me_ << "Looks feasible" << std::endl;
  return true;
}


void QGHandler::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf)
{

  int error=0, n = rel_->getNumVars();
  double *a = new double[n];
  const double linCoeffTol = 1e-6;
  VariableConstIterator vbeg = rel_->varsBegin(), vend = rel_->varsEnd();

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  
  if (error==0) {
    *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol); 
    *c  = fval - InnerProduct(x, a, minlp_->getNumVars());
    delete [] a;
  } else {
    logger_->msgStream(LogError) << me_ <<" Gradient not defined at this point"
      << std::endl;
  }
  return;
}


void QGHandler::linearizeObj_()
{
  ObjectivePtr o;
  o = minlp_->getObjective();
  std::string name = "obj_dummy_var";

  if (!o) {
    assert(!"need objective in QG!");
  } else if (o->getFunctionType() != Linear) {
    VariablePtr vPtr     = rel_->newVariable(-INFINITY, INFINITY, 
                                            Continuous, name); 
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    FunctionPtr f;
    assert(o->getObjectiveType()==Minimize);
    rel_->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel_->newObjective(f, 0.0, o->getObjectiveType());
    oNl_    = true;
    objVar_ = vPtr;
  }
  return;
}


int QGHandler::OAFromPoint_(const double *x, const double *inf_x,
                            SeparationStatus *status)
{
  double c;
  int error=0;
  ObjectivePtr o;
  UInt numCuts = 0;
  FunctionPtr f, f2;
  double vio, lpvio;
  ConstraintPtr con, newcon;
  double act=-INFINITY, nlpact = -INFINITY;
  LinearFunctionPtr lf = LinearFunctionPtr(); 

  *status=SepaContinue;


  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it; 
    f = con->getFunction();
    act = con->getActivity(x, &error); 
    nlpact =  con->getActivity(inf_x, &error);

    if (error==0) {
      if (con->getUb() < INFINITY) {
        vio = std::max(nlpact-con->getUb(), 0.0);

        if (vio>solAbsTol_ && vio > (fabs(con->getUb())*solRelTol_)) {
          linearAt_(f, act, x, &c, &lf);
          lpvio = std::max(lf->eval(inf_x)-con->getUb()+c, 0.0);

          if (lpvio>solAbsTol_ && lpvio > (fabs(con->getUb()-c)*solRelTol_)) {
            f2 = (FunctionPtr) new Function(lf);
            newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c,
                                         "lnrztn_cut");
            ++numCuts;
            ++(stats_->cuts);
            *status = SepaResolve;
#if SPEW
            logger_->msgStream(LogDebug) << me_ <<" OA cut: " << std::endl
              << std::setprecision(9);
            newcon->write(logger_->msgStream(LogDebug));
#endif
          } else{
#if SPEW
            logger_->msgStream(LogDebug) << me_ <<" no OA cut added " 
              << std::endl;
#endif
          }
        }
      } else if(con->getLb() > -INFINITY) {
        vio = std::max(con->getLb()-nlpact, 0.0);

        if (vio>solAbsTol_ && vio > (fabs(con->getLb())*solRelTol_)) {
          linearAt_(f, act, x, &c, &lf);
          lpvio = std::max(con->getLb()-c-lf->eval(inf_x), 0.0);

          if (lpvio>1e-4 && lpvio >(fabs(con->getLb()-c)*solRelTol_)) {
            f2 = (FunctionPtr) new Function(lf);
            newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY,
                                         "lnrztn_cut");
            ++numCuts; 
            ++(stats_->cuts);
            *status = SepaResolve;
#if SPEW
            logger_->msgStream(LogDebug) << me_ << "OA cut: " << std::endl
              << std::setprecision(9);
            newcon->write(logger_->msgStream(LogDebug));
#endif
          }
        }
      } else {
 #if SPEW
            logger_->msgStream(LogDebug) << me_ <<" Constraint do not have"
             << " either lower or upper bound" << std::endl;
#endif     
      }
    }	else {
      logger_->msgStream(LogError) << me_ << "Constraint not defined at"
        << " at least one of the points. "<<  std::endl;
    }
  }
  o = minlp_->getObjective();
  if (oNl_ && o) {
    f = o->getFunction();
    act=o->eval(x,&error);
    nlpact = o->eval(inf_x, &error);
    if (error==0) {
      vio = std::max(nlpact-relobj_, 0.0);
      if (vio > solAbsTol_ && vio > (fabs(relobj_)*solRelTol_)) {
        linearAt_(f, act, x, &c, &lf);
        lpvio = std::max(c+lf->eval(inf_x)-relobj_, 0.0);
        if (lpvio>solAbsTol_ && lpvio >(fabs(relobj_+c)*solRelTol_)) {
          lf->addTerm(objVar_, -1.0);
          f2 = (FunctionPtr) new Function(lf);
          newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut"); 
          ++numCuts;
          ++(stats_->cuts);
          *status = SepaResolve;
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "OA cut: " << std::endl
            << std::setprecision(9);
          newcon->write(logger_->msgStream(LogDebug));
#endif
        } else {
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "No objective OA cut: " 
            << std::endl;
#endif
        }
      }
    }	else {
      logger_->msgStream(LogError) << me_
                    << "Objective not defined at this solution point"
                    << std::endl;
    }
  }
  return numCuts;
}


int QGHandler::OAFromPointInf_(const double *x, const double *inf_x, 
                               SeparationStatus *status)
{
  double c;
  int error=0;
  double lpact;
  int ncuts = 0;
  ObjectivePtr o;
  FunctionPtr f, f2;
  double act=-INFINITY;
  ConstraintPtr con, newcon;
  LinearFunctionPtr lf = LinearFunctionPtr(); 

  *status=SepaContinue;
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it; 
    f = con->getFunction();
    act = con->getActivity(x, &error);
    if (error==0) {
      if (con->getUb() < INFINITY) {
        linearAt_(f, act, x, &c, &lf);
        f2 = (FunctionPtr) new Function(lf);
        lpact = f2->eval(inf_x, &error);
        if (lpact - con->getUb() + c > solAbsTol_ && 
            lpact - con->getUb() + c >(fabs(con->getUb()-c)*solRelTol_)) {
          newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c,
                                       "lnrztn_cut");
          ++ncuts;
          ++(stats_->cuts);
          *status = SepaResolve;
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "OA cut: " << std::endl
            << std::setprecision(9);
          newcon->write(logger_->msgStream(LogDebug));
#endif
        }
      }

      if (con->getLb() > -INFINITY) {
        linearAt_(f, act, x, &c, &lf);
        f2 = (FunctionPtr) new Function(lf);
        lpact = f2->eval(inf_x, &error);
        if (lpact - con->getLb() + c < -solAbsTol_  || 
            lpact - con->getLb() + c <-(fabs(con->getLb()-c)*solRelTol_)) {
          newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY,
                                       "lnrztn_cut");
          ++ncuts; 
          ++(stats_->cuts);
          *status = SepaResolve;
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "OA cut: " << std::endl
            << std::setprecision(9);
          newcon->write(logger_->msgStream(LogDebug));
#endif
        }
      }
    } else {
      logger_->msgStream(LogError) << me_ << "Constraint not defined at" <<
        " at least one of the points: "<<  std::endl;;
    }
  }

  return ncuts;
}


void QGHandler::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}


void QGHandler::relaxInitInc(RelaxationPtr rel, bool *is_inf)
{
  rel_ = rel;
  relax_(is_inf);
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


void QGHandler::relax_(bool *is_inf)
{
  ConstraintPtr c;
  linearizeObj_();
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
       ++it) {
    c = *it;
    if (c->getFunctionType()!=Constant && c->getFunctionType() != Linear) {
      nlCons_.push_back(c);
    }
  }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Number of nonlinear constraints "
    " = " << nlCons_.size() << std::endl;
  logger_->msgStream(LogDebug) << me_ << "Nonlinear solver used = "
    " = " << nlpe_->getName() << std::endl;
#endif
  initLinear_(is_inf);

#if SPEW
  logger_->msgStream(LogDebug2) << me_ << "Initial relaxation:" 
    << std::endl;
  rel_->write(logger_->msgStream(LogDebug2));
#endif
  return;
}


void QGHandler::separate(ConstSolutionPtr sol, NodePtr , RelaxationPtr rel, 
                         CutManager *, SolutionPoolPtr s_pool, ModVector &,
                         ModVector &, bool *sol_found, SeparationStatus *status)
{      
  double value;
  VariableType v_type;
  VariableConstIterator v_iter;
  const double *x = sol->getPrimal();

  for (v_iter=rel->varsBegin(); v_iter!=rel->varsEnd(); ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type==Binary || v_type==Integer) {
      value = x[(*v_iter)->getIndex()];
      if (fabs(value - floor(value+0.5)) > intTol_) {
#if SPEW
        logger_->msgStream(LogDebug) 
          << me_ << "solution is not integer feasible" << std::endl;	  
#endif
        return;
      }
    }
  }
#if SPEW
  logger_->msgStream(LogDebug)<< me_ 
    << "solution is integer feasible, may need to add outer-aprroximation cuts"
    << std::endl;	  
#endif
  cutIntSol_(sol, s_pool, sol_found, status);
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


void QGHandler::unfixCons_()
{
  Modification *m = 0;
  while(prMods_.empty() == false) {
    m = prMods_.top();
    m->undoToProblem(minlp_);
    prMods_.pop();
    delete m;
  }
  return;
}


void QGHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval, 
                          bool *sol_found)
{
  double val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();

  if (val <= bestval) {
    const double *x = nlpe_->getSolution()->getPrimal();
#if SPEW
    logger_->msgStream(LogDebug) 
      << me_ << "new solution found, value = " << val << std::endl;
#endif
    s_pool->addSolution(x, val);
    *sol_found = true;
  }
  *nlpval = val;
  return;
}


void QGHandler::writeStats(std::ostream &out) const
{
  out
    << me_ << "number of nlps solved       = " << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps   = " << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps     = " << stats_->nlpF << std::endl
    << me_ << "number of cuts added        = " << stats_->cuts << std::endl;
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
