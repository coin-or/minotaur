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
#include <iterator>     // std::distance
#include <forward_list>


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


PerspCon::PerspCon(EnvPtr env, ProblemPtr p)
: env_(env), p_(p), cons_(0), bVar_(0), sType_(0), consVec_(0), bVarVec_(0),
  varsInfoVec_(0) 

{
  timer_ = env->getNewTimer();
  logger_ = env->getLogger();
  //logger_ = (LoggerPtr) new Logger((LogLevel) 
      //env->getOptions()->findInt("handler_log_level")->getValue());
  absTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
}

PerspCon::~PerspCon()
{
  if (timer_) {
    delete timer_;
  }
  // What else to reset here?
  p_ = 0;
  env_ = 0;
}


void PerspCon::populate_(std::unordered_map<UInt,double> varsInfo)
{
  consVec_.push_back(cons_);
  bVarVec_.push_back(bVar_);
  varsInfoVec_.push_back(varsInfo);
  return;
}


bool PerspCon::boundBinVar_()
{
  double cu;
  VariablePtr var = 0;
  bool boundsok = true;
  VariableGroup nlVarFixVal;
  std::unordered_map<UInt,double> varsInfo;

  cu = cons_->getUb(); 
  if (fabs(cu) <= absTol_) {
    checkNVars_(&boundsok, var, &varsInfo);
    if (!boundsok) {
      return false;
    } else {
      if (var != 0) {
        // Find a linear constraint containing variables of nonlinear part of
        // the constraint with equality
        searchVarIdx_(var, &boundsok, &varsInfo); 
        if (boundsok) {
          populate_(varsInfo);    
        } else {
          // Try structure 3 here
          return false;        
        }
      } else {
        populate_(varsInfo);    
      }
      return true;
    }
  } else {
    checkNVars_(&boundsok, var, &varsInfo);
    if (!boundsok) {
      return false;
    } 
    checkLVars_(&boundsok, var, &varsInfo);
    if (!boundsok) {
      return false;
    } else {
      if (var != 0) {
        searchVarIdx_(var, &boundsok, &varsInfo); 
        if (!boundsok) {
          return false;        
        }
      }
    }

    int error = 0;
    FunctionPtr cf = cons_->getFunction();
    double *y = new double[p_->getNumVars()];  
    std::fill(y, y + p_->getNumVars(), 0);
    for (std::unordered_map<UInt,double>::iterator it = varsInfo.begin(); it != varsInfo.end(); ++it) {
      y[it->first] = it->second;
    }
    double act = cf->eval(y, &error);
    delete [] y;
    
    if (error==0) {    
      if (cu+absTol_ < act) {  
        bVar_->setLb_(1);
        return false;
      } else {
        populate_(varsInfo); 
      }
    } else {
      populate_(varsInfo); 
    }
  }

  return true; 
}

void PerspCon::checkLVars_(bool *boundsok, VariablePtr var, 
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
        val = checkVarBounds_(v, boundsok);
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

void PerspCon::searchVarIdx_(VariablePtr var, bool *boundsok, 
                          std::unordered_map<UInt,double> *varsInfo) 
{
  // We check linear equality constraints of the form l <= a^Tx <= l
  FunctionPtr f;
  ConstraintPtr c;
  LinearFunctionPtr lf; 
  double val, val1, val2;
  bool varFound, varFound1;
  UInt numVar = (*varsInfo).size(), lb, ub, vIdx;
  std::unordered_map<UInt,double>::const_iterator mit;//why const?

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

void PerspCon::checkNVars_(bool *boundsok, VariablePtr var, 
                          std::unordered_map<UInt,double> *varsInfo)
{ 
  double val; 
  ConstVariablePtr v;
  NonlinearFunctionPtr nlf = cons_->getNonlinearFunction();

  if (nlf) {
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
      v = *it;
      val = checkVarBounds_(v, boundsok);
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
      val = checkVarBounds_(v, boundsok);
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


double PerspCon::checkVarBounds_(VariablePtr var, bool* varbounded)
{
  UInt numvars;
  //FunctionPtr f;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  std::vector<double> xub; // make it a list
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
    //f = c->getFunction();
    //numvars = f->getNumVars();
    lf = c->getLinearFunction();
    numvars = lf->getNumTerms();
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


bool PerspCon::findBinVarsInCons_(std::vector<VariablePtr>* binaries)
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


void PerspCon::displayInfo_()
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


void PerspCon::evalConstraint_()
{
  std::vector<VariablePtr> binaries;
  
  if (!(findBinVarsInCons_(&binaries))) {
    return;
  }
  
  for (std::vector<VariablePtr>::iterator it = binaries.begin();
       it!=binaries.end(); ++it) {
    bVar_ = *it;
    if (boundBinVar_()) {
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
            if (boundBinVar_()) {
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


bool PerspCon::multiTermsFunc_(LinearFunctionPtr lf, VariablePtr var, 
                            std::forward_list<impliVar> *varList, double lb,
                            double ub, double val, bool z0)
{
  double vc, vlb, vub;
  VariablePtr v;
  impliVar impli;
  bool isFound = true, ubChecked = false, cFeas = false, cInf = false;
  std::forward_list<impliVar>::iterator it1;
  
  if (fabs(ub-val) < absTol_) {
    ubChecked = 1;
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      if (v != var) {
        vlb = v->getLb();
        vub = v->getUb();
        vc = itvar->second;
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
      cFeas = false; cInf = false;
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        v = itvar->first;
        if (v != var) {
          vlb = v->getLb();
          vub = v->getUb();
          vc = itvar->second;
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

  if (isFound) {
    ++c2_;
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
      if (!z0) {
        std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
          " Fixing this variable to 1.\n";
        var->setLb_(1);
      } else {
        std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
          " Fixing this variable to 0.\n";
        var->setUb_(0);      
      }
      return true;
    }
  }
  return false;
}


bool PerspCon::twoTermsFunc_(LinearFunctionPtr lf, VariablePtr var, 
                            std::forward_list<impliVar> *varList, double lb,
                            double ub, bool z0)
{
  VariablePtr v;
  impliVar impli;
  double vc, bc = 0, val, vlb, vub;
  std::forward_list<impliVar>::iterator it1;

  for (VariableGroupConstIterator itvar=lf->termsBegin();
       itvar!=lf->termsEnd(); ++itvar) {
    if (itvar->first != var) {
      v = itvar->first;
      vc = itvar->second;
      if (fabs(vc) < absTol_) {
        return false;
      }
    } else {
      if (z0) {
        bc = itvar->second;
      }
    }
  }

  for (it1 = (*varList).begin(); it1 != (*varList).end(); 
        ++it1) {
    if (v == (*it1).var) {
      break;
    }
  }
        
  ++c1_;
  vlb = v->getLb();
  vub = v->getUb();

  if (fabs(lb-ub) < absTol_) {
    val = (ub-bc)/vc;
    if ((val <= vlb - absTol_)  || (val >= vub + absTol_)) {
      if (!z0) {
        std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
          " Fixing this variable to 1.\n";
        var->setLb_(1);
      } else {
        std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
          " Fixing this variable to 0.\n";
        var->setUb_(0);      
      }
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
          if (!z0) {
            std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
              " Fixing this variable to 1.\n";
            var->setLb_(1);
          } else {
            std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
              " Fixing this variable to 0.\n";
            var->setUb_(0);      
          }
          return true;
        }
        // obtained upper bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          // new variable
          if (fabs(vlb) < absTol_) {
            impli.fixedVal.push_back(0);
          } else {
            impli.fixedVal.push_back(-INFINITY);
          }
          impli.fixedVal.push_back(val);
          (*varList).push_front(impli);
        } else {
          // existing variable
          if (val <= (*it1).fixedVal[1] - absTol_) {
            (*it1).fixedVal[1] = val;
          }
        }
      } else {
        if (val >= vub + absTol_) {
          if (!z0) {
            std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
              " Fixing this variable to 1.\n";
            var->setLb_(1);
          } else {
            std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
              " Fixing this variable to 0.\n";
            var->setUb_(0);      
          }
          return true;
        }
         // obtained lower bound
         if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal.push_back(val);
          if (fabs(vub) < absTol_) {
            impli.fixedVal.push_back(0);
          } else {
            impli.fixedVal.push_back(INFINITY);
          }
          (*varList).push_front(impli);
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
           if (!z0) {
            std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
              " Fixing this variable to 1.\n";
            var->setLb_(1);
           } else {
             std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
               " Fixing this variable to 0.\n";
             var->setUb_(0);      
           }        
           return true;
         }
          // obtained lower bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal.push_back(val);
          if (fabs(vub) < absTol_) {
            impli.fixedVal.push_back(0);
          } else {
            impli.fixedVal.push_back(INFINITY);
          }
          (*varList).push_front(impli);
        } else {
          if (val >= (*it1).fixedVal[0] + absTol_) {
            (*it1).fixedVal[0] = val;
          }
        }
      } else {
         if (val <= vlb - absTol_) {
           if (!z0) {
            std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
              " Fixing this variable to 1.\n";
            var->setLb_(1);
           } else {
             std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
               " Fixing this variable to 0.\n";
             var->setUb_(0);      
           }        
           return true;
         }
        // obtained upper bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          if (fabs(vlb) < absTol_) {
            impli.fixedVal.push_back(0);
          } else {
            impli.fixedVal.push_back(-INFINITY);
          }
          impli.fixedVal.push_back(lb/vc);
          (*varList).push_front(impli);
        } else {
          if (val <= (*it1).fixedVal[1] - absTol_) {
            (*it1).fixedVal[1] = val;
          }
        }              
      }             
    }
  }
      
  return false;
}

void PerspCon::deriveImpli_(VariablePtr var)
{
  bool isFixed0 = false, isFixed1 = false;
  UInt numVars;
  double lb, ub, val;
  impliVar impli;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  std::forward_list<impliVar> varList0, varList1;
  std::forward_list<impliVar>::iterator it1;

  for (ConstrSet::iterator it=var->consBegin(); it!=var->consEnd(); ++it) {
    c = *it;
    if (c->getFunctionType() == Linear) {
      lb = c->getLb();
      ub = c->getUb();
      lf = c->getLinearFunction();
      val = lf->getWeight(var);
      if (fabs(val) < absTol_) {
        continue;      
      }
      numVars = lf->getNumTerms();
      if (numVars == 2) {
        isFixed0 = twoTermsFunc_(lf, var, &varList0, lb, ub, 0);
        if (!isFixed0) {
          isFixed1 = twoTermsFunc_(lf, var, &varList1, lb, ub, 1);
        }
      } else if (numVars > 2) {
        //if (fabs(lb-ub) > absTol_) {
          if ((fabs(lb) < absTol_) || (fabs(ub) < absTol_)) {
            isFixed0 = multiTermsFunc_(lf, var, &varList0, lb, ub, 0, 0);
          }
          if (!isFixed0) {
            if ((fabs(lb-val) < absTol_) || (fabs(ub-val) < absTol_)) {
              isFixed1 = multiTermsFunc_(lf, var, &varList1, lb, ub, val, 1);
            }
          }
        //}
      }  
      if (isFixed0 || isFixed1) {
        return;
      }
    }
  }
    
  isFixed0 = addImplications_(&varList0);
  isFixed1 = addImplications_(&varList1);

  if (!isFixed0 && !isFixed1) {
    if (!(varList0.empty())) {
      impli0_.insert({var, varList0});  
    }
      
    if (!(varList1.empty())) {
      impli1_.insert({var, varList1});  
    }
  } else if (isFixed0 && isFixed1) {
    std::cout << "INFEASIBLE PROBLEM!!!!!!!!!\n";
    exit(1); // To be
  } else if (isFixed0) {
    std::cout <<"Infeasibility with "<< var->getName() << " = 0." <<
      " Fixing this variable to 1.\n";
    var->setLb_(1); 
  } else {
    std::cout <<"Infeasibility with "<< var->getName() << " = 1." <<
      " Fixing this variable to 0.\n";
    var->setUb_(0);  
  }

  return;
}

bool PerspCon::addImplications_(std::forward_list<impliVar> *varList)
{
  double lb, ub, valLb, valUb;
  std::forward_list<impliVar>::iterator it1;
  std::forward_list<impliVar>::iterator prev = (*varList).before_begin();  

  for (it1 = (*varList).begin(); it1 != (*varList).end(); ) {
    lb = (*it1).var->getLb();
    ub = (*it1).var->getUb();
    valLb = (*it1).fixedVal[0];
    valUb = (*it1).fixedVal[1];
    if ((valUb <= valLb - absTol_) || (lb - absTol_ >= valUb) || (ub + absTol_ <= valLb)) {
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


void PerspCon::implications_()
{
  timer_->start();
  c1_ = c2_ = c3_ = 0;
  std::cout << "--------------------------\n"; 
  std::cout << "Details of variable fixing in PerspCon class\n"; 
  std::cout << "--------------------------\n"; 
  VariablePtr var;
  ConstraintPtr c;

  for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); 
       ++it) {
    var = *it;
    switch (var->getType()) {
    case Binary:
    case ImplBin:
      deriveImpli_(var);
      break;
    case Integer:
    case ImplInt:
      if ((fabs(var->getLb()) < absTol_) && 
          (fabs(var->getUb() - 1) < absTol_)) {
        deriveImpli_(var);
      } 
      break;
    default:
      break;
    }
  }

  if ((impli0_.size() == 0) && (impli1_.size() == 0)) {
    return;  
  }

  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    c = *it;
    if (c->getFunctionType() == Linear) {
      if (impli0_.size() != 0) {
        addImplications_(c, 0);
      }
      if (impli1_.size() != 0) {
        addImplications_(c, 1);
      }
    }
  }


  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator mit;
    
  std::cout << "----------------z = 0 case-----------------------\n";
  for (mit = impli0_.begin(); mit != impli0_.end(); ++mit) {
    std::cout << "---------------------------------------\n";
    std::cout << "Binary var " << mit->first->getName() << "\n";
    for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      std::cout << "var " << (*it1).var->getName() << " " << (*it1).fixedVal[0] << "\n";
    }
  }
  std::cout << "--------------------------\n"; 
  std::cout << "----------------z = 1 case-----------------------\n";
  for (mit = impli1_.begin(); mit != impli1_.end(); ++mit) {
    std::cout << "---------------------------------------\n";
    std::cout << "Binary var " << mit->first->getName() << "\n";
    for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      std::cout << "var " << (*it1).var->getName() << " " << (*it1).fixedVal[0] << "\n";
    }
  }  

  bool s0 = (impli0_.size()) >0?1:0, s1 = (impli1_.size()) >0?1:0; 
  std::cout << "---------------------------------------\n";
  std::cout << "Structure z0, z1, and time taken, and Collection count " 
    << s0 << " " << s1 << " " << timer_->query() << " " << c1_ << " " 
    << c2_ << " " << c3_ << "\n"; 
  std::cout << "---------------------------------------\n";
  timer_->stop();
  
  return;
}

void PerspCon::addImplications_(ConstraintPtr c, bool z0)
{
  bool found;
  impliVar impli;
  VariablePtr var;
  LinearFunctionPtr lf;
  UInt listSize, count;
  double lb, ub, val, vc;
  impli.fixedVal.push_back(-INFINITY);
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator 
    it, itEnd;
  impli.fixedVal.push_back(-INFINITY);

  if (z0 == 0) {
    it = impli0_.begin();
    itEnd = impli0_.end();
  } else {
    it = impli1_.begin();
    itEnd = impli1_.end(); 
  }

  lb = c->getLb();
  ub = c->getUb();
  if (fabs(lb-ub) < absTol_) {
    lf = c->getLinearFunction();
    for (; it != itEnd; ++it) {
      it1 = (it->second).begin();
      listSize = std::distance(it1, (it->second).end());
      if (lf->getNumTerms() <= (2 + listSize)) {
        val = 0;
        count = 0;
        for (VariableGroupConstIterator itvar=lf->termsBegin();
             itvar!=lf->termsEnd(); ++itvar) {
          var = itvar->first;
          if (var == it->first) {
            val = val + itvar->second;
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
          if ((val <= impli.var->getLb() - absTol_) || (val >= impli.var->getUb() + absTol_)) {
            if (z0 == 0) {
              std::cout <<"Infeasibility with "<< (it->first)->getName() 
                << "= 0. " << " Fixing this variable to 1.\n";
              (it->first)->setLb_(1);           
              impli0_.erase(it);
              if (impli0_.empty()) {
                break;
              }
            } else {
               std::cout <<"Infeasibility with "<< (it->first)->getName() 
                << "= 1. " << " Fixing this variable to 0.\n";
              (it->first)->setUb_(0);           
              impli1_.erase(it);
              if (impli1_.empty()) {
                break;
              }             
            }
          } else {
            ++c3_;
            impli.fixedVal[0] = val;
            (it->second).push_front(impli);          
          }
          break;
        }
      }
    }
  }
}


void PerspCon::findPRCons()
{

  /// To determine all the variables fixed by binary variables
  //Perform this step if there are linear constraints and binary variables 
 
  implications_(); 
  exit(1);

  //// This to check only nonlinear constraints one by one
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    cons_ = *it;
    switch(cons_->getFunctionType()) {
    case Nonlinear:
    case Quadratic:
    //case Bilinear:
    //case Polynomial:
    //case Multilinear:
      evalConstraint_();
      break;
    default:
      break;
    }
  }

 
//#if SPEW 
  //displayInfo_();
  //exit(1);
//#endif  

  cons_ = 0;
  bVar_ = 0;
  return;
}


bool PerspCon::getStatus_()
{
  if(consVec_.size() > 0){
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
