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

using std::endl;

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
:env_(EnvPtr()), p_(ProblemPtr()), sType_(0), cons_(0), binvar_(0),
  fixbvar_(0), nviv_(0), lviv_(0) 
{
}


PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env), p_(p), sType_(0), cons_(0), binvar_(0), fixbvar_(0),
  nviv_(0), lviv_(0) 

{
  logger_ = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("handler_log_level")->getValue());
}


bool PerspCon::boundBinVar(ConstConstraintPtr cons, VariablePtr& binvar)
{
  int error = 0;
  std::vector<double > nviv;
  std::vector<double > lviv;
  FunctionPtr cf = cons->getFunction();
  bool boundsok = false, lboundsok = false;
  double cu = cons->getUb(), cl = cons->getLb(), act;
  
  double *y = new double[p_->getNumVars()];  
  std::fill(y, y + p_->getNumVars(), 0);
  
  nviv = checkNVars(cons, binvar, &boundsok);
  lviv = checkLVars(cons, binvar, &lboundsok);
  
  if (boundsok == true && lboundsok == true) {
    UInt i = 0;
    ConstVariablePtr var;
    const QuadraticFunctionPtr qf = cons->getQuadraticFunction();
    const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
    if (nlf) {
      for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
        var = *it;
        y[var->getIndex()] = nviv[i];
        i++;
      }
    } else if (qf) {
      for (VarIntMapConstIterator it=qf->varsBegin(); it!=qf->varsEnd(); ++it) {
        var = it->first;
        y[var->getIndex()] = nviv[i];
        i++;
      }
    }
   const LinearFunctionPtr lf = cons->getLinearFunction();
   if (lf) {
     i =0;
     for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
          ++it) {
       var = it->first;
       y[var->getIndex()] = lviv[i];
       i++;
     }
   }

  
   act = cf->eval(y, &error);
   double absTol = env_->getOptions()->findDouble("solAbs_tol")->getValue();
   if (error == 0) {    
     if (cu != INFINITY ) {
       if (cu + absTol < act) {
         fixbvar_.push_back(binvar);
       } else {
         cons_.push_back(cons);
         sType_.push_back('1');  
         nviv_.push_back(nviv);
         lviv_.push_back(lviv);
         binvar_.push_back(binvar);
         delete [] y;
         return true;
       }
     } else if (cl != -INFINITY) {
       if (cl - absTol > act) {
         fixbvar_.push_back(binvar);
       } else {
         cons_.push_back(cons);
         sType_.push_back('1');  
         nviv_.push_back(nviv);
         lviv_.push_back(lviv);
         binvar_.push_back(binvar);
         delete [] y;
         return true;
       }
     }
   } else {
     logger_->msgStream(LogError) << me_ 
       << "Constraint is not defined at this point" << std::endl;
   }
  } else if (boundsok == true) {
    if ((cu == 0) || (cl == 0)) {
      cons_.push_back(cons);
      sType_.push_back('2');  
      nviv_.push_back(nviv);
      lviv_.push_back(lviv);
      binvar_.push_back(binvar);
      delete [] y;
      return true;
    }
  }
  delete [] y;
  return false;
}


std::vector<double > PerspCon::checkLVars(ConstConstraintPtr cons,
                                         ConstVariablePtr binvar,
                                         bool* lboundsok)
{  
  double temp;
  ConstVariablePtr var;
  std::vector<double > lviv;
  const LinearFunctionPtr lf = cons->getLinearFunction();
  
  if (lf) {
    for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
         ++it) {
      var = it->first;
      temp = 0.0;
      if(var != binvar){
        temp = checkVarBounds(var, binvar, lboundsok);
        if (!lboundsok) {
          lviv.clear();
          return lviv;
        } else {
          lviv.push_back(temp);
        }
      } else {
        temp = 0.0;
        lviv.push_back(temp);
      }
    }
  } 
  return lviv;
}


std::vector<double> PerspCon::checkNVars(ConstConstraintPtr cons,
                                         ConstVariablePtr binvar,
                                         bool* boundsok)
{  
  double temp;
  ConstVariablePtr var;
  std::vector<double > nviv;
  const QuadraticFunctionPtr qf = cons->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();

  if (nlf) {
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
      var = *it;
      temp = checkVarBounds(var, binvar, boundsok);
      if (!boundsok) {
        nviv.clear();
        return nviv;
      } else {
        nviv.push_back(temp);
      }
    }
  } else if (qf) {
    for (VarIntMapConstIterator it=qf->varsBegin(); it!=qf->varsEnd(); ++it) {
      var = it->first;
      temp = checkVarBounds(var, binvar, boundsok);
      if (!boundsok) {
        nviv.clear();
        return nviv;
      } else {
        nviv.push_back(temp);
      }
    }
  }
  return nviv;
}


double PerspCon::checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar,
                                bool* varbounded)
{
  UInt numvars;
  FunctionType type;
  ConstConstraintPtr c;
  std::vector<double> xub;
  std::vector<double> xlb;
  bool vub = false, vlb = false;
  double coeffvar, coeffbin, cub, clb;

  if (var->getLb() == 0) {
    vlb = true;  
  }

  if (var->getUb() == 0) {
    vub = true;  
  }

  for (ConstrSet::iterator it= var->consBegin(); it!= var->consEnd(); ++it) {
    c = *it;
    type = c->getFunctionType();
    const FunctionPtr f = c->getFunction();
  
    if (type != Linear) {
      continue;
    } 
    
    numvars = f->getNumVars();
    const LinearFunctionPtr lf = c->getLinearFunction();
    coeffvar = lf->getWeight(var);
  
    if (numvars > 2 || coeffvar == 0) {
      continue;
    }

    cub = c->getUb();
    clb = c->getLb();
    
    if (numvars == 1) {
      if ((coeffvar > 0 && cub == 0) ||
        (coeffvar < 0 && clb == 0)) {
        vub = true;
        if (vlb == true) {
          *varbounded = true;
          return 0;   
        }
      } else if ((coeffvar > 0 && clb == 0) ||
        (coeffvar < 0 && cub == 0)  ) {
        vlb = true;      
        if (vub == true) {
          *varbounded = true;
          return 0;
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
      if (cub != INFINITY) {
        if (cub == 0) {
          if (coeffvar < 0) {
            vlb = true;
            if (vub == true) {
              *varbounded = true;
              return 0;
            }
          } else {
            vub = true;
            if (vlb == true) {
              *varbounded = true;
              return 0;
            }    
          }
        } else {
           if (coeffvar < 0) {
             if ( std::find(xub.begin(), xub.end(), -cub) != xub.end() ) {
               *varbounded = true;
               return -cub;
             } else {
               xlb.push_back(-cub); 
             }
           } else {
              if ( std::find(xlb.begin(), xlb.end(), cub) != xlb.end() ) {
               *varbounded = true;
               return cub;
             } else {
               xub.push_back(cub); 
             }         
          }       
        }
      }

      if (clb != -INFINITY) {
        if (clb == 0) {
          if (coeffvar < 0) {
            vub = true;
            if (vlb == true) {
              *varbounded = true;
              return 0;
            } 
          } else {
            vlb = true;
            if (vub == true) {
              *varbounded = true;
              return 0;
            }
          }
        } else {
           if (coeffvar < 0) {
             if ( std::find(xlb.begin(), xlb.end(), -clb) != xlb.end() ) {
               *varbounded = true;
               return -clb;
             } else {
               xub.push_back(-clb); 
             }
           } else {
              if ( std::find(xub.begin(), xub.end(), clb) != xub.end() ) {
               *varbounded = true;
               return clb;
             } else {
               xlb.push_back(clb); 
             }         
          }       
        }
      }
    }
  }
  *varbounded = false;
  return 0;
}


bool PerspCon::checkVarTypes(ConstConstraintPtr cons, VarSetPtr binaries)
{
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
      if (lf != NULL) {
        if (lf->hasVar(var)) {
          if (lf->getWeight(var)!=0) {
            binaries->insert(var);
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
          if (lf != NULL) {
            if (lf->hasVar(var)) {
              if (lf->getWeight(var)!=0) {
                binaries->insert(var);
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
      break;
    }
  }
  return true;
}


void PerspCon::displayInfo()
{
  UInt s = cons_.size();
  std::ostream &out = logger_->msgStream(LogInfo);

  if (s > 0) {
    out << "----------------------------------------------------"<< std::endl;
    out << me_ << "Details of constraints amenable to perspective reformulation:";
    for (UInt i = 0; i < cons_.size() ; ++i) {
      out << std::endl; 
      out << i+1 << ". ";
      cons_[i]->write(out);
      out << "Structure type: S" << sType_[i] << std::endl;
      out << "Associated binary variable: " << binvar_[i]->getName() << std::endl;
      out << "Fixed values of variables (appear in nonlinear followed by linear terms) "
        "when " << binvar_[i]->getName() << " takes 0: (";
      for (UInt j= 0; j < nviv_[i].size(); ++j) {
        out << nviv_[i][j] << " ";
      }
      for (UInt j= 0; j < lviv_[i].size(); ++j) {
        out << lviv_[i][j] << " ";
      }
      out << ")" << std::endl;
    }
    out << "\n Note: Description of S1 and S2 are in paper...... "<< std::endl;
    out << "----------------------------------------------------"<< std::endl;
 
  }
}


void PerspCon::evalConstraint(ConstConstraintPtr cons,
                                     VariablePtr& binvar)
{
  VarSetPtr binaries = (VarSetPtr) new VarSet();
  bool vartypeok = checkVarTypes(cons, binaries);
  if (!vartypeok) {
    return;
  }

  if (binaries->size() > 0) {
    for (VarSetConstIterator it= binaries->begin(); it!=binaries->end();
         ++it) {
      binvar = *it;
      if (boundBinVar(cons, binvar)) {
        return;      
      }
    }
    
  }

  bool newvar;
  UInt numvars;
  VariablePtr bvar;
  FunctionType type; 
  VariableType vartype;
  ConstConstraintPtr c;
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  ConstVariablePtr var = *(nlf->varsBegin());
  
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

    if( (numvars != 2) || (lf->getWeight(var) == 0) ){
      continue;
    }
    
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      binvar = itvar->first;
      vartype = binvar->getType();
    
      if (vartype == Continuous) {
        continue;
      }
      if (lf->getWeight(binvar) != 0) {
        if ((vartype == Binary) || (vartype == ImplBin) ||
            ((vartype = Integer) && (binvar->getLb() == 0) 
             && (binvar->getUb()==1))) {
          newvar = true;
          if (binaries->size() > 0) {
            for (VarSetConstIterator itv= binaries->begin(); 
                 itv!=binaries->end(); ++itv) {
              bvar = *itv;
              if (binvar == bvar) { 
                newvar = false;
                break;
              }
            }
          }  
          if (newvar) {
            if (boundBinVar(cons, binvar)) {
              return;
            } else {
              binaries->insert(binvar);
            }
          }
        } else {
          continue;
        }
      }
    }    
  }
  return;
}


void PerspCon::generateList()
{
  FunctionType type;
  VariablePtr binvar;
  ConstConstraintPtr cons;

  for (ConstraintConstIterator it= p_->consBegin(); it!= p_->consEnd(); ++it) {
    cons = *it;
    type = cons->getFunctionType();
    
    if (type == Linear) {
      continue;
    }
    evalConstraint(cons, binvar);
  }

 if (fixbvar_.size() > 0) {
   FunctionPtr fnew;
   LinearFunctionPtr lfnew;
   for (UInt i= 0; i < fixbvar_.size() ; ++i) {
     lfnew = (LinearFunctionPtr) new LinearFunction();
     lfnew->addTerm(fixbvar_[i], 1);
     fnew = (FunctionPtr) new Function(lfnew);
     p_->newConstraint(fnew, 1.0, 1.0,
                       "Perspective: fixing binary variable to 1");
   }
   fixbvar_.clear();
 }

 logger_->msgStream(LogInfo) << me_ 
   <<"Total no. of constraints amenable to PR = " << cons_.size() << std::endl;

#if SPEW 
  displayInfo();
#endif  
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
  p_.reset();
  env_.reset();
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
