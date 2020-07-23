//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file NlPresHandler.cpp
 * \brief Presolve nonlinear contraints.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string.h> // for memset

#include "MinotaurConfig.h"
#include "Constraint.h"
#include "ConBoundMod.h"
#include "Environment.h"
#include "Function.h"
#include "HessianOfLag.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Node.h"
#include "Problem.h"
#include "NlPresHandler.h"
#include "VarBoundMod.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Option.h"
#include "PreAuxVars.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"

#include "QuadraticFunction.h"


using namespace Minotaur;
const std::string NlPresHandler::me_ = "NlPresHandler: ";

NlPresHandler::NlPresHandler()
  : doPersp_(false),
    doQuadCone_(false),
    env_(EnvPtr()),
    eTol_(1e-6),
    logger_(LoggerPtr()),
    p_(ProblemPtr()),
    zTol_(1e-6)
{
  stats_.cBnd = 0;
  stats_.cImp = 0;
  stats_.conDel = 0;
  stats_.pRefs = 0;
  stats_.iters = 0;
  stats_.nMods = 0;
  stats_.qCone = 0;
  stats_.time = 0;
  stats_.timeN = 0;
  stats_.varDel = 0;
  stats_.vBnd = 0;
}


NlPresHandler::NlPresHandler(EnvPtr env, ProblemPtr p)
  : env_(env),
    eTol_(1e-6),
    p_(p),
    zTol_(1e-6)
{
  logger_ = env->getLogger();
  doPersp_ = env->getOptions()->findBool("persp_ref")->getValue();
  doQuadCone_ = env->getOptions()->findBool("quad_cone_ref")->getValue();
  stats_.cBnd = 0;
  stats_.cImp = 0;
  stats_.conDel = 0;
  stats_.conRel = 0;
  stats_.pRefs = 0;
  stats_.iters = 0;
  stats_.nMods = 0;
  stats_.qCone = 0;
  stats_.time = 0;
  stats_.timeN = 0;
  stats_.varDel = 0;
  stats_.vBnd = 0;
}


NlPresHandler::~NlPresHandler()
{
  //env_.reset();
  env_ = 0;
}


void NlPresHandler::chkRed_(ProblemPtr p, bool purge_cons, bool *changed,
                            ModQ *, SolveStatus &status)
{
  ConstraintPtr c;
//  ConBoundModPtr mod;
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf;
  QuadraticFunctionPtr qf;
  double lfu, lfl;
  double nlfu, nlfl;
  double impl_lb, impl_ub;
  int error = 0;
//  double bigm = (true==purge_cons)?INFINITY:100000;

  for (ConstraintConstIterator cit=p->consBegin(); cit!=p->consEnd();
       ++cit) {
    c = *cit;
    if (c->getFunctionType()!=Linear && c->getFunctionType()!=Constant) {
      lf = c->getFunction()->getLinearFunction();

      qf = c->getFunction()->getQuadraticFunction();
      
      nlf = c->getFunction()->getNonlinearFunction();
      lfu = lfl = 0.0;

      nlfu = nlfl = 0.0;
      if (lf) {
        lf->computeBounds(&lfl, &lfu);
      } 

      error = 0;
      if (qf) {
        qf->computeBounds(&nlfl, &nlfu);
      } else if (nlf) {
        nlf->computeBounds(&nlfl, &nlfu, &error);
        assert(error==0);
      }

      impl_lb = nlfl + lfl;
      impl_ub = nlfu + lfu;

      if (impl_ub + eTol_ < c->getLb() || impl_lb - eTol_ > c->getUb()) {
        status = SolvedInfeasible;
        ++stats_.infBnds;
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " problem infeaible because of "
                                      << c->getName() << std::endl;
#endif
        break;
      }



      // if the implied-lb is more than constraint-lb, but not too much more,
      // relax the constraint lb further.
      //if (c->getLb()>-INFINITY && impl_lb+eTol_ > c->getLb()
      //    && impl_lb < c->getLb()+bigm-eTol_) {
      //  mod = (ConBoundModPtr) new ConBoundMod(c, Lower, c->getLb()-bigm);
      //  std::cout << "conboundmod\n";
      //  mod->write(std::cout);
      //  mod->applyToProblem(p);
      //  *changed = true;
//#if SPEW
      //  logger_->msgStream(LogDebug2) << me_ << " removed lb of constraint "
      //                                << c->getName() << std::endl;
//#endif//
      //  if (false==purge_cons) {
      //    mods->push_back(mod);
      //    ++stats_.conRel;
      //  } else {
      //    delete mod;
      //  }
      //}
      //// similary for implied-ub
      //if (c->getUb()<INFINITY && impl_ub-eTol_ < c->getUb()
      //    && impl_ub > c->getUb()-bigm+eTol_) {
      //  mod = (ConBoundModPtr) new ConBoundMod(c, Upper, c->getUb()+bigm);
      //  mod->write(std::cout);
      //  std::cout << "mod " << mod << " con " << c << "\n";
      //  mod->applyToProblem(p);
      //  *changed = true;
//#if SPEW
      //  logger_->msgStream(LogDebug2) << me_ << " removed ub of constraint "
      //                                << c->getName() << std::endl;
//#endif//
      //  if (false==purge_cons) {
      //    mods->push_back(mod);
      //    ++stats_.conRel;
      //  } else {
      //    delete mod;
      //  }
      //}


      // delete the constraint if unbounded on both sides
      if (c->getUb()==INFINITY && c->getLb()==-INFINITY && true==purge_cons) {
        ++stats_.conDel;
        p->markDelete(c);
        *changed = true;
#if SPEW
        logger_->msgStream(LogDebug2) << me_ << " marked constraint "
                                      << c->getName() << " for deleting."
                                      << std::endl;
#endif
      }
    }
  }
}


// TODO: make it work even when binaries are absent from a constraint.
void  NlPresHandler::coeffImpr_(bool *changed)
{
  ConstraintPtr c;
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf;
  QuadraticFunctionPtr qf;
  double ll, uu;
  double cu, cl;
  double a0;
  VariablePtr z;

  if (p_->getSize()->bins<1) {
    return;
  }
  for (ConstraintConstIterator cit=p_->consBegin(); cit!=p_->consEnd();
       ++cit) {
    c = *cit;
    if (c->getFunctionType()==Linear || c->getFunctionType()==Constant) {
      continue;
    }
    if (c->getLb()>-INFINITY && c->getUb()<INFINITY) {
      continue;
    }
    lf   = c->getFunction()->getLinearFunction();
    if (!lf) {
      //c->write(std::cout);
      continue;
    }
    nlf   = c->getFunction()->getNonlinearFunction();
    qf   = c->getFunction()->getQuadraticFunction();
    cu = c->getUb();
    cl = c->getLb();
    for (VariableGroupConstIterator it2=lf->termsBegin();
         it2 != lf->termsEnd(); ++it2) {
      z = it2->first;
      a0 = it2->second;
      if (nlf){
        if ((z->getType()!=Binary && z->getType()!=ImplBin)
            || fabs(z->getUb()-z->getLb()-1) > eTol_
            || p_->isMarkedDel(z)
            || nlf->hasVar(z)) {
          continue;
        }
      }
      if (qf){
        if ((z->getType()!=Binary && z->getType()!=ImplBin)
          || fabs(z->getUb()-z->getLb()-1) > eTol_
          || p_->isMarkedDel(z)
          || qf->hasVar(z)) {
        continue;
        }
      }
      computeImpBounds_(c, z, 0.0, &ll, &uu);
      if (uu < cu-eTol_ && uu+a0 >= cu) {
        // constraint is redundant when z=0
        assert(a0 > 0);
        lf->incTerm(z, uu-cu);
        p_->changeBound(c, Upper, uu);
        *changed = true;
        ++stats_.cImp;
        break;
      } else if (ll > cl+eTol_ && ll+a0 < cl) {
        // constraint is redundant when z=0
        assert(a0 < 0);
        lf->incTerm(z, ll-cl);
        p_->changeBound(c, Lower, ll);
        *changed = true;
        ++stats_.cImp;
        break;
      }
      computeImpBounds_(c, z, 1.0, &ll, &uu);
      if (uu < cu-eTol_ && uu-a0 > cu) {
        // constraint is redundant when z=1
        assert(a0 < 0);
        lf->incTerm(z, cu-uu);
        *changed = true;
        ++stats_.cImp;
        break;
      } else if (ll > cl+eTol_ && ll-a0 < cl) {
        // constraint is redundant when z=1
        assert(a0 > 0);
        lf->incTerm(z, cl-ll);
        *changed = true;
        ++stats_.cImp;
        break;
      }
    }
  }
}


void NlPresHandler::bin2LinF_(ProblemPtr p, LinearFunctionPtr lf,
                              UInt nz, const UInt *irow,
                              const UInt *jcol,
                              const double *values, PreAuxVarsPtr mod)
{
  VariablePtr v1, v2, v3;
  LinearFunctionPtr lf3;
  FunctionPtr f;
  double lb, ub;

  for (UInt i=0; i<nz; ++i) {
    if (fabs(values[i])>1e-12) {
      v1 = p->getVariable(irow[i]);
      v2 = p->getVariable(jcol[i]);
      if (v1==v2) {
        assert(Binary == v1->getType() || ImplBin == v1->getType());
        lf->incTerm(v1, 0.5*values[i]);
      } else if ((Binary == v1->getType() || ImplBin == v1->getType()) &&
                 (Binary == v2->getType() || ImplBin == v2->getType())) {
        v3 = p->newVariable(0.0, 1.0, ImplBin);
        lf->incTerm(v3, values[i]);
        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v3, 1.0);
        lf3->addTerm(v1, -1.0);
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, 0.0);

        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v3, 1.0);
        lf3->addTerm(v2, -1.0);
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, 0.0);

        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v1, 1.0);
        lf3->addTerm(v2, 1.0);
        lf3->addTerm(v3, -1.0);
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, 1.0);
        mod->insert(v3);
      } else {
        if (Binary==v1->getType() || ImplBin==v1->getType()) {
          // exchange
          v3 = v1;
          v1 = v2;
          v2 = v3;
        }
        // v1 is continuous, v2 is binary.
        assert (Binary==v2->getType() || ImplBin==v2->getType());
        lb = (v1->getLb()<0.0) ? v1->getLb() : 0.0;
        ub = (v1->getUb()>0.0) ? v1->getUb() : 0.0;
        v3 = p->newVariable(lb, ub, Continuous);
        lf->incTerm(v3, values[i]);

        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v1, -1.0);
        lf3->addTerm(v3, 1.0);
        lf3->addTerm(v2, -v1->getLb());
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, -v1->getLb());

        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v1, 1.0);
        lf3->addTerm(v3, -1.0);
        lf3->addTerm(v2, v1->getUb());
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, v1->getUb());

        // v3 - ub*v2 <= 0
        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v3, 1.0);
        lf3->addTerm(v2, -v1->getUb());
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, 0);

        // v3 - lb*v2 >= 0 ... -v3 + lb*v2 <= 0
        lf3 = (LinearFunctionPtr) new LinearFunction();
        lf3->addTerm(v3, -1.0);
        lf3->addTerm(v2, v1->getLb());
        f = (FunctionPtr) new Function(lf3);
        p->newConstraint(f, -INFINITY, 0);

        mod->insert(v3);
      }
    }
  }
}


void  NlPresHandler::bin2Lin_(ProblemPtr p, PreModQ *mods, bool *changed)
{
  ConstraintPtr c;
  LinearFunctionPtr lf;
  FunctionPtr f;
  HessianOfLagPtr hess;
  double *mult = 0;
  int err = 0;
  double *x = 0;
  double *values = 0;
  double *grad = 0;
  double nlconst = 0;
  UInt *irow = 0;
  UInt *jcol = 0;
  UInt nz = 0;
  PreAuxVarsPtr mod;

  p->calculateSize();
  if (0==p->getSize()->quadCons && Quadratic!=p->getSize()->objType) {
    return;
  }

  mod = new PreAuxVars();
  p->setNativeDer(); // TODO: avoid setting it up repeatedly.
  hess = p->getHessian();
  nz = hess->getNumNz();
  mult = new double[p->getNumCons()];
  values = new double[nz];
  irow = new UInt[nz];
  jcol = new UInt[nz];
  x = new double[p->getNumVars()];
  grad = new double[p->getNumVars()];
  memset(mult, 0, p->getNumCons()*sizeof(double));
  memset(values, 0, nz*sizeof(UInt));
  memset(irow, 0, nz*sizeof(UInt));
  memset(jcol, 0, nz*sizeof(UInt));
  memset(x, 0, p->getNumVars()*sizeof(double));

  hess->fillRowColIndices(irow, jcol);

  // objective
  f = p->getObjective()->getFunction();
  if (f) {
    if (Quadratic==f->getType()) {
      hess->fillRowColValues(x, 1.0, mult, values, &err); assert(0==err);
      if (canBin2Lin_(p, nz, irow, jcol, values)) {
        memset(grad, 0, p->getNumVars()*sizeof(double));
        f->evalGradient(x, grad, &err); assert(0==err);
        nlconst = f->eval(x, &err); assert(0==err);
        lf = (LinearFunctionPtr) new LinearFunction(grad, p->varsBegin(),
                                                    p->varsEnd(), 1e-12);
        bin2LinF_(p, lf, nz, irow, jcol, values, mod);
        f = (FunctionPtr) new Function(lf);
        p->newObjective(f, p->getObjective()->getConstant()+nlconst, Minimize);
        *changed = true;
      }
    }
  }
  

  // constraints
  for (ConstraintConstIterator cit=p->consBegin(); cit!=p->consEnd();
       ++cit) {
    c = *cit;
    f = c->getFunction(); 
    if (!f) {
      continue;
    }
    if (Quadratic==f->getType()) {
      mult[c->getIndex()] = 1.0;
      memset(values, 0, nz*sizeof(UInt));
      hess->fillRowColValues(x, 0.0, mult, values, &err); assert(0==err);
      mult[c->getIndex()] = 0.0;
      if (canBin2Lin_(p, nz, irow, jcol, values)) {
        lf = f->getLinearFunction();
        if (!lf) {
          lf = (LinearFunctionPtr) new LinearFunction();
        }
        bin2LinF_(p, lf, nz, irow, jcol, values, mod);
        if (lf->getNumTerms()>0) {
          f = (FunctionPtr) new Function(lf);
          p->newConstraint(f, c->getLb(), c->getUb());
        } else {
          //lf.reset();
          lf = 0;
        }
        p->markDelete(c);
        *changed = true;
        cit = p->consBegin()+c->getIndex();
      }
    }
  }

  if (mod->getSize()>0) {
    mods->push_back(mod);
  } else {
    delete mod;
  }
  p->delMarkedCons();
  delete [] mult;
  delete [] x;
  delete [] values;
  delete [] grad;
  delete [] irow;
  delete [] jcol;

}


bool NlPresHandler::canBin2Lin_(ProblemPtr p, UInt nz, const UInt *irow,
                                const UInt *jcol, const double *values)
{
  VariablePtr v1, v2;
  bool all_zeros = true;

  for (UInt i=0; i<nz; ++i) {
    if (fabs(values[i])>1e-12) {
      all_zeros = false;
      v1 = p->getVariable(jcol[i]);
      v2 = p->getVariable(irow[i]);
      // if neither binary, return false
      if ((v1->getType()==Continuous || v1->getType()==Integer) &&
          (v2->getType()==Continuous || v2->getType()==Integer)) {
        return false;
      }
      // if either one is unbounded, return false
      if ((v1->getUb()>=INFINITY || v1->getLb()<=-INFINITY) ||
          (v2->getUb()>=INFINITY || v2->getLb()<=-INFINITY)) {
        return false;
      }
    }
  }

  if (all_zeros) {
    return false; // all values are zero. it is really a linear function
                  // but minotaur thinks it is quadratic.
  }
  return true;
}


#if 0
bool  NlPresHandler::canLin_(FunctionPtr f)
{
  CGraphPtr cg;
  NonlinearFunctionPtr nlf;
  LTHessStor stor;
  UInt i;
  UInt nz;
  UInt *cols;
  std::deque<UInt> *indq;
  VariablePtr x1;
  VariablePtr x2;

  if (f->getQuadraticFunction()) {
    assert(!"can not do explicit quadratic function yet!");
  }
  
  nlf = f->getNonlinearFunction();
  if (!nlf) {
    assert(!"Need nonlinear function!");
  }

  assert(Quadratic==nlf->getType());

  stor.nlVars = nlf->numVars();
  i = 0;
  stor.rows   = new VariablePtr[stor.nlVars];
  stor.colQs  = new std::deque<UInt>[stor.nlVars];
  stor.starts = new UInt[stor.nlVars+1];
  stor.nz = 0;
  for(VariableSet::iterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
    stor.rows[i] = (*it);
    ++i;
  }

  nlf->fillHessStor(&stor);

  nz = 0;
  for (i=0; i<stor.nlVars; ++i) {
    stor.starts[i] = nz;
    nz += (stor.colQs+i)->size();
  }
  stor.starts[i] = nz;
  stor.nz = nz;
  stor.cols = new UInt[nz];
  cols = stor.cols;
  indq = stor.colQs;
  nz = 0;
  for (i=0; i<stor.nlVars; ++i, ++indq) {
    x1 = stor.rows[i];
    for (std::deque<UInt>::iterator it2=indq->begin(); it2!=indq->end(); 
         ++it2,++cols) {
      *cols = *it2;
      ++nz;
      x2 = p_->getVar(*cols);
      if (Binary!=x1->getType() && ImplBin!=x1->getType() && 
          Binary!=x2->getType() && ImplBin!=x2->getType()) {
        return false;
      }
    }
  }
  assert(nz == stor.nz);
  nlf->finalHessStor(&stor);
}
#endif


void  NlPresHandler::computeImpBounds_(ConstraintPtr c, VariablePtr z,
                                       double zval, double *lb, double *ub)
{
  VariablePtr v;
  double ll = 0.;
  double uu = 0.;
  double l1, u1, a2, b2;
  ConstraintPtr c2;
  LinearFunctionPtr lf2;
  NonlinearFunctionPtr nlf;
  QuadraticFunctionPtr qf;
  ModStack mods;
  VarBoundModPtr m;
  ModificationPtr m2;
  int error = 0;

  if (zval<0.5) {
    m = (VarBoundModPtr) new VarBoundMod(z, Upper, 0.0);
  } else {
    m = (VarBoundModPtr) new VarBoundMod(z, Lower, 1.0);
  }
  m->applyToProblem(p_);
  mods.push(m);

  for (VarSet::iterator it=c->getFunction()->varsBegin();
       it!=c->getFunction()->varsEnd(); ++it) {
    v = *it;
    if (v==z) {
      continue;
    }

    l1 = v->getLb();
    u1 = v->getUb();
    for (ConstrSet::iterator it2=v->consBegin(); it2!=v->consEnd(); ++it2) {
      c2 = *it2;
      if (c2->getFunctionType()!=Linear) {
        continue;
      }
      lf2 = c2->getLinearFunction();
      if (lf2->getNumTerms()==2 && lf2->hasVar(z)) {
        b2 = lf2->getWeight(z);
        a2 = lf2->getWeight(v);
        if (a2>0 && (c2->getUb()-zval*b2)/a2 < u1) {
          u1 = (c2->getUb()-zval*b2)/a2;
        }
        if (a2<0 && (c2->getUb()-zval*b2)/a2 > l1) {
          l1 = (c2->getUb()-zval*b2)/a2;
        }
        if (a2>0 && (c2->getLb()-zval*b2)/a2 > l1) {
          l1 = (c2->getLb()-zval*b2)/a2;
        }
        if (a2<0 && (c2->getLb()-zval*b2)/a2 < u1) {
          u1 = (c2->getLb()-zval*b2)/a2;
        }
      }
    }
    if (l1>v->getLb()) {
      m = (VarBoundModPtr) new VarBoundMod(v, Lower, l1);
      m->applyToProblem(p_);
      mods.push(m);
    }
    if (u1<v->getUb()) {
      m = (VarBoundModPtr) new VarBoundMod(v, Upper, u1);
      m->applyToProblem(p_);
      mods.push(m);
    }
  }
  c->getFunction()->getLinearFunction()->computeBounds(&ll, &uu);
  nlf = c->getFunction()->getNonlinearFunction();
  if (nlf){
    nlf->computeBounds(&l1, &u1, &error);
  }
  qf = c->getFunction()->getQuadraticFunction();
  if (qf){
    qf->computeBounds(&l1, &u1);
  }
  ll += l1;
  uu += u1;
  *lb = ll;
  *ub = uu;
  while (!mods.empty()) {
    m2 = mods.top();
    mods.pop();
    m2->undoToProblem(p_);
    delete m2;
  }
}


void NlPresHandler::copyBndsFromRel_(RelaxationPtr rel, ModVector &p_mods)
{
  VarBoundModPtr mod;
  VariablePtr xp;
  VariablePtr xr;

  for (VariableConstIterator it=p_->varsBegin(); it!=p_->varsEnd();
       ++it) {
    xp = *it;
    xr = rel->getRelaxationVar(xp);
    if (!xr) {
      continue;
    }
    if (xr->getLb() > xp->getLb()+eTol_) {
      mod = (VarBoundModPtr) new  VarBoundMod(xp, Lower, xr->getLb());
      mod->applyToProblem(p_);
      p_mods.push_back(mod);
    }
    if (xr->getUb() < xp->getUb()-eTol_) {
      mod = (VarBoundModPtr) new  VarBoundMod(xp, Upper, xr->getUb());
      mod->applyToProblem(p_);
      p_mods.push_back(mod);
    }
  }
}


std::string NlPresHandler::getName() const
{
  return "NlPresHandler (presolving nonlinear constraints).";
}


void NlPresHandler::perspRef_(ProblemPtr p, PreModQ *, bool *changed)
{
  ConstraintPtr c, c2;
  FunctionPtr f;
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf, pnlf;  
  double alpha, beta;
  double lb, ub;
  double dtmp;
  int err = 0;

  VarSet indvars0; // all binary variables. This set does not change.
  VarSet nlvars;
  VarSet indvars;
  VarSet candvars;
  VariablePtr v, z;

  p->calculateSize();

  // first find all binary variables
  for (VariableConstIterator vit=p->varsBegin(); vit!=p->varsEnd(); ++vit) {
    v = *vit;
    if (Binary==v->getType() || ImplBin==v->getType()) {
      indvars0.insert(v);
    }
  }

  // constraints
  for (ConstraintConstIterator cit=p->consBegin(); cit!=p->consEnd(); ++cit) {
    c = *cit;
    f = c->getFunction(); 
    if (!f) {
      continue;
    }
    if (Quadratic!=f->getType() && Nonlinear!=f->getType()) {
      continue;
    }

    nlvars.clear();
    indvars = indvars0;

    nlf = f->getNonlinearFunction();
    nlvars.insert(nlf->varsBegin(), nlf->varsEnd());

    // search for an indicator variable: We really need an implication graph
    // here.
    for (VarSetIter it=nlvars.begin(); it!=nlvars.end(); ++it) {
      v = *it;
      candvars = indvars;
      indvars.clear();
      for (ConstrSet::iterator cit2=v->consBegin(); cit2!=v->consEnd() &&
           candvars.size()>0; ++cit2) {
        c2 = *cit2;
        if (c2->getFunctionType()!=Linear) {
          continue;
        }
        lf = c2->getLinearFunction();
        if (lf->getNumTerms()==2) {
          alpha = beta = 0.0;
          alpha = lf->getWeight(v);
          for (VariableGroupConstIterator it2 = lf->termsBegin();
               it2!=lf->termsEnd(); ++it2) {
            z = (it2)->first;
            if (z->getId() != v->getId()) {
              beta = (it2)->second;
              break;
            }
          } 
          if (candvars.end() == candvars.find(z)) {
            continue;
          }
          // check if z turns v off.
          // The constraint is of the form lb <= alpha.v + beta.z <= ub
          lb = c2->getLb();
          ub = c2->getUb();
          if (alpha>0) {
            ub = ub/alpha;
            lb = lb/alpha;
          } else {
            dtmp = lb;
            lb = ub/alpha;
            ub = dtmp/alpha;
          }
          lb = std::max(lb, v->getLb());
          ub = std::min(ub, v->getUb());
          // lb and ub are now bounds on v;
          if (fabs(lb)<zTol_ && fabs(ub)<zTol_) {
            indvars.insert(z);
          }
        }
      }
    }
    if (false == indvars.empty()) {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "found a candidate for "
                                    << "nonlinear perspective reformulation"
                                    << std::endl;
      c->write(logger_->msgStream(LogDebug2));
      logger_->msgStream(LogDebug2) << me_ << "Indicator Variables:"
                                    << std::endl;
      for (VarSet::const_iterator vit=indvars.begin(); vit!=indvars.end();
           ++vit) {
        (*vit)->write(logger_->msgStream(LogDebug2));
      }
      logger_->msgStream(LogDebug2) << std::endl;
#endif

      // doing the reformulation now.
      z = *(indvars.begin());
      nlf = c->getFunction()->getNonlinearFunction();
      if (false==nlf->hasVar(z)) {
        pnlf = nlf->getPersp(z, 1e-6, &err); assert(err==0);
        p->changeConstraint(c, pnlf);
        *changed = true;
        ++stats_.pRefs;
#if SPEW
        c->write(logger_->msgStream(LogDebug2));
#endif 
      }
    } 
    // else {
    //   std::cout << "Did not find it!\n";
    //   c->write(std::cout);
    // }
  }
}


SolveStatus NlPresHandler::presolve(PreModQ *mods, bool *changed0)
{
  bool changed = true;
  Timer *tim = env_->getNewTimer();
  SolveStatus status = Started;
  ModQ *dmods = 0; // NULL

  tim->start();
  while(changed==true && stats_.iters < 5) {
    logger_->msgStream(LogDebug2) << me_ << "starting presolve iter "
                                  << stats_.iters << std::endl;
    changed = false;
    chkRed_(p_, true, &changed, dmods, status);
    p_->delMarkedCons();
    varBndsFromCons_(p_, true, &changed, dmods, status);
    if (SolvedInfeasible==status) {
      stats_.time += tim->query();
      delete tim;
      return SolvedInfeasible;
    }
    coeffImpr_(&changed);
    bin2Lin_(p_, mods, &changed);
    if (doPersp_ && false ) {
      perspRef_(p_, mods, &changed);
    }
    if (doQuadCone_ && false ) {
      quadConeRef_(p_, mods, &changed);
    }
    ++stats_.iters;
    if (changed) {
      *changed0 = true;
    }
  }
  stats_.time += tim->query();
  delete tim;
  return Finished;
}


bool NlPresHandler::presolveNode(RelaxationPtr rel, NodePtr,
                                 SolutionPoolPtr s_pool, ModVector &p_mods,
                                 ModVector &r_mods)
{
  SolveStatus status = Started;
  simplePresolve(rel, s_pool, r_mods, status);
  if (true==modProb_) {
    copyBndsFromRel_(rel, p_mods);
  }
  return (status==SolvedInfeasible);
}


void NlPresHandler::simplePresolve(ProblemPtr p, SolutionPoolPtr s_pool,
                                   ModVector &t_mods, SolveStatus &status) 
{
  bool changed = true;
  ModQ mods;
  UInt max_iters = 4;
  UInt min_iters = 2;
  UInt iters = 1;
  int err = 0;
  double stime = env_->getTime(err); assert (0==err);
  double ub = (s_pool)?s_pool->getBestSolutionValue():INFINITY;

  while (true==changed && iters<=max_iters && 
         iters<=min_iters && status!=SolvedInfeasible) {
    changed = false;
    ++iters;
    chkRed_(p, false, &changed, &mods, status);
    if (SolvedInfeasible==status) {
      break;
    }
    varBndsFromCons_(p, false, &changed, &mods, status);
    if (SolvedInfeasible==status) {
      break;
    }
    if (ub < INFINITY) {
      fixObjBins_(p, ub, &changed, &mods, status);
      if (SolvedInfeasible==status) {
        break;
      }
    }
  }
  
  for (ModQ::const_iterator it=mods.begin(); it!=mods.end(); ++it) {
    t_mods.push_back(*it);
  }
  stats_.nMods += mods.size();
  stats_.timeN += (env_->getTime(err) - stime); assert (0==err);
}


void NlPresHandler::fixObjBins_(ProblemPtr p, double ub, bool *changed,
                                ModQ *mods, SolveStatus &status)
{
  FunctionPtr f = p->getObjective()->getFunction();
  NonlinearFunctionPtr nlf;
  LinearFunctionPtr lf;
  double nlfl, nlfu;
  double lfl, lfu;
  double olb;
  int error = 0;
  double a0;
  VariablePtr z;
  VarBoundModPtr mod;

  if (f && ub<INFINITY) {
    lf = f->getLinearFunction();
    nlf = f->getNonlinearFunction();
    if (lf) {
      lfl = 0.0;
      lfu = 0.0;
      nlfl = 0.;
      nlfu = 0.;
      if (nlf) {
        nlf->computeBounds(&nlfl, &nlfu, &error);
        assert(error==0);
      }
      lf->computeBounds(&lfl, &lfu);
      olb = lfl+nlfl;
      if (olb<=-INFINITY) {
        return;
      } else if (olb>ub) {
        status = SolvedInfeasible;
        return ;
      }
      //std::cout << "obj lb = " << lfl+nlfl << " obj ub = " <<
      //lfu+nlfu << " ub = " << ub << std::endl;

      for (VariableGroupConstIterator it2=lf->termsBegin();
           it2 != lf->termsEnd(); ++it2) {
        z = it2->first;
        if ((z->getType()==Binary || z->getType()==ImplBin)
            && (z->getUb()-z->getLb()) > eTol_
            && !(p->isMarkedDel(z))) {
          a0 = it2->second;
          if (a0>0 && olb+a0>ub) {
            *changed = true;
            mod = (VarBoundModPtr) new VarBoundMod(z, Upper, 0.0);
            mod->applyToProblem(p);
            mods->push_back(mod);
            ++(stats_.nMods);
          } else if (a0<0 && olb-a0>ub) {
            *changed = true;
            mod = (VarBoundModPtr) new VarBoundMod(z, Lower, 1.0);
            mods->push_back(mod);
            mod->applyToProblem(p);
            ++(stats_.nMods);
          }
        }
      }
    }
  }
}


//
// x_1^2 + x_2^2 + x_3^2 <= K + Mz
// if (M>0), we rewrite it as
// sqrt(x_1^2 + x_2^2 + x_3^2 + eps) <= (1-z).sqrt(K+eps) + z.sqrt(K+M+eps)
// if (M<0), we rewrite it as
// x_1^2 + x_2^2 + x_3^2 <= (K+M) + (1-z).(-M), and then again we get the same 
// sqrt(x_1^2 + x_2^2 + x_3^2 + eps) <= (1-z).sqrt(K+eps) + z.sqrt(K+M+eps)
//
// TODO: implement for qf and K<0
// TODO: implement isSumOfSquares
//
void NlPresHandler::quadConeRef_(ProblemPtr p, PreModQ *, bool *changed)
{
  ConstraintPtr c;
  FunctionPtr f;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf, lf2;
  NonlinearFunctionPtr nlf, nlf2;
  VariablePtr z;
  const double eps = 1e-4;
  int err = 0;
  double M, K;

  for (ConstraintConstIterator cit=p->consBegin(); cit!=p->consEnd();
       ++cit) {
    c = *cit;
    K = c->getUb();
    if (K==INFINITY || K<0.0) {
      continue;
    }

    f = c->getFunction(); 
    if (!f) {
      continue;
    }
    if (Quadratic!=f->getType()) {
      continue;
    }
    qf = f->getQuadraticFunction();
    lf = f->getLinearFunction();
    nlf = f->getNonlinearFunction();
    if (!lf || lf->getNumTerms()!=1) {
      continue;
    }

    z = lf->termsBegin()->first;
    if (z->getType()!=Binary && z->getType()!=ImplBin) {
      continue;
    }
    M = -1.0*lf->termsBegin()->second;

    if(nlf && qf) {
      continue;
    } else if (qf) {
      assert(!"implement me!");
    } else if (nlf) {
      if (nlf->isSumOfSquares()) {
        nlf2 = nlf->cloneWithVars(p->varsBegin(), &err);
        if (err) { continue; }
        nlf2->addConst(eps, err); if (err) { continue; }
        nlf2->sqrRoot(err);  if (err) { continue; }

        //c->write(std::cout);
        // nlf2 <= (1-z).sqrt(K+eps) + z.sqrt(K+M+eps)
        lf2 = (LinearFunctionPtr) new LinearFunction();
        lf2->addTerm(z, sqrt(K+eps)-sqrt(K+M+eps));
        f = (FunctionPtr) new Function(lf2, nlf2);

        p->changeConstraint(c, nlf2);
        p->changeConstraint(c, lf2, -INFINITY, sqrt(K+eps));
        ++(stats_.qCone);
        //c->write(std::cout);
        *changed = true;
      }
    }
  }
}







void NlPresHandler::quad_bound(VariablePtr v, double a,double b, double *l,
                                     double *u){
  double s;
  double t;
  double r;
  double etol = 1e-7;

  if (a == 0){
    *u = std::max(v->getLb()*(b+etol), v->getUb()*(b+etol));
    *l = std::min(v->getLb()*(b+etol), v->getUb()*(b+etol));
  }
  else {
    *u = std::max(v->getLb()*(a*v->getLb()+b), v->getUb()*(a*v->getUb()+b));
    *l = std::min(v->getLb()*(a*v->getLb()+b), v->getUb()*(a*v->getUb()+b));

    if(a < 0){
      if (v->getLb() == INFINITY){
        *l = -INFINITY;
      }
    }

    s = b/2;
    t = s/(-a);
    if (t > v->getLb()){
      r = -2*a*v->getUb();
      if (r < b){
        if (a < 0){
          *u = s*t;
        }
        else{
          *l = s*t;
        }
      }
    }
  }
 }

                                     
                                     
                                     


void NlPresHandler::quad_bound_range(VariablePtr v, double a,double blb, double bub, double *l,
                                     double *u){
  double llb;
  double lub;
  double ulb;
  double uub;

  quad_bound(v, a, blb,  &llb, &lub);
  quad_bound(v, a, bub,  &ulb, &uub);
  *l = std::min(llb, ulb);
  *u = std::max(lub, uub);
 }




                                     
void NlPresHandler::x_bound_with_c(double a,double b,double c, double *l,
                                     double *u, double *l2){
  double delta;
  double xl = -INFINITY;
  double xu = INFINITY;

  b = b/2;
  delta = b*b + a*c;
  


  if (b >= 0){
    if (c > 0){
      if (delta < 0){
        xl = INFINITY;
      }
      else if (a == 0 && b ==0){
        xl = INFINITY;
        }
      
      else{
        if (a > 0){
          xl = (-b + sqrt(delta))/a;
          *l2 = (-b - sqrt(delta))/a;
          }
        if (a < 0){
          xu = (-b - sqrt(delta))/a;
          xl = (-b + sqrt(delta))/a;
          } 
        }
      
    }
    else{
      if (a < 0){
        xu = (-b - sqrt(delta))/a;
        xl = (-b + sqrt(delta))/a;
      }
      if (a > 0){
        if(delta >= 0){
          xl = (-b + sqrt(delta))/a;
          *l2 = (-b - sqrt(delta))/a;

        }

        
      }
    }
  }
  else{
    if (c > 0){
      if (a > 0){
        xl = (-b + sqrt(delta))/a;
        *l2 = (-b - sqrt(delta))/a;
      }
      else if (a < 0){
        if(delta >= 0){
          xl = (-b + sqrt(delta))/a;
          xu = (-b - sqrt(delta))/a;
        }
        else{
          xl = INFINITY;
        }
      }
    }
    else {
      if(a<0){
        xu = (-b - sqrt(delta))/a;
        xl = (-b + sqrt(delta))/a;
      }
      if (delta >= 0){
        if (a > 0){
          xl = (-b + sqrt(delta))/a;
          *l2 = (-b + sqrt(delta))/a;
        }
      
      }
    }
  }

  
  if (c == INFINITY){
    xl = -INFINITY;
    xu = INFINITY;

  }

  if (a == 0){
    if (b > 0){
      xl = c/(2*b);
    }
    if (b < 0){
      xu = c/(2*b);
    }
  }

  *l = xl;
  *u = xu;


}  
  



void NlPresHandler::linear_var_set(VariableSet lfvars, FunctionPtr f, VariableSet *linear_terms){
  
  VariablePtr v;
  
  for (VariableSet::iterator it = lfvars.begin(); it != lfvars.end(); ++it){
    v=*it;
    if (f->isLinearIn(v)==true){
      linear_terms->insert(v);
    }
  }

}



void NlPresHandler::get_other_linear_var_bounds(VariablePtr v, double *lflb, double *lfub, 
                                          VariableSet linear_terms, LinearFunctionPtr lf){
  VariablePtr v2;
  double a = 0;
  double lb = 0;
  double ub = 0;
  for (VariableSet::iterator it = linear_terms.begin(); it != linear_terms.end(); ++it){
    v2 = *it;
    if (v2 != v){
      a = lf->getWeight(v2);
      if (a>0) {
        lb += a*v2->getLb();
        ub += a*v2->getUb();
      } else {
        lb += a*v2->getUb();
        ub += a*v2->getLb();
      }

    }
  }
  *lflb = lb;
  *lfub = ub;
}





void NlPresHandler::get_other_quad_var_bounds(VariablePtr v, double *qflb, double *qfub, VariableSet qfvars,
                                        QuadraticFunctionPtr qf, LinearFunctionPtr lf){
  VariablePtr v2;
  double lb = 0;
  double ub = 0;
  double a = 0;
  double blb = 0;
  double bub = 0;
  

  for (VariableSet::iterator it = qfvars.begin(); it != qfvars.end(); ++it){
    v2 = *it;
    if (v2 != v){

      a = qf->getWeight(v2, v2);
      qf->bndsquadterms_2(&blb, &bub, v2, v);
      blb += lf->getWeight(v2);
      bub += lf->getWeight(v2);

      quad_bound_range(v2, a, blb, bub, &lb, &ub);
      *qflb += lb;
      *qfub += ub;

    }


  }



}




void NlPresHandler::bilinear_bounds(VariablePtr v, double *qflb1, double *qfub1, VariableSet qfvars,
                                        QuadraticFunctionPtr qf){

  VariablePtr v3;
  double a = 0;
  double b = 0;
  *qflb1 = 0;
  *qfub1 = 0;
  

  for (VariableSet::iterator it = qfvars.begin(); it != qfvars.end(); ++it){
    v3 = *it; 
    if (v3->getIndex() != v->getIndex()){
      if(qf->getWeight(v, v3) == 0){
        a=0;
        b=0;
      }
      else{
        a = (qf->getWeight(v, v3))*v3->getLb();
        b = (qf->getWeight(v, v3))*v3->getUb();
      }
      
      
      *qflb1 += std::min(a,b);
      *qfub1 += std::max(a,b);
    }


  }

}






void NlPresHandler::lin_var_bound(VarBoundModVector lfmod, LinearFunctionPtr lf, QuadraticFunctionPtr qf, 
                                VariableSet qfvars,  VariableSet linear_terms, double cl, double cu, int &change){

  double lflb = 0;
  double lfub = 0;
  double qflb = 0;
  double qfub = 0;
  double qflb1 = 0;
  double qfub1 = 0;
  double lvlb = 0;
  double lvub = 0;

  double a = 0;
  double blb = 0;
  double bub = 0;
  change = 0;

  VariablePtr v;

  VariableSet::iterator it2;

  for (VariableSet::iterator it = qfvars.begin(); it != qfvars.end(); ++it){
    v = *it;

      blb = 0;
      bub = 0;

      a = qf->getWeight(v,v);
      bilinear_bounds(v, &blb, &bub, qfvars,qf);
      blb += lf->getWeight(v);
      bub += lf->getWeight(v);
      quad_bound_range(v, a, blb, bub, &qflb1, &qfub1);
      qflb += qflb1;
      qfub += qfub1;
      
    }

  for (VariableSet::iterator it = linear_terms.begin(); it != linear_terms.end(); ++it){
    v = *it;

    get_other_linear_var_bounds(v, &lflb, &lfub, linear_terms, lf);

    if (lf->getWeight(v) > 0){
      lvlb = (cl - lfub - qfub)/(lf->getWeight(v));
      lvub = (cu - lflb - qflb)/(lf->getWeight(v));
    }

    if (lf->getWeight(v) < 0){
      lvub = (cl - lfub - qfub)/(lf->getWeight(v));
      lvlb = (cu - lflb - qflb)/(lf->getWeight(v));
    }
  
    if (lvlb > v->getLb()) {
      lfmod.push_back((VarBoundModPtr) new VarBoundMod(v, Lower, lvlb));
    }
    if (lvub < v->getUb()) {
      lfmod.push_back((VarBoundModPtr) new VarBoundMod(v, Upper, lvub));
    }
     
  }


  if (false==lfmod.empty()) {
    for (VarBoundModVector::iterator it=lfmod.begin(); it!=lfmod.end(); ++it) {
      (*it)->applyToProblem(p_);
      ++stats_.vBnd;
      
    }
    change = 1;
}

}





void NlPresHandler::quad_var_bound(VarBoundModVector qfmod, LinearFunctionPtr lf, QuadraticFunctionPtr qf, 
                                  VariableSet qfvars,  VariableSet linear_terms, double cl, double cu,
                                  int &change){

  VariablePtr v;

  double lflb = 0;
  double lfub = 0;

  change = 0;
  
  for (VariableSet::iterator it = linear_terms.begin(); it != linear_terms.end(); ++it){
    v = *it;
    lflb += std::min(lf->getWeight(v)*v->getLb(), lf->getWeight(v)*v->getUb());
    lfub += std::max(lf->getWeight(v)*v->getLb(), lf->getWeight(v)*v->getUb());
  }

  for (VariableSet::iterator it = qfvars.begin(); it != qfvars.end(); ++it){
    v = *it;
   
    double qflb = 0;
    double qfub = 0;
    double qvlb = 0;
    double qvub = 0;

    double a = 0;
    double blb = 0;
    double bub = 0;
    double clb = 0;
    double cub = 0;

    double w = 0;
    double x = 0;
    double y = 0;
    double z = 0;

    double lbu1 = -INFINITY;
    double ubu1 = INFINITY;
    double lbul1 = -INFINITY;
 
    double lbu2 = -INFINITY;
    double ubu2 = INFINITY;
    double lbul2 = -INFINITY;

    get_other_quad_var_bounds(v, &qflb, &qfub, qfvars, qf, lf);

    w = cl - lflb - qflb;
    x = cl - lfub - qfub;
    y = cu - lflb - qflb;
    z = cu - lfub - qfub;
    clb = std::min(w,x); clb = std::min(clb,y); clb = std::min(clb,z);
    cub = std::max(w,x); cub = std::max(cub,y); cub = std::max(cub,z);
    a = qf->getWeight(v, v);
    qf->bndsquadterms(&blb, &bub, v);
    

    blb += lf->getWeight(v);
    bub += lf->getWeight(v);


    x_bound_with_c(a, bub, clb, &lbu1, &ubu1, &lbul1);

    if (a>0 && bub>=0 && clb>0 && v->getLb()<lbul1){
      lbu1 = -INFINITY;
    }

    if (a>0 && bub>=0 && clb<0 && v->getLb()<lbul1){
      lbu1 = -INFINITY;
    }
    
    if (a>0 && bub<0 && clb>0 && v->getLb()<lbul1){
      lbu1 = -INFINITY;
    }

    if (a>0 && bub<0 && clb<0 && v->getLb()<lbul1){
      lbu1 = -INFINITY;
    }


    x_bound_with_c(-a, -blb, -cub, &lbu2, &ubu2, &lbul2);

    if (-a>0 && -blb>=0 && -cub>0 && v->getLb()<lbul2){
      lbu2 = -INFINITY;
    }

    if (-a>0 && -blb>=0 && -cub<0 && v->getLb()<lbul1){
      lbu1 = -INFINITY;
    }

    if (-a>0 && -blb<0 && -cub>0 && v->getLb()<lbul2){
      lbu2 = -INFINITY;
    }

    if (-a>0 && -blb<0 && -cub<0 && v->getLb()<lbul1){
      lbu1 = -INFINITY;
    }    

  qvlb = std::max(lbu1, lbu2);
  qvub = std::min(ubu1, ubu2);

   
  if (qvlb > v->getLb()) {
      qfmod.push_back((VarBoundModPtr) new VarBoundMod(v, Lower, qvlb));
    }
    if (qvub < v->getUb()) {
      qfmod.push_back((VarBoundModPtr) new VarBoundMod(v, Upper, qvub));
    }
  
  }

  if (false==qfmod.empty()) {
      for (VarBoundModVector::iterator it=qfmod.begin(); it!=qfmod.end(); ++it) {
        (*it)->applyToProblem(p_);
        ++stats_.vBnd;
      }
    change = 1;

    }

  
}


void NlPresHandler::varBndsFromCons_(ProblemPtr p, bool apply_to_prob,
                                     bool *changed, ModQ* mods,
                                     SolveStatus &status)
{
  ConstraintPtr c;
  LinearFunctionPtr lf;
  NonlinearFunctionPtr nlf;
  FunctionPtr f;
  QuadraticFunctionPtr qf;
  
  double lfu, lfl, ub, lb;
  double ub1, lb1, ub2, lb2, ubc, lbc;
  VarBoundModVector dmods;
  int change1 = 0;
  int change2 = 0;

  
  VarBoundModVector lfmod;
  VarBoundModVector qfmod;
  

  VariableSet lfvars;
  VariableSet qfvars;
  VariableSet linear_terms;

  status = Started;

  for (ConstraintConstIterator cit=p->consBegin(); cit!=p->consEnd();
       ++cit) {
    c = *cit;
    
    // std::cout<<"Constraint no. =============="<< c->getName()<<"\n";

    if (c->getFunctionType()!=Linear && c->getFunctionType()!=Constant) {
      lf = c->getFunction()->getLinearFunction();

      qf = c->getFunction()->getQuadraticFunction();
      f = c->getFunction();
      
      nlf = c->getFunction()->getNonlinearFunction();


      lfu = lfl = 0.0;

      if (qf){
        ubc = c->getUb();
        lbc = c->getLb();
        // std::cout<<"ccccccccccccll"<<lb<<"\n";
        // std::cout<<"ccccccccccccuu"<<ub<<"\n";
        qf->computeBounds(&lb1, &ub1);
        if (lf) {
          lf->computeBounds(&lb2, &ub2);
        } else {
          lb2 = 0.0;
          ub2 = 0.0;
        }

        ub = std::min(ubc, ub1 + ub2);
        lb = std::max(lbc, lb1+lb2);
        // std::cout<<"cons__bounduuubbb"<<ub<<"\n";
        // std::cout<<"cons_boundlllllbb"<<lb<<"\n";
      
        if (lf) {
          lf->getVars(&lfvars);
          qf->getVars(&qfvars);
          linear_var_set(lfvars, f, &linear_terms);
          
          lin_var_bound(lfmod, lf, qf, qfvars, linear_terms, lb, ub, change1);
          quad_var_bound(qfmod, lf, qf, qfvars,  linear_terms, lb, ub,  change2);
        } else {
          change1 = 0;
          change2 = 0;
        }
        //std::cout<<"kkkkkkkkkkkk============"<<qfmod.empty();
        // std::cout<<"change1"<<change1<<"\n";
        // std::cout<<"change2"<<change2<<"\n";
        if (change1 == 1 || change2 == 1){
          // std::cout<<"channnnnnnnnnnnnggggggggggeeeeeeeeee"<<"\n";
          *changed = true;
        }
        
        lfvars.clear();
        qfvars.clear();
        linear_terms.clear();
      }
      if (nlf){
        if (lf) {
          lf->computeBounds(&lfl, &lfu);
        }
        ub = c->getUb()-lfl;
        lb = c->getLb()-lfu;
        nlf->varBoundMods(lb, ub, dmods, &status);
      }
      
      if (SolvedInfeasible == status) {
        dmods.clear();
        break;
      } else if (SolveError == status) {
        dmods.clear();
        break;
      }
      if (false==dmods.empty()) {
        for (VarBoundModVector::iterator it=dmods.begin(); it!=dmods.end(); ++it) {
          (*it)->applyToProblem(p);
          ++stats_.vBnd;
#if SPEW
          logger_->msgStream(LogDebug2) << me_ << " ";
          (*it)->write(logger_->msgStream(LogDebug2));
#endif
          if (false==apply_to_prob) {
            mods->push_back(*it);
          } else {
            delete (*it);
          }
        }
        dmods.clear();
        *changed = true;
      }
    }
  }
}


void NlPresHandler::writePreStats(std::ostream &out) const
{
  out << me_ << "Statistics for presolve by NlPresHandler:"        << std::endl
    << me_ << "Number of iterations           = " << stats_.iters  << std::endl
    << me_ << "Time taken in initial presolve = " << stats_.time   << std::endl
    << me_ << "Time taken in node presolves   = " << stats_.timeN  << std::endl
    << me_ << "Number of variables deleted    = " << stats_.varDel << std::endl
    << me_ << "Number of constraints deleted  = " << stats_.conDel << std::endl
    << me_ << "Number of constraints relaxed  = " << stats_.conRel << std::endl
    << me_ << "Number of perspective reform.  = " << stats_.pRefs  << std::endl
    << me_ << "Times variables tightened      = " << stats_.vBnd   << std::endl
    << me_ << "Times constraints tightened    = " << stats_.cBnd   << std::endl
    << me_ << "Times coefficients improved    = " << stats_.cImp   << std::endl
    << me_ << "Times quad. changed to conic   = " << stats_.qCone  << std::endl
    << me_ << "Changes in nodes               = " << stats_.nMods  << std::endl
    ;
}


void NlPresHandler::writeStats(std::ostream &out) const
{
  writePreStats(out);
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
