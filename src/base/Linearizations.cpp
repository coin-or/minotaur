// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
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
                               VariablePtr objVar)
: env_(env),
  rel_(rel),
  minlp_(minlp),
  //lpe_(EnginePtr()),
  nlpe_(EnginePtr()),
  solC_(NULL),
  nlpx_(NULL),
  nbhSize_(20),
  varPtrs_(0),
  //isAllCons_(1),
  objVarVal_(INFINITY)
{
  nlCons_ = nlCons;
  logger_ = env->getLogger();
  //MS: set the option for root_LinSchemes
  rs1_ = env_->getOptions()->findDouble("root_linScheme1")->getValue();
  rs2Per_ = env_->getOptions()->findDouble("root_linScheme2")->getValue();
  rs2NbhSize_ = env_->getOptions()->findDouble("root_linScheme2_nbhSize")->getValue();
  rs3_ = env_->getOptions()->findInt("root_linScheme3")->getValue();
  rgs1_ = env_->getOptions()->findBool("root_genLinScheme1")->getValue();
  //rgs2_ = env_->getOptions()->findBool("root_genLinScheme2")->getValue();
  rsg2Per_ = env_->getOptions()->findDouble("root_linGenScheme2_per")->getValue();
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

//MS: delete unused functions
void Linearizations::cutAtLineSearchPt_(const double *xIn, const double *xOut,
                                        double* xNew, ConstraintPtr con,
                                        double lpObj)
{
  double nlpact;
  bool lsPtFound;

  lsPtFound = lineSearchPt_(xIn, xOut, xNew, con, nlpact);

  if (lsPtFound) {
    int error = 0;
    std::stringstream sstm;
    //ConstraintPtr newcon;
    LinearFunctionPtr lf = 0;
    FunctionPtr f = con->getFunction();
    double c, cUb = con->getUb();
    linearAt_(f, nlpact, xNew, &c, &lf, &error);
    if (error == 0) {
      ++(stats_->cuts);
      sstm << "_OACutRoot_" << stats_->cuts;
      f = (FunctionPtr) new Function(lf);
      rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
      if (oNl_) {
        objCut_(xNew, lpObj);
      }
    }
  }
  return;
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


//void Linearizations::objToCons_(ProblemPtr problem, VariablePtr vObj, VariablePtr newVar)
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


void Linearizations::findCenter()
{
  timer_->start();
  double lb, ub;
  FunctionPtr fnewc;
  ConstraintPtr con;
  FunctionType fType;
  VariablePtr vPtr, v;
  std::vector<ConstraintPtr > cp;
  ProblemPtr inst_C = minlp_->clone();
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
  //inst_C->write(std::cout);
  if (solC_) {
    if (fabs(nlpe_->getSolution()->getObjValue()) <= solAbsTol_) {
      double act;
      int error = 0;
      for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
        con = *it;
        act = con->getActivity(solC_, &error);
        if (error == 0) {
          ub = con->getUb();
          if ((fabs(act-ub) < solAbsTol_) ||
              (ub != 0 && fabs(act-ub) < fabs(ub)*solRelTol_)) {
            delete [] solC_;
            solC_ = 0;
            break;
          }
        }
      }
      if (solC_ == 0) {
        for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
             ++it) {
          con = *it;
          lb = con->getLb();
          ub = con->getUb();
          if (con->getFunctionType() == Linear)  {
            if (lb != -INFINITY && ub != INFINITY) {
              if (fabs(lb-ub) <= solAbsTol_) {
                //isAllCons_ = 0;
                inst_C->markDelete(con);
              }
            }
          }
        }
  
        inst_C->delMarkedCons();
        inst_C->prepareForSolve();
        nlpe_->clear();
        nlpe_->load(inst_C);
        solveNLP_();
        if (solC_ && fabs(nlpe_->getSolution()->getObjValue()) <= solAbsTol_) {
          delete [] solC_;
          solC_ = 0;
        }
      }
    }
  }

  delete nlpe_;
  nlpe_ = 0;
  delete inst_C;
  inst_C = 0;
  stats_->linSchemesTime = stats_->linSchemesTime + timer_->query();
  timer_->stop();
  return;
}


void Linearizations::solveNLP_()
{ 
  EngineStatus nlpStatus = nlpe_->solve();

  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    if (solC_) {
      delete [] solC_;
      solC_ = 0;
    }
    
    {
      //std::cout << "Center " << std::setprecision(6) << nlpe_->getSolution()->getObjValue() << "\n";
      //exit(1);
      UInt numVars = minlp_->getNumVars();
      const double *temp = nlpe_->getSolution()->getPrimal();
      solC_ = new double[numVars];
      std::copy(temp, temp+numVars, solC_);
    }
    break;
  case (EngineIterationLimit):
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
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
                                  std::vector<ConstraintPtr > &consToLin,
                                  bool &foundActive, bool &foundVio)
{
  int error = 0;
  ConstraintPtr c;
  double act, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
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
        consToLin.push_back(c);
      } else if ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_)) {
        if (!foundVio) {
          if (!foundActive) {
            foundActive = true;         
          }
          consToLin.push_back(c);
        }
      }
    }
  }
  return;
}


std::vector<UInt > Linearizations::isFeas_(double *x, 
                                           std::vector<UInt > varConsPos,
                                           bool &foundActive, bool &foundVio)
{
  UInt j;
  int error = 0;
  ConstraintPtr c;
  double act, cUb;
  std::vector<UInt > consPos;

  for (UInt i = 0; i < varConsPos.size(); ++i) {
    j = varConsPos[i];
    c = nlCons_[j];
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if ((act > cUb + solAbsTol_) &&
          (cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
        if (!foundVio) {
          foundVio = true;
          if (consPos.size() != 0) {
            consPos.clear();          
          }         
        }
        consPos.push_back(j);
      } else if ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_)) {
        if (!foundVio) {
          if (!foundActive) {
            foundActive = true;         
            if (consPos.size() != 0) {
              consPos.clear();          
            }         
          }
          consPos.push_back(j);
        }
      } 
    }
  }
  return consPos;
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

// It is ensured that xOut does not satisfy con and a boundary point between
// xIn and xOut is to be found
bool Linearizations::lineSearchPt_(const double *xIn, const double *xOut,
                                   double* x, ConstraintPtr con, double &nlpact)
{
  double *xl, *xu;
  bool ptFound = true;
  int error = 0, repPt = 0;
  FunctionPtr f = con->getFunction();
  UInt numVars =  minlp_->getNumVars();
  double cUb = con->getUb(), repSolOld = INFINITY, lambdaIn = 0.5,
         lambdaOut = 0.5;

  xl = new double[numVars];
  xu = new double[numVars];
  std::copy(xIn,xIn+numVars,xl);
  std::copy(xOut,xOut+numVars,xu);
  
  while (true) {
    for (UInt i = 0 ; i < numVars; ++i) {
      x[i] = lambdaIn*xl[i] + lambdaOut*xu[i];
    }
    nlpact = f->eval(x, &error);
    if (error == 0) {
      if (nlpact > cUb + solAbsTol_ && (cUb==0 ||
                                      nlpact > cUb + fabs(cUb)*solRelTol_)) {
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
      ptFound = false;
      break;
    }
    if (nlpact == repSolOld) {
      ++repPt;
    } else {
      repPt = 0;
      repSolOld = nlpact;
    }

    if (repPt == 10) {
      break;
    }
  }

  delete [] xl;
  delete [] xu;
  return ptFound;
}
 

void Linearizations::rootLinearizations(ConstSolutionPtr sol)
{
  timer_->start();
  
  FunctionPtr f; 
  ConstraintPtr con;
  bool isFound = false;
  UInt nVarIdx, lVarIdx;
  double lVarCoeff = 0, nVarCoeff = 0, ub;
   
  nlpx_ = sol->getPrimal();
 
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
  /// General scheme at root
  // Option for general scheme
  if (solC_ && (rgs1_ || rsg2Per_)) {
    // populate varPtrs_ with index of variables in nonlinear constraints
    // and objective
    varsInNonlinCons_();
    // General scheme using center and  positive spanning vectors
    if (nlCons_.size() > 0 && rgs1_) {
      rootLinGenScheme1(-INFINITY);
    }

    // General scheme using nonlinear solution and positive spanning vectors
    // Similar conditions as in rgs1
    if (rsg2Per_) {
      rootLinGenScheme2_();
    }
  }
  stats_->linSchemesTime = stats_->linSchemesTime + timer_->query();
  timer_->stop();
  return;
}


void Linearizations::rootLinGenScheme2_()
{
  VariablePtr v;
  FunctionPtr f;
  ConstraintPtr con;
  //bool isFound = false;
  std::vector<UInt > varIdx;
  UInt fixIdx, vIdx;
  int error, firstnnz= -1;
  std::vector<double* > nlconsGrad;
  std::vector<UInt > varConsPos;
  std::vector<UInt > varPos;
  std::vector<double > unitVec;
  double val, rhs = 0.0, fixCoeff, coeff;
  int n = minlp_->getNumVars(), nr = rel_->getNumVars();
  
  double *xOut = new double[n]; 
  std::copy(nlpx_, nlpx_+n, xOut);
    
  // rhs of the plane passing through nlpx_ and has solC-nlpx_ as normal;
  // fixIdx is the first variable in varPtrs_ with nonzero coeff on this plane
  for (UInt i = 0; i < varPtrs_.size(); ++i) {
    v = varPtrs_[i];
    vIdx = v->getIndex();
    rhs = rhs + nlpx_[vIdx]*(solC_[vIdx] - nlpx_[vIdx]);
    if ((firstnnz == -1) && ((solC_[vIdx] - nlpx_[vIdx]) != 0)) {
      firstnnz = i;
      fixIdx = vIdx;
    }
  }

  if (firstnnz == -1 || rhs == 0) {
    return;  
  }
   
 //// Gradient of constraints at nonlinear solution nlpx_
  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    error = 0;
    f = con->getFunction();
    
    double *grad = new double[nr];
    std::fill(grad, grad+nr, 0);
    
    f->evalGradient(nlpx_, grad, &error);
    
    if (error == 0) {
      nlconsGrad.push_back(grad);
    } else {
      nlconsGrad.push_back(0);
    }
  }

  // coefficient of the fix var in any direction
  fixCoeff = rhs/(solC_[fixIdx] - nlpx_[fixIdx]);
     
  // linearly independent directions for each var
  for (UInt i = 0; i < varPtrs_.size(); ++i) {
    v = varPtrs_[i];
    vIdx = v->getIndex();
    if ((int(i) < firstnnz) || ((solC_[vIdx] - nlpx_[vIdx]) == 0)) {
      // Coeff of var is zero in the hyperplane expression
      // Nonlinear constraints containing var v
      varPos.push_back(i);
      varCons_(varPos, varConsPos);
      varPos.clear();
      
      if (varConsPos.size() == 0) {
        continue;      
      }
      
      unitVec.push_back(1);
      varIdx.push_back(vIdx);
      exploreDir_(xOut, varIdx, varConsPos, nlconsGrad, unitVec);
      
      //// reverse the direction
      unitVec[0] = -1;
      exploreDir_(xOut, varIdx, varConsPos, nlconsGrad, unitVec);
    } else if (int(i) > firstnnz) {
      // in this case coeff of v could be nonzero in the hyperplane expression 
      // list of constraints containing var v
      varPos.push_back(i);
      varPos.push_back(firstnnz);
      varCons_(varPos, varConsPos);
      varPos.clear();
      
      if (varConsPos.size() == 0) {
        continue;      
      }
      
      varIdx.push_back(vIdx);
      varIdx.push_back(fixIdx);
      coeff = rhs/(solC_[vIdx] - nlpx_[vIdx]);
      val = sqrt(pow(fixCoeff, 2) + pow(coeff, 2));   
      unitVec.push_back(coeff/val);
      unitVec.push_back(-fixCoeff/val);
      exploreDir_(xOut, varIdx, varConsPos, nlconsGrad, unitVec);
      
      //// reverse the direction
      unitVec[0] = -coeff/val;
      unitVec[1] = fixCoeff/val;
      exploreDir_(xOut, varIdx, varConsPos, nlconsGrad, unitVec);
    } else {
      continue;    
    }
    varIdx.clear();
    unitVec.clear();
    varConsPos.clear();
  }

  for (UInt i = 0; i < nlCons_.size(); ++i) {
    if (nlconsGrad[i]) {
      delete [] nlconsGrad[i];
      nlconsGrad[i] = 0;
    }
  }
  delete [] xOut;
  return;
}

void Linearizations::exploreDir_(double *xOut, std::vector<UInt > varIdx,
                                 std::vector<UInt > varConsPos, 
                                std::vector<double *> nlconsGrad,
                                std::vector<double > unitVec) //MS: later see if vbnd has to be changed or not.
{
  UInt idx;
  double delta = 0.2;
  bool isFound = false;
  int nr = rel_->getNumVars();
  std::vector<double* > lastGrad;
  lastGrad.resize(nlCons_.size(), 0); //MS: Can be made efficient.
 
  //else if (val > nbhSize) {
    //vbnd = xOut[idx] + alpha[0]*nbhSize;
  //}

  for (UInt i = 0; i < varIdx.size(); ++i) {
    idx = varIdx[i];
    xOut[idx] = xOut[idx] + delta*unitVec[i];
  }
  
  for (UInt i = 0; i < varConsPos.size(); ++i) {
    idx = varConsPos[i];
    if (nlconsGrad[idx]) {
      double * grad = new double[nr];
      std::copy(nlconsGrad[idx], nlconsGrad[idx]+nr, grad);
      lastGrad[idx] = grad;
    }
  }

  while (delta <= nbhSize_) {
    isFound = findLinPoint_(xOut, varConsPos, lastGrad);
    // double the stepsize delta if linearization at this point is not useful
    if (!isFound) {
      delta = 2*delta;
    }
    for (UInt i = 0; i < varIdx.size(); ++i) {
      idx = varIdx[i];
      xOut[idx] = xOut[idx] + delta*unitVec[i];
    }
  }

  for (UInt i = 0; i < varConsPos.size(); ++i) {
    idx = varConsPos[i];   
    if (lastGrad[idx]) {
      delete [] lastGrad[idx];
      lastGrad[idx] = 0;
    }
  }

  for (UInt i = 0; i < varIdx.size(); ++i) {
    idx = varIdx[i];
    xOut[idx] = nlpx_[idx];
  }
  return;
} 

void Linearizations::findLinPoint_(double *xOut)
{
  //bool foundActive = false, foundVio = false;
  //std::vector<ConstraintPtr> vConsPos = isFeas_(xOut, foundActive, foundVio);
        
  //if (foundVio) {
    //// point outside feasible region. Do line search
    //findBoundaryPt_(xOut, vConsPos);
  //} else if (foundActive) {
    ////point on the feasible region. Directly add linearizations
    //genLin_(xOut, vConsPos);
  //} 
  return;
}


bool Linearizations::findLinPoint_(double *xOut, 
                                   std::vector<UInt > varConsPos,
                                   std::vector<double* > & lastGrad) 
{
  double lambda = 0.5;
  bool foundActive = false, foundVio = false, cutAdded = false;
  std::vector<UInt> vConsPos = isFeas_(xOut, varConsPos, foundActive, foundVio);
        
  if (foundVio) {
    // point outside feasible region. Do line search
    cutAdded = boundaryPt_(xOut, vConsPos, lastGrad);
  } else if (foundActive) {
    //point on the feasible region. Directly add linearizations
    cutAdded = genLin_(xOut, vConsPos, lastGrad);
  } else {
    double bnd;
    VariablePtr v;
    bool isCont = true;
    UInt n =  minlp_->getNumVars(), vIdx;
    
    double* x = new double[n];
    std::copy(solC_, solC_+n, x);
    
    // point inside feasible region. Move along xOut - x^C from x^C
    while (isCont) {
      for (UInt i = 0 ; i < n; ++i) {
        x[i] = x[i] + lambda*(xOut[i] - solC_[i]);
      }
  
      foundActive = false, foundVio = false;
      vConsPos = isFeas_(x, varConsPos, foundActive, foundVio);
      if (foundVio) {
        // point outside feasible region. Do line search
        cutAdded = boundaryPt_(x, vConsPos, lastGrad);
        break;
      } else if (foundActive) {
        //point on the feasible region. Directly add linearizations
        cutAdded = genLin_(x, vConsPos, lastGrad);
        break;
      }
      vConsPos.clear();

      for (UInt i = 0; i < varPtrs_.size(); ++i) {
        v = varPtrs_[i];
        vIdx = v->getIndex();
        bnd = v->getLb();
        if ((x[vIdx] < bnd - solAbsTol_) || 
            (bnd!=0 && (x[vIdx] < bnd - bnd*solRelTol_))) {
          isCont = false;
          break;        
        } 
        bnd = v->getUb();
        if ((x[vIdx] > bnd + solAbsTol_) || 
            (bnd!=0 && (x[vIdx] > bnd + bnd*solRelTol_))) {
          isCont = false;
          break;        
        }
      }
    }
    delete [] x;
  }
  return cutAdded;
}


void Linearizations::varCons_(std::vector<UInt > varPos,
                              std::vector<UInt > & varConsPos)
{
  VariablePtr v;
  ConstraintPtr con;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;

  for (UInt j = 0; j < nlCons_.size(); ++j) {
    con = nlCons_[j];
    qf = con->getQuadraticFunction();
    nlf = con->getNonlinearFunction();
    for (UInt i = 0; i < varPos.size(); ++i) {
      v = varPtrs_[i];
      if (nlf) {
        if (nlf->hasVar(v)) {
          varConsPos.push_back(j);
          continue;
        }
      }
      if (qf) {
        if (qf->hasVar(v)) {
          varConsPos.push_back(j);
        }
      }
    }
  }
  return;
}


void Linearizations::search_(double varbound, UInt vIdx, double val,  
                             std::vector<UInt > varIdx, double *xOut,
                             std::vector<double > &alphaSign, UInt pos,
                             bool isAllOne)
{
  double alpha;
  //std::cout << "pos alphaSign" << pos << " " << alphaSign.size() << std::endl;
  setStepSize_(varbound, alpha, vIdx, val, alphaSign[pos]);
  //std::cout <<"Stepsize step: varUb, alpha, val " << varbound << " " << alpha << " " << val << "\n";
  
  if (alpha == 0) {
    return;  
  }

  if (isAllOne) {
    UInt idx;
    for (UInt i = 0; i < varPtrs_.size(); ++i) {
      idx = varPtrs_[i]->getIndex();
      alphaSign[i] = alpha*alphaSign[i];  
      xOut[idx] = xOut[idx] + alphaSign[i];
    } 
  } else {
    for (UInt i = 0; i < alphaSign.size(); ++i) {
      alphaSign[i] = alpha*alphaSign[i];  
      xOut[varIdx[i]] = xOut[varIdx[i]] + alphaSign[i];
    }
  }
  foundLinPt_(vIdx, varIdx, pos, alphaSign, varbound, xOut, isAllOne);
  return;
}

// determine which out of varPtrs_ is bounding 
void Linearizations::boundingVar_(double &varbound, 
                                  UInt &pos, 
                                  double *lastDir,
                                  std::vector<double > &alphaSign)
{
  VariablePtr v;
  UInt idx;
  double diff, minDiff = INFINITY, bound;
    
  for (UInt i = 0; i < varPtrs_.size(); ++i) {
    v = varPtrs_[i];
    idx = v->getIndex();
    if (lastDir[i] < 0) {
      bound = v->getLb();
      diff = nlpx_[idx] - bound;
      alphaSign[i] = -1;
    } else if (lastDir[i] > 0) {
      bound = v->getUb();
      diff = bound - nlpx_[idx];
      alphaSign[i] = 1;
    } else {
      alphaSign[i] = 0;
      continue;    
    }
    if (diff < minDiff) {
      pos = i;
      varbound = bound;    
    }
  }
  return;
}

void Linearizations::boundingVar_(double &vbnd,
                                  UInt i, VariablePtr fixVar, double coeff,
                                 double fixCoeff, 
                                 int &alpha, 
                                 std::vector<UInt > &varIdx)
{
  VariablePtr v = varPtrs_[i];
  double diffCurrent, diffFix, val1, val2;
  UInt vIdx = v->getIndex(), fixIdx = fixVar->getIndex();

  if (coeff < 0) {
    val1 = v->getLb();
    if (val1 == -INFINITY) {
      diffCurrent = nbhSize_;
      val1 = nlpx_[vIdx] - nbhSize_;
    } else {
      diffCurrent = nlpx_[vIdx] - val1;
    }
    if (fixCoeff < 0) {
      val2 = fixVar->getLb();
      if (val2 == -INFINITY) {
        diffFix = nbhSize_;
        val2 = nlpx_[fixIdx] - nbhSize_;
      } else {
        diffFix = nlpx_[fixIdx] - val2;
      }
      alpha = -1;
      if (diffCurrent <= diffFix) {
        vbnd = val1;
        varIdx.push_back(vIdx);
        varIdx.push_back(fixIdx);
      } else {
        vbnd = val2;
        varIdx.push_back(fixIdx);
        varIdx.push_back(vIdx);
      }
    } else {
      val2 = fixVar->getUb();
      if (val2 == INFINITY) {
        diffFix = nbhSize_;
        val2 = nbhSize_+ nlpx_[fixIdx];
      } else {
        diffFix = val2 - nlpx_[fixIdx];
      }
      if (diffCurrent <= diffFix) {
        alpha = -1;
        vbnd = val1;
        varIdx.push_back(vIdx); 
        varIdx.push_back(fixIdx);
      } else {
        alpha = 1;
        vbnd = val2;
        varIdx.push_back(fixIdx);
        varIdx.push_back(vIdx);
      }
    }
  } else {
    val1 = v->getUb();
    if (val1 == INFINITY) {
      diffCurrent = nbhSize_;
      val1 = nbhSize_+ nlpx_[vIdx];
    } else {
      diffCurrent = val1 - nlpx_[vIdx];
    }
    if (fixCoeff < 0) {
      val2 = fixVar->getLb();
      if (val2 == -INFINITY) {
        diffFix = nbhSize_;
        val2 = nlpx_[fixIdx] - nbhSize_;
      } else {
        diffFix = nlpx_[fixIdx] - val2;
      }
      if (diffCurrent <= diffFix) {
        alpha = 1;
        vbnd = val1;
        varIdx.push_back(vIdx);
        varIdx.push_back(fixIdx);
      } else {
        alpha = -1;
        vbnd = val2;
        varIdx.push_back(fixIdx);
        varIdx.push_back(vIdx);
      }
    } else {
      alpha = 1;
      val2 = fixVar->getUb();
      if (val2 == INFINITY) {
        diffFix = nbhSize_;
        val2 = nlpx_[fixIdx] + nbhSize_;
      } else {
        diffFix = val2 - nlpx_[fixIdx];
      }
      if (diffCurrent <= diffFix) {
        vbnd = val1;
        varIdx.push_back(vIdx);
        varIdx.push_back(fixIdx);
      } else {
        vbnd = val2;
        varIdx.push_back(fixIdx);
        varIdx.push_back(vIdx);
      }
    } 
  }
  return;
}
// determine which out of vIdx and fixIdx is bounding 
//void Linearizations::boundingVar_(double &varbound,
                                  //UInt vpos, VariablePtr fixVar, double coeff,
                                 //double fixCoeff, 
                                 //std::vector<double > &alphaSign, 
                                 //std::vector<UInt > &varIdx)
//{
  //VariablePtr v = varPtrs_[vpos];
  //double diffCurrent, diffFix, val1, val2;
  //UInt vIdx = v->getIndex(), fixIdx = fixVar->getIndex();

  //if (coeff < 0) {
    //val1 = v->getLb();
    //diffCurrent = nlpx_[vIdx] - val1;
    //if (fixCoeff < 0) {
      //val2 = fixVar->getLb();
      //alphaSign.push_back(-1); alphaSign.push_back(-1); // one for each
      //diffFix = nlpx_[fixIdx] - val2;
      //if (diffCurrent < diffFix) {
        //varbound = val1;
        //varIdx.push_back(vIdx);
        //varIdx.push_back(fixIdx);
      //} else {
        //varbound = val2;
        //varIdx.push_back(fixIdx);
        //varIdx.push_back(vIdx);
      //}
    //} else {
      //val2 = fixVar->getUb();
      //diffFix = val2 - nlpx_[fixIdx];
      //if (diffCurrent < diffFix) {
        //varbound = val1;
        //varIdx.push_back(vIdx); alphaSign.push_back(-1);
        //varIdx.push_back(fixIdx); alphaSign.push_back(1);
      //} else {
        //varbound = val2;
        //varIdx.push_back(fixIdx); alphaSign.push_back(1);
        //varIdx.push_back(vIdx); alphaSign.push_back(-1);
      //}
    //}
  //} else {
    //val1 = v->getUb();
    //diffCurrent = val1 - nlpx_[vIdx];
    //if (fixCoeff < 0) {
      //val2 = fixVar->getLb();
      //diffFix = nlpx_[fixIdx] - val2;
      //if (diffCurrent < diffFix) {
        //varbound = val1;
        //varIdx.push_back(vIdx); alphaSign.push_back(1);
        //varIdx.push_back(fixIdx); alphaSign.push_back(-1);
      //} else {
        //varbound = val2;
        //varIdx.push_back(fixIdx); alphaSign.push_back(-1);
        //varIdx.push_back(vIdx); alphaSign.push_back(1);
      //}
    //} else {
      //val2 = fixVar->getUb();
      //diffFix = val2 - nlpx_[fixIdx];
      //alphaSign.push_back(1); alphaSign.push_back(1); // one for each
      //if (diffCurrent < diffFix) {
        //varbound = val1;
        //varIdx.push_back(vIdx);
        //varIdx.push_back(fixIdx);
      //} else {
        //varbound = val2;
        //varIdx.push_back(fixIdx);
        //varIdx.push_back(vIdx);
      //}
    //} 
  //}
  //return;
//}


bool Linearizations::rootLinGenScheme1(double lpObj)
{
  if (varPtrs_.size() > 0 && (nlCons_.size() > 0 || oNl_)) {
    double val;   
    VariablePtr v;
    UInt vIdx, numOldCuts = stats_->cuts, n = minlp_->getNumVars();
    double bound, vLb = INFINITY, vUb = INFINITY; // for last direction 
    
    double *xOut = new double[n];
    std::copy(solC_, solC_ + n, xOut);
    
    // coordinate direction along each variable in varPtrs_.
    for (UInt i = 0; i < varPtrs_.size(); ++i) {
      v = varPtrs_[i];
      vIdx = v->getIndex();
      // determine indices of variables that are bounding in the last
      // direction (and its opposite) of search and bound values 
      val = v->getUb();
      
      //// for last direction
      bound = val - solC_[vIdx];
      if (bound < vUb) {
        vUb = bound;
      }

      // coordinate direction for each variable 
      if (val == INFINITY) {
        xOut[vIdx] = xOut[vIdx] + 50;       // if variable is unbounded above
      } else {
        xOut[vIdx] = val;
      }
      cutsAtBoundary_(xOut, lpObj, vIdx);
      
      /// Reverse search direction if previous direction was unsuccessful 
      val = v->getLb();
      //// for last direction
      bound = solC_[vIdx] - val;
      if (bound < vLb) {
        vLb = bound;
      }

      if (val == -INFINITY) {
        xOut[vIdx] = xOut[vIdx] - 50;
      } else {
        xOut[vIdx] = val;
      }
      cutsAtBoundary_(xOut, lpObj, vIdx);
      xOut[vIdx] = solC_[vIdx];
    }
    
    //// Last direction in positive spanning set
    if (vUb == INFINITY) {
      vUb = 50;
    }
    for (UInt i = 0 ; i < n; ++i) {
      xOut[i] = xOut[i] + vUb;
    }
    cutsAtBoundary_(xOut, lpObj, -1);

    //// Reverse direction 
    if (vLb == INFINITY) {
      vLb = 50;
    }
    for (UInt i = 0 ; i < n; ++i) {
      xOut[i] = xOut[i] - vLb;
    }
    cutsAtBoundary_(xOut, lpObj, -1);

    delete [] xOut;
    
    stats_->rgs1Cuts = stats_->cuts - numOldCuts; 
    if (stats_->rgs1Cuts > 0) {
      return true;
    }
  }
  return false;
}


void Linearizations::candConsForObj_(double *xOut,
                                     std::vector<ConstraintPtr > &consToLin)
{
  ConstraintPtr con;
  FunctionType fType;
  double lb, ub, act;
  LinearFunctionPtr lf;
  bool active = false, vio = false;
  for (ConstraintConstIterator it=minlp_->consBegin(); it!=minlp_->consEnd();
       ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
    if (fType == Linear) {
      lf = con->getLinearFunction();
      act = lf->eval(xOut);
      if (fabs(lb-ub) <= solAbsTol_) {
        continue;
        //if ((fabs(act-ub) > solAbsTol_) &&
            //(ub == 0 || fabs(act-ub) > fabs(ub)*solRelTol_)) {
          //if (!vio) {
            //vio = true;
            //if (consToLin.size() != 0) {
              //consToLin.clear();          
            //}         
          //}
          //consToLin.push_back(con);
        //} else {
          //if (!vio) {
            //if (!active) {
              //active = true;         
            //}
            //consToLin.push_back(con);
          //}
        //}
      } else {
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
      }
    }  
  }

  return;
}


void Linearizations::cutsAtBoundary_(double *xOut, double lpObj, int vIdx)
{
  bool active, vio, ptFound;
  std::vector<ConstraintPtr > consToLin; // cons to add linearizations
  double* xnew = new double[minlp_->getNumVars()];
  
  if (nlCons_.size() > 0) {   
    active = false, vio = false;
    candLinCons_(xOut, consToLin, active, vio);
    if (vio) {
      ptFound = findBoundaryPt_(xnew, xOut, consToLin, vIdx);
      if (ptFound) {
        // add linearization to active nonlinear constraints only if the
        // linearizations are far apart
        genLin_(xnew, consToLin);
        if (oNl_) {
          objCut_(xnew, -INFINITY);
        }
      }
    } else if (active) {
      genLin_(xOut, consToLin);
      if (oNl_) {
        objCut_(xOut, -INFINITY);
      }
    }
  } else if (oNl_) {
    candConsForObj_(xOut, consToLin);
    ptFound = boundaryPtForObj_(xnew, xOut, consToLin, vIdx);
    if (ptFound) {
      objCut_(xnew, lpObj);
    }
  }
  delete [] xnew;
  return;
}

void Linearizations::setStepSize_(double &varbound, double &alpha,
                                   UInt vIdx, double val, double boundSign)
{ 
  if (varbound != boundSign*INFINITY) {
    alpha = fabs(varbound-val);
  } else {
    if (fabs(nlpx_[vIdx] - solC_[vIdx]) != 0) {
      alpha = fabs(nlpx_[vIdx] - solC_[vIdx]);
    } else {
      alpha = fabs(val) + 4;
    }
    varbound = val + boundSign*(fabs(10*val) + 10); // parameter here
  }
  alpha = 0.25*alpha;
  return;
}
          

void Linearizations::varsInNonlinCons_()
{
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
  }
  //exit(1);
  return;
}


void Linearizations::foundLinPt_(UInt vIdx, std::vector<UInt> varIdx, 
                                 UInt pos,
                                 std::vector<double> alphaSign, double varbound,
                                 double *xOut, bool isAllOne)
{
  //int aSign = 1, error = 0;
  //double act, cUb;
  //ConstraintPtr con;
  //std::vector<ConstraintPtr > vioCons;
  
  /* find constraints violated at xOut. If no constraint is violated then
   * move further along the given direction, If all linear constraints are
   * violated then stop and return. 
   */
  //if (alphaSign[pos] < 0) {
    //aSign = -1;
  //}
  //while (true) {
    //for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
      //con = *it;
      //cUb = con->getUb();
      //act = con->getActivity(xOut, &error);
      //if (error == 0) {
        //if ((act > cUb + solAbsTol_) &&
            //(cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) { // violated cons
          //vioCons.push_back(con);
        //}
      //}   
    //}
    //if (vioCons.size() == 0) {
      //newPoint_(isAllOne, varIdx, xOut, alphaSign);  
      ////if (alphaSign[pos]*(xOut[vIdx]-varbound) > 0) 
      //if (aSign*(xOut[vIdx]-varbound) > 0) {
        //return;
      //}
    //} else {
      //break;    
    //}
  //}
  
  //[> find point on boundary along the direction and add cuts<]
  //findBoundaryPt_(xOut, vioCons);
  //return;
}

void Linearizations::newPoint_(bool isAllOne,
                               std::vector<UInt> varIdx, double *xOut,
                               std::vector<double> alphaSign)
{
  UInt idx;
  if (isAllOne) {
    for (UInt i = 0; i < varPtrs_.size(); ++i) {
      idx = varPtrs_[i]->getIndex();
      xOut[idx] = xOut[idx] + alphaSign[i];
    }
  } else {
    for (UInt i = 0; i < varIdx.size(); ++i) {
      idx  = varIdx[i];
      xOut[idx] = xOut[idx] + alphaSign[i];
    }
  }
  return;
}

bool Linearizations::boundaryPt_(const double *x,
                                 std::vector<UInt > &vioConsPos,
                                 std::vector<double* > &lastGrad)
{
  ConstraintPtr con;
  int error = 0, repPt = 0;
  UInt numVars =  minlp_->getNumVars();
 
  double* xl = new double[numVars];
  double* xu = new double[numVars];
  double* xnew = new double[numVars];
  bool firstVio, firstActive, cutAdded = false;
  double act, cUb, repSol, repSolOld = INFINITY, lambda1 = 0.5, lambda2 = 0.5;

  std::copy(x, x+numVars, xu);
  std::copy(solC_, solC_+numVars, xl);

  while (true) {
    for (UInt i = 0 ; i < numVars; ++i) {
      xnew[i] = lambda1*xl[i] + lambda2*xu[i];
    }
    repSol = 0;
    firstVio = false, firstActive = false;

    for (UInt k = 0; k < vioConsPos.size(); ) {
      con = nlCons_[vioConsPos[k]];
      act = con->getActivity(xnew, &error);
      if (error != 0) {
        delete [] xnew;
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
            vioConsPos.erase(vioConsPos.begin(), vioConsPos.begin() + k);
            k = 0;
          }
        }
        ++k;
      } else if ((fabs(act-cUb) <= solAbsTol_) ||
            (cUb != 0 && fabs(act- cUb) <= fabs(cUb)*solRelTol_)) { // active
        if (firstVio) {
          vioConsPos.erase(vioConsPos.begin() + k);
        } else {
          if (!firstActive) {
            firstActive = true;         
            if (k != 0) {
              vioConsPos.erase(vioConsPos.begin(),vioConsPos.begin() + k);
              k = 0;
            }
          }
          ++k;
        }
      } else {
        if (firstVio || firstActive) {
           vioConsPos.erase(vioConsPos.begin() + k);
        } else {
           ++k;
        }   
      }
    }
    //MS: Implement this login in Scheme 3 as well
    if (repSol == repSolOld) {
      ++repPt;    
    } else {
      repPt = 0;
      repSolOld = repSol;    
    }

    if (repPt == 5) {
      firstVio = false;
      firstActive = true;    
    }

    if (!firstVio) {
      if (!firstActive) {
        std::copy(xnew,xnew+numVars,xl);
      } else {
        // add linearization to active nonlinear constraints only if the
        // linearizations are far apart
        cutAdded = genLin_(xnew, vioConsPos, lastGrad);
        break;
      }
    } else {
      std::copy(xnew,xnew+numVars,xu);
    } 
  }
  
  delete [] xnew;
  delete [] xl;
  delete [] xu;
  return cutAdded;
}

bool Linearizations::genLin_(double *x, std::vector<UInt > vioConsPos,
                                     std::vector<double *> &lastGrad)
{
  UInt cIdx;
  FunctionPtr f;
  ConstraintPtr con;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  bool isCont, isFound = false;
  int error, n = rel_->getNumVars();
  double angle, PI = 3.14159265, d, c, m1, m2, cUb, act;
  VariableConstIterator vbeg = rel_->varsBegin(), vend = rel_->varsEnd();
  const double linCoeffTol =
    env_->getOptions()->findDouble("conCoeff_tol")->getValue();

  for (UInt j = 0; j < vioConsPos.size(); ++j) {
    cIdx = vioConsPos[j];
    angle = 0;
    error = 0;
    isCont = false;
    double *a = new double[n];
    std::fill(a, a+n, 0.);
    
    con = nlCons_[cIdx];
    f = con->getFunction();
    f->evalGradient(x, a, &error);
    
    if (error == 0) {
      if (lastGrad[cIdx] == NULL) {
        isCont = true;
      } else {
        // compute angle
        d = InnerProduct(a, lastGrad[cIdx], n);
        m1 = sqrt(InnerProduct(a, a, n));
        m2 = sqrt(InnerProduct(lastGrad[cIdx], lastGrad[cIdx], n));
        angle  = d/(m1*m2); // MS: make sure division by 0 does not appear.
        angle  = acos(angle)*180/PI;
      }
    } else {
      delete [] a;
      a = 0;
      continue;          
    }
       
    //std::cout << "angle " << angle << "\n"; 
    if (fabs(angle) >= rsg2Per_ || isCont) {
      cUb = con->getUb();
      act = con->getActivity(x, &error);
      if (error == 0) {
        lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol);
        c  = act - InnerProduct(x, a, minlp_->getNumVars());
        ++(stats_->rgs2Cuts);
        sstm << "_OACutRoot_" << stats_->rgs2Cuts;
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
        if (lastGrad[cIdx]) {
          delete [] lastGrad[cIdx];
          lastGrad[cIdx] = 0;
        }
        lastGrad[cIdx] = a;
        isFound = true;
      } else {
        delete [] a;      
        a = 0;
      }
    } else {
      delete [] a;
      a = 0;
    }
  }
  return isFound;
}

void Linearizations::genLin_(const double *x,
                             std::vector<ConstraintPtr > vioCons)
{
  int error;
  FunctionPtr f;
  ConstraintPtr con;
  double c, cUb, act;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;

  for (CCIter it = vioCons.begin(); it!=vioCons.end(); ++it) {
    error = 0;
    con = *it;
    act = con->getActivity(x, &error);
    if (error == 0) {
      f = con->getFunction();
      linearAt_(f, act, x, &c, &lf, &error);
      if (error == 0) {
        cUb = con->getUb();
        ++(stats_->cuts);
        sstm << "_OACutRoot_" << stats_->cuts;
        f = (FunctionPtr) new Function(lf);
        rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        sstm.str("");
      } 
    }
  }
  return;
}


bool Linearizations::boundaryPtForObj_(double* xnew, const double *xOut,
                                     std::vector<ConstraintPtr> &vioCons,
                                     int vIdx)
{ 
  ConstraintPtr con;
  int error = 0, repPt = 0;
  bool firstVio, firstActive;
  UInt numVars =  minlp_->getNumVars();

  double* xl = new double[numVars];
  double* xu = new double[numVars];
  double cUb, act, lb, repSol, repSolOld = INFINITY, lambdaIn = 0.5,
         lambdaOut = 0.5;

  std::copy(xOut, xOut+numVars, xu);
  std::copy(solC_, solC_+numVars, xl);

  if (vIdx != -1) {
    std::copy(solC_, solC_+numVars, xnew);
  }


  while (true) {
    if (vIdx == -1) {
      for (UInt i = 0 ; i < numVars; ++i) {
        xnew[i] = lambdaIn*xl[i] + lambdaOut*xu[i];
      }
    } else {
      xnew[vIdx] = lambdaIn*xl[vIdx] + lambdaOut*xu[vIdx];
    
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

bool Linearizations::findBoundaryPt_(double* xnew, const double *xOut,
                                     std::vector<ConstraintPtr> &vioCons,
                                     int vIdx)
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
    
  if (vIdx != -1) {
    std::copy(solC_, solC_+numVars, xnew);
  }

  while (true) {
    if (vIdx == -1) {
      for (UInt i = 0 ; i < numVars; ++i) {
        xnew[i] = lambdaIn*xl[i] + lambdaOut*xu[i];
      }
    } else {
      xnew[vIdx] = lambdaIn*xl[vIdx] + lambdaOut*xu[vIdx];
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
  bool shouldCont;
  ConstraintPtr newcon;
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
    if (vUb == INFINITY) {
      vLb = -50;
      vUb = 50;
    } else {
      vLb = vUb - 100;
    }
  } else {
    if (vUb == INFINITY) {
      vUb = vLb + 100;
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
      (UB !=0 && stopCond < fabs(UB)*solRelTol_ )) { 
    delete [] b1;
    return;
  }

  while (maxVio >= stopCond) { 
    //add a new cut at the point indexed i
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
            } else {
              xc.erase(xc.begin() + j);
              yc.erase(yc.begin() + j);
              linVioVal.erase(linVioVal.begin() + j);
            }
          } else {
            // delete point if violates newcon
            xc.erase(xc.begin() + j);
            yc.erase(yc.begin() + j);
            linVioVal.erase(linVioVal.begin() + j);
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
                if (isObj) {
                  if (lVarIdx == objVar_->getIndex()) {
                    act = act - b1[lVarIdx];
                  } 
                }
                vio = std::max(act-UB, 0.0); 
                linVioVal.insert(linVioVal.begin()+j+1,vio);
                xc.erase(xc.begin()+j+2);
                yc.erase(yc.begin()+j+2);        
                linVioVal.erase(linVioVal.begin()+j+2);        
              }
            }
            break;
          }  else {
            // delete point if violates newcon
            xc.erase(xc.begin() + j);
            yc.erase(yc.begin() + j);        
            linVioVal.erase(linVioVal.begin() + j);        
            --j;
          }  
        }
      }
    }

    maxVio = *(std::max_element(linVioVal.begin(), linVioVal.end()));
    if ((maxVio < solAbsTol_) || 
        (UB !=0 && maxVio < fabs(UB)*solRelTol_ )) { 
      break;
    }
    i = std::max_element(linVioVal.begin(), linVioVal.end())-linVioVal.begin();     
  }
  delete [] b1;
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


//MS: add esh all from LP solution - working
void Linearizations::rootLinScheme3(EnginePtr lpe, SeparationStatus *status)
{
  UInt numNl = nlCons_.size();
  if ((solC_ == 0 && numNl > 0) || oNl_) {
    //// ESH to all nonlinear constraints individually 
    //int error = 0;
    //FunctionPtr f;
    //bool vio, active;
    //ConstraintPtr con;
    //const double *lpx;
    //std::stringstream sstm;
    //double c, act, cUb, lpObj;
    //LinearFunctionPtr lf = 0;
    //UInt numOldCuts, initCuts = stats_->cuts;
    //double* xNew = new double[minlp_->getNumVars()];

    //if (numNl > 0) {
      //for (UInt i = 1; i <= rs3_; ++i) {
        //vio = false, active = false;
        //numOldCuts = stats_->cuts;
        //lpx = lpe->getSolution()->getPrimal();
        //lpObj = lpe->getSolution()->getObjValue();
        //for (CCIter it = nlCons_.begin(); it!=nlCons_.end(); ++it) {
          //con = *it;
          //cUb = con->getUb();
          //act = con->getActivity(lpx, &error);
          //if (error == 0) {
            //if ((act > cUb + solAbsTol_) &&
                //(cUb == 0 || act > cUb + fabs(cUb)*solRelTol_)) {
              //vio = true;
              //cutAtLineSearchPt_(solC_, lpx, xNew, con, lpObj);
            //} else if ((fabs(act-cUb) <= solAbsTol_) ||
                //(cUb != 0 && fabs(act-cUb) <= fabs(cUb)*solRelTol_)) {
              ////ConstraintPtr newcon;
              //active = true;
              //lf = 0;
              //f = con->getFunction();
              //linearAt_(f, act, lpx, &c, &lf, &error);
              //if (error == 0) {
                //++(stats_->cuts);
                //f = (FunctionPtr) new Function(lf);
                //sstm << "_OACutRoot_" << stats_->cuts;
                //rel_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
                //sstm.str("");
              //}
            //}
            //// no linearizations for constraints that are inactive at lpx
          //} else {
            //logger_->msgStream(LogError) << me_ << "Constraint" <<  con->getName()
              //<< " is not defined at this point." << std::endl;
          //}
        //}
        //if (oNl_ && active) {
          //objCut_(lpx, lpObj);
        //}
        //if (numOldCuts < stats_->cuts) {
          //if (vio) {
            //// not a boundary point when some cons are nonlinear
            //lpe->solve();
            //if (shouldStop_(lpe->getStatus())) {
              //break;    
            //} 
          //}
        //} else {
          //break;
        //}
      //}
    //} else if (oNl_) {
       //for (UInt i = 1; i <= rs3_; ++i) {
        //numOldCuts = stats_->cuts;
        //lpx = lpe->getSolution()->getPrimal();
        //lpObj = lpe->getSolution()->getObjValue();
        //objCut_(lpx, lpObj);
        //if (numOldCuts < stats_->cuts) {
          //lpe->solve();
          //if (shouldStop_(lpe->getStatus())) {
            //break;    
          //} 
        //} else {
          //break;
        //}
      //} 
    //}

    //stats_->rs3Cuts = stats_->cuts - initCuts; 
    //if (stats_->rs3Cuts > 0) {
      //*status = SepaResolve;
    //}
    //delete [] xNew;
    //return;

    //// ESH only at the boundary point
    double lpObj;
    const double *lpx;
    bool vio = false, active = false, ptFound;
    std::vector<ConstraintPtr > consToLin; // cons to add linearizations
    UInt numOldCuts, initCuts = stats_->cuts;
 
    if (numNl > 0) { 
      double* xnew = new double[minlp_->getNumVars()];
      for (UInt i = 1; i <= rs3_; ++i) {
        vio = false, active = false;
        lpx = lpe->getSolution()->getPrimal();
        lpObj = lpe->getSolution()->getObjValue();
        candLinCons_(lpx, consToLin, active, vio);
        if (vio) {
          // find boundary points and add cuts to nonlinear constraints and
          // objective (if nonlinear)
          numOldCuts = stats_->cuts;
          ptFound = findBoundaryPt_(xnew, lpx, consToLin, -1);
          if (ptFound) {
            // add linearization to active nonlinear constraints only if the
            // linearizations are far apart
            genLin_(xnew, consToLin);
            if (oNl_) {
              objCut_(xnew, lpObj);
            }
          }
          consToLin.clear();
          if (numOldCuts < stats_->cuts) {
            lpe->solve();
            if (shouldStop_(lpe->getStatus())) {
              break;    
            }
          } else {
            break;
          }
        } else if (active) {
          // Add cuts to objective and active constraints
          genLin_(lpx, consToLin);
          if (oNl_) {
            objCut_(lpx, lpObj);
          }
          consToLin.clear();
          break;
        } else {
          break;
        }
      }
      delete [] xnew;
    } else if (oNl_) {
      for (UInt i = 1; i <= rs3_; ++i) {
        numOldCuts = stats_->cuts;
        lpx = lpe->getSolution()->getPrimal();
        lpObj = lpe->getSolution()->getObjValue();
        objCut_(lpx, lpObj);
        if (numOldCuts < stats_->cuts) {
          lpe->solve();
          if (shouldStop_(lpe->getStatus())) {
            break;    
          } 
        } else {
          break;
        }
      }
    }

    stats_->rs3Cuts = stats_->cuts - initCuts; 
    if (stats_->rs3Cuts > 0) {
      *status = SepaResolve;
    }
  }
  
  return;
}

void Linearizations::objCut_(const double* xNew, double relobj)
{
  double c, act;
  int error = 0;
  FunctionPtr f;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  ObjectivePtr o = minlp_->getObjective();
  
  act = o->eval(xNew, &error);
  if (error == 0) {
    if (relobj != -INFINITY) {
      if (((act > relobj + solAbsTol_) &&
          (relobj == 0 || (act > relobj + fabs(relobj)*solRelTol_))) ||
          (relobj == -INFINITY)) {
        lf = 0;
        f = o->getFunction();
        linearAt_(f, act, xNew, &c, &lf, &error);
        if (error == 0) {
          ++(stats_->cuts);
          lf->addTerm(objVar_, -1.0);
          f = (FunctionPtr) new Function(lf);
          sstm << "_OACutObj_" << stats_->cuts;
          rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
          sstm.str("");
          //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        }
      }
    } else {
      lf = 0;
      f = o->getFunction();
      linearAt_(f, act, xNew, &c, &lf, &error);
      if (error == 0) {
        ++(stats_->cuts);
        lf->addTerm(objVar_, -1.0);
        f = (FunctionPtr) new Function(lf);
        sstm << "_OACutObj_" << stats_->cuts;
        rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
        sstm.str("");
        //newcon = rel_->newConstraint(f, -INFINITY, -1.0*c, sstm.str());
      }   
    }
  }	else {
    logger_->msgStream(LogError) << me_
      <<"objective not defined at this point."<< std::endl;
  }
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


double Linearizations::maxVio(const double *x, int &index)
{
  ConstraintPtr c;
  int error=0, i = 0;
  double act, cUb, vio = 0.0, max = -INFINITY;

  for (CCIter it=nlCons_.begin(); it!=nlCons_.end(); ++it, ++i) {
    c = *it;
    act = c->getActivity(x, &error);
    if (error == 0) {
      cUb = c->getUb();
      if (act > cUb+solAbsTol_ && (cUb == 0 ||
                                   act > cUb + fabs(cUb)*solRelTol_)) {
        //if (fabs(cUb) > solAbsTol_ && fabs(cUb) > solRelTol_) {
        if (fabs(cUb) > solAbsTol_) {
          vio = 100*(act - cUb)/fabs(cUb);      
        } else {
          vio = act - cUb;
        }
        if (vio > max) {
          max = vio;          
          index = i; 
        }
      }      
    }
  }
  return max;
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
