//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file PerspCon.cpp
 * \Define base class that determines constraints amenable to perspective
 * reformulation (PR).
 * \Author Meenarli Sharma, India Institute of Technology  Bombay
 */

#include <iostream>
using std::endl;

#include "PerspCon.h"

#include "Function.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "NonlinearFunction.h"
#include "Option.h"
#include "QuadraticFunction.h"

//# define SPEW 0

using namespace Minotaur;
const std::string PerspCon::me_ = "QGHandler: ";

/*********************************************************************************/
// Functionality for identifying structure (3) f(x) <=b or f(x,z) <=b (4)or 
// f(x,z)+g(y) <= b (8)or f(x)+g(y) <=b (7) and  lz<=x<=uz, z is binary variable and 
// x, y are continuous variables.
/*********************************************************************************/

PerspCon::PerspCon()
:env_(EnvPtr()),p_(ProblemPtr()), cList_(0), binVar_(0), lbc_(0), ubc_(0), 
  l_(0), u_(0) /*, sType_(0)*/
{
}


PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env),p_(p), cList_(0), binVar_(0), lbc_(0), ubc_(0), l_(0), u_(0)
/*  , sType_(0)*/
{
  logger_ = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("handler_log_level")->getValue());
}


bool PerspCon::checkLVars(ConstConstraintPtr cons, ConstVariablePtr binvar)
{
  ConstVariablePtr var;
  bool varbounded;
  const LinearFunctionPtr lf = cons->getLinearFunction();
  // Do not consider binvar 
  if (lf) {
    for (VariableGroupConstIterator it=lf->termsBegin();it!=lf->termsEnd();
         ++it) {
      var = it->first;
      if (it->first == binvar) {
        continue;
      }
      // Check if a variable of linear function is controlled by binvar
      varbounded = checkVarBounds(var, binvar);
      // If variable is not bounded, then constraint is not a perspective constraint.
      if (varbounded == false && cons->getUb()<=0) {
        //s_ += 7;
        return true;
      }
      if (varbounded == false && cons->getUb() > 0) {
        return false;
      }
    }

    if ( cons->getUb() >= 0){
      //s_ += 3;
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}


bool PerspCon::checkNVars(const NonlinearFunctionPtr nlf, ConstVariablePtr binvar)
{  
  ConstVariablePtr var;
  bool varbounded;
  // Check if variables in the nonlinear part of the constraint is controlled 
  // by binvar
  for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it ) {
    var = *it;
    varbounded = checkVarBounds(var, binvar);
    // If variable is not bounded, then constraint is not a perspective constraint.
    if (varbounded == false) {
      return false;
    }
  }
  return true;
}


bool PerspCon::checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar)
{
  // Shows if variable is upper bounded.
  bool vub = false;
  // Shows if variable is lower bounded.
  bool vlb = false;

  // If any bound is assigned from variable bounds, 
  // corresponding bound constraint will be uninitialized as below.
  // Lb bounding constraint.
  ConstConstraintPtr lbcons = (ConstConstraintPtr) new Constraint();
  // Ub bounding constraint.
  ConstConstraintPtr ubcons = (ConstConstraintPtr) new Constraint();
  ConstConstraintPtr c;
  FunctionType type;
  double coeffvar;
  double coeffbin;
  UInt numvars;
  // Iterate through each constraint in which variable var is appearing
  for (ConstrSet::iterator it= var->consBegin(); it!= var->consEnd(); ++it) {
    c = *it;
    // Function of constraint.
    const FunctionPtr f = c->getFunction();
    // Type of constraint.
    type = c->getFunctionType();
    // Consider only linear constraints.
    if (type != Linear) {
      continue;
    } 
    // Number of variables in the constraint should be two
    // and one of them is current variable and the other one is binvar.
    numvars = f->getNumVars();
    if (numvars != 2) {
      continue;
    }
    // Get linear function.
    const LinearFunctionPtr lf = c->getLinearFunction();

    // Coefficient of variable.
    coeffvar = lf->getWeight(var);
    // Coefficient of binary variable.
    coeffbin = lf->getWeight(binvar);
    //If this binvar or var is not in the constraint then move to next constraint
    if (coeffbin==0 || coeffvar==0) {
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
     if (coeffvar < 0 && c->getLb() == 0) {
       vub = true;
       ubcons = c;
     } 
     if (coeffvar > 0 && c->getUb() == 0) {
       vub = true;
       ubcons = c;
     }
   }
  
   if (vlb ==true && vub == true) {
     break;
   }  
  } // end of for loop to consider all constraints that has the variable.

  if (vlb == 0 && var->getLb() == 0){
     vlb = true;
   }
 
  // If variable is both bounded from up and down from binary variable, then
  // it is bounded by the binary variable.
  if ( (vub == true) && (vlb == true) ) {
    l_.push_back(lbcons->getName());
    u_.push_back(ubcons->getName());
    return true;
  }
  // If it comes here, variable is not bounded by binary variable.
  return false;
}


bool PerspCon::checkVarTypes(ConstConstraintPtr cons, ConstVariablePtr& binvar)
{
  // Current variable considered.
  ConstVariablePtr var;
  // Type of variable considered.
  VariableType type;
  // Number of binary variables.
  UInt numbins = 0;
  // Get function of constraint.
  const FunctionPtr f = cons->getFunction();
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  const LinearFunctionPtr lf = cons->getLinearFunction();
  QuadraticFunctionPtr qf = cons->getQuadraticFunction();

  // Iterate through all variables.
  for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    var = *it;
    type = var->getType();
    // Check the type of variable.
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
      // If number of binary variables is more than one
      // Do not consider constraint for perspective cuts generation.
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
      // If number of binary variables is more than one
      // Do not consider constraint for perspective cuts generation.
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
        // It is a general variable
        // Do not consider constraint.
        return false;
      }
      break;
    case Continuous: /* Do nothing.*/ 
      break;
    default:
      // If it comes to here, we have a variable which was not expected 
      // when algorithm was designed.
      return false;
    }
  }
  // If it comes here, it means all variables are continuous 
  // or there exists only one binary variable. 
  return true;
}


void PerspCon::displayInfo_()
{
  std::ostream& out = logger_->msgStream(LogDebug);
  for (UInt i= 0; i < cList_.size() ; ++i) {
    out << me_ << "Perspective constraint is: " << std::endl;
    cList_[i]->write(out);
    out << me_ << "Binary variable is: " << binVar_[i]->getName() << std::endl;
    out << me_ << "Name of Lower bounding constraint: " ;
    for (UInt j=0; j < lbc_[i].size(); ++j) {
      out << lbc_[i][j] << ", ";
    }

    out << me_ << "\nName of Upper bounding constraint: ";
    for (UInt j=0; j < ubc_[i].size(); ++j) {
      out << ubc_[i][j] << ", ";
    }
    //out << me_ << "\nStructure of the constraint: " << sType_[i];
    out << std::endl << "------------------------------" << std::endl;
  }
  out << me_ << "Total no. of constraints amenable to PR " << cList_.size()
      << std::endl;
}


bool PerspCon::evalConstraint(ConstConstraintPtr cons, VariablePtr& binvar)
{
  FunctionType type;
  // Function type of constraint.
  type = cons->getFunctionType();
  // Do not consider linear constraints.
  if (type == Linear) {
    return false;
  }
  //Get function of constraint.
  //const FunctionPtr f = cons->getFunction(); 
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction(); 
  // Add one more parameter that stores the binary variable.
  // vartypeok = true, if no. of binary variable in constraint is 
  // less than or equal to 1, and that binary variable is in the linear part
  // of the constranit cons
  bool vartypeok = checkVarTypes(cons, binvar);
  if (vartypeok ==  false) {
    return false;
  }
 
  // Check which cont. variables of the function are bounded by binary.
  bool boundsok = false;
  if (binvar == NULL) {
    //s_ = 0;
    VarSetPtr binaries = (VarSetPtr) new VarSet();
    // Select first variable of the nonlinear part of the
    // constraint for initial binary search.
    ConstVariablePtr initvar = *(nlf->varsBegin());
    initialBinary(initvar, binaries);

    // If there is no binary controlling the select variable
    // then stop.
    if (binaries->size() == 0) {
      return false;
    }
    for (VarSetConstIterator it= binaries->begin(); it!=binaries->end(); ++it) {
      binvar = *it;
      // Check at least one binary varible that is controlling initvar is
      // controlling rest of the variables in the nonlinear part of the
      // constriant function
      boundsok = checkNVars(nlf, binvar);
      if (boundsok == true) {
        boundsok = checkLVars(cons, binvar);
        if (boundsok == true) {
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
  } else {
    // Check if each variable in the nonlinear part of the constraint 
    // is bounded by binary variable binvar.
    //s_ = 1; 
    boundsok = checkNVars(nlf, binvar);
    if (boundsok == true) {
      boundsok = checkLVars(cons, binvar);
      if (boundsok == true) {
        return true;
       } else {
         return false;       
       }
    }  else {
      return false;    
    }
  }

  // Control never comes here.
  return true;
}


void PerspCon::generateList()
{
  // Pointer to current constraint being checked.
  ConstConstraintPtr cons;
  // Shows if constraint amenable to PR.
  bool ispersp =  false;
  // Iterate through each constraint.
  for (ConstraintConstIterator it= p_->consBegin(); it!= p_->consEnd(); ++it) {
    cons = *it;
    // Binary variable 
    VariablePtr binvar; 
    ispersp = evalConstraint(cons, binvar);
    if (ispersp) {
      cList_.push_back(cons);
      // Vector of binary variables of PR constraints
      binVar_.push_back(binvar);
      lbc_.push_back(l_);
      ubc_.push_back(u_);
      //sType_.push_back(s_);
    }
    l_.clear();
    u_.clear();
  }
#if SPEW 
  displayInfo_();
#endif
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
  // Set binaries contain those binary variables that appear in a linear
  // constraint (involving only two terms)  with variable var
  ConstConstraintPtr cons;
  FunctionType type; 
  UInt numvars;
  // Iterate through each constraint in which variable var appear.
  for (ConstrSet::const_iterator it= var->consBegin(); it!=var->consEnd(); ++it) {
    cons = *it;
    // Function of constraint.
    const FunctionPtr f = cons->getFunction();
    // Type of function.
    type = cons->getFunctionType();
    // Only consider linear constraints.
    if (type != Linear) {
      continue;
    }

    // Number of variables should be two.
    numvars = f->getNumVars();
    if (numvars != 2) {
      continue;
    }
    // Get linear function.
    const LinearFunctionPtr lf = cons->getLinearFunction();

    if (lf->getWeight(var) == 0) {
      continue;    
    }
    // Check if the other variable is binary.
    ConstVariablePtr curvar;
    VariableType vartype;
    double varlb;
    double varub;
    for (VariableGroupConstIterator itvar=lf->termsBegin(); itvar!=lf->termsEnd(); ++itvar) {
      curvar = itvar->first;
      vartype = curvar->getType();
      if (vartype == Continuous) {
        continue;
      }
      if (lf->getWeight(curvar) != 0) {
        if (vartype == Binary || vartype == ImplBin) {
          binaries->insert(curvar);
        }
        if (vartype == Integer) {
          varlb = curvar->getLb();
          varub = curvar->getUb();
          if ( (varlb == 0) && (varub==1) ) {
            binaries->insert(curvar);
          }
        }
      }
    } // end of for for variables.
  } // end of for loop.
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
