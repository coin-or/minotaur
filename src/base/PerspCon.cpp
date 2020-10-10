//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCon.cpp
 * \brief Define base class for finding constraints amenable to
 *  perspective reformulation (PR) in the given problem.
 * \Author Meenarli Sharma, Indian Institute of Technology Bombay
 */

#include <cmath>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <iterator>     // std::distance
#include <forward_list>


#include "Objective.h"
#include "Option.h"
#include "Logger.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"

using namespace Minotaur;
const std::string PerspCon::me_ = "PerspCon: ";

PerspCon::PerspCon()
:env_(EnvPtr()), p_(ProblemPtr()), cons_(0), bVar_(0), prConsVec_(0), 
  gubList0_(0), gubList1_(0)
{
  //stats_ = new prStats();
  //stats_->bvInLin = 0; 
  //stats_->bvInNlin = 0; 
  //stats_->bvInBoth = 0; 
  //stats_->bvIsZ = 0;  
  //stats_->bvIsZ1 = 0;
  //stats_->type1 = 0; 
  //stats_->type2 = 0; 
  //stats_->uniq = 0;
  //stats_->varFixing0 = 0;
  //stats_->varFixing1 = 0;     

  //stats_->ouniq = false;
  //stats_->obvInLin = false;
  //stats_->obvInNlin = false;
  //stats_->obvIsZ = false;
  //stats_->obvIsZ1 = false; 
}
//MS: include all the declared variables in .h file

PerspCon::PerspCon(EnvPtr env, ProblemPtr p)
: env_(env), p_(p), cons_(0), bVar_(0), prConsVec_(0), gubList0_(0),
  gubList1_(0)
{
  //stats_ = new prStats();
  //stats_->bvInLin = 0; 
  //stats_->bvInNlin = 0; 
  //stats_->bvInBoth = 0; 
  //stats_->bvIsZ = 0;  
  //stats_->bvIsZ1 = 0;
  //stats_->type1 = 0; 
  //stats_->type2 = 0;
  //stats_->uniq = 0;
  //stats_->varFixing0 = 0;
  //stats_->varFixing1 = 0;     
     
  //stats_->ouniq = false;
  //stats_->obvInLin = false;
  //stats_->obvInNlin = false;
  //stats_->obvIsZ = false;
  //stats_->obvIsZ1 = false;
  
  timer_ = env->getNewTimer();
  logger_ = env->getLogger();
  absTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  relTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
}


PerspCon::~PerspCon()
{
  if (timer_) {
    delete timer_;
  }
 
  // What else to reset here?
  p_ = 0;
  env_ = 0;
  
  delGUBList_();
}


void PerspCon::populate_(UInt type, VariableGroup nVarVal, 
                         VariableGroup lVarVal)
{
 
  if (isObj_) {
    prObj_.isPR = 1;
    prObj_.binVar = bVar_;
    prObj_.binVal = binVal_;
    prObj_.nNonzeroVar = nVarVal;
    prObj_.lNonzeroVar = lVarVal;
    prObj_.bisect = 1 - isInFunc_;

    //MS: to be removed later!
    //bool isFound = true;
    //ObjectivePtr o = p_->getObjective();
    //LinearFunctionPtr lf = o->getFunction()->getLinearFunction();
    //QuadraticFunctionPtr  qf = o->getFunction()->getQuadraticFunction();
    //NonlinearFunctionPtr  nlf = o->getFunction()->getNonlinearFunction();
    
    //if (lf && lf->hasVar(bVar_)) {
      //stats_->obvInLin = true;
    //}

    //if ((nlf && nlf->hasVar(bVar_)) || (qf && qf->hasVar(bVar_))) {
      //stats_->obvInNlin = true;
    //}
    
    //if (binVal_) {
      //stats_->obvIsZ1 = true;
    //} else {
      //stats_->obvIsZ = true;
    //}
    
    //for (UInt i = 0; i < prConsVec_.size() ; ++i) {
      //if (prConsVec_[i].binVar == bVar_) {
        //isFound = false;
        //break;      
      //}
    //}
    //if (isFound) {
      //stats_->ouniq = true;
    //}
  } else {
    //////////////////////////////////////////////////////////
    //MS: to be removed later!
    //UInt n = 0;
    //bool isFound = true;
    //LinearFunctionPtr lf = cons_->getLinearFunction();
    //QuadraticFunctionPtr qf = cons_->getQuadraticFunction();
    //NonlinearFunctionPtr nlf = cons_->getNonlinearFunction();

    //if (lf && lf->hasVar(bVar_)) {
      //n = 1;
    //}
     
    //if ((nlf && nlf->hasVar(bVar_)) || (qf && qf->hasVar(bVar_))) {
      //n = n + 2;
    //}

    //if ( n == 1 ) {
      //++stats_->bvInLin;
    //}
 
    //if ( n == 2 ) {
      //++stats_->bvInNlin;
    //}
 
    //if ( n == 3 ) {
      //++stats_->bvInBoth;
    //}
          
    //if (binVal_) {
      //++stats_->bvIsZ1;
    //} else {
      //++stats_->bvIsZ;
    //}

    //if (type == 1) {
      //++stats_->type1;
    //} else {
      //++stats_->type2;
    //}
    
    //for (UInt i = 0; i < prConsVec_.size() ; ++i) {
      //if (prConsVec_[i].binVar == bVar_) {
        //isFound = false;
        //break;      
      //}
    //}
    //if (isFound) {
      //++stats_->uniq;
    //}
    //////////////////////////////////////////////////////////
    prCons p;
    p.type = type;
    p.cons = cons_;
    p.binVar = bVar_;
    p.binVal = binVal_;
    p.lNonzeroVar = lVarVal;
    p.nNonzeroVar = nVarVal;
    p.bisect = 1 - isInFunc_;
    prConsVec_.push_back(p);


  }
  
 //p.numVarInNonLin = (cons_->getFunction()->getNumVars());
 //if (cons_->getLinearFunction()) {
   //p.numVarInNonLin -= (cons_->getLinearFunction()->getNumTerms());
 //}

 // MS: a little presolve. Check!
 //lf = cons_->getLinearFunction();
 //if (lf) {
   //for (VariableGroupConstIterator itvar=lf->termsBegin();
        //itvar!=lf->termsEnd(); ++itvar) {
     //var = itvar->first;
     //coeff = itvar->second;
     //vit = lVarVal.find(var);
     //if (vit == lVarVal.end()) {
       //vit = nVarVal.find(var);
       //if (vit != nVarVal.end()) {
         //fixedVal = vit->second;
       //} else {
         //continue;
       //}
     //} else {
       //fixedVal = vit->second;
     //:

   //}
 //}
 
 //if (p.bisect == 0 && type == 1) {
   //int error = 0;
   //double act, ub;
   //double * x = new double[p_->getNumVars()];
   // Value of x for different binVal_
   //std::fill(x, x+p_->getNumVars(), 0);
   //for (VariableGroupConstIterator vt=nVarVal.begin(); vt!=nVarVal.end();
        //++vt) {
    //x[(vt->first)->getIndex()] = vt->second; 
   //}
   //for (VariableGroupConstIterator vt=lVarVal.begin(); vt!=lVarVal.end();
        //++vt) {
    //x[(vt->first)->getIndex()] = vt->second; 
   //}
   //x[bVar_->getIndex()] = 1;
   //act = cons_->getActivity(x, &error);
   
   //if (error == 0) {
     //ub = cons_->getUb();
     //if ((fabs(act-ub) < absTol_) && (ub == 0 || 
                                      //(fabs(act-ub) < fabs(ub)*relTol_))) {
       //p.bisect = 1;
     //}
   //}
   //delete [] x;
 //}

 //prConsVec_.push_back(p);
 return;
}


bool PerspCon::isControlled_(std::vector<VariablePtr> binaries)
{
  bool isFound, isBinFixed;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator itB, itE;

  if (binVal_) {
    itE = impli1_.end();
    itB = impli1_.begin();
  } else {
    itE = impli0_.end();
    itB = impli0_.begin();
  }
    
  for (iit_ = itB; iit_ != itE;) {
    bVar_ = iit_->first;
    isFound = false;
    for (std::vector<VariablePtr>::iterator it = binaries.begin();
         it!=binaries.end(); ++it) {
      if (bVar_ == *it ) {
        isFound = true;
        break;      
      }
    }  

    if (isFound) {
      ++iit_;
      continue;    
    } else {
      isBinFixed = false;
      if (boundBinVar_(isBinFixed)) {
        return true;      
      }
      if (isBinFixed) {
        fixBinaryVar_(bVar_, binVal_);
        if (binVal_) {
          iit_ = impli1_.erase(iit_);
        } else {
          iit_ = impli0_.erase(iit_);
        }     
      } else {
        ++iit_;
      }
    }
  }

  return false;
}


// Returns 1 if binary var bVar_ is present in cons_
void PerspCon::checkBinPos_()
{
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;

  if (isObj_) {
    ObjectivePtr o = p_->getObjective();
    lf = o->getFunction()->getLinearFunction();
    qf = o->getFunction()->getQuadraticFunction();
    nlf = o->getFunction()->getNonlinearFunction();
  } else {
    lf = cons_->getLinearFunction();
    qf = cons_->getQuadraticFunction();
    nlf = cons_->getNonlinearFunction();
  }

  if (nlf && nlf->hasVar(bVar_)) {
    isInFunc_ = 1;
    return;
  }

  if (qf && qf->hasVar(bVar_)) {
    isInFunc_ = 1;
    return;
  }

  if (lf && lf->hasVar(bVar_)) {
    if (fabs(lf->getWeight(bVar_)) >= absTol_) {
      isInFunc_ = 1;    
      return;
    }    
  }

  isInFunc_ = 0;
}


bool PerspCon::boundBinVar_(bool &isBinFixed)
{
  VariableGroup nVarVal;
  double * x = new double[p_->getNumVars()];
  std::fill(x, x+p_->getNumVars(), 0.);
  if (binVal_) {
    x[bVar_->getIndex()] = 1;
  } else {
    x[bVar_->getIndex()] = 0;
  }

  // check if variables in the nonlinear part of the functions are controlled 
  bool boundsok = checkNVars_(x, nVarVal);
  
  if (boundsok) {
    UInt type = 0;
    int error = 0;
    VariableGroup lVarVal;
    QuadraticFunctionPtr qf;
    NonlinearFunctionPtr nlf;
    double cu, nval = 0, lval = 0;
    // checking for on-off set of type 2
  
    if (isObj_) {
      ObjectivePtr o = p_->getObjective();
      cu = -1*(o->getConstant()); 
      qf = o->getFunction()->getQuadraticFunction();
      nlf = o->getFunction()->getNonlinearFunction();
    } else {
      cu = cons_->getUb();
      qf = cons_->getQuadraticFunction();
      nlf = cons_->getNonlinearFunction();
    }

    if (nlf) {
      nval = nlf->eval(x, &error); 
    }

    if (error == 0) {
      if (qf) {
        nval = nval + qf->eval(x); 
      }

      //if (isObj_ && (cu + absTol_ <= nval)) {
        //prObj_.isPR = 1;
        //prObj_.binVar = bVar_;
        //prObj_.binVal = binVal_;
        //prObj_.nNonzeroVar = nVarVal;
        //prObj_.bisect = 1 - isInFunc_;
        
        ////prObj_.numVarInNonLin = (p_->getObjective()->getFunction()->getNumVars()); 
        ////if (p_->getObjective()->getLinearFunction()) {
          ////prObj_.numVarInNonLin -= (p_->getObjective()->getLinearFunction()->getNumTerms());
        ////}
      //}
    } else {
      delete [] x;
      return false;
    }
      
    boundsok = checkLVars_(x, lVarVal, nVarVal, lval);
    if (boundsok) {
      if (cu + absTol_ <= (nval + lval)) {
        isBinFixed = 1;
        boundsok = false;
      } else {
        type = 1;
        boundsok = true;
        populate_(type, nVarVal, lVarVal); 
      }       
    } else {
      if (cu - absTol_ <= (nval + lval)) {
        type = 2;
        boundsok = true;
        populate_(type, nVarVal, lVarVal); 
      }        
    }

    //check for on-off set of type 1
    //if (lf) {
      //boundsok = checkLVars_(x, lVarVal, nVarVal, &lval);
      //lval = lf->eval(x); 
      //if (boundsok) {
        //if (cu + absTol_ <= (nval + lval)) {
          //isBinFixed = 1;
          //boundsok = false;
        //} else {
          //type = 1;
          //boundsok = true;
          //populate_(type, nVarVal, lVarVal); 
        //}       
      //} else {
        //if (cu - absTol_ <= (nval + lval)) {
          //type = 2;
          //boundsok = true;
          //populate_(type, nVarVal, lVarVal); 
        //}        
      //}
    //} else {
      //boundsok = false;
      //if (cu + absTol_ <= nval) {
        //isBinFixed = 1;
        //boundsok = false;
      //} else {
        //type = 1;
        //boundsok = true;
        //populate_(type, nVarVal, lVarVal); 
      //}
    //}
  }
  delete [] x;
  return boundsok;
}

// Returns false if at least one variable in the linear part of constraint
// cons_ is not controlled by bVar_. lVarVal contains those variables that are
// not in the nonlinear part as well.
bool PerspCon::checkLVars_(double *x, VariableGroup &lVarVal, VariableGroup &nVarVal, double &lval)
{  
  ConstVariablePtr v;
  LinearFunctionPtr lf;
  bool isFound, allFound;
  std::forward_list<impliVar>::iterator it1;
  
  if (isObj_) {
    allFound = false;
    lf = p_->getObjective()->getFunction()->getLinearFunction();
  } else {
    allFound = true;
    lf = cons_->getLinearFunction();
  } 
  
  if (lf) {
    for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
         ++it) {
      v = it->first;
      if (v == iit_->first) {
        continue; 
      }
      
      if (nVarVal.find(v) != nVarVal.end()) {
        continue;
      }
      
      for (it1 = (iit_->second).begin(); it1 != (iit_->second).end(); ++it1) {
        isFound = false;
        if (v == (*it1).var) {
          isFound = true;
          x[v->getIndex()] = (*it1).fixedVal[0];
          //if (fabs(x[v->getIndex()]) >= absTol_) {
            lVarVal.insert({v, x[v->getIndex()]}); // storing value even if 0
          //}
          break;
        } 
      }
      if (false == isFound) {
        allFound = false;      
      }
    }
    lval = lf->eval(x); 
  }     

  return allFound;
}


bool PerspCon::checkNVars_(double *x, VariableGroup &nVarVal)
{
  ConstVariablePtr v;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  bool chkNlPart = false, isFound;
  std::forward_list<impliVar>::iterator it1;
  
  if (isObj_) {
    FunctionPtr f = p_->getObjective()->getFunction();
    qf = f->getQuadraticFunction();
    nlf = f->getNonlinearFunction();
  } else {
    qf = cons_->getQuadraticFunction();
    nlf = cons_->getNonlinearFunction();
  }

  if (nlf == 0 && qf == 0) {
    return false;
  }
   
  if (nlf) {
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
      v = *it;
      if (v == iit_->first) {
        continue; 
      }
      
      isFound = false;
      for (it1 = (iit_->second).begin(); it1 != (iit_->second).end(); ++it1) {
        if (v == (*it1).var) {
          isFound = true;
          chkNlPart = true;
          x[v->getIndex()] = (*it1).fixedVal[0];
          //if (fabs(x[v->getIndex()]) >= absTol_) {
            nVarVal.insert({v, x[v->getIndex()]});  // storing value even if 0         
          //}
          break;
        }
      }

      if (!isFound) {
        return false;
      }
    }
  } 

  if (qf) {
    for (VarIntMapConstIterator it=qf->varsBegin(); it!=qf->varsEnd(); ++it) {
      v = it->first;
      if ((v == iit_->first) || (nlf && nlf->hasVar(v))) {
        continue;        
      }

      isFound = false;
      for (it1 = (iit_->second).begin(); it1 != (iit_->second).end(); ++it1) {
        if (v == (*it1).var) {
          isFound = true;
          chkNlPart = true;
          x[v->getIndex()] = (*it1).fixedVal[0];
          if (fabs(x[v->getIndex()]) >= absTol_) {
            nVarVal.insert({v, x[v->getIndex()]});          
          }
          break;
        }
      }

      if (!isFound) {
        return false;
      }
    }
  }
  return chkNlPart;
}


void PerspCon::findBinVarsInFunc_(std::vector<VariablePtr>* binaries)
{
  FunctionPtr f;
  VariablePtr var;

  if (isObj_) {
    f = p_->getObjective()->getFunction();
  } else {
    f = cons_->getFunction();
  }

  for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    var = *it;
    if (isBinary_(var)) {
      (*binaries).push_back(var);
    }
  }
  return;
}


void PerspCon::displayInfo_()
{
  //MS: make display better. Keep limited information remove S1 and S2
  prCons p;
  bool isObjPR = 0, isZ = 0, zIn = 0;
  //UInt n = 0;
  UInt s = prConsVec_.size(), type1 = 0, type2 = 0;

  std::ostream &out = logger_->msgStream(LogInfo);
 
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator mit;
    
  out << "----------------------------------------------------"<< std::endl;
  out << "------- Variable Fixing ------------------------------"<< std::endl;
  out << me_ << "----------------z = 0 case-----------------------\n";
  for (mit = impli0_.begin(); mit != impli0_.end(); ++mit) {
    out << me_ << "---------------------------------------\n";
    out << me_ << "Binary var name and ID " << mit->first->getName() << " " << mit->first->getIndex() << "\n";
    for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      out << me_ << "var " << (*it1).var->getName() << " " << (*it1).fixedVal[0] << "\n";
    }
  }
  
  out << me_ << "--------------------------\n"; 
  out << me_ << "----------------z = 1 case-----------------------\n";
  for (mit = impli1_.begin(); mit != impli1_.end(); ++mit) {
    out << me_ << "---------------------------------------\n";
    out << me_ << "Binary var name and ID " << mit->first->getName() << " " << mit->first->getIndex() << "\n";
    for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      out << me_ << "var " << (*it1).var->getName() << " " << (*it1).fixedVal[0] << "\n";
    }
  }  

  out << "---------- PR details --------------------------------"<< std::endl;
  if (prObj_.isPR) {
    out << me_ <<"Objective is PR amenable." << std::endl;
    out << "Associated binary variable and val: " << (prObj_.binVar)->getName() << " and " <<
      (prObj_.binVal) << std::endl;
    //out << "Associated binary variable present in function: " << 1 - prObj_.bisect << std::endl;
    out << me_ <<"Vars controlled and their fixed value: ";

    for (VariableGroupConstIterator vt=prObj_.nNonzeroVar.begin(); vt!=prObj_.nNonzeroVar.end(); ++vt) {
      out << "(" << vt->first->getName() << ", " << vt->second << "), ";
    }


    for (VariableGroupConstIterator vt=prObj_.lNonzeroVar.begin(); vt!=prObj_.lNonzeroVar.end(); ++vt) {
      out << "(" << vt->first->getName() << ", " << vt->second << "), ";
    }

    isObjPR = 1;
    isZ = 1 - prObj_.bisect;
    //out << "No. of vars in nonlinear part " << 1 - prObj_.numVarInNonLin << std::endl;
    out << "\n----------------------------------------------------"<< std::endl;
  } else {
    out << me_ <<"Objective is not PR amenable." << std::endl;
  }
  if (s > 0) {
    s = 0;
    out << "----------------------------------------------------"<< std::endl;
    out << "*********************************************************************"<< std::endl;
    out << me_ << "Details of constraints amenable to perspective reformulation:\n";
    out << "*********************************************************************"<< std::endl;
    for (UInt i = 0; i < prConsVec_.size() ; ++i) {
      out << std::endl; 
      p = prConsVec_[i];
      out << i+1 << ". ";
      p.cons->write(out);
      if (p.bisect) {
        ++s;      
      }

      if (1 - p.bisect) {
        ++zIn;
      }

      switch (p.type) {
      case 1:
        type1 += 1;
        break;
      case 2:
        type2 += 1;
        break;
      default:
        break;
      }
      //if (p.numVarInNonLin > 1) {
        //++n;      
      //}
      out << "Structure type: S" << p.type << std::endl;
      out << "Associated binary variable and val: " << (p.binVar)->getName() << " and " <<
        (p.binVal) << std::endl;

      out << me_ <<"Vars controlled and their fixed value: ";

      for (VariableGroupConstIterator vt=p.nNonzeroVar.begin(); vt!=p.nNonzeroVar.end(); ++vt) {
        out << "(" << vt->first->getName() << ", " << vt->second << "), ";
      }


      for (VariableGroupConstIterator vt=p.lNonzeroVar.begin(); vt!=p.lNonzeroVar.end(); ++vt) {
        out << "(" << vt->first->getName() << ", " << vt->second << "), ";
      }

      std::cout << "\n";
    }
    out << "----------------------------------------------------"<< std::endl;
 
    out << me_ <<"Total nonlinear constraints in problem = " <<
      p_->getNumCons() - p_->getNumLinCons() << std::endl; 
    out << me_ <<"Number of constraints amenable to PR = " << 
      prConsVec_.size() << std::endl;
    out << me_ <<"No. of type 1 and 2 = " << type1 << " " << type2 << "\n";


    //out << me_ <<"No. of constraints amenable to PR with bisect amenable structure = " << s << std::endl;
    //out << me_ <<"No. of constraints amenable to PR with more than one term in nonlinear part of the function = " << n << std::endl;
    out << "----------------------------------------------------"<< std::endl;
    //out << me_ << isObjPR << " " << isZ << " " << p_->getNumCons() - p_->getNumLinCons() << " " << prConsVec_.size() << " " <<  type1 << " " << type2 << " " << zIn <<  " " << stats_->ouniq << "\n";
  } else {
    out << me_ <<"Number of constraints amenable to PR = 0" << std::endl;
    out << me_ << isObjPR << " "  << isZ << " " <<  p_->getNumCons() - p_->getNumLinCons()  << " " << 0 << " " << 0 << " " <<  0 << " " << 0 << " " << 0 << "\n";
  }

  //out << me_ << "Objective: " << prObj_.isPR << " " << stats_->obvInLin << " " << stats_->obvInNlin << " " << stats_->obvIsZ << " " << stats_->obvIsZ1 <<  " " << stats_->uniq << "\n";

  //out << me_ << "PR Cons details: " << p_->getNumCons() - p_->getNumLinCons() << " " << prConsVec_.size() << " " << stats_->bvInLin  << " " << stats_->bvInNlin  << " " << stats_->bvInBoth << " " << stats_->bvIsZ  << " " << stats_->bvIsZ1  << " " << stats_->type1  << " " << stats_->type2 << " " << stats_->uniq << "\n";
}


void PerspCon::detect_()
{
  bool isFound = false, isBinFixed;
  std::vector<VariablePtr> binaries;
  
  /// Binary variables in the constraint under consideration
  findBinVarsInFunc_(&binaries);
 
  /// Check if variables in the constraint are controlled by some binary
  //varaible in the same constraint
  for (std::vector<VariablePtr>::iterator it = binaries.begin();
       it!=binaries.end(); ++it) {
    bVar_ = *it;
    checkBinPos_();
    isBinFixed = 0;
    iit_ = impli0_.find(bVar_);
    if (iit_ != impli0_.end()) {
      binVal_ = 0;
      isFound = boundBinVar_(isBinFixed);    
      if (isBinFixed) {
        fixBinaryVar_(bVar_, binVal_);
        if (binVal_) {
          impli1_.erase(iit_);
        } else {
          impli0_.erase(iit_);
        }     
      }
    }
    if (isFound) {
      return;
    } else {
      binVal_ = 1;
      iit_ = impli1_.find(bVar_);
      if (iit_ != impli1_.end()) {
        isFound = boundBinVar_(isBinFixed);    
        if (isBinFixed) {
          fixBinaryVar_(bVar_, binVal_);
          if (binVal_) {
            impli1_.erase(iit_);
          } else {
            impli0_.erase(iit_);
          }     
        }
      }
      if (isFound) {
        return;
      }
    }
  }
    
  isInFunc_ = 0;

  /// Check if there exists a binary variable in impli0_ that controls all the
  //variables in the constraint
  if (impli0_.size() > 0) {
    binVal_ = 0;
    isFound = isControlled_(binaries);
  }

  /// Check if there exists a binary variable in impli0_ that controls all the
  //variables in the constraint
  if (!isFound && impli1_.size() > 0) {
    binVal_ = 1;
    isFound = isControlled_(binaries);
  }

  return;
}


bool PerspCon::multiTermsFunc_(ConstraintPtr c, VariablePtr var, 
                            std::forward_list<impliVar> *varList, 
                            double val, bool z)
{
  VariablePtr v;
  impliVar impli;
  std::forward_list<impliVar>::iterator it1;
  LinearFunctionPtr lf = c->getLinearFunction();
  double vc, vlb, vub, lb = c->getLb(), ub = c->getUb();
  bool isFound = true, ubChecked = false, cFeas = false, cInf = false, 
       allBin = true;
  
  if (fabs(ub-val) < absTol_) {
    ubChecked = 1;
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      if (v != var) {
        vc = itvar->second;
        if (z && allBin) {
          if (isBinary_(v)) {
            if ((fabs(vc - ub) >= absTol_)) {
              allBin = false;
            }
          } else {
            if ((fabs(vc) >= absTol_)) {
              allBin = false;
            }
          }
        }

        vlb = v->getLb();
        vub = v->getUb();
        if (vc >= absTol_) {
          if (fabs(vlb) < absTol_) {
            cFeas = 1;
          } else if (vlb >= absTol_) {
            cInf = 1; 
          } else {
            isFound = false;
            break;
          }
        } else if (vc <= -absTol_) {
          if (fabs(vub) < absTol_) {
            cFeas = 1;
          } else if (vub <= -absTol_) {
            cInf = 1; 
          } else {
            isFound = false;
            break;
          }
        }
      }
    }
  }

  if ((fabs(lb-val) < absTol_) && (fabs(lb-ub) >= absTol_)) {
    if ((ubChecked && !isFound) || (ubChecked == 0)) {
      isFound = true; cFeas = false; cInf = false;
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        v = itvar->first;
        if (v != var) {
          vc = itvar->second;
          if (z && allBin) {
            if (isBinary_(v)) {
              if ((fabs(vc - lb) >= absTol_)) {
                allBin = false;
              }
            } else {
              if ((fabs(vc) >= absTol_)) {
                allBin = false;
              }
            }
          }

          vlb = v->getLb();
          vub = v->getUb();
          if (vc >= absTol_) {
            if (fabs(vub) < absTol_) {
              cFeas = 1;
            } else if (vub <= -absTol_) {
              cInf = 1; 
            } else {
              isFound = false;
              break;
            }
          } else if (vc <= -absTol_) {
            if (fabs(vlb ) < absTol_) {
              cFeas = 1;
            } else if (vlb >= absTol_) {
              cInf = 1; 
            } else {
              isFound = false;
              break;
            }
          } 
        }
      }   
    }
  }
  
  if (z && allBin) {
    bool consFound = false;
    for (std::forward_list<ConstraintPtr>::iterator cit = gubList1_.begin();
         cit != gubList1_.end(); ++cit) {
      if (c == *cit) {
        consFound = true;
        break;      
      }    
    }
    if (!consFound) {
      gubList1_.push_front(c);
    }
  }

  if (isFound) {
    if (cFeas && !cInf) {
      impli.fixedVal.push_back(0);
      impli.fixedVal.push_back(0);
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        v = itvar->first;
        if ((v == var) || (fabs(itvar->second) < absTol_)) {
          continue;
        }
        for (it1 = (*varList).begin(); it1 != (*varList).end(); 
           ++it1) {
          if (v == (*it1).var) {
            break;
          }
        }
        if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal[0] = 0;
          impli.fixedVal[1] = 0;
          (*varList).push_front(impli);
        } else {
          if ((*it1).fixedVal[0] <= -absTol_) {
            (*it1).fixedVal[0] = 0;
          }
          if ((*it1).fixedVal[1] >= absTol_) {
            (*it1).fixedVal[1] = 0;
          }
        }
      }
    } else if (cInf) {
      fixBinaryVar_(var, z);
      return true;
    }
  }
  return false;
}


bool PerspCon::twoTermsFunc_(ConstraintPtr c, VariablePtr var, 
                            std::forward_list<impliVar> *varList, bool z)
{
  VariablePtr v;
  impliVar impli;
  double vc, bc = 0, val, vlb, vub;
  bool isFixed = false, allBin = true;
  double lb = c->getLb(), ub = c->getUb();
  std::forward_list<impliVar>::iterator it1;
  LinearFunctionPtr lf = c->getLinearFunction();

  for (VariableGroupConstIterator itvar=lf->termsBegin();
       itvar!=lf->termsEnd(); ++itvar) {
    if (itvar->first != var) {
      v = itvar->first;
      vc = itvar->second;
      if (fabs(vc) < absTol_) {
        return false;
      }
      if (z || (fabs(lb-ub) < absTol_)) {
        if (isBinary_(v)) {
          if ((fabs(vc - ub) >= absTol_)) {
            allBin = false;
          }        
        } else {
          if ((fabs(vc) >= absTol_)) {
            allBin = false;
          }
        }
      } else {
        allBin = false;
      }

    } else {
      bc = itvar->second;
      if ((fabs(bc - ub) >= absTol_)) {
        allBin = false;
      }
      if (!z) {
        bc = 0;
      } 
    }
  }

  
  if (allBin) {
    bool consFound = false;
    if (z) {
      for (std::forward_list<ConstraintPtr>::iterator cit = gubList1_.begin();
           cit != gubList1_.end(); ++cit) {
        if (c == *cit) {
          consFound = true;
          break;      
        }    
      }
      if (!consFound) {
        gubList1_.push_front(c);
      }
    } else {
      for (std::forward_list<ConstraintPtr>::iterator cit = gubList0_.begin();
           cit != gubList0_.end(); ++cit) {
        if (c == *cit) {
          consFound = true;
          break;      
        }    
      }
      if (!consFound) {
        gubList0_.push_front(c);
      }
    }
  } 
  
  for (it1 = (*varList).begin(); it1 != (*varList).end(); 
        ++it1) {
    if (v == (*it1).var) {
      break;
    }
  } 
        
  vlb = v->getLb();
  vub = v->getUb();

  if (fabs(lb-ub) < absTol_) {
    val = (ub-bc)/vc;
    if ((val <= vlb - absTol_)  || (val >= vub + absTol_)) {
      isFixed = true;
    }
    if (isBinary_(v)) {
      if ((fabs(val) >= absTol_) && (fabs(val) <= 1-absTol_)) {
        isFixed = true;            
      }          
    }
    
    if (isFixed) {
      fixBinaryVar_(var, z);
      return true;
    }
    if (it1 == (*varList).end()) {
      impli.var = v;
      impli.fixedVal.push_back(val);
      impli.fixedVal.push_back(val);
      (*varList).push_front(impli);
    } else {
      if (((*it1).fixedVal[0] + absTol_) <= val) {
        (*it1).fixedVal[0] = val;
      }
      if (val <= ((*it1).fixedVal[1] - absTol_)) {
        (*it1).fixedVal[1] = val;
      }
    }
  } else {
    if (ub != INFINITY) {
      val = (ub-bc)/vc;
      if (vc >= absTol_) {
        if (val <= vlb - absTol_) {
          fixBinaryVar_(var, z);
          return true;
        }
        // obtained upper bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          // new variable
          impli.fixedVal.push_back(vlb);
          impli.fixedVal.push_back(val);
        } else {
          // existing variable
          if (val <= (*it1).fixedVal[1] - absTol_) {
            (*it1).fixedVal[1] = val;
          }
        }
      } else {
        if (val >= vub + absTol_) {
          fixBinaryVar_(var, z);
          return true;
        }
         // obtained lower bound
         if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal.push_back(val);
          impli.fixedVal.push_back(vub);
        } else {
          if (val >= (*it1).fixedVal[0] + absTol_) {
            (*it1).fixedVal[0] = val;
          }
        }               
      }              
    }

    if (lb != -INFINITY) {
      val = (lb-bc)/vc;
      if (vc >= absTol_) {
         if (val >= vub + absTol_) {
          fixBinaryVar_(var, z);
           return true;
         }
          // obtained lower bound
        if (it1 == (*varList).end()) {
          if (impli.fixedVal.size()) {
            if (impli.fixedVal[0] < val) {
              impli.fixedVal[0] = val;
            }
             if (impli.fixedVal[1] > vub) {
              impli.fixedVal[1] = vub;
            }         
          } else {
            impli.var = v;
            impli.fixedVal.push_back(val);
            impli.fixedVal.push_back(vub);
          }
        } else {
          if (val >= (*it1).fixedVal[0] + absTol_) {
            (*it1).fixedVal[0] = val;
          }
        }
      } else {
         if (val <= vlb - absTol_) {
          fixBinaryVar_(var, z);
           return true;
         }
        // obtained upper bound
        if (it1 == (*varList).end()) {
          if (impli.fixedVal.size()) {
            if (impli.fixedVal[0] < vlb) {
              impli.fixedVal[0] = vlb;
            }
             if (impli.fixedVal[1] > val) {
              impli.fixedVal[1] = val;
            }         
          } else {
            impli.var = v;
            impli.fixedVal.push_back(vlb);
            impli.fixedVal.push_back(val);
          }
        } else {
          if (val <= (*it1).fixedVal[1] - absTol_) {
            (*it1).fixedVal[1] = val;
          }
        }              
      }             
    }
          
    if (impli.fixedVal.size()) {
      (*varList).push_front(impli);
    }
  }
      
  return false;
}

void PerspCon::fixBinaryVar_(VariablePtr var, bool z)
{
  if (z) {
    //++stats_->varFixing0;
    // Infeasibility with var = 1. Fixing this variable to 0.
    var->setUb_(0);      
  } else {
    //++stats_->varFixing1;
    // Infeasibility with var = 0. Fixing this variable to 1.
    var->setLb_(1);
  }
  return;
}

void PerspCon::deriveImpli_(VariablePtr var)
{
  UInt numVars;
  impliVar impli;
  ConstraintPtr c;
  double lb, ub, val;
  LinearFunctionPtr lf;
  bool isFixed0 = false, isFixed1 = false;
  std::forward_list<impliVar>::iterator it1;
  std::forward_list<impliVar> varList0, varList1;

  // Iterate over each constraint in which variable var appears
  for (ConstrSet::iterator it=var->consBegin(); it!=var->consEnd(); ++it) {
    c = *it;
    // consider a cons if linear
    if (c->getFunctionType() == Linear) {
      lf = c->getLinearFunction();
      val = lf->getWeight(var);
      // continue if var has 0 coefficient
      if (fabs(val) < absTol_) {
        continue;      
      }
    
      numVars = lf->getNumTerms();
      if (numVars == 2) {
        // store vars whose lower or upper bound are set by var = 0
        isFixed0 = twoTermsFunc_(c, var, &varList0, 0);
        
        // if var is not fixed to 0, then store vars whose lower or upper 
        // bound are set by var = 1
        if (!isFixed0) {
          isFixed0 = twoTermsFunc_(c, var, &varList1, 1);
        }
      } 
      else if (numVars > 2) {
        lb = c->getLb();
        ub = c->getUb();
        if ((fabs(lb) < absTol_) || (fabs(ub) < absTol_)) {
          isFixed0 = multiTermsFunc_(c, var, &varList0, 0, 0);
        } else {
          isFixed0 = false;
        }
        if (!isFixed0) {
          if ((fabs(lb-val) < absTol_) || (fabs(ub-val) < absTol_)) {
            isFixed0 = multiTermsFunc_(c, var, &varList1, val, 1);
          }
        }
      } 
     // if the binary var is fixed to either 0 or 1 then return 
      if (isFixed0) {
        return;
      }
    }
  }
  
  // based on bounds obtained obtain vars that can be controlled with var = 0
  // and 1
  isFixed0 = addImplications_(&varList0);
  isFixed1 = addImplications_(&varList1);

  if (!isFixed0 && !isFixed1) {
    // if var not fixed then save the implications obtained
    if (!(varList0.empty())) {
      impli0_.insert({var, varList0});  
    }
      
    if (!(varList1.empty())) {
      impli1_.insert({var, varList1});  
    }
  } else if (isFixed0 && isFixed1) {
    // if var is found to be fixed to both values then infeasible
    assert(!"In PerspCon: Problem infeasible.");
  } else if (isFixed0) {
    // Infeasibility with var = 0. Fixing this variable to 1.
    var->setLb_(1); 
  } else {
    // Infeasibility with var = 1. Fixing this variable to 0.
    var->setUb_(0);  
  }

  return;
}


bool PerspCon::addImplications_(std::forward_list<impliVar> *varList)
{
  /// Check this deletion once
  double lb, ub, valLb, valUb;
  std::forward_list<impliVar>::iterator it1;
  std::forward_list<impliVar>::iterator prev = (*varList).before_begin();  

  for (it1 = (*varList).begin(); it1 != (*varList).end(); ) {
    lb = (*it1).var->getLb();
    ub = (*it1).var->getUb();
    valLb = (*it1).fixedVal[0];
    valUb = (*it1).fixedVal[1];
    if ((valUb <= valLb - absTol_) || (lb - absTol_ >= valUb) 
        || (ub + absTol_ <= valLb)) {
      return true;
    }

    if (valLb + absTol_ <= valUb) {
      it1 = (*varList).erase_after(prev);
    } else {
      (*it1).fixedVal.pop_back();
      prev = it1;
      ++it1;    
    }
  }
  return false;
}


bool PerspCon::isBinary_(VariablePtr var)
{
  switch (var->getType()) {
  case Binary:
  case ImplBin:
    return true;
    break;
  case Integer:
  case ImplInt:
    if ((fabs(var->getLb()) < absTol_) && 
        (fabs(var->getUb() - 1) < absTol_)) {
      return true;
    } 
    break;
  default:
    break;
  }
  return false;
}


void PerspCon::removeSingleton_()
{
  VariablePtr var;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  double vc, val1, val2;

 // Only if presolved is off perform this 
 // Move singleton constraints as bounds
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    c = *it;
    lf = c->getLinearFunction();
    if ((c->getFunctionType() == Linear) && 
        lf->getNumTerms() == 1) {
      vc = lf->termsBegin()->second;
      var = lf->termsBegin()->first;
      if (vc > -absTol_) {
        val1 = c->getUb();
        if (val1 != INFINITY) {
          val2 = val1/vc ;
          if (val2 <= val1 - absTol_) {
            var->setUb_(val2);
          }        
        }
        val1 = c->getLb();
        if (val1 != -INFINITY) {
          val2 = val1/vc ;
          if (val2 >= val1 + absTol_) {
            var->setLb_(val2);
          }        
        }
      } else {
        val1 = c->getUb();
        if (val1 != INFINITY) {
          val2 = val1/vc ;
          if (val2 >= val1 + absTol_) {
            var->setLb_(val2);
          }        
        }
        val1 = c->getLb();
        if (val1 != -INFINITY) {
          val2 = val1/vc ;
          if (val2 <= val1 - absTol_) {
            var->setUb_(val2);
          }        
        }     
      }
    }
  }
  return;
}

//MS: clear timer
void PerspCon::implications()
{
  //timer_->start();
  bool isFound;
  VariablePtr var;
  ConstraintPtr c;

  // Remove singleton if presolve is off
  if (env_->getOptions()->findBool("presolve")->getValue() == false) {
    removeSingleton_();
  }

  for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); 
       ++it) {
    var = *it;
    // Find variables controlled by each binary variable
    if (isBinary_(var)) {
      deriveImpli_(var);
    }
  }

  // Return if no implication is found
  if ((impli0_.size() == 0) && (impli1_.size() == 0)) {
    //std::cout << "collection time " << std::setprecision(6) << timer_->query() << std::endl;
    //timer_->stop();
    //std::cout << "C23 details: 0 0 0 0 0 0" << std::endl;
    //exit(1);
    return;  
  }

  // Derive more implications for equality constraints in which only one var
  // is not controlled but others are controlled by some binary variable
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    c = *it;
    if ((c->getFunctionType() == Linear) && 
        (fabs(c->getLb() - c->getUb()) < absTol_)) {
      // Case: binary var when fixed to 0
      if (impli0_.size() != 0) {
        isFound = false;
        // Consider constraint which is not in gubList0_
        if (c->getLinearFunction()->getNumTerms() == 2) {
          for (std::forward_list<ConstraintPtr>::iterator cit = gubList0_.begin();
               cit != gubList0_.end(); ++cit) {
            if (c == *cit) {
              isFound = true;
              break;      
            }    
          }
        }

        if (!isFound) {
          addImplications_(c, 0);
        }
      }
      // Case: binary var when fixed to 1
      if (impli1_.size() != 0) {
        isFound = false;
        // Consider constraint which is not in gubList1_
        for (std::forward_list<ConstraintPtr>::iterator cit = gubList1_.begin();
             cit != gubList1_.end(); ++cit) {
          if (c == *cit) {
            isFound = true;
            break;      
          }    
        }
        if (!isFound) {
          addImplications_(c, 1);
        }
      }
    }
  }

  // MS: small presolve. Add description. (Check)
  //std::vector<ConstraintPtr > cp;
  //std::forward_list<impliVar>::iterator it1;
  //std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator mit;
  //for (mit = impli0_.begin(); mit != impli0_.end(); ++mit) {
    //for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      //var = (*it1).var;
      //for (ConstrSet::const_iterator cIter=var->consBegin(); cIter!=var->consEnd(); ++cIter) {
        //c = *cIter;
        //lf = c->getLinearFunction();

        //if (lf) {
          //if (c->getFunctionType() == Linear) {
            //coeff = lf->getWeight(var);
          //} else {
            //if (lf->hasVar(var)) {
              //coeff = lf->getWeight(var);
            //}          
          //}
          //lb = c->getLb();
          //ub = c->getUb();
          //if (lb != -INFINITY && ub != INFINITY) {
            //if (fabs(lb-ub) < solAbsTol_) {
              //continue;       
            //}
            //cp.push_back(c);
            //continue;
          //} else if (lb != -INFINITY) {
            //if ((coeff >= absTol_ && var->getLb() >= absTol_ && (*it1).fixedVal[0] >= absTol_) ||
                //(coeff <= absTol_ && var->getUb() <= absTol_ && (*it1).fixedVal[0] <= absTol_)) {



            //ub = INFINITY;
            //lfc = con->getLinearFunction()->clone();
            //lfc->addTerm(vPtr, 1.0);
          //} else if (ub != INFINITY ) {
            //lb = -INFINITY;
            //lfc = con->getLinearFunction()->clone();
            //lfc->addTerm(vPtr, -1.0);
          //} else {
            //inst_C->markDelete(con);
            //continue;
          //}
          //if (coeff >= absTol_) {
            //if (fabs() > absTol_) {
            
            //}          
          //} else {
          
          //}
        //}
      //}    
    //}
  //}

    
  //displayInfo_();
  //

  //bool found; 
  //std::forward_list<ConstraintPtr >::iterator git;
  //std::forward_list<impliVar>::iterator it1;
  //std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator mit;
  //UInt c0 = 0, c1 = 0, g0 = 0, g1 = 0, cr = 0, cb = 0, ci = 0, nb = 0; 
  //for (mit = impli0_.begin(); mit != impli0_.end(); ++mit) {
    //for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      //++c0;
    //}
  //}
  
  //for (mit = impli1_.begin(); mit != impli1_.end(); ++mit) {
    //for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      //++c1;
    //}
  //}
    
  ////for (git = gubList0_.begin(); git != gubList0_.end(); ++git) {
    ////++g0;
  ////}
  
  ////for (git = gubList1_.begin(); git != gubList1_.end(); ++git) {
    ////++g1;
  ////}

  ////std::cout << "C1 details: " << c0 - g0 << " " << c1 - g1 << std::endl;

  //for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); 
       //++it) {
    //var = *it;
        
    //if (isBinary_(var)) {
      //if (impli0_.find(var) != impli0_.end() || impli1_.find(var) != impli1_.end()) {
        //++nb;
      //}
    //}
 
   //found = 0; 
    //for (mit = impli0_.begin(); mit != impli0_.end(); ++mit) {
      //for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
        //if (var == (*it1).var) {
            //switch (var->getType()) {
            //case Binary:
            //case ImplBin:
              //++cb;
              //break;
            //case Integer:
            //case ImplInt:
              //++ci;
              //break;
            //default:
              //++cr;
              //break;
            //}
            //found = 1;
            //break;
        //}
      //}
      //if (found) {
        //break;
      //}
    //}
    //if (found) {
      //continue;    
    //} else {
      //for (mit = impli1_.begin(); mit != impli1_.end(); ++mit) {
        //for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
          //if (var == (*it1).var) {
            //switch (var->getType()) {
            //case Binary:
            //case ImplBin:
              //++cb;
              //break;
            //case Integer:
            //case ImplInt:
              //++ci;
              //break;
            //default:
              //++cr;
              //break;
            //}
            //found = 1;
            //break;
          //}
        //}
        //if (found) {
          //break;
        //}
      //}   
    //}
  //}
  //std::cout << "C23 details: " << c0 << " " << c1 << " " << nb << " " << cr << " " << cb << " " << ci << " " << std::endl;
  //exit(1);
  //std::cout << "collection time " << std::setprecision(6) << timer_->query() << std::endl;
  //timer_->stop();
  //exit(1);
  return;
}

void PerspCon::delGUBList_()
{
  LinearFunctionPtr lf;
  ConstraintPtr c;
  VariablePtr v, v1;
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator 
    it;
  std::forward_list<impliVar>::iterator prev;

  while (!gubList0_.empty()) {
    c = gubList0_.front();
    lf = c->getLinearFunction();
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      it = impli0_.find(v);
      if (it != impli0_.end()) {
        prev =  (it->second).before_begin();
        for (it1 = (it->second).begin(); it1 != (it->second).end();) {
          v1 = (*it1).var;
          if (lf->hasVar(v1)) {
            it1 = (it->second).erase_after(prev);
          } else {
            prev = it1;
            ++it1;
          }
        }

        if (it->second.empty()) {
          impli0_.erase(it);
        }
      }
    }
    gubList0_.pop_front();
  }


  while (!gubList1_.empty()) {
    c = gubList1_.front();
    lf = c->getLinearFunction();

    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      it = impli1_.find(v);
      if (it != impli1_.end()) {
        prev =  (it->second).before_begin();
        for (it1 = (it->second).begin(); it1 != (it->second).end();) {
          v1 = (*it1).var;
          if (lf->hasVar(v1)) {
            it1 = (it->second).erase_after(prev);
          } else {
            prev = it1;
            ++it1;
          }
        }
        if (it->second.empty()) {
          impli1_.erase(it);
        }
      }
    }
    gubList1_.pop_front();
  }
}


void PerspCon::addImplications_(ConstraintPtr c, bool z)
{
  bool found;
  impliVar impli;
  VariablePtr var;
  LinearFunctionPtr lf;
  UInt listSize, count;
  double ub, val, vc;
  impli.fixedVal.push_back(-INFINITY);
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator 
    it, itEnd;
  impli.fixedVal.push_back(-INFINITY);

  if (z) {
    it = impli1_.begin();
    itEnd = impli1_.end(); 
  } else {
    it = impli0_.begin();
    itEnd = impli0_.end();
  }

  ub = c->getUb();
    
  lf = c->getLinearFunction();
  for (; it != itEnd; ) {
    it1 = (it->second).begin();
    listSize = std::distance(it1, (it->second).end());
    if (lf->getNumTerms() <= (2 + listSize)) {
      val = 0;
      count = 0;
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        var = itvar->first;
        if (var == it->first) {
          if (z) {
            val = val + itvar->second;
          }
          continue;
        }

        found = false;
        for (it1 = (it->second).begin(); it1 != (it->second).end(); ++it1) {
          if (var == (*it1).var) {
            found = true;
            val = val + (itvar->second)*((*it1).fixedVal[0]);
            break;
          }
        }
        if (!found) {
          ++count;
          if (count > 1) {
            break;
          }
          impli.var = var;
          vc = itvar->second;
        }
      }
      if (count == 1 && (fabs(vc) >= absTol_)) {
        val = (ub-val)/vc;
        found = true;
        if ((val <= impli.var->getLb() - absTol_) 
            || (val >= impli.var->getUb() + absTol_)) {
          found = false;
        }
        if (isBinary_(impli.var)) {
          if ((fabs(val) >= absTol_) && (fabs(val) <= 1-absTol_)) {
            found = false;            
          }          
        }
        
        if (!found) {
          if (z == 0) {
            // Infeasibility with var = 0. Fixing this variable to 1.
            (it->first)->setLb_(1);           
            impli1_.erase(it->first);
            it = impli0_.erase(it);
            if (impli0_.empty()) {
              break;
            }
          } else {
            // Infeasibility with var = 1. Fixing this variable to 0.
            (it->first)->setUb_(0);           
            impli0_.erase(it->first);
            it = impli1_.erase(it);
            if (impli1_.empty()) {
              break;
            }             
          }
        } else {
          impli.fixedVal[0] = val;
          (it->second).push_front(impli);          
          ++it;
        }
      } else {
        ++it;        
      }
    } else {
      ++it;      
    }
  }
}


void PerspCon::findPRCons()
{
  /// To determine all the variables fixed by binary variables
  //Perform this step if there are linear constraints and binary variables 
  timer_->start();
  implications(); 


  isObj_ = 0;
  //// This to check only nonlinear constraints one by one
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    cons_ = *it;
    switch(cons_->getFunctionType()) {
    case Linear:
      break;
    case Nonlinear:
    case Quadratic:
    case Bilinear:
    case Polynomial:
    case Multilinear:
      detect_();
      break;
    default:
      break;
    }
  }
  ObjectivePtr o = p_->getObjective();
  FunctionType fType = o->getFunctionType();
  if (!o) {
    assert(!"need objective in QG!");
  } else if (fType != Linear && fType != Constant) {
    isObj_ = 1;
    detect_();
  }
  //std::cout << "Fixed to 0 and 1: " << stats_->varFixing0 << " " << stats_->varFixing1 << "\n";
  //exit(1);
  //std::cout << "Total PR detection time " << std::setprecision(6) << timer_->query() << std::endl;
  //timer_->stop();
  ////#if SPEW 
  //displayInfo_();
  ////p_->write(std::cout);
  //exit(1);
  //#endif  

  // Delete implications derived from constraints in gubList0_ and gubList1_
  //MS: delGUBList_();

  cons_ = 0;
  bVar_ = 0;
  return;
}


bool PerspCon::getStatus()
{
  if(prConsVec_.size() > 0 || prObj_.isPR){
    return true; 
  } else {
    return false;
  }
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
