//
//     Minotaur -- It's only 1/2 bull  
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//
 
/**
 * \file MultiSolHeur.cpp
 * \brief Define the MultiSolHeur class for generating a feasible solution 
 * using NLP and MILP solves for MINLPs.
 * \author Meenarli Sharma, Prashant Palkar, Indian Institute of Technology
 * Bombay
 */

#include "MinotaurConfig.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "MILPEngine.h"
#include "MultiSolHeur.h"
#include "Function.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Node.h"
#include "Operations.h"
#include "Option.h"
#include "SolutionPool.h"
#include "Objective.h"
#include "ProblemSize.h"
#include "QuadraticFunction.h"
#include "Timer.h"
#include "Types.h"
#include "VarBoundMod.h"
#include "Variable.h"
#include <cmath>
#include <iomanip>
#include <cstdlib>

using namespace Minotaur;

//#define SPEW 1

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string MultiSolHeur::me_ = "MultiSol Heuristic: ";

MultiSolHeur::MultiSolHeur()
: env_(EnvPtr()),
  minlp_(ProblemPtr()),
  miqp_(ProblemPtr()),
  nlpe_(EnginePtr()),
  milpe_(MILPEnginePtr()),
  logger_(LoggerPtr()),   // NULL
  ub_(INFINITY),
  lb_(-INFINITY),
  objVar_(VariablePtr()),
  bestSol_(SolutionPtr()),   // NULL
  newSolFound_(false),
  nlCons_(0),
  xC_(NULL),
  stats_(NULL),
  timer_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  solATol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();

}


MultiSolHeur::MultiSolHeur(EnvPtr env, ProblemPtr p, EnginePtr nlpe,
    MILPEnginePtr milpe)
: env_(env),
  minlp_(p),
  miqp_(ProblemPtr()),
  nlpe_(nlpe),
  milpe_(milpe),
  ub_(INFINITY),
  lb_(-INFINITY),
  objVar_(VariablePtr()),
  bestSol_(SolutionPtr()),   // NULL
  newSolFound_(false),
  nlCons_(0),
  xC_(NULL)
{
  // initialize the logger pointer
  logger_ = env->getLogger();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  objATol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  solATol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();

  
  // statistics for Feasibilty Pump heuristic
  stats_                 = new MultiSolHeurStats();
  stats_->milpS          = 0;
  stats_->nlpS           = 0;
  stats_->nlpInf         = 0;
  stats_->nlpLim         = 0;
  stats_->nlpOthr        = 0;
  stats_->time           = 0;
  stats_->cuts           = 0;

  timer_                 = env_->getNewTimer();
}


MultiSolHeur::~MultiSolHeur()
{
  //MS: other deletion
  delete stats_;

  if (timer_) {
    delete timer_;
  }

  if (xC_) {
    delete xC_;
  }

  if (bestSol_) {
    delete bestSol_;
  }
  delete nlpe_;
  nlpe_ = 0;
}


void MultiSolHeur::improveSol_(const double * x0)
{
  double gapA = ub_ - lb_, gapR = 0;
  if (fabs(ub_) != 0) {
    gapR = (ub_-lb_)/fabs(ub_);
  }
  while (gapA > objATol_ && gapR > objRTol_) {
    if (newSolFound_) {
      //FunctionPtr f = newObj_(x0, bestSol_->getPrimal());
      newSolFound_ = false;
    }
  
  }
  //int error = 0;
  //VariablePtr v;
  //FunctionPtr fnewc;
  //int n = miqp_->getNumVars();
  //double *a = new double[n];
  //const double *x = bestSol_->getPrimal();
  //LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
  //FunctionPtr f = minlp_->getObjective()->getFunction();
 
  //std::fill(a, a+n, 0.);
  //f->evalGradient(x, a, &error);

  //// Modify objective 
  //if(error == 0) {
    //miqp_->removeObjective();
    //for (VariableConstIterator vit=miqp_->varsBegin(); vit!=miqp_->varsEnd()-1;
         //++vit) {
      //v = *vit;
      //lfc->addTerm(v, a[v->getIndex()]);
    //}
    //fnewc = (FunctionPtr) new Function(lfc);
    //miqp_->newObjective(fnewc, 0.0, Minimize);

    //// Define TR
    //relMods_(x);
  
    //++(stats_->milpS);
    //milpe_->load(miqp_);        

    //undoMods_();
  //}
  
  //delete [] a;
  return;
}


/* Find nearest integer point from the line segment joining x^o and x^c.  
 *
 */
void MultiSolHeur::solveAtSol(ConstSolutionPtr sol, RelaxationPtr rel,
                              SolutionPoolPtr sPool)
{
  ConstraintPtr con;
  FunctionType fType;
  for (ConstraintConstIterator it = minlp_->consBegin();
       it != minlp_->consEnd(); ++it) {
    con = *it;
    fType = con->getFunctionType();
    if (fType != Constant && fType != Linear) {
      nlCons_.push_back(con);
    }
  }
  // Find center
  findCenter_();
  
  // Construct the MIQP
  if (xC_) {
    const double *x0 = sol->getPrimal();
    newMIQP_(rel, x0, sPool);
    if (bestSol_) {
      improveSol_(x0);    
    }
  }
}

FunctionPtr MultiSolHeur::newObj_(const double * x0, const double * x)
{
  UInt vidx;
  VariablePtr v;
  bool hasInt = 0;
  double c1 = 0, c2 = 0, c3 = 0, c;
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  QuadraticFunctionPtr qf = (QuadraticFunctionPtr) new QuadraticFunction();
  for (VariableConstIterator v_iter=miqp_->varsBegin(); 
       v_iter!=miqp_->varsEnd(); ++v_iter) {
    v = *v_iter;
    if (v->getType() == Binary || v->getType() == Integer) {
      hasInt = 1;
      vidx = v->getIndex();
      v->setFunType_(Nonlinear);
      c1 = x0[vidx] - x[vidx];
      c2 = c2 + c1*c1;
      c3 = c3 + c1*x[vidx];
      c = -2*x[vidx];
      lf->addTerm(v,c);
      qf->addTerm(v, v, 1);
      c = -2*c1;
      qf->addTerm(v, objVar_, c);
    }
  }
  if (hasInt) {
    c3 = 2*c3;
    lf->addTerm(objVar_, c3);
    qf->addTerm(objVar_, objVar_, c2);
    FunctionPtr f = (FunctionPtr) new Function(lf, qf);
    return f;
  }
  return 0;
}
/* Heuristic exploring integer points around x^0 with same objective as the 
 * original problem
 */
void MultiSolHeur::newMIQP_(RelaxationPtr rel, const double * x0,
                            SolutionPoolPtr sPool)
{
  //UInt vidx;
  //VariablePtr v;
  //bool hasInt = 0;
  //double c1 = 0, c2 = 0, c3 = 0, c;
  miqp_ = rel->clone(env_);
  //LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  //QuadraticFunctionPtr qf = (QuadraticFunctionPtr) new QuadraticFunction();
  VariablePtr newVPtr = miqp_->newVariable(0, 1, Continuous, "lambda", VarHand);
  objVar_ = newVPtr;
  FunctionPtr f = newObj_(x0, xC_);
  //for (VariableConstIterator v_iter=miqp_->varsBegin(); 
       //v_iter!=miqp_->varsEnd(); ++v_iter) {
    //v = *v_iter;
    //if (v->getType() == Binary || v->getType() == Integer) {
      //hasInt = 1;
      //vidx = v->getIndex();
      //v->setFunType_(Nonlinear);
      //c1 = x0[vidx] - xC_[vidx];
      //c2 = c2 + c1*c1;
      //c3 = c3 + c1*xC_[vidx];
      //c = -2*xC_[vidx];
      //lf->addTerm(v,c);
      //qf->addTerm(v, v, 1);
      //c = -2*c1;
      //qf->addTerm(v, newVPtr, c);
    //}
  //}
  if (f) {
    //c3 = 2*c3;
    //lf->addTerm(newVPtr, c3);
    //qf->addTerm(newVPtr, newVPtr, c2);
    //FunctionPtr f = (FunctionPtr) new Function(lf, qf);
    miqp_->removeObjective();
    miqp_->newObjective(f, 0.0, Minimize);
    miqp_->prepareForSolve();
    //std::cout << "Original MINLP \n";
    //minlp_->write(std::cout);
    //std::cout << "Relaxation \n";
    //rel->write(std::cout);
    //std::cout << "MIQP \n";
    //miqp_->write(std::cout);
    //exit(1);

    itrMilpSolve_(sPool);

    //UInt itr = 1;
    //bool isTerm = false;
    //SolveStatus milpStatus;
    //while (!isTerm) { 
      //solveMILP(milpStatus);
      ///// Solve fixed NLP for each solution in the MILP solution pool
      //if (milpStatus == SolvedOptimal) {
        ////load problem 
        //double oldBest = sPool->getBestSolutionValue();
        //nlpe_->load(minlp_);
        //bestObjVal_ = oldBest;
        //ConstSolutionPtr milpSol;
        //int numSols = milpe_->getNumSols();
        ///// Infeasible solutions are stored in a vector. 
        //std::vector<SolutionPtr> infSols;
        //std::vector<SolutionPtr> limSols;
        //for (int i=-1; i < numSols-1; ++i) {
          //milpSol = milpe_->getSolutionFromPool(i);
          //findSol_(milpSol, sPool, oldBest, isTerm, infSols, limSols);
          //if (isTerm) {
            //break;
          //}
        //}
        //if (!isTerm) {
          //SeparationStatus status = SepaContinue;
          //for (std::vector<SolutionPtr>::iterator it=infSols.begin(); it!=infSols.end(); 
               //++it) {
            //cutToCons_((*it)->getPrimal(), milpSol->getPrimal(), &status);
            //delete *it;
          //}
          //for (std::vector<SolutionPtr>::iterator it=limSols.begin(); it!=limSols.end(); 
               //++it) {
            //cutsAtLpSol_((*it)->getPrimal(), &status);
            //delete *it;
          //}
          //if (status != SepaResolve) {
            //isTerm = 1;          
          //}
        //}
      //} else {
        //isTerm = true;      
      //}
      //std::cout << "Iteration, NLs solved and infeasible " << itr << " " << stats_->nlpS << " " << stats_->nlpInf << std::endl;
      //++itr;
    //}
  }
}


void MultiSolHeur::itrMilpSolve_(SolutionPoolPtr sPool)
{
  UInt itr = 1;
  bool isTerm = false;
  SolveStatus milpStatus;
  while (!isTerm) { 
    solveMILP(milpStatus);
    /// Solve fixed NLP for each solution in the MILP solution pool
    if (milpStatus == SolvedOptimal) {
      //load problem 
      double oldBest = sPool->getBestSolutionValue();
      nlpe_->load(minlp_);
      ub_ = oldBest;
      ConstSolutionPtr milpSol;
      int numSols = milpe_->getNumSols();
      /// Infeasible solutions are stored in a vector. 
      std::vector<SolutionPtr> infSols;
      std::vector<SolutionPtr> limSols;
      for (int i=-1; i < numSols-1; ++i) {
        milpSol = milpe_->getSolutionFromPool(i);
        findSol_(milpSol, sPool, isTerm, infSols, limSols);
        //findSol_(milpSol, sPool, oldBest, isTerm, infSols, limSols);
        if (isTerm) {
          break;
        }
      }
      if (!isTerm) {
        SeparationStatus status = SepaContinue;
        for (std::vector<SolutionPtr>::iterator it=infSols.begin(); it!=infSols.end(); 
             ++it) {
          cutToCons_((*it)->getPrimal(), milpSol->getPrimal(), &status);
          delete *it;
        }
        for (std::vector<SolutionPtr>::iterator it=limSols.begin(); it!=limSols.end(); 
             ++it) {
          cutsAtLpSol_((*it)->getPrimal(), &status);
          delete *it;
        }
        if (status != SepaResolve) {
          isTerm = 1;          
        }
      }
    } else {
      isTerm = true;      
    }
    std::cout << "Iteration, NLs solved and infeasible " << itr << " " << stats_->nlpS << " " << stats_->nlpInf << std::endl;
    ++itr;
  }
}

void MultiSolHeur::cutToCons_(const double *nlpx, const double *lpx,
                           SeparationStatus *status)
{
  int error = 0;
  ConstraintPtr con;
  double nlpact, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    con = *it;
    nlpact =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((nlpact > cUb + solATol_) &&
          (cUb == 0 || nlpact > cUb+fabs(cUb)*solRTol_)) {
        addCut_(nlpx, lpx, con, status);
      } 
    }	
  }
  return;
}


void MultiSolHeur::cutsAtLpSol_(const double *lpx,
                             SeparationStatus *status)
{
  int error=0;
  FunctionPtr f;
  ConstraintPtr con;
  LinearFunctionPtr lf;
  std::stringstream sstm;
  //ConstraintPtr newcon;
  double c, lpvio, act, cUb;

  for (CCIter it = nlCons_.begin(); it != nlCons_.end(); ++it) {
    lf = 0;
    con = *it;
    f = con->getFunction();
    act =  con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      if ((act > cUb + solATol_) &&
          (cUb == 0 || act > cUb+fabs(cUb)*solRTol_)) {
        linearAt_(f, act, lpx, &c, &lf, &error);
        if (error == 0) {
          lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
          if ((lpvio > solATol_) && ((cUb-c)==0 ||
                                   (lpvio>fabs(cUb-c)*solRTol_))) {
            ++(stats_->cuts);
            *status = SepaResolve;
            sstm << "_qgCut_" << stats_->cuts;
            f = (FunctionPtr) new Function(lf);
            miqp_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
            //newcon = miqp_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
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

  return;
}


void MultiSolHeur::addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con,
                        SeparationStatus *status)
{
  int error=0;
  //ConstraintPtr newcon;
  std::stringstream sstm;
  LinearFunctionPtr lf = 0;
  double c, lpvio, act, cUb;
  FunctionPtr f = con->getFunction();

  act = con->getActivity(nlpx, &error);
  if (error == 0) {
    linearAt_(f, act, nlpx, &c, &lf, &error);
    if (error == 0) {
      cUb = con->getUb();
      lpvio = std::max(lf->eval(lpx)-cUb+c, 0.0);
      if ((lpvio > solATol_) &&
          ((cUb-c)==0 || (lpvio>fabs(cUb-c)*solRTol_))) {
        ++(stats_->cuts);
        sstm << "_qgCut_" << stats_->cuts;
        *status = SepaResolve;
        f = (FunctionPtr) new Function(lf);
        miqp_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        //newcon = miqp_->newConstraint(f, -INFINITY, cUb-c, sstm.str());
        return;
      } else {
        delete lf;
        lf = 0;
      }
    }
  }	else {
    logger_->msgStream(LogError) << me_ << " constraint not defined at"
      << " this point. "<<  std::endl;
  }
  return;
}
 
void MultiSolHeur::linearAt_(FunctionPtr f, double fval, const double *x,
                          double *c, LinearFunctionPtr *lf, int *error)
{
  int n = miqp_->getNumVars();
  double *a = new double[n];

  VariableConstIterator vbeg = miqp_->varsBegin(), vend = miqp_->varsEnd();
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


//void MultiSolHeur::newMIQP(RelaxationPtr rel, const double * x0)
  //UInt n = 0;
  //VariablePtr v, newVPtr;
  //CNode * xn, * x0n, * lamn, * w1n, *c1n, *c2n;
  //std::vector<CNode *> temp;
  //const double *x = sol->getPrimal();
  //CGraphPtr t = (CGraphPtr) new CGraph();
  //ProblemPtr miqp = rel->clone(env_);
  //newVPtr = miqp->newVariable(0, 1, Continuous, "lambda", VarHand);
  ////std::cout << "Problem before construction\n";
  ////inst_C->write(std::cout);
            
  //// Node for lambda
  //lamn = t->newNode(newVPtr);
  //c2nlamn = t->newNode(newVPtr);
  //c1n  = t->newNode(Minotaur::OpUMinus, lamn, 0);

          
  //for (VariableConstIterator v_iter=miqp->varsBegin(); 
                  //v_iter!=miqp->varsEnd(); ++v_iter) {
          //v = *v_iter;
          //if (v->getType() == Binary || v->getType() == Integer) {
            //v->setFunType_(Nonlinear);
            //xn  = t->newNode(v);
            //x0n = t->newNode(x0[v->getIndex()]);


          //}
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
  ////std::cout << "Shortest distance NLP \n";
  ////inst_C->write(std::cout);
  
  //delete inst_C;
  //inst_C = 0;
  //delete [] childr;
  //temp.clear();
  //return;
//}

void MultiSolHeur::findCenter_()
{
  if (nlCons_.size() > 0) { 
    ifNonlinCons_();
  } else {
    ObjectivePtr o = minlp_->getObjective();
    FunctionType fType = o->getFunctionType();
       
    if (!o) {
      assert(!"need objective in QG!");
    } else if (fType != Linear && fType != Constant) {
      ifOnlyNonlinObj_();
    } 
  }

  return;
}


void MultiSolHeur::ifOnlyNonlinObj_()
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
        if (fabs(lb-ub) <= solATol_) {
          //hasEqCons_ = 1;
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
    if (fabs(lb-ub) <= solATol_) {
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

  //// To print interior point
  //if (xC_) {
    //std::cout << "Center found " << nlpe_->getSolution()->getObjValue() << std::endl;  
  //} else {
    //std::cout << "Center not found\n";  
  //}

  delete inst_C;
  inst_C = 0;
  return;
}

void MultiSolHeur::ifNonlinCons_()
{
  double lb, ub;
  FunctionPtr f;
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
  f = (FunctionPtr) new Function(lfc);
  inst_C->newObjective(f, 0.0, Minimize);

  // Modify all inequality constraints
  for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
       ++it) {
    con = *it;
    lb = con->getLb();
    ub = con->getUb();
    fType = con->getFunctionType();
    if (fType == Linear)  {
      if (lb != -INFINITY && ub != INFINITY) {
        if (fabs(lb-ub) <= solATol_) {
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
      inst_C->changeConstraint(con, lfc, lb, ub);
    } else if (fType == Constant) {
      continue;   
    } else {
      lb = con->getLb();
      ub = con->getUb();
      if (con->getLinearFunction()) {
        lfc = con->getLinearFunction()->clone();
        lfc->addTerm(vPtr, -1.0);
      } else {
        lfc = (LinearFunctionPtr) new LinearFunction();
        lfc->addTerm(vPtr, -1.0);
      }
      inst_C->changeConstraint(con, lfc, lb, ub);     
    }
  }

  for (UInt i = 0; i < cp.size(); ++i) {
    con = cp[i];
    lb = con->getLb(), ub = con->getUb();
    lfc = con->getLinearFunction()->clone();
    lfc->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lfc);
    inst_C->newConstraint(f, lb, INFINITY);

    lfc = con->getLinearFunction()->clone();
    lfc->addTerm(vPtr, -1.0);
    f = (FunctionPtr) new Function(lfc);
    inst_C->newConstraint(f, -INFINITY, ub);
  }
  cp.clear();
  inst_C->delMarkedCons();

  for (VariableConstIterator vit=inst_C->varsBegin(); vit!=inst_C->varsEnd()-1;
       ++vit) {
    v = *vit;
    lb = v->getLb(), ub = v->getUb();
    if (fabs(lb-ub) <= solATol_) {
      continue;
    }

    if (lb != -INFINITY) {
      lfc = (LinearFunctionPtr) new LinearFunction();
      lfc->addTerm(vPtr, 1.0);
      lfc->addTerm(v, 1.0);
      f = (FunctionPtr) new Function(lfc);
      inst_C->newConstraint(f, lb, INFINITY);
    }
    
    if (ub != INFINITY) {
      lfc = (LinearFunctionPtr) new LinearFunction();
      lfc->addTerm(vPtr, -1.0);
      lfc->addTerm(v, 1.0);
      f = (FunctionPtr) new Function(lfc);
      inst_C->newConstraint(f, -INFINITY, ub);
    }
  }

  //std::cout <<" ORIGINAL PROBLEM \n";
  //minlp_->write(std::cout);
  //std::cout <<" MODIFIED PROBLEM \n";
  //inst_C->write(std::cout);
 
  inst_C->prepareForSolve();  
  nlpe_->load(inst_C);
  solveNLP_();

  // To print interior point
  //if (xC_) {
    //std::cout << "Center found " << nlpe_->getSolution()->getObjValue() << std::endl;  
  //} else {
    //std::cout << "Center not found\n";  
  //}

  delete inst_C;
  inst_C = 0;
  return;
}

///// Considering only nonlinear constraints
//void MultiSolHeur::ifNonlinCons_()
//{
  //double lb, ub;
  //FunctionPtr f;
  //ConstraintPtr con;
  //FunctionType fType;
  //VariablePtr vPtr, v;
  //std::vector<ConstraintPtr > cp;
  //ProblemPtr inst_C = minlp_->clone(env_);
  //LinearFunctionPtr lfc = (LinearFunctionPtr) new LinearFunction();
 
  //// Modify objective 
  //vPtr = inst_C->newVariable(-INFINITY, 0, Continuous, "eta", VarHand);
  //vPtr->setFunType_(Nonlinear);
  //inst_C->removeObjective();
  //lfc->addTerm(vPtr, 1.0);
  //f = (FunctionPtr) new Function(lfc);
  //inst_C->newObjective(f, 0.0, Minimize);

  //// Modify the nonlinear constraints and let linear constraints as they are
  //for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
     //++it) {
    //con = *it;
    //fType = con->getFunctionType();
    //if (fType != Linear && fType != Constant)  {
      //lb = con->getLb();
      //ub = con->getUb();
      //if (con->getLinearFunction()) {
        //lfc = con->getLinearFunction()->clone();
        //lfc->addTerm(vPtr, -1.0);
      //} else {
        //lfc = (LinearFunctionPtr) new LinearFunction();
        //lfc->addTerm(vPtr, -1.0);
      //}
      //inst_C->changeConstraint(con, lfc, lb, ub);
    //}
  //}  

  //inst_C->prepareForSolve();
  //nlpe_->load(inst_C);
  //solveNLP_();

  ////std::cout <<" ORIGINAL PROBLEM \n";
  ////minlp_->write(std::cout);
  ////std::cout <<" MODIFIED PROBLEM \n";
  ////inst_C->write(std::cout);
  
  //if (nlpe_->getStatusString() == "ProvenUnbounded") {
    //logger_->msgStream(LogDebug) << me_ 
      //<< " Problem for finding center is unbounded." <<
     //" Solving a restricted problem." << std::endl;

    //for (ConstraintConstIterator it=inst_C->consBegin(); it!=inst_C->consEnd();
         //++it) {
      //con = *it;
      //lb = con->getLb();
      //ub = con->getUb();
      //fType = con->getFunctionType();
      //if (fType == Linear)  {
        //if (lb != -INFINITY && ub != INFINITY) {
          //if (fabs(lb-ub) <= solATol_) {
            //continue;       
          //}
          //cp.push_back(con);
          //inst_C->markDelete(con);
          //continue;
        //} else if (lb != -INFINITY) {
          //ub = INFINITY;
          //lfc = con->getLinearFunction()->clone();
          //lfc->addTerm(vPtr, 1.0);
        //} else if (ub != INFINITY ) {
          //lb = -INFINITY;
          //lfc = con->getLinearFunction()->clone();
          //lfc->addTerm(vPtr, -1.0);
        //} 
        //inst_C->changeConstraint(con, lfc, lb, ub);
      //}  
    //}

    //for (UInt i = 0; i < cp.size(); ++i) {
      //con = cp[i];
      //lb = con->getLb(), ub = con->getUb();
      //lfc = con->getLinearFunction()->clone();
      //lfc->addTerm(vPtr, 1.0);
      //f = (FunctionPtr) new Function(lfc);
      //inst_C->newConstraint(f, lb, INFINITY);

      //lfc = con->getLinearFunction()->clone();
      //lfc->addTerm(vPtr, -1.0);
      //f = (FunctionPtr) new Function(lfc);
      //inst_C->newConstraint(f, -INFINITY, ub);
    //}
    //cp.clear();
    //inst_C->delMarkedCons();

    //for (VariableConstIterator vit=inst_C->varsBegin(); vit!=inst_C->varsEnd()-1;
         //++vit) {
      //v = *vit;
      //lb = v->getLb(), ub = v->getUb();
      //if (fabs(lb-ub) <= solATol_) {
        //continue;
      //}

      //if (lb != -INFINITY) {
        //lfc = (LinearFunctionPtr) new LinearFunction();
        //lfc->addTerm(vPtr, 1.0);
        //lfc->addTerm(v, 1.0);
        //f = (FunctionPtr) new Function(lfc);
        //inst_C->newConstraint(f, lb, INFINITY);
      //}
      
      //if (ub != INFINITY) {
        //lfc = (LinearFunctionPtr) new LinearFunction();
        //lfc->addTerm(vPtr, -1.0);
        //lfc->addTerm(v, 1.0);
        //f = (FunctionPtr) new Function(lfc);
        //inst_C->newConstraint(f, -INFINITY, ub);
      //}
    //}
    //inst_C->write(std::cout);

    //inst_C->prepareForSolve();  
    //nlpe_->load(inst_C);
    //solveNLP_();
  //} 

  //// To print interior point
  ////if (xC_) {
    ////std::cout << "Center found " << nlpe_->getSolution()->getObjValue() << std::endl;  
  ////} else {
    ////std::cout << "Center not found\n";  
  ////}

  //delete inst_C;
  //inst_C = 0;
  //return;
//}


void MultiSolHeur::solveNLP_()
{ 
  if (xC_) {
    delete [] xC_;
    xC_ = 0;
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
      xC_ = new double[numVars];
      std::copy(temp, temp+numVars, xC_);
    }
    break;
  case (ProvenUnbounded):
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


void MultiSolHeur::findSol_(ConstSolutionPtr sol, SolutionPoolPtr sPool,
                            bool &isTerm,
                            //double oldBest, bool &isTerm,
                            std::vector<SolutionPtr> & infSols,
                            std::vector<SolutionPtr> & limSols)
{
  fixInts_(sol->getPrimal());           // Fix integer variables
 
  EngineStatus nlpStatus = nlpe_->solve();  // Solve fixed NLP
  ++(stats_->nlpS);
  
  unfixInts_();            // Unfix integer variables
  
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    {
      double nlpval = nlpe_->getSolutionValue();
      if ((ub_ - objATol_ > nlpval) ||
          (ub_ != 0 && (ub_ - fabs(ub_)*objRTol_ > nlpval))) {
        if (bestSol_ ) {
          delete bestSol_;
        } 
        ub_ = nlpval;
        newSolFound_ = true;
        bestSol_ = new Solution(nlpe_->getSolution());
        std::cout << "Best solution value: " << nlpval << "\n";
      }
      //if ((oldBest - objATol_ > nlpval) ||
          //(oldBest != 0 && (oldBest - fabs(oldBest)*objRTol_ > nlpval))) {
        sPool->addSolution(nlpe_->getSolution());
        isTerm = true;
      //}
    }
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
    {
      ++(stats_->nlpInf);
      SolutionPtr newsol = new Solution(nlpe_->getSolution());
      infSols.push_back(newsol);
    }
    break;
  case (EngineIterationLimit):
    {
      ++(stats_->nlpLim);
      SolutionPtr newsol = new Solution(sol);
      limSols.push_back(newsol);
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
    ++(stats_->nlpOthr);
    logger_->msgStream(LogError) << me_ << "NLP engine status = "
      << nlpe_->getStatusString() << std::endl;
    logger_->msgStream(LogError)<< me_ << "No cut generated, may cycle!"
      << std::endl;
    isTerm = true;
    break;
  }

  return;
}


void MultiSolHeur::fixInts_(const double *x)
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
      mods_.push(m);
    }
  }
  return;
}

void MultiSolHeur::undoMods_()
{
  Modification *m = 0;

  while(mods_.empty() == false) {
    m = mods_.top();
    m->undoToProblem(miqp_);
    mods_.pop();
    delete m;
  }
  return;
}

void MultiSolHeur::relMods_(const double *x)
{
  UInt k = 1; 
  double lb, ub;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=miqp_->varsBegin(); vit!=miqp_->varsEnd();
       ++vit) {
    v = *vit;
    lb = std::max(ceil(x[v->getIndex()] - k), v->getLb());
    ub = std::min(floor(x[v->getIndex()] + k), v->getUb());
    m = new VarBoundMod2(v, lb, ub);
    m->applyToProblem(miqp_);
    mods_.push(m);
  }
  return;
}

void MultiSolHeur::unfixInts_()
{
  Modification *m = 0;

  while(mods_.empty() == false) {
    m = mods_.top();
    m->undoToProblem(minlp_);
    mods_.pop();
    delete m;
  }
  return;
}

//void MultiSolHeur::solveMILP(RelaxationPtr rel, SolveStatus &status)
void MultiSolHeur::solveMILP(SolveStatus &status)
{
  //milpe_->load(rel);        
  ++(stats_->milpS);
  milpe_->load(miqp_);        
  EngineStatus milpStatus = milpe_->solve();

  switch (milpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    status = SolvedOptimal;
    break;
  case (EngineIterationLimit): 
    status = IterationLimitReached;
    break;
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible):
  case (ProvenObjectiveCutOff):
    logger_->msgStream(LogError) << me_ << "MILP engine status = "
      << milpStatus << std::endl;
    status = SolvedInfeasible;
    break;
  case (ProvenUnbounded):
  case (ProvenFailedCQFeas):
  case (ProvenFailedCQInfeas):
  case (FailedFeas):
  case (FailedInfeas):
  case (EngineError):
  case (EngineUnknownStatus):
  default:
    logger_->msgStream(LogError) << me_ << "MILP engine status= "
      << milpStatus << std::endl;
    assert(!"MILP Engine errored: Stopped. Check error log.");
    status = SolveError;
    break;
  }
  return;
}


void MultiSolHeur::writeStats(std::ostream &out) const
{
  out << me_ << "number of nlps solved         = " << stats_->nlpS
    << std::endl
    << me_ << "numer of infeasible NLPs      = " << stats_->nlpInf
    << std::endl
    << me_ << "total time taken             = " << stats_->time
    << std::endl;
  if (ub_ < INFINITY) {
    out << me_ << "best objective value         = " 
      << ub_ << std::endl;
  }
}
