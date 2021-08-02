// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2021 The MINOTAUR Team.
// 

/** 
 * \file Linearizations.cpp
 * \Briefly define a class for adding linearizations in linearization
 * based methods. Add extra linearizations to the nonlinear constraints
 * in a problem (and not the nonlinear objective).
 * \Author Meenarli Sharma, Indian Institute of Technology Bombay
 */


#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "MinotaurConfig.h"

#include "CNode.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "HessianOfLag.h"
#include "Logger.h"
#include "Node.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "Timer.h"
#include "ProblemSize.h"
#include "Linearizations.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string Linearizations::me_ = "Linearizations: ";


Linearizations::Linearizations(EnvPtr env, RelaxationPtr rel,
                               ProblemPtr minlp, std::vector<ConstraintPtr> nlCons,
                               VariablePtr objVar, ConstSolutionPtr sol)
: env_(env),
  rel_(rel),
  minlp_(minlp),
  nlpe_(EnginePtr()),
  solC_(NULL),
  nbhSize_(20),
  varPtrs_(0),
  changeVar_(0),
  nlpDuals_(0),
  hasEqCons_(0),
  numDir_(300),
  isBoundPt_(0)
{
  nlCons_ = nlCons;
  logger_ = env->getLogger();
  rs1_ = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  rs2Per_ = env_->getOptions()->findDouble("root_linScheme2")->getValue();
  rs2NbhSize_ = env_->getOptions()->findDouble("root_linScheme2_nbhSize")->getValue();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  rgs1_ = env_->getOptions()->findBool("root_linGenScheme1")->getValue();
  rgs2Per_ = env_->getOptions()->findDouble("root_linGenScheme2_per")->getValue();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();

  timer_ = env->getNewTimer();
  stats_ = new LinStats();
  stats_->cuts = 0;
  stats_->rs1Cuts = 0;
  stats_->rs2Cuts = 0;
  stats_->rs3Cuts = 0;
  stats_->rgs1Cuts = 0;
  stats_->rgs2Cuts = 0;
  stats_->linSchemesTime = 0;

  //if (rs1_ || rs2Per_ || rgs1_ || rgs2Per_ || rs3_) {
  UInt n =  minlp_->getNumVars();
  nlpx_ = new double[n];
  const double * x = sol->getPrimal();
  std::copy(x, x+n, nlpx_);
  //} else {
    //nlpx_ = 0;  
  //}

  if (rgs2Per_ ) {
    UInt numCons = minlp_->getNumCons();
    const double *temp = sol->getDualOfCons();
    nlpDuals_ = new double[numCons];
    std::copy(temp, temp+numCons, nlpDuals_);
  } else {
    nlpDuals_ = 0;  
  }

  if (objVar) {
    oNl_ = true;
    objVar_ = objVar;
  } else {
    objVar_ = 0;  
    oNl_ = false;
  }
 }


Linearizations::~Linearizations()
{ 
  if (stats_) {
    delete stats_;
  }
  if (solC_) {
    delete [] solC_;
    solC_ = 0;
  }
  if (nlpx_) {
    delete [] nlpx_;
    nlpx_ = 0;
  }
  if (nlpDuals_) {
    delete [] nlpDuals_;
    nlpDuals_ = 0;
  }
  if (timer_) {
    delete timer_;
  }
  env_ = 0;
  rel_ = 0;
  minlp_ = 0;
  nlCons_.clear();
}


bool Linearizations::addCutAtRoot_(double *x, FunctionPtr fun, UInt &newConId,
                                   double UB, bool isObj)
{
  int error = 0;
  FunctionPtr f;
  double c, act;
  ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = LinearFunctionPtr();

  act = fun->eval(x, &error);
  if (error == 0) {
    linearAt_(fun, act, x, &c, &lf, &error);
    if (error == 0) {
      ++(stats_->cuts);
      sstm << "_linCutRoot_" << stats_->cuts;
      if (isObj) {
        lf->addTerm(objVar_, -1.0);
      }
      f = (FunctionPtr) new Function(lf);
      newcon = rel_->newConstraint(f, -INFINITY, UB-c, sstm.str());
      newConId = newcon->getIndex();
      sstm.str("");
      return true;
    }
  }	else {
    logger_->msgStream(LogError) << me_ << "Function can not be evaluated" <<
      " at this point." << std::endl;
  }
  return false;
}


bool Linearizations::linPart_(double *b1, UInt lVarIdx, double lVarCoeff,
                              double act, FunctionPtr f, double UB)
{
  int error = 0;
  double nlTerm = 0; 
  QuadraticFunctionPtr qf = f->getQuadraticFunction();
  NonlinearFunctionPtr nlf = f->getNonlinearFunction();

  if (nlf) {
    nlTerm = nlf->eval(b1, &error); 
  } 

  if (error == 0) {
    if (qf) {
      nlTerm = nlTerm + qf->eval(b1); 
    }
    b1[lVarIdx] = (UB - nlTerm - act)/lVarCoeff;    
    return true;
  }
  return false;
}


//void Linearizations::objToCons_(ProblemPtr problem, VariablePtr vObj,
//VariablePtr newVar)
//{
//// only moving objective to constraints using variable objVar_
  //assert(problem);
  //ObjectivePtr oPtr = problem->getObjective();
  //FunctionType objFunType = oPtr->getFunctionType();

  //if (oPtr) {
    //if (objFunType == Constant || objFunType == Linear) {
      //return;
    //}
    //FunctionPtr f = oPtr->getFunction();

    //QuadraticFunctionPtr qf = f->getQuadraticFunction();
    //NonlinearFunctionPtr nlf = f->getNonlinearFunction();
    //// add a new variable
    //// add this variable to the objective
    //LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    //lf->addTerm(vObj, -1.0);
    //lf->addTerm(newVar, -1.0);

    //if (qf) {
      //// remove quadratic parts from the objective
      //qf = problem->removeQuadFromObj();
      //// add a new constraint containing the new variable and the quadratic.
      //// qf - lf <= 0
    //}
    //if (objFunType == Nonlinear) {
      //nlf = problem->removeNonlinFromObj();
    //}

    //if (qf && nlf) {
      //f = (FunctionPtr) new Function(lf, qf, nlf);
    //} else if (qf) {
      //f = (FunctionPtr) new Function(lf, qf);
    //} else if (nlf) {
      //f = (FunctionPtr) new Function(lf, nlf);
    //} 
      //problem->newConstraint(f, -INFINITY, 0.0);
  //}
//}


void Linearizations::ifNonlinCons_()
{
  double lb, ub;
  FunctionPtr fnewc;
  ConstraintPtr con;
  FunctionType fType;
  VariablePtr vPtr, v;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone(env_);
  LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
 
  // Modify objective 
  vPtr = inst_C->newVariable(-INFINITY, 0, Continuous, "eta", VarHand);
  vPtr->setFunType_(Nonlinear);
  inst_C->removeObjective();
  lfc->addTerm(vPtr, 1.0);
  fnewc = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(fnewc, 0.0, Minimize);

  // Modify the nonlinear constraints and let linear constraints as they are
  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
    //if (fType == Constant) {
      //inst_C->markDelete(con);
      //continue;
    //}  
    if (fType == Linear)  {
      //if (fabs(lb-ub) <= solAbsTol_) {
        //hasEqCons_ = 1;
      //}
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

  //inst_C->delMarkedCons();
  inst_C->prepareForSolve();
  nlpe_->load(inst_C);
  solveNLP_();
  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);
  
  if (nlpe_->getStatusString() == "ProvenUnbounded") {
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
    nlpe_->load(inst_C);
    solveNLP_();
  
    if (solC_) {
      if (fabs(nlpe_->getSolution()->getObjValue()) <= solAbsTol_) {
        delete [] solC_;
        solC_ = 0;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "NLP engine status for restricted center problem = "
        << nlpe_->getStatusString() << std::endl;
    }
  } else {
    if (solC_) {
      if (fabs(nlpe_->getSolution()->getObjValue()) <= solAbsTol_) {
        delete [] solC_;
        solC_ = 0;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "NLP engine status for center problem = "
        << nlpe_->getStatusString() << std::endl;
    }
  }

  //// To print interior point
  //if (solC_) {
    //std::cout << "Center found " << nlpe_->getSolution()->getObjValue() << std::endl;  
  //} else {
    //std::cout << "Center not found\n";  
  //}
  //exit(1);

  delete inst_C;
  inst_C = 0;
  delete nlpe_;
  nlpe_ = 0;
  return;
}


void Linearizations::ifOnlyNonlinObj_()
{
  double lb, ub;
  FunctionPtr fnewc;
  ConstraintPtr con;
  FunctionType fType;
  VariablePtr vPtr, v;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone(env_);
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
    if (fType == Constant) {
      inst_C->markDelete(con);
      continue;
    } else if (fType == Linear)  {
      if (lb != -INFINITY && ub != INFINITY) {
        if (fabs(lb-ub) <= solAbsTol_) {
          hasEqCons_ = 1;
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
      inst_C->changeConstraint(con, lfc, lb, ub);
    } else {
      logger_->msgStream(LogError) << me_ << "Unidentified constraint type." 
        << std::endl;
    }
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
  inst_C->prepareForSolve();
  nlpe_->load(inst_C);
  solveNLP_();
  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);
  //exit(1);

  if (solC_) {
    if (fabs(nlpe_->getSolution()->getObjValue()) <= solAbsTol_) {
      isBoundPt_ = true;      
    }
  } else {
    logger_->msgStream(LogError) << me_ 
      << "NLP engine status for center problem = "
      << nlpe_->getStatusString() << std::endl;
  }
  //// To print interior point
  //if (solC_) {
    //std::cout << "Center found " << nlpe_->getSolution()->getObjValue() << std::endl;  
  //} else {
    //std::cout << "Center not found\n";  
  //}

  delete inst_C;
  inst_C = 0;
  delete nlpe_;
  nlpe_ = 0;
  return;
}


void Linearizations::findCenter()
{
  timer_->start();
  isBoundPt_ = false;

  if (rs3_) {
    if (nlCons_.size() > 0) { 
      ifNonlinCons_();
    }
  } else {
    if (nlCons_.size() > 0) { 
      ifNonlinCons_();
    } else if (oNl_) {
      ifOnlyNonlinObj_();  
    }
  }
  
  if (solC_) {
    if (!isBoundPt_) {
      double l1 = 0.5, l2 = 0.5;
      for (UInt i = 0 ; i < minlp_->getNumVars(); ++i) {
        solC_[i] = l1*nlpx_[i] + l2*solC_[i];
      }
    }
  }

  stats_->linSchemesTime = stats_->linSchemesTime + timer_->query();
  timer_->stop();
  return;
}


void Linearizations::solveNLP_()
{ 
  if (solC_) {
    delete [] solC_;
    solC_ = 0;
  }
  EngineStatus nlpStatus = nlpe_->solve();
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      //std::cout << "Center " << std::setprecision(6) << nlpe_->getSolution()->getObjValue() << "\n";
      //exit(1);
      UInt numVars = minlp_->getNumVars();
      const double *temp = nlpe_->getSolution()->getPrimal();
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


bool Linearizations::findIntersectPt_(std::vector<UInt > newConsId,
                                      VariablePtr vl, VariablePtr vnl,
                                      double * iP)
{
  ConstraintPtr con = rel_->getConstraint(newConsId[0]);
  LinearFunctionPtr lf = con->getLinearFunction();
  double a = lf->getWeight(vl), b = lf->getWeight(vnl), e = con->getUb();
  
  con = rel_->getConstraint(newConsId[1]);
  lf = con->getLinearFunction();
  double c = lf->getWeight(vl), d = lf->getWeight(vnl), f = con->getUb();

  /* we solve the linear system
   * ax+by=e
   * cx+dy=f
   * where, x is iP[1] and y is iP[0]
   */
  double determinant = a*d - b*c; 
  if(determinant != 0) {
    iP[1] = (e*d - b*f)/determinant;
    iP[0] = (a*f - c*e)/determinant;
  } else {
    std::cout << "Cramer equations system: determinant is zero. " <<
              "Either no solution or infinitely many solutions exist.\n";
    return false; 
  }
  return true;
}


void Linearizations::insertNewPt_(UInt j, UInt i, std::vector<double > &xc,
                             std::vector<double> &yc, ConstraintPtr newcon,
                             VariablePtr vl, VariablePtr vnl, bool &shouldCont)
{
  double f = newcon->getUb();
  LinearFunctionPtr lf = newcon->getLinearFunction();
  
  double d = lf->getWeight(vl), c = lf->getWeight(vnl), x1 = xc[j], y1 = yc[j],
  x2 = xc[i], y2 = yc[i], x, y;

  // point of intersection of newcon with the lin from j and j-1
  double a = y1-y2, b = x2-x1, e = y1*(x2-x1) - x1*(y2-y1), det = a*d - b*c; 
  if(det != 0) {
    x = (e*d - b*f)/det;
    y = (a*f - e*c)/det;
    xc.insert(xc.begin()+j,x);
    yc.insert(yc.begin()+j,y);
  } else {
    std::cout << "Cramer equations system: determinant is zero. " <<
              "Either no solution or infinitely many solutions exist.\n"; 
    shouldCont = false;
  }
}


void Linearizations::candLinCons_(const double *x,
                                  std::vector<UInt > &consToLin,
                                  bool &foundActive, bool &foundVio)
{
  UInt i = 0;
  int error = 0;
  ConstraintPtr c;
  double act, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it, ++i) {
    c = *it;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        if (!foundVio) {
          foundVio = true;
          if (consToLin.size() != 0) {
            consToLin.clear();
          }
        }
        consToLin.push_back(i);
      } else if ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_)) {
        if (!foundVio) {
          if (!foundActive) {
            foundActive = true;         
          }
          consToLin.push_back(i);
        }
      }
    }
  }
  return;
}


void Linearizations::linearAt_(FunctionPtr f, double fval, const double *x,
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


bool Linearizations::rootLinearizationsUniS()
{
  timer_->start();
  
  FunctionPtr f; 
  ConstraintPtr con;
  bool isFound = false, allFound = true;
  UInt nVarIdx, lVarIdx;
  double lVarCoeff = 0, nVarCoeff = 0, ub;
   
 
  if (rs1_ || rs2Per_) { 
    for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
      con = *it;
      f = con->getFunction();
      lVarIdx = 0; lVarCoeff = 0; nVarCoeff = 0;
      // constraints with only one var in the nonlinear part which is not in
      // the linear part 
      isFound = uniVarNlFunc_(f, lVarCoeff, lVarIdx, nVarIdx, nVarCoeff, 0);
      //MS: see if this if-else has to be changed
      if (isFound) {
        ub = con->getUb();
        if (rs1_ > 0) {
          rootLinScheme1_(f, lVarCoeff, lVarIdx, nVarIdx, nVarCoeff, ub, 0);
        }
        if (rs2Per_ > 0) { // there is a default neighborhood
          rootLinScheme2_(f, ub, lVarCoeff, lVarIdx, nVarIdx, 0);
        }
      } else {
        if (allFound) {
          allFound = false;       
        }
      }
    }

    if (oNl_) {      
      ObjectivePtr o = minlp_->getObjective();
      f = o->getFunction();
      lVarIdx = 0; lVarCoeff = 0; nVarCoeff = 0;
      isFound = uniVarNlFunc_(f, lVarCoeff, lVarIdx, nVarIdx, nVarCoeff, 1);
      if (isFound) {
        ub = o->getConstant();
        if (rs1_ > 0) {
          rootLinScheme1_(f, lVarCoeff, lVarIdx, nVarIdx, nVarCoeff, -1*ub, 1);
        }
        if (rs2Per_ > 0) { // there is a default neighborhood
          rootLinScheme2_(f, -1*ub, lVarCoeff, lVarIdx, nVarIdx, 1);
        }
      }      
    }
  }

  stats_->linSchemesTime = stats_->linSchemesTime + timer_->query();
  timer_->stop();
  return allFound;
}


void Linearizations::rootLinearizationsGen()
{
  timer_->start();
  /// General scheme at root
  if (solC_) {
    // General scheme using center and  positive spanning vectors
    if (!hasEqCons_) {
      varsInNonlinCons_();
    }
    
    if (rgs1_) {
      rootLinGenScheme1_();
    }

    // General scheme using nonlinear solution and positive spanning vectors
    // Similar conditions as in rgs1
    if (rgs2Per_) {
      rootLinGenScheme2_();
    }
  }
  stats_->linSchemesTime = stats_->linSchemesTime + timer_->query();
  timer_->stop();
  return;
}


void Linearizations::setStepSize_(double &alpha, std::vector<VariablePtr > vars,
                                  double *xOut, std::vector<double > unitVec)
{
  int error = 0;
  double val = 0;
  UInt idx, i, j, n = minlp_->getNumVars(), nnz = minlp_->getNumHessNnzs();
  UInt *iRow = new UInt[nnz];
  UInt *jCol = new UInt[nnz];
  double *values = new double[nnz];

  minlp_->getHessian()->fillRowColIndices((Minotaur::UInt *)iRow,
                                          (Minotaur::UInt *)jCol);
  minlp_->getHessian()->fillRowColValues((double *)xOut, 1, nlpDuals_,
                                         (double *)values, &error);

  if (error == 0) {
    if (vars.size() == 1) {
      for (UInt k = 0; k < nnz; ++k) {
        idx = vars[0]->getIndex();
        i = iRow[k];
        if (i > idx) {
          break;
        } else {
          if ((i == idx) && (i == jCol[k])) {
            val = values[k]*pow(unitVec[0],2);
            break;
          }
        }
      }
    } else {
      double *s = new double[n];
      double *uv = new double[n];
      std::fill(s, s+n, 0);
      std::fill(uv, uv+n, 0);
      for (UInt k = 0; k < vars.size(); ++k) {
        uv[vars[k]->getIndex()] = unitVec[k];
      }
      for (UInt k = 0; k < nnz; ++k) {
        i = iRow[k];
        j = jCol[k];
        val = values[k];
        s[j] = s[j] + val*uv[i];
        s[i] = s[i] + val*uv[j];
      }

      val = 0;
      for (UInt k = 0; k < vars.size(); ++k) {
        i = vars[k]->getIndex();
        val = val + s[i]*unitVec[k];
      }
      delete [] s;
      delete [] uv;
    }
  }

  //std::cout << "s^THs " << val << "\n";

  if (fabs(val) < solAbsTol_) {
    alpha = 1;
  } else {
    alpha = 0.25;
  }
  delete [] iRow;
  delete [] jCol;
  delete [] values;
  return;
}


void Linearizations::search_(std::vector<VariablePtr > vars,
                             std::vector<double* > nlconsGrad, double *xOut,
                             double* objGrad, std::vector<double > dir)
{
  double alpha;
  bool isFound;
  UInt idx, n = minlp_->getNumVars(), numNl = nlCons_.size();
  
 // Determine alpha 
  if (numNl > 0) { 
    if (varPtrs_.size() < numDir_) { 
      setStepSize_(alpha, vars, xOut, dir);
    } else {
      alpha = 0.25; 
    }
  } else if (oNl_) {
    alpha = 1; 
  }
    
  double * gradObj;
  std::vector<double* > gradCons;

  if (oNl_) {
    //MS: try changing this!!!
    gradObj = new double[n];
    std::copy(objGrad, objGrad+n, gradObj);
  } 
  
  if (numNl > 0) {
    gradCons.resize(nlCons_.size(), 0); //MS: Can be made efficient.
  }

  isFound = newPoint_(vars, xOut, alpha, dir);

  if (isFound) {
    if (numNl > 0) {
      for (UInt i = 0; i < nlCons_.size(); ++i) {
        if (nlconsGrad[i]) {
          double * grad = new double[n];
          std::copy(nlconsGrad[i], nlconsGrad[i]+n, grad);
          gradCons[i] = grad;
        }
      }
    }

    while (true) {
      findLinPoint_(xOut, gradCons, gradObj, alpha);
      isFound = newPoint_(vars, xOut, alpha, dir);
      if (!isFound) {
        break;
      }
    }
  }

  if (oNl_) {
    delete [] gradObj;
    gradObj = 0;    
  } 
  
  if (numNl > 0) {
    for (UInt i = 0; i < nlCons_.size(); ++i) {
      if (gradCons[i]) {
        delete [] gradCons[i];
        gradCons[i] = 0;
      }
    }
  }

  for (UInt i = 0; i < vars.size(); ++i) {
    idx = vars[i]->getIndex();
    xOut[idx] = nlpx_[idx];
  }
  return;
}


double Linearizations::angleBetVectors_(double *v1, double *v2, int n)
{
  double angle, PI = 3.1415927, d, m1, m2;
  // compute angle
  d = InnerProduct(v1, v2, n);
  m1 = sqrt(InnerProduct(v1, v1, n));
  m2 = sqrt(InnerProduct(v2, v2, n));
  angle  = d/(m1*m2);
  angle  = acos(angle)*180/PI;
  return angle;
}


bool Linearizations::genLinObj_(double *x, double* &lastGradObj)
{  
  double angle;
  FunctionPtr f;
  bool cutsAdded = false, isCont = false;
  int error = 0, n = minlp_->getNumVars();
  double *a = new double[n];
  std::fill(a, a+n, 0.);
  f = minlp_->getObjective()->getFunction();

  f->evalGradient(x, a, &error);
  if (error == 0) {
    if (lastGradObj == NULL) {
      isCont = true;
    } else {
      angle = angleBetVectors_(a, lastGradObj, n);
    }
    //std::cout << "angle " << angle << "\n";
  
    if (fabs(angle) >= rgs2Per_ || isCont) {
      cutsAdded = objCut_(x);
      if (lastGradObj) {
        delete [] lastGradObj;
        lastGradObj = 0;
      }
      lastGradObj = a;
    } else {
      delete [] a;
      a = 0;      
    }
  } else {
    delete [] a;
    a = 0;    
  }
  return cutsAdded;
}


bool Linearizations::isInteriorPt_(double *xOut,
                                   std::vector<double* > & lastGrad,
                                   double * &lastGradObj, double &alpha)                                  
{
  std::vector<UInt> vConsPos;
  UInt numNl = nlCons_.size();
  std::vector<ConstraintPtr > cons; 
  bool foundActive = false, foundVio = false;

  if (numNl == 0) {
    //MS: are var bounds considered here??
    candConsForObj_(xOut, cons, foundActive, foundVio);
  } else {
    candLinCons_(xOut, vConsPos, foundActive, foundVio);
  }

  if (foundVio) {
    // point outside feasible region. Perform line search.
    bool ptFound;
    double* xnew = new double[minlp_->getNumVars()];
    if (numNl == 0) {
      ptFound = boundaryPtForObj_(xnew, xOut, cons);
    } else {
      ptFound = boundaryPtForCons_(xnew, xOut, vConsPos);
    }
    if (ptFound) {
      genLin_(xnew, vConsPos, lastGrad, lastGradObj, alpha);
    }
    delete [] xnew;
  } else if (foundActive) {
    //point on the boundary. Directly add linearizations.
    genLin_(xOut, vConsPos, lastGrad, lastGradObj, alpha);
  } else {
    return true;  
  }
  return false;
}


void Linearizations::findLinPoint_(double *xOut,
                                   std::vector<double* > & lastGrad,
                                   double * &lastGradObj, double &alpha)
{
  bool interior = isInteriorPt_(xOut, lastGrad, lastGradObj, alpha);

  if (interior) {
    double bnd;
    VariablePtr v;
    double lambda = 0.5;
    UInt n =  minlp_->getNumVars(), vIdx;
    double* x = new double[n];
    std::copy(solC_, solC_+n, x);

    // point inside feasible region. Move along xOut - x^C from x^C
    while (interior) {
      for (UInt i = 0 ; i < n; ++i) {
        x[i] = x[i] + lambda*(xOut[i] - solC_[i]);
      }
      interior = isInteriorPt_(x, lastGrad, lastGradObj, alpha);
      for (UInt i = 0; i < varPtrs_.size(); ++i) {
        v = varPtrs_[i];
        vIdx = v->getIndex();
        bnd = v->getLb();
        if ((x[vIdx] < bnd - solAbsTol_) &&
            (bnd ==0 || (x[vIdx] < bnd - bnd*solRelTol_))) {
          interior = false;
          break;
        }
        bnd = v->getUb();
        if ((x[vIdx] > bnd + solAbsTol_) &&
            (bnd==0 || (x[vIdx] > bnd + bnd*solRelTol_))) {
          interior = false;
          break;
        }
      }
    }
    delete [] x;
  }
  return;
}


bool Linearizations::newPoint_(std::vector<VariablePtr> vars, double *xOut,
                               double alpha, std::vector<double> dir)
{
  UInt idx;
  for (UInt i = 0; i < vars.size(); ++i) {
    idx = vars[i]->getIndex();
    xOut[idx] = xOut[idx] + alpha*dir[i];
    if (!(boundCheck_(dir[i], xOut[idx], vars[i]))) {
      return false;
    }
  }
  return true;
}


bool Linearizations::boundCheck_(double dirVal, double varVal, VariablePtr v)
{ 
  double bound;
  if (dirVal > 0) {
    bound = v->getUb();
    if (bound == INFINITY) {
      bound = nlpx_[v->getIndex()] + 50; //MS: can be parameterized
    }
    if (varVal - bound > solAbsTol_) {
      return false;
    }
  } else {
    bound = v->getLb();
    if (bound == -INFINITY) {
      bound = nlpx_[v->getIndex()] - 50; // MS: can be parameterized
    }
    if (bound - varVal > solAbsTol_) {
      return false;
    }
  }
  return true;
}


void Linearizations::exploreDir_(std::vector<VariablePtr > vars,
                                 std::vector<double > dir, double* xOut,
                                 double* objGrad,
                                 std::vector<double* > nlconsGrad)
{
  
  if (!(atBound_(dir, vars))) {
    search_(vars, nlconsGrad, xOut, objGrad, dir);
  }
  
  // Reverse direction
  for (UInt i = 0; i < dir.size(); ++i) {
    dir[i] = -dir[i];
  }

  if (!(atBound_(dir, vars))) {
    search_(vars, nlconsGrad, xOut, objGrad, dir);
  }
}


//// For 1/-1 components in unit direction
void Linearizations::rootLinGenScheme2_()
{
  VariablePtr v;
  int firstnnz= -1;
  UInt numVars = varPtrs_.size(), vIdx;
  
  //// variable to be fixed in finding search direction
  if (numVars > 1) {
    for (UInt i = 0; i < numVars; ++i) {
      v = varPtrs_[i];
      vIdx = v->getIndex();
      if ((firstnnz == -1) && (fabs(solC_[vIdx] - nlpx_[vIdx]) != 0)) {
        firstnnz = i;
        break;
      }
    }

    if (firstnnz == -1) {
      return;
    }
  }

  int error;
  FunctionPtr f;
  double * objGrad = 0;
  ConstraintPtr con;
  std::vector<double > dir; 
  std::vector<VariablePtr > vars;
  UInt n = minlp_->getNumVars(), numOldCuts = stats_->cuts;
  
  std::vector<double* > nlconsGrad;
  
  double *xOut = new double[n]; 
  std::copy(nlpx_, nlpx_+n, xOut);
  
  if (oNl_) {
    double *objgrad = new double[n];
    std::fill(objgrad, objgrad+n, 0.);
    f = minlp_->getObjective()->getFunction();
    f->evalGradient(nlpx_, objgrad, &error);
    if (error != 0) {
      delete [] objgrad;
      objGrad = 0;      
    } else {
      objGrad = objgrad;    
    }
  }
  
   ////// Gradient of constraints at nonlinear solution nlpx_

  if (nlCons_.size() > 0) {
    for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
      con = *it;
      error = 0;
      f = con->getFunction();
      double *grad = new double[n];
      std::fill(grad, grad+n, 0);
      f->evalGradient(nlpx_, grad, &error);
      if (error == 0) {
        nlconsGrad.push_back(grad);
      } else {
        nlconsGrad.push_back(0);
      }
    }
  }
  
  if (nlCons_.size() > 0 || (!isBoundPt_ && !hasEqCons_)) {
  // coordinate direction is considered if there is only single variable
    std::vector<double > lastDir;
    lastDir.resize(numVars, 0);
    for (UInt i = 0; i < numVars; ++i) {
      v = varPtrs_[i];
      vIdx = v->getIndex();
      if ((int(i) < firstnnz) || (fabs(solC_[vIdx] - nlpx_[vIdx]) < solAbsTol_)
          || (numVars == 1)) {
        // Coeff of var is zero in the hyperplane expression
        // for last direction
        //lastDir[i] = -1;
        //if (firstnnz != -1) {
          //lastDir[firstnnz] = lastDir[firstnnz] + 1;
        //}
        
        dir.push_back(1);
        vars.push_back(v);
        exploreDir_(vars, dir, xOut, objGrad, nlconsGrad);
      } else if (int(i) > firstnnz) {
        // for last direction
        //lastDir[i] = -1;
        //lastDir[firstnnz] = lastDir[firstnnz] + 1;
        
        // unit vector
        dir.push_back(1);
        dir.push_back(-1);
        vars.push_back(v);
        vars.push_back(varPtrs_[firstnnz]);
        exploreDir_(vars, dir, xOut, objGrad, nlconsGrad);
      } else {
        continue;    
      }
      dir.clear();
      vars.clear();
    }
    
    // for last direction
    //if (numVars > 1) {
      //exploreDir_(varPtrs_, lastDir, xOut, objGrad, nlconsGrad);
    //} 
  } else {
    //// Line search between center and nlp solution
    //std::cout << isBoundPt_ << " " << hasEqCons_ << "\n";
    FunctionType type;
    bool cutsAdded, isCont = true;
    double dist = InnerProduct(solC_, nlpx_, n), bound;
    if (fabs(dist) > solAbsTol_) {
      double alpha = 0.25;
      double * grad = new double[n];
      std::copy(objGrad, objGrad+n, grad);
      //while (alpha <= 1) {
      while (true) {
        for (UInt i = 0; i < n; ++i) {
          xOut[i] = xOut[i] + alpha * (solC_[i] - nlpx_[i]);
        }
  
        cutsAdded = genLinObj_(xOut, grad);
        if (!cutsAdded) {
          alpha = 2*alpha;
        }

        for (VariableConstIterator vit = minlp_->varsBegin(); 
             vit != minlp_->varsEnd(); ++vit) {
          v = *vit;
          type = v->getFunType();
          if ((type == Linear || type == Constant)) {
            continue;
          }
          bound = v->getUb();
          vIdx = v->getIndex();
          if (bound == INFINITY) {
            bound = nlpx_[vIdx] + 50; //MS: can be parameterized
          }
          if (xOut[vIdx] - bound > solAbsTol_) {
            isCont = false;
            break;
          }
          bound = v->getLb();
          if (bound == -INFINITY) {
            bound = nlpx_[vIdx] - 50; //MS: can be parameterized
          }
          if (bound - xOut[vIdx] > solAbsTol_) {
            isCont = false;
            break;
          }
        }
        if (!isCont) {
          break;        
        }
      }
      delete [] grad;
      grad = 0;
    }
  }

  for (UInt i = 0; i < nlCons_.size(); ++i) {
    if (nlconsGrad[i]) {
      delete [] nlconsGrad[i];
      nlconsGrad[i] = 0;
    }
  }

  delete [] xOut;
  if (objGrad) {
    delete [] objGrad;
    objGrad = 0;  
  }
  stats_->rgs2Cuts = stats_->cuts - numOldCuts; 
  return;
}

//// For unit direction
//void Linearizations::rootLinGenScheme2_()
//{
  //VariablePtr v;
  //int firstnnz = -1;
  //double fixCoeff, rhs = 0.0;
  //UInt fixIdx, numVars = varPtrs_.size(), vIdx;
  
  ////// variable to be fixed in finding search direction
  //if (numVars > 1) {
    //for (UInt i = 0; i < numVars; ++i) {
      //v = varPtrs_[i];
      //vIdx = v->getIndex();
      //rhs = rhs + nlpx_[vIdx]*(solC_[vIdx] - nlpx_[vIdx]);
      //if ((firstnnz == -1) && (fabs(solC_[vIdx] - nlpx_[vIdx]) > solAbsTol_)) {
        //firstnnz = i;
        //fixIdx = vIdx;
      //}
    //}

    //// For unit Direction
    //if (firstnnz == -1) {
      //return;
    //}

    //if (rhs == 0) {
      //// find a parallel plane
      //rhs = 10;
    //}
    //// coefficient of the fix var in any direction
    //fixCoeff = rhs/(solC_[fixIdx] - nlpx_[fixIdx]);
  //}


  //int error;
  //FunctionPtr f;
  //double * objGrad = 0;
  //ConstraintPtr con;
  //std::vector<double > dir;
  //std::vector<VariablePtr > vars;
  //double coeff, lastVal = 0, val;   
  //UInt n = minlp_->getNumVars(), numOldCuts = stats_->cuts;
  
  //std::vector<double* > nlconsGrad;
  
  //double *xOut = new double[n]; 
  //std::copy(nlpx_, nlpx_+n, xOut);


  //if (oNl_) {
    //double *objgrad = new double[n];
    //std::fill(objgrad, objgrad+n, 0.);
    //f = minlp_->getObjective()->getFunction();
    //f->evalGradient(nlpx_, objgrad, &error);
    //if (error != 0) {
      //delete [] objgrad;
      //objGrad = 0;      
    //} else {
      //objGrad = objgrad;    
    //}
  //}
  
   ////// Gradient of constraints at nonlinear solution nlpx_

  //if (nlCons_.size() > 0) {
    //for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
      //con = *it;
      //error = 0;
      //f = con->getFunction();
      //double *grad = new double[n];
      //std::fill(grad, grad+n, 0);
      //f->evalGradient(nlpx_, grad, &error);
      //if (error == 0) {
        //nlconsGrad.push_back(grad);
      //} else {
        //nlconsGrad.push_back(0);
      //}
    //}
  //}
  //if (nlCons_.size() > 0 || (!isBoundPt_ && !hasEqCons_)) {
  //// coordinate direction is considered if there is only single variable
    //std::vector<double > lastDir;
    //lastDir.resize(numVars, 0); 
    //for (UInt i = 0; i < numVars; ++i) {
      //v = varPtrs_[i];
      //vIdx = v->getIndex();
      //if ((int(i) < firstnnz) || (fabs(solC_[vIdx] - nlpx_[vIdx]) < solAbsTol_)
          //|| (numVars == 1)) {
        //// Coeff of var is zero in the hyperplane expression
        //// for last direction
        //lastDir[i] = -1;
        //lastVal = lastVal + 1;
        
        //dir.push_back(1);
        //vars.push_back(v);
        //exploreDir_(vars, dir, xOut, objGrad, nlconsGrad);
      //} else if (int(i) > firstnnz) {
        //coeff = rhs/(solC_[vIdx] - nlpx_[vIdx]);
        //// for last direction
        //lastDir[i] = -coeff;
        //lastVal = lastVal + pow(coeff, 2);
        //lastDir[firstnnz] = lastDir[firstnnz] + fixCoeff;
    
        //// unit vector
        //val = sqrt(pow(coeff,2) + pow(fixCoeff,2));
        //dir.push_back(coeff/val);
        //dir.push_back(-fixCoeff/val);
        //vars.push_back(v);
        //vars.push_back(varPtrs_[firstnnz]);
        //exploreDir_(vars, dir, xOut, objGrad, nlconsGrad);
      //} else {
        //continue;    
      //}
      //dir.clear();
      //vars.clear();
    //}
    
    //if (numVars > 1) {
      ///// last direction in positive spanning set 
      //lastVal = lastVal + pow(lastDir[fixIdx], 2);
      //lastVal = sqrt(lastVal);

      //for (UInt i = 0; i < varPtrs_.size(); ++i) {
        //dir.push_back(lastDir[i]/lastVal);
      //}
      //exploreDir_(varPtrs_, dir, xOut, objGrad, nlconsGrad);
    //} 
  //} else {
    ////// Line search between center and nlp solution
    //std::cout << isBoundPt_ << " " << hasEqCons_ << "\n";
    //FunctionType type;
    //bool cutsAdded, isCont = true;
    //double dist = InnerProduct(solC_, nlpx_, n), bound;
    //if (fabs(dist) > solAbsTol_) {
      //double alpha = 0.25;
      //double * grad = new double[n];
      //std::copy(objGrad, objGrad+n, grad);
      ////while (alpha <= 1) {
      //while (true) {
        //for (UInt i = 0; i < n; ++i) {
          //xOut[i] = xOut[i] + alpha * (solC_[i] - nlpx_[i]);
        //}
  
        //cutsAdded = genLinObj_(xOut, grad);
        //if (!cutsAdded) {
          //alpha = 2*alpha;
        //}

        //for (VariableConstIterator vit = minlp_->varsBegin(); 
             //vit != minlp_->varsEnd(); ++vit) {
          //v = *vit;
          //type = v->getFunType();
          //if ((type == Linear || type == Constant)) {
            //continue;
          //}
          //bound = v->getUb();
          //vIdx = v->getIndex();
          //if (bound == INFINITY) {
            //bound = nlpx_[vIdx] + 50; //MS: can be parameterized
          //}
          //if (xOut[vIdx] - bound > solAbsTol_) {
            //isCont = false;
            //break;
          //}
          //bound = v->getLb();
          //if (bound == -INFINITY) {
            //bound = nlpx_[vIdx] - 50; //MS: can be parameterized
          //}
          //if (bound - xOut[vIdx] > solAbsTol_) {
            //isCont = false;
            //break;
          //}
        //}
        //if (!isCont) {
          //break;        
        //}
      //}
      //delete [] grad;
      //grad = 0;
    //}
  //}

  //for (UInt i = 0; i < nlCons_.size(); ++i) {
    //if (nlconsGrad[i]) {
      //delete [] nlconsGrad[i];
      //nlconsGrad[i] = 0;
    //}
  //}

  //delete [] xOut;
  //if (objGrad) {
    //delete [] objGrad;
    //objGrad = 0;  
  //}
  //stats_->rgs2Cuts = stats_->cuts - numOldCuts; 
  //return;
//}


bool Linearizations::atBound_(std::vector<double > dir,
                              std::vector<VariablePtr > vars)
{
  UInt vIdx;
  VariablePtr v;
  for (UInt i = 0; i < vars.size(); ++i) {
    v = vars[i];
    vIdx = v->getIndex();
    if (dir[i] > 0) {
      if (fabs(v->getUb() - nlpx_[vIdx]) <= solAbsTol_) {
        return true;
      }
    } else {
       if (fabs(nlpx_[vIdx] - v->getLb()) <= solAbsTol_) {
        return true;
      }
    }
  }
  return false;
}


void Linearizations::rootLinGenScheme1_()
{
  double val;   
  VariablePtr v;
  //double bnd, vLb = INFINITY, vUb = INFINITY; // for last direction 
  UInt vIdx, numOldCuts = stats_->cuts, n = minlp_->getNumVars();
  
  double *xOut = new double[n];
  std::copy(solC_, solC_ + n, xOut);
  
  // coordinate direction along each variable in varPtrs_.
  //if (!isBoundPt_) 
  if (nlCons_.size() > 0 || (!isBoundPt_ && !hasEqCons_)) {
    changeVar_.push_back(0);
    for (UInt i = 0; i < varPtrs_.size(); ++i) {
      v = varPtrs_[i];
      vIdx = v->getIndex();
      changeVar_[0] = vIdx;
      // determine indices of variables that are bounding in the last
      // direction (and its opposite) of search and bound values 
      val = v->getUb();
      
      //// for last direction
      //bnd = val - solC_[vIdx];
      //if (bnd < vUb) {
        //vUb = bnd;
      //}

      // coordinate direction for each variable 
      if (val == INFINITY) {
        xOut[vIdx] = xOut[vIdx] + 50;       // if variable is unbounded above
      } else {
        xOut[vIdx] = val;
      }
      cutsAtBoundary_(xOut);
      xOut[vIdx] = solC_[vIdx];
      
      /// Reverse search direction if previous direction was unsuccessful 
      val = v->getLb();
      //// for last direction
      //bnd = solC_[vIdx] - val;
      //if (bnd < vLb) {
        //vLb = bnd;
      //}

      if (val == -INFINITY) {
        xOut[vIdx] = xOut[vIdx] - 50;
      } else {
        xOut[vIdx] = val;
      }
      cutsAtBoundary_(xOut);
      xOut[vIdx] = solC_[vIdx];
    }
      
    changeVar_.clear();
    
    //// Last direction in positive spanning set
    //if (vUb == INFINITY) {
      //vUb = 50;
    //}
    //for (UInt i = 0 ; i < n; ++i) {
      //xOut[i] = xOut[i] + vUb;
    //}
    //for (UInt i = 0; i < varPtrs_.size(); ++i) {
      //changeVar_.push_back(varPtrs_[i]->getIndex());
    //}
    //cutsAtBoundary_(xOut);

    ////// Reverse direction 
    //std::copy(solC_, solC_ + n, xOut);
    //if (vLb == INFINITY) {
      //vLb = 50;
    //}
    //for (UInt i = 0 ; i < n; ++i) {
      //xOut[i] = xOut[i] - vLb;
    //}
    //cutsAtBoundary_(xOut);

    //changeVar_.clear();
  } else {
    //// Line search between center and nlp solution
    bool isCont = true;
    FunctionType type;
    //std::cout << isBoundPt_ << " " << hasEqCons_ << "\n";
    double dist = InnerProduct(solC_, nlpx_, n), bound;
    if (fabs(dist) > solAbsTol_) {
      double *xOut1 = new double[n];
      std::fill(xOut1, xOut1+n, 0.);
      double alpha = 0.2;   // MS: can be parameterized.
      while (alpha <= 1) {
        if (isCont) {
          for (UInt i = 0; i < n; ++i) {
            xOut[i] = solC_[i] + alpha * (nlpx_[i] - solC_[i]);
            xOut1[i] = solC_[i] - alpha * (nlpx_[i] - solC_[i]);
          }
        } else {
           for (UInt i = 0; i < n; ++i) {
            xOut[i] = solC_[i] + alpha * (nlpx_[i] - solC_[i]);
          }       
        }
        objCut_(xOut);
        if (isCont) {
          for (VariableConstIterator vit = minlp_->varsBegin(); 
               vit != minlp_->varsEnd(); ++vit) {
            v = *vit;
            type = v->getFunType();
            if ((type == Linear || type == Constant)) {
              continue;
            }
            bound = v->getUb();
            vIdx = v->getIndex();
            if (bound == INFINITY) {
              bound = nlpx_[vIdx] + 50; //MS: can be parameterized
            }
            if (xOut[vIdx] - bound > solAbsTol_) {
              isCont = false;
              break;
            }
            bound = v->getLb();
            if (bound == -INFINITY) {
              bound = nlpx_[vIdx] - 50; //MS: can be parameterized
            }
            if (bound - xOut[vIdx] > solAbsTol_) {
              isCont = false;
              break;
            }
          }
        
          if (isCont) {
            objCut_(xOut1);
          }
        }
        alpha = alpha + 0.2;  //MS: can be parameterized.
      }
      delete [] xOut1;
    } else {
      std::cout << "Interior and center are same\n";
    }
  } 
    
  delete [] xOut;
  stats_->rgs1Cuts = stats_->cuts - numOldCuts; 
  return;
}


void Linearizations::candConsForObj_(double *xOut,
                                     std::vector<ConstraintPtr > &consToLin,
                                     bool &active, bool &vio)
{
  //int error = 0;
  ConstraintPtr con;
  FunctionType fType;
  double lb, ub, act;
  LinearFunctionPtr lf;
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd();
       ++it) {
    con = *it;
    ub = con->getUb();
    fType = con->getFunctionType();
    if (fType == Linear) {
      lb = con->getLb();
      lf = con->getLinearFunction();
      act = lf->eval(xOut);
      if (ub != INFINITY && ((act > ub + solAbsTol_) &&
          (ub == 0 || act > ub + fabs(ub)*solRelTol_))) {
        if (!vio) {
          vio = true;
          if (consToLin.size() != 0) {
            consToLin.clear();          
          }         
        }
        consToLin.push_back(con);
      } else if (ub != INFINITY && ((fabs(act-ub) <= solAbsTol_) ||
                 (ub != 0 && fabs(act- ub) <= fabs(ub)*solRelTol_))) {
        if (!vio) {
          if (!active) {
            active = true;         
          }
          consToLin.push_back(con);
        }
      } else if (lb != -INFINITY && ((-act > -lb + solAbsTol_) &&
          (lb == 0 || -act > -lb + fabs(lb)*solRelTol_))) {
        if (!vio) {
          vio = true;
          if (consToLin.size() != 0) {
            consToLin.clear();          
          }         
        }
        consToLin.push_back(con);
      } else if (lb != -INFINITY && ((fabs(-act+lb) <= solAbsTol_) ||
                 (lb != 0 && fabs(-act+lb) <= fabs(lb)*solRelTol_))) {
        if (!vio) {
          if (!active) {
            active = true;         
          }
          consToLin.push_back(con);
        }
      }
    } else if (fType == Constant) {
      continue;    
    } 
  }

  if (minlp_->getNumCons() == 0) {
    active = true;  
  } 

  return;
}


void Linearizations::cutsAtBoundary_(double *xOut)
{
  UInt numNl = nlCons_.size();
  std::vector<UInt > consToLin; // cons to add linearizations
  std::vector<ConstraintPtr > cons; 
  bool active = false, vio = false;
  
  if (numNl == 0) {
    candConsForObj_(xOut, cons, active, vio);
  } else {
    candLinCons_(xOut, consToLin, active, vio);
  }
  
  if (vio) {
    // point outside feasible region. Perform line search.
    bool ptFound;
    double* xnew = new double[minlp_->getNumVars()];
    if (numNl == 0) {
      ptFound = boundaryPtForObj_(xnew, xOut, cons);
    } else {
      ptFound = boundaryPtForCons_(xnew, xOut, consToLin);
    }
    if (ptFound) {
      genLin_(xnew, consToLin);
    }
    delete [] xnew;
  } else if (active) {
    //point on the boundary. Directly add linearizations.
    genLin_(xOut, consToLin);
  } 
  return;
}


void Linearizations::varsInNonlinCons_()
{
  //// K is used to limit number of directions in root schemes 4 and 5
  VariablePtr v;
  FunctionType type;
  for (VariableConstIterator vit = minlp_->varsBegin(); 
       vit != minlp_->varsEnd(); ++vit) {
    v = *vit;
    type = v->getFunType();
    if (!(type == Linear || type == Constant)) {
      //std::cout << v->getName() << std::endl;
      varPtrs_.push_back(v);
    }
    if (varPtrs_.size() == numDir_) {
      break;    
    }
  }
  //exit(1);
  return;
}


void Linearizations::genLin_(double *x, std::vector<UInt > vioConsPos,
                                     std::vector<double *> &lastGrad,
                                     double* &lastGradObj, double &alpha)
{ 
  UInt cIdx;
  FunctionPtr f;
  bool isCont, cutsAdded = 0;
  int error, n = minlp_->getNumVars();
  double angle, cUb, act, c;
 

  if (oNl_) {
    cutsAdded = genLinObj_(x, lastGradObj);
  } 
  
  if (nlCons_.size() > 0) {
    int nr = rel_->getNumVars();
    ConstraintPtr con;
    std::stringstream sstm;
    LinearFunctionPtr lf = 0;
    VariableConstIterator vbeg = rel_->varsBegin(), vend = rel_->varsEnd();
    const double linCoeffTol =
    env_->getOptions()->findDouble("conCoeff_tol")->getValue();
    for (UInt j = 0; j < vioConsPos.size(); ++j) {
      error = 0;
      isCont = false;
      double *a = new double[nr];
      std::fill(a, a+nr, 0.);
      cIdx = vioConsPos[j];

      con = nlCons_[cIdx];
      f = con->getFunction();

      f->evalGradient(x, a, &error);
      if (error == 0) {
        if (lastGrad[cIdx] == NULL) {
          isCont = true;
        } else {
          // compute angle
          angle = angleBetVectors_(lastGrad[cIdx], a, n);
        }
      } else {
        delete [] a;
        a = 0;
        continue;
      }

      //std::cout << "angle " << angle << "\n";
      if (fabs(angle) >= rgs2Per_ || isCont) {
        cUb = con->getUb();
        act = con->getActivity(x, &error);
        if (error == 0) {
          cutsAdded = 1;
          lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol);
          c  = act - InnerProduct(x, a, minlp_->getNumVars());
          ++(stats_->cuts);
          sstm << "_OACutRoot_" << stats_->cuts;
          f = (FunctionPtr) new Function(lf);
          rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
          sstm.str("");
          if (lastGrad[cIdx]) {
            delete [] lastGrad[cIdx];
            lastGrad[cIdx] = 0;
          }
          lastGrad[cIdx] = a;
        } else {
          delete [] a;
          a = 0;
        }
      } else {
        delete [] a;
        a = 0;
      }
    }
  }
  if (!cutsAdded) {
    alpha = 2*alpha;
  }
  return;
}


void Linearizations::genLin_(const double *x, std::vector<UInt > vioCons)
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
        sstm << "_OACutRoot_" << stats_->cuts;
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      } 
    }
  }

  if (oNl_) {
    objCut_(x);
  }
  return;
}


bool Linearizations::boundaryPtForObj_(double* xnew, const double *xOut,
                                     std::vector<ConstraintPtr> &vioCons)
{ 
  UInt j;
  ConstraintPtr con;
  int error = 0, repPt = 0;
  bool firstVio, firstActive;
  UInt numVars =  minlp_->getNumVars(), varToChange = changeVar_.size(); 

  double* xl = new double[numVars];
  double* xu = new double[numVars];
  double cUb, act, lb, repSol, repSolOld = INFINITY, lambdaIn = 0.5,
         lambdaOut = 0.5;

  std::copy(xOut, xOut+numVars, xu);
  std::copy(solC_, solC_+numVars, xl);

  if (varToChange > 0) {
    std::copy(solC_, solC_+numVars, xnew);
  }


  while (true) {
    if (varToChange == 0) {
      for (UInt i = 0 ; i < numVars; ++i) {
        xnew[i] = lambdaIn*xl[i] + lambdaOut*xu[i];
      }
    } else {
      for (UInt i = 0 ; i < varToChange; ++i) {
        j = changeVar_[i];
        xnew[j] = lambdaIn*xl[j] + lambdaOut*xu[j];
      }
    }
    repSol = 0;
    firstVio = false, firstActive = false;

    for (UInt k = 0; k < vioCons.size(); ) {
      con = vioCons[k];
      act = con->getActivity(xnew, &error);
      if (error != 0) {
        delete [] xl;
        delete [] xu;
        return false;
      }
      lb = con->getLb();
      cUb = con->getUb();
      repSol = repSol + act;
      if (cUb != INFINITY && ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_))) { // violated
        if (!firstVio) {
          firstVio = true;
          if (k != 0) {
            vioCons.erase(vioCons.begin(), vioCons.begin() + k);
            k = 0;
          }
        }
        ++k;
      } else if (cUb != INFINITY && ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_))) { // active
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
      }  else if (lb != -INFINITY && ((-act > -lb + solAbsTol_) &&
          (lb == 0 || -act > -lb + fabs(lb)*solRelTol_))) { // violated
        if (!firstVio) {
          firstVio = true;
          if (k != 0) {
            vioCons.erase(vioCons.begin(), vioCons.begin() + k);
            k = 0;
          }
        }
        ++k;
      } else if (lb != -INFINITY && ((fabs(-act+lb) <= solAbsTol_) ||
            (lb != 0 && fabs(-act+lb) <= fabs(lb)*solRelTol_))) { // active
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
    
    if (fabs(repSol-repSolOld) <= solAbsTol_) {
      ++repPt;
    } else {
      repPt = 0;
      repSolOld = repSol;
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

bool Linearizations::boundaryPtForCons_(double* xnew, const double *xOut,
                                     std::vector<UInt > &vioCons)
{
  UInt j;
  ConstraintPtr con;
  int error = 0, repPt = 0;
  bool firstVio, firstActive;
  UInt numVars =  minlp_->getNumVars(), varToChange = changeVar_.size(); 

  double* xl = new double[numVars];
  double* xu = new double[numVars];
  double act, cUb, repSol, oldSol = INFINITY, lambdaIn = 0.5, lambdaOut = 0.5;

  std::copy(xOut, xOut+numVars, xu);
  std::copy(solC_, solC_+numVars, xl);
    
  if (varToChange > 0) {
    std::copy(solC_, solC_+numVars, xnew);
  }

  while (true) {
    if (varToChange == 0) {
      for (UInt i = 0 ; i < numVars; ++i) {
        xnew[i] = lambdaIn*xl[i] + lambdaOut*xu[i];
      }
    } else {
      for (UInt i = 0 ; i < varToChange; ++i) {
        j = changeVar_[i];
        xnew[j] = lambdaIn*xl[j] + lambdaOut*xu[j];
      }
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


void Linearizations::rootLinScheme1_(FunctionPtr fun, double lVarCoeff,
                            UInt lVarIdx, UInt nVarIdx, double nVarCoeff,
                            double UB, bool isObj)
{
  double iP[2]; // intersection point
  UInt newConId;
  ConstraintPtr newcon;
  bool shouldCont, varDel;
  std::vector<UInt > newConsId;
  VariablePtr vnl = NULL, vl = NULL;
  std::vector<double > linVioVal, xc, yc; // xc and yc  nonlinear and lin var
  int i, error = 0, n = rel_->getNumVars();
  double act, vio, cUb, y1, y2, vLb, vUb, maxVio, stopCond; 

  double *b1 = new double[n];
  std::fill(b1, b1+n, 0.);
  
  vl = rel_->getVariable(lVarIdx); vnl = rel_->getVariable(nVarIdx);  
  vLb = vnl->getLb(); vUb = vnl->getUb();

  if (vLb == -INFINITY) { // MS: this can be parametrized later if need be.
    //std::cout << "Infi case: LB infi\n";
    vLb = nlpx_[nVarIdx] - 50;
    if (vUb == INFINITY) {
      //std::cout << "Infi case: UB also infi\n";
      vUb = nlpx_[nVarIdx] + 50;
    }
  } else {
    if (vUb == INFINITY) {
      //std::cout << "Infi case: UB infi\n";
      vUb = nlpx_[nVarIdx] + 50;
    } 
  }
    
  b1[nVarIdx] = vLb;
  act = nVarCoeff*vLb;
  shouldCont = linPart_(b1, lVarIdx, lVarCoeff, act, fun, UB);
  if (shouldCont) {
    shouldCont = addCutAtRoot_(b1, fun, newConId, UB, isObj);
    if (shouldCont) {
      ++(stats_->rs1Cuts);
      y1 = b1[lVarIdx];
      newConsId.push_back(newConId); 
    } else {
      delete [] b1;
      return;    
    }
  } else {
    delete [] b1;
    return;    
  }
  // upper bound of var in nonlinear cons
  b1[nVarIdx] = vUb;
  act = nVarCoeff*vUb;
  shouldCont = linPart_(b1, lVarIdx, lVarCoeff, act, fun, UB);  
  if (shouldCont) {
    shouldCont = addCutAtRoot_(b1, fun, newConId, UB, isObj);
    if (shouldCont) {
      ++(stats_->rs1Cuts);
      y2 = b1[lVarIdx];
      newConsId.push_back(newConId); 
    } else {
      delete [] b1;
      return;    
    }
  } else {
    delete [] b1;
    return;    
  }

  shouldCont = findIntersectPt_(newConsId, vl, vnl, iP);
  newConsId.clear();
  if (!shouldCont) {
    delete [] b1;
    return;    
  }

  // populate points and their violation in cyclic order from left to right
  xc.push_back(vLb);
  yc.push_back(y1);
  linVioVal.push_back(0);
      
  b1[nVarIdx] = iP[0], b1[lVarIdx] = iP[1];
  
  act = fun->eval(b1, &error);
  if (error != 0) {
    delete [] b1;
    return;    
  }
  if (isObj) {
    if (lVarIdx == objVar_->getIndex()) {
      act = act - b1[lVarIdx];
    } 
  }
  
  vio = std::max(act-UB, 0.0); 
  xc.push_back(iP[0]), yc.push_back(iP[1]), linVioVal.push_back(vio);
  xc.push_back(vUb), yc.push_back(y2), linVioVal.push_back(0);

  // starting from intersection point
  i = 1;
  maxVio = linVioVal[i];
  if (fabs(UB) > solAbsTol_) { 
    stopCond = UB*rs1_/100;    
  } else {
    stopCond = maxVio*rs1_/100;    
  }

  if ((stopCond < solAbsTol_) || 
      (UB != 0 && stopCond < fabs(UB)*solRelTol_)) { 
    delete [] b1;
    return;
  }

  while (maxVio >= stopCond) { 
    //add a new cut at the point indexed i
    varDel = false;
    b1[nVarIdx] = xc[i];
    shouldCont = addCutAtRoot_(b1, fun, newConId, UB, isObj);
    if (shouldCont) {
      ++(stats_->rs1Cuts);
      newcon = rel_->getConstraint(newConId);
      cUb = newcon->getUb();
      // Move right and determine first point that satisfy the newcon
      for (UInt j = i+1; j < xc.size(); ) {
        b1[nVarIdx] = xc[j], b1[lVarIdx] = yc[j];
        act = newcon->getActivity(b1, &error);
        if (error == 0) {
          if ((act < cUb + solAbsTol_) ||
              (cUb =! 0 && act < cUb + fabs(cUb)*solRelTol_)) {
            //insert new point just before index j
            insertNewPt_(j, j-1, xc, yc, newcon, vl, vnl, shouldCont); 
            // compute violation at the new point
            if (shouldCont) {
              b1[nVarIdx] = xc[j], b1[lVarIdx] = yc[j];
              act = fun->eval(b1, &error);
              if (error != 0) {
                xc.erase(xc.begin() + j);
                yc.erase(xc.begin() + j);
              } else {
                if (isObj) {
                  if (lVarIdx == objVar_->getIndex()) {
                    act = act - b1[lVarIdx];
                  } 
                }
                vio = std::max(act-UB, 0.0); 
                linVioVal.insert(linVioVal.begin()+j,vio);
                break;
              }
            } 
          } else {
            // delete point if violates newcon
            updateInfo_(xc, yc, linVioVal, j);
          }
        }
      }

      int j = i-1;
      while (j >= 0) {
        b1[nVarIdx] = xc[j], b1[lVarIdx] = yc[j];
        act = newcon->getActivity(b1, &error);
        if (error == 0) {
          if ((act < cUb + solAbsTol_) ||
              (cUb =! 0 && act < cUb + fabs(cUb)*solRelTol_)) {
            insertNewPt_(j+1, j, xc, yc, newcon, vl, vnl, shouldCont); 
            if (shouldCont) {
              b1[nVarIdx] = xc[j+1], b1[lVarIdx] = yc[j+1];
              act = fun->eval(b1, &error);
              if (error != 0) {
                xc.erase(xc.begin() + (j+1));
                yc.erase(yc.begin() + (j+1));
              } else {
                varDel = true;
                if (isObj) {
                  if (lVarIdx == objVar_->getIndex()) {
                    act = act - b1[lVarIdx];
                  } 
                }
                vio = std::max(act-UB, 0.0); 
                linVioVal.insert(linVioVal.begin()+j+1,vio);
                updateInfo_(xc, yc, linVioVal, j+2);
                break;
              }
            }
          }  else {
            // delete point if violates newcon
            updateInfo_(xc, yc, linVioVal, j);
            --i;
            --j;
          }  
        }
      }
    } 
    if (!varDel) {
      updateInfo_(xc, yc, linVioVal, i);
    }
    maxVio = *(std::max_element(linVioVal.begin(), linVioVal.end()));
    if ((maxVio <= solAbsTol_) || 
        (UB !=0 && maxVio <= fabs(UB)*solRelTol_ )) { 
      break;
    }
    i = std::max_element(linVioVal.begin(), linVioVal.end())-linVioVal.begin();     
  }
  delete [] b1;
  return;
}


void Linearizations::updateInfo_(std::vector<double > &xc,
                                 std::vector<double > &yc,
                                 std::vector<double > &linVioVal, UInt j)
{

  xc.erase(xc.begin() + j); 
  yc.erase(yc.begin() + j);
  linVioVal.erase(linVioVal.begin() + j);
  return;
}


void Linearizations::rootLinScheme2_(FunctionPtr f, double UB,
                                     double lVarCoeff,
                                     UInt lVarIdx, UInt nVarIdx, bool isObj)
{
  int error = 0;
  VariablePtr vnl;
  double lastSlope, delta, nlpSlope, nbhSize;
  UInt n = minlp_->getNumVars(), numOldCuts = stats_->cuts;
  
  vnl = rel_->getVariable(nVarIdx);
  
  double* npt = new double[n];
  std::fill(npt, npt+n, 0.);
  
  double *grad = new double[n];
  std::fill(grad, grad+n, 0.);
  
  f->evalGradient(nlpx_, grad, &error);

  if (error == 0) {
    nlpSlope = -1*(grad[nVarIdx]/lVarCoeff);
    lastSlope = nlpSlope;       // nlpSlope is going to be used later on as well
    
    nbhSize = std::max(vnl->getLb(), nlpx_[nVarIdx] - rs2NbhSize_);    
    //nbhSize = vnl->getLb();    
    if (nlpx_[nVarIdx] - nbhSize >= 1) {
      delta = 0.5;  
    } else {
      delta = nlpx_[nVarIdx] - nbhSize;  
    }

    npt[nVarIdx] = nlpx_[nVarIdx] - delta;
       
    if (delta != 0) {
      while (npt[nVarIdx] >= nbhSize) {
        grad[nVarIdx] = 0; grad[lVarIdx] = 0;
        rScheme2Cut_(f, UB, delta, lVarCoeff, lastSlope, nVarIdx, npt, grad,
                     isObj);
        npt[nVarIdx] =  npt[nVarIdx] - delta;
      }
    }
    
    nbhSize = std::min(vnl->getUb(), nlpx_[nVarIdx] + rs2NbhSize_);
    //nbhSize = vnl->getUb();    
    if (nbhSize - nlpx_[nVarIdx] >= 1) {
      delta = 0.5;  //MS: can be a parameter  
    } else {
      delta = nbhSize - nlpx_[nVarIdx];  
    }

    lastSlope = nlpSlope;
    npt[nVarIdx] = nlpx_[nVarIdx] + delta;

    if (delta != 0) {
      while (npt[nVarIdx] <= nbhSize) {
        grad[nVarIdx] = 0; grad[lVarIdx] = 0;
        rScheme2Cut_(f, UB, delta, lVarCoeff, lastSlope, nVarIdx, npt, grad,
                     isObj);
        npt[nVarIdx] =  npt[nVarIdx] + delta;
      }
    }
  }
  stats_->rs2Cuts = stats_->rs2Cuts + stats_->cuts - numOldCuts;
  delete [] grad;
  delete [] npt;
  return;
}


void Linearizations::rScheme2Cut_(FunctionPtr f, double UB, double &delta,
                                  double lVarCoeff, double &lastSlope,
                                  UInt nVarIdx, double * npt, double * grad,
                                  bool isObj)
{
  int error = 0;
  double newSlope, angle, tanTheta, PI = 3.14159265;
  
  f->evalGradient(npt, grad, &error);
  if (error == 0) {
    newSlope = -1*(grad[nVarIdx]/lVarCoeff);
    tanTheta = (newSlope-lastSlope)/(1+newSlope*lastSlope);
    angle = atan(tanTheta)*180/PI;
    //std::cout << " angle " << angle << "\n";

    // Add new linearization if angle between the two lines is at least rs2Per_
    if (fabs(angle) <  rs2Per_) {
      delta = 2*delta;
      return;
    }

    lastSlope = newSlope;
    //ConstraintPtr newcon;
    std::stringstream sstm;
    FunctionPtr newF;
    LinearFunctionPtr lf = 0;
    VariableConstIterator vbeg = rel_->varsBegin(), vend = rel_->varsEnd();
    const double linCoeffTol =
      env_->getOptions()->findDouble("conCoeff_tol")->getValue();
    double c, act = f->eval(npt, &error);

    if (error == 0) {
      lf = (LinearFunctionPtr) new LinearFunction(grad, vbeg, vend, linCoeffTol);
      c  = act - InnerProduct(npt, grad, minlp_->getNumVars());
      ++(stats_->cuts);
      sstm << "_OACutRoot_" << stats_->cuts;
      if (isObj) {
        lf->addTerm(objVar_, -1.0);
      }
      newF = (FunctionPtr) new Function(lf);
      rel_->newConstraint(newF, -INFINITY, UB-c, sstm.str());
      //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //newcon->write(std::cout);
    }
  }
  return;
}


bool Linearizations::shouldStop_(EngineStatus eStatus)
{
  bool shouldStop = true;
  switch (eStatus) {
   case (ProvenOptimal):
   case (ProvenLocalOptimal):
     shouldStop = false;
     break;
   case (FailedInfeas):
     logger_->msgStream(LogInfo) << me_ << "failed to converge "
     << "(infeasible) in root" << std::endl;
     break;
   case (ProvenFailedCQInfeas):
     logger_->msgStream(LogInfo) << me_ << "constraint qualification "
                                        << "violated in root "
                                        << std::endl;
     break;
   case (ProvenUnbounded):
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
     break;
   case (EngineError):
   case (ProvenInfeasible):
   case (ProvenLocalInfeasible):
   case (ProvenObjectiveCutOff):
   default:
     break;
  }
  return shouldStop;
}


//// ESH only at the boundary point
void Linearizations::rootLinScheme3(EnginePtr lpe, SeparationStatus *status)
{
  timer_->start();
  UInt numNl = nlCons_.size();
  if ((solC_ != 0 && numNl > 0) || oNl_) {
    const double *lpx;
    UInt numVars =  minlp_->getNumVars(); 
    UInt numOldCuts, initCuts = stats_->cuts;
 
    if (numNl > 0) { 
      bool vio, active, ptFound;
      std::vector<UInt > consToLin; // cons to add linearizations
      double* xnew = new double[numVars];
      for (UInt i = 1; i <= rs3_; ++i) {
        vio = false, active = false;
        lpx = lpe->getSolution()->getPrimal();
        candLinCons_(lpx, consToLin, active, vio);
        if (vio) {
          numOldCuts = stats_->cuts;
          ptFound = boundaryPtForCons_(xnew, lpx, consToLin);
          if (ptFound) {
            genLin_(xnew, consToLin);
          }
          if (numOldCuts < stats_->cuts) {
            lpe->solve();
            if (shouldStop_(lpe->getStatus())) {
              break;    
            }
          } else {
            break;
          }
        } else if (active) {
          genLin_(lpx, consToLin);
          break;
        } else {
          break;
        }
        consToLin.clear();
      }
      delete [] xnew;
    } else if (oNl_) {
      double dist;
      double *lpxPrev = new double[numVars];
      lpx = lpe->getSolution()->getPrimal();
      for (UInt i = 1; i <= rs3_; ++i) {
        numOldCuts = stats_->cuts;
        objCut_(lpx);
        if (numOldCuts < stats_->cuts) {
          std::copy(lpx, lpx+numVars, lpxPrev);
          lpe->solve();
          if (shouldStop_(lpe->getStatus())) {
            break;    
          } 
          lpx = lpe->getSolution()->getPrimal();
          dist = InnerProduct(lpx, lpxPrev, numVars);
          if (fabs(dist) <= solAbsTol_) {
            break;
          }
        } else {
          break;
        }
      }
      delete [] lpxPrev;
    }

    stats_->rs3Cuts = stats_->cuts - initCuts; 
    if (stats_->rs3Cuts > 0) {
      *status = SepaResolve;
    }
  
  }
  stats_->linSchemesTime = stats_->linSchemesTime + timer_->query();
  timer_->stop();
  return;
}


bool Linearizations::objCut_(const double* xNew)
{
  double c, act;
  int error = 0;
  FunctionPtr f;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  ObjectivePtr o = minlp_->getObjective();
  
  act = o->eval(xNew, &error);
  if (error == 0) {
    f = o->getFunction();
    linearAt_(f, act, xNew, &c, &lf, &error);
    if (error == 0) {
      ++(stats_->cuts);
      lf->addTerm(objVar_, -1.0);
      f = (FunctionPtr) new Function(lf);
      sstm << "_OACutRootObj_" << stats_->cuts;
      rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      return true;
    }   
  } else {
    logger_->msgStream(LogError) << me_
      <<"objective not defined at this point."<< std::endl;
  }
  return false;
}


bool Linearizations::uniVarNlFunc_(FunctionPtr f, double &lVarCoeff,
                                   UInt & lVarIdx, UInt & nVarIdx,
                                   double &nVarCoeff, bool isObj)
{
  double coeff;
  UInt nlTerms = 0, qTerms = 0, idx;
  bool foundVar = false, foundNVar = false;
  LinearFunctionPtr lf = f->getLinearFunction(); 
  QuadraticFunctionPtr qf = f->getQuadraticFunction();
  NonlinearFunctionPtr nlf = f->getNonlinearFunction();

  const double linCoeffTol =
    env_->getOptions()->findDouble("conCoeff_tol")->getValue();

  if (nlf) {
    nlTerms = nlf->numVars();
    if (nlTerms != 1) {
      return false;    
    }
    nVarIdx= (*(nlf->varsBegin()))->getIndex(); //index of var in nonlinear term
  }

  if (qf) {
    qTerms = qf->getNumVars();
    if (qTerms != 0) {
      if (qTerms > 1) {
        return false;    
      }
      if (nlTerms == 1) {
        if (nVarIdx != ((qf->varsBegin())->first)->getIndex()) {
          return false;      
        }
      } else {
        nVarIdx = ((qf->varsBegin())->first)->getIndex(); 
      }
    }
  }
  
  if (lf) {
    for(VariableGroupConstIterator vit = lf->termsBegin();
        vit != lf->termsEnd(); ++vit) {
      coeff = vit->second;
      idx = (vit->first)->getIndex();
      if (idx == nVarIdx) {
        foundNVar = true;
        nVarCoeff = coeff;
        continue;
      }
      if (fabs(coeff) > linCoeffTol && foundVar == false) {
        lVarIdx = idx;
        foundVar = true;
        lVarCoeff = coeff;
      }
      if (foundVar && foundNVar) {
        break;
      }
    }
  }
  
  if (foundVar) {
    return true;
  } else {
    if (isObj) {
      lVarCoeff = -1;
      lVarIdx = objVar_->getIndex();
      return true;
    }  
  }

  return false;
}


void Linearizations::writeStats(std::ostream &out) const
{
  out
    << me_ << "time taken in linearization schemes  = "
    << stats_->linSchemesTime << std::endl
    << me_ << "number of cuts in root scheme 1      = "
    << stats_->rs1Cuts << std::endl
    << me_ << "number of cuts in root scheme 2      = "
    << stats_->rs2Cuts << std::endl
    << me_ << "number of cuts in root scheme 3      = "
    << stats_->rs3Cuts << std::endl
    << me_ << "number of cuts in root gen. scheme 1 = "
    << stats_->rgs1Cuts << std::endl
    << me_ << "number of cuts in root gen. scheme 2 = "
    << stats_->rgs2Cuts << std::endl
    << me_ << "number of total cuts                 = "
    << stats_->cuts << std::endl;

  return;
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
