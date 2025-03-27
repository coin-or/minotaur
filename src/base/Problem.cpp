//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Problem.cpp
 * \brief Define base class Problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string.h>  // for memset

#include "Environment.h"
#include "MinotaurConfig.h"
#include "Operations.h"
#include "Problem.h"
using namespace Minotaur;
const std::string Problem::me_ = "Problem: ";
Problem::Problem(EnvPtr env)
  : cons_(0),
    consModed_(false),
    debugSol_(0),
    engine_(0),
    hessian_(0),
    jacobian_(0),
    nativeDer_(false),
    nextCId_(0),
    nextSId_(0),
    nextVId_(0),
    numDCons_(0),
    numDVars_(0),
    obj_(0),
    size_(0),
    vars_(0),
    varsModed_(false)

{
  logger_ = env->getLogger();
}

Problem::~Problem()
{
  VariableIterator viter;
  ConstraintIterator citer;
  SOSIterator siter;
  VariablePtr v;

  for (viter = vars_.begin(); viter != vars_.end(); viter++) {
    v = *viter;
    v->clearConstraints_();
    delete v;
  }

  for (viter = varsRem_.begin(); viter != varsRem_.end(); viter++) {
    delete *viter;
  }

  for (citer = cons_.begin(); citer != cons_.end(); ++citer) {
    delete *citer;
  }
  for (siter = sos1_.begin(); siter != sos1_.end(); siter++) {
    delete *siter;
  }
  for (siter = sos2_.begin(); siter != sos2_.end(); siter++) {
    delete *siter;
  }
  if (engine_) {
    engine_->clear();
  }
  if (hessian_) {
    delete hessian_;
  }
  if (jacobian_) {
    delete jacobian_;
  }
  if (size_) {
    delete size_;
  }
  if (obj_) {
    delete obj_;
  }
  if (debugSol_) {
    delete debugSol_;
  }
  vars_.clear();
  cons_.clear();
  sos1_.clear();
  sos2_.clear();
}

void Problem::addToObj(LinearFunctionPtr lf)
{
  assert(engine_ == 0 ||
         (!"Cannot change objective after loading problem to engine\n"));
  if (obj_) {
    obj_->add_(lf);
  } else {
    assert(!"Cannot add lf to an empty objective!");
  }
  consModed_ = true;
}

void Problem::addToObj(double c)
{
  assert(engine_ == 0 ||
         (!"Cannot change objective after loading problem to engine\n"));
  if (obj_) {
    obj_->add_(c);
  } else {
    assert(!"Cannot add c to an empty objective!");
  }
  consModed_ = true;
}

void Problem::addToCons(ConstraintPtr cons, double c)
{
  cons->add_(c);
}

void Problem::calculateSize(bool shouldRedo)
{
  if (!size_) {
    shouldRedo = true;
    size_ = (ProblemSizePtr) new ProblemSize();
  }

  if (consModed_ || varsModed_) {
    shouldRedo = true;
  }

  if (shouldRedo) {
    size_->vars = vars_.size();
    size_->cons = cons_.size();
    size_->objs = (obj_) ? 1 : 0;

    countVarTypes_();
    countConsTypes_();
    countObjTypes_();
  }
  consModed_ = false;
  varsModed_ = false;
  return;
}

void Problem::changeBoundByInd(UInt ind, BoundType lu, double new_val)
{

  assert(
      ind < vars_.size() ||
      !"Problem::changeBound: index of variable exceeds no. of variables.");

  if (lu == Lower) {
    vars_[ind]->setLb_(new_val);
  } else {
    vars_[ind]->setUb_(new_val);
  }
  if (engine_) {
    engine_->changeBound(vars_[ind], lu, new_val);
  }
}

void Problem::changeBoundByInd(UInt ind, double new_lb, double new_ub)
{

  assert(
      ind < vars_.size() ||
      !"Problem::changeBound: index of variable exceeds no. of variables.");

  vars_[ind]->setLb_(new_lb);
  vars_[ind]->setUb_(new_ub);
  if (engine_) {
    engine_->changeBound(vars_[ind], new_lb, new_ub);
  }
}

void Problem::changeBound(VariablePtr var, BoundType lu, double new_val)
{

  assert(var == vars_[var->getIndex()] ||
         !"Problem: Bound of variable not in a problem can't be changed.");

  if (lu == Lower) {
    var->setLb_(new_val);
  } else {
    var->setUb_(new_val);
  }
  if (engine_) {
    engine_->changeBound(var, lu, new_val);
  }
}

void Problem::changeBound(VariablePtr var, double new_lb, double new_ub)
{

  assert(
      var == vars_[var->getIndex()] ||
      !"Problem: Bound of variable that is not in problem can't be changed.");

  var->setLb_(new_lb);
  var->setUb_(new_ub);
  if (engine_) {
    engine_->changeBound(var, new_lb, new_ub);
  }
}

void Problem::changeBound(ConstraintPtr con, double new_lb, double new_ub)
{

  assert(con == cons_[con->getIndex()] ||
         !"Problem: Bound of constraint that is not in problem can't be "
          "changed.");
  assert(engine_ == 0 ||
         (!"Cannot change constraint after loading problem to engine\n"));

  con->setLb_(new_lb);
  con->setUb_(new_ub);
  consModed_ = true;
}

void Problem::changeBound(ConstraintPtr con, BoundType lu, double new_val)
{

  assert(con == cons_[con->getIndex()] ||
         !"Problem: Bound of constraint that is not in problem can't be "
          "changed.");

  if (engine_) {
    engine_->changeBound(con, lu, new_val);
  }
  if (lu == Lower) {
    con->setLb_(new_val);
  } else {
    con->setUb_(new_val);
  }
  consModed_ = true;
}

void Problem::changeConstraint(ConstraintPtr con, NonlinearFunctionPtr nlf)
{
  // simply replacing lf is sufficient to take care of jacobian and hessian as
  // well.

  FunctionPtr f = con->getFunction();

  assert(f);
  assert(con == getConstraint(con->getIndex()));

  // It is important to apply changes to engine first. Some engines use the
  // old constraint stored in problem to make changes.
  if (engine_) {
    engine_->changeConstraint(con, nlf);
  }

  for (VarSet::iterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    (*vit)->outOfConstraint_(con);
  }

  con->changeNlf_(nlf);

  f = con->getFunction();
  for (VarSet::iterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    (*vit)->inConstraint_(con);
  }
  consModed_ = true;
}

void Problem::changeConstraint(ConstraintPtr con, LinearFunctionPtr lf,
                               double lb, double ub)
{
  // simply replacing lf is sufficient to take care of jacobian and hessian as
  // well.

  FunctionPtr f = con->getFunction();

  assert(f);
  assert(con == getConstraint(con->getIndex()));

  // It is important to apply changes to engine first. Some engines use the
  // old constraint stored in problem to make changes.
  if (engine_) {
    engine_->changeConstraint(con, lf, lb, ub);
  }

  for (VarSet::iterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    (*vit)->outOfConstraint_(con);
  }

  con->changeLf_(lf);
  con->setLb_(lb);
  con->setUb_(ub);

  f = con->getFunction();
  for (VarSet::iterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
    (*vit)->inConstraint_(con);
  }
  consModed_ = true;
}

void Problem::changeObj(FunctionPtr f, double cb)
{
  std::string name = (obj_) ? obj_->getName() : "obj";
  if (engine_) {
    engine_->changeObj(f, cb);
  }

  if (obj_) {
    delete obj_;
  }
  obj_ = (ObjectivePtr) new Objective(f, cb, Minimize, name);
  consModed_ = true;
}

int Problem::checkConVars() const
{
  ConstraintPtr c;
  VariablePtr v;
  int err = 0;

  for (ConstraintConstIterator citer = cons_.begin(); citer != cons_.end();
       ++citer) {
    c = *citer;
    for (VarSet::iterator vit = c->getFunction()->varsBegin();
         vit != c->getFunction()->varsEnd(); ++vit) {
      v = *vit;
      if (v != vars_[v->getIndex()]) {
        err = 1;
        logger_->msgStream(LogError)
            << me_ << "variable " << v->getName() << " in constraint "
            << c->getName() << " is not a variable of this problem."
            << std::endl;
        c->write(logger_->msgStream(LogError));
        logger_->msgStream(LogError) << std::endl;
      }
    }
  }
  return err;
}


void Problem::classifyCon()
{
  const double tol = 1e-5;
  ConstraintPtr c;
  FunctionPtr f;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  bool isClassified;
  bool isDiagQuadClassified;

  resConTypCnt();

  for (ConstraintConstIterator citer = cons_.begin(); citer != cons_.end();
       ++citer) {
    ConstraintStats stats;
    c = *citer;
    f = c->getFunction();
    isClassified = false;
    isDiagQuadClassified = false;
    if (f->getType() == Quadratic) {
      c->setType(Quad);
      qf = f->getQuadraticFunction();
      lf = f->getLinearFunction();
      if (qf) {
        stats.nterms = qf->getNumTerms();
      } else {
        // Handle the null qf case
        stats.nterms = 0;
      }
      if (lf) {
        stats.nvars = lf->getNumTerms();
      }
      stats.nsqterm = 0;
      stats.nbilterm = 0;
      stats.nposcoefone = 0;
      stats.nnegcoefone = 0;
      stats.wt1 = 0;
      stats.wt2 = 0;
      stats.eqwt = 0;
      if (qf) {
        VariablePairGroupConstIterator it0 = qf->begin();
        double wt0 = it0->second;
        for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
             ++it) {
          VariablePair vp = it->first;
          double wt = it->second;
          if (abs(wt - 1) < tol) {
            ++stats.nposcoefone;
          } else if (abs(wt + 1) < tol) {
            ++stats.nnegcoefone;
          }
          if (wt > tol) {
            ++stats.nposcoef;
          } else if (wt < -tol) {
            ++stats.nnegcoef;
          }
          if (vp.first == vp.second) {
            ++stats.nsqterm;
          } else {
            ++stats.nbilterm;
          }
          if (abs(wt0 - wt) < tol) {
            ++stats.eqwt;
          }
        }
      }
      if (stats.nterms > 2) {
        if (!isDiagQuadClassified) {
          isDiagQuadClassified =
              isDiagonalQuadratic_(c, stats) || isDiagQuadClassified;
        }
        if (!isClassified) {
          isClassified = isSimpleBall_(c, stats) || isClassified;
        }
        if (!isClassified) {
          isClassified = isEllipsoid_(c, stats) || isClassified;
        }
        if (!isClassified) {
          isClassified = isComplementSimpleBall_(c, stats) || isClassified;
        }
        if (!isClassified) {
          isClassified = isComplementEllipsoid_(c, stats) || isClassified;
        }
        if (!isClassified) {
          isClassified = isSocSimple_(c, stats) || isClassified;
        }
        if (!isClassified) {
          isClassified = isPureBilinear_(c, stats) || isClassified;
        }
      } else {
        if (!isDiagQuadClassified) {
          isDiagQuadClassified =
              isDiagonalQuadratic_(c, stats) || isDiagQuadClassified;
        }
        if (!isClassified) {
          isClassified = isPureBilinear_(c, stats) || isClassified;
        }
        if (stats.nterms == 2) {
          if (!isClassified) {
            isClassified = isSocSimple_(c, stats) || isClassified;
          }
        }
      }
    } else if (f->getType() != Constant && f->getType() != Linear) {
      c->setType(NonLin);
    } else if (f->getType() == Linear) {
      lf = f->getLinearFunction();
      stats.nvars = lf->getNumTerms();
      stats.nposcoefone = 0;
      stats.nnegcoefone = 0;
      stats.nposcoef = 0;
      stats.nnegcoef = 0;
      stats.nposcont = 0;
      stats.nnegcont = 0;
      stats.nposbin = 0;
      stats.nnegbin = 0;
      stats.nposint = 0;
      stats.nnegint = 0;
      stats.wt1 = 0;
      stats.wt2 = 0;
      stats.sumnegwt = 0;
      stats.con = 0;
      for (VariableGroupConstIterator it = lf->termsBegin();
           it != lf->termsEnd(); ++it) {
        VariablePtr v = it->first;
        double wt = it->second;
        if (f->getNumVars() == 2) {
          if (wt > tol) {
            stats.wt1 = wt;
          } else if (wt < tol) {
            stats.wt2 = wt;
          }
        }
        if (abs(wt - 1) < tol) {
          ++stats.nposcoefone;
        } else if (abs(wt + 1) < tol) {
          ++stats.nnegcoefone;
        }
        if (wt > tol) {
          ++stats.nposcoef;
        } else if (wt < -tol) {
          ++stats.nnegcoef;
        }
        if (wt > tol && v->getType() != Binary && v->getType() != Integer &&
            v->getType() != ImplBin && v->getType() != ImplInt) {
          ++stats.nposcont;
        } else if (wt < -tol && v->getType() != Binary &&
                   v->getType() != Integer && v->getType() != ImplInt &&
                   v->getType() != ImplBin) {
          ++stats.nnegcont;
        }
        if (v->getType() == Binary || v->getType() == ImplBin) {
          if (wt > tol) {
            ++stats.nposbin;
          } else if (wt < -tol) {
            ++stats.nnegbin;
            stats.sumnegwt += abs(wt);
          }
        }
        if (v->getType() == Integer || v->getType() == ImplInt) {
          if (wt > tol) {
            ++stats.nposint;
          } else if (wt < -tol) {
            ++stats.nnegint;
          }
        }
      }
      // Continuing from the previous for-loop where stats are calculated...
      double sumnegwt = stats.sumnegwt;
      for (VariableGroupConstIterator it2 = lf->termsBegin();
           it2 != lf->termsEnd(); ++it2) {
        double wtn = it2->second;
        VariablePtr vn = it2->first;
        if (vn->getType() == Binary || vn->getType() == ImplBin) {
          if (c->getUb() + sumnegwt == std::abs(wtn)) {
            ++stats.con;
          }
        }
      }
      // Now pass the populated stats structure to each checker function
      if (stats.nvars == 2) {
        if (!isClassified) {
          isClassified = isAggregation_(c) || isClassified;
        }
        if (!isClassified) {
          isClassified = isPrecedence_(c, stats) || isClassified;
        }
        if (!isClassified) {
          isClassified = isVariableBound_(c, stats) || isClassified;
        }
      } else if (stats.nposbin + stats.nnegbin == stats.nvars) {
        if (stats.nposcoefone + stats.nnegcoefone == stats.nvars) {
          if (!isClassified) {
            isClassified = isSetPartitioning_(c, stats) || isClassified;
          }
          if (!isClassified) {
            isClassified = isSetPacking_(c, stats) || isClassified;
          }
          if (!isClassified) {
            isClassified = isSetCovering_(c, stats) || isClassified;
          }
          if (!isClassified) {
            isClassified = isCardinality_(c, stats) || isClassified;
          }
          if (!isClassified) {
            isClassified = isInvariantKnapsack_(c, stats) || isClassified;
          }
        } else if (stats.nposcoef + stats.nnegcoef == stats.nvars) {
          if (!isClassified) {
            isClassified = isEquationKnapsack_(c, stats) || isClassified;
          }
          if (!isClassified) {
            isClassified = isBinPacking_(c, stats) || isClassified;
          }
          if (!isClassified) {
            isClassified = isKnapsack_(c, stats) || isClassified;
          }
        }
      }
      if (!isClassified) {
        isClassified = isIntegerKnapsack_(c, stats) || isClassified;
      }
      if (!isClassified) {
        isClassified = isMixedBinary_(c, stats) || isClassified;
      }
      if (!isClassified) {
        isClassified = isNoSpecificStructure_(c) || isClassified;
      }
    }
  }
  printConstraintStatistics_();
  printConstraintStatisticsQuad_();
}

bool Problem::isAggregation_(ConstraintPtr c)
{
  if (c->getLb() == c->getUb()) {
    c->setType(Aggregation);
    ++size_->countAggregation;
    return true;
  }
  return false;
}

bool Problem::isPrecedence_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (stats.wt1 == -(stats.wt2) &&
      (c->getUb() <= INFTY && c->getLb() >= -INFTY) &&
      ((stats.nposbin + stats.nnegbin == 2) ||
       (stats.nposint + stats.nnegint == 2) ||
       (stats.nposcont + stats.nnegcont == 2))) {
    c->setType(Precedence);
    ++size_->countPrecedence;
    return true;
  }
  return false;
}

bool Problem::isVariableBound_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (((stats.nposbin + stats.nnegbin == 2) ||
       (stats.nposcoefone + stats.nnegcoefone == 2) ||
       ((stats.nposbin + stats.nnegbin == 1) &&
        (stats.nposcont + stats.nnegcont) == 1) ||
       ((stats.nposbin + stats.nnegbin == 1) &&
        (stats.nposint + stats.nnegint) == 1)) &&
      (c->getUb() <= INFTY && c->getLb() >= -INFTY)) {
    c->setType(VariableBound);
    ++size_->countVariableBound;
    return true;
  }
  return false;
}

bool Problem::isSetPartitioning_(ConstraintPtr c,
                                 const ConstraintStats& stats)
{
  if (((c->getLb() == 1 - stats.nnegcoefone &&
        c->getUb() == 1 - stats.nnegcoefone) ||
       (c->getLb() == stats.nposcoefone - 1 &&
        c->getUb() == stats.nposcoefone - 1))) {
    c->setType(SetPartition);
    ++size_->countSetPartitioning;
    return true;
  }
  return false;
}

bool Problem::isSetPacking_(ConstraintPtr c, const ConstraintStats& stats)
{
  if ((c->getLb() == stats.nposcoefone - 1 ||
       c->getUb() == 1 - stats.nnegcoefone) &&
      (c->getLb() >= -INFTY || c->getUb() <= INFTY)) {
    c->setType(SetPack);
    ++size_->countSetPacking;
    return true;
  }
  return false;
}

bool Problem::isSetCovering_(ConstraintPtr c, const ConstraintStats& stats)
{
  if ((c->getLb() == 1 - stats.nnegcoefone && c->getUb() <= INFTY) ||
      (c->getUb() == stats.nposcoefone - 1 && c->getLb() >= -INFTY)) {
    c->setType(SetCover);
    ++size_->countSetCovering;
    return true;
  }
  return false;
}

bool Problem::isCardinality_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (c->getLb() == c->getUb() && c->getUb() <= 2 + stats.nnegcoefone) {
    c->setType(Cardinality);
    ++size_->countCardinality;
    return true;
  }
  return false;
}

bool Problem::isInvariantKnapsack_(ConstraintPtr c,
                                   const ConstraintStats& stats)
{
  if (((c->getUb() >= 2 - stats.nnegcoefone && c->getLb() == -INFTY) ||
       (c->getLb() == stats.nposcoefone - 2 && c->getUb() == INFTY)) &&
      (isInt(c->getUb()))) {
    c->setType(InvariantKnapsack);
    ++size_->countInvariantKnapsack;
    return true;
  }
  return false;
}

bool Problem::isEquationKnapsack_(ConstraintPtr c,
                                  const ConstraintStats& stats)
{
  if ((c->getUb() + stats.sumnegwt >= 2 && c->getUb() == c->getLb()) &&
      (isInt(c->getUb()))) {
    c->setType(EquationKnapsack);
    ++size_->countEquationKnapsack;
    return true;
  }
  return false;
}

bool Problem::isBinPacking_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (c->getUb() + stats.sumnegwt >= 2 && (isInt(c->getUb())) &&
      stats.con >= 1) {
    c->setType(BinPack);
    ++size_->countBinPacking;
    return true;
  }
  return false;
}

bool Problem::isKnapsack_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (((c->getLb() + 2 <= stats.sumnegwt && c->getUb() <= INFTY) &&
       (isInt(c->getLb()))) ||
      (c->getUb() + stats.sumnegwt >= 2 && c->getLb() >= -INFTY &&
       (isInt(c->getUb())))) {
    c->setType(Knapsack);
    ++size_->countKnapsack;
    return true;
  }
  return false;
}

bool Problem::isIntegerKnapsack_(ConstraintPtr c,
                                 const ConstraintStats& stats)
{
  if (stats.nposint + stats.nnegint == stats.nvars && (isInt(c->getUb()))) {
    if (c->getLb() >= -INFTY && c->getUb() <= INFTY) {
      c->setType(IntegerKnapsack);
      ++size_->countIntegerKnapsack;
      return true;
    }
  }
  return false;
}

bool Problem::isMixedBinary_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (stats.nposcont + stats.nnegcont + stats.nposbin + stats.nnegbin ==
      stats.nvars) {
    if (((c->getLb() >= -INFTY && c->getUb() <= INFTY) ||
         (c->getUb() == c->getLb())) &&
        (stats.nposbin + stats.nnegbin >= 1)) {
      c->setType(MixedBinary);
      ++size_->countMixedBinary;
      return true;
    }
  }
  return false;
}

bool Problem::isNoSpecificStructure_(ConstraintPtr c)
{
  ++size_->countNoSpecificStructure;
  c->setType(General);
  return true;
}

bool Problem::isDiagonalQuadratic_(ConstraintPtr c,
                                   const ConstraintStats& stats)
{
  if (stats.nsqterm == stats.nterms + stats.nvars) {
    c->setType(DiagQuad);
    ++size_->countDiagQuad;
    return true;
  }
  return false;
}

bool Problem::isSimpleBall_(ConstraintPtr c, const ConstraintStats& stats)
{
  const double lb = c->getLb();
  const double ub = c->getUb();

  if ((ub > 0 && lb == -INFTY && checkQuadCond(stats) &&
       checkCoeffCond(stats, true)) ||
      (ub == INFTY && lb < 0 && checkQuadCond(stats) &&
       checkCoeffCond(stats, false))) {
    c->setType(SimpleBall);
    ++size_->countSimpleBall;
    return true;
  }
  return false;
}

bool Problem::isComplementSimpleBall_(ConstraintPtr c,
                                      const ConstraintStats& stats)
{
  const double lb = c->getLb();
  const double ub = c->getUb();

  if ((ub == INFTY && lb > 0 && checkQuadCond(stats) &&
       checkCoeffCond(stats, true)) ||
      (ub < 0 && lb == -INFTY && checkQuadCond(stats) &&
       checkCoeffCond(stats, false))) {
    c->setType(ComplementSimpleBall);
    ++size_->countComplementSimpleBall;
    return true;
  }
  return false;
}

bool Problem::isEllipsoid_(ConstraintPtr c, const ConstraintStats& stats)
{
  const double lb = c->getLb();
  const double ub = c->getUb();

  if ((lb == -INFTY && ub > 0 && ub < INFTY && checkQuadCond(stats) &&
       stats.nposcoef == stats.nterms + stats.nvars) ||
      (lb < 0 && ub == INFTY && checkQuadCond(stats) &&
       stats.nnegcoef == stats.nterms + stats.nvars)) {
    c->setType(Ellipsoid);
    ++size_->countEllipsoid;
    return true;
  }
  return false;
}

bool Problem::isComplementEllipsoid_(ConstraintPtr c,
                                     const ConstraintStats& stats)
{
  const double lb = c->getLb();
  const double ub = c->getUb();

  if ((lb > 0 && ub == INFTY && checkQuadCond(stats) &&
       stats.nposcoef == stats.nterms + stats.nvars) ||
      (lb == -INFTY && ub < 0 && checkQuadCond(stats) &&
       stats.nnegcoef == stats.nterms + stats.nvars)) {
    c->setType(ComplementEllipsoid);
    ++size_->countComplementEllipsoid;
    return true;
  }
  return false;
}

bool Problem::isPureBilinear_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (stats.nbilterm == stats.nterms + stats.nvars) {
    c->setType(PureBilinear);
    ++size_->countPureBilinear;
    return true;
  }
  return false;
}

bool Problem::isSocSimple_(ConstraintPtr c, const ConstraintStats& stats)
{
  if (c->getLb() == -INFTY && c->getUb() >= 0 && stats.nnegcoef == 1 &&
      stats.nsqterm == stats.nterms) {
    if (c->getUb() == 0 && stats.nsqterm == stats.nterms + stats.nvars &&
        stats.nposcoef == stats.nterms + stats.nvars - 1) {
      QuadraticFunctionPtr qf = c->getQuadraticFunction();
      for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
           it++) {
        if (it->second < 0 && it->first.first->getLb() >= 0) {
          c->setType(SOCRepresentableQuad);
          ++size_->countQuadConvertSOC;
          return true;
        }
      }
    }
    if (stats.nsqterm == stats.nterms && stats.nvars == 1 && c->getUb() > 0) {
      QuadraticFunctionPtr qf = c->getQuadraticFunction();
      LinearFunctionPtr lf = c->getLinearFunction();
      VariableGroupConstIterator it0 = lf->termsBegin();
      if (it0->second > 0) {
        for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
             it++) {
          if (it->second < 0 &&
              it0->first->getId() == it->first.first->getId() &&
              abs(it->second) * (it->first.first->getLb()) >=
                  sqrt(c->getUb()) &&
              abs((it0->second / (sqrt(abs(it->second)) * sqrt(c->getUb())) -
                   2)) <= 1e-7) {
            c->setType(SOCRepresentableQuad);
            ++size_->countQuadConvertSOC;
            return true;
          }
        }
      }
      if (it0->second < 0) {
        for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
             it++) {
          if (it->second < 0 &&
              it0->first->getId() == it->first.first->getId() &&
              abs(it->second) * (it->first.first->getLb()) -
                      sqrt(c->getUb()) >=
                  0 &&
              abs((abs(it0->second) /
                       (sqrt(abs(it->second)) * sqrt(c->getUb())) -
                   2)) <= 1e-7) {
            c->setType(SOCRepresentableQuad);
            ++size_->countQuadConvertSOC;
            return true;
          }
        }
      }
    }
  }
  if (c->getLb() <= 0 && c->getUb() == INFTY &&
      stats.nnegcoef == stats.nterms - 1 && stats.nsqterm == stats.nterms) {
    if (c->getLb() == 0 && stats.nsqterm == stats.nterms + stats.nvars &&
        stats.nnegcoef == stats.nterms + stats.nvars - 1) {
      QuadraticFunctionPtr qf = c->getQuadraticFunction();
      for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
           it++) {
        if (it->second > 0 && it->first.first->getLb() >= 0) {
          c->setType(SOCRepresentableQuad);
          ++size_->countQuadConvertSOC;
          return true;
        }
      }
    }
    if (stats.nsqterm == stats.nterms && stats.nvars == 1 && c->getLb() < 0) {
      QuadraticFunctionPtr qf = c->getQuadraticFunction();
      LinearFunctionPtr lf = c->getLinearFunction();
      VariableGroupConstIterator it0 = lf->termsBegin();
      if (it0->second < 0) {
        for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
             it++) {
          if (it->second > 0 &&
              it0->first->getId() == it->first.first->getId() &&
              (it->second) * (it->first.first->getLb()) >=
                  sqrt(-c->getLb()) &&
              abs((-it0->second / (sqrt(it->second) * sqrt(-c->getLb())) -
                   2)) <= 1e-7) {
            c->setType(SOCRepresentableQuad);
            ++size_->countQuadConvertSOC;
            return true;
          }
        }
      }
      if (it0->second > 0) {
        for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end();
             it++) {
          if (it->second > 0 &&
              it0->first->getId() == it->first.first->getId() &&
              it->second * (it->first.first->getLb()) - sqrt(-c->getLb()) >=
                  0 &&
              abs((it0->second / (sqrt(it->second) * sqrt(-c->getLb())) -
                   2)) <= 1e-7) {
            c->setType(SOCRepresentableQuad);
            ++size_->countQuadConvertSOC;
            return true;
          }
        }
      }
    }
  }
  return false;
}

void Problem::printConstraintStatistics_()
{
  const int wid = 8;
  logger_->msgStream(LogExtraInfo)
      << "-------------------------------" << std::endl
      << "|   Linear Constraint Types   |\n"
      << "|-----------------------------|\n"
      << "|Aggregation        |" << std::setw(wid) << size_->countAggregation
      << " |\n"
      << "|Precedence         |" << std::setw(wid) << size_->countPrecedence
      << " |\n"
      << "|Variable Bound     |" << std::setw(wid)
      << size_->countVariableBound << " |\n"
      << "|Partitioning       |" << std::setw(wid)
      << size_->countSetPartitioning << " |\n"
      << "|Set Packing        |" << std::setw(wid) << size_->countSetPacking
      << " |\n"
      << "|Set Covering       |" << std::setw(wid) << size_->countSetCovering
      << " |\n"
      << "|Cardinality        |" << std::setw(wid) << size_->countCardinality
      << " |\n"
      << "|Invariant Knapsack |" << std::setw(wid)
      << size_->countInvariantKnapsack << " |\n"
      << "|Equation Knapsack  |" << std::setw(wid)
      << size_->countEquationKnapsack << " |\n"
      << "|Bin Packing        |" << std::setw(wid) << size_->countBinPacking
      << " |\n"
      << "|Knapsack           |" << std::setw(wid) << size_->countKnapsack
      << " |\n"
      << "|Integer Knapsack   |" << std::setw(wid)
      << size_->countIntegerKnapsack << " |\n"
      << "|Mixed Binary       |" << std::setw(wid) << size_->countMixedBinary
      << " |\n"
      << "|Others             |" << std::setw(wid)
      << size_->countNoSpecificStructure << " |\n"
      << "-------------------------------\n";
}

void Problem::printConstraintStatisticsQuad_()
{
  const int wid = 8;
  logger_->msgStream(LogExtraInfo)
      << "------------------------------------" << std::endl
      << "|   Quadratic Constraint Types     |\n"
      << "|----------------------------------|\n"
      << "|Diagonal Quadratic       |" << std::setw(wid)
      << size_->countDiagQuad << "|\n"
      << "|Simple Ball              |" << std::setw(wid)
      << size_->countSimpleBall << "|\n"
      << "|Ellipsoid                |" << std::setw(wid)
      << size_->countEllipsoid << "|\n"
      << "|Complement Simple Ball   |" << std::setw(wid)
      << size_->countComplementSimpleBall << "|\n"
      << "|Complement Ellipsoid     |" << std::setw(wid)
      << size_->countComplementEllipsoid << "|\n"
      << "|Pure Bilinear            |" << std::setw(wid)
      << size_->countPureBilinear << "|\n"
      << "|SOC Representable        |" << std::setw(wid)
      << size_->countQuadConvertSOC << "|\n"
      << "------------------------------------\n";
}

// Lock Number Calculation (Not used anywhere)

/*
void Problem::lockNum_() {
 std::map<VariablePtr, std::pair<int, int>> Locknumber;
 for(ConstraintConstIterator citer = cons_.begin(); citer != cons_.end();
      ++citer) {
 ConstraintPtr c = *citer;
 FunctionPtr f = c->getFunction();
 LinearFunctionPtr lf = f->getLinearFunction();
 for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd();
++it) { VariablePtr v = it->first; double wt = it->second; if (c->getUb() ==
c->getLb()) { Locknumber[v].first += 1;  // Increment the count for the upper
lock Locknumber[v].second += 1; // Increment the count for the lower lock
  }else {
    if (wt > 0) {
     Locknumber[v].first += 1; // Increment the count for the upper lock
  }else {
     Locknumber[v].second += 1; // Increment the count for the lower lock
    }
   }
  }
 }
}
*/

// Does not clone Jacobian and Hessian yet.
ProblemPtr Problem::clone(EnvPtr env) const
{
  ConstraintPtr c;
  ConstConstraintPtr cc;
  VariablePtr v;
  ConstVariablePtr cv;
  FunctionPtr f;
  ObjectivePtr oPtr;
  VariableConstIterator vit0;
  int err = 0;
  VarVector vvec;

  ProblemPtr clonePtr = (ProblemPtr) new Problem(env);

  // Copy the variables.
  for (VariableConstIterator it = vars_.begin(); it != vars_.end(); ++it) {
    cv = *it;
    v = clonePtr->newVariable(cv->getLb(), cv->getUb(), cv->getType(),
                              cv->getName(), cv->getSrcType());
    v->setState_(cv->getState());
    v->setSrcType(cv->getSrcType());
    v->setFunType_(cv->getFunType());
    v->setId_(cv->getId());
    v->setInitVal_(cv->getInitVal());
  }

  vit0 = clonePtr->varsBegin();
  // add constraints
  for (ConstraintConstIterator it = cons_.begin(); it != cons_.end(); ++it) {
    cc = *it;
    // clone the function.
    f = cc->getFunction()->cloneWithVars(vit0, &err);
    assert(err == 0);
    c = clonePtr->newConstraint(f, cc->getLb(), cc->getUb(), cc->getName());
    c->setId_(cc->getId());
    c->setState_(cc->getState());
  }

  // copy SOS1 constraints
  vvec.clear();
  for (SOSConstIterator it = sos1_.begin(); it != sos1_.end(); ++it) {
    for (VariableConstIterator it2 = (*it)->varsBegin();
         it2 != (*it)->varsEnd(); ++it2) {
      vvec.push_back(*it2);
    }
    clonePtr->newSOS((*it)->getNz(), (*it)->getType(), (*it)->getWeights(),
                     vvec, (*it)->getPriority());
    vvec.clear();
  }

  // add objective
  oPtr = getObjective();
  if (oPtr) {
    if (oPtr->getFunction()) {
      f = oPtr->getFunction()->cloneWithVars(vit0, &err);
      assert(err == 0);
      clonePtr->newObjective(f, oPtr->getConstant(), oPtr->getObjectiveType(),
                             oPtr->getName());
    } else {
      clonePtr->newObjective(oPtr->getConstant(), oPtr->getObjectiveType());
    }
  }

  clonePtr->jacobian_ = JacobianPtr();  // NULL.
  clonePtr->nextCId_ = nextCId_;
  clonePtr->nextSId_ = nextSId_;
  clonePtr->nextVId_ = nextVId_;
  clonePtr->hessian_ = HessianOfLagPtr();  // NULL.
  clonePtr->numDVars_ = numDVars_;
  clonePtr->numDCons_ = numDCons_;
  clonePtr->engine_ = 0;
  clonePtr->consModed_ = consModed_;
  clonePtr->varsModed_ = varsModed_;

  // clone size
  if (size_) {
    clonePtr->size_ = (ProblemSizePtr) new ProblemSize();
    clonePtr->size_->vars = size_->vars;
    clonePtr->size_->cons = size_->cons;
    clonePtr->size_->objs = size_->objs;
    clonePtr->size_->bins = size_->bins;
    clonePtr->size_->fixed = size_->fixed;
    clonePtr->size_->ints = size_->ints;
    clonePtr->size_->conts = size_->conts;
    clonePtr->size_->linCons = size_->linCons;
    clonePtr->size_->SOS1Cons = size_->SOS1Cons;
    clonePtr->size_->SOS2Cons = size_->SOS2Cons;
    clonePtr->size_->bilinCons = size_->bilinCons;
    clonePtr->size_->multilinCons = size_->multilinCons;
    clonePtr->size_->quadCons = size_->quadCons;
    clonePtr->size_->nonlinCons = size_->nonlinCons;
    clonePtr->size_->consWithLin = size_->consWithLin;
    clonePtr->size_->consWithBilin = size_->consWithBilin;
    clonePtr->size_->consWithMultilin = size_->consWithMultilin;
    clonePtr->size_->consWithQuad = size_->consWithQuad;
    clonePtr->size_->consWithNonlin = size_->consWithNonlin;
    clonePtr->size_->linTerms = size_->linTerms;
    clonePtr->size_->multiLinTerms = size_->multiLinTerms;
    clonePtr->size_->quadTerms = size_->quadTerms;
    clonePtr->size_->objLinTerms = size_->objLinTerms;
    clonePtr->size_->objQuadTerms = size_->objQuadTerms;
    clonePtr->size_->objType = size_->objType;
  } else {
    clonePtr->size_ = ProblemSizePtr();  // NULL
  }
  clonePtr->nativeDer_ = nativeDer_;  // NULL

  return clonePtr;
}

ProblemPtr Problem::shuffle(bool varshuff, bool conshuff, EnvPtr env)
{
  ConstraintPtr c;
  ConstConstraintPtr cc;
  VariablePtr v;
  ConstVariablePtr cv;
  FunctionPtr f;
  ObjectivePtr obj;
  VariableConstIterator vit0;
  int err = 0;
  int i;
  VarVector shuf_vars;

  ProblemPtr newp = (ProblemPtr) new Problem(env);
  int rand_seedvar = 8;
  srand(rand_seedvar);

  if (varshuff == 1) {
    shuf_vars = vars_;
    std::random_shuffle(shuf_vars.begin(), shuf_vars.end());
    std::vector<UInt> variableindex;
    for (VariableConstIterator it = shuf_vars.begin(); it != shuf_vars.end();
         ++it) {
      cv = *it;
      variableindex.push_back(cv->getIndex());
    }

    std::vector<UInt> variableaddress(vars_.size());
    for (UInt j = 0; j != vars_.size(); ++j) {
      variableaddress[variableindex[j]] = j;
    }

    // Copy the variables.
    i = 0;
    for (VariableConstIterator it = shuf_vars.begin(); it != shuf_vars.end();
         ++it) {
      cv = *it;
      v = newp->newVariable(cv->getLb(), cv->getUb(), cv->getType(),
                            cv->getName(), cv->getSrcType());
      v->setState_(cv->getState());
      v->setSrcType(cv->getSrcType());
      v->setFunType_(cv->getFunType());
      v->setId_(i);
      v->setIndex_(i);
      v->setInitVal_(cv->getInitVal());
      i = i + 1;
    }

    vit0 = newp->vars_.begin();

    // add constraints
    if (conshuff == 1) {
      i = 0;
      ConstraintVector cons2 = cons_;
      std::random_shuffle(cons2.begin(), cons2.end());
      for (ConstraintConstIterator it = cons2.begin(); it != cons2.end();
           ++it) {
        cc = *it;
        // clone the function.
        f = cc->getFunction()->cloneWithVarsPermute(vit0, variableaddress,
                                                    &err);
        assert(err == 0);
        c = newp->newConstraint(f, cc->getLb(), cc->getUb(), cc->getName());
        c->setId_(i);
        c->setIndex_(i);
        c->setState_(cc->getState());
        ++i;
      }
    }

    else {
      for (ConstraintConstIterator it = cons_.begin(); it != cons_.end();
           ++it) {
        cc = *it;
        // clone the function.
        f = cc->getFunction()->cloneWithVarsPermute(vit0, variableaddress,
                                                    &err);
        assert(err == 0);
        c = newp->newConstraint(f, cc->getLb(), cc->getUb(), cc->getName());
        c->setId_(cc->getId());
        c->setState_(cc->getState());
      }
    }

    obj = getObjective();
    if (obj) {
      f = obj->getFunction()->cloneWithVarsPermute(vit0, variableaddress,
                                                   &err);
      assert(err == 0);
      newp->newObjective(f, obj->getConstant(), obj->getObjectiveType(),
                         obj->getName());
    }

  } else {
    // Copy the variables.
    for (VariableConstIterator it = vars_.begin(); it != vars_.end(); ++it) {
      cv = *it;
      v = newp->newVariable(cv->getLb(), cv->getUb(), cv->getType(),
                            cv->getName(), cv->getSrcType());
      v->setState_(cv->getState());
      v->setSrcType(cv->getSrcType());
      v->setFunType_(cv->getFunType());
      v->setId_(cv->getId());
      v->setInitVal_(cv->getInitVal());
    }

    vit0 = newp->vars_.begin();

    // add constraints

    if (conshuff == 1) {
      i = 0;
      ConstraintVector cons2 = cons_;
      std::random_shuffle(cons2.begin(), cons2.end());
      for (ConstraintConstIterator it = cons2.begin(); it != cons2.end();
           ++it) {
        cc = *it;
        // clone the function.
        f = cc->getFunction()->cloneWithVars(vit0, &err);
        assert(err == 0);
        c = newp->newConstraint(f, cc->getLb(), cc->getUb(), cc->getName());
        c->setId_(i);
        c->setIndex_(i);
        c->setState_(cc->getState());
        ++i;
      }
    }

    else {
      for (ConstraintConstIterator it = cons_.begin(); it != cons_.end();
           ++it) {
        cc = *it;
        // clone the function.
        f = cc->getFunction()->cloneWithVars(vit0, &err);
        assert(err == 0);
        c = newp->newConstraint(f, cc->getLb(), cc->getUb(), cc->getName());
        c->setId_(cc->getId());
        c->setState_(cc->getState());
      }
    }
    // add objective
    obj = getObjective();
    if (obj) {
      f = obj->getFunction()->cloneWithVars(vit0, &err);
      assert(err == 0);
      newp->newObjective(f, obj->getConstant(), obj->getObjectiveType(),
                         obj->getName());
    }
  }

  logger_->msgStream(LogError)
      << me_ << "Warning: "
      << "SOS1 not impemented in shuffle()" << std::endl;
  // copy SOS1 constraints
  // shuf_vars.clear();
  // for (SOSConstIterator it=sos1_.begin(); it!=sos1_.end(); ++it) {
  //   for (VariableConstIterator it2 = (*it)->varsBegin();
  //        it2!=(*it)->varsEnd(); ++it2) {
  //     shuf_vars.push_back(*it2);
  //   }
  //   newp->newSOS((*it)->getNz(), (*it)->getType(), (*it)->getWeights(),
  //                    shuf_vars, (*it)->getPriority());
  //   shuf_vars.clear();
  // }

  // Now clone everything else...
  newp->jacobian_ = JacobianPtr();  // NULL.
  newp->nextCId_ = nextCId_;
  newp->nextSId_ = nextSId_;
  newp->nextVId_ = nextVId_;
  newp->hessian_ = HessianOfLagPtr();  // NULL.
  newp->numDVars_ = numDVars_;
  newp->numDCons_ = numDCons_;
  newp->engine_ = 0;
  newp->consModed_ = consModed_;
  newp->varsModed_ = varsModed_;

  // clone size
  if (size_) {
    newp->size_ = (ProblemSizePtr) new ProblemSize();
    newp->size_->vars = size_->vars;
    newp->size_->cons = size_->cons;
    newp->size_->objs = size_->objs;
    newp->size_->bins = size_->bins;
    newp->size_->fixed = size_->fixed;
    newp->size_->ints = size_->ints;
    newp->size_->conts = size_->conts;
    newp->size_->linCons = size_->linCons;
    newp->size_->SOS1Cons = size_->SOS1Cons;
    newp->size_->SOS2Cons = size_->SOS2Cons;
    newp->size_->bilinCons = size_->bilinCons;
    newp->size_->multilinCons = size_->multilinCons;
    newp->size_->quadCons = size_->quadCons;
    newp->size_->nonlinCons = size_->nonlinCons;
    newp->size_->consWithLin = size_->consWithLin;
    newp->size_->consWithBilin = size_->consWithBilin;
    newp->size_->consWithMultilin = size_->consWithMultilin;
    newp->size_->consWithQuad = size_->consWithQuad;
    newp->size_->consWithNonlin = size_->consWithNonlin;
    newp->size_->linTerms = size_->linTerms;
    newp->size_->multiLinTerms = size_->multiLinTerms;
    newp->size_->quadTerms = size_->quadTerms;
    newp->size_->objLinTerms = size_->objLinTerms;
    newp->size_->objQuadTerms = size_->objQuadTerms;
    newp->size_->objType = size_->objType;
  } else {
    newp->size_ = ProblemSizePtr();  // NULL
  }
  newp->nativeDer_ = nativeDer_;  // Boolean

  // newp->write(std::cout);

  return newp;
}

void Problem::cg2qf()
{
  ConstraintPtr c;
  FunctionPtr f;

  double* mult = 0;
  int err = 0;
  double* x = 0;
  double* values = 0;
  double* grad = 0;

  UInt* irow = 0;
  UInt* jcol = 0;
  UInt nz = 0;

  VariableConstIterator vit0;
  QuadraticFunctionPtr qfnew;
  LinearFunctionPtr lfnew;
  FunctionPtr fnew;
  double nlconst = 0;
  std::map<FunctionPtr, std::pair<double, double>> newCons;

  vit0 = vars_.begin();

  setNativeDer();
  hessian_ = getHessian();
  nz = hessian_->getNumNz();
  mult = new double[getNumCons()];
  values = new double[nz];
  irow = new UInt[nz];
  jcol = new UInt[nz];
  x = new double[getNumVars()];

  grad = new double[getNumVars()];

  memset(mult, 0, getNumCons() * sizeof(double));
  memset(values, 0, nz * sizeof(UInt));
  memset(irow, 0, nz * sizeof(UInt));
  memset(jcol, 0, nz * sizeof(UInt));
  memset(x, 0, getNumVars() * sizeof(double));
  memset(grad, 0, getNumVars() * sizeof(double));

  hessian_->fillRowColIndices(irow, jcol);

  // objective
  f = getObjective()->getFunction();
  if (f) {
    if (Quadratic == f->getType()) {
      hessian_->fillRowColValues(x, 1.0, mult, values, &err);
      assert(0 == err);

      qfnew = (QuadraticFunctionPtr) new QuadraticFunction(nz, values, irow,
                                                           jcol, vit0);
      memset(grad, 0, getNumVars() * sizeof(double));

      f->evalGradient(x, grad, &err);
      assert(0 == err);
      lfnew = (LinearFunctionPtr) new LinearFunction(grad, varsBegin(),
                                                     varsEnd(), 1e-12);

      nlconst = f->eval(x, &err);
      assert(0 == err);

      ObjectiveType newobjtype;
      newobjtype = getObjective()->getObjectiveType();

      fnew = (FunctionPtr) new Function(lfnew, qfnew);

      ObjectivePtr newobj =
          (ObjectivePtr) new Objective(fnew, nlconst, newobjtype);

      delete obj_;
      obj_ = newobj;
      consModed_ = true;
    }
  }

  for (ConstraintConstIterator cit = consBegin(); cit != consEnd(); ++cit) {
    c = *cit;
    f = c->getFunction();

    if (!f) {
      continue;
    }
    if (Quadratic == f->getType()) {
      mult[c->getIndex()] = 1.0;
      memset(values, 0, nz * sizeof(UInt));
      hessian_->fillRowColValues(x, 0.0, mult, values, &err);
      assert(0 == err);
      qfnew = (QuadraticFunctionPtr) new QuadraticFunction(nz, values, irow,
                                                           jcol, vit0);
      mult[c->getIndex()] = 0.0;

      memset(grad, 0, getNumVars() * sizeof(double));
      f->evalGradient(x, grad, &err);
      assert(0 == err);
      if (std::all_of(grad, grad + getNumVars(),
                      [](double comp) { return fabs(comp) < 1e-8; })) {
        lfnew = 0;
      } else {
        lfnew = (LinearFunctionPtr) new LinearFunction(grad, varsBegin(),
                                                       varsEnd(), 1e-12);
      }
      nlconst = f->eval(x, &err);
      assert(0 == err);

      //refine
      fnew = (FunctionPtr) new Function(lfnew, qfnew);

      newCons.insert(std::make_pair(
          fnew, std::make_pair(c->getLb() - nlconst, c->getUb() - nlconst)));

      markDelete(c);
    }
  }

  for (std::map<FunctionPtr, std::pair<double, double>>::iterator it =
           newCons.begin();
       it != newCons.end(); ++it) {
    newConstraint(it->first, it->second.first, it->second.second);
  }

  delMarkedCons();
  setNativeDer();

  delete[] mult;
  delete[] values;
  delete[] irow;
  delete[] jcol;
  delete[] x;
  delete[] grad;
  newCons.clear();
}

void Problem::countConsTypes_()
{
  ConstraintIterator citer;
  ConstraintPtr cPtr;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  UInt linCons = 0, bilinCons = 0, multilinCons = 0, quadCons = 0,
       nonlinCons = 0;
  UInt consWithLin = 0, consWithBilin = 0, consWithMultilin = 0,
       consWithQuad = 0, consWithNonlin = 0;
  UInt linTerms = 0, quadTerms = 0, bilinTerms = 0, sqTerms = 0;

  for (citer = cons_.begin(); citer != cons_.end(); ++citer) {
    cPtr = *citer;
    switch (cPtr->getFunctionType()) {
    case Constant:  // TODO: for now consider it linear
    case Linear:
      linCons++;
      break;
    case Quadratic:
      quadCons++;
      break;
    default:
      nonlinCons++;
      break;
    }
    lf = cPtr->getLinearFunction();
    if (lf) {
      consWithLin++;
      linTerms += lf->getNumTerms();
    }
    qf = cPtr->getQuadraticFunction();
    if (qf) {
      if (qf->getNumBilTerms() > 0) {
        consWithBilin++;
        bilinTerms += qf->getNumBilTerms();
      }
      if (qf->getNumSqTerms() > 0) {
        sqTerms += qf->getNumSqTerms();
      }
      consWithQuad++;
      quadTerms += qf->getNumTerms();
    }
    if (cPtr->getNonlinearFunction()) {
      consWithNonlin++;
    }
  }

  size_->SOS1Cons = sos1_.size();
  size_->SOS2Cons = sos2_.size();
  size_->linCons = linCons;
  size_->bilinCons = bilinCons;
  size_->multilinCons = multilinCons;
  size_->quadCons = quadCons;
  size_->nonlinCons = nonlinCons;

  size_->consWithLin = consWithLin;
  size_->consWithBilin = consWithBilin;
  size_->consWithMultilin = consWithMultilin;
  size_->consWithQuad = consWithQuad;
  size_->consWithNonlin = consWithNonlin;

  size_->linTerms = linTerms;
  size_->bilinTerms = bilinTerms;
  size_->sqTerms = sqTerms;
  size_->quadTerms = quadTerms;
  return;
}

void Problem::countObjTypes_()
{
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  UInt linTerms = 0, quadTerms = 0;

  if (obj_) {
    size_->objType = obj_->getFunctionType();
    lf = obj_->getLinearFunction();
    if (lf) {
      linTerms += lf->getNumTerms();
    }
    qf = obj_->getQuadraticFunction();
    if (qf) {
      quadTerms += qf->getNumTerms();
    }
    if (obj_->getNonlinearFunction()) {
    }
  }

  size_->objLinTerms = linTerms;
  size_->objQuadTerms = quadTerms;
  return;
}

void Problem::countVarTypes_()
{
  VariableIterator viter;
  VariablePtr vPtr;
  UInt bins = 0, ints = 0, conts = 0, fixed = 0;

  for (viter = vars_.begin(); viter != vars_.end(); viter++) {
    vPtr = *viter;
    switch (vPtr->getType()) {
    case (Binary):
      ++bins;
      break;
    case (Integer):
      ++ints;
      break;
    case (Continuous):
      ++conts;
      break;
    default:
      break;
    }
    if (fabs(vPtr->getUb() - vPtr->getLb()) < 1e-9) {
      ++fixed;
    }
  }
  size_->bins = bins;
  size_->ints = ints;
  size_->conts = conts;
  size_->fixed = fixed;
  findVarFunTypes_();
  return;
}

void Problem::delMarkedCons()
{
  if (numDCons_ > 0) {
    ConstraintPtr c;
    UInt i;
    std::vector<ConstraintPtr> copycons;
    std::vector<ConstraintPtr> delcons;

    for (ConstraintIterator it = cons_.begin(); it != cons_.end(); ++it) {
      c = *it;
      if (c->getState() == DeletedCons) {
        delcons.push_back(c);
      } else {
        copycons.push_back(c);
      }
    }

    if (engine_) {
      engine_->removeCons(delcons);
    }

    for (ConstraintIterator it = delcons.begin(); it != delcons.end(); ++it) {
      c = *it;
      for (VarSet::iterator vit = c->getFunction()->varsBegin();
           vit != c->getFunction()->varsEnd(); ++vit) {
        (*vit)->outOfConstraint_(c);
      }
      delete c;
      c = 0;
    }

    i = 0;
    for (ConstraintIterator it = copycons.begin(); it != copycons.end();
         ++it) {
      (*it)->setIndex_(i);
      ++i;
    }

    cons_ = copycons;
    consModed_ = true;
    numDCons_ = 0;
  }
}

void Problem::delMarkedVars(bool keep)
{
  assert(engine_ == 0 ||
         (!"Cannot delete variables after loading problem to engine\n"));
  if (numDVars_ > 0) {
    VariablePtr v = 0;
    UInt i = 0;
    std::vector<VariablePtr> copyvars;
    for (VariableIterator it = vars_.begin(); it != vars_.end(); ++it) {
      v = *it;
      if (v->getState() == DeletedVar) {
        for (ConstrSet::const_iterator cit = v->consBegin();
             cit != v->consEnd(); ++cit) {
          (*cit)->delFixedVar_(v, v->getLb());
        }
        if (obj_) {
          obj_->delFixedVar_(v, v->getLb());
        }
        if (keep) {
          varsRem_.push_back(v);
        } else {
          delete v;
        }
        v = 0;
      } else {
        v->setIndex_(i);
        ++i;
        copyvars.push_back(v);
      }
    }
    vars_ = copyvars;

    varsModed_ = true;
    numDVars_ = 0;
  }
}

ProblemType Problem::findType()
{
  calculateSize();

  if (size_->cons == size_->linCons &&
      (Constant == size_->objType || Linear == size_->objType)) {
    return (size_->bins + size_->ints > 0) ? MILP : LP;
  } else if (size_->cons == size_->linCons && (Quadratic == size_->objType)) {
    return (size_->bins + size_->ints > 0) ? MIQP : QP;

  } else if (size_->cons ==
                 size_->linCons + size_->bilinCons + size_->quadCons &&
             (Quadratic == size_->objType || Linear == size_->objType ||
              Constant == size_->objType)) {
    return (size_->bins + size_->ints > 0) ? MIQCQP : QCQP;
  } else if (isPolyp_()) {
    return (size_->bins + size_->ints > 0) ? MIPOLYP : POLYP;
  } else {
    return (size_->bins + size_->ints > 0) ? MINLP : NLP;
  }
}

void Problem::findVarFunTypes_()
{
  FunctionType ftype;
  FunctionPtr of = (obj_) ? obj_->getFunction() : FunctionPtr();
  FunctionPtr f;
  VariablePtr v;
  NonlinearFunctionPtr nlf;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;

#if 0
  for (VariableConstIterator it=vars_.begin(); it!=vars_.end(); ++it) {
    ftype = Constant;
    v = *it;
    for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
      f = (*cit)->getFunction();
      if (f) {
       ftype = funcTypesAdd(ftype, f->getVarFunType(v));
      }
    }
    v->setFunType_(ftype);
  }

  if (of) {
    for (VarSet::iterator vit=of->varsBegin(); vit!=of->varsEnd(); ++vit) {
      v = *vit;
      ftype = funcTypesAdd(v->getFunType(), of->getVarFunType(v));
      v->setFunType_(ftype);
    }
  }
#else
  for (VariableConstIterator it = vars_.begin(); it != vars_.end(); ++it) {
    (*it)->setFunType_(Constant);
  }
  for (ConstraintConstIterator it = cons_.begin(); it != cons_.end(); ++it) {
    f = (*it)->getFunction();
    if (f) {
      lf = f->getLinearFunction();
      qf = f->getQuadraticFunction();
      nlf = f->getNonlinearFunction();
      if (lf) {
        for (VariableGroupConstIterator lit = lf->termsBegin();
             lit != lf->termsEnd(); ++lit) {
          v = lit->first;
          ftype = funcTypesAdd(v->getFunType(), Linear);
          v->setFunType_(ftype);
        }
      }
      if (qf) {
        VarCountConstMap* vmap = qf->getVarMap();
        for (VarCountConstMap::const_iterator lit = vmap->begin();
             lit != vmap->end(); ++lit) {
          v = lit->first;
          ftype = funcTypesAdd(v->getFunType(), Quadratic);
          v->setFunType_(ftype);
        }
      }
      if (nlf) {
        for (VariableSet::const_iterator lit = nlf->varsBegin();
             lit != nlf->varsEnd(); ++lit) {
          v = *lit;
          v->setFunType_(Nonlinear);
        }
      }
    }
  }
  if (of) {
    for (VarSet::iterator vit = of->varsBegin(); vit != of->varsEnd();
         ++vit) {
      v = *vit;
      ftype = funcTypesAdd(v->getFunType(), of->getVarFunType(v));
      v->setFunType_(ftype);
    }
  }

#endif
}

ConstraintPtr Problem::getConstraint(UInt index) const
{
  return cons_[index];
}

DoubleVector* Problem::getDebugSol() const
{
  return debugSol_;
}

HessianOfLagPtr Problem::getHessian() const
{
  return hessian_;
}

JacobianPtr Problem::getJacobian() const
{
  return jacobian_;
}

UInt Problem::getNumHessNnzs() const
{
  if (hessian_) {
    return hessian_->getNumNz();
  }
  return 0;
}

UInt Problem::getNumJacNnzs() const
{
  if (jacobian_) {
    return jacobian_->getNumNz();
  }
  return 0;
}

UInt Problem::getNumLinCons()
{
  return size_->linCons;
}

UInt Problem::getNumSOS1()
{
  return size_->SOS1Cons;
}

UInt Problem::getNumSOS2()
{
  return size_->SOS2Cons;
}

ObjectivePtr Problem::getObjective() const
{
  return obj_;
}

// evaluate the objective at a given x. x must be of dimension 'n', the
// number of variables in the problem
double Problem::getObjValue(const double* x, int* err) const
{
  if (obj_) {
    return obj_->eval(x, err);
  } else {
    return 0;
  }
}

ConstProblemSizePtr Problem::getSize() const
{
  return size_;
}

VariablePtr Problem::getVariable(UInt index) const
{
  return vars_[index];
}

bool Problem::hasNativeDer() const
{
  return nativeDer_;
}

bool Problem::isDebugSolFeas(double atol, double rtol)
{
  if (debugSol_) {
    double lb, ub;
    int err = 0;
    double act;
    double* x = 0;
    bool isfeas = true;
    int i;

    logger_->msgStream(LogNone) << std::setprecision(7);

    x = &(*debugSol_)[0];  // convert doublevector * to double array pointer
    i = 0;
    for (VariableConstIterator it = vars_.begin(); it != vars_.end(); ++it) {
      lb = (*it)->getLb();
      ub = (*it)->getUb();
      act = x[i];
      if ((act > ub + atol) && (act > ub + fabs(ub) * rtol)) {
        logger_->msgStream(LogDebug2)
            << me_ << "variable ub constraint " << (*it)->getName()
            << " violated by the debug sol."
            << " activity = " << act << " but ub = " << ub << std::endl;
        isfeas = false;
        break;
      }
      if ((act < lb - atol) && (act < lb - fabs(lb) * rtol)) {
        logger_->msgStream(LogDebug2)
            << me_ << "variable lb constraint " << (*it)->getName()
            << " violated by the debug sol."
            << " activity = " << act << " but lb = " << lb << std::endl;
        isfeas = false;
        break;
      }
      ++i;
    }

    for (ConstraintConstIterator it = cons_.begin(); it != cons_.end();
         ++it) {
      err = 0;
      act = (*it)->getActivity(x, &err);
      lb = (*it)->getLb();
      ub = (*it)->getUb();
      if (err) {
        logger_->msgStream(LogError) << me_ << "eval error for constraint "
                                     << (*it)->getName() << std::endl;
      } else if ((act > ub + atol) && (act > ub + fabs(ub) * rtol)) {
        logger_->msgStream(LogError)
            << me_ << "ub constraint " << (*it)->getName()
            << " violated by the debug sol."
            << " activity = " << act << " but ub = " << ub << std::endl;
        isfeas = false;
        break;
      } else if ((act < lb - atol) && (act < lb - fabs(lb) * rtol)) {
        logger_->msgStream(LogError)
            << me_ << "lb constraint " << (*it)->getName()
            << " violated by the debug sol."
            << " activity = " << act << " but lb = " << lb << std::endl;
        isfeas = false;
        break;
      }
    }

    act = obj_->eval(x, &err);


    if (isfeas) {
      logger_->msgStream(LogDebug1)
          << me_ << "debug solution is feasible" << std::endl;
    }
    return isfeas;
  }
  return false;
}

bool Problem::isLinear()
{
  if (size_) {
    if (size_->cons == size_->linCons &&
        (Constant == size_->objType || Linear == size_->objType)) {
      return true;
    }
  }
  return false;
}

bool Problem::isMarkedDel(ConstConstraintPtr con)
{
  return (con->getState() == DeletedCons);
}

bool Problem::isMarkedDel(ConstVariablePtr var)
{
  return (var->getState() == DeletedVar);
}

bool Problem::isPolyp_()
{
  // assume that we have already check for linear, quadratic ...
  FunctionPtr f;
  if (obj_ && obj_->getFunction() &&
      (obj_->getFunction()->getType() == Nonlinear ||
       obj_->getFunction()->getType() == OtherFunctionType)) {
    return false;
  }
  for (ConstraintIterator it = cons_.begin(); it != cons_.end(); ++it) {
    f = (*it)->getFunction();
    if (f &&
        (f->getType() == Nonlinear || f->getType() == OtherFunctionType)) {
      return false;
    }
  }
  return false;
}

bool Problem::isQP()
{
  if (size_) {
    if (isLinear()) {
      return false;
    } else if ((size_->linCons == size_->cons) &&
               (Constant == size_->objType || Linear == size_->objType ||
                Quadratic == size_->objType)) {
      return true;
    }
  }
  return false;
}

bool Problem::isQuadratic()
{
  if (size_) {
    if (isLinear()) {
      return false;
    } else if ((size_->linCons + size_->quadCons + size_->bilinCons ==
                size_->cons) &&
               (Constant == size_->objType || Linear == size_->objType ||
                Quadratic == size_->objType)) {
      return true;
    }
  }
  return false;
}

void Problem::markDelete(ConstraintPtr con)
{
  con->setState_(DeletedCons);
  ++numDCons_;
}

void Problem::markDelete(VariablePtr var)
{
  assert(engine_ == 0 ||
         (!"Cannot delete variables after loading problem to engine\n"));
  var->setState_(DeletedVar);
  ++numDVars_;
}

void Problem::negateObj()
{
  if (engine_) {
    engine_->negateObj();
  }
  if (obj_) {
    obj_->negate_();
  }
  if (hessian_) {
    hessian_->negateObj();
  }
}

VariablePtr Problem::newBinaryVariable()
{
  assert(engine_ == 0 ||
         ("Cannot add variables after loading problem to engine\n"));
  VariablePtr v;
  std::string name;
  std::stringstream name_stream;
  name_stream << "var" << vars_.size();
  name = name_stream.str();
  v = newVariable(0.0, 1.0, Binary, name);
  return v;
}

VariablePtr Problem::newBinaryVariable(std::string name)
{
  assert(engine_ == 0 ||
         (!"Cannot add variables after loading problem to engine\n"));
  VariablePtr v = newVariable(0.0, 1.0, Binary, name);
  return v;
}

ConstraintPtr Problem::newConstraint(FunctionPtr f, double lb, double ub,
                                     std::string name)
{
  ConstraintPtr c =
      (ConstraintPtr) new Constraint(nextCId_, cons_.size(), f, lb, ub, name);
  ++nextCId_;
  if (f) {
    for (VarSet::iterator vit = f->varsBegin(); vit != f->varsEnd(); ++vit) {
      (*vit)->inConstraint_(c);
    }
  }
  cons_.push_back(c);
  if (engine_ != 0) {
    engine_->addConstraint(c);
  }
  consModed_ = true;
  return c;
}

ConstraintPtr Problem::newConstraint(FunctionPtr funPtr, double lb, double ub)
{
  // set a name and call newConstraint above.
  std::stringstream name_stream;
  std::string name;
  ConstraintPtr c;

  // build a name
  name_stream << "cons" << cons_.size();
  name = name_stream.str();

  // make a constraint,
  c = (ConstraintPtr)newConstraint(funPtr, lb, ub, name);
  if (engine_ != 0) {
    engine_->addConstraint(c);
  }
  consModed_ = true;

  return c;
}

ObjectivePtr Problem::newObjective(FunctionPtr f, double cb,
                                   ObjectiveType otyp)
{
  assert(engine_ == 0 ||
         (!"Cannot add objective after loading problem to engine\n"));
  // XXX: set name.
  std::string name;
  std::stringstream name_stream;
  ObjectivePtr o;
  name_stream << "obj";

  name = name_stream.str();
  o = newObjective(f, cb, otyp, name);
  return o;
}

ObjectivePtr Problem::newObjective(double cb, ObjectiveType otyp)
{
  assert(engine_ == 0 ||
         (!"Cannot add objective after loading problem to engine\n"));
  if (obj_) {
    delete obj_;
    obj_ = 0;
  }
  obj_ = new Objective(cb, otyp);
  consModed_ = true;
  return obj_;
}

ObjectivePtr Problem::newObjective(FunctionPtr f, double cb,
                                   ObjectiveType otyp, std::string name)
{
  assert(engine_ == 0 ||
         (!"Cannot add objective after loading problem to engine\n"));

  if (obj_) {
    delete obj_;
    obj_ = 0;
  }
  obj_ = new Objective(f, cb, otyp, name);
  consModed_ = true;
  return obj_;
}

SOSPtr Problem::newSOS(int n, SOSType type, const double* weights,
                       const VarVector& vars, int priority, std::string name)
{
  SOSPtr sos = new SOS(n, type, weights, vars, nextSId_, priority, name);
  ++nextSId_;
  if (SOS1 == type) {
    sos1_.push_back(sos);
  } else if (SOS2 == type) {
    sos2_.push_back(sos);
  }
  return sos;
}

SOSPtr Problem::newSOS(int n, SOSType type, const double* weights,
                       const VarVector& vars, int priority)
{
  std::string name;
  std::stringstream name_stream;
  if (SOS1 == type) {
    name_stream << "sos1_" << (sos1_.size() + sos2_.size());
  } else if (SOS2 == type) {
    name_stream << "sos2_" << (sos1_.size() + sos2_.size());
  } else {
    name_stream << "sos_" << (sos1_.size() + sos2_.size());
  }
  name = name_stream.str();
  SOSPtr sos = newSOS(n, type, weights, vars, priority, name);
  return sos;
}

VariablePtr Problem::newVariable(VarSrcType stype)
{
  assert(engine_ == 0 ||
         (!"Cannot add variables after loading problem to engine\n"));
  VariablePtr v;
  std::string name;
  std::stringstream name_stream;
  name_stream << "var" << vars_.size();
  name = name_stream.str();
  v = newVariable(-INFINITY, INFINITY, Continuous, name, stype);
  return v;
}

VariablePtr Problem::newVariable(double lb, double ub, VariableType vtype,
                                 VarSrcType stype)
{
  assert(engine_ == 0 ||
         (!"Cannot add variables after loading problem to engine\n"));
  VariablePtr v;
  std::string name;
  std::stringstream name_stream;
  name_stream << "var" << vars_.size();
  name = name_stream.str();
  v = newVariable(lb, ub, vtype, name, stype);
  return v;
}

VariablePtr Problem::newVariable(double lb, double ub, VariableType vtype,
                                 std::string name, VarSrcType stype)
{
  assert(engine_ == 0 ||
         (!"Cannot add variables after loading problem to engine\n"));
  VariablePtr v = new Variable(nextVId_, vars_.size(), lb, ub, vtype, name);
  v->setSrcType(stype);
  ++nextVId_;
  vars_.push_back(v);
  varsModed_ = true;
  return v;
}

void Problem::newVariables(VariableConstIterator v_begin,
                           VariableConstIterator v_end, VarSrcType stype)
{
  assert(engine_ == 0 ||
         (!"Cannot add variables after loading problem to engine\n"));
  VariableConstIterator v_iter;

  for (v_iter = v_begin; v_iter != v_end; v_iter++) {
    newVariable((*v_iter)->getLb(), (*v_iter)->getUb(), (*v_iter)->getType(),
                (*v_iter)->getName(), stype);
  }
}

void Problem::objToCons()
{
  std::string name = "eta";
  if (!obj_) {
    assert(!"No objective function in the problem!");
  } else if (obj_->getFunctionType() != Linear &&
             obj_->getFunctionType() != Constant) {
    FunctionPtr fold = 0, fnew = 0;
    double objCons = obj_->getConstant();
    ObjectiveType objType = obj_->getObjectiveType();
    LinearFunctionPtr lf = obj_->getLinearFunction();
    const QuadraticFunctionPtr qf = obj_->getQuadraticFunction();
    const NonlinearFunctionPtr nlf = obj_->getNonlinearFunction();
    LinearFunctionPtr lfnew = (LinearFunctionPtr) new LinearFunction();
    VariablePtr vPtr = newVariable(-INFINITY, INFINITY, Continuous, name);
    lfnew->addTerm(vPtr, -1.0);
    if (nlf != NULL && qf != NULL) {
      fold = (FunctionPtr) new Function(lfnew, qf, nlf);
    } else if (nlf) {
      fold = (FunctionPtr) new Function(lfnew, nlf);
    } else if (qf) {
      fold = (FunctionPtr) new Function(lfnew, qf);
    }
    name = "objToCons";
    newConstraint(fold, -INFINITY, 0.0, name);
    if (!lf) {
      lf = (LinearFunctionPtr) new LinearFunction();
    }
    lf->addTerm(vPtr, 1.0);
    removeObjective();
    fnew = (FunctionPtr) new Function(lf);
    newObjective(fnew, objCons, objType);
  }
  return;
}

void Problem::prepareForSolve()
{
  bool reload = false;

  if (consModed_ || varsModed_) {
    reload = true;
  }
  calculateSize();
  if (nativeDer_ && (true == reload || !hessian_)) {
    setNativeDer();
  }
}

void Problem::removeObjective()
{
  assert(engine_ == 0 ||
         (!"Cannot change objective after loading problem to engine\n"));
  if (obj_) {
    delete obj_;
    obj_ = 0;
  }
  return;
}

QuadraticFunctionPtr Problem::removeQuadFromObj()
{
  assert(engine_ == 0 ||
         (!"Cannot change objective after loading problem to engine\n"));
  if (obj_) {
    return obj_->removeQuadratic_();
  }
  consModed_ = true;
  return QuadraticFunctionPtr();  // NULL
}

NonlinearFunctionPtr Problem::removeNonlinFromObj()
{
  assert(engine_ == 0 ||
         (!"Cannot change objective after loading problem to engine\n"));
  if (obj_) {
    return obj_->removeNonlinear_();
  }
  consModed_ = true;
  return NonlinearFunctionPtr();  // NULL
}

void Problem::resetDer()
{
  jacobian_ = JacobianPtr();     // NULL.
  hessian_ = HessianOfLagPtr();  // NULL.
}

void Problem::resConTypCnt()
{
  size_->countAggregation = 0;
  size_->countPrecedence = 0;
  size_->countVariableBound = 0;
  size_->countSetPartitioning = 0;
  size_->countSetPacking = 0;
  size_->countSetCovering = 0;
  size_->countCardinality = 0;
  size_->countInvariantKnapsack = 0;
  size_->countEquationKnapsack = 0;
  size_->countBinPacking = 0;
  size_->countKnapsack = 0;
  size_->countIntegerKnapsack = 0;
  size_->countMixedBinary = 0;
  size_->countNoSpecificStructure = 0;
  size_->countDiagQuad = 0;
  size_->countSimpleBall = 0;
  size_->countEllipsoid = 0;
  size_->countComplementSimpleBall = 0;
  size_->countComplementEllipsoid = 0;
  size_->countPureBilinear = 0;
  size_->countQuadConvertSOC = 0;
}

void Problem::reverseSense(ConstraintPtr cons)
{
  cons->reverseSense_();
  consModed_ = true;
}

void Problem::setDebugSol(const DoubleVector& x)
{
  if (debugSol_) {
    delete debugSol_;
  }
  debugSol_ = new DoubleVector(x);
}

void Problem::setEngine(Engine* engine)
{
  if (engine_ && engine_ != engine) {
    engine_->clear();
  }
  engine_ = engine;
}

void Problem::setHessian(HessianOfLagPtr hessian)
{
  hessian_ = hessian;
}

void Problem::setIndex_(VariablePtr v, UInt i)
{
  v->setIndex_(i);
}

void Problem::setInitialPoint(const double* x)
{
  const double* xp = x;
  if (!x || vars_.size() == 0) {
    return;
  }

  for (VariableIterator viter = vars_.begin(); viter != vars_.end();
       ++viter, ++xp) {
    (*viter)->setInitVal_(*xp);
  }
}

void Problem::setInitialPoint(const double* x, size_t k)
{
  const double* xp = x;
  VariableIterator viter = vars_.begin();

  if (!x || vars_.size() == 0) {
    return;
  }

  for (size_t i = 0; i < k; ++viter, ++xp, ++i) {
    (*viter)->setInitVal_(*xp);
  }
}

void Problem::setInitVal(VariablePtr v, double val)
{
  v->setInitVal_(val);
}

void Problem::setInitValByInd(UInt ind, double val)
{
  vars_[ind]->setInitVal_(val);
}

void Problem::setJacobian(JacobianPtr jacobian)
{
  jacobian_ = jacobian;
}

void Problem::setNativeDer()
{
  calculateSize();
  nativeDer_ = true;
  if (jacobian_) {
    delete jacobian_;
    jacobian_ = 0;
  }
  if (hessian_) {
    delete hessian_;
    hessian_ = 0;
  }
  jacobian_ = (JacobianPtr) new Jacobian(cons_, vars_.size());
  hessian_ = (HessianOfLagPtr) new HessianOfLag(this);
}

void Problem::setVarType(VariablePtr var, VariableType type)
{
  assert(
      var == vars_[var->getIndex()] ||
      !"Problem: Type of variable that is not in problem can't be changed.");

  if (size_) {
    switch (var->getType()) {
    case (Binary):
    case (ImplBin):
      --(size_->bins);
      break;
    case (Integer):
    case (ImplInt):
      --(size_->ints);
      break;
    default:
      --(size_->conts);
    }

    switch (type) {
    case (Binary):
    case (ImplBin):
      ++(size_->bins);
      break;
    case (Integer):
    case (ImplInt):
      ++(size_->ints);
      break;
    default:
      ++(size_->conts);
    }
  }
  var->setType_(type);
  varsModed_ = true;
}

void Problem::subst(VariablePtr out, VariablePtr in, double rat)
{
  bool stayin;
  assert(engine_ == 0 ||
         (!"Cannot substitute variables after loading problem to engine\n"));
  ConstrQ q;

  for (ConstrSet::const_iterator it = out->consBegin(); it != out->consEnd();
       ++it) {
    (*it)->subst_(out, in, rat, &stayin);
    if (stayin) {
      in->inConstraint_(*it);
    } else {
      in->outOfConstraint_(*it);
    }
    q.push_back(*it);
  }

  for (ConstrQ::iterator it = q.begin(); it != q.end(); ++it) {
    out->outOfConstraint_(*it);
  }
  q.clear();

  obj_->subst_(out, in, rat);
  consModed_ = varsModed_ = true;
}

void Problem::unsetEngine()
{
  engine_ = 0;
}

void Problem::write(std::ostream& out, std::streamsize out_p) const
{
  ConstraintConstIterator citer;
  VariableConstIterator viter;
  std::streamsize old_p = out.precision();

  out.precision(out_p);
  if (size_) {
    //writeSize(out);
  }

  for (viter = vars_.begin(); viter != vars_.end(); ++viter) {
    (*viter)->write(out);
  }

  if (obj_) {
    obj_->write(out);
    out << std::endl;
  }

  for (citer = cons_.begin(); citer != cons_.end(); ++citer) {
    (*citer)->write(out);
    out << std::endl;
  }
  out.precision(old_p);

  //for (citer = cons_.begin(); citer != cons_.end(); ++citer) {
  //  (*citer)->displayFunctionMap();
  //}
}

double Problem::getSizeEstimate()
{
  double estimate = 0, lb, ub;
  VariableConstIterator viter;

  for (viter = vars_.begin(); viter != vars_.end(); ++viter) {
    lb = (*viter)->getLb();
    ub = (*viter)->getUb();
    if (lb == -INFINITY || ub == INFINITY) {
      return -1;
    } else {
      estimate += pow(ub - lb, 2);
    }
  }
  return pow(estimate, 0.5);
}

void Problem::statistics(std::ostream& out) const{
  std::cout << "No of variables: " << size_->vars
  << ", integer variables: " << size_->ints
  << ", binary variables: " << size_->bins
  << ", constraints: " << size_->cons << std::endl;
}

void Problem::writeSize(std::ostream& out) const
{
  const int wid = 8;

  out << "--------------------------------------------------" << std::endl;
  out << "|Model Statistics                                |" << std::endl;
  out << "|------------------------------------------------|" << std::endl;
  out << "| variables                             " << std::setw(wid)
      << size_->vars << " |" << std::endl;
  out << "| binary variables                      " << std::setw(wid)
      << size_->bins << " |" << std::endl;
  out << "| general integer variables             " << std::setw(wid)
      << size_->ints << " |" << std::endl;
  out << "| continuous variables                  " << std::setw(wid)
      << size_->conts << " |" << std::endl;
  out << "| fixed variables                       " << std::setw(wid)
      << size_->fixed << " |" << std::endl;
  out << "| constraints                           " << std::setw(wid)
      << size_->cons << " |" << std::endl;
  out << "| linear constraints                    " << std::setw(wid)
      << size_->linCons << " |" << std::endl;
  out << "| SOS1 constraints                      " << std::setw(wid)
      << size_->SOS1Cons << " |" << std::endl;
  out << "| SOS2 constraints                      " << std::setw(wid)
      << size_->SOS2Cons << " |" << std::endl;
  out << "| quadratic constraints                 " << std::setw(wid)
      << size_->quadCons << " |" << std::endl;
  out << "| nonlinear constraints                 " << std::setw(wid)
      << size_->nonlinCons << " |" << std::endl;
  out << "| constraints with linear terms         " << std::setw(wid)
      << size_->consWithLin << " |" << std::endl;
  out << "| constraints with bilinear terms       " << std::setw(wid)
      << size_->consWithBilin << " |" << std::endl;
  out << "| constraints with quadratic terms      " << std::setw(wid)
      << size_->consWithQuad << " |" << std::endl;
  out << "| linear terms in constraints           " << std::setw(wid)
      << size_->linTerms << " |" << std::endl;
  out << "| bilinear terms in constraints         " << std::setw(wid)
      << size_->bilinTerms << " |" << std::endl;
  out << "| square terms in constraints           " << std::setw(wid)
      << size_->sqTerms << " |" << std::endl;
  out << "| quadratic terms in constraints        " << std::setw(wid)
      << size_->quadTerms << " |" << std::endl;
  out << "| objectives                            " << std::setw(wid)
      << size_->objs << " |" << std::endl;
  out << "| linear terms in objective             " << std::setw(wid)
      << size_->objLinTerms << " |" << std::endl;
  out << "| quadratic terms in objective          " << std::setw(wid)
      << size_->objQuadTerms << " |" << std::endl;
  out << "| Type of objective:                  " << std::setw(10)
      << getFunctionTypeString(size_->objType) << " |" << std::endl;
  out << "--------------------------------------------------" << std::endl;
}
