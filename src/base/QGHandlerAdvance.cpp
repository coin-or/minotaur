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
#include "BrCand.h"
#include "BrVarCand.h"
#include "CGraph.h"
#include "CNode.h"
#include "Constraint.h"
#include "ConBoundMod.h"
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
#include "QGHandlerAdvance.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Timer.h"
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
  rs3_(0),
  maxVioPer_(0),
  objVioMul_(0),
  nodeDep_(0),
  consDual_(0),
  findC_(0),
  cutMethod_("ecp"),
  //lpdist_(-1),
  lastNodeId_(-1),
  lpdist_(-1),
  prCutGen_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new QGStats();
  stats_->cuts = 0;
  stats_->nlpS = 0;
  stats_->nlpM = 0;
  stats_->nlpF = 0;
  stats_->nlpI = 0;
  stats_->nlpIL = 0;
  stats_->fracCuts = 0;
  stats_->preNodes = 0;
  stats_->fix = 0;
}


QGHandlerAdvance::~QGHandlerAdvance()
{
  if (stats_) {
    delete stats_;
  }

  if (extraLin_) {
    delete extraLin_;
  } else {
    if (solC_ && findC_) {
      delete [] solC_;
      solC_ = 0;
    }
  }

  if (prCutGen_) {
    delete prCutGen_;
  }

  env_ = 0;
  lpe_ = 0;
  rel_ = 0;
  nlpe_ = 0;
  minlp_ = 0;
  extraLin_ = 0;
  undoMods_();
  nlCons_.clear();
  consDual_.clear();
}


void QGHandlerAdvance::dualBasedCons_(ConstSolutionPtr sol)
{
  //// QGMV2: Dual based scheme
  double K = 0.5; // MS: can be parametrized.
  highDualCons_.clear();
  double act, maxDual = -INFINITY;
  const double * consDual = sol->getDualOfCons();

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    act = consDual[(*it)->getIndex()];
    if (act > maxDual) {
      maxDual = act;
    }
  }

  if (maxDual > 0) {
    for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
      act = consDual[(*it)->getIndex()];
      if (act >= K*maxDual) {
        highDualCons_.push_back(*it);
      }
    }
  }

  ////  For QGMV2 and QGMV3: Score based scheme
  double lambda1 = 0.05, lambda2 = 0.95;
  //const double * consDual = sol->getDualOfCons();
  //consDual_.clear();
  //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    //consDual_.push_back(consDual[(*it)->getIndex()]);
  for (UInt i = 0; i < nlCons_.size(); ++i) {
    consDual_[i] = lambda1*consDual_[i] +
      lambda2*(consDual[(nlCons_[i])->getIndex()]);
  }

  return;
}

void QGHandlerAdvance::addCutAtRoot_(ConstraintPtr con, const double * x,
                                     bool isObj)
{
  int error = 0;
  FunctionPtr f;
  double c, act, ub;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  //ConstraintPtr newcon;

  if (isObj) {
    ObjectivePtr o = minlp_->getObjective();
    act = o->eval(x, &error);
    if (error) {
      logger_->msgStream(LogError) << me_ << "Objective" <<
        " is not defined at this point." << std::endl;
      return;
    }
    ub = 0;
    ++(stats_->cuts);
    f = o->getFunction();
    sstm << "_qgObjCutRoot_" << stats_->cuts;
  } else {
    act = con->getActivity(x, &error);
    if (error) {
      logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName() <<
        " is not defined at this point." << std::endl;
      return;
    }
    ++(stats_->cuts);
    ub = con->getUb();
    f = con->getFunction();
    sstm << "_qgCutRoot_" << stats_->cuts;
  }

  linearAt_(f, act, x, &c, &lf, &error);

  if (error == 0) {
    if (isObj) {
      lf->addTerm(objVar_, -1.0);
    }
    f = (FunctionPtr) new Function(lf);
    rel_->newConstraint(f, -INFINITY, ub-c, sstm.str());
    //newcon = rel_->newConstraint(f, -INFINITY, ub-c, sstm.str());
    //std::cout <<"In QG \n";
    //newcon->write(std::cout);
  }
}

//void QGHandlerAdvance::addInitLinearX_(ConstSolutionPtr sol)
//{
  ////UInt sat = 0, nonsat = 0, bisec = 0;
  ////UInt sat = 0, act = 0, inte = 0;
  ////UInt nonsat = 0, ana = 0, bisec = 0;

  //for (UInt i = 0; i < nlCons_.size(); ++i) {
    //addCutAtRoot_(nlCons_[i], sol, 0);
  //}

  //if (oNl_) {
    //addCutAtRoot_(ConstraintPtr(), sol, 1);
  //}

  //if (prCutGen_) {
    //bool isFound;
    //UInt feas, bIdx;
    //const double * x = sol->getPrimal();
    //double *y = new double[minlp_->getNumVars()];
    //std::vector<prCons> prCons = prCutGen_->getPRCons();
    //double *ptToCut = 0, *prPt = new double[minlp_->getNumVars()];

    //for (UInt i = 0; i < prCons.size(); ++i) {
      //std::copy(x, x+(minlp_->getNumVars()), y);
      //isFound = prCutGen_->prVars(y, prPt, i, 0);
      //if (isFound) {
        //bIdx = (prCons[i].binVar)->getIndex();
        //feas = prCutGen_->isFeasible(prPt, i, 0, 0);
        //if (!feas) {
          //// PR infeasibility
          //isFound = false;
          ////++nonsat;
          //if (prCons[i].bisect) {
            ////++bisec;
            //isFound = prCutGen_->bisecPt(y, prPt, i, 0, 0);
          //} else {
            //if (prCons[i].binVal == 0) {
              //y[bIdx] = 1;
            //} else {
              //y[bIdx] = 0;
            //}
            //feas = prCutGen_->isFeasible(y, i, 0, 0);
            //y[bIdx] = x[bIdx];
            //if (feas) {
              ////++bisec;
              //isFound = prCutGen_->bisecPt(y, prPt, i, 0, 0);
            //} else {
              ////++ana;
              //isFound = prCutGen_->lineSearchAC(y, prPt, i, 0, 0);
            //}
          //}
        //} else {
          ////MS: which all cut points to be considered?
          ////++sat;
          //if (feas == 2) {
            ////++inte;
            //if (prCons[i].type == 1) {
              //if (!((prCons[i].cons)->getLinearFunction())) { // no linear function
                //bool found1 = 0, found2 = 0;
                //if (prCons[i].binVal == 0) {
                  //y[bIdx] = 1;
                  //feas = prCutGen_->isFeasible(y, i, 0, 0);
                  //y[bIdx] = x[bIdx];
                  //if (!feas) {
                    //if (prCutGen_->uniDirZSearch(y, prPt, i, 1)) {
                      //found2 = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                                                //VariablePtr(), CutManagerPtr(), 0);
                    //}
                  //} else {
                    //found2 =  prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                                                //VariablePtr(), CutManagerPtr(), 0);
                  //}

                  //std::copy(x, x+(minlp_->getNumVars()), y);
                  //if (prCutGen_->uniDirZSearch(y, prPt, i, 0)) {
                    //found1 = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                                              //VariablePtr(), CutManagerPtr(), 0);
                  //}
                //} else {
                  //y[bIdx] = 0;
                  //feas = prCutGen_->isFeasible(y, i, 0, 0);
                  //y[bIdx] = x[bIdx];
                  //if (!feas) {
                    //if (prCutGen_->uniDirZSearch(y, prPt, i, 0)) {
                      //found2 = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                                                //VariablePtr(), CutManagerPtr(), 0);
                    //}
                  //} else {
                    //found2 =  prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                                                //VariablePtr(), CutManagerPtr(), 0);
                  //}

                  //std::copy(x, x+(minlp_->getNumVars()), y);
                  //if (prCutGen_->uniDirZSearch(y, prPt, i, 1)) {
                    //found1 = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                                              //VariablePtr(), CutManagerPtr(), 0);
                  //}
                //}

                //if (found1 || found2) {
                  //continue;
                //}
              //}
            //}
          //}
          ////else {
            ////++act;
          ////}
        //}
      //}
      ////else {
        ////++sat;
        ////++act;
      ////}

      //if (isFound) {
        //isFound = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0,
                               //VariablePtr(), CutManagerPtr(), 0);
      //}

      //if (!isFound) {
        //addCutAtRoot_(prCons[i].cons, sol, 0);
      //}
    //}

    //prObj prO = prCutGen_->getPRObj();
    //if (prO.isPR) {
      //std::copy(x, x+(minlp_->getNumVars()), y);
      //isFound = prCutGen_->prVars(y, prPt, 0, 1);
      //if (isFound) {
        //feas = prCutGen_->isFeasible(prPt, 0, 1,
                                       //sol->getObjValue());
        //if (feas == 0) {
          //isFound = 0;
          //if (prO.bisect) {
            //isFound = prCutGen_->bisecPt(y, prPt, 0, 1, sol->getObjValue());
          //} else {
            //bIdx = (prO.binVar)->getIndex();
            //if (prO.binVal == 0) {
              //y[bIdx] = 1;
            //} else {
              //y[bIdx] = 0;
            //}
            //feas = prCutGen_->isFeasible(y, 0, 1, sol->getObjValue());
            //if (feas) {
              //isFound = prCutGen_->bisecPt(y, prPt, 0, 1, sol->getObjValue());
            //} else {
              //isFound = prCutGen_->lineSearchAC(y, prPt, 0, 1, sol->getObjValue());
            //}
          //}
        //}
      //}

      //if (isFound) {
        //isFound = prCutGen_->addPC(rel_, y, 0, ptToCut, prPt, 1, objVar_,
                         //CutManagerPtr(), sol->getObjValue());
      //}

      //if (!isFound) {
        //addCutAtRoot_(ConstraintPtr(), sol, 1);
      //}
    //}
    //if (prPt) {
      //delete [] prPt;
    //}

    //if (y) {
      //delete [] y;
    //}
    ////std::cout << "# of prCons unsat, bisec, ana " << nonsat << " " << bisec << " " << ana << "\n";
    ////std::cout << "# of prCons sat act and interior " << sat << " " << act << " " << inte << "\n";
    ////std::cout << "# of prCons, sat and unsat at root, and bisec amenable " << prCons.size() << " " <<
      ////sat << " " << nonsat << " " << bisec << "\n";
    ////exit(1);
  //}
  ////std::cout <<"Sat and unsat " << sat << " " << notSat << "\n";
  ////exit(1);
  //return;
//}


void QGHandlerAdvance::addInitLinearX_(ConstSolutionPtr sol)
{
  const double * x = sol->getPrimal();
  for (UInt i = 0; i < nlCons_.size(); ++i) {
    addCutAtRoot_(nlCons_[i], x, 0);
  }

  if (oNl_) {
    addCutAtRoot_(ConstraintPtr(), x, 1);
  }

  if (prCutGen_) {
    FunctionPtr f;
    bool isFound, binVal;
    UInt feas, bIdx, vIdx;
    std::vector<prCons> prCons = prCutGen_->getPRCons();
   
    double *ptToCut = 0, *prPt = new double[minlp_->getNumVars()];
    double *y = new double[minlp_->getNumVars()];
    std::fill(y, y + minlp_->getNumVars(), 0.0);
    std::fill(prPt, prPt + minlp_->getNumVars(), 0);

    for (UInt i = 0; i < prCons.size(); ++i) {
      binVal = prCons[i].binVal;
      bIdx = (prCons[i].binVar)->getIndex();
      
      if ((!binVal && fabs(x[bIdx]) < intTol_) ||
          (binVal && fabs(1-x[bIdx]) < intTol_)) {
        prCutGen_->fixedValue(prPt, 0, i);
        addCutAtRoot_(prCons[i].cons, prPt, 0);
        continue;
      } else {
        f = prCons[i].cons->getFunction();
        for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
             ++vit) {
          vIdx = (*vit)->getIndex();
          y[vIdx] = x[vIdx];
        }
        y[bIdx] = x[bIdx];
        isFound = prCutGen_->prVars(y, prPt, i, 0);
      }
      if (isFound) {
        feas = prCutGen_->isFeasible(prPt, i, 0, 0);
        if (!feas) {
          if (!(prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), CutManagerPtr(), 0))) {
            addCutAtRoot_(prCons[i].cons, x, 0);
          }
          //// PR infeasibility
          //if (!(prInfeasibility_(prCons[i].bisect, prCons[i].binVal, y, prPt, ptToCut, i, 0, 0, bIdx, x[bIdx]))) {
            //if (!(prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), CutManagerPtr(), 0))) {
              //addCutAtRoot_(prCons[i].cons, x, 0);
            //}
          //}
          //// OR
          prInfeasibility_(prCons[i].bisect, prCons[i].binVal, y, prPt, ptToCut, i, 0, 0, bIdx, x[bIdx]);
          continue;
        } else {
          if (feas == 2) {
            //if (prCons[i].type == 1) {
              //if (!((prCons[i].cons)->getLinearFunction())) { // no linear function
                if (!(prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), CutManagerPtr(), 0))) {
                  addCutAtRoot_(prCons[i].cons, x, 0);
                }
                prFeasibleInactive_(prCons[i].binVal, bIdx, i, prCons[i].cons->getFunction(), x, y, prPt, ptToCut, CutManagerPtr());
                continue;
              //}
            //}
          }
        }
      }

      if (isFound) {
        isFound = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), CutManagerPtr(), 0);
      }

      if (!isFound) {
        addCutAtRoot_(prCons[i].cons, x, 0);
      }
    }

    // For objective
    prObj prO = prCutGen_->getPRObj();
    if (prO.isPR) {
      binVal = prO.binVal;
      bIdx = (prO.binVar)->getIndex();
      if ((!binVal && fabs(x[bIdx]) < intTol_) ||
          (binVal && fabs(1-x[bIdx]) < intTol_)) {
        prCutGen_->fixedValue(prPt, 1, 0);
        addCutAtRoot_(ConstraintPtr(), x, 1);
      } else {
        f = minlp_->getObjective()->getFunction();
        for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
             ++vit) {
          vIdx = (*vit)->getIndex();
          y[vIdx] = x[vIdx];
        }
        y[bIdx] = x[bIdx];
        isFound = prCutGen_->prVars(y, prPt, 0, 1);
        if (isFound) {
          feas = prCutGen_->isFeasible(prPt, 0, 1, sol->getObjValue());
          if (!feas) {
            // at original point
            if (!(prCutGen_->addPC(rel_, y, 0, ptToCut, prPt, 1, objVar_, CutManagerPtr(), sol->getObjValue()))) {
              addCutAtRoot_(ConstraintPtr(), x, 1);
            }
            isFound = 0;
            if (prO.bisect) {
              isFound = prCutGen_->bisecPt(y, prPt, 0, 1, sol->getObjValue());
            } else {
              if (prO.binVal == 0) {
                y[bIdx] = 1;
              } else {
                y[bIdx] = 0;
              }
              feas = prCutGen_->isFeasible(y, 0, 1, sol->getObjValue());
              y[bIdx] = x[bIdx];
              if (feas) {
                isFound = prCutGen_->bisecPt(y, prPt, 0, 1, sol->getObjValue());
              } else {
                isFound = prCutGen_->lineSearchAC(y, prPt, 0, 1, sol->getObjValue());
              }
            }
            if (isFound) {
              prCutGen_->addPC(rel_, y, 0, ptToCut, prPt, 1, objVar_, CutManagerPtr(), sol->getObjValue());
            }

            if (prPt) {
              delete [] prPt;
            }
            if (y) {
              delete [] y;
            }
            return;
          }
        }

        if (isFound) {
          isFound = prCutGen_->addPC(rel_, y, 0, ptToCut, prPt, 1, objVar_, CutManagerPtr(), sol->getObjValue());
        }

        if (!isFound) {
          addCutAtRoot_(ConstraintPtr(), x, 1);
        }
      }
    }

    if (prPt) {
      delete [] prPt;
    }

    if (y) {
      delete [] y;
    }
  }
  return;
}


void QGHandlerAdvance::prFeasibleInactive_(bool binVal, UInt bIdx, UInt i, 
                                           FunctionPtr f, const double *x, 
                                           double * y, double * prPt, 
                                           const double * ptToCut, CutManagerPtr cutMan)
{
  UInt vIdx;
  bool feas;

  if (binVal == 0) {
    y[bIdx] = 1;
    feas = prCutGen_->isFeasible(y, i, 0, 0);
    y[bIdx] = x[bIdx];
    if (!feas) {
      if (prCutGen_->uniDirZSearch(y, prPt, i, 1)) {
        prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), cutMan, 0);
      }
    } else {
      prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), cutMan, 0);
    }

    for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
         ++vit) {
      vIdx = (*vit)->getIndex();
      y[vIdx] = x[vIdx];
    }
    y[bIdx] = x[bIdx];

    if (prCutGen_->uniDirZSearch(y, prPt, i, 0)) {
      prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), cutMan, 0);
    }
  } else {
    y[bIdx] = 0;
    feas = prCutGen_->isFeasible(y, i, 0, 0);
    y[bIdx] = x[bIdx];
    if (!feas) {
      if (prCutGen_->uniDirZSearch(y, prPt, i, 0)) {
        prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), cutMan, 0);
      }
    } else {
      prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), cutMan, 0);
    }

    for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
         ++vit) {
      vIdx = (*vit)->getIndex();
      y[vIdx] = x[vIdx];
    }
    y[bIdx] = x[bIdx];

    if (prCutGen_->uniDirZSearch(y, prPt, i, 1)) {
      prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), cutMan, 0);
    }
  }

  return;
}


bool QGHandlerAdvance::prInfeasibility_(bool bisect, bool binVal,
                                        double * y, double * prPt, const double *ptToCut, UInt i,
                                        bool isObj, double relVal, UInt bIdx,
                                        double xVal)
{
  bool feas, isFound = false;
  if (bisect) {
    isFound = prCutGen_->bisecPt(y, prPt, i, isObj, relVal);
  } else {
    if (binVal) {
      y[bIdx] = 0;
    } else {
      y[bIdx] = 1;
    }
    feas = prCutGen_->isFeasible(y, i, isObj, relVal);
    y[bIdx] = xVal;
    if (feas) {
      isFound = prCutGen_->bisecPt(y, prPt, i, isObj, relVal);
    } else {
      isFound = prCutGen_->lineSearchAC(y, prPt, i, isObj, relVal);
    }
  }

  if (isFound) {
    isFound = prCutGen_->addPC(rel_, y, i, ptToCut, prPt, 0, VariablePtr(), CutManagerPtr(), 0);
  }  
  return isFound;
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
      if ((relobj_ >= nlpval-objAbsTol_) ||
          (nlpval != 0 && (relobj_ >= nlpval-fabs(nlpval)*objRelTol_))) {
          *status = SepaPrune;
      } else {
        const double * nlpx = nlpe_->getSolution()->getPrimal();
        // Gradient inequalities to nonlinear objective and cons
        for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
          gradientIneq_(nlpx, lpx, cutMan, status, *it, 0);
        }

        if (prCutGen_) {
          bool binVal;
          UInt bIdx, vIdx;
          std::vector<prCons> prCons = prCutGen_->getPRCons();
          
          FunctionPtr f;
          double *y = new double[minlp_->getNumVars()];
          std::fill(y, y + minlp_->getNumVars(), 0.0);
          double *prPt = new double[minlp_->getNumVars()];
          std::fill(prPt, prPt + minlp_->getNumVars(), 0);

          //// Perspective cut for constraints
          for (UInt i = 0; i < prCons.size(); ++i) {
            binVal = prCons[i].binVal;
            bIdx = (prCons[i].binVar)->getIndex();
            if ((!binVal && fabs(nlpx[bIdx]) < intTol_) ||
                (binVal && fabs(1-nlpx[bIdx]) < intTol_)) {
              continue;
            }

            if (!(prCutGen_->addPC(rel_, nlpx, i, lpx, nlpx, 0, VariablePtr(), cutMan, 0))) {
              gradientIneq_(nlpx, lpx, cutMan, status, prCons[i].cons, 0);
            }

            if (prCutGen_->isFeasible(nlpx, i, 0, 0) == 2) {
              //// if (prCons[i].type == 1) {
                //// if (!((prCons[i].cons)->getLinearFunction())) { // no linear function
                  f = prCons[i].cons->getFunction();
                  for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
                       ++vit) {
                    vIdx = (*vit)->getIndex();
                    y[vIdx] = nlpx[vIdx];
                    prPt[vIdx] = nlpx[vIdx];
                  }
                  y[bIdx] = nlpx[bIdx];
                  prPt[bIdx] = nlpx[bIdx];                 
                  prFeasibleInactive_(prCons[i].binVal, bIdx, i, prCons[i].cons->getFunction(), nlpx, y, prPt, lpx, CutManagerPtr());
                ////}
              ////}
            } 
          }

          if (prPt) {
            delete [] prPt;
          }

          if (y) {
            delete [] y;
          }

          // Perspective cut for objective
          prObj prO = prCutGen_->getPRObj();
          if (prO.isPR) {
            binVal = prO.binVal;
            bIdx = (prO.binVar)->getIndex();
            if ((!binVal && fabs(nlpx[bIdx]) > intTol_) ||
                (binVal && fabs(1-nlpx[bIdx]) > intTol_)) {
              if (!(prCutGen_->addPC(rel_, nlpx, 0, lpx, nlpx, 1, objVar_, cutMan, relobj_))) {
                gradientIneq_(nlpx, lpx, cutMan, status, ConstraintPtr(), 1);  // to obj
              }
            }
          }
        } else {
          gradientIneq_(nlpx, lpx, cutMan, status, ConstraintPtr(), 1); // to obj
        }
      }
    }
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible):
  case (ProvenObjectiveCutOff):
    {
      ++(stats_->nlpI);
      const double * nlpx = nlpe_->getSolution()->getPrimal();
      for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
        gradientIneq_(nlpx, lpx, cutMan, status, *it, 0);
      }
      if (prCutGen_) {
        UInt pCuts = prCutGen_->getNumCuts();
        perspectiveCutsConsX_(nlpx, lpx, cutMan, status);
        if (prCutGen_->getNumCuts() > pCuts) {
          *status = SepaResolve;
        }
      }
    }
    break;
  case (EngineIterationLimit):
    {
      ++(stats_->nlpIL);
      //MS: can NLP solution be used in any way?
      //const double * nlpx = nlpe_->getSolution()->getPrimal();
      consCutsAtLpSol_(lpx, cutMan, status);

      if (prCutGen_) {
        UInt pCuts = prCutGen_->getNumCuts();
        perspectiveCutsObjX_(lpx, cutMan, status);
        perspectiveCutsConsX_(lpx, lpx, cutMan, status);
        if (prCutGen_->getNumCuts() > pCuts) {
          *status = SepaResolve;
        }
      } else {
        objCutAtLpSol_(lpx, cutMan, status, 0);
      }
    }
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


void QGHandlerAdvance::perspectiveCutsObjX_(const double *lpx,
                             CutManager *cutMan, SeparationStatus *status)
{
  bool binVal;
  UInt feas, bIdx, vIdx;
  std::vector<prCons> prCons = prCutGen_->getPRCons();

  FunctionPtr f;
  double *y = new double[minlp_->getNumVars()];
  std::fill(y, y + minlp_->getNumVars(), 0.0);
  double *prPt = new double[minlp_->getNumVars()];
  std::fill(prPt, prPt + minlp_->getNumVars(), 0);

  // For objective
  prObj prO = prCutGen_->getPRObj();
  if (prO.isPR) {
    binVal = prO.binVal;
    bIdx = (prO.binVar)->getIndex();
    if ((!binVal && fabs(lpx[bIdx]) > intTol_) ||
        (binVal && fabs(1-lpx[bIdx]) > intTol_)) {

      if (!(prCutGen_->addPC(rel_, lpx, 0, lpx, lpx, 1, objVar_, cutMan, relobj_))) {
        gradientIneq_(lpx, lpx, cutMan, status, ConstraintPtr(), 1);  // to obj
      }

      feas = prCutGen_->isFeasible(lpx, 0, 1, relobj_);
      if (!feas) {
        f = minlp_->getObjective()->getFunction();
        for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
             ++vit) {
          vIdx = (*vit)->getIndex();
          y[vIdx] = lpx[vIdx];
          prPt[vIdx] = lpx[vIdx];
        }
        y[bIdx] = lpx[bIdx];
        prPt[vIdx] = lpx[bIdx];
    
        if (prCutGen_->lineSearchAC(y, prPt, 0, 1, relobj_)) {
          prCutGen_->addPC(rel_, y, 0, lpx, prPt, 1, objVar_, cutMan, relobj_);
        }
      }
    }
  } else {
    objCutAtLpSol_(lpx, cutMan, status, 0);
  }

  if (prPt) {
    delete [] prPt;
  }

  if (y) {
    delete [] y;
  }

  return;
}

void QGHandlerAdvance::perspectiveCutsConsX_(const double *nlpx, const double *lpx,
                             CutManager *cutMan, SeparationStatus *status)
{
  if (prCutGen_) {
    bool binVal;
    UInt feas, bIdx, vIdx;
    std::vector<prCons> prCons = prCutGen_->getPRCons();
    
    FunctionPtr f;
    double *y = new double[minlp_->getNumVars()];
    std::fill(y, y + minlp_->getNumVars(), 0.0);
    double *prPt = new double[minlp_->getNumVars()];
    std::fill(prPt, prPt + minlp_->getNumVars(), 0);

    for (UInt i = 0; i < prCons.size(); ++i) {
      binVal = prCons[i].binVal;
      bIdx = (prCons[i].binVar)->getIndex();
      if ((!binVal && fabs(nlpx[bIdx]) < intTol_) ||
          (binVal && fabs(1-nlpx[bIdx]) < intTol_)) {
        continue;
      } else {
        if (!(prCutGen_->addPC(rel_, nlpx, i, lpx, nlpx, 0, VariablePtr(), CutManagerPtr(), 0))) {
          gradientIneq_(nlpx, lpx, cutMan, status, prCons[i].cons, 0);
        }

        feas = prCutGen_->isFeasible(nlpx, i, 0, 0);
        if (!feas) {
          //// PR infeasibility
          f = prCons[i].cons->getFunction();
          for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
               ++vit) {
            vIdx = (*vit)->getIndex();
            y[vIdx] = nlpx[vIdx];
            prPt[vIdx] = nlpx[vIdx];
          }
          y[bIdx] = nlpx[bIdx];
          prPt[bIdx] = nlpx[bIdx];
          // cut generated using analytical center
          if (prCutGen_->lineSearchAC(y, prPt, i, 0, 0)) {
            prCutGen_->addPC(rel_, y, i, lpx, prPt, 0, VariablePtr(), CutManagerPtr(), 0);
          } 
          //else {
            //if (!(prCutGen_->addPC(rel_, nlpx, i, lpx, nlpx, 0, VariablePtr(), CutManagerPtr(), 0))) {
              //gradientIneq_(nlpx, lpx, cutMan, status, prCons[i].cons, 0);
            //}
          //}
        } else {
          //if (!(prCutGen_->addPC(rel_, nlpx, i, lpx, nlpx, 0, VariablePtr(), CutManagerPtr(), 0))) {
            //gradientIneq_(nlpx, lpx, cutMan, status, prCons[i].cons, 0);
          //}
          if (feas == 2) {
            //if (prCons[i].type == 1) {
              //if (!((prCons[i].cons)->getLinearFunction())) { // no linear function
                f = prCons[i].cons->getFunction();
                for (VarSetConstIterator vit=f->varsBegin(); vit!=f->varsEnd();
                     ++vit) {
                  vIdx = (*vit)->getIndex();
                  y[vIdx] = nlpx[vIdx];
                  prPt[vIdx] = nlpx[vIdx];
                }
                y[bIdx] = nlpx[bIdx];
                prPt[bIdx] = nlpx[bIdx];
                if (binVal == 0) {
                  if (prCutGen_->uniDirZSearch(y, prPt, i, 0)) {
                    prCutGen_->addPC(rel_, y, i, lpx, prPt, 0, VariablePtr(), CutManagerPtr(), 0);
                  }
                } else {
                  if (prCutGen_->uniDirZSearch(y, prPt, i, 1)) {
                    prCutGen_->addPC(rel_, y, i, lpx, prPt, 0, VariablePtr(), CutManagerPtr(), 0);
                  }
                }
              //}
            //}
          }
        }
      }
    }

    if (prPt) {
      delete [] prPt;
    }

    if (y) {
      delete [] y;
    }
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


void QGHandlerAdvance::initLinear_(bool *isInf)
{
  *isInf = false;

  nlpe_->load(minlp_);
  solveNLP_();

  switch (nlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    ++(stats_->nlpF);
    addInitLinearX_(nlpe_->getSolution());
    break;
  case (EngineIterationLimit):
    ++(stats_->nlpIL);
    addInitLinearX_(nlpe_->getSolution());
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
      if ((act >= cUb + solAbsTol_) &&
          (cUb == 0 || act >= cUb + fabs(cUb)*solRelTol_)) {
        return false;
      }
    }	else {
      logger_->msgStream(LogError) << me_ << c->getName() <<
        " constraint not defined at this point."<< std::endl;
      return false;
    }
  }

  if (prCutGen_) {
    std::vector<prCons> prCons = prCutGen_->getPRCons();
    for (UInt i = 0; i < prCons.size(); ++i) {
      c = prCons[i].cons;
      act = c->getActivity(x, &error);
      if (error == 0) {
        cUb = c->getUb();
        if ((act >= cUb + solAbsTol_) &&
            (cUb == 0 || act >= cUb + fabs(cUb)*solRelTol_)) {
          return false;
        }
      }	else {
        logger_->msgStream(LogError) << me_ << c->getName() <<
          " constraint not defined at this point."<< std::endl;
        return false;
      }
    }
  }

  if (oNl_) {
    error = 0;
    relobj_ = sol->getObjValue();
    act = minlp_->getObjValue(x, &error);
    if (error == 0) {
      if ((act >= relobj_ + objAbsTol_) &&
          (relobj_ == 0 || (act >= relobj_ + fabs(relobj_)*objRelTol_))) {
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
    std::string name = "dummy_obj_var";
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


//void QGHandlerAdvance::cutToCons_(const double *nlpx, const double *lpx,
                             //CutManager *cutman, SeparationStatus *status)
//{
  //int error = 0;
  //ConstraintPtr con;
  //double nlpact, cUb;

  //for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    //con = *it;
    //nlpact =  con->getActivity(lpx, &error);
    //if (error == 0) {
      //cUb = con->getUb();
      //if ((nlpact > cUb + solAbsTol_) &&
          //(cUb == 0 || nlpact > cUb+fabs(cUb)*solRelTol_)) {
        //addCut_(nlpx, lpx, con, cutman, status);
      //} else {
//#if SPEW
        //logger_->msgStream(LogDebug) << me_ << " constraint " << con->getName() <<
          //" feasible at LP solution. No OA cut to be added." << std::endl;
//#endif
      //}
    //}	else {
      //logger_->msgStream(LogError) << me_ << " constraint not defined at" <<
        //" this point. "<<  std::endl;
    //}
  //}
  //return;
//}


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

  //if (prCutGen_) {
    //std::vector<prCons> prCons = prCutGen_->getPRCons();
    //for (UInt i = 0; i < prCons.size(); ++i) {
      //con = prCons[i].cons;
      //act =  con->getActivity(lpx, &error);
      //if (error == 0) {
        //cUb = con->getUb();
        //if ((act > cUb + solAbsTol_) &&
          //(cUb == 0 || act > cUb+fabs(cUb)*solRelTol_)) {
          //ECPTypeCut_(lpx, cutMan, con, act);
        //}
      //}
    //}
  //}

  if (temp < stats_->cuts) {
    *status = SepaResolve;
  }
  return;
}


void QGHandlerAdvance::objCutAtLpSol_(const double *lpx, CutManager *,
                                  SeparationStatus *status, bool fracNode)
{
  if (oNl_) {
    int error = 0;
    FunctionPtr f;
    LinearFunctionPtr lf;
    double c, lpvio, act;
    //double test = 0;
    //ConstraintPtr newcon;
    std::stringstream sstm;
    ObjectivePtr o = minlp_->getObjective();

    act = o->eval(lpx, &error);
    if (error == 0) {
      lpvio = std::max(act-relobj_, 0.0);
      if ((fracNode == 1) && (maxVioPer_ > 0) && (lpvio > objAbsTol_)) {
        //if ((fabs(relobj_) < solAbsTol_) || (fabs(lpvio) < solAbsTol_) ||

        if (fabs(relobj_) < objAbsTol_) {

          if (lpvio < objVioMul_) {
            //test = lpvio;
            return;
          }
        } else {
          //test = lpvio/fabs(relobj_);
           if ((lpvio/fabs(relobj_)) < objVioMul_) {
            return;
          }
        //std::cout << std:setprecision(6) <<  "lpvio here " << lpvio << " relobj_ " << relobj_ << " ratio " << test << "\n";
        }

        //std::cout << "lpvio " << lpvio << " relobj_ " << relobj_ << "\n";
      }
      if ((lpvio > objAbsTol_) &&
          (relobj_ == 0 || lpvio > fabs(relobj_)*objRelTol_)) {
          lf = 0;
          f = o->getFunction();
          linearAt_(f, act, lpx, &c, &lf, &error);
          if (error == 0) {
            lpvio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            if ((lpvio > objAbsTol_) && ((relobj_ == 0) || (lpvio > fabs(relobj_)*objRelTol_))) {
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


//void QGHandlerAdvance::addCut_(const double *nlpx, const double *lpx,
                        //ConstraintPtr con, CutManager*,
                        //SeparationStatus *status)
//{
  //int error = 0;
  ////ConstraintPtr newcon;
  //std::stringstream sstm;
  //LinearFunctionPtr lf = 0;
  //double c, lpvio, act, cUb;
  //FunctionPtr f = con->getFunction();

  //act = con->getActivity(nlpx, &error);

  //if (error == 0) {
    //linearAt_(f, act, nlpx, &c, &lf, &error);
    //if (error == 0) {
      //cUb = con->getUb();
      //lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      //if ((lpvio > solAbsTol_) &&
          //((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRelTol_))) {
        //++(stats_->cuts);
        //sstm << "_qgCut_" << stats_->cuts;
        //*status = SepaResolve;
        //f = (FunctionPtr) new Function(lf);
        //rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        ////newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //return;
      //} else {
        //delete lf;
        //lf = 0;
      //}
    //}
  //}	else {
    //logger_->msgStream(LogError) << me_ << " constraint not defined at"
      //<< " this point. "<<  std::endl;
  //}
  //return;
//}


void QGHandlerAdvance::gradientIneq_(const double *nlpx, const double *lpx,
                             CutManager *, SeparationStatus *status,
                             ConstraintPtr con, bool isObj)
{
  int error = 0;
  FunctionPtr f;
  double act, ub, c0, aTol, rTol;

  if (isObj) {
    ub = relobj_;
    aTol = objAbsTol_;
    rTol = objRelTol_;
    ObjectivePtr o = minlp_->getObjective();
    f = o->getFunction();
    c0 = o->getConstant();
  } else {
    c0 = 0;
    aTol = solAbsTol_;
    rTol = solRelTol_;
    ub = con->getUb();
    f = con->getFunction();
    act =  con->getActivity(lpx, &error);
  }

  act = f->eval(lpx, &error) + c0;

  if (error == 0) {
    if ((act >= ub + aTol) && (ub == 0 || act >= ub+fabs(ub)*rTol)) {
      act = f->eval(nlpx, &error);
      if (error == 0) {
        double c = 0;
        std::stringstream sstm;
        LinearFunctionPtr lf = 0;
        linearAt_(f, act, nlpx, &c, &lf, &error);
        if (error == 0) {
          if (isObj) {
            act = std::max(lf->eval(lpx)+c+c0-ub, 0.0);
            ub = -(c+c0);
          } else {
            act = std::max(lf->eval(lpx)+c-ub, 0.0);
            ub = ub - c;
          }
          if ((act >= solAbsTol_) &&
              (ub == 0 || (act >= fabs(ub)*solRelTol_))) {
            ++(stats_->cuts);
            if (isObj) {
              sstm << "_qgObjCut_" << stats_->cuts;
              lf->addTerm(objVar_, -1.0);
            } else {
              sstm << "_qgCut_" << stats_->cuts;
            }

            *status = SepaResolve;
            f = (FunctionPtr) new Function(lf);
            rel_->newConstraint(f, -INFINITY, ub, sstm.str());
            //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
            return;
          } else {
            delete lf;
            lf = 0;
          }
        }
      } else {
        logger_->msgStream(LogError) << me_ << " function is not defined at"
          << " fixed-NLP point (nlpx). "<<  std::endl;
      }
    } else {
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "LP solution (lpx) is feasible. No OA cut to be added." << std::endl;
#endif
      return;
    }
  } else {
    logger_->msgStream(LogError) << me_ << " Function not defined at" <<
      " lpx point. "<<  std::endl;
  }
}


//void QGHandlerAdvance::cutToObj_(const double *nlpx, const double *lpx,
                            //CutManager *, SeparationStatus *status)
//{
  //if (oNl_) {
    //int error = 0;
    //FunctionPtr f;
    //double c, vio, act;
    ////ConstraintPtr newcon;
    //std::stringstream sstm;
    //ObjectivePtr o = minlp_->getObjective();

    //act = o->eval(lpx, &error);
    //if (error == 0) {
      //vio = std::max(act-relobj_, 0.0);
      //if ((vio > objAbsTol_)
        //&& (relobj_ == 0 || vio > fabs(relobj_)*objRelTol_)) {
        //act = o->eval(nlpx, &error);
        //if (error == 0) {
          //f = o->getFunction();
          //LinearFunctionPtr lf = 0;
          //linearAt_(f, act, nlpx, &c, &lf, &error);
          //if (error == 0) {
            //vio = std::max(c+lf->eval(lpx)-relobj_, 0.0);
            //if ((vio > objAbsTol_) && ((relobj_ == 0)
                                     //|| (vio > fabs(relobj_)*objRelTol_))) {
              //++(stats_->cuts);
              //sstm << "_qgObjCut_" << stats_->cuts;
              //lf->addTerm(objVar_, -1.0);
              //*status = SepaResolve;
              //f = (FunctionPtr) new Function(lf);
              ////newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
              //rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
            //} else {
              //delete lf;
              //lf = 0;
            //}
          //}
        //}
      //}  else {
//#if SPEW
        //logger_->msgStream(LogDebug) << me_ << " objective feasible at LP "
          //<< " solution. No OA cut to be added." << std::endl;
//#endif
      //}
    //}	else {
      //logger_->msgStream(LogError) << me_
        //<< " objective not defined at this solution point." << std::endl;
    //}
  //}
  //return;
//}


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
  bool isPR = env_->getOptions()->findBool("perspective")->getValue();

  linearizeObj_();
  
  if (env_->getOptions()->findBool("persp_ref")->getValue()) {
    perspReform_();  
  }

  //PerspConPtr pCons = (PerspConPtr) new PerspCon(env_, minlp_);
  //pCons->implications();
  //impli0_ = pCons->getImplications0();
  //impli1_ = pCons->getImplications1();
  //delete pCons;

  if (isPR) {
    bool found;
    UInt it0 = 0;
    prCutGen_ = (PerspCutGeneratorPtr) new PerspCutGenerator(env_, minlp_,
                                                             nlpe_->emptyCopy());
    prCutGen_->findPRCons();
    std::vector<prCons> prCons = prCutGen_->getPRCons();
    // assuming constraint ordering in minlp_ used here and in
    // class PerspCutGenerator are the same
    for (ConstraintConstIterator it = minlp_->consBegin();
         it != minlp_->consEnd(); ++it) {
      c = *it;
      fType = c->getFunctionType();
      if (fType != Constant && fType != Linear) {
        found = false;
        for (UInt it1 = it0; it1 < prCons.size(); ++it1) {
          if (prCons[it1].cons == c) {
            it0 = it1 + 1;
            found = true;
            break;
          }
        }
        if (found) {
          continue;
        } else {
          nlCons_.push_back(c); // nonlinear constraints that are not pr amenable
        }
      }
    }

    //if (oNl_) {
      //if ((prCutGen_->getPRObj()).isPR) {
        //oNl_ = 0;
      //}
    //}
  } else {
    for (ConstraintConstIterator it = minlp_->consBegin();
         it != minlp_->consEnd(); ++it) {
      c = *it;
      fType = c->getFunctionType();
      if (fType != Constant && fType != Linear) {
        nlCons_.push_back(c);
      }
    }
  }

  initLinear_(isInf);

  if (*isInf) {
    return;
  }

  //MS: Presently either one of linearization schemes and PR can be eanabled
  if (!isPR) {
    bool temp1 = 0, temp2 = 0;
    UInt nlCons = nlCons_.size();
    rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
    maxVioPer_ = env_->getOptions()->findDouble("maxVioPer")->getValue();
    //objVioMul_ = env_->getOptions()->findDouble("objVioMul")->getValue();
    cutMethod_ = env_->getOptions()->findString("cutMethod")->getValue();
    bool rg1 = env_->getOptions()->findBool("root_linGenScheme1")->getValue();
    double rs1 = env_->getOptions()->findDouble("root_linScheme1")->getValue();
    double rs2Per = env_->getOptions()->findDouble("root_linScheme2")->getValue();
    double rg2 = env_->getOptions()->findDouble("root_linGenScheme2_per")->getValue(); //MS: change name in Environment

    if (rs3_ && nlCons > 0) {
      temp1 = 1;
    }

    if (maxVioPer_ && (cutMethod_ == "esh") && (nlCons > 0)) {
      temp2 = 1;
    }

    if (((nlCons_.size() > 0) || oNl_)) {
      if (nlCons_.size() > 0) {
        nodeDep_ = 10;
      } else {
        nodeDep_ = 5;
      }

      if (rs1 || rs2Per ||  rs3_ || rg1 || rg2) {
        extraLin_ = new Linearizations(env_, rel_, minlp_, nlCons_, objVar_,
                                       nlpe_->getSolution());
        if (temp1 || temp2 || rg1 || rg2) {
          extraLin_->setNlpEngine(nlpe_->emptyCopy());
          extraLin_->findCenter();
          if (temp2) {
            solC_ = extraLin_->getCenter();
            if (solC_ == 0) {
              maxVioPer_ = 0;
            }
          }
        }
        if (rs1 || rs2Per || rg1 || rg2) {
          extraLin_->rootLinearizations();
        }
      } else if (temp2) {
        findC_ = 1;
        findCenter_();
        if (solC_ == 0) {
          maxVioPer_ = 0;
        } else {
          const double *nlpx = nlpe_->getSolution()->getPrimal();
          double l1 = 0.5, l2 = 0.5; //MS: can be parametrized.
          for (UInt i = 0 ; i < minlp_->getNumVars(); ++i) {
            solC_[i] = l1*nlpx[i] + l2*solC_[i];
          }
        }
      }
    } else {
      rs3_ = 0;
    }
  }

 //// For dual multiplier based maxvio rule and score based rule
 //// Also make appropriate changes in the updateUb_()
  //if (maxVioPer_ && (nlCons > 0)) {
    //consDual_.resize(nlCons, 0);
    //dualBasedCons_(nlpe_->getSolution());
  //}
  return;
}


void QGHandlerAdvance::solveCenterNLP_(EnginePtr nlpe)
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
      //std::cout << "Center " << std::setprecision(6) << nlpe_->getSolution()->getObjValue() << "\n";
      //exit(1);
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


void QGHandlerAdvance::findCenter_()
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
    //if (fType == Constant) {
      //inst_C->markDelete(con);
      //continue;
    //} else
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
  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);

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
    //inst_C->write(std::cout);

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


void QGHandlerAdvance::shortestDist_(ConstSolutionPtr sol)
{

  EnginePtr nlpe = nlpe_->emptyCopy(); //Engine for modified problem
  ProblemPtr inst_C = minlp_->clone(env_);
  UInt n = minlp_->getNumVars();
  std::vector<CNode *> temp;
  CNode * n1, * n2, * nf;
  VariablePtr v;
  const double *x = sol->getPrimal();
  //std::cout << "Problem before construction\n";
  //inst_C->write(std::cout);

  CGraphPtr t = (CGraphPtr) new CGraph();

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
  //n1 = t->newNode(Minotaur::OpSqrt, nf, 0);
  //t->setOut(n1);
  t->setOut(nf);
  t->finalize();

  inst_C->removeObjective();
  FunctionPtr f = (FunctionPtr) new Function(t);
  inst_C->newObjective(f, 0.0, Minimize);
  inst_C->prepareForSolve();
  //std::cout << "Shortest distance NLP \n";
  //inst_C->write(std::cout);
  nlpe->load(inst_C);
  shortestNlpStatus_ = nlpe->solve();

  switch(shortestNlpStatus_) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
          lpdist_ = sqrt(nlpe->getSolution()->getObjValue());
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

  delete nlpe;
  nlpe = 0;
  delete inst_C;
  inst_C = 0;
  delete [] childr;
  temp.clear();
  return;
}

void QGHandlerAdvance::separate(ConstSolutionPtr sol, NodePtr node,
                                RelaxationPtr, CutManager *cutMan,
                                SolutionPoolPtr s_pool, ModVector &,
                                ModVector &, bool *sol_found,
                                SeparationStatus *status)
{
  const double *x = sol->getPrimal();

  //if (node->getId() == 0) {
    //shortestDist_(sol);
    ////std::cout << "LP obj " << sol->getObjValue() << "\n";
  //}
  //else {
    //std::cout << "LP obj " << sol->getObjValue() << "\n";
    //std::cout << "Shortest dist NLP solve status and shortest distance ; "
      //<< shortestNlpStatus_ << " ; " << std::setprecision(6) << lpdist_ << std::endl;
    ////exit(1);
  //}
  //if (node->getId() == 0) {
    ////shortestDist_(sol);
  ////} else {
    ////std::cout << "Shortest dist NLP solve status and shortest distance ; "
      ////<< shortestNlpStatus_ << " ; " << std::setprecision(6) << lpdist_ << std::endl;
    ////exit(1);
    //int error = 0;
    //double act, lpvio;

    //relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
    //ObjectivePtr o = minlp_->getObjective();
    //act = o->eval(x, &error);
    //if (error == 0) {
      //lpvio = std::max(act-relobj_, 0.0);
      //if (fabs(relobj_) > solAbsTol_) {
        //std::cout << "lpvio " << lpvio << " relobj_ " << relobj_ << " ratio " << lpvio/fabs(relobj_) << "\n";
      //} else {
        //std::cout << "lpvio " << lpvio << " relobj_ " << relobj_ << " ratio " << lpvio << "\n";

      //}
    //}
  //} else {
    //exit(1);
  //}

  if (prCutGen_ == 0) {
    if ((node->getId() == 0) && oNl_ && (maxVioPer_ > 0)) {
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
          //std::cout << std::setprecision(6) << "lpvio here " << lpvio << " relobj_ " << relobj_ << " ratio " << lpvio/fabs(relobj_) << "\n";
        } else {
          //std::cout << std::setprecision(6) << "lpvio here " << lpvio << " relobj_ " << relobj_ << " ratio " << lpvio << "\n";
          objVioMul_ = lpvio;
        }
        if (nlCons_.size() == 0 && oNl_) {
          if (objVioMul_ > 1000) {
            nodeDep_ = floor(nodeDep_/2);
          } else if (objVioMul_ < 0.5) {
            nodeDep_ = nodeDep_*2;
            objVioMul_ = 2*objVioMul_;
          }
        } else {
          if (objVioMul_ < 0.5) {
            objVioMul_ = 2*objVioMul_;
          }
        }
        //std::cout << "Initial multiplier " << objVioMul_ << "\n";
      }
    }
  }

  *status = SepaContinue;

  if (prCutGen_ == 0) {
    if (rs3_ && node->getId() == 0) {
      rs3_ = 0;
      extraLin_->rootLinScheme3(lpe_, status);
      if (*status == SepaResolve) {
        return;
      }
    }
  }

  if (isIntFeas_(x)) {
    fixInts_(x);            // Fix integer variables
    relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
    if (prCutGen_) {
      // Modifying PR amenable constraints when variables are fixed 
      prModNLP_(x); 
    } else {
      solveNLP_();            // solve NLP
    }
    undoMods_();            // Unfix integer variables
    //std::cout << "Node " << node->getId() << "\n";
    cutIntSol_(x, cutMan, s_pool, sol_found, status);
  } else {
     if (maxVioPer_ > 0) {
       relobj_ = (sol) ? sol->getObjValue() : -INFINITY;
       maxVio_(sol, node, cutMan, status); // maxViolation
    }
  }
  return;
}


void QGHandlerAdvance::perspReform_()
{
  //MS: to be done for objective
  int err = 0;
  VariablePtr z;
  FunctionPtr f;
  VariablePtr v;
  ConstraintPtr c;
  bool changed = false;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;  
  NonlinearFunctionPtr nlf, pnlf;  
  double rhs = 0, zc = 0, eps = 1e-6; //MS: eps value 
  
  VariableGroupConstIterator mit;
  PerspConPtr prc = (PerspConPtr) new PerspCon(env_, minlp_);
  prc->findPRCons();
  std::vector<prCons> prCons = prc->getPRCons();
   
  //std::cout << "Before \n";
  //p->write(std::cout); 
  for (UInt i = 0; i < prCons.size(); ++i) {
    // doing the reformulation now.
    c = prCons[i].cons;
    f = c->getFunction();
    z = prCons[i].binVar;

    nlf = f->getNonlinearFunction();


    if (nlf && nlf->hasVar(z)) {
      continue;
    }

    qf = f->getQuadraticFunction();
    if (qf && qf->hasVar(z)) {
      continue;
    }

    if (false == f->hasVar(z)) {
      z->inConstraint_(c);    
    }

    if (nlf) {
      CGraphPtr cg = dynamic_cast <CGraphPtr> (nlf);
      pnlf = cg->getPersp(z, eps, &err, qf, minlp_->getNumVars()+1, 
                          prCons[i].nNonzeroVar, intTol_); assert(err==0);
    } else {
      if (qf) {
        pnlf = qf->getPersp(z, eps, prCons[i].nNonzeroVar, intTol_);
      }
    }

    if (qf) {
      f->removeQuadratic();    
    }

    changed = true;
    minlp_->changeConstraint(c, pnlf); 

    // update upper bound
    rhs = c->getUb()*eps; 
      
    // modify linear part and rhs
    lf = f->getLinearFunction();
    if (lf) {
      for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           ++vt) {
        v = vt->first;
        if (v == z) {
          continue;
        }
        mit = (prCons[i].lNonzeroVar).find(v);
        if (mit != ((prCons[i].lNonzeroVar).end())) {
          if (fabs(mit->second) > intTol_) {
            zc += mit->second;
          }       
        } else {
 
          mit = (prCons[i].nNonzeroVar).find(v);
          if (mit != ((prCons[i].nNonzeroVar).end())) {
            if (fabs(mit->second) > intTol_) {
              zc += mit->second;
            }       
          }       
        }
      }

      if (fabs(zc) > intTol_) {
        if (lf->hasVar(z)) {
          lf->incTerm(z, zc-c->getUb()*(1-eps));      
        } else {
          lf->addTerm(z, zc-c->getUb()*(1-eps));      
        }
      }
      rhs = zc + c->getUb()*eps; 
      if (fabs(rhs) > intTol_) {
        minlp_->changeBound(c, Upper, rhs); 
      }
    } else {
       zc = -1*c->getUb()*(1-eps);
       if (fabs(zc) > intTol_) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(z, zc);      
        minlp_->changeConstraint(c, lf, -INFINITY, rhs); 
      } else {
        if (fabs(rhs) > intTol_) {
          minlp_->changeBound(c, Upper, rhs); 
        }
      }
    }
  }

  if (changed) {
    minlp_->prepareForSolve();  
  }
  //std::cout << "After \n";
  //p->write(std::cout);
  //exit(1);
}

void QGHandlerAdvance::prModNLP_(const double *)
{
  //std::vector<prCons> prCons = prCutGen_->getPRCons();
         
  //if (prCons.size()) {
    //double sb;
    //bool binval;
    //UInt bIdx;
    //ConstraintPtr c;
    //ConBoundMod *m;
    //Modification *m1;
    //std::stack<Modification *> prMods;
   
    //for (UInt i = 0; i < prCons.size(); ++i) {
      //c = prCons[i].cons;
      //binval = prCons[i].binVal;
      //bIdx = (prCons[i].binVar)->getIndex();
      
      //if (binval) {
        //sb = 1 - x[bIdx];
      //} else {
        //sb = x[bIdx];
      //}  
     
       
      //if ((fabs(sb) < intTol_) && (prCons[i].type != 2)) {
        //m = new ConBoundMod(c, Upper, (c->getUb() + 0.1));
        //m->applyToProblem(minlp_);
        //prMods.push(m);
      //}
    //}

    //if (prMods.size()) {
      //++(stats_->nlpM);
    //}

    //solveNLP_();

    //// undo modifications
    //while(prMods.empty() == false)  {
      //m1 = prMods.top();
      //m1->undoToProblem(minlp_);
      //prMods.pop();
      //delete m1;
    //}

  //} else {
    //solveNLP_();
  //}

  //return;  
}


void QGHandlerAdvance::ESHTypeCut_(const double *x, CutManager *cutMan)
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


void QGHandlerAdvance::genLin_(const double *x, std::vector<UInt > vioCons,
                               CutManager *cutMan)
{
  int error;
  FunctionPtr f;
  ConstraintPtr con;
  double c, cUb, act;
  LinearFunctionPtr lf;
  std::stringstream sstm;

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
        sstm << "_qgCut_" << stats_->cuts;
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      }
    }
  }

  if (oNl_) {
    SeparationStatus s = SepaContinue;
    objCutAtLpSol_(x, cutMan, &s, 1);
  }
  return;
}


bool QGHandlerAdvance::boundaryPtForCons_(double* xnew, const double *xOut,
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


//void QGHandlerAdvance::ESHTypeCut_(const double *lpx, CutManager *cutMan,
                             //ConstraintPtr con)
//{
  //int error = 0;
  //double *x, *xl, *xu;
  //bool ptChanged = true;
  //FunctionPtr f = con->getFunction();
  //UInt numVars =  minlp_->getNumVars(), repPt = 0;
  //double cUb = con->getUb(), nlpact, repSol = INFINITY,
         //lambda1 = 0.5, lambda2 = 0.5;

  //x  = new double[numVars];
  //xl = new double[numVars];
  //xu = new double[numVars];
  //std::copy(lpx,lpx+numVars,xu);
  //std::copy(solC_,solC_+numVars,xl);


  //while (true) {
    //for (UInt i = 0 ; i < numVars; ++i) {
      //x[i] = lambda1*xl[i] + lambda2*xu[i];
    //}
    //nlpact = f->eval(x, &error);
    //if (error == 0) {
      //if (nlpact > cUb+solAbsTol_ && (cUb == 0 ||
                                  //nlpact > cUb+fabs(cUb)*solRelTol_)) {
        //std::copy(x,x+numVars,xu);
      //} else {
        //if (fabs(cUb-nlpact) <= solAbsTol_ ||
            //(cUb!=0 && fabs(cUb-nlpact) <= fabs(cUb)*solRelTol_)) {
            //break;
        //} else {
          //std::copy(x,x+numVars,xl);
        //}
      //}
    //} else {
      //ptChanged = false;
      //break;
    //}
    //if (nlpact == repSol) {
      //++repPt;
    //} else {
      //repPt = 0;
      //repSol = nlpact;
    //}
    //if (repPt == 10) {
      //break;
    //}
  //}

  //SeparationStatus s = SepaContinue;
  //if (ptChanged) {
    //double c;
    //std::stringstream sstm;
    //LinearFunctionPtr lf = 0;
    //linearAt_(f, nlpact, x, &c, &lf, &error);
    //if (error == 0) {
      //++(stats_->cuts);
      //sstm << "_qgCut_" << stats_->cuts;
      //f = (FunctionPtr) new Function(lf);
      //rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      ////newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //// Cut to nonlinear objective
      //objCutAtLpSol_(x, cutMan, &s);
    //}
  //} else {
    //nlpact =  con->getActivity(lpx, &error);
    //ECPTypeCut_(lpx, cutMan, con, nlpact);

    //// Cut to nonlinear objective
    //objCutAtLpSol_(lpx, cutMan, &s);
  //}


  //delete [] x;
  //delete [] xl;
  //delete [] xu;
  //return;
//}


//void QGHandlerAdvance::maxVio_(ConstSolutionPtr sol, NodePtr node,
                               //CutManager *cutMan,
                               //SeparationStatus *status)
//{
  //int error = 0;
  //ConstraintPtr c;
  //double act, cUb, vio = 0.0;
  //const double *x = sol->getPrimal();
  //UInt temp = stats_->cuts, nodeId = node->getId();

  //if (node->getDepth() >= nodeDep_ && stats_->fracCuts > 0) {
    //return;
  //}

  //if (cutMethod_ == "ecp" || (nlCons_.size() == 0 && oNl_)) {
    //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    ////for (CCIter it=highDualCons_.begin(); it!=highDualCons_.end(); ++it) {
      //c = *it;
      //act = c->getActivity(x, &error);
      //if (error == 0) {
        //cUb = c->getUb();
        //if (act > cUb+solAbsTol_ && (cUb == 0 ||
                                     //act > cUb + fabs(cUb)*solRelTol_)) {
          ////std::cout << "node " << node->getId() << " vio " << act -cUb << "\n";
          //if (fabs(cUb) > solAbsTol_) {
            //vio = (act - cUb)/fabs(cUb);
          //} else {
            //vio = (act - cUb);
          //}
          ////std::cout << vio << "\n";
          //if (vio >= maxVioPer_) {
            //ECPTypeCut_(x, cutMan, c, act);
          //}
        //}
      //}
    //}

    //if (oNl_) {
      //SeparationStatus s = SepaContinue;
      //objCutAtLpSol_(x, cutMan, &s, 1);
    //}
  //} else if (cutMethod_ == "esh") {
     //for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it) {
    ////for (CCIter it=highDualCons_.begin(); it!=highDualCons_.end(); ++it) {
      //c = *it;
      //act = c->getActivity(x, &error);
      //if (error == 0) {
        //cUb = c->getUb();
        //if (act > cUb+solAbsTol_ && (cUb == 0 ||
                                     //act > cUb + fabs(cUb)*solRelTol_)) {
          ////std::cout << "node " << node->getId() << " vio " << act -cUb << "\n";
          //if (fabs(cUb) > solAbsTol_) {
            //vio = (act - cUb)/fabs(cUb);
          //} else {
            //vio = (act - cUb);
          //}
          ////std::cout << vio << "\n";
          //if (vio >= maxVioPer_) {
            //ESHTypeCut_(x, cutMan);
            //break;
          //}
        //}
      //}
    //}
  //}

  //stats_->fracCuts = stats_->fracCuts + (stats_->cuts - temp);

  //if ((temp < stats_->cuts) && (nodeId != UInt(lastNodeId_))) {
    //*status = SepaResolve;
  //}

  ////std::cout << "Node " << node->getId() << " depth " << node->getDepth()
    ////<< " vio val " << val << " bnd " << bnd << " max vio % " << max << "\n";
  //lastNodeId_ = nodeId;
  //return;
//}


//// Score based rule
void QGHandlerAdvance::maxVio_(ConstSolutionPtr sol, NodePtr node,
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
            //std::cout << "act = "<< act << " vio = " << vio << " dual = " << consDual_[i] << " cub = " << cUb << std::endl;
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
        //if (fabs(parentScore) > 1e-3 && fabs(totScore) > 1e-2
        //if (fabs(parentScore) > 1e-1 && fabs(totScore) > (maxVioPer_*fabs(parentScore)))  //MS: here maxVioPer_ is in times (0.5, 1, 2, 5,..)
          //std::cout << std::setprecision(6) << "node, score, and parent's score "<< nodeId << " " << totScore << " " << parentScore << "\n";
        if (fabs(totScore) >= (maxVioPer_*fabs(parentScore + .001))) { //MS: here maxVioPer_ is in times (0.5, 1, 2, 5,..)
          //std::cout << std::setprecision(6) << "node, score, and parent's score, maxvio "<< nodeId << " " << totScore << " " << parentScore << " " <<maxVioPer_ <<"\n";
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
  //nlpe->load(inst_C);
  //shortestNlpStatus_ = nlpe->solve();

  //switch(shortestNlpStatus_) {
  //case (ProvenOptimal):
  //case (ProvenLocalOptimal):
    //lpdist_ = sqrt(nlpe->getSolution()->getObjValue());
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


void QGHandlerAdvance::ECPTypeCut_(const double *lpx, CutManager *,
                                   ConstraintPtr con, double act)
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

  if ((bestval - objAbsTol_ > nlpval) ||
        (bestval != 0 && (bestval - fabs(bestval)*objRelTol_ > nlpval))) {
    const double *x = nlpe_->getSolution()->getPrimal();
    s_pool->addSolution(x, nlpval);
    *sol_found = true;

    //if (maxVioPer_ && (nlCons_.size() > 0)) {
      //dualBasedCons_(nlpe_->getSolution());
    //}
  }
  return;
}


void QGHandlerAdvance::writeStats(std::ostream &out) const
{
  bool impli = 0;

  if (impli0_.size() || impli1_.size()) {
    impli = 1;
  }

  if (extraLin_ != 0) {
    extraLin_->writeStats(out);
  }

  //std::cout << "Shortest dist NLP solve status and shortest distance ; "
      //<< shortestNlpStatus_ << " ; " << std::setprecision(6) << lpdist_ << std::endl;

  out
    << me_ << "number of nlps solved                       = "
    << stats_->nlpS << std::endl
    << me_ << "number of pr modified nlps                  = "
    << stats_->nlpM << std::endl
    << me_ << "number of infeasible nlps                   = "
    << stats_->nlpI << std::endl
    << me_ << "number of feasible nlps                     = "
    << stats_->nlpF << std::endl
    << me_ << "number of nlps hit engine iterations limit  = "
    << stats_->nlpIL << std::endl
    << me_ << "number of frac cuts added                   = "
    << stats_->fracCuts << std::endl
    << me_ << "number of total cuts added                  = "
    << stats_->cuts << std::endl
    << me_ << "problem has implications                    = "
    << impli << std::endl
    << me_ << "number of presolved nodes                   = "
    << stats_->preNodes << std::endl
    << me_ << "number of presolved nodes with var fixing   = "
    << stats_->fix << std::endl;

  if (prCutGen_) {
    prCutGen_->writeStats(out);
  }


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
