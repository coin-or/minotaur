// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

/** 
 * \file AnalyticalCenter.cpp
 * \Briefly define a class for finding analytical center of the feasible
 * region.
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

#include "AnalyticalCenter.h"
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
#include "ProblemSize.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "VarBoundMod.h"
#include "Variable.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string AnalyticalCenter::me_ = "Linearizations: ";

AnalyticalCenter::AnalyticalCenter(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe)
: env_(env),
  minlp_(minlp),
  nlpe_(nlpe)
{
  logger_ = env->getLogger();
  timer_ = env->getNewTimer();
 }


AnalyticalCenter::~AnalyticalCenter()
{ 
  env_ = 0;
  nlpe_ = 0;
  minlp_ = 0;
}

 
void AnalyticalCenter::modifyOnlyNonlinear(double * &solC)
{
  double lb, ub;
  FunctionPtr fnewc;
  ConstraintPtr con;
  FunctionType fType;
  VariablePtr vPtr, v;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone(env_);
  LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();  
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
 
  // Modify objective 
  vPtr = inst_C->newVariable(-INFINITY, 0, Continuous, "eta", VarHand);
  vPtr->setFunType_(Nonlinear);
  inst_C->removeObjective();
  lfc->addTerm(vPtr, 1.0);
  fnewc = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(fnewc, 0.0, Minimize);

  // Modify the nonlinear constraints and leave linear constraints unchanged
  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
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
  nlpe_->load(inst_C);
  solveNLP_(solC);
  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);
  
  if (nlpe_->getStatusString() == "ProvenUnbounded") {
    logger_->msgStream(LogDebug) << me_ 
      << " Problem for finding center is unbounded." <<
     " Solving a restricted problem." << std::endl;

    // Adding auxiliary variable to linear constraints and variable bounds
    for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
         ++it) {
      con = *it;
      lb = con->getLb();
      ub = con->getUb();
      fType = con->getFunctionType();
      if (fType == Linear)  {
        if (lb != -INFINITY && ub != INFINITY) {
          if (fabs(lb-ub) < solAbsTol_) {
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
    inst_C->delMarkedCons();

    for (VariableConstIterator vit = inst_C->varsBegin(); 
         vit != inst_C->varsEnd()-1; ++vit) {
      v = *vit;
      lb = v->getLb(), ub = v->getUb();
      if (fabs(lb-ub) < solAbsTol_) {
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
    solveNLP_(solC);
  
    if (solC) {
      if (nlpe_->getSolution()->getObjValue() > -solAbsTol_) {
        delete [] solC;
        solC = 0;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "NLP engine status for restricted center problem = "
        << nlpe_->getStatusString() << std::endl;
    }
  } else {
    if (solC) {
      if (nlpe_->getSolution()->getObjValue() > -solAbsTol_) {
        delete [] solC;
        solC = 0;
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


void AnalyticalCenter::modifyWhole(double * &solC)
{
  double lb, ub;
  FunctionPtr fnewc;
  ConstraintPtr con;
  FunctionType fType;
  //UInt hasEqCons = 0;
  VariablePtr vPtr, v;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone(env_);
  LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
 
  // Modify objective 
  vPtr = inst_C->newVariable(-INFINITY, 0, Continuous, "eta", VarHand);
  vPtr->setFunType_(Nonlinear);
  inst_C->removeObjective();
  lfc->addTerm(vPtr, 1.0);
  fnewc = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(fnewc, 0.0, Minimize);

  // consider all inequality constraints including variable bounds - to find center
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
        if (fabs(lb-ub) < solAbsTol_) {
          //hasEqCons = 1;
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
    if (fabs(lb-ub) < solAbsTol_) {
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
  solveNLP_(solC);
  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);
  //exit(1);

  if (solC) {
    if (nlpe_->getSolution()->getObjValue() > -solAbsTol_) {
      delete [] solC;
      solC = 0;
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


void AnalyticalCenter::solveNLP_(double *&solC)
{ 
  if (solC) {
    delete [] solC;
    solC = 0;
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
      solC = new double[numVars];
      std::copy(temp, temp+numVars, solC);
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
