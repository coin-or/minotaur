//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file SimplexQuadCutGen.h
 * \brief Define class for generating cuts for quadratic constraints from
 * the simplex tableau
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 */

#include "SimplexQuadCutGen.h"

#include <string.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iterator>

#include "Constraint.h"
#include "Cut.h"
#include "Environment.h"
#include "Function.h"
#include "LPEngine.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "Operations.h"
#include "Option.h"
#include "Problem.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Timer.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

const std::string SimplexQuadCutGen::me_ = "SimplexQuadCutGen: ";

SimplexQuadCutGen::SimplexQuadCutGen() { }

SimplexQuadCutGen::SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe,
                                     double ub)
  : env_(env),
    eTol_(1e-8),
    p_(p),
    lpe_(lpe),
    tabInfo_(0),
    ub_(ub)
{
  bounds_.clear();
  nrounds_ = 3;
  minChangeFrac_ = 0.1;
  ncuts_ = 0;
  maxCuts_ = std::min(static_cast<UInt>(std::max(ceil(0.2 * p->getNumVars()),
                                                 ceil(0.05 * p->getNumCons()))),
                      static_cast<UInt>(20));
  maxrounds_ = 25;
  minDepth_ = 1e-3;
  basicInd_.clear();
  nbOrig_.clear();
  nbSlack_.clear();
  sb_.clear();
  timer_ = env->getTimer();
  stats_.gencuts = 0;
  stats_.cutsadded = 0;
  stats_.numrounds = 0;
  stats_.time = 0.0;
}

SimplexQuadCutGen::~SimplexQuadCutGen()
{
  if(tabInfo_) {
    delete tabInfo_;
  }
  basicInd_.clear();
  nbOrig_.clear();
  nbSlack_.clear();
  sb_.clear();
}

bool SimplexQuadCutGen::doCutting(double curlb)
{
  double oldgap;
  double newgap;

  if(stats_.numrounds <= 25) {
    if(stats_.numrounds > nrounds_) {
      if(ub_ < INFINITY) {
        oldgap = ub_ - bounds_[0];
        newgap = ub_ - curlb;
        if((oldgap - newgap) / ub_ >= minChangeFrac_) {
          bounds_.erase(bounds_.begin());
          bounds_.push_back(curlb);
          return true;
        }
      } else {
        if((curlb - bounds_[0]) / bounds_[0] >= minChangeFrac_) {
          bounds_.erase(bounds_.begin());
          bounds_.push_back(curlb);
          return true;
        }
      }
    } else {
      bounds_.push_back(curlb);
      return true;
    }
  }
  return false;
}

int SimplexQuadCutGen::generateCuts(RelaxationPtr rel, ConstSolutionPtr sol)
{
  SimplexCutVector cuts;
  ConstraintPtr c;
  FunctionType ftype;
  double act;
  int error;
  UInt iter_cuts = 0;
  double stime = timer_->query();
  double iter_time;

  if(doCutting(sol->getObjValue())) {
    ++stats_.numrounds;
    preprocessSimplexTab();
    for(ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
        ++cit) {
      c = *cit;
      ftype = c->getFunction()->getType();
      if(ftype != Quadratic && ftype != Bilinear) {
        // Linear constraints should be satisfied thus no cuts will be generated
        continue;
      }
      act = c->getActivity(sol->getPrimal(), &error);
      if(error == 0) {
        if(act <= c->getUb() + eTol_ && act >= c->getLb() - eTol_) {
          // Constraint already satisfied
          continue;
        }
      } else {
#if SPEW
        env_->getLogger()->msgStream(LogDebug)
            << me_ << "Constraint " << c->getName()
            << " is not defined at this point." << std::endl;
#endif
      }
      env_->getLogger()->msgStream(LogDebug)
          << me_ << " : Generating cuts for the constraint " << c->getName()
          << std::endl;
      relaxConsBNB_(cuts, c, sol->getPrimal(), rel);
    }
    iter_cuts = cuts.size();
    disableFactorization();
    stats_.gencuts += iter_cuts;
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "No. of cuts generated: " << iter_cuts << std::endl;
    addCutsToRel_(cuts, rel, sol->getPrimal(), iter_cuts);
    stats_.cutsadded += iter_cuts;
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "No. of cuts added: " << iter_cuts << std::endl;
    iter_time = timer_->query() - stime;
    stats_.time += iter_time;
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "Time taken in cut generation: " << iter_time << std::endl;
  }
  return iter_cuts;
}

void SimplexQuadCutGen::preprocessSimplexTab()
{
  lpe_->enableFactorization();
  getBasicInfo_();
  sortVariables_();
  getSlackBounds_();
}

void SimplexQuadCutGen::disableFactorization()
{
  lpe_->disableFactorization();
}

void SimplexQuadCutGen::addCutsToRel_(SimplexCutVector cuts, RelaxationPtr rel,
                                      const double* x, UInt& ncuts)
{
  SimplexCutPtr cut;
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr c;
  double minelem;

  ncuts = 0;

  calcDepth_(cuts, x);
  std::sort(cuts.begin(), cuts.end());

  for(SimplexCutVector::iterator it = cuts.begin();
      it != cuts.end() && ncuts < maxCuts_; ++it) {
    cut = (*it);
    // both lower and upper bounds must not be finite
    assert(!(cut->lb > -INFINITY && cut->ub < INFINITY));
    if(cut->depth < minDepth_) {
#if SPEW
      env_->getLogger()->msgStream(LogDebug)
          << me_ << " Cut not added since the depth of cut is very small"
          << std::endl;
#endif
      delete cut;
      continue;
    }
    lf = (LinearFunctionPtr) new LinearFunction(cut->coef, rel->varsBegin(),
                                                rel->varsEnd(), 1e-9);
    env_->getLogger()->msgStream(LogDebug)
        << me_ << "Depth of cut = " << std::fixed << std::setprecision(6)
        << cut->depth << std::endl;
    if(cut->lb > -INFINITY) {
      if(fabs(cut->lb) > 1e-3) {
        minelem = getMin_(cut, fabs(cut->lb));
        lf->multiply(1 / minelem);
        cut->lb = cut->lb / minelem;
      }
    } else {
      if(fabs(cut->ub) > 1e-3) {
        minelem = getMin_(cut, fabs(cut->ub));
        lf->multiply(1 / minelem);
        cut->ub = cut->ub / minelem;
      }
    }
    f = (FunctionPtr) new Function(lf);
    c = rel->newConstraint(f, cut->lb, cut->ub);
    ++ncuts;
#if SPEW
    env_->getLogger()->msgStream(LogDebug)
        << me_ << " added new cut. Depth of cut = " << cut->depth << std::endl;
    c->write(env_->getLogger()->msgStream(LogDebug));
#endif
    delete cut;
  }
}

void SimplexQuadCutGen::delTabInfo_()
{
  delete[] tabInfo_->colLower;
  delete[] tabInfo_->colUpper;
  delete[] tabInfo_->rowLower;
  delete[] tabInfo_->rowUpper;
  delete[] tabInfo_->rowRhs;
  delete[] tabInfo_->origTab;
  delete[] tabInfo_->rowStart;
  delete[] tabInfo_->indices;
  delete[] tabInfo_->rowLen;
}

void SimplexQuadCutGen::calcDepth_(SimplexCutVector cuts, const double* x)
{
  double norm = 0;
  SimplexCutPtr cut;

  for(SimplexCutVector::iterator vit = cuts.begin(); vit != cuts.end(); ++vit) {
    cut = (*vit);
    norm = sqrt(InnerProduct(cut->coef, cut->coef, tabInfo_->ncol));

    if(cut->lb > -INFINITY) {
      cut->depth =
          (cut->lb - InnerProduct(x, cut->coef, tabInfo_->ncol)) / norm;
    } else {
      cut->depth =
          (InnerProduct(x, cut->coef, tabInfo_->ncol) - cut->ub) / norm;
    }
  }
}

void SimplexQuadCutGen::getBasicInfo_()
{
  if(!lpe_->IsOptimalBasisAvailable()) {
    env_->getLogger()->msgStream(LogDebug)
        << me_ << "Solver provided no Optimal Basis" << std::endl;
  }

  tabInfo_ = (TableauInfo*)new TableauInfo();
  tabInfo_->ncol = lpe_->getNumCols();
  tabInfo_->nrow = lpe_->getNumRows();
  tabInfo_->colLower = lpe_->getColLower();
  tabInfo_->colUpper = lpe_->getColUpper();
  tabInfo_->rowLower = lpe_->getRowLower();
  tabInfo_->rowUpper = lpe_->getRowUpper();
  tabInfo_->rowRhs = lpe_->getRightHandSide();
  tabInfo_->origTab = lpe_->getOriginalTableau();
  tabInfo_->rowStart = lpe_->getRowStarts();
  tabInfo_->indices = lpe_->getIndicesofVars();
  tabInfo_->rowLen = lpe_->getRowLength();
}

double SimplexQuadCutGen::getMin_(SimplexCutPtr cut, double rhs)
{
  double minelem = INFINITY;
  for(int i = 0; i < tabInfo_->ncol; ++i) {
    if(fabs(cut->coef[i]) > eTol_ && fabs(cut->coef[i]) < minelem) {
      minelem = fabs(cut->coef[i]);
    }
  }
  if(rhs < minelem) {
    minelem = rhs;
  }
  return minelem;
}

void SimplexQuadCutGen::getSlackBounds_()
{
  double lb, ub, rhs;
  int row, upto;

  sb_.clear();
  for(int i = 0; i < nnbSlack_; ++i) {
    row = nbSlack_[i];
    lb = 0.0;
    ub = 0.0;
    upto = tabInfo_->rowStart[row] + tabInfo_->rowLen[row];
    for(int j = tabInfo_->rowStart[row]; j < upto; ++j) {
      getTermBounds_(tabInfo_->origTab[j],
                     tabInfo_->colLower[tabInfo_->indices[j]],
                     tabInfo_->colUpper[tabInfo_->indices[j]], lb, ub);
    }

    if(lb < tabInfo_->rowLower[row] - eTol_) {
      lb = tabInfo_->rowLower[row];
    }
    if(ub > tabInfo_->rowUpper[row] + eTol_) {
      ub = tabInfo_->rowUpper[row];
    }

    rhs = tabInfo_->rowRhs[row];
    sb_.insert(std::make_pair(row, std::make_pair(rhs - ub, rhs - lb)));
  }
}

void SimplexQuadCutGen::getTermBounds_(double coef, double vlb, double vub,
                                       double& lb, double& ub)
{
  if(fabs(coef) < eTol_) {
    // Coef is zero
    return;
  }

  if(coef > eTol_) {
    lb += coef * vlb;
    ub += coef * vub;
  } else {
    lb += coef * vub;
    ub += coef * vlb;
  }
}

void SimplexQuadCutGen::substituteAndRelax_(RelaxationPtr rel, const double* x,
                                            VariablePtr bkeep,
                                            VariablePtr bsubst, double beta,
                                            double* coefs, double& cutConst,
                                            bool under)
{
  double* origRow = new double[tabInfo_->ncol];
  double* slackRow = new double[tabInfo_->nrow];
  std::map<int, double> row;
  double rhs = 0.0;
  VariablePtr nb;
  bool lower;
  double elem;
  double cb = 0., cn = 0., cnst = 0.;

  lpe_->getBInvARow(basicInd_[bsubst->getIndex()], origRow, slackRow);
  for(int i = 0; i < nnbOrig_; ++i) {
    elem = origRow[nbOrig_[i]];
    if(fabs(elem) > eTol_) {
      beta += elem * x[nbOrig_[i]];
      nb = rel->getVariable(nbOrig_[i]);
      lower = fabs(x[nb->getIndex()] - nb->getLb()) < eTol_ ? true : false;
      relaxTermBNB_(elem, lower, bkeep->getLb(), bkeep->getUb(), nb->getLb(),
                    nb->getUb(), cb, cn, cnst, under);
      coefs[bkeep->getIndex()] += cb;
      coefs[nbOrig_[i]] += cn;
      cutConst += cnst;
    }
  }

  for(int i = 0; i < nnbSlack_; ++i) {
    elem = slackRow[nbSlack_[i]];
    if(fabs(elem) > eTol_) {
      lower = sb_[nbSlack_[i]].second > eTol_ ? true : false;
      relaxTermBNB_(elem, lower, bkeep->getLb(), bkeep->getUb(),
                    sb_[nbSlack_[i]].first, sb_[nbSlack_[i]].second, cb, cn,
                    cnst, under);
      coefs[bkeep->getIndex()] += cb;
      slackSubstitute_(nbSlack_[i], cn, row, rhs);
      for(std::map<int, double>::iterator itrow = row.begin();
          itrow != row.end(); ++itrow) {
        coefs[itrow->first] -= itrow->second;
      }
      cutConst += cnst + rhs;
    }
  }

  coefs[bkeep->getIndex()] -= beta;

  delete[] origRow;
  delete[] slackRow;
  row.clear();
}

void SimplexQuadCutGen::relaxConsBNB_(SimplexCutVector& cuts, ConstraintPtr c,
                                      const double* x, RelaxationPtr rel)
{
  // Assume that the constraint is of the form y = x1x2 or y = x^2
  LinearFunctionPtr lf = c->getFunction()->getLinearFunction();
  QuadraticFunctionPtr qf = c->getFunction()->getQuadraticFunction();
  VariablePtr v1, v2, y;
  SimplexCutPtr cut;
  double x1val, x2val, yval, cutConst;
  double viotol = 0.1;
  double maxvio;

  assert(lf->getNumTerms() == 1);
  assert(qf->getNumTerms() == 1);
  v1 = rel->getRelaxationVar(qf->begin()->first.first);
  v2 = rel->getRelaxationVar(qf->begin()->first.second);
  y = rel->getRelaxationVar(lf->termsBegin()->first);

  if(v1->getIndex() == v2->getIndex()) {
    x1val = x[v1->getIndex()];
    yval = x[y->getIndex()];
    if(yval < x1val * x1val - eTol_) {
      return;
    }
    // Violated constraint y - x1^2 <= 0
    maxvio = pow((v1->getUb() - v1->getLb()) / 2, 2);
    if(yval - x1val * x1val < viotol * maxvio) {
#if SPEW
      env_->getLogger()->msgStream(LogDebug)
          << me_ << "Constraint " << c->getName()
          << " has insufficient violation. No cuts generated" << std::endl;
#endif
      return;
    }
    cut = (SimplexCutPtr) new SimplexCut();
    cutConst = 0;
    cut->coef = new double[rel->getNumVars()];
    memset(cut->coef, 0, rel->getNumVars() * sizeof(double));
    cut->coef[y->getIndex()] += 1.0;
    substituteAndRelax_(rel, x, v1, v1, x1val, cut->coef, cutConst, true);
    cut->lb = -INFINITY;
    cut->ub = -cutConst;
    cuts.push_back(cut);
  } else {
    x1val = x[v1->getIndex()];
    x2val = x[v2->getIndex()];
    yval = x[y->getIndex()];
    maxvio = ((v1->getUb() - v1->getLb()) * (v2->getUb() - v2->getLb())) / 4;
    if(fabs(yval - x1val * x2val) < viotol * maxvio) {
#if SPEW
      env_->getLogger()->msgStream(LogDebug)
          << me_ << "Constraint " << c->getName()
          << " has insufficient violation. No cuts generated" << std::endl;
#endif
      return;
    }
    if(yval < x1val * x2val - eTol_) {
      // Violated constraint y - x1x2 >= 0
      // v1 is kept basic and v2 is substituted
      cut = (SimplexCutPtr) new SimplexCut();
      cutConst = 0;
      cut->coef = new double[rel->getNumVars()];
      memset(cut->coef, 0, rel->getNumVars() * sizeof(double));
      cut->coef[y->getIndex()] += 1.0;
      substituteAndRelax_(rel, x, v1, v2, x2val, cut->coef, cutConst, false);
      cut->lb = -cutConst;
      cut->ub = INFINITY;
      cuts.push_back(cut);

      // v2 is kept basic and v1 is substituted
      cut = (SimplexCutPtr) new SimplexCut();
      cutConst = 0;
      cut->coef = new double[rel->getNumVars()];
      memset(cut->coef, 0, rel->getNumVars() * sizeof(double));
      cut->coef[y->getIndex()] += 1.0;
      substituteAndRelax_(rel, x, v2, v1, x1val, cut->coef, cutConst, false);
      cut->lb = -cutConst;
      cut->ub = INFINITY;
      cuts.push_back(cut);
    } else {
      // Violated constraint y - x1x2 <= 0
      // v1 is kept basic and v2 is substituted
      cut = (SimplexCutPtr) new SimplexCut();
      cutConst = 0;
      cut->coef = new double[rel->getNumVars()];
      memset(cut->coef, 0, rel->getNumVars() * sizeof(double));
      cut->coef[y->getIndex()] += 1.0;
      substituteAndRelax_(rel, x, v1, v2, x2val, cut->coef, cutConst, true);
      cut->lb = -INFINITY;
      cut->ub = -cutConst;
      cuts.push_back(cut);

      // v2 is kept basic and v1 is substituted
      cut = (SimplexCutPtr) new SimplexCut();
      cutConst = 0;
      cut->coef = new double[rel->getNumVars()];
      memset(cut->coef, 0, rel->getNumVars() * sizeof(double));
      cut->coef[y->getIndex()] += 1.0;
      substituteAndRelax_(rel, x, v2, v1, x1val, cut->coef, cutConst, true);
      cut->lb = -INFINITY;
      cut->ub = -cutConst;
      cuts.push_back(cut);
    }
  }
}

int SimplexQuadCutGen::relaxTermBNB_(double coef, bool lower, double bl,
                                     double bu, double nl, double nu,
                                     double& cb, double& cn, double& cnst,
                                     bool under)
{
  if(under) {
    if(coef > eTol_) {
      if(lower) {
        cb = coef * nl;
        cn = coef * bl;
        cnst = -coef * bl * nl;
      } else {
        cb = coef * nu;
        cn = coef * bu;
        cnst = -coef * bu * nu;
      }
    } else {
      if(lower) {
        cb = coef * nl;
        cn = coef * bu;
        cnst = -coef * bu * nl;
      } else {
        cb = coef * nu;
        cn = coef * bl;
        cnst = -coef * bl * nu;
      }
    }
  } else {
    if(coef < eTol_) {
      if(lower) {
        cb = coef * nl;
        cn = coef * bl;
        cnst = -coef * bl * nl;
      } else {
        cb = coef * nu;
        cn = coef * bu;
        cnst = -coef * bu * nu;
      }
    } else {
      if(lower) {
        cb = coef * nl;
        cn = coef * bu;
        cnst = -coef * bu * nl;
      } else {
        cb = coef * nu;
        cn = coef * bl;
        cnst = -coef * bl * nu;
      }
    }
  }
  return 1;
}

void SimplexQuadCutGen::slackSubstitute_(int slackInd, double coef,
                                         std::map<int, double>& row,
                                         double& rhs)
{
  // Get the row from tabInfo_
  int upto = tabInfo_->rowStart[slackInd] + tabInfo_->rowLen[slackInd];
  row.clear();
  for(int j = tabInfo_->rowStart[slackInd]; j < upto; ++j) {
    row.insert(
        std::make_pair(tabInfo_->indices[j], coef * tabInfo_->origTab[j]));
  }
  rhs = coef * tabInfo_->rowRhs[slackInd];
}

void SimplexQuadCutGen::sortVariables_()
{
  int* basicVars = new int[tabInfo_->nrow];
  std::vector<int> nonbasicslack(tabInfo_->nrow, 0);

  lpe_->getBasics(basicVars);
  basicInd_.clear();

  for(int i = 0; i < tabInfo_->nrow; ++i) {
    if(basicVars[i] < tabInfo_->ncol) {
      basicInd_.insert(std::make_pair(basicVars[i], i));
    } else {
      nonbasicslack[basicVars[i] - tabInfo_->ncol] = 1;
    }
  }

  nnbOrig_ = 0;
  nbOrig_.clear();
  for(int i = 0; i < tabInfo_->ncol; ++i) {
    if(basicInd_.count(i) == 0) {
      nbOrig_.push_back(i);
      ++nnbOrig_;
    }
  }

  nnbSlack_ = 0;
  nbSlack_.clear();
  for(int i = 0; i < tabInfo_->nrow; ++i) {
    if(nonbasicslack[i] == 0) {
      nbSlack_.push_back(i);
      ++nnbSlack_;
    }
  }

  delete[] basicVars;
  nonbasicslack.clear();
}

void SimplexQuadCutGen::writeStats(std::ostream& out) const
{
  out << me_ << "Statistics for cutting:" << std::endl
      << me_ << "Number of rounds of cutting = " << stats_.numrounds
      << std::endl
      << me_ << "Number of cuts generated    = " << stats_.gencuts << std::endl
      << me_ << "Number of cuts added        = " << stats_.cutsadded
      << std::endl
      << me_ << "Time taken in cutting       = " << stats_.time << std::endl;
}
