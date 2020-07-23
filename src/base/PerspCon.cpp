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

PerspCon::PerspCon()
:env_(EnvPtr()), p_(ProblemPtr()), cons_(0), bVar_(0), sType_(0), consVec_(0),
  bVarVec_(0), varsInfoVec_(0) 
{
  absTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
}


PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env), p_(p), cons_(0), bVar_(0), sType_(0), consVec_(0), bVarVec_(0),
  varsInfoVec_(0) 

{
  logger_ = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("handler_log_level")->getValue());
  absTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
}


void PerspCon::populate(std::unordered_map<UInt,double> varsInfo)
{
  consVec_.push_back(cons_);
  bVarVec_.push_back(bVar_);
  varsInfoVec_.push_back(varsInfo);
  return;
}


bool PerspCon::boundBinVar()
{
  double cu;
  VariablePtr var = 0;
  bool boundsok = true;
  VariableGroup nlVarFixVal;
  std::unordered_map<UInt,double> varsInfo;

  cu = cons_->getUb(); 
  if (fabs(cu) <= absTol_) {
    checkNVars(&boundsok, var, &varsInfo);
    if (!boundsok) {
      return false;
    } else {
      if (var != 0) {
        // Find a linear constraint containing variables of nonlinear part of
        // the constraint with equality
        searchVarIdx(var, &boundsok, &varsInfo); 
        if (boundsok) {
          populate(varsInfo);    
        } else {
          // Try structure 3 here
          return false;        
        }
      } else {
        populate(varsInfo);    
      }
      return true;
    }
  } else {
    checkNVars(&boundsok, var, &varsInfo);
    if (!boundsok) {
      return false;
    } 
    checkLVars(&boundsok, var, &varsInfo);
    if (!boundsok) {
      return false;
    } else {
      if (var != 0) {
        searchVarIdx(var, &boundsok, &varsInfo); 
        if (!boundsok) {
          return false;        
        }
      }
    }

    int error = 0;
    FunctionPtr cf = cons_->getFunction();
    double *y = new double[p_->getNumVars()];  
    std::fill(y, y + p_->getNumVars(), 0);
    for (auto it = varsInfo.begin(); it != varsInfo.end(); ++it) {
      y[it->first] = it->second;
    }
    double act = cf->eval(y, &error);
    delete [] y;
    
    if (error==0) {    
      if (cu+absTol_ < act) {  
        bVar_->setLb_(1);
        return false;
      } else {
        populate(varsInfo); 
      }
    } else {
      populate(varsInfo); 
    }
  }

  return true; 
}

void PerspCon::checkLVars(bool *boundsok, VariablePtr var, 
                          std::unordered_map<UInt,double> *varsInfo)
{  
  double val;
  ConstVariablePtr v;
  const LinearFunctionPtr lf = cons_->getLinearFunction();
  
  if (lf) {
    for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
         ++it) {
      v = it->first;
      val = 0.0;
      if(v != bVar_){
        if ( (*varsInfo).find(v->getIndex()) != (*varsInfo).end()) {
          continue;     
        }
        val = checkVarBounds(v, boundsok);
        if (!(*boundsok)) {
          if (var != 0) {
            var = 0;
            (*varsInfo).clear();
            return;
          }
          var= v;
        } else {
          (*varsInfo).insert(std::make_pair(v->getIndex(),val));
        }
      } else {
        (*varsInfo).insert(std::make_pair(v->getIndex(),0.0));
      }
    }
  }     
  *boundsok = true;
  return;
}

void PerspCon::searchVarIdx(VariablePtr var, bool *boundsok, 
                          std::unordered_map<UInt,double> *varsInfo) 
{
  // We check linear equality constraints of the form l <= a^Tx <= l
  FunctionPtr f;
  ConstraintPtr c;
  LinearFunctionPtr lf; 
  double val, val1, val2;
  bool varFound, varFound1;
  UInt numVar = (*varsInfo).size(), lb, ub, vIdx;
  std::unordered_map<UInt,double>::const_iterator mit;

  for (ConstrSet::iterator it=var->consBegin(); it!=var->consEnd(); ++it) {
    c = *it;
    if (c->getFunctionType() == Linear) {
      lb = c->getLb();
      ub = c->getUb();
      if (fabs(lb - ub) <= absTol_) {
        f = c->getFunction();
        if (f->getNumVars() == numVar) {
          lf = f->getLinearFunction(); 
          if (lf) {
            val = 0;
            varFound  = true;
            varFound1 = false;
            for(VariableGroupConstIterator vit = lf->termsBegin();
                vit != lf->termsEnd(); ++vit) {
              vIdx = (vit->first)->getIndex();
              if (!varFound && (vIdx == var->getIndex())) {
                varFound1 = true;
                val1 = vit->second;
                continue;
              }
              mit = (*varsInfo).find(vIdx);
              if (mit != (*varsInfo).end()) {
                varFound = false;
                break;              
              }
              val += (mit->second)*(vit->second);
            }
            if (varFound && varFound1) {
              *boundsok = true;
              val2 = (ub - val)/val1;
              (*varsInfo).insert(std::make_pair(vIdx, val2));
              return;
            }
          }
        }
      }    
    }
  }
          
  (*varsInfo).clear();
  return;
}

void PerspCon::checkNVars(bool *boundsok, VariablePtr var, 
                          std::unordered_map<UInt,double> *varsInfo)
{ 
  double val; 
  ConstVariablePtr v;
  NonlinearFunctionPtr nlf = cons_->getNonlinearFunction();

  if (nlf) {
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
      v = *it;
      val = checkVarBounds(v, boundsok);
      if (!*boundsok) {
        if (var != 0) {
          var = 0;
          (*varsInfo).clear();
          return;
        }
        var = v;
      } else {
        (*varsInfo).insert(std::make_pair(v->getIndex(),val));
      }
    }
  } 
  
  QuadraticFunctionPtr qf = cons_->getQuadraticFunction();
  if (qf) {
    for (VarIntMapConstIterator it=qf->varsBegin(); it!=qf->varsEnd(); ++it) {
      v = it->first;
      if ((*varsInfo).find(v->getIndex()) != (*varsInfo).end()) {
        continue;     
      }
      val = checkVarBounds(v, boundsok);
      if (!*boundsok) {
        if (var != 0) {
          var = 0;
          (*varsInfo).clear();
          return;
        }
        var = v;
      } else {
        (*varsInfo).insert(std::make_pair(v->getIndex(),val));
      }
    }
  }
  *boundsok = true;
  return;
}


double PerspCon::checkVarBounds(VariablePtr var,
                                bool* varbounded)
{
  UInt numvars;
  FunctionPtr f;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  std::vector<double> xub;
  std::vector<double> xlb;
  double coeffvar, coeffbin, cub, clb, val;

  if (var->getLb()==0) {
    xlb.push_back(0); 
  }
  if (var->getUb()==0) {
    if (std::find(xlb.begin(), xlb.end(), 0)!=xlb.end()) {
      *varbounded = true;
      return 0;
    } else {
      xub.push_back(0); 
    }
  }
  for (ConstrSet::iterator it=var->consBegin(); it!=var->consEnd(); ++it) {
    c = *it;
    if (c->getFunctionType()!=Linear) {
      continue;
    } 
    f = c->getFunction();
    numvars = f->getNumVars();
    lf = c->getLinearFunction();
    if (numvars > 2) {
      continue;
    } else {
      coeffvar = lf->getWeight(var);
      if (coeffvar == 0) {
        continue;      
      }    
    }
    cub = c->getUb();
    clb = c->getLb();
    if (numvars == 1) {
      if ((coeffvar > 0 && cub == 0) ||
        (coeffvar < 0 && clb == 0)) {
        if (std::find(xlb.begin(), xlb.end(), 0) != xlb.end()) {
          *varbounded = true;
          return 0;
        } else {
          xub.push_back(0); 
        }
      } else if ((coeffvar > 0 && clb == 0) ||
        (coeffvar < 0 && cub == 0)) {
         if (std::find(xub.begin(), xub.end(), 0)!=xub.end()) {
          *varbounded = true;
          return 0;
        } else {
          xlb.push_back(0); 
        }     
      } else {
        continue;
      }
    } else { 
      coeffbin = lf->getWeight(bVar_);
      if (coeffbin==0) {
        continue;
      }
     /* linear constraint with two terms, var and binvar */
      if (cub<INFINITY) {
        if (coeffvar<0) {
          val = -cub/fabs(coeffvar);
          if (std::find(xub.begin(), xub.end(), val)!=xub.end()) {
            *varbounded = true;
            return val;
          } else {
            xlb.push_back(val); 
          }
        } else {
          val = cub/coeffvar;
           if (std::find(xlb.begin(), xlb.end(), val)!=xlb.end()) {
            *varbounded = true;
            return val;
          } else {
            xub.push_back(val); 
          }         
        }       
      } 
      if (clb>-INFINITY) {
        if (coeffvar<0) {
          val = -clb/fabs(coeffvar);
          if (std::find(xlb.begin(), xlb.end(), val)!=xlb.end()) {
            *varbounded = true;
            return val;
          } else {
            xub.push_back(val); 
          }
        } else {
          val = clb/coeffvar;
           if (std::find(xub.begin(), xub.end(), val)!=xub.end()) {
            *varbounded = true;
            return val;
          } else {
            xlb.push_back(val); 
          }         
        }       
      }
    }
  }
  *varbounded = false;
  return 0;
}


bool PerspCon::findBinVarsInCons(std::vector<VariablePtr>* binaries)
{
  VariablePtr var;
  FunctionPtr f = cons_->getFunction();

  for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    var = *it;
    switch (var->getType()) {
    case Binary:
    //case ImplBin:
      (*binaries).push_back(var);
      break;
    case Integer:
    //case ImplInt:
      if ((var->getLb() == 0) && (var->getUb() == 1)) {
        (*binaries).push_back(var);
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
  UInt s = consVec_.size();
  std::ostream &out = logger_->msgStream(LogInfo);
  if (s > 0) {
    out << me_ <<"Total nonlinear constraints in problem = " << p_->getNumCons()-p_->getNumLinCons() << std::endl; 
    out << me_ <<"Number of constraints amenable to PR = " << consVec_.size() << std::endl;


    out << "----------------------------------------------------"<< std::endl;
    out << me_ << "Details of constraints amenable to perspective reformulation:";
    for (UInt i = 0; i < consVec_.size() ; ++i) {
      out << std::endl; 
      out << i+1 << ". ";
      consVec_[i]->write(out);
      out << "Structure type: S" << sType_[i] << std::endl;
      out << "Associated binary variable: " << bVarVec_[i]->getName() << std::endl;
      out << "Value of variables when " <<
        bVarVec_[i]->getName() << " = 0:" << std::endl;
      //for (VariableGroupConstIterator it=nlVarFixVal_[i].begin(); it!=nlVarFixVal_[i].end(); ++it) {
        //out << "var " << it->first->getName() << " value " << it->second << std::endl;
      //}
      //for (VariableGroupConstIterator it=lVarFixVal_[i].begin(); it!=lVarFixVal_[i].end(); ++it) {
        //out << "var " << it->first->getName() << " value " << it->second << std::endl;
      //}
    }
    out << "\nNote: Description of S1 and S2 are in paper...... "<< std::endl;
    out << "----------------------------------------------------"<< std::endl;
 
  } else {
    out << me_ <<"Number of constraints amenable to PR = 0" << std::endl;
  }
  //p_->write(std::cout);
}


void PerspCon::evalConstraint()
{
  std::vector<VariablePtr> binaries;
  
  if (!(findBinVarsInCons(&binaries))) {
    return;
  }
  
  for (std::vector<VariablePtr>::iterator it = binaries.begin();
       it!=binaries.end(); ++it) {
    bVar_ = *it;
    if (boundBinVar()) {
      return;      
    }
  }
 
 // If there are more than one binary variables in the function and if two
 // control the other variables then the two binary variables are equal. See
 // if this condition arises.
 
  bool newvar;
  UInt numvars;
  ConstraintPtr c;
  VariablePtr var;
  FunctionType type; 
  VariableType vartype;
  LinearFunctionPtr lf;
  const NonlinearFunctionPtr nlf = cons_->getNonlinearFunction();
  
  if (nlf) {
    var = *(nlf->varsBegin());
  } else {
    const QuadraticFunctionPtr qf = cons_->getQuadraticFunction();
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
    lf = c->getLinearFunction();
    if ((numvars!=2) || (lf->getWeight(var)==0)) {
      continue;
    }
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      bVar_ = itvar->first;
      vartype = bVar_->getType();
      if (vartype==Continuous) {
        continue;
      }
      if (lf->getWeight(bVar_)!=0) {
        if (vartype==Binary || vartype==ImplBin ||
            (vartype==Integer && bVar_->getLb()==0 && bVar_->getUb()==1) ||
            (vartype==ImplInt && bVar_->getLb()==0 && bVar_->getUb()==1)) { //check if we need implied vars anywhere
          newvar = true;
          if (binaries.size()>0) {
            if (find(binaries.begin(), binaries.end(), bVar_)!=binaries.end()) {
              binaries.push_back(bVar_);
              newvar = false;
              break;            
            }
          }  
          if (newvar) {
            if (boundBinVar()) {
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
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    cons_ = *it;
    switch(cons_->getFunctionType()) {
    case Nonlinear:
    case Quadratic:
    //case Bilinear:
    //case Polynomial:
    //case Multilinear:
      evalConstraint();
      break;
    default:
      break;
    }
  }

 
//#if SPEW 
  //displayInfo();
  //exit(1);
//#endif  

  cons_ = 0;
  bVar_ = 0;
  return;
}


bool PerspCon::getStatus()
{
  if(consVec_.size() > 0){
    return true; 
  } else {
    return false;
  }
}


PerspCon::~PerspCon()
{
  //p_.reset();
  //env_.reset();
  //logger_.reset();
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
