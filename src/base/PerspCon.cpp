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
#include <algorithm>

#include "Option.h"
#include "Logger.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"

using namespace Minotaur;

const std::string PerspCon::me_ = "PerspCon: ";
//MS: assume constraint form considered is f(x)<=b
PerspCon::PerspCon()
:env_(EnvPtr()), p_(ProblemPtr()), sType_(0), cons_(0), binvar_(0),
  nlVarFixVal_(0), lVarFixVal_(0) 
{

}


PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env), p_(p), sType_(0), cons_(0), binvar_(0), nlVarFixVal_(0),
  lVarFixVal_(0) 
{
  logger_ = env->getLogger();
}


void PerspCon::populate (ConstraintPtr cons, VariablePtr binvar, 
               VariableGroup nlVarFixVal, VariableGroup lVarFixVal)
{
  cons_.push_back(cons);
  binvar_.push_back(binvar);
  lVarFixVal_.push_back(lVarFixVal);
  nlVarFixVal_.push_back(nlVarFixVal);
  return;
}


bool PerspCon::boundBinVar(ConstraintPtr cons, VariablePtr& binvar)
{
  int error = 0;
  double cu, act;
  ConstVariablePtr var;
  bool nboundsok = false, lboundsok;
  VariableGroup lVarFixVal, nlVarFixVal;
  
  checkNVars(cons, binvar, &nboundsok, &nlVarFixVal);
  if (nboundsok==false) {
    nlVarFixVal.clear();
    return false;
  }
  LinearFunctionPtr lf = cons->getLinearFunction();
  if (lf) {
    checkLVars(cons, binvar, &lboundsok, &lVarFixVal, &nlVarFixVal);
  } else {
    lboundsok = true;  
  }

  double *y = new double[p_->getNumVars()];  
  const QuadraticFunctionPtr qf = cons->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  double absTol = env_->getOptions()->findDouble("solAbs_tol")->getValue();

  std::fill(y, y + p_->getNumVars(), 0);
  for (VariableGroupConstIterator it=nlVarFixVal.begin(); it!=nlVarFixVal.end();
       ++it) {
    y[it->first->getIndex()] = it->second;
  }
  cu = cons->getUb();
  if (nboundsok == true && lboundsok == true) {
    FunctionPtr cf = cons->getFunction();
    for (VariableGroupConstIterator it=lVarFixVal.begin();
         it!=lVarFixVal.end(); ++it) {
      y[it->first->getIndex()] = it->second;
    }
    act = cf->eval(y, &error);
    if (error == 0) {    
      if (cu+absTol < act) {  
        binvar->setLb_(1);
      } else {
        populate(cons, binvar, nlVarFixVal, lVarFixVal);
        if (lf) {
          if (lf->hasVar(binvar)) {
            if (lf->getWeight(binvar) > 0) {          
              if (act <= cu-lf->getWeight(binvar)) {
                //MS: modify constraint here; Merge different types later
                cons->setUb_(cu-lf->getWeight(binvar));
                lf->removeVar(binvar,0);
                binvar->outOfConstraint_(cons);
                sType_.push_back(4);  
              } else {
                sType_.push_back(-2);  
              }
            } else {
              sType_.push_back(3);  
            }
          } else {
            sType_.push_back(4);  
          }
        } else {
          sType_.push_back(4);  
        }
        delete [] y;
        return true;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "Constraint is not defined at this point" << std::endl;
      delete [] y;
      return false;
    }
  } else if (nboundsok == true) {
    if (nlf) {
      act = nlf->eval(y, &error);
    } else if (qf) {
      act = qf->eval(y);
    }
    if (error == 0) {    
      if (act >= cu) {
        populate(cons, binvar, nlVarFixVal, lVarFixVal);
        if (lf->hasVar(binvar) == false) {
          sType_.push_back(1);  
        } else {
          if (lf->getWeight(binvar) > 0) {          
            sType_.push_back(-1);  
          } else {
            sType_.push_back(0);  
          } // MS: To be verified after writing.....
        }       
        delete [] y;
        return true;
      } 
    } else {
     logger_->msgStream(LogError) << me_ 
       << "Constraint is not defined at this point" << std::endl;
     delete [] y;
     return false;
    }
  }
  delete [] y;
  return false; 
}


void PerspCon::checkLVars(ConstraintPtr cons, VariablePtr binvar,
                                         bool *lboundsok,
                                         VariableGroup *lVarFixVal,
                                         VariableGroup *nlVarFixVal)
{  
  double val;
  ConstVariablePtr var;
  const LinearFunctionPtr lf = cons->getLinearFunction();
  
  VariableGroupConstIterator mit;
  for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
       ++it) {
    var = it->first;
    if (var != binvar) {
      mit = (*nlVarFixVal).find(var);
      if (mit!=(*nlVarFixVal).end()) {
        (*lboundsok) = true;
        continue;     
      }
      val = checkVarBounds(var, binvar, lboundsok);
      if (!*lboundsok) {
        (*lVarFixVal).clear();
        return;
      } else {
        (*lVarFixVal).insert(std::make_pair(var,val));
      }
    } else {
      *lboundsok = true;
      (*lVarFixVal).insert(std::make_pair(var,0.0));
    }
  }
  return;
}


void PerspCon::checkNVars(ConstraintPtr cons, VariablePtr binvar,
                                         bool* boundsok, 
                                         VariableGroup *nlVarFixVal)
{ 
  double val; 
  ConstVariablePtr var;
  const QuadraticFunctionPtr qf = cons->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();

  if (nlf) {
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
      var = *it;
      val = checkVarBounds(var, binvar, boundsok);
      if (!*boundsok) {
        return;
      } else {
        (*nlVarFixVal).insert(std::make_pair(var,val));
      }
    }
  } else if (qf) {
    //VariableGroupConstIterator mit;
    for (VarIntMapConstIterator it=qf->varsBegin(); it!=qf->varsEnd(); ++it) {
      var = it->first;
      //mit = (*nlVarFixVal).find(var);
      //if (mit!=(*nlVarFixVal).end()) {
        //continue;     
      //}
      val = checkVarBounds(var, binvar, boundsok);
      if (!*boundsok) {
        return;
      } else {
        (*nlVarFixVal).insert(std::make_pair(var,val));
      }
    }
  }
  return;
}


double PerspCon::checkVarBounds(VariablePtr var, VariablePtr binvar,
                                bool* varbounded)
{
  UInt numvars;
  FunctionPtr f;
  ConstraintPtr c;
  FunctionType type;
  LinearFunctionPtr lf;
  std::vector<double> xlb, xub;
  double coeffvar, coeffbin, cub, clb;

  if (var->getLb() == 0) {
    xlb.push_back(0); 
  }
  if (var->getUb() == 0) {
    if (xlb[0] == 0) {
      *varbounded = true;
      return 0;
    } else {
      xub.push_back(0); 
    }
  }
  for (ConstrSet::iterator it=var->consBegin(); it!=var->consEnd(); ++it) {
    c = *it;
    type = c->getFunctionType();
    if (type!=Linear) {
      continue;
    } 
    f = c->getFunction();
    numvars = f->getNumVars();
    lf = c->getLinearFunction();
    coeffvar = lf->getWeight(var);
    if (numvars>2 || coeffvar==0) {
      continue;
    }
    cub = c->getUb();
    clb = c->getLb();
    if (numvars == 1) {
      if ((coeffvar > 0 && cub == 0) || (coeffvar< 0 && clb == 0)) {
        if (std::find(xlb.begin(), xlb.end(), 0) != xlb.end()) {
          *varbounded = true;
          return 0;
        } else {
          xub.push_back(0); 
        }
      } else if ((coeffvar > 0 && clb == 0) || (coeffvar < 0 && cub == 0)) {
         if (std::find(xub.begin(), xub.end(), 0) != xub.end()) {
          *varbounded = true;
          return 0;
        } else {
          xlb.push_back(0); 
        }     
      } else {
        continue;
      }
    } else { 
      coeffbin = lf->getWeight(binvar);
      if (coeffbin==0) {
        continue;
      }
     /* linear constraint with two terms, var and binvar */
      if (cub < INFINITY) {
        if (coeffvar < 0) {
          if (std::find(xub.begin(), xub.end(), -cub)!=xub.end()) {
            *varbounded = true;
            return -cub;
          } else {
            xlb.push_back(-cub); 
          }
        } else {
           if (std::find(xlb.begin(), xlb.end(), cub)!=xlb.end()) {
            *varbounded = true;
            return cub;
          } else {
            xub.push_back(cub); 
          }         
        }       
      } else if (clb > -INFINITY) {
        if (coeffvar < 0) {
          if (std::find(xlb.begin(), xlb.end(), -clb)!=xlb.end()) {
            *varbounded = true;
            return -clb;
          } else {
            xub.push_back(-clb); 
          }
        } else {
           if (std::find(xub.begin(), xub.end(), clb)!=xub.end()) {
            *varbounded = true;
            return clb;
          } else {
            xlb.push_back(clb); 
          }         
        }       
      }
    }
  }
  *varbounded = false;
  return 0;
}


bool PerspCon::checkVarTypes(ConstraintPtr cons, VarSetPtr binaries)
{
  VariablePtr var;
  VariableType type;
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
      if (nlf != NULL && nlf->hasVar(var)) {
        return false;
      } else if (qf != NULL && qf->hasVar(var)) {
        return false;
      }  
      if (lf != NULL) {
        if (lf->hasVar(var) && lf->getWeight(var) != 0) {
          binaries->insert(var);
        }
      }
      break;
    case Integer:
    case ImplInt:
      if ((var->getLb() == 0) && (var->getUb() == 1)) {
        if (nlf != NULL && nlf->hasVar(var)) {
          return false;
        } else if (qf != NULL && qf->hasVar(var)) {
          return false;
        }  
        if (lf != NULL) {
          if (lf->hasVar(var) && lf->getWeight(var) != 0) {
            binaries->insert(var);
          }
        }
      } 
      break;
    case Continuous:  
      break;
    default:
      return false; /* Unexpected variable */
      break;
    }
  }
  return true;
}


void PerspCon::displayInfo()
{
  //MS: make display better. Keep limited information remove S1 and S2
  UInt s = cons_.size();
  std::ostream &out = logger_->msgStream(LogInfo);
  if (s > 0) {
    out << me_ <<"Total nonlinear constraints in problem = " << p_->getNumCons()-p_->getNumLinCons() << std::endl; 
    out << me_ <<"Number of constraints amenable to PR = " << cons_.size() << std::endl;


    out << "----------------------------------------------------"<< std::endl;
    out << me_ << "Details of constraints amenable to perspective reformulation:";
    for (UInt i = 0; i < cons_.size() ; ++i) {
      out << std::endl; 
      out << i+1 << ". ";
      cons_[i]->write(out);
      out << "Structure type: " << sType_[i] << std::endl;
      out << "Associated binary variable: " << binvar_[i]->getName() << std::endl;
      out << "Value of variables when " <<
        binvar_[i]->getName() << " = 0:" << std::endl;
      for (VariableGroupConstIterator it=nlVarFixVal_[i].begin(); it!=nlVarFixVal_[i].end(); ++it) {
        out << "var " << it->first->getName() << " value " << it->second << std::endl;
      }
      for (VariableGroupConstIterator it=lVarFixVal_[i].begin(); it!=lVarFixVal_[i].end(); ++it) {
        out << "var " << it->first->getName() << " value " << it->second << std::endl;
      }
    }
    out << "\nNote: Description of S1 and S2 are in paper...... "<< std::endl;
    out << "----------------------------------------------------"<< std::endl;
 
  } else {
    out << me_ <<"Number of constraints amenable to PR = 0" << std::endl;
  }
}


void PerspCon::evalConstraint(ConstraintPtr cons)
{
  VariablePtr binvar;
  VarSetPtr binaries = (VarSetPtr) new VarSet();
  bool vartypeok = checkVarTypes(cons, binaries);
  if (!vartypeok) {
    return;
  }
  if (binaries->size() > 0) {
    for (VarSetConstIterator it=binaries->begin(); it!=binaries->end(); ++it) {
      binvar = *it;
      if (boundBinVar(cons, binvar)) {
        return;      
      }
    }
  }
  bool newvar;
  UInt numvars;
  ConstraintPtr c;
  VariablePtr var;
  FunctionType type; 
  VariableType vartype;
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  
  if (nlf) {
    var = *(nlf->varsBegin());
  } else {
    const QuadraticFunctionPtr qf = cons->getQuadraticFunction();
    if (qf) {
      var = qf->varsBegin()->first;
    }
  }
  
  for (ConstrSet::const_iterator it= var->consBegin(); it!=var->consEnd();
       ++it) {
    c = *it;
    type = c->getFunctionType();
    const FunctionPtr f = c->getFunction();
    if (type != Linear) {
      continue;
    }
    numvars = f->getNumVars();
    const LinearFunctionPtr lf = c->getLinearFunction();
    if ((numvars != 2) || (lf->getWeight(var) == 0)) {
      continue;
    }
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      binvar = itvar->first;
      vartype = binvar->getType();
      if (vartype == Continuous) {
        continue;
      } else if (vartype==Binary || vartype==ImplBin ||
          ((vartype==Integer || vartype==ImplInt) && (binvar->getLb()==0
           && binvar->getUb()==1))) {
        if (lf->getWeight(binvar) != 0) {
          newvar = true;
          if (binaries->size() > 0) {
            if ((*binaries).find(binvar)!=(*binaries).end()) {
              binaries->insert(binvar);
              newvar = false;
              break;            
            }
          }  
          if (newvar) {
            if (boundBinVar(cons, binvar)) {
              return;
            } 
          }
        }
      }
      break;
    }    
  }
  return;
}


void PerspCon::findPRCons()
{
  FunctionType type;
  ConstraintPtr cons;
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    cons = *it;
    type = cons->getFunctionType();
    if (type == Linear) {
      continue;
    }
    evalConstraint(cons);
  }
//#if SPEW 
  displayInfo();
  exit(1);
//#endif  
  return;
}


bool PerspCon::getStatus()
{
  if(cons_.size() > 0){
    return true; 
  } else {
    return false;
  }
}


PerspCon::~PerspCon()
{
  cons_.clear();
  sType_.clear();
  binvar_.clear();
  nlVarFixVal_.clear();
  lVarFixVal_.clear();
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
