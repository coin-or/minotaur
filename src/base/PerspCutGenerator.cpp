//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
//
/**
 * \file PerspCutGenerator.cpp
 * \brief Declare PerspCutGenerator class.
 * \author Meenarli Sharma, Indian Institute of Technology Bombay
*/

#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


#include "AnalyticalCenter.h"
#include "PerspCutGenerator.h"
#include "LinearFunction.h"
#include "CutManager.h"
#include "Environment.h"
#include "NonlinearFunction.h"
#include "Objective.h"
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
const std::string PerspCutGenerator::me_ = "PerspCutGenerator: ";

PerspCutGenerator::PerspCutGenerator()
  : env_(EnvPtr()),  minlp_(ProblemPtr()), nlpe_(EnginePtr()), cvxPt(0),
  stats_(0), prCons_(0), solC_(0)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
}


PerspCutGenerator::PerspCutGenerator(EnvPtr env, ProblemPtr minlp,
                                     EnginePtr nlpe)
  : env_(env), minlp_(minlp), nlpe_(nlpe), cvxPt(0), prCons_(0), solC_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("feasAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("feasRel_tol")->getValue();
  objAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  objRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();

  logger_ = env->getLogger();
  stats_ = new PRStats();
  stats_->imprvPt = 0;
  stats_->infPt = 0;
  stats_->cuts = 0;
}


void PerspCutGenerator::fixedValue(double * prPt, bool isObj, UInt i)
{
  bool binval;
  VariablePtr v;
  UInt indexvar, binindex;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  VariableGroupConstIterator mit, mit1;

  if (isObj) {
    FunctionPtr f = minlp_->getObjective()->getFunction();
    binindex = prObj_.binVar->getIndex();
    binval = prObj_.binVal;
    qf = f->getQuadraticFunction();
    nlf = f->getNonlinearFunction();
    mit1 = (prObj_.nNonzeroVar).end();
  } else {
    binval = prCons_[i].binVal;
    binindex = prCons_[i].binVar->getIndex();
    ConstraintPtr c = prCons_[i].cons;
    qf = c->getQuadraticFunction();
    nlf = c->getNonlinearFunction();
    mit1 = (prCons_[i].nNonzeroVar).end();
  }


  if (nlf) {
    for (VarSetConstIterator vt = nlf->varsBegin(); vt != nlf->varsEnd();
         ++vt) {
      v = *vt;
      indexvar = v->getIndex();
      if (indexvar == binindex) {
        continue;
      }
      if (isObj) {
        mit = (prObj_.nNonzeroVar).find(v);
      } else {
        mit = (prCons_[i].nNonzeroVar).find(v);
      }

      if (mit != mit1) {
        prPt[indexvar] = mit->second;
      } else {
        prPt[indexvar] = 0;
      }
    }
  }

  if (qf) {
    for (VarIntMapConstIterator vt = qf->varsBegin(); vt != qf->varsEnd();
         ++vt) {
      v = vt->first;
      indexvar = v->getIndex();
      if ((nlf && nlf->hasVar(v)) || (indexvar == binindex)) {
        continue;
      }
      if (isObj) {
        mit = (prObj_.nNonzeroVar).find(v);
      } else {
        mit = (prCons_[i].nNonzeroVar).find(v);
      }

      if (mit != mit1) {
        prPt[indexvar] = mit->second;
      } else {
        prPt[indexvar] = 0;
      }
    }
  }

  //if (lf) {
    //VariableGroupConstIterator mit;
    //if (type != 2) { // either 1 or 3
      //for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           //++vt) {
        //v = vt->first;
        //indexvar = v->getIndex();
        //if ((nlf && nlf->hasVar(v)) ||
            //(qf && qf->hasVar(v)  ||
            //(indexvar == binindex)) {
            //continue;
        //}

        //mit = (prCons_[itn].lNonzeroVar).find(v);
        //if (mit != ((prCons_[itn].lNonzeroVar).end())) {
          //prPt[indexvar] =  mit->second;
        //} else {
          //prPt[indexvar] = 0;
        //}
      //}
    //} else {
      //for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           //++vt) {
        //v = vt->first;
        //indexvar = v->getIndex();
        //prPt[indexvar] = 0;
      //}
    //}
  //}

  if (!binval) {
    prPt[binindex] = 0;
  } else {
    prPt[binindex] = 1;
  }
  return;
}


void PerspCutGenerator::findPRCons()
{
  PerspConPtr prc = (PerspConPtr) new PerspCon(env_, minlp_);
  prc->findPRCons();
  if (prc->getStatus()) {
    prObj_ = prc->getPRObj();
    prCons_ = prc->getPRCons();
  }

  if (env_->getOptions()->findBool("perspective_cuts")->getValue()) {
    if (prCons_.size()) {
      for (UInt i = 0; i != prCons_.size(); ++i) {
        if (!(prCons_[i].bisect)) {
          // find analytical center
          //std::stack<Modification *> varMods;
          //std::cout << "Before applying changes \n";
          //minlp_->write(std::cout);
          //fixBinVars_(varMods);
          //std::cout << "After fixing vars \n";
          //minlp_->write(std::cout);
          AnalyticalCenterPtr ac = (AnalyticalCenterPtr) new AnalyticalCenter(env_, minlp_, nlpe_);
          ac->modifyOnlyNonlinear(solC_);
          //unfixBinVars_(varMods);
          //std::cout << "After unfixing vars \n";
          //minlp_->write(std::cout);
          delete ac;
          break;
        }
      }
    } else {
      if (prObj_.isPR) {
        solC_ = new double[minlp_->getNumVars()];
        AnalyticalCenterPtr ac = (AnalyticalCenterPtr) new AnalyticalCenter(env_, minlp_, nlpe_);
        ac->modifyWhole(solC_);
        delete ac;
      }
    }
  }
  delete prc;
  return;
}


//void PerspCutGenerator::fixBinVars_(std::stack<Modification *> &varMods)
//{
  //double val;
  //VariablePtr v;
  //VarBoundMod2 *m = 0;

  //for (UInt i = 0; i < prCons_.size(); ++i) {
    //v = prCons_[i].binVar;

    //if (prCons_[i].binVal) {
      //val = 0;
    //} else {
      //val = 1;
    //}

    //if (v->getLb() != v->getUb()) {
      //std::cout << "fixing " << v->getName() << "\n";
      //m = new VarBoundMod2(v, val, val);
      //m->applyToProblem(minlp_);
      //varMods.push(m);
    //}
  //}
  //return;
//}


//void PerspCutGenerator::unfixBinVars_(std::stack<Modification *> &varMods)
//{
  //ModificationPtr m = 0;
  //while(varMods.empty() == false) {
    //m = varMods.top();
    //m->undoToProblem(minlp_);
    //varMods.pop();
    //delete m;
  //}
  //return;
//}


bool PerspCutGenerator::getStatus()
{
   if (prCons_.size() > 0 || prObj_.isPR) {
     return true;
   } else {
     return false;
   }
}


void PerspCutGenerator::linearAt(RelaxationPtr rel, const double * x,
                               const double *prPt, double *c,
                               LinearFunctionPtr *lf, UInt itn, int *error,
                               bool isObj)
{
  FunctionPtr f;
  ObjectivePtr o;
  ConstraintPtr cp;
  UInt type, binindex, varindex;

  if (isObj) {
    o = minlp_->getObjective();
    f = o->getFunction();
  } else {
    cp = prCons_[itn].cons;
    f = cp->getFunction();
  }

  UInt nvars = minlp_->getNumVars();
  double * cgrad = new double[nvars];
  std::fill(cgrad, cgrad + nvars, 0.0);

  //*c = 0;
  f->evalGradient(prPt, cgrad, error);

  if (*error == 0) {
    bool binval;
    VariablePtr bp, v;
    LinearFunctionPtr lfn;
    QuadraticFunctionPtr qfn;
    NonlinearFunctionPtr nlfn;
    double zCoeff = 0, val, sb, ub;
    VariableGroupConstIterator mit, mit1;
    *lf = (LinearFunctionPtr) new LinearFunction();

    if (isObj) {
      type = 2;
      bp = prObj_.binVar;
      binval = prObj_.binVal;
      ub = -1*(o->getConstant());
      lfn = f->getLinearFunction();
      qfn = f->getQuadraticFunction();
      nlfn = f->getNonlinearFunction();
      mit1 = (prObj_.nNonzeroVar).end();
      //bp = (rel->getRelaxationVar(prObj_.binVar));
      binindex = bp->getIndex();
    } else {
      ub = cp->getUb();
      bp = prCons_[itn].binVar;
      type = prCons_[itn].type;
      binval = prCons_[itn].binVal;
      mit1 = (prCons_[itn].nNonzeroVar).end();
      f = cp->getFunction();
      lfn = cp->getLinearFunction();
      qfn = cp->getQuadraticFunction();
      nlfn = cp->getNonlinearFunction();
      binindex = bp->getIndex();
      //bp = (rel->getRelaxationVar(prCons_[itn].binVar));
    }

    if (binval) {
      sb = 1 - x[binindex];
    } else {
      sb = x[binindex];
    }

    if (nlfn) {
      for (VarSetConstIterator vt=nlfn->varsBegin(); vt!=nlfn->varsEnd();
           ++vt) {
        v = *vt;
        //v = rel->getRelaxationVar(*vt);
        if (v == bp) {
          continue;
        }
        if (isObj) {
          mit = (prObj_.nNonzeroVar).find(v);
        } else {
          mit = (prCons_[itn].nNonzeroVar).find(v);
        }

        if (mit != mit1) {
          val = mit->second;
        } else {
          val = 0;
        }

        //varindex = (rel->getRelaxationVar(v))->getIndex();
        varindex = v->getIndex();
        (*lf)->addTerm(rel->getRelaxationVar(v), cgrad[varindex]);

        *c -= cgrad[varindex]*val;
        zCoeff += (cgrad[varindex]*((val-x[varindex])/sb));
      }
      val = nlfn->eval(prPt, error);
      if (*error == 0) {
        zCoeff += val;
      } else {
        lf = 0;
        delete [] cgrad;
        return;
      }
    }

    if (qfn) {
      for (VarIntMapConstIterator vt=qfn->varsBegin(); vt!=qfn->varsEnd();
           ++vt) {
        //v = rel->getRelaxationVar(vt->first);
        v = vt->first;
        if (v == bp || (*lf)->hasVar(rel->getRelaxationVar(v))) {
          continue;
        }

        if (isObj) {
          mit = (prObj_.nNonzeroVar).find(v);
        } else {
          mit = (prCons_[itn].nNonzeroVar).find(v);
        }

        if (mit != mit1) {
          val = mit->second;
        } else {
          val = 0;
        }

        //varindex = (rel->getRelaxationVar(v))->getIndex();
        varindex = v->getIndex();
        (*lf)->addTerm(rel->getRelaxationVar(v), cgrad[varindex]);

        *c -= (cgrad[varindex] *val);
        zCoeff += (cgrad[varindex]*((val-x[varindex])/sb));
      }
      zCoeff += qfn->eval(prPt);
    }
    zCoeff -= ub;

    if (lfn) {
      if (type != 2) { // either 1 or 3
        for (VariableGroupConstIterator vt=lfn->termsBegin();
             vt!=lfn->termsEnd(); ++vt) {
          v = vt->first;
          //v = rel->getRelaxationVar(vt->first);
          if ((*lf)->hasVar(rel->getRelaxationVar(v)) || v == bp) {
            continue;
          } else {
            mit = (prCons_[itn].lNonzeroVar).find(v);
            if (mit != ((prCons_[itn].lNonzeroVar).end())) {
              val = mit->second;
            } else {
              val = 0;
            }

            varindex = v->getIndex();
            //varindex = (rel->getRelaxationVar(v))->getIndex();
            (*lf)->addTerm(rel->getRelaxationVar(v), cgrad[varindex]);
            *c -= (cgrad[varindex] *val);
            zCoeff += (cgrad[varindex]   *((val-x[varindex])/sb));
          }
        }
        zCoeff += lfn->eval(prPt);
      } else {
        for (VariableGroupConstIterator vt=lfn->termsBegin();
             vt!=lfn->termsEnd();
             ++vt) {
          v = vt->first;
          //v = rel->getRelaxationVar(vt->first);
          if ((*lf)->hasVar(rel->getRelaxationVar(v)) || v == bp) {
            continue;
          } else {
            (*lf)->addTerm(rel->getRelaxationVar(v), vt->second);
          }
        }
      }
    }

    if (binval) {
      *c -= zCoeff;
    }
    (*lf)->addTerm(rel->getRelaxationVar(bp), zCoeff);
  }

  delete [] cgrad;
  return;
}

//bool PerspCutGenerator::isFeasible(const double *x, UInt i, bool isObj,
                                   //double relVal)
//{
  //bool binval;
  //int error = 0;
  //FunctionPtr f;
  //UInt binindex;
  //double act, sb, ub, c0, aTol, rTol;

  //if (isObj) {
    //ub = relVal;
    //aTol = objAbsTol_;
    //rTol = objRelTol_;
    //binval = prObj_.binVal;
    //ObjectivePtr o = minlp_->getObjective();
    //f = o->getFunction();
    //c0 = o->getConstant();
    //binindex = (prObj_.binVar)->getIndex();
  //} else {
    //c0 = 0;
    //aTol = solAbsTol_;
    //rTol = solRelTol_;
    //ConstraintPtr cp = prCons_[i].cons;
    //ub = cp->getUb();
    //f = cp->getFunction();
    //binval = prCons_[i].binVal;
    //binindex = (prCons_[i].binVar)->getIndex();
  //}

  //if (binval) {
    //sb = 1 - x[binindex];
  //} else {
    //sb = x[binindex];
  //}

  //if (fabs(sb) < intTol_) {
    //return true;
  //} else {
    //act = f->eval(x, &error) + c0;
    //if (error == 0) {
      //if ((act >= ub + aTol) || (ub != 0 &&
                                    //act >= ub + fabs(ub)*rTol)) {
        //return false;
      //}
    //}	else {
      //return false;
    //}
  //}

  //return true;
//}


UInt PerspCutGenerator::isFeasible(const double *x, UInt i, bool isObj,
                                   double relVal)
{
  bool binval;
  int error = 0;
  FunctionPtr f;
  UInt binindex;
  double act, sb, ub, c0, aTol, rTol;

  if (isObj) {
    ub = relVal;
    aTol = objAbsTol_;
    rTol = objRelTol_;
    binval = prObj_.binVal;
    ObjectivePtr o = minlp_->getObjective();
    f = o->getFunction();
    c0 = o->getConstant();
    binindex = (prObj_.binVar)->getIndex();
  } else {
    c0 = 0;
    aTol = solAbsTol_;
    rTol = solRelTol_;
    ConstraintPtr cp = prCons_[i].cons;
    ub = cp->getUb();
    f = cp->getFunction();
    binval = prCons_[i].binVal;
    binindex = (prCons_[i].binVar)->getIndex();
  }

  if (binval) {
    sb = 1 - x[binindex];
  } else {
    sb = x[binindex];
  }

  if (fabs(sb) < intTol_) {
    return 1; // active
  } else {
    act = f->eval(x, &error) + c0;
    if (error == 0) {
      if ((act >= ub + aTol) || (ub != 0 &&
                                    act >= ub + fabs(ub)*rTol)) {
        return 0; // infeasible
      } else if ((fabs(act-ub) < solAbsTol_) && (ub == 0 ||
                                      (fabs(act-ub) < fabs(ub)*solRelTol_))) {

        return 1; // active
      } else {
        return 2; //interior
      }
    }	else {
      return 0; // infeasible
    }
  }

  return 0;
}


bool PerspCutGenerator::changeVarForm(VariablePtr v, double xc, double sb,
                                    UInt itn, double *newPt, bool isObj)
{
  VariableGroupConstIterator mit, mit1;
  if (isObj) {
    mit = (prObj_.nNonzeroVar).find(v);
    mit1 = (prObj_.nNonzeroVar).end();
  } else {
    mit = (prCons_[itn].nNonzeroVar).find(v);
    mit1 = (prCons_[itn].nNonzeroVar).end();
  }

  if (mit != mit1) {
    *newPt = ((xc - (1-sb)*(mit->second))/sb);
    return true;
  } else {
    *newPt = xc/sb;
    return true;
  }

  return false;
}

// Returns the same point if z is 1 for case in which z = 0 fixes the x
// variables. Similarly, if z = 0, in z = 1 case.
bool PerspCutGenerator::prVars(const double *x, double *prPt, UInt itn,
                               bool isObj)
{
  bool isFound;
  VariablePtr v;
  double sb, binval;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  UInt indexvar, binindex, type;

  if (isObj) {
    binval = prObj_.binVal;
    binindex = (prObj_.binVar)->getIndex();

  } else {
    binval = prCons_[itn].binVal;
    binindex = (prCons_[itn].binVar)->getIndex();
  }

  if (binval) {
    sb = 1 - x[binindex];
  } else {
    sb = x[binindex];
  }

  if (fabs(sb) < intTol_) {
    return false;
  } else if (fabs(1-sb) < intTol_) {
    std::copy(x, x+minlp_->getNumVars(), prPt);
    return true;
  }

  if (isObj) {
    type = 2;
    FunctionPtr f = minlp_->getObjective()->getFunction();
    lf = f->getLinearFunction();
    qf = f->getQuadraticFunction();
    nlf = f->getNonlinearFunction();
  } else {
    type = prCons_[itn].type;
    ConstraintPtr c = prCons_[itn].cons;
    lf = c->getLinearFunction();
    qf = c->getQuadraticFunction();
    nlf = c->getNonlinearFunction();
  }

  // Generate solution prPt=((x-(1-z)\hat{x})/z,1), x are continuous and z is binary
  // variable, z = 0 case
  std::fill(prPt, prPt + minlp_->getNumVars(), 0);
  if (nlf) {
    for (VarSetConstIterator vt = nlf->varsBegin(); vt != nlf->varsEnd();
         ++vt) {
      v = *vt;
      indexvar = v->getIndex();
      isFound = changeVarForm(v, x[indexvar], sb, itn, &(prPt[indexvar]), isObj);
      if (!isFound) {
        return false;
      }
    }
  }

  if (qf) {
    for (VarIntMapConstIterator vt = qf->varsBegin(); vt != qf->varsEnd();
         ++vt) {
      v = vt->first;
      indexvar = v->getIndex();
      isFound = changeVarForm(v, x[indexvar], sb, itn, &(prPt[indexvar]), isObj);
      if (!isFound) {
        return false;
      }
    }
  }

  if (lf) {
    VariableGroupConstIterator mit;
    if (type != 2) { // either 1 or 3
      for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           ++vt) {
        v = vt->first;
        indexvar = v->getIndex();
        mit = (prCons_[itn].lNonzeroVar).find(v);
        if (mit != ((prCons_[itn].lNonzeroVar).end())) {
          prPt[indexvar] =  ((x[indexvar] - (1-sb)*(mit->second))/sb);
        } else {
          prPt[indexvar] =  (x[indexvar]/sb);
        }
      }
    } else {
      for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd();
           ++vt) {
        v = vt->first;
        indexvar = v->getIndex();
        prPt[indexvar] = x[indexvar]/sb;
      }
    }
  }

  if (binval) {
    prPt[binindex] = 0;
  } else {
    prPt[binindex] = 1;
  }
  return true;
}


//void PerspCutGenerator::linearAt(RelaxationPtr rel, FunctionPtr f, double fval,
                         //const double *x, double *c, LinearFunctionPtr *lf,
                         //int *error)
//{
  //int n = rel->getNumVars();
  //double *a = new double[n];
  //const double linCoeffTol = 1e-6;
  //VariableConstIterator vbeg = rel->varsBegin(), vend = rel->varsEnd();

  //std::fill(a, a+n, 0.);
  //f->evalGradient(x, a, error);

  //if (*error==0) {
    //*lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, linCoeffTol);
    //*c  = fval - InnerProduct(x, a, minlp_->getNumVars());
  //} else {
    //logger_->msgStream(LogError) << me_ <<" gradient not defined at this point."
      //<< std::endl;
//#if SPEW
    //logger_->msgStream(LogDebug) << me_ <<" gradient not defined at this point."
      //<< std::endl;
//#endif
  //}
  //delete [] a;
  //return;
//}


//void PerspCutGenerator::atIntPt(RelaxationPtr rel , const double *nlpx,
                         //const double *lpx, SeparationStatus *pcStatus,
                         //CutManager *)
//{
  //int error = 0;
  //ConstraintPtr con;
  //LinearFunctionPtr lf;
  //double c, vio, nlpact, cUb;
  //*pcStatus = SepaContinue;

  //for (UInt it = 0; it < cons_.size(); ++it) {
    //con = cons_[it];
    //nlpact = con->getActivity(lpx, &error);
    //if (error == 0) {
      //cUb = con->getUb();
      //vio = std::max(nlpact-cUb, 0.0);
      //if (vio > solAbsTol_ || (cUb!=0 && vio > fabs(cUb)*solRelTol_)) {
        //lf = LinearFunctionPtr();
        //getCutFun(rel, &lf, it, nlpx, &c, &error);
        //if (error == 0) {
          //vio = std::max(lf->eval(lpx)-c, 0.0); //MS: is c 0??
          //if (vio > solAbsTol_ || (c != 0 && vio > fabs(c)*solRelTol_)) {
            //addCutToRel(rel, lf, c);
            //*pcStatus = SepaResolve;
          //}
        //}
      //}
    //}
  //}
  //return;
//}


//void PerspCutGenerator::atPtx(RelaxationPtr rel, const double *x)
//{
  //int error;
  //FunctionPtr f;
  //ConstraintPtr con;
  //double act, cUb;
  //for (UInt it = 0; it < cons_.size(); ++it) {
    //error = 0;
    //con = cons_[it];
    //cUb = con->getUb();
    //f = con->getFunction();
    //act = f->eval(x, &error);
    //if (error == 0) {
      //if (act-cUb > solAbsTol_ ||
          //(cUb != 0 && act-cUb > fabs(cUb)*solRelTol_)) {
        //cvxCombPt(rel, x, it);
      //} else {
        //cutToCons(rel, x, it);
      //}
    //}
  //}
  //return;
//}


//void PerspCutGenerator::oriFeasPt(RelaxationPtr rel, const double *x)
//{
  //for (UInt it = 0; it < cons_.size(); ++it) {
    //cutToCons(rel, x, it);
  //}
  //return;
//}


//void PerspCutGenerator::cutToCons(RelaxationPtr rel, const double *x, UInt it)
//{
  //int error;
  //FunctionPtr f;
  //ConstraintPtr con;
  //double c, act, cUb, sb;
  //LinearFunctionPtr lf = LinearFunctionPtr();

  //error=0;
  //con = cons_[it];
  //cUb = con->getUb();
  //lf = LinearFunctionPtr();
  //sb = x[binvar_[it]->getIndex()];
  //if (fabs(sb - floor(sb+0.5)) <= intTol_) {
    //getCutFun(rel, &lf, it, x, &c, &error);
    //if (error == 0) {
      //addCutToRel(rel, lf, c);
    //}
    //return;
  //}

  //double *y = new double[minlp_->getNumVars()];
  //prVars(x, y, it, &error);
  //if (error == 0) {
    //f = con->getFunction();
    //act = (f->eval(y, &error));
    //if (error == 0) {
      //if (sb*act - cUb*sb > solAbsTol_) {
        ////MS: add bisec or convex combi based on structure type and generate
        ////point y to add cut
        //bisecPt(x, y, it); //MS: remove bool of this function
        ////cvxCombPt(x, y, it); //MS: modify it. add conditions appropriately
        //++(stats_->imprvPt);
      //}
      //linearAt(rel, y, &c, &lf, it, &error); // MS: add cut directly here
      //if (error == 0) {
        //addCutToRel(rel, lf, c);
      //}
    //}
  //}
  //delete [] y;
  //return;
//}


//void PerspCutGenerator::getCutFun(RelaxationPtr rel, LinearFunctionPtr *lf,
                                //UInt it, const double *x, double *c,
                                //int *error)
//{
  //double act;
  //FunctionPtr f = cons_[it]->getFunction();
  //if (x[binvar_[it]->getIndex()] < 0.5) {
    //act = f->eval(x, error);
    //if (*error == 0) {
      //linearAt(rel, f, act, x, c, lf, error);
      //*c = cons_[it]->getUb()-*c; //MS: pay attenetion why is it this way?
    //}
  //} else {
    //linearAt(rel, x, c, lf, it, error);
  //}
  //return;
//}


bool PerspCutGenerator::addCutToRel(RelaxationPtr rel, const double *ptToCut,
                                    LinearFunctionPtr lf, double ub,
                                    VariablePtr objVar, double relVal,
                                    CutManagerPtr)
{
  // Add cut to cutmanager is cutmanager is not NULL
  //ConstraintPtr newcon;
  std::stringstream sstm;
  // MS: check if ptToCut null case
  if (ptToCut) {
    double act;
    act = std::max(lf->eval(ptToCut) - relVal - ub, 0.0);
    if ((act >= solAbsTol_) &&
        (ub == 0 || (act >= fabs(ub)*solRelTol_))) {
      ++(stats_->cuts);
      if (objVar) {
        sstm << "_ObjPCut_" << stats_->cuts;
        lf->addTerm(objVar, -1.0);
      } else {
        sstm << "_PCut_" << stats_->cuts;
      }

      FunctionPtr f = (FunctionPtr) new Function(lf);
      rel->newConstraint(f, -INFINITY, ub, sstm.str());
      //newcon = rel->newConstraint(f, -INFINITY, ub, sstm.str());
      //std::cout <<"In PCut \n";
      //newcon->write(std::cout);
    } else {
      delete lf;
      lf = 0;
      return false;
    }
  } else {
    ++(stats_->cuts);
    if (objVar) {
      sstm << "_ObjPCut_" << stats_->cuts;
      lf->addTerm(objVar, -1.0);
    } else {
      sstm << "_PCut_" << stats_->cuts;
    }
    FunctionPtr f =  (FunctionPtr) new Function(lf);
    rel->newConstraint(f, -INFINITY, ub, sstm.str());
    //newcon = rel->newConstraint(f, -INFINITY, ub, sstm.str());
    //std::cout <<"In PCut \n";
    //newcon->write(std::cout);
  }
  return true;
}

bool PerspCutGenerator::uniDirZSearch(double *y, double * newPt, UInt i,
                                double zdir)
{
  int error = 0;
  bool isCont = true;
  ConstraintPtr cp = prCons_[i].cons;
  FunctionPtr f = cp->getFunction();
  double zU = zdir, zL, ub = cp->getUb(), act, lastPt = zU;
  UInt binindex = (prCons_[i].binVar)->getIndex(), ptCount = 0;

  zL = y[binindex];

  while (isCont) {
    y[binindex] = (zL + zU)/2;
    isCont = prVars(y, newPt, i, 0);
    if (isCont) {
      act = f->eval(newPt, &error);
      if (error == 0) {
        if ((act >= ub + solAbsTol_) || (ub != 0 &&
                                      act >= ub + fabs(ub)*solRelTol_)) {
          // newPt is still infeasible
          zU = y[binindex];
        } else if ((fabs(act-ub) < solAbsTol_) && (ub == 0 ||
                                      (fabs(act-ub) < fabs(ub)*solRelTol_))) {
          // NewPt found
          return true;
        } else {
          zL = y[binindex]; // interior
        }
      }	else {
        isCont = false;
      }
      if (lastPt - y[binindex] < 1e-8) {
        ++ptCount;
      } else {
        ptCount = 0;
      }
      if (ptCount == 10) {
        return true;
      }
    }
  }

  return false;
}


bool PerspCutGenerator::lineSearch_(double *y, double * newPt, UInt i,
                                bool isObj, double relVal)
{
  UInt vIdx;
  int error = 0;
  FunctionPtr f;
  bool isCont = true;
  double ub, c0, aTol, rTol, act;
  double *oPt = new double[minlp_->getNumVars()];
  std::copy(y, y+(minlp_->getNumVars()), oPt);
  double *iPt = new double[minlp_->getNumVars()];
  std::copy(solC_, solC_+(minlp_->getNumVars()), iPt);

  if (isObj) {
    ub = relVal;
    aTol = objAbsTol_;
    rTol = objRelTol_;
    ObjectivePtr o = minlp_->getObjective();
    f = o->getFunction();
    c0 = o->getConstant();
  } else {
    c0 = 0;
    aTol = solAbsTol_;
    rTol = solRelTol_;
    ConstraintPtr cp = prCons_[i].cons;
    ub = cp->getUb();
    f = cp->getFunction();
  }

  while (isCont) {
    for (VariableConstIterator vit=minlp_->varsBegin(); vit!=minlp_->varsEnd();
         ++vit) {
      vIdx = (*vit)->getIndex();
      y[vIdx] = (iPt[vIdx] + oPt[vIdx])/2;
    }
    isCont = prVars(y, newPt, i, isObj);
    if (isCont) {
      act = f->eval(newPt, &error) + c0;
      if (error == 0) {
        if ((act >= ub + aTol) || (ub != 0 &&
                                      act >= ub + fabs(ub)*rTol)) {
          // newPt is still infeasible
          std::copy(y, y+(minlp_->getNumVars()), oPt);
        } else if ((act < ub - aTol) || (ub != 0 &&
                                      act < ub - fabs(ub)*rTol)) {
          std::copy(y, y+(minlp_->getNumVars()), iPt);
        } else {
        // NewPt found
          delete [] iPt;
          delete [] oPt;
          return true;
        }
      }	else {
        isCont = false;
      }
    }
  }
  delete [] iPt;
  delete [] oPt;
  return false;
}


// This function assumes the point (y*,1) or (y*,0) is in the feasible region.
bool PerspCutGenerator::bisecPt(double *y, double * newPt, UInt i,
                                bool isObj, double relVal)
{
  int error = 0;
  UInt binindex;
  FunctionPtr f;
  bool isCont = true, binval;
  double zU, zL, ub, c0, aTol, rTol, act;
  //double *y = new double[minlp_->getNumVars()];
  //std::copy(x, x+(minlp_->getNumVars()), y);

  if (isObj) {
    ub = relVal;
    aTol = objAbsTol_;
    rTol = objRelTol_;
    binval = prObj_.binVal;
    binindex = (prObj_.binVar)->getIndex();
    ObjectivePtr o = minlp_->getObjective();
    f = o->getFunction();
    c0 = o->getConstant();
  } else {
    c0 = 0;
    aTol = solAbsTol_;
    rTol = solRelTol_;
    ConstraintPtr cp = prCons_[i].cons;
    ub = cp->getUb();
    f = cp->getFunction();
    binval = prCons_[i].binVal;
    binindex = (prCons_[i].binVar)->getIndex();
  }

  if (binval) {
    zU = 0;
  } else {
    zU = 1;
  }

  zL = y[binindex];

  while (isCont) {
    y[binindex] = (zL + zU)/2;
    isCont = prVars(y, newPt, i, isObj);
    if (isCont) {
      act = f->eval(newPt, &error) + c0;
      if (error == 0) {
        if ((act >= ub + aTol) || (ub != 0 &&
                                      act >= ub + fabs(ub)*rTol)) {
          // newPt is still infeasible
          zL = y[binindex];
        } else if ((act < ub - aTol) || (ub != 0 &&
                                      act < ub - fabs(ub)*rTol)) {
          zU = y[binindex];
        } else {
        // NewPt found
          return true;
        }
      }	else {
        isCont = false;
      }
    }
  }
  return false;
}


bool PerspCutGenerator::lineSearchAC(double *y, double * newPt, UInt i,
                                bool isObj, double relVal)
{
  // check if the analytical center satisfies this constraint, else for (x,
  // 1). If yes, then apply line search to find point
  if (solC_ == 0) {
    return false;
  }

  double val;
  UInt binIdx;
  bool isFound, binval;

  if (isObj) {
    binval = prObj_.binVal;
    binIdx = (prObj_.binVar)->getIndex();
  } else {
    binval = prCons_[i].binVal;
    binIdx = (prCons_[i].binVar)->getIndex();
  }

  if (binval) {
    val = 1 - solC_[binIdx];
  } else {
    val = solC_[binIdx];
  }

  if (fabs(val) < intTol_) {
    return false;
  }

  if (isFeasible(solC_, i, isObj, relVal)) {
    return lineSearch_(y, newPt, i, isObj, relVal);
      // line search
  } else {
    val = solC_[binIdx];
    if (binval) {
      solC_[binIdx] = 0;
    } else {
      solC_[binIdx] = 1;
    }

    if (isFeasible(solC_, i, isObj, relVal)) {
      isFound = lineSearch_(y, newPt, i, isObj, relVal);
    }  else {
      isFound = false;
    }
    solC_[binIdx] = val;
  }
  return isFound;
}

//void PerspCutGenerator::oriInfeasPt(RelaxationPtr rel, const double *x)
//{
   //for (UInt it = 0; it < cons_.size(); ++it) {
    //cvxCombPt(rel, x, it);
  //}
  //return;
//}


//void PerspCutGenerator::cvxCombPt(RelaxationPtr , const double *, UInt )
//{
  //return;
//}

bool PerspCutGenerator::addPC(RelaxationPtr rel, const double *x, UInt i,
                              const double *ptToCut, const double *prPt, bool isObj,
                              VariablePtr objVar, CutManagerPtr cutMan, double relVal)
{
  bool binval;
  UInt binindex;
  int error = 0;
  double sb, ub;
  //const double *x = sol->getPrimal();
  LinearFunctionPtr lf = LinearFunctionPtr();

  if (isObj) {
    binval = prObj_.binVal;
    //relVal = sol->getObjValue();
    binindex = (prObj_.binVar)->getIndex();
  } else {
    //relVal = 0;
    binval = prCons_[i].binVal;
    binindex = (prCons_[i].binVar)->getIndex();
  }

  if (binval) {
    sb = 1 - x[binindex];
  } else {
    sb = x[binindex];
  }

  if (fabs(sb) < intTol_) {
    std::cout <<"No PC\n";
    return false;
  } else {
    ub = 0;
    linearAt(rel, x, prPt, &ub, &lf, i, &error, isObj);
    // add perspective cut
    if (error == 0) {
      return addCutToRel(rel, ptToCut, lf, ub, objVar, relVal, cutMan);
    }
  }

  return false;
}


void PerspCutGenerator::writeStats(std::ostream &out) const
{
  //MS: last two statistics to be deleted later.
  if (prCons_.size() > 0 || prObj_.isPR) {
    out
    << me_ << "number of cuts added                       = " << stats_->cuts << std::endl;
    //<< me_ << "number of improved points        = " << stats_->imprvPt << std::endl
    //<< me_ << "number of feasible cum improved points        = " << stats_->infPt << std::endl;
  }
  return;
}

std::string PerspCutGenerator::getName() const
{
  return "PerspCutGenerator";
}


PerspCutGenerator::~PerspCutGenerator()
{
  if (stats_) {
    delete stats_;
  }
  if (solC_) {
    delete solC_;
  }
  env_ = 0;
  minlp_ = 0;
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
