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

using namespace Minotaur;

PerspCon::PerspCon()
:env_(EnvPtr()),p_(ProblemPtr()), cList_(0), binVar_(0), lbc_(0),
  ubc_(0), l_(0), u_(0), st_(0) 
{
}


PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env),p_(p), cList_(0), binVar_(0), lbc_(0), ubc_(0), l_(0),
  u_(0), st_(0)
{
  logger_ = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("handler_log_level")->getValue());
}


bool PerspCon::checkAllVars(ConstConstraintPtr cons, ConstVariablePtr binvar, 
                            ConstVariablePtr initvar, UInt index)
{
  bool varbounded;
  ConstVariablePtr var;
  const FunctionPtr f = cons->getFunction();
  
   if(initvar){   
    for (VarSetConstIterator it=f->varsBegin();it!=f->varsEnd(); ++it) {
      var = *it;
      if (var->getIndex() == initvar->getIndex()){
         l_.push_back(initl_[index]);
         u_.push_back(initu_[index]);
       } else if (var->getIndex() == binvar->getIndex()){
         continue;
       } else {
         varbounded = checkVarBounds(var, binvar);
         if (!varbounded) {
           l_.clear();
           u_.clear();
           return false;
         }
       }
    }
   } else {
     for (VarSetConstIterator it=f->varsBegin();it!=f->varsEnd(); ++it) {
       var = *it;
        if (var->getIndex() == binvar->getIndex()){
          continue;
        } else {
          varbounded = checkVarBounds(var, binvar);
          if (!varbounded) {
            l_.clear();
            u_.clear();
            return false;
          }
        }
      }
   } 
   return true;
}


bool PerspCon::checkNVars(ConstConstraintPtr cons, ConstVariablePtr binvar)
{  
  bool varbounded;
  ConstVariablePtr var;
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  
  VarSetConstIterator it=nlf->varsBegin();
  for (it++; it!=nlf->varsEnd(); ++it) {
    var = *it;
    varbounded = checkVarBounds(var, binvar);
    if (!varbounded) {
      l_.clear();
      u_.clear();
      return false;
    }
  }
  return true;
}


bool PerspCon::checkLVars(ConstConstraintPtr cons, ConstVariablePtr binvar)
{  
  bool varbounded;
  ConstVariablePtr var;
  const LinearFunctionPtr lf = cons->getLinearFunction();
  if(lf){
    for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd(); ++it) {
      var = it->first;
      if(var->getIndex() != binvar->getIndex()){
        varbounded = checkVarBounds(var, binvar);
        if (!varbounded) {
          return false;
        }
      }
    }
  } 
    return true;
}



bool PerspCon::checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar, bool pi)
{
  UInt numvars;
  double coeffvar, coeffbin;
  bool vub = false, vlb = false;

  FunctionType type;
  ConstConstraintPtr c;
  ConstConstraintPtr lbc = (ConstConstraintPtr) new Constraint();
  ConstConstraintPtr ubc = (ConstConstraintPtr) new Constraint();

  if(var->getLb() == 0){
    vlb = true;  
  }

  if(var->getUb() == 0){
    vub = true;  
  }

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
        (coeffvar < 0 && c->getLb() ==0)){
        vub = true;
        ubc = c;

      } else if ((coeffvar > 0 && c->getLb() ==0) ||
        (coeffvar < 0 && c->getUb() ==0)  ){
        vlb = true;      
        lbc = c;
      } else {
        continue;
      }
    } else {
      coeffbin = lf->getWeight(binvar);
      if (coeffbin==0) {
        continue;
      }
      
      if (vlb == false) {
        if ((coeffvar < 0 && c->getUb() == 0) ||
            (coeffvar > 0 && c->getLb() == 0)) {
          vlb = true;
          lbc = c;
        }         
      }
      if (vub == false) {
        if ((coeffvar < 0 && c->getLb() == 0) ||
            (coeffvar > 0 && c->getUb() == 0)) {         
          vub = true;
          ubc = c;
        } 
      }
    }
   if ((vub == true) && (vlb == true)) {
     if (pi) {
       l_.push_back(lbc->getName());
       u_.push_back(ubc->getName());
     } else {
       initl_.push_back(lbc->getName());
       initu_.push_back(ubc->getName());
     } 
     return true;
   } 
  }
  return false;
}



bool PerspCon::checkVarTypes(ConstConstraintPtr cons, ConstVariablePtr& binvar, bool indi)
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
    case ImplBin:
      if(indi){
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
        }
      }
      if (qf != NULL) {
        if (qf->hasVar(var)) {
          return false;
        } 
      } 
      if (nlf != NULL) {
          if (nlf->hasVar(var)) {
            return false;
          } 
      }
      break;
    case Integer:
      if ((var->getLb() == 0) && (var->getUb() == 1)) {
        if(indi){
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
          }
        }
        if (qf != NULL) {
          if (qf->hasVar(var)) {
            return false;
          } 
        }
        if (nlf != NULL) {
          if (nlf->hasVar(var)) {
            return false;
          } 
        }
      } 
      break;
    case Continuous: /* Do nothing.*/ 
      break;
    default:
      // Unexpected variable.
      return false;
    }
  }
  return true;
}


void PerspCon::displayInfo(const std::string m)
{
  UInt s = cList_.size();
  std::ostream &out = logger_->msgStream(LogInfo);
  std::ostream &out1 = logger_->msgStream(LogExtraInfo);

  if (s > 0) {
    out << m <<"Total no. of constraints amenable to PR: " << s << std::endl;
    out1 << m << "Details of constraints amenable to perspective reformulation:" 
      << std::endl; 
    out1 << "------------------------------" << std::endl;

    for (UInt i= 0; i < cList_.size() ; ++i) {
      out1 << i+1 << ". ";
      cList_[i]->write(out1);
      out1 << "Associated binary variable: " << binVar_[i]->getName() << std::endl;
      out1 << "Structure type: S" << sType_[i] << std::endl;
      out1 << "Name of Lower bounding constraint: " ;


      // Take care of the multiple commas when entry is not there
      for (UInt j=0; j < lbc_[i].size(); ++j) {
        if (j < lbc_[i].size()-1){
          out1 << lbc_[i][j] << ", ";
        } else {
          out1 << lbc_[i][j];
        }
      }

      out1 << "\nName of Upper bounding constraint: ";

      for (UInt j=0; j < ubc_[i].size(); ++j) {
        if (j < ubc_[i].size()-1){
          out1 << ubc_[i][j] << ", ";
        } else {
          out1 << ubc_[i][j];
        }
      }
     out1 << std::endl << "------------------------------" << std::endl;
    }
    out1 << "\nNotes (1) Absence of lower/upper bounding constraint name for any variable"
      <<" means non-negativity/non-positivility constraint on that variable.";
    out1 << "\n (2) Description of S1 and S2 are in paper......";
 
  } else {
    out << m <<"Problem instance does not have constraints amenable to PR."<< std::endl;
  }
}


bool PerspCon::evalConstraint(ConstConstraintPtr cons, VariablePtr& binvar)
{
  FunctionType type;
  type = cons->getFunctionType();
  double cu = cons->getUb();
  //double cl = cons->getLb(), cu = cons->getUb();
  
  if (type == Linear){
    return false;
  } 

  //act = f->eval(x, &error);
  //if (act > cu){
    //assert(!"PerspCutHandler: problem is infeasible");
    //// Add more information to this ifeasibility in printing
    //return false;
  //}

  if (cu <= 0){
    // vartypeok = false, if a binary variable is in the nonlinear part of the 
    // constraint.
    bool vartypeok = checkVarTypes(cons, binvar);
    if (!vartypeok) {
      return false;
    }
    
  } else {
    // vartypeok = false, if no. of binary variables in the constraint is greater
    // than 1, or a binary variable is in the nonlinear part of the constraint.
    // binvar stores the binary variable in the constraint.
    bool vartypeok = checkVarTypes(cons, binvar, 1);
    if (!vartypeok) {
      return false;
    }
  }

  bool boundsok = false, lboundsok = false;
  if (binvar == NULL) {
    const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
    VarSetPtr binaries = (VarSetPtr) new VarSet();
    // We select first variable of the nonlinear part of the constraint for
    // generating a list of binary variables that may control other
    // variables in the constraint.
    ConstVariablePtr initvar = *(nlf->varsBegin());
    // Set binaries contains binary variables controling initvar
    initialBinary(initvar, binaries);

    if (binaries->size() == 0) {
      return false;
    }

    UInt index = 0;

    int error = 0;
    double act =0;
    double initsol [p_->getNumVars()] = {0}; 
    const double *x = initsol;
    FunctionPtr f = cons->getFunction();

    for (VarSetConstIterator it= binaries->begin(); it!=binaries->end(); ++it, index++) {
      binvar = *it;
      //if(cu==0 || cl==0)
      if(cu <=0){
        l_.push_back(initl_[index]);
        u_.push_back(initu_[index]);
        boundsok = checkNVars(cons, binvar);
        lboundsok = checkLVars(cons, binvar);
        if(boundsok == true && lboundsok == true){
          act = f->eval(x, &error);
          if (act > cu){
            assert(!"PerspCutHandler: problem is infeasible");
            return false;
          } else {
            st_ = 1;
            return true;
          }
        } else if(boundsok){
          st_ = 2;
          return true;
        }
      } else {
          boundsok = checkAllVars(cons, binvar, initvar, index);
          if(boundsok){
            act = f->eval(x, &error);
            if (act > cu){
              assert(!"PerspCutHandler: problem is infeasible");
              return false;
            } else {
              st_ = 1;
              return true;
            }
          }
      }
    }
    return false;
  } else {
    boundsok = checkAllVars(cons, binvar, VariablePtr());
    if(boundsok){
      st_ = 1;
      // Make st_ char
    }
    return boundsok;
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
      sType_.push_back(st_);
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
        if ((vartype == Binary) || (vartype == ImplBin) ||
            ((vartype = Integer) && (curvar->getLb() == 0) && (curvar->getUb()==1))) {
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
        //} else if (vartype == Integer) {
          //if ((curvar->getLb() == 0) && (curvar->getUb()==1) ) {
            //newvar = true;
            //for (VarSetConstIterator itv= binaries->begin(); itv!=binaries->end(); ++itv) {
              //binvar = *itv;
              //if (curvar == binvar){
                //newvar = false;
                //break;
              //}
            //}
            //if (newvar) {
              //boundsok = checkVarBounds(var, curvar,0);
                //if (boundsok){
                  //binaries->insert(curvar);
                //} 
            //}
          //}
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
  logger_.reset();
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
