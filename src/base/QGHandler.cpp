// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
// 

/**
 * \file QGHandler.cpp
 * \brief Define a processor for the textbook type Quesada-Grossmann
 * Algorithm.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>

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
#include "QuadraticFunction.h"

//#define SPEW 1

using namespace Minotaur;


typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string QGHandler::me_ = "QGHandler: ";

QGHandler::QGHandler()
  : env_(EnvPtr()),         // NULL
    minlp_(ProblemPtr()),
    rel_(RelaxationPtr()),
    nlpe_(EnginePtr()),
    nlpStatus_(EngineUnknownStatus),
    nlpWs_(WarmStartPtr()), // NULL
    objVar_(VariablePtr()),
    nlCons_(0),
    oNl_(false),
    isFeas_(true),
    linCoeffTol_(1e-6),
    stats_(0),
    solAbsTol_(1e-5),
    solRelTol_(1e-4),
    eTol_(1e-1),
    eLinTol_(1e-6),
    numCuts_(0),
    intTol_(1e-6),
    lastNode_(0)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


QGHandler::QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe) 
  : env_(env),
    minlp_(minlp),
    rel_(RelaxationPtr()),
    nlpe_(nlpe),
    nlpStatus_(EngineUnknownStatus),
    nlpWs_(WarmStartPtr()), // NULL
    objVar_(VariablePtr()),
    nlCons_(0),
    oNl_(false),
    isFeas_(true),
    linCoeffTol_(1e-6),
    solAbsTol_(1e-5),
    solRelTol_(1e-4),
    eTol_(1e-1),
    eLinTol_(1e-6),
    numCuts_(0),
    lastNode_(0)
{
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("handler_log_level")->getValue());

  intTol_  = env_->getOptions()->findDouble("int_tol")->getValue();

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
  if (lastSol_) {
    delete [] lastSol_;
  }
  env_.reset();
  minlp_.reset();
}



void QGHandler::relax_(RelaxationPtr rel, bool *is_inf)
{
  ConstraintPtr c;
  rel_ = rel;
  linearizeObj_(rel);
  lastSol_ = new double[rel_->getNumVars()];
  numvars_ = minlp_->getNumVars();
  
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd(); 
       ++it) {
    c = *it;
    if (c->getFunctionType()!=Constant && c->getFunctionType() != Linear) {
      nlCons_.push_back(c);
    }
  }

#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "Number of nonlinear constraints "
    " = " << nlCons_.size() << std::endl;
  logger_->MsgStream(LogDebug) << me_ << "Nonlinear solver used = "
    " = " << nlpe_->getName() << std::endl;
#endif
    initLinear_(is_inf);

#if SPEW
  logger_->MsgStream(LogDebug2) << me_ << "Initial relaxation:" 
                                << std::endl;
  rel_->write(logger_->MsgStream(LogDebug2));
#endif
}


void QGHandler::relaxInitFull(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}


void QGHandler::relaxInitInc(RelaxationPtr rel, bool *is_inf)
{
  relax_(rel, is_inf);
}


void QGHandler::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{
  assert(!"QGHandler::relaxNodeFull not implemented!");
}

void QGHandler::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  // do nothing.
}

void QGHandler::initLinear_(bool *isInf)
{
  const double *x;

  *isInf = false;
    
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
  case (ProvenLocalInfeasible): // can happen with NLPs
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
    logger_->MsgStream(LogError) << me_ << "NLP engine status = " 
                                 << nlpStatus_ << std::endl;
    assert(!"QGHandler: Cannot proceed further");
  }

}


void QGHandler::addInitLinearX_(const double *x)
{
  ConstraintPtr con, newcon;
  double act = 0;
  double c;
  FunctionPtr f, f2;
  LinearFunctionPtr lf = LinearFunctionPtr();
  ObjectivePtr o;
  Int error;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it;
    act = con->getActivity(x, &error);
    f = con->getFunction();
    linearAt_(f, act, x, &c, &lf);
    f2 = (FunctionPtr) new Function(lf);

    /// Looks like this was a bug when we didn't check the direction of the 
    /// constraint to add the linearization
    if (con->getUb() < INFINITY) {
      newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c,
                                   "lnrztn_cut");
      ++(stats_->cuts);
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "initial constr. cut: ";
      newcon->write(logger_->MsgStream(LogDebug));
#endif
    }

    if (con->getLb() > -INFINITY) {
      newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY, "lnrztn_cut");
      ++(stats_->cuts);      
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "initial constr. cut: ";
      newcon->write(logger_->MsgStream(LogDebug));
#endif
    } else if (con->getLb() - act > -solAbsTol_) {
      assert(!"bug here?");
      newcon = rel_->newConstraint(f2, act-c, INFINITY, "lnrztn_cut");
      ++(stats_->cuts);      
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "initial constr. cut: ";
      newcon->write(logger_->MsgStream(LogDebug));
#endif
    }
  }

  o = minlp_->getObjective();
  if (oNl_ && o) {
    f = o->getFunction();
    act = o->eval(x, &error);
    linearAt_(f, act, x, &c, &lf);
    lf->addTerm(objVar_, -1.);
    f2 = (FunctionPtr) new Function(lf);
    newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut");
    ++(stats_->cuts);
#if SPEW
    logger_->MsgStream(LogDebug) << me_ << "initial obj cut: " << std::endl
                                 << std::setprecision(9);
    newcon->write(logger_->MsgStream(LogDebug));
#endif
  } else {
    logger_->MsgStream(LogDebug) << "QG Handler: Problem does not have a " 
                                 << "nonlinear objective." << std::endl;
  }
}


void QGHandler::linearizeObj_(RelaxationPtr rel)
{
  ObjectivePtr o;
  o = minlp_->getObjective();
  std::string name     = "obj_dummy_var";
  if (!o) {
    assert(!"need objective in QG!");
  } else if (o->getFunctionType() != Linear) {
    VariablePtr vPtr     = rel->newVariable(-INFINITY, INFINITY, 
                                            Continuous, name); 
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

    FunctionPtr f;
    
    assert(o->getObjectiveType()==Minimize);
    
    rel->removeObjective();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    rel->newObjective(f, 0.0, o->getObjectiveType());

    oNl_    = true;
    objVar_ = vPtr;
  }
}


bool QGHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, 
                           bool &)
{

  FunctionPtr f;
  ConstraintPtr c;
  double act;
  const double *x = sol->getPrimal();
  Int error;

  rel_ = rel;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    c = *it;
    act = c->getActivity(x, &error);
    if (act > c->getUb() + solAbsTol_ || act < c->getLb() - solAbsTol_) {
#if SPEW
      logger_->MsgStream(LogDebug) 
        << me_ << "constraint not feasible" << std::endl
        << me_;
      c->write(logger_->MsgStream(LogDebug2));
      logger_->MsgStream(LogDebug) 
        << me_ << "activity = " << act << std::endl;  
#endif
      isFeas_ = false;
      return false;
    }
  }
#if SPEW
  logger_->MsgStream(LogDebug) 
    << me_ << "all nonlinear constraints feasible." << std::endl;
#endif
  objCutOff = false;
  if (true == oNl_ ) {
    //double alpha = sol->getObjValue();
    // It is important to check against x[.] value and not against
    // sol->getObjValue(). [See e.g. Feasibility Pump]
    double alpha = x[objVar_->getIndex()]; 
    relobj_ = alpha;
    act = minlp_->getObjValue(x, &error);

#if SPEW
    logger_->MsgStream(LogDebug) << me_ << "LP value = " << alpha 
                                 << std::endl
                                 << me_ << "NLP activity = " << act 
                                 << std::endl;

#endif
    if (alpha < act - solAbsTol_) {
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "objective not feasible" 
                                   << std::endl;
#endif
      isFeas_ = false;
      objCutOff = true;
      return false;
    }
  }
  isFeas_ = true;
  return true;
}


void QGHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr, 
                         CutManager *, SolutionPoolPtr s_pool,
                         bool *sol_found, SeparationStatus *status)
{
  // check integer feasibility of sol; must add cuts if is integer feasible
  numvars_ = minlp_->getNumVars();
  VariableConstIterator v_iter;
  VariableType v_type;
  double value;
  const double *x = sol->getPrimal();
  bool is_int_feas = true; 
  bool repeat_sol=true;
  Int i;

  if (node->getId()==0 || lastNode_ != node->getId()) {
    lastNode_ = node->getId();
    for (i=0; i < numvars_; i++){
      lastSol_[i]=x[i];
    }
  } else {
    for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd();
         ++v_iter) {
      i = (*v_iter)->getIndex();
      value = x[i];
      if (fabs(value-lastSol_[i]) > 0.1*solAbsTol_) {
        repeat_sol=false;
        break;
      }
    }
    if (!repeat_sol){
      for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd();
           ++v_iter) {
        i = (*v_iter)->getIndex();
        lastSol_[i] = x[i];
      }
    } else {
      assert(!"QGHandler: Cannot proceed further");
    }
  }

  for (v_iter=rel_->varsBegin(); v_iter!=rel_->varsEnd(); 
       ++v_iter) {
    v_type = (*v_iter)->getType();
    if (v_type==Binary || v_type==Integer) {
      value = x[(*v_iter)->getIndex()];
      if (fabs(value - floor(value+0.5)) > intTol_) {
        is_int_feas = false;
        break;
      }
    }
  }

  // TODO: There may be other situations when we want to add cuts with different
  // methods

  if (is_int_feas) {
#if SPEW
     logger_->MsgStream(LogDebug) 
        << me_ << "solution is integer feasible, may need to add cuts" << std::endl;	  
#endif
     cutIntSol_(sol, s_pool, sol_found, status);
  }
}


void QGHandler::cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                           bool *sol_found, SeparationStatus *status)
{
  const double *nlp_x;
  double nlpval = INFINITY;
  const double *x = sol->getPrimal();
  double lp_obj = (sol) ? sol->getObjValue() : -INFINITY;

  fixInts_(x);
  solveNLP_();
  unfixInts_();

  switch(nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):

    ++(stats_->nlpF);
    updateUb_(s_pool, &nlpval, sol_found);
#if SPEW
     logger_->MsgStream(LogDebug) 
        << me_ << "solved fixed NLP to optimality, "
		  << "lp_obj = " << lp_obj 
		  << ", nlpval = " << nlpval << std::endl;
#endif
    if (lp_obj > nlpval-solAbsTol_ || 
        lp_obj > nlpval-(fabs(nlpval)+1e-2)*solRelTol_){
      *status = SepaPrune;
      break;
    } else {
      nlp_x = nlpe_->getSolution()->getPrimal();
      OAFromPoint_(nlp_x, sol, status);
      break;
    }
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): // can happen with NLPs
  case (ProvenObjectiveCutOff):
    ++(stats_->nlpI);
    nlp_x = nlpe_->getSolution()->getPrimal();
    OAFromPoint_(nlp_x, sol, status);         
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
    logger_->MsgStream(LogError) << me_ << "NLP engine status = " 
                                 << nlpe_->getStatusString() << std::endl
                                 << me_ << "No cut generated, may cycle!"
                                 << std::endl;
    *status = SepaError;

  }
  if (numCuts_ != 0){
    *status = SepaResolve;
  }
}


void QGHandler::fixInts_(const double *x)
{
  VariablePtr v;
  double xval;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd(); 
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5); // round it to integer.
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp_);
      nlpMods_.push(m);
    }
  }
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
}


void QGHandler::copyLPBounds_(std::stack<Modification *> *mods)
{
  VariablePtr v, lpvar;
  VarBoundMod2 *m = 0;
  Int i=0;

  for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd(); 
       ++vit, ++i) {
    v = *vit;
    lpvar = rel_->getVariable(i);
    m = new VarBoundMod2(v, lpvar->getLb(), lpvar->getUb());
    m->applyToProblem(minlp_);
    mods->push(m);
  }
}


void QGHandler::undoLPBounds_(std::stack<Modification *> *mods)
{
  Modification *m = 0;
  while (mods->empty()==false) {
    m = mods->top();
    m->undoToProblem(minlp_);
    mods->pop();
    delete m;
  }
}


void QGHandler::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
}


void QGHandler::OAFromPoint_(const double *x, ConstSolutionPtr sol,
                             SeparationStatus *status)
{
  relobj_ = sol->getObjValue();
  OAFromPoint_(x, sol->getPrimal(), status);
}


void QGHandler::OAFromPoint_(const double *x, const double *inf_x,
                             SeparationStatus *status)
{
  /*
   * x is the solution of fixed NLP ==> act = g(x), if  act <= u then the
   * constraint is satisfied.
   * sol is the pointer to the solution of relaxation
   */

  double act=-INFINITY, nlpact = -INFINITY;
  ConstraintPtr con, newcon;
  double c;
  LinearFunctionPtr lf = LinearFunctionPtr(); // NULL
  FunctionPtr f, f2;
  ObjectivePtr o;
  UInt num_cuts = 0;
  int error;
  double vio, lpvio;
  

  *status=SepaContinue;
#if SPEW
    logger_->MsgStream(LogDebug)
      << me_ << "Adding linearizations, inf_x = " << inf_x << std::endl;
#endif

  std::vector<ConstraintPtr> conVect;
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it; 
    f = con->getFunction();
    act = con->getActivity(x, &error); assert (error==0);
    nlpact =  con->getActivity(inf_x, &error); assert (error==0);

    if(con->getUb() < INFINITY) {
      vio = std::max(nlpact-con->getUb(), 0.);
      if (vio>solAbsTol_) {
        linearAt_(f, act, x, &c, &lf);
        lpvio = std::max(lf->eval(inf_x)-con->getUb()+c, 0.);
#if SPEW
        logger_->MsgStream(LogDebug) << me_ << con->getName() 
                                     << " nlp-vio = " << vio
                                     << " lp-vio = " << lpvio
                                     << std::endl;
#endif
        if (lpvio>1e-4) {
          f2 = (FunctionPtr) new Function(lf);
          newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c,
                                       "lnrztn_cut");
          conVect.push_back(newcon);
          ++num_cuts;
          *status = SepaResolve;
#if SPEW
          logger_->MsgStream(LogDebug) << me_ <<" OA cut: " << std::endl
                                       << std::setprecision(9);
          newcon->write(logger_->MsgStream(LogDebug));
#endif
        }
      }
    }

    if(con->getLb() > -INFINITY) {
      vio = std::max(con->getLb()-nlpact, 0.);
      linearAt_(f, act, x, &c, &lf);
      f2 = (FunctionPtr) new Function(lf);
      if (vio>solAbsTol_) {
        lpvio = std::max(con->getLb()-c-lf->eval(inf_x), 0.);
#if SPEW
        logger_->MsgStream(LogDebug) << me_ << con->getName() 
                                     << " nlp-vio = " << vio
                                     << " lp-vio = " << lpvio
                                     << std::endl;
#endif
        if (lpvio>1e-4) {
          newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY,
                                       "lnrztn_cut");
          conVect.push_back(newcon);
          ++num_cuts; 
          *status = SepaResolve;
#if SPEW
          logger_->MsgStream(LogDebug) << me_ << "OA cut: " << std::endl
                                       << std::setprecision(9);
          newcon->write(logger_->MsgStream(LogDebug));
#endif
        }
      }
    }
  }

  o = minlp_->getObjective();
  if (oNl_ && o) {
    f = o->getFunction();
    if (inf_x) {
      act = o->eval(inf_x, &error);
      vio = std::max(act-relobj_, 0.);
    } else {
      vio = 1.;
    }
    if (vio > solAbsTol_) {
      linearAt_(f, act, inf_x, &c, &lf);
      lf->addTerm(objVar_, -1.);
      f2 = (FunctionPtr) new Function(lf);
      newcon = rel_->newConstraint(f2, -INFINITY, -1.0*c, "objlnrztn_cut"); 
      conVect.push_back(newcon);
      ++num_cuts;
      *status = SepaResolve;
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "OA cut: " << std::endl
        << std::setprecision(9);
      newcon->write(logger_->MsgStream(LogDebug));
#endif
    }
  }

  Int numvio = 0;
  for ( UInt i=0; i < conVect.size(); i++) {
    nlpact =  conVect[i]->getActivity(inf_x, &error);
    if ( (nlpact - conVect[i]->getUb() > solAbsTol_)
         || (conVect[i]->getLb() - nlpact > solAbsTol_ )) {
      ++numvio;
    }
  }
  if (numvio == 0) {
    num_cuts=0;
  }
  if (num_cuts == 0 ) {
    double *x_alpha = new double[numvars_];
    for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
      con = *it;
      act = con->getActivity(inf_x, &error);
      if (con->getUb() < INFINITY && act-con->getUb() > solAbsTol_) {
        cutXLP_(x, inf_x, x_alpha, con, true);
        OAFromPoint_(x_alpha, inf_x, status);
      } else if (con->getLb() > -INFINITY && con->getLb()-act > solAbsTol_) {
        cutXLP_(x, inf_x, x_alpha, con, false);
        OAFromPoint_(x_alpha, inf_x, status);
      }
    }
    o = minlp_->getObjective();
    if (oNl_) { 
      cutXLPObj_(x, inf_x, x_alpha, o->getFunction());
      *status = SepaResolve;
    }
  }
  stats_->cuts += num_cuts;
}


Int QGHandler::OAFromPointInf_(const double *x, const double *inf_x, 
                            SeparationStatus *status)
{
  Int ncuts = 0;

  double act=-INFINITY;
  double lpact;
  ConstraintPtr con, newcon;
  double c;
  LinearFunctionPtr lf = LinearFunctionPtr(); // NULL
  FunctionPtr f, f2;
  ObjectivePtr o;
  Int error;

  *status=SepaContinue;

  std::vector<ConstraintPtr> conVect;
  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    con = *it; 
    f = con->getFunction();
    act = con->getActivity(x, &error);
     
    if(con->getUb() < INFINITY) {
      linearAt_(f, act, x, &c, &lf);
      f2 = (FunctionPtr) new Function(lf);
      lpact = f2->eval(inf_x, &error);
      if (lpact - con->getUb() + c > solAbsTol_) {
        newcon = rel_->newConstraint(f2, -INFINITY, con->getUb()-c,
                                     "lnrztn_cut");
        conVect.push_back(newcon);
        ++ncuts;
        *status = SepaResolve;
#if SPEW
        logger_->MsgStream(LogDebug) << me_ << "OA cut: " << std::endl
          << std::setprecision(9);
        newcon->write(logger_->MsgStream(LogDebug));
#endif
      }
    }
 
    if(con->getLb() > -INFINITY) {
      linearAt_(f, act, x, &c, &lf);
      f2 = (FunctionPtr) new Function(lf);
      lpact = f2->eval(inf_x, &error);
      if (lpact - con->getLb() + c < -solAbsTol_) {
        newcon = rel_->newConstraint(f2, con->getLb()-c, INFINITY,
                                     "lnrztn_cut");
        conVect.push_back(newcon);
        ++ncuts; 
        *status = SepaResolve;
#if SPEW
        logger_->MsgStream(LogDebug) << me_ << "OA cut: " << std::endl
          << std::setprecision(9);
        newcon->write(logger_->MsgStream(LogDebug));
#endif
      }
    }
  }
  return ncuts;
}


void QGHandler::cutXLP_(const double *x_nlp, const double *x_lp,
                        double *x_alpha, ConstraintPtr con, bool dir)
{
  cutXLP_(x_nlp, x_lp, x_alpha, con->getFunction(), con->getUb(),
          con->getLb(), dir);
}


void QGHandler::cutXLP_(const double *x_nlp, const double *x_lp,
                        double *x_alpha, FunctionPtr fn, double ub, double lb,
                        bool dir)
{
  numvars_ = minlp_->getNumVars();
  double *gr = new double[numvars_];
  FunctionPtr fn2;
  LinearFunctionPtr lf;
  ConstraintPtr newcon;

  double alpha_l = 0.0;
  double alpha_u = 1.0;
  bool stop = false;
  double alpha;
  Int error = 0;
  double innProd = 0.0;
  double eval_x_alpha = 0.0;
  double eval_x_lp;
  
  double inn_gr_xlp;
  double inn_gr_xalpha;
  if (dir) {
    eval_x_lp = fn->eval(x_lp, &error)-ub;
    while (!stop) {
      alpha = 0.5 * (alpha_l + alpha_u);
      std::fill(gr, gr+numvars_, 0.0);
      for (Int i = 0; i < numvars_; i++) {
        x_alpha[i] = (1-alpha) * x_lp[i]+alpha * x_nlp[i];
      }
      fn->evalGradient(x_alpha, gr, &error);
      eval_x_alpha = fn->eval(x_alpha, &error)-ub;
      if (eval_x_alpha > -eLinTol_) {
        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
        innProd = inn_gr_xlp - inn_gr_xalpha;
        if (innProd > solAbsTol_ || alpha_u - alpha_l < 1e-8) {
          stop = true;
        }
      }
      if (eval_x_alpha > -solAbsTol_ || innProd < solAbsTol_) {
        if (eval_x_lp - eval_x_alpha > solAbsTol_) {        
          alpha_u = alpha;
        } else {
          alpha_l = alpha;
        }
      }  
    }
  } 
  else {
    eval_x_lp = fn->eval(x_lp, &error) - lb;
    while (!stop) {
      alpha = 0.5 * (alpha_l + alpha_u);
      std::fill(gr, gr+numvars_, 0.0);
      for (Int i = 0; i < numvars_; i++) {
        x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
      } 
      fn->evalGradient(x_alpha, gr, &error);
      if (eval_x_alpha < solAbsTol_) {
        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
        innProd = inn_gr_xlp - inn_gr_xalpha;
        if (innProd < -eLinTol_) {
          stop = true;
        }
      }
      if (eval_x_alpha < -solAbsTol_ || innProd < solAbsTol_) {
        if (eval_x_lp - eval_x_alpha > solAbsTol_) {
          alpha_u = alpha;
        } else alpha_l = alpha;
      } 
    }
  }
}


void QGHandler::cutXLP_(const double *x_nlp, const double *x_lp,
                        ConstraintPtr con, SeparationStatus *status, bool dir)
{
  double *x_alpha = new double[numvars_];
  numvars_ = minlp_->getNumVars();
  double *gr = new double[numvars_];

  FunctionPtr fn = con->getFunction();
  FunctionPtr fn2 = FunctionPtr();
  LinearFunctionPtr lf;
  ConstraintPtr newcon;

  double alpha_l = 0.0;
  double alpha_u = 1.0;
  bool stop = false;
  double alpha;
  Int error = 0;
  double innProd = 0.0;
  double eval_x_alpha = 0;
  double eval_x_lp;

  double inn_gr_xlp;
  double inn_gr_xalpha;
  double c;

  if (dir) {
    eval_x_lp= con->getActivity(x_lp,&error)-con->getUb();

    while (!stop)
    {
      alpha = 0.5 * (alpha_l + alpha_u);
      std::fill(gr, gr+numvars_, 0.0);
      for (Int i = 0; i < numvars_; i++) {
        x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
      }
      fn->evalGradient(x_alpha, gr, &error);
      eval_x_alpha = con->getActivity(x_alpha, &error) - con->getUb();
      if (eval_x_alpha > -eLinTol_) {
        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
        innProd = inn_gr_xlp - inn_gr_xalpha;
        if (innProd > eLinTol_) {
          stop = true;
          linearAt_(fn, 0.0, x_alpha, &c, &lf);
          if (lf) {
            fn2 = (FunctionPtr) new Function(lf);
            newcon = rel_->newConstraint(fn2, -INFINITY, inn_gr_xalpha,
                                         "lnrztn_cut");
#if SPEW
            logger_->MsgStream(LogDebug) << me_ << "Obj cut: " << std::endl
              << std::setprecision(9);
            newcon->write(logger_->MsgStream(LogDebug));
#endif
          } 
          *status = SepaResolve;
          stats_->cuts += 1;
        }
      }
      if (eval_x_alpha < -eLinTol_ || innProd < eLinTol_) {
        if (eval_x_lp - eval_x_alpha > eLinTol_) {
          alpha_u = alpha;
        } else alpha_l = alpha;
      }
    }
  } else {
    eval_x_lp= con->getActivity(x_lp,&error) - con->getUb();
    while (!stop) {
      alpha = 0.5 * (alpha_l + alpha_u);
      std::fill(gr, gr+numvars_, 0.0);
      for (Int i = 0; i < numvars_; i++) {
        x_alpha[i] = (1-alpha) * x_lp[i] + alpha * x_nlp[i];
      }
      fn->evalGradient(x_alpha, gr, &error);
      if (eval_x_alpha < eLinTol_) {
        inn_gr_xlp = InnerProduct(gr, x_lp, numvars_);
        inn_gr_xalpha = InnerProduct(gr, x_alpha, numvars_);
        innProd = inn_gr_xlp - inn_gr_xalpha;
        if (innProd < -eLinTol_) {
          stop = true;
          linearAt_(fn, 0.0, x_alpha, &c, &lf);
          fn2 = (FunctionPtr) new Function(lf);
          newcon = rel_->newConstraint(fn2, INFINITY, inn_gr_xalpha,
                                       "lnrztn_cut");
          *status = SepaResolve;
          ++(stats_->cuts);
#if SPEW
          logger_->MsgStream(LogDebug) << me_ << "Obj cut: " << std::endl
            << std::setprecision(9);
          newcon->write(logger_->MsgStream(LogDebug));
#endif
        }
      }
      else {
        eval_x_alpha = con->getActivity(x_alpha, &error) - con->getUb();
        if (eval_x_lp - eval_x_alpha < -eLinTol_)
        {
          alpha_u = alpha;
        } else alpha_l = alpha;
      }
    }
  }
}


void QGHandler::cutXLPObj_(const double *, const double *x_lp,
                           double *, FunctionPtr fn)
{

  FunctionPtr fn2;
  LinearFunctionPtr lf = LinearFunctionPtr();
  ConstraintPtr newcon;
  double c;
  Int error = 0;

  double eval_x_lp = fn->eval(x_lp, &error);

  numvars_ = minlp_->getNumVars();
  eval_x_lp = fn->eval(x_lp, &error);
  linearAt_(fn, eval_x_lp, x_lp, &c, &lf);
  lf->addTerm(objVar_, -1.);
  fn2 = (FunctionPtr) new Function(lf);
  newcon = rel_->newConstraint(fn2, -INFINITY, -1.0*c, "objlnrztn_cut");
  ++(stats_->cuts);
#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "Obj cut: " << std::endl
    << std::setprecision(9);
  newcon->write(logger_->MsgStream(LogDebug));
#endif
  
}


void QGHandler::linearAt_(FunctionPtr f, double fval, const double *x, 
                          double *c, LinearFunctionPtr *lf)
{
  Int n = rel_->getNumVars();
  double *a = new double[n];
  VariableConstIterator vbeg = rel_->varsBegin();
  VariableConstIterator vend = rel_->varsEnd();
  Int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol_); 
  *c  = fval - InnerProduct(x, a, numvars_);
  delete [] a;
}


void QGHandler::updateUb_(SolutionPoolPtr s_pool, double *nlpval, 
                          bool *sol_found)
{
  double     val = nlpe_->getSolutionValue();
  double bestval = s_pool->getBestSolutionValue();
 

  if (val <= bestval) {
    const double *x = nlpe_->getSolution()->getPrimal();
#if SPEW
    logger_->MsgStream(LogDebug) 
      << me_ << "new solution found, value = " << val << std::endl;
#endif
    s_pool->addSolution(x, val);
    *sol_found = true;
  }
  *nlpval = val;
}


void QGHandler::writeStats(std::ostream &out) const
{
  out
    << me_ << "number of nlps solved       = " << stats_->nlpS << std::endl
    << me_ << "number of infeasible nlps   = " << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps     = " << stats_->nlpF << std::endl
    << me_ << "number of cuts added        = " << stats_->cuts << std::endl;
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
