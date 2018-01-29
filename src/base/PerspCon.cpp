//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCon.cpp
 * \brief Define base class for generating list of constraints amenable to
 *  perspective reformulation (PR).
 * \Author Meenarli Sharma, India Institute of Technology  Bombay
 */

#include <iostream>
#include <cmath>
using std::endl;

#include "Option.h"
#include "Logger.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"

# define SPEW 0

using namespace Minotaur;

PerspCon::PerspCon()
:env_(EnvPtr()),p_(ProblemPtr()), cList_(0), binVar_(0), lbc_(0), ubc_(0), 
  l_(0), u_(0) 
{
}


PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env),p_(p), cList_(0), binVar_(0), lbc_(0), ubc_(0), l_(0), u_(0)
{
  logger_ = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("handler_log_level")->getValue());
}


bool PerspCon::checkAllVars(ConstConstraintPtr cons, ConstVariablePtr binvar)
{
  bool varbounded;
  ConstVariablePtr var;
  const FunctionPtr f = cons->getFunction();
  
  for (VarSetConstIterator it=f->varsBegin();it!=f->varsEnd();
       ++it) {
    var = *it;
    varbounded = checkVarBounds(var, binvar);
    if (varbounded == false ) {
     return false;
    } 
  }
  return true; 
}


bool PerspCon::checkNVars(ConstConstraintPtr cons, ConstVariablePtr binvar,
                          bool initvar)
{  
  bool varbounded;
  ConstVariablePtr var;
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  
  if(initvar == false){
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it ) {
      var = *it;
      varbounded = checkVarBounds(var, binvar);
      if (varbounded == false) {
        return false;
      }
    }
    return true;
  } else {
    VarSetConstIterator it=nlf->varsBegin();
    for (it++; it!=nlf->varsEnd(); ++it ) {
      var = *it;
      varbounded = checkVarBounds(var, binvar);
      if (varbounded == false) {
        return false;
      }
    }
    return true;
  }  
}


bool PerspCon::checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar, bool pi)
{
  UInt numvars;
  double coeffvar, coeffbin;
  bool vub = false, vuub = false, vlb = false, vllb = false;

  FunctionType type;
  ConstConstraintPtr c;
  ConstConstraintPtr lbcons = (ConstConstraintPtr) new Constraint();
  ConstConstraintPtr ubcons = (ConstConstraintPtr) new Constraint();
  
  for (ConstrSet::iterator it= var->consBegin(); it!= var->consEnd(); ++it) {
    c = *it;
    const FunctionPtr f = c->getFunction();
    type = c->getFunctionType();
  
    if (type != Linear) {
      continue;
    } 
    
    numvars = f->getNumVars();
    const LinearFunctionPtr lf = c->getLinearFunction();
    coeffvar = lf->getWeight(var);
  
    if (numvars > 2 || coeffvar == 0) {
      continue;
    }

    if (numvars == 1){
      if ((coeffvar > 0 && c->getUb() ==0) ||
        (coeffvar < 0 && c->getLb() ==0)  ){
        vuub = true;
        ubcons = c;

      } else if ((coeffvar > 0 && c->getLb() ==0) ||
        (coeffvar < 0 && c->getUb() ==0)  ){
        vllb = true;      
        lbcons = c;
      } else {
        continue;
      }
    } 
    
    coeffbin = lf->getWeight(binvar);
  
    if (coeffbin==0) {
      continue;
    }
  
   if (vlb == false) {
     if (coeffvar < 0 && c->getUb() == 0) {
       vlb = true;
       lbcons = c;
     }         
     if (coeffvar > 0 && c->getLb() == 0) {
       vlb = true;
       lbcons = c;
     } 
   }

   if (vub == false) {
     if (coeffvar < 0 && coeffbin > 0 && c->getLb() == 0) {
       vub = true;
       ubcons = c;
     } 
     if (coeffvar > 0 && coeffbin < 0 && c->getUb() == 0) {
       vub = true;
       ubcons = c;
     }
   }
  
   if (vlb ==true && vub == true) {
     break;
   }  
  }

  if (vlb == 0 && (var->getLb() == 0 || vllb == 1) ){
     vlb = true;
   }
 
  if (vub == 0 && (var->getUb() == 0 || vuub == 1) ){
     vub = true;
   }

  if ( (vub == true) && (vlb == true) ) {
    if (pi == 1) {
      l_.push_back(lbcons->getName());
      u_.push_back(ubcons->getName());
    } else {
      initl_.push_back(lbcons->getName());
      initu_.push_back(ubcons->getName());
    } 
    return true;
  } else {
     return false;
  }
}


bool PerspCon::checkVarTypes(ConstConstraintPtr cons, ConstVariablePtr& binvar)
{
  UInt numbins = 0;
  VariableType type;
  ConstVariablePtr var;
  
  const FunctionPtr f = cons->getFunction();
  const LinearFunctionPtr lf = cons->getLinearFunction();
  QuadraticFunctionPtr qf = cons->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();

  for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    var = *it;
    type = var->getType();
  
    switch (type) {
    case Binary:
      if (lf != NULL) {
        if (lf->hasVar(var)) {
          if (lf->getWeight(var)!=0) {
            binvar = var;
            numbins += 1; 
          }
          if (numbins >= 2) {
            return false;
          } 
        }
      } else if (qf != NULL) {
        if (qf->hasVar(var)) {
          return false;
        } 
      } else if (nlf != NULL) {
          if (nlf->hasVar(var)) {
            return false;
          } 
      }
      break;
    case ImplBin:
      if (lf != NULL) {
        if (lf->hasVar(var)) {
          if (lf->getWeight(var)!=0) {
            binvar = var;
            numbins += 1; 
          }
          if (numbins >= 2) {
            return false;
          } 
        }
      } else if (qf != NULL) {
        if (qf->hasVar(var)) {
          return false;
        } 
      } else if (nlf != NULL) {
          if (nlf->hasVar(var)) {
            return false;
          } 
      }
      break;
    case Integer:
      if ( (var->getLb() == 0) && (var->getUb() == 1) ) {
        if (lf != NULL) {
          if (lf->hasVar(var)) {
            if (lf->getWeight(var)!=0) {
              binvar = var;
              numbins += 1; 
            }
            if (numbins >= 2) {
              return false;
            } 
          }
        } else if (qf != NULL) {
          if (qf->hasVar(var)) {
            return false;
          } 
        } else if (nlf != NULL) {
          if (nlf->hasVar(var)) {
            return false;
          } 
        }
      } else {
        // Var is a general integer variable thus constraint not considered
        // for PR
        return false;
      }
      break;
    case Continuous: /* Do nothing.*/ 
      break;
    default:
      // Unexpected variable.
      return false;
    }
  }
  // Either all variables are continuous or there is only one binary 
  // variable and remaining continuous in the constraint
  return true;
}


void PerspCon::displayInfo(const std::string m)
{
  UInt s = cList_.size();
  std::ostream &out = logger_->msgStream(LogDebug);

  if (s >0) {
    out << m <<"Total no. of constraints amenable to PR: " << s << std::endl;
    out << m << "Details of constraints amenable to perspective reformulation:" 
      << std::endl; 
    out << "------------------------------" << std::endl;

    for (UInt i= 0; i < cList_.size() ; ++i) {
      out << i+1 << ". ";
      cList_[i]->write(out);
      out << "Binary variable is: " << binVar_[i]->getName() << std::endl;
      out << "Name of Lower bounding constraint: " ;

      for (UInt j=0; j < lbc_[i].size(); ++j) {
        if (j < lbc_[i].size()-1){
          out << lbc_[i][j] << ", ";
        } else {
          out << lbc_[i][j];
        }
      }

      out << "\nName of Upper bounding constraint: ";

      for (UInt j=0; j < ubc_[i].size(); ++j) {
        if (j < ubc_[i].size()-1){
          out << ubc_[i][j] << ", ";
        } else {
          out << ubc_[i][j];
        }
      }
      out << "\nNote: Absence of lower/upper bounding constraint name for any "
        <<"means non-negativity/non-positivility constraint on that variable.";
      out << std::endl << "------------------------------" << std::endl;
    }
  } else {
    out << m <<"Problem does not have constraints amenable to PR."<< std::endl;
  }
}


bool PerspCon::evalConstraint(ConstConstraintPtr cons, VariablePtr& binvar)
{
  FunctionType type;
  type = cons->getFunctionType();
  double cl = cons->getLb(), cu = cons->getUb();

  // Consider the nonlinear constraint only if its upper bound is nonnegative or 
  // lower bound nonpositive
  if (cl > 0 || cu < 0 || type == Linear){
    return false;
  } 

  // vartypeok = false, if no. of binary variables in the constraint is greater
  // than 1, or binary variable is in the nonlinear part of the constraint.
  // binvar stores the binary variable in the constraint.
  bool vartypeok = checkVarTypes(cons, binvar);
  if (vartypeok ==  false) {
    return false;
  }
 
  bool boundsok = false;

  if (binvar == NULL) {
    const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
    VarSetPtr binaries = (VarSetPtr) new VarSet();
    // We select first variable of the nonlinear part of the constraint for
    // generating list of probable binary variables that may control remaining
    // continuous variables in the constraint.
    ConstVariablePtr initvar = *(nlf->varsBegin());
    // Set binaries contain those binary variables that control variable initvar
    initialBinary(initvar, binaries);

    if (binaries->size() == 0) {
      return false;
    }

    UInt index = 0;

    for (VarSetConstIterator it= binaries->begin(); it!=binaries->end(); ++it, index++) {
      binvar = *it;
      // Check if at least one binary varible in set binaries control rest of
      // the continuous variables in the nonlinear part of the constraint
      //boundsok = checkNVars(cons, binvar, initvar);

      //if (boundsok == true) {
        //if(cons->getLinearFunction()) {
          //if (cons->getLinearFunction()->getNumTerms() > 1 && 
              //((cu > 0 && cu != +INFINITY ) || (cl < 0 && cl != -INFINITY))) {
            //boundsok = checkLVars(cons, binvar);
            //if (boundsok == true) {
              //l_.push_back(initl_[index]);
              //u_.push_back(initu_[index]);
              //return true;
            //}  
          //}
        //} else {
          //l_.push_back(initl_[index]);
          //u_.push_back(initu_[index]);
          //return true;
        //}
      //}
      
      if(cu==0 || cl==0){
          boundsok = checkNVars(cons, binvar, 1);
      } else {
        if(cons->getLinearFunction() && 
           cons->getLinearFunction()->getNumTerms() > 1 ) {
            boundsok = checkAllVars(cons, binvar);
        } else {
          boundsok = checkNVars(cons, binvar, 1);
        }
      }
      if (boundsok == true) {
        l_.push_back(initl_[index]);
        u_.push_back(initu_[index]);
        return true;
      }     
    }
    return false;
  } else {
    //boundsok = checkNVars(cons, binvar);

    //if (boundsok == true) {
      //if(cons->getLinearFunction()){
        //if (cons->getLinearFunction()->getNumTerms() > 1 && 
            //((cu > 0 && cu != +INFINITY ) || (cl < 0 && cl != -INFINITY))) {
          //boundsok = checkLVars(cons, binvar);
          //if (boundsok == true) {
            //return true;
          //} else {
            //return false;
          //}  
        //}
      //} else {
        //return true;
      //}
    //} else {
      //return false;
    //}

    if(cu==0 || cl==0){
      boundsok = checkNVars(cons, binvar);
    } else {
      if(cons->getLinearFunction() && 
         cons->getLinearFunction()->getNumTerms() > 1 ) {
        boundsok = checkAllVars(cons, binvar);
      } else {
        boundsok = checkNVars(cons, binvar);
      }
    }
    if (boundsok == true) {
      return true;
    } else {
      return false;
    }
  }
  return true;
}


void PerspCon::generateList()
{
  ConstConstraintPtr cons;
  bool isPR =  false;

  for (ConstraintConstIterator it= p_->consBegin(); it!= p_->consEnd(); ++it) {
    cons = *it;
    VariablePtr binvar; 
    isPR = evalConstraint(cons, binvar);

    if (isPR) {
      cList_.push_back(cons);
      binVar_.push_back(binvar);
      lbc_.push_back(l_);
      ubc_.push_back(u_);
    }
    l_.clear();
    initl_.clear();
    u_.clear();
    initu_.clear();
  }
}


bool PerspCon::getStatus()
{
  if (cList_.size() > 0){
    return true; 
  } else {
    return false;
  }
}


void PerspCon::initialBinary(ConstVariablePtr var, VarSetPtr binaries)
{
  UInt numvars;
  double varlb, varub;
  bool boundsok = false, newvar;
  
  FunctionType type; 
  VariablePtr binvar; 
  VariableType vartype;
  ConstConstraintPtr cons;

  for (ConstrSet::const_iterator it= var->consBegin(); it!=var->consEnd(); ++it) {
    cons = *it;
    type = cons->getFunctionType();
    const FunctionPtr f = cons->getFunction();

    if (type != Linear) {
      continue;
    }

    numvars = f->getNumVars();
    const LinearFunctionPtr lf = cons->getLinearFunction();

    if (numvars != 2 || lf->getWeight(var) == 0 || 
        (cons->getLb()!=0 && cons->getUb()!=0)) {
      continue;
    } 
    
    ConstVariablePtr curvar;
    
    for (VariableGroupConstIterator itvar=lf->termsBegin(); itvar!=lf->termsEnd(); ++itvar) {
      curvar = itvar->first;
      vartype = curvar->getType();
    
      if (vartype == Continuous) {
        continue;
      }
      if (lf->getWeight(curvar) != 0) {
        if (vartype == Binary || vartype == ImplBin) {
          newvar = true;
          for (VarSetConstIterator itv= binaries->begin(); itv!=binaries->end(); ++itv) {
            binvar = *itv;
            if (curvar == binvar){
              newvar = false;
              break;
            }
          }
          if (newvar) {
            boundsok = checkVarBounds(var, curvar,0);
              if (boundsok){
                binaries->insert(curvar);
              } 
          }
        } else if (vartype == Integer) {
          varlb = curvar->getLb();
          varub = curvar->getUb();
          if ( (varlb == 0) && (varub==1) ) {
            newvar = true;
            for (VarSetConstIterator itv= binaries->begin(); itv!=binaries->end(); ++itv) {
              binvar = *itv;
              if (curvar == binvar){
                newvar = false;
                break;
              }
            }
            if (newvar) {
              boundsok = checkVarBounds(var, curvar,0);
                if (boundsok){
                  binaries->insert(curvar);
                } 
            }
          }
        } else{
          continue;
        }
      }
    }    
  } 
}


PerspCon::~PerspCon()
{
  env_.reset();
  p_.reset();
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
