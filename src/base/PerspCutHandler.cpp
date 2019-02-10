//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.cpp
 * \brief Declare PerspCutHandler class to generate perspective cuts.
 * \author Meenarli Sharma, Indian Institute of Technology Bombay
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


#include "PerspCutHandler.h"
#include "LinearFunction.h"
#include "CutManager.h"
#include "Environment.h"
#include "NonlinearFunction.h"
#include "Operations.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "Constraint.h"
#include "Function.h"
#include "Solution.h"
#include "Option.h"
#include "Logger.h"
#include "Node.h"
#include "PerspCon.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string PerspCutHandler::me_ = "PerspCutHandler: ";

PerspCutHandler::PerspCutHandler()
  : env_(EnvPtr()),  minlp_(ProblemPtr()), stats_(0), cons_(0), binvar_(0),
  sType_(0), nviv_(0), lviv_(0)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
}


PerspCutHandler::PerspCutHandler(EnvPtr env, ProblemPtr minlp)
  : env_(env), minlp_(minlp), cons_(0), binvar_(0), sType_(0), nviv_(0),
  lviv_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = env->getLogger();

  stats_ = new PRStats();
  stats_->imprvPt = 0;
  stats_->infPt = 0;
  stats_->cuts = 0;
}


void PerspCutHandler::findPRCons()
{
  PerspConPtr prc = (PerspConPtr) new PerspCon(minlp_, env_);
  prc->findPRCons();
  if (prc->getStatus()) {
    cons_ = prc->getPRCons();
    binvar_ = prc->getPRBinVar();
    sType_ = prc->getPRStruct();
    nviv_ = prc->getXNV();
    lviv_ = prc->getXLV();
  }
}


bool PerspCutHandler::getStatus()
{
   if (cons_.size() > 0) {
     return true;   
   } else {
     return false;   
   }
}


void PerspCutHandler::linearAt(RelaxationPtr rel, const double *y, double *c,
                        LinearFunctionPtr *lf, UInt itn, int *error)
{
  ConstraintPtr cp = cons_[itn];
  FunctionPtr f = cp->getFunction();
  UInt nvars = minlp_->getNumVars();
  double * cgrad = new double[nvars];
  std::fill(cgrad, cgrad + nvars, 0.0);
  
  *c = 0;
  f->evalGradient(y, cgrad, error); 
  if (*error == 0) {
    double conseval = 0, val;
    VariableGroupConstIterator mit;
    VariablePtr bp = rel->getRelaxationVar(binvar_[itn]), v;
    UInt binindex = bp->getIndex(), varindex;
    const LinearFunctionPtr lfn = cp->getLinearFunction();
    const QuadraticFunctionPtr qfn = cp->getQuadraticFunction();
    const NonlinearFunctionPtr nlfn = cp->getNonlinearFunction();

    *lf = (LinearFunctionPtr) new LinearFunction(); 
    if (nlfn) {
      for (VarSetConstIterator vt=nlfn->varsBegin(); vt!=nlfn->varsEnd();
           ++vt) {
        v = rel->getRelaxationVar(*vt);
        varindex = v->getIndex();
        (*lf)->addTerm(v, cgrad[varindex]);
        mit = nviv_[itn].find(v);
        if (mit != nviv_[itn].end()) {
          val = mit->second;     
        } else {
          val = 0;          
          //MS: error message or some message.
        }
        conseval += (cgrad[varindex] 
                     *((val-y[varindex])/y[binindex])); 
        *c -= (cgrad[varindex] *val); 
      }
    } else if (qfn) {
      for (VarIntMapConstIterator vt=qfn->varsBegin(); vt!=qfn->varsEnd();
           ++vt) {
        v = rel->getRelaxationVar(vt->first);
        varindex = v->getIndex();
        (*lf)->addTerm(v, cgrad[varindex]);
        mit = nviv_[itn].find(v);
        if (mit != nviv_[itn].end()) {
          val = mit->second;     
        } else {
          val = 0;          
        }
        conseval += (cgrad[varindex] 
                     *((val-y[varindex])/y[binindex])); 
        *c -= (cgrad[varindex] *val);
      }
    }
    conseval-=cp->getUb();
    if (-1 <= sType_[itn] && sType_[itn] <= 1) {
      for (VariableGroupConstIterator vt=lfn->termsBegin();
           vt!=lfn->termsEnd();
           ++vt) {
        v = rel->getRelaxationVar(vt->first);
        if (!((*lf)->hasVar(v))) {
          (*lf)->addTerm(v, vt->second);
          conseval += vt->second;
        }
      }
      conseval += nlfn->eval(y, error);
    } else {
      conseval += f->eval(y, error);
      for (VariableGroupConstIterator vt=lfn->termsBegin();
           vt!=lfn->termsEnd();
           ++vt) {
        v = rel->getRelaxationVar(vt->first);
        varindex = v->getIndex();
        if (varindex != binindex) {
          if (!((*lf)->hasVar(v))) {
            (*lf)->addTerm(v, cgrad[varindex]);
            mit = lviv_[itn].find(v);
            if (mit != lviv_[itn].end()) {
              val = mit->second;     
            } else {
              val = 0;         //MS: error message here 
            }
            conseval += (cgrad[varindex] 
                       *((val-y[varindex])/y[binindex])); 
            *c -= (cgrad[varindex] *val);
          }
        }
      }
    }
  } else {
    logger_->msgStream(LogError) << me_ <<" Gradient of the"
      <<  " function is not defined at the current point" <<std::endl;
  }
  delete [] cgrad;
  return;
}

//void PerspCutHandler::linearAt(RelaxationPtr rel, const double *y, double *c,
                        //LinearFunctionPtr *lf, UInt itn, int *error)
//{
  //ConstraintPtr cp = cons_[itn];
  //FunctionPtr f = cp->getFunction();
  //UInt nvars = minlp_->getNumVars();
  //double * cgrad = new double[nvars];
  //std::fill(cgrad, cgrad + nvars, 0.0);
  //f->evalGradient(y, cgrad, error);
 
  //*c = 0;
  //if (*error == 0) {
    //double conseval, val;
    //VariableGroupConstIterator mit;
    //VariablePtr bp = rel->getRelaxationVar(binvar_[itn]), v;
    //UInt binindex = bp->getIndex(), varindex;
    //const LinearFunctionPtr lfn = cp->getLinearFunction();
    //const QuadraticFunctionPtr qfn = cp->getQuadraticFunction();
    //const NonlinearFunctionPtr nlfn = cp->getNonlinearFunction();

    //*lf = (LinearFunctionPtr) new LinearFunction(); 
    //if (1 <= sType_[itn] && sType_[itn] <= 3) {
      //for (VariableGroupConstIterator vt=lfn->termsBegin();
           //vt!=lfn->termsEnd();
           //++vt) {
        //v = rel->getRelaxationVar(vt->first);
        //(*lf)->addTerm(v, vt->second);
      //}
      //conseval = nlfn->eval(y, error);
    //} else {
      //conseval = f->eval(y, error);
    //}
    //if (*error == 0) {
      //if (nlfn) {
        //for (VarSetConstIterator vt=nlfn->varsBegin(); vt!=nlfn->varsEnd();
             //++vt) {
          //v = rel->getRelaxationVar(*vt);
          //varindex = v->getIndex();
          //(*lf)->addTerm(v, cgrad[varindex]);
          //mit = nviv_[itn].find(v);
          //if (mit != nviv_[itn].end()) {
            //val = mit->second;     
          //} else {
            //val = 0;          
            ////MS: error message or some message.
          //}
          //conseval += (cgrad[varindex] 
                       //*((val-y[varindex])/y[binindex])); 
          //*c -= (cgrad[varindex] *val); 
        //}
      //} else if (qfn) {
        //for (VarIntMapConstIterator vt=qfn->varsBegin(); vt!=qfn->varsEnd();
             //++vt) {
          //v = rel->getRelaxationVar(vt->first);
          //varindex = v->getIndex();
          //(*lf)->addTerm(v, cgrad[varindex]);
          //mit = nviv_[itn].find(v);
          //if (mit != nviv_[itn].end()) {
            //val = mit->second;     
          //} else {
            //val = 0;          
          //}
          //conseval += (cgrad[varindex] 
                       //*((val-y[varindex])/y[binindex])); 
          //*c -= (cgrad[varindex] *val);
        //}
      //}
  
      //if (lfn && (sType_[itn] > 3 || sType_[itn] == 0)) {
        //for (VariableGroupConstIterator vt=lfn->termsBegin();
             //vt!=lfn->termsEnd();
             //++vt) {
          //v = rel->getRelaxationVar(vt->first);
          //varindex = v->getIndex();
          //if(varindex != binindex){
            //(*lf)->addTerm(v, cgrad[varindex]);
            //mit = lviv_[itn].find(v);
            //if (mit != lviv_[itn].end()) {
              //val = mit->second;     
            //} else {
              //val = 0;          
            //}
            //conseval += (cgrad[varindex] 
                         //*((val-y[varindex])/y[binindex])); 
            //*c -= (cgrad[varindex] *val);
          //}
        //}
      //}   
      //conseval-=cp->getUb();
      //(*lf)->addTerm(bp, conseval);
    //} else {
      //logger_->msgStream(LogError) << me_ <<" function is not defined "
        //<< "at the current point" <<std::endl;
    //}
  //} else {
    //logger_->msgStream(LogError) << me_ <<" Gradient of the"
      //<<  " function is not defined at the current point" <<std::endl;
  //}
  //delete [] cgrad;
  //return;
//}    


bool PerspCutHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, 
                                 bool &, double & )
{
  int error=0;
  FunctionPtr f;
  ConstraintPtr c;
  double act, sb, cUb;
  const double *x = sol->getPrimal();
  UInt binindex, nvars = minlp_->getNumVars();
  
  double * y = new double[nvars];
  for (UInt it=0; it<cons_.size(); ++it) { 
    binindex = binvar_[it]->getIndex();
    sb = x[binindex];
    if (fabs(sb) < intTol_) {
      continue;
    }
    c = cons_[it];
    f = c->getFunction();
    if (fabs(1-sb) > intTol_) {  
      prVars(x, y, it, &error);
      if (error == 0) {
        act = sb*(f->eval(y, &error));
      } else {
      //MS: add assertion that feasibility cannot be checked. Similarly, in
      //QGHandler.
      }
    } else {
      act = sb*(f->eval(x, &error));   
    }
    cUb = c->getUb();
    if (error==0) {
      if (act>cUb*sb+solAbsTol_ || (cUb!=0 
                                    && act>cUb*sb+fabs(cUb*sb)*solRelTol_)) {
        delete [] y;
        return false;        
      }      
    }	else {
      delete [] y;
      return false;
    }
  }
  delete [] y;
  return true; 
}

void PerspCutHandler::prVars(const double *x, double *y, UInt itn,
                             int *error)
{
  VariablePtr v;
  ConstraintPtr c = cons_[itn];
  VariableGroupConstIterator mit;
  UInt indexvar, binindex, nvars = minlp_->getNumVars();
  const LinearFunctionPtr lf = c->getLinearFunction();
  const QuadraticFunctionPtr qf = c->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = c->getNonlinearFunction();
  
  // Generate solution y=(x/z,1), x are continuous and z is binary variable
  std::fill(y, y + nvars, 0);
  binindex = binvar_[itn]->getIndex();
  if (nlf) {
    for (VarSetConstIterator vt=nlf->varsBegin(); vt!=nlf->varsEnd();
         ++vt) {
      v = *vt;
      indexvar = v->getIndex();
      mit = nviv_[itn].find(v);
      if (mit != nviv_[itn].end()) {
        y[indexvar] = (x[indexvar] - (1-x[binindex])*(mit->second))/x[binindex];
      } else {
#if SPEW
        logger_->msgStream(LogDebug) << me_ <<"Fixed value of variable" <<
         v->getName() << " on fixing binary variable is unknown." << std::endl;
#endif
        *error = 1;
        return;      
      }
    }
  } else if (qf) {
    for (VarIntMapConstIterator vt=qf->varsBegin(); vt!=qf->varsEnd();
         ++vt) {
      v = vt->first;
      indexvar = v->getIndex();
      mit = nviv_[itn].find(v);
      if (mit != nviv_[itn].end()) {
        y[indexvar] = (x[indexvar] - (1-x[binindex])*(mit->second))/x[binindex];
      } else {
        *error = 1;
        return;      
      }
    }
  }

  if (lf) {
    if (sType_[itn] > 1) {
      for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           ++vt) {
        v = vt->first;
        indexvar = v->getIndex();
        mit = lviv_[itn].find(v);
        if (mit != lviv_[itn].end()) {
          y[indexvar] = (x[indexvar] - (1-x[binindex])*(mit->second))/x[binindex];
        } else {
          *error = 1;
          return;      
        }
      }
    } else {
      for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           ++vt) {
        v = vt->first;
        indexvar = v->getIndex();
        y[indexvar] = x[indexvar]/x[binindex];
      }
    }
  }   
  y[binindex] = 1;
  return; 
}


void PerspCutHandler::linearAt(RelaxationPtr rel, FunctionPtr f, double fval,
                         const double *x, double *c, LinearFunctionPtr *lf,
                         int *error)
{
  int n = minlp_->getNumVars();
  double *a = new double[n];
  const double linCoeffTol = 1e-6; //MS: remove hardcode
  VariableConstIterator vbeg = rel->varsBegin(), vend = rel->varsEnd();

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

void PerspCutHandler::pcutAtInt(RelaxationPtr rel , const double *nlpx, 
                         const double *lpx, SeparationStatus *pcStatus, CutManager *)
{
  int error=0;
  ConstraintPtr con;
  LinearFunctionPtr lf; 
  double c, vio, nlpact, cUb;
  *pcStatus = SepaContinue;

  for (UInt it=0; it<cons_.size(); ++it) {
    con = cons_[it]; 
    nlpact = con->getActivity(lpx, &error);
    if (error == 0) {
      cUb = con->getUb();
      vio = std::max(nlpact-cUb, 0.0);
      if (vio > solAbsTol_ || (cUb!=0 && vio > fabs(cUb)*solRelTol_)) {
        lf = LinearFunctionPtr(); 
        intPCut(rel, &lf, it, nlpx, &c, &error); 
        if (error == 0) { 
          vio = std::max(lf->eval(lpx)-c, 0.0); //MS: is c 0??
          if (vio>solAbsTol_ || (c!=0 && vio>fabs(c)*solRelTol_)) {
            addCutToRel(rel, lf, c);
            *pcStatus = SepaResolve;  
          }
        }
      }
    }	
  }
  return;
}


void PerspCutHandler::pcutAtPoint(RelaxationPtr rel, const double *x)
{
  int error;
  FunctionPtr f;
  ConstraintPtr cptr;
  double c, act, cUb, sb;
  LinearFunctionPtr lf = LinearFunctionPtr();
  double *y = new double[minlp_->getNumVars()];

  for (UInt it = 0; it < cons_.size(); ++it) {
    error=0;
    cptr = cons_[it];
    cUb = cptr->getUb();
    lf = LinearFunctionPtr();
    sb = x[binvar_[it]->getIndex()];
    if (fabs(sb - floor(sb+0.5)) < intTol_) {  
      intPCut(rel, &lf, it, x, &c, &error); 
      if (error == 0) {
        addCutToRel(rel, lf, c);
      }
      continue;
    }
    
    prVars(x, y, it, &error);
    if (error == 0) {
      f = cptr->getFunction();
      act = (f->eval(y, &error));
      if (error==0) {
        if (sb*act>cUb*sb+solAbsTol_) {
          if (bisecPt(x, y, it)) {
            ++(stats_->imprvPt);
            linearAt(rel, y, &c, &lf, it, &error);
            if (error==0) {
              addCutToRel(rel, lf, c);
            } 
          }
        } else {
          linearAt(rel, y, &c, &lf, it, &error);
          if (error == 0) {
            addCutToRel(rel, lf, c);
          }
        }
      }  
    }

  }
  delete [] y;
  return;
}

void PerspCutHandler::intPCut(RelaxationPtr rel, LinearFunctionPtr *lf, UInt it,
                              const double *x, double *c, int *error)
{
  double act;
  ConstraintPtr cptr = cons_[it];
  FunctionPtr f = cptr->getFunction();
  if (x[binvar_[it]->getIndex()] < 0.5) {
    act = f->eval(x, error);
    if (*error==0) {
      linearAt(rel, f, act, x, c, lf, error); 
      *c = cptr->getUb()-*c;
    }
  } else {
    linearAt(rel, x, c, lf, it, error);
  }
  return;
}


//void PerspCutHandler::addCutOriFeas(RelaxationPtr rel, UInt it, const double *x)
//{
  //int error=0;
  //FunctionPtr f;
  //ConstraintPtr cptr = cons_[it];
  //LinearFunctionPtr lf = LinearFunctionPtr();
  //double *y = new double[minlp_->getNumVars()];
  //double c, act, cUb = cptr->getUb(), sb = x[binvar_[it]->getIndex()];

  //if (fabs(sb - floor(sb+0.5)) < intTol_) {  
    //intPCut(rel, it, x, y, &c, sb, &lf, &error); 
    //if (error == 0) {
      //addCutToRel(rel, lf, c);
    //}
    //delete [] y;
    //return;
  //}
  
  //prVars(x, y, it);
  //f = cptr->getFunction();
  //act = sb*(f->eval(y, &error));
  //if (error==0) {
    //if (act>cUb*sb+solAbsTol_) {
      //bisecPt(x, y, it);
      //++(stats_->imprvPt);
      //if (y) {
        //linearAt(rel, y, &c, &lf, it, &error);
        //if (error==0) {
          //addCutToRel(rel, lf, c);
        //} 
      //}
    //}
  //}  
  //delete [] y;
  //return;
//}


void PerspCutHandler::addCutToRel(RelaxationPtr rel, LinearFunctionPtr lf, double c)
{
  std::stringstream sstm;
  FunctionPtr f =  (FunctionPtr) new Function(lf);
  
  ++(stats_->cuts);  
  sstm << "_PCut_" << stats_->cuts;     
  rel->newConstraint(f, -INFINITY, c, sstm.str()); 
  //ConstraintPtr cptr;
  //cptr = rel->newConstraint(f, -INFINITY, c, sstm.str()); 
  return;
}


void PerspCutHandler::addPCut(RelaxationPtr rel, UInt it, const double *x)
{
  int error=0;
  FunctionPtr f;
  std::stringstream sstm;
  ConstraintPtr cptr = cons_[it];
  LinearFunctionPtr lf = LinearFunctionPtr();
  double * y = new double[minlp_->getNumVars()];
  double c, act, cUb = cptr->getUb(), sb = x[binvar_[it]->getIndex()];

  if (fabs(sb - floor(sb+0.5)) < intTol_) {  
    intPCut(rel, &lf, it, x, &c, &error); 
    if (error == 0) {
      addCutToRel(rel, lf, c);
    }
    delete [] y;
    return;
  }

  f = cptr->getFunction();
  act = f->eval(x, &error);
  if (error == 0) {
    if (act>cUb+solAbsTol_ || (cUb!=0 && act>cUb+fabs(cUb)*solRelTol_)) {
      ++(stats_->infPt);
      feasBisecPt(x, y, it); //MS: check how many times this happen and then modify
    } else {
      prVars(x, y, it, &error);
      if (error == 0) {
        act = sb*(f->eval(y, &error));
        if (error==0) {
          if (act>cUb*sb+solAbsTol_) {
            bisecPt(x, y, it);
            ++(stats_->imprvPt);
          }
        } else {
          delete [] y;
          return;      
        }
      }
    }
  } else {
    delete [] y;
    return;  
  }
  if (y) {
    linearAt(rel, y, &c, &lf, it, &error);
    if (error==0) {
      addCutToRel(rel, lf, c);
    } 
  }
  delete [] y;
  return;
}

bool PerspCutHandler::bisecPt(const double *x, double * y, UInt it)
{
  int error=0;
  ConstraintPtr cPtr = cons_[it];
  const FunctionPtr f = cPtr->getFunction();
  UInt bId = binvar_[it]->getIndex();
  double cUb = cPtr->getUb(), zL = x[bId], zU = 1, zBi=0, temp = zL, act;

  while (true) {
    zBi = (zL+zU)/2;  //MS: check this logic
    for (VarSetConstIterator v=f->varsBegin(); v!=f->varsEnd(); ++v) {
      y[(*v)->getIndex()] = y[(*v)->getIndex()]*temp/zBi;    
    }
    y[bId] = 1; 
    temp = zBi;
    act = zBi*(f->eval(y, &error));
    if (error == 0) {
      if (act>cUb*zBi+solAbsTol_ || (cUb!=0 && 
                                     act>cUb*zBi+fabs(cUb*zBi)*solRelTol_)) {
         zL = zBi;        
      } else {
        if (cUb*zBi-act<=solAbsTol_ && 
            (cUb==0 || cUb*zBi-act<=fabs(cUb*zBi)*solRelTol_)) {
            break;
        } else {
          zU = zBi;        
        }
      }
    } else {
      return false;
      break;
    }
  }
  std::cout << "constraint no. " << it << " z value " << zBi << std::endl;
  return true;
}

void PerspCutHandler::bisecPtOri(const double *x, double * w, double * w1, UInt it) 
{
  int error=0;
  ConstraintPtr cPtr = cons_[it];
  double cUb = cPtr->getUb(), act;
  UInt nvars = minlp_->getNumVars(), vId;
  const FunctionPtr f = cPtr->getFunction();
  
  std::fill(w1, w1 + nvars, 0);
  while (true) {
    for (VarSetConstIterator v=f->varsBegin(); v!=f->varsEnd(); ++v) {
      vId = (*v)->getIndex();
      w1[vId] = 0.5*x[vId] + 0.5*w[vId];    
    }
    act = f->eval(w1, &error);
    if (error==0) {
      if (act>cUb+solAbsTol_ || (cUb!=0 && act>cUb+fabs(cUb)*solRelTol_)) {
        x = w1;        
      } else {
        break;        
      }
    } else {
      w1 = NULL;
      break;
    }
  }
  return;
}


void PerspCutHandler::vertexPt(double * w, UInt it)
{
  for (VariableGroupConstIterator vt=nviv_[it].begin(); vt!=nviv_[it].end(); ++vt) {
      w[vt->first->getIndex()] = vt->second;
  }
  for (VariableGroupConstIterator vt=lviv_[it].begin(); vt!=lviv_[it].end(); ++vt) {
      w[vt->first->getIndex()] = vt->second;
  }
  return;
}


void PerspCutHandler::feasBisecPt(const double *x, double *y, UInt it)
{
  ConstraintPtr cPtr = cons_[it];
  UInt nvars = minlp_->getNumVars();
  double * w = new double[nvars];
  double * w1 = new double[nvars];
  std::fill(w, w + nvars, 0);
  vertexPt(w, it);
  bisecPtOri(x, w, w1, it);
  if (w1) {
    bisecPt(w1, y, it); 
  } else {
    y = NULL;  
  }
  delete [] w;
  delete [] w1;
  return;
}


void PerspCutHandler::separate(ConstSolutionPtr, NodePtr , RelaxationPtr , 
                             CutManager *, SolutionPoolPtr , ModVector & ,
                             ModVector & , bool *, SeparationStatus *)
{
  // do nothing.
}

void PerspCutHandler::writeStats(std::ostream &out) const
{
  //MS: last two statistics to be deleted later.
  if (cons_.size() > 0) {
    out
    << me_ << "number of cuts added        = " << stats_->cuts << std::endl
    << me_ << "number of improved points        = " << stats_->imprvPt << std::endl
    << me_ << "number of feasible cum improved points        = " << stats_->infPt << std::endl;
  }
  return;
}
 
std::string PerspCutHandler::getName() const
{
  return "PerspCutHandler";
}



PerspCutHandler::~PerspCutHandler()
{
  if (stats_) {
    delete stats_;
  }
  cons_.clear();
  prcons_.clear();
  binvar_.clear();
  sType_.clear();
  nviv_.clear();
  lviv_.clear();
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
