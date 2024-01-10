//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file ParQGHandler.cpp
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
#include "ParQGHandler.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include "QuadraticFunction.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string ParQGHandler::me_ = "ParQGHandler: ";

ParQGHandler::ParQGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlCons_(0),
  nlpe_(nlpe),
  nlpStatus_(EngineUnknownStatus),
  objVar_(VariablePtr()),
  oNl_(false),
  rel_(RelaxationPtr()),
  relobj_(0.0),
  node_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  storeCutsAtNode_ = env_->getOptions()->findBool("storeCutsAtNode")->getValue();
  logger_ = env->getLogger();

  stats_ = new ParQGStats();
  stats_->nlpS = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->cuts = 0;
}


ParQGHandler::~ParQGHandler()
{
  if (stats_) {
    delete stats_;
  }

  env_ = 0;
  rel_ = 0;
  minlp_ = 0;
  nlCons_.clear();
}


void ParQGHandler::addInitLinearX_(const double *x)
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
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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


void ParQGHandler::cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan,
                           SolutionPoolPtr s_pool, bool *sol_found,
                           SeparationStatus *status)
{
  const double *lpx = sol->getPrimal();
  relobj_ = (sol) ? sol->getObjValue() : -INFINITY;

  fixInts_(lpx);           // Fix integer variables
#pragma omp critical (fixedNLPSolve)
  {
    solveNLP_();
  }
  unfixInts_();            // Unfix integer variables
  
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
  }

 if (*status == SepaContinue) {
   // happens due to tolerance. No linearizations are generated so prune the node
   *status = SepaPrune;
 }

#if SPEW
  logger_->msgStream(LogDebug) << me_ << "NLP solve status = "
    << nlpe_->getStatusString() << " and separation status = " << *status <<
    std::endl;
#endif
  return;
}


void ParQGHandler::fixInts_(const double *x)
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


void ParQGHandler::loadProbToEngine()
{
  nlpe_->load(minlp_);
}


void ParQGHandler::initLinear_(bool *isInf)
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
    assert(!"In ParQGHandler: stopped at root. Check error log.");
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "root NLP solve status = "
    << nlpe_->getStatusString() << std::endl;
#endif
  return;
}


bool ParQGHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &,
                           double &)
{
  int error = 0;
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
        "constraint not defined at this point."<< std::endl;
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


void ParQGHandler::setObjVar()
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


void ParQGHandler::linearizeObj_()
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


void ParQGHandler::linearAt_(FunctionPtr f, double fval, const double *x,
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


void ParQGHandler::cutToCons_(const double *nlpx, const double *lpx,
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


void ParQGHandler::cutsAtLpSol_(const double *lpx, CutManager *cutman,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  ConstraintPtr con, newcon;
  double c, act, cUb, lpvio;

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
        if (error==0) {
          lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
          if ((lpvio > solAbsTol_) && ((cUb-c)==0 ||
                                   (lpvio>fabs(cUb-c)*solRelTol_))) {
            ++(stats_->cuts);
            sstm << "_qgCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
            *status = SepaResolve;
            f = (FunctionPtr) new Function(lf);
            newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
            CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                          cUb-c, false,false);
            cut->setCons(newcon);
            if (node_) {
              node_->addCutToPool(cut, rel_);
            }
            cutman->addCutToPool(cut);
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
              sstm << "_qgObjCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
              newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                            -1.0*c, false,false);
              cut->setCons(newcon);
              if (node_) {
                node_->addCutToPool(cut, rel_);
                cut->setName_(sstm.str());
              }
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


void ParQGHandler::addCut_(const double *nlpx, const double *lpx,
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
        //newcon->write(std::cout);
        CutPtr cut = (CutPtr) new Cut(minlp_->getNumVars(),f, -INFINITY,
                                      cUb-c, false,false);
        cut->setCons(newcon);
        if (node_) {
          node_->addCutToPool(cut, rel_);
        }
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
 

void ParQGHandler::cutToObj_(const double *nlpx, const double *lpx,
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
      if ((vio > solAbsTol_) &&
          (relobj_ == 0 || vio > fabs(relobj_)*solRelTol_)) {
        act = o->eval(nlpx, &error);
        if (error == 0) {
          f = o->getFunction();
          LinearFunctionPtr lf = 0;
          linearAt_(f, act, nlpx, &c, &lf, &error);
          if (error == 0) {
            vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((vio > solAbsTol_) && ((relobj_-c)==0
                                     || vio > fabs(relobj_-c)*solRelTol_)) {
              ++(stats_->cuts);
              sstm << "_qgObjCut_Thr_" << omp_get_thread_num() << "_" << stats_->cuts;
              lf->addTerm(objVar_, -1.0);
              *status = SepaResolve;
              f = (FunctionPtr) new Function(lf);
              newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              CutPtr cut = (CutPtr) new Cut(rel_->getNumVars(),f, -INFINITY,
                                            -1.0*c, false,false);
              cut->setCons(newcon);
              if (node_) {
                node_->addCutToPool(cut, rel_);
              }
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


void ParQGHandler::relaxInitFull(RelaxationPtr , bool *)
{
  //Does nothing
}


void ParQGHandler::relaxInitInc(RelaxationPtr rel, bool *isInf)
{
  rel_ = rel;
  relax_(isInf);
  return;
}


void ParQGHandler::relaxNodeFull(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void ParQGHandler::relaxNodeInc(NodePtr , RelaxationPtr , bool *)
{
  //Does nothing
}


void ParQGHandler::nlCons()
{
  ConstraintPtr c;
  FunctionType fType;
  
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd();
       ++it) {
    c = *it;
    fType = c->getFunctionType();
    if (fType!=Constant && fType!=Linear) {
      nlCons_.push_back(c);
    }
  }
}


void ParQGHandler::setRelaxation(RelaxationPtr rel)
{
  rel_ = rel;
}


void ParQGHandler::relax_(bool *isInf)
{
  nlCons();
  linearizeObj_();
  initLinear_(isInf);
  return;
}


void ParQGHandler::separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                         CutManager *cutMan, SolutionPoolPtr s_pool,
                         ModVector &, ModVector &, bool *sol_found,
                         SeparationStatus *status)
{
  double val;
  VariablePtr var;
  VariableType v_type;
  const double *x = sol->getPrimal();

  *status = SepaContinue;
  for (VariableConstIterator v_iter = rel->varsBegin();
       v_iter != rel->varsEnd(); ++v_iter) {
    var = *v_iter;
    v_type = var->getType();
    if (v_type == Binary || v_type == Integer) {
      val = x[var->getIndex()];
      if (fabs(val - floor(val+0.5)) > intTol_) {
        return;
      }
    }
  }

  if (storeCutsAtNode_) {
    node_ = node;
  }

  cutIntSol_(sol, cutMan, s_pool, sol_found, status);
  node_ = 0;
  return;
}


void ParQGHandler::solveNLP_()
{
  nlpStatus_ = nlpe_->solve();
  ++(stats_->nlpS);
  return;
}


void ParQGHandler::unfixInts_()
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


void ParQGHandler::updateUb_(SolutionPoolPtr s_pool, double nlpval,
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


void ParQGHandler::writeStats(std::ostream &out) const
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


std::string ParQGHandler::getName() const
{
  return "ParQGHandler (Quesada-Grossmann)";
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
