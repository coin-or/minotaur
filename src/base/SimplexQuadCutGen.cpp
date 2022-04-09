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

#include <array>
#include <cmath>
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
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

const std::string SimplexQuadCutGen::me_ = "SimplexQuadCutGen";

SimplexQuadCutGen::SimplexQuadCutGen() {}

SimplexQuadCutGen::SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe)
    : env_(env), eTol_(1e-6), p_(p), lpe_(lpe), tabInfo_(0) {
  ncuts_ = 0;
  minDepth_ = 1e-3;
  iter_ = 0;
  allCuts_.clear();
  basicInd_.clear();
  nbOrig_.clear();
  nbSlack_.clear();
  sb_.clear();
}

SimplexQuadCutGen::~SimplexQuadCutGen() {
  for (SimplexCutVector::iterator it = allCuts_.begin(); it != allCuts_.end();
       ++it) {
    delete (*it);
  }
  allCuts_.clear();
  if (tabInfo_) {
    delete tabInfo_;
  }
  basicInd_.clear();
  nbOrig_.clear();
  nbSlack_.clear();
  sb_.clear();
}

int SimplexQuadCutGen::generateCuts(RelaxationPtr rel, const double *x) {
  if (iter_ >= env_->getOptions()->findInt("simplex_cut_rounds")->getValue()) {
    return 0;
  }
  ConstraintPtr c;
  FunctionType ftype;
  double act, cutConst, coef;
  int error = 0;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;
  // oxo - product of two original non-basic variables
  // oxs - product of an original and a slack non-basic variables
  // sxs - product of two slack non-basic variables
  QuadTerm oxo, oxs, sxs;
  // cutCoefo - Coefficient of cut for original variables
  // cutCoefs - Coefficient of cut for slack variables
  std::map<int, double> cutCoefo, cutCoefs;
  VariablePtr v1, v2;
  int ncuts;
  SimplexCutVector cuts;

  ++iter_;
  lpe_->enableFactorization();
  getBasicInfo_();
  sortVariables_();
  getSlackBounds_();
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
       ++cit) {
    c = *cit;
    ftype = c->getFunction()->getType();
    if (ftype != Quadratic && ftype != Bilinear) {
      // Linear constraints should be satisfied thus no cuts will be generated
      continue;
    }
    act = c->getActivity(x, &error);
    if (error == 0) {
      if (act <= c->getUb() + eTol_ && act >= c->getLb() - eTol_) {
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
    qf = c->getFunction()->getQuadraticFunction();
    lf = c->getFunction()->getLinearFunction();
    cutConst = 0.0;

    assert(qf);  // An infeasible constraint must have some quadratic part

    for (VariablePairGroupConstIterator qit = qf->begin(); qit != qf->end();
         ++qit) {
      v1 = rel->getRelaxationVar(qit->first.first);
      v2 = rel->getRelaxationVar(qit->first.second);
      coef = qit->second;
      if (basicInd_.count(v1->getIndex()) > 0) {
        // v1 is basic
        if (basicInd_.count(v2->getIndex()) > 0) {
          // v2 is basic
          multiplyBB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs, sxs,
                      cutCoefo, cutCoefs, cutConst);
        } else {
          // v2 is non-basic
          multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        }
      } else {
        // v1 is non-basic
        if (basicInd_.count(v2->getIndex()) > 0) {
          // v2 is basic
          multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        } else {
          // v2 is non-basic
          multiplyNBNB_(v1->getIndex(), v2->getIndex(), coef, oxo);
        }
      }
    }

    if (lf) {
      for (VariableGroupConstIterator lit = lf->termsBegin();
           lit != lf->termsEnd(); ++lit) {
        v1 = rel->getRelaxationVar(lit->first);
        coef = lit->second;
        if (basicInd_.count(v1->getIndex()) > 0) {
          // v1 is basic
          multiplyCB_(v1->getIndex(), coef, x, cutCoefo, cutCoefs, cutConst);
        } else {
          // v1 is non-basic
          multiplyCNB_(v1->getIndex(), coef, cutCoefo);
        }
      }
    }

    if (act > c->getUb() + eTol_) {
      ncuts = relaxQuadTerms_(cuts, rel, oxo, oxs, sxs, cutCoefo, cutCoefs,
                              cutConst, x, true, c->getUb());
      if (ncuts == 0) {
#if SPEW
        env_->getLogger()->msgStream(LogDebug)
            << me_ << "Constraint " << c->getName()
            << " is infeasible at this point yet no cuts generated"
            << std::endl;
#endif
      } else {
        ncuts_ += ncuts;
      }
    } else {
      ncuts = relaxQuadTerms_(cuts, rel, oxo, oxs, sxs, cutCoefo, cutCoefs,
                              cutConst, x, false, c->getLb());
      if (ncuts == 0) {
#if SPEW
        env_->getLogger()->msgStream(LogDebug)
            << me_ << "Constraint " << c->getName()
            << " is infeasible at this point yet no cuts generated"
            << std::endl;
#endif
      } else {
        ncuts_ += ncuts;
      }
    }
  }
  lpe_->disableFactorization();
  addCutsToRel_(cuts, rel, x, ncuts);
  return ncuts;
}

void SimplexQuadCutGen::addCutsToRel_(SimplexCutVector cuts, RelaxationPtr rel,
                                      const double *x, int &ncuts) {
  SimplexCutPtr cut;
  FunctionPtr f;
  ConstraintPtr c;
  double depth;

  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    // both lower and upper bounds must not be finite
    assert(!(cut->lb > -INFINITY && cut->ub < INFINITY));
    depth = calcDepth_(cut, x);
    if (depth < minDepth_) {
#if SPEW
      env_->getLogger()->msgStream(LogDebug)
          << me_ << " Cut not added since the depth of cut is very small"
          << std::endl;
#endif
      --ncuts;
      continue;
    }
    if (cut->lb > -INFINITY) {
      if (fabs(cut->lb) > 1e-3) {
        cut->lf->multiply(1 / fabs(cut->lb));
        cut->lb = cut->lb / fabs(cut->lb);
      }
    } else {
      if (fabs(cut->ub) > 1e-3) {
        cut->lf->multiply(1 / fabs(cut->ub));
        cut->ub = cut->ub / fabs(cut->ub);
      }
    }
    f = (FunctionPtr) new Function(cut->lf);
    c = rel->newConstraint(f, cut->lb, cut->ub);
#if SPEW
    env_->getLogger()->msgStream(LogDebug)
        << me_ << " added new cut. Depth of cut = " << depth << std::endl;
    c->write(env_->getLogger()->msgStream(LogDebug));
#endif
    allCuts_.push_back(cut);
  }
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, VariablePtr v,
                                   double coef, double cnst) {
  SimplexCutPtr cut;
  if (fabs(coef) < eTol_) {
    return;
  }
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cut->lf->incTerm(v, coef);
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst;
    }
  }
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, RelaxationPtr rel,
                                   int slackInd, double coef, double cnst) {
  SimplexCutPtr cut;
  std::map<int, double> row;
  double rhs = 0.0;

  if (fabs(coef) < eTol_) {
    return;
  }

  slackSubstitute_(slackInd, coef, row, rhs);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    for (std::map<int, double>::iterator itrow = row.begin();
         itrow != row.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst + rhs;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst + rhs;
    }
  }
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, VariablePtr v1,
                                   VariablePtr v2, double coef1, double coef2,
                                   double cnst) {
  SimplexCutPtr cut;
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cut->lf->incTerm(v1, coef1);
    cut->lf->incTerm(v2, coef2);
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst;
    }
  }
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, RelaxationPtr rel,
                                   int slackInd1, int slackInd2, double coef1,
                                   double coef2, double cnst) {
  SimplexCutPtr cut;
  std::map<int, double> row1, row2;
  double rhs1 = 0.0, rhs2 = 0.0;

  slackSubstitute_(slackInd1, coef1, row1, rhs1);
  slackSubstitute_(slackInd2, coef2, row2, rhs2);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    for (std::map<int, double>::iterator itrow = row1.begin();
         itrow != row1.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    for (std::map<int, double>::iterator itrow = row2.begin();
         itrow != row2.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst + rhs1 + rhs2;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst + rhs1 + rhs2;
    }
  }
  row1.clear();
  row2.clear();
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, RelaxationPtr rel,
                                   VariablePtr orig_v, int slackInd,
                                   double coef1, double coef2, double cnst) {
  SimplexCutPtr cut;
  std::map<int, double> row;
  double rhs = 0.0;

  slackSubstitute_(slackInd, coef2, row, rhs);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cut->lf->incTerm(orig_v, coef1);
    for (std::map<int, double>::iterator itrow = row.begin();
         itrow != row.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst + rhs;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst + rhs;
    }
  }
  row.clear();
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector &cuts, VariablePtr v1,
                                   VariablePtr v2, double c1v1, double c1v2,
                                   double c2v1, double c2v2, double cnst1,
                                   double cnst2) {
  SimplexCutPtr cut;
  SimplexCutVector cuts_new;

  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cuts_new.push_back(getNewCut_(cut, v1, v2, c2v1, c2v2, cnst2));
    cut->lf->incTerm(v1, c1v1);
    cut->lf->incTerm(v2, c1v2);
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst1;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst1;
    }
  }
  cuts.insert(cuts.end(), cuts_new.begin(), cuts_new.end());
  cuts_new.clear();
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector &cuts, RelaxationPtr rel,
                                   VariablePtr orig_v, int slackInd,
                                   double c1v1, double c1v2, double c2v1,
                                   double c2v2, double cnst1, double cnst2) {
  SimplexCutPtr cut;
  SimplexCutVector cuts_new;
  std::map<int, double> row;
  double rhs = 0.0;

  slackSubstitute_(slackInd, c1v2, row, rhs);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cuts_new.push_back(
        getNewCut_(cut, rel, orig_v, slackInd, c2v1, c2v2, cnst2));
    cut->lf->incTerm(orig_v, c1v1);
    for (std::map<int, double>::iterator itrow = row.begin();
         itrow != row.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst1 + rhs;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst1 + rhs;
    }
  }
  cuts.insert(cuts.end(), cuts_new.begin(), cuts_new.end());
  row.clear();
  cuts_new.clear();
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector &cuts, RelaxationPtr rel,
                                   int slackInd1, int slackInd2, double c1v1,
                                   double c1v2, double c2v1, double c2v2,
                                   double cnst1, double cnst2) {
  SimplexCutPtr cut;
  SimplexCutVector cuts_new;
  std::map<int, double> row1, row2;
  double rhs1 = 0.0, rhs2 = 0.0;

  slackSubstitute_(slackInd1, c1v1, row1, rhs1);
  slackSubstitute_(slackInd2, c1v2, row2, rhs2);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cuts_new.push_back(
        getNewCut_(cut, rel, slackInd1, slackInd2, c2v1, c2v2, cnst2));
    for (std::map<int, double>::iterator itrow = row1.begin();
         itrow != row1.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    for (std::map<int, double>::iterator itrow = row2.begin();
         itrow != row2.end(); ++itrow) {
      cut->lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst1 + rhs1 + rhs2;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst1 + rhs1 + rhs2;
    }
  }
  cuts.insert(cuts.end(), cuts_new.begin(), cuts_new.end());
  row1.clear();
  row2.clear();
  cuts_new.clear();
}

void SimplexQuadCutGen::addTerm_(std::map<int, double> &t, int v, double coef) {
  if (fabs(coef) > eTol_) {
    std::map<int, double>::iterator it = t.find(v);
    if (it == t.end()) {
      t.insert(std::make_pair(v, coef));
    } else {
      double nv = it->second + coef;
      if (fabs(nv) < eTol_) {
        t.erase(v);
      } else {
        it->second = nv;
      }
    }
  }
}

void SimplexQuadCutGen::addTerm_(QuadTerm &t, VarProd vp, double coef) {
  if (fabs(coef) > eTol_) {
    QuadTerm::iterator it = t.find(vp);
    if (it == t.end()) {
      t.insert(std::make_pair(vp, coef));
    } else {
      double nv = it->second + coef;
      if (fabs(nv) < eTol_) {
        t.erase(vp);
      } else {
        it->second = nv;
      }
    }
  }
}

void SimplexQuadCutGen::delTabInfo_() {
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

double SimplexQuadCutGen::calcDepth_(SimplexCutPtr cut, const double *x) {
  double depth;
  double norm = 0;

  for (VariableGroupConstIterator it = cut->lf->termsBegin();
       it != cut->lf->termsEnd(); ++it) {
    norm += (it->second) * (it->second);
  }
  norm = sqrt(norm);

  if (cut->lb > -INFINITY) {
    depth = (cut->lb - cut->lf->eval(x)) / norm;
  } else {
    depth = (cut->lf->eval(x) - cut->ub) / norm;
  }
  return depth;
}

void SimplexQuadCutGen::getBasicInfo_() {
  if (!lpe_->IsOptimalBasisAvailable()) {
    env_->getLogger()->msgStream(LogDebug)
        << me_ << "Solver provided no Optimal Basis" << std::endl;
  }

  tabInfo_ = (TableauInfo *)new TableauInfo();
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

SimplexCutVector SimplexQuadCutGen::getCutVec_(RelaxationPtr rel,
                                               std::map<int, double> cutCoefo,
                                               std::map<int, double> cutCoefs,
                                               double cutConst, bool under,
                                               double rhs) {
  SimplexCutVector cuts;
  SimplexCutPtr cut = (SimplexCutPtr) new SimplexCut();
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  std::map<int, double>::iterator it, itrow;
  std::map<int, double> row;
  double rowrhs = 0.0;

  for (it = cutCoefo.begin(); it != cutCoefo.end(); ++it) {
    lf->incTerm(rel->getVariable(it->first), it->second);
  }

  for (it = cutCoefs.begin(); it != cutCoefs.end(); ++it) {
    slackSubstitute_(it->first, it->second, row, rowrhs);
    for (itrow = row.begin(); itrow != row.end(); ++itrow) {
      lf->incTerm(rel->getVariable(itrow->first), -(itrow->second));
    }
    cutConst += rowrhs;
    row.clear();
  }

  cut->lf = lf;
  if (under) {
    cut->lb = -INFINITY;
    cut->ub = rhs - cutConst;
  } else {
    cut->lb = rhs - cutConst;
    cut->ub = INFINITY;
  }
  cut->numInactive = 0;
  cuts.push_back(cut);
  return cuts;
}

SimplexCutPtr SimplexQuadCutGen::getNewCut_(SimplexCutPtr cut, VariablePtr v1,
                                            VariablePtr v2, double coef1,
                                            double coef2, double cnst) {
  SimplexCutPtr new_cut = (SimplexCutPtr) new SimplexCut();
  new_cut->lf = cut->lf->clone();
  new_cut->lf->incTerm(v1, coef1);
  new_cut->lf->incTerm(v2, coef2);
  if (cut->lb > -INFINITY) {
    new_cut->lb = cut->lb - cnst;
  } else {
    new_cut->lb = -INFINITY;
  }

  if (cut->ub < INFINITY) {
    new_cut->ub = cut->ub - cnst;
  } else {
    new_cut->ub = INFINITY;
  }
  new_cut->numInactive = 0;
  return new_cut;
}

SimplexCutPtr SimplexQuadCutGen::getNewCut_(SimplexCutPtr cut,
                                            RelaxationPtr rel,
                                            VariablePtr orig_v, int slackInd,
                                            double coef1, double coef2,
                                            double cnst) {
  SimplexCutPtr new_cut = (SimplexCutPtr) new SimplexCut();
  std::map<int, double> row;
  double rhs = 0.0;

  slackSubstitute_(slackInd, coef2, row, rhs);
  new_cut->lf = cut->lf->clone();
  new_cut->lf->incTerm(orig_v, coef1);
  for (std::map<int, double>::iterator it = row.begin(); it != row.end();
       ++it) {
    new_cut->lf->incTerm(rel->getVariable(it->first), -(it->second));
  }
  if (cut->lb > -INFINITY) {
    new_cut->lb = cut->lb - (cnst + rhs);
  } else {
    new_cut->lb = -INFINITY;
  }

  if (cut->ub < INFINITY) {
    new_cut->ub = cut->ub - (cnst + rhs);
  } else {
    new_cut->ub = INFINITY;
  }
  row.clear();
  new_cut->numInactive = 0;
  return new_cut;
}

SimplexCutPtr SimplexQuadCutGen::getNewCut_(SimplexCutPtr cut,
                                            RelaxationPtr rel, int slackInd1,
                                            int slackInd2, double coef1,
                                            double coef2, double cnst) {
  SimplexCutPtr new_cut = (SimplexCutPtr) new SimplexCut();
  std::map<int, double> row1, row2;
  double rhs1 = 0.0, rhs2 = 0.0;

  slackSubstitute_(slackInd1, coef1, row1, rhs1);
  slackSubstitute_(slackInd2, coef2, row2, rhs2);
  new_cut->lf = cut->lf->clone();
  for (std::map<int, double>::iterator it = row1.begin(); it != row1.end();
       ++it) {
    new_cut->lf->incTerm(rel->getVariable(it->first), -(it->second));
  }
  for (std::map<int, double>::iterator it = row2.begin(); it != row2.end();
       ++it) {
    new_cut->lf->incTerm(rel->getVariable(it->first), -(it->second));
  }
  if (cut->lb > -INFINITY) {
    new_cut->lb = cut->lb - (cnst + rhs1 + rhs2);
  } else {
    new_cut->lb = -INFINITY;
  }

  if (cut->ub < INFINITY) {
    new_cut->ub = cut->ub - (cnst + rhs1 + rhs2);
  } else {
    new_cut->ub = INFINITY;
  }
  row1.clear();
  row2.clear();
  new_cut->numInactive = 0;
  return new_cut;
}

void SimplexQuadCutGen::getSlackBounds_() {
  double lb, ub, rhs;
  int row, upto;

  sb_.clear();
  for (int i = 0; i < nnbSlack_; ++i) {
    row = nbSlack_[i];
    lb = 0.0;
    ub = 0.0;
    upto = tabInfo_->rowStart[row] + tabInfo_->rowLen[row];
    for (int j = tabInfo_->rowStart[row]; j < upto; ++j) {
      getTermBounds_(tabInfo_->origTab[j],
                     tabInfo_->colLower[tabInfo_->indices[j]],
                     tabInfo_->colUpper[tabInfo_->indices[j]], lb, ub);
    }

    if (lb < tabInfo_->rowLower[row] - eTol_) {
      lb = tabInfo_->rowLower[row];
    }
    if (ub > tabInfo_->rowUpper[row] + eTol_) {
      ub = tabInfo_->rowUpper[row];
    }

    rhs = tabInfo_->rowRhs[row];
    sb_.insert(std::make_pair(row, std::make_pair(rhs - ub, rhs - lb)));
  }
}

void SimplexQuadCutGen::getTermBounds_(double coef, double vlb, double vub,
                                       double &lb, double &ub) {
  if (fabs(coef) < eTol_) {
    // Coef is zero
    return;
  }

  if (coef > eTol_) {
    lb += coef * vlb;
    ub += coef * vub;
  } else {
    lb += coef * vub;
    ub += coef * vlb;
  }
}

void SimplexQuadCutGen::multiplyBB_(int b1, int b2, double coef,
                                    const double *x, QuadTerm &oxo,
                                    QuadTerm &oxs, QuadTerm &sxs,
                                    std::map<int, double> &cutCoefo,
                                    std::map<int, double> &cutCoefs,
                                    double &cutConst) {
  double *origRow1 = new double[tabInfo_->ncol];
  double *origRow2 = new double[tabInfo_->ncol];
  double *slackRow1 = new double[tabInfo_->nrow];
  double *slackRow2 = new double[tabInfo_->nrow];
  double beta1 = x[b1];
  double beta2 = x[b2];
  double elem1, elem2;

  lpe_->getBInvARow(basicInd_[b1], origRow1, slackRow1);
  lpe_->getBInvARow(basicInd_[b2], origRow2, slackRow2);

  // We assume non-basic slack variables will be zero
  for (int i = 0; i < nnbOrig_; ++i) {
    beta1 += origRow1[nbOrig_[i]] * x[nbOrig_[i]];
    beta2 += origRow2[nbOrig_[i]] * x[nbOrig_[i]];
  }

  // We assume that the coef is always nonzero
  for (int i = 0; i < nnbOrig_; ++i) {
    elem1 = origRow1[nbOrig_[i]];
    if (fabs(elem1) > eTol_) {
      for (int j = 0; j < nnbOrig_; ++j) {
        elem2 = origRow2[nbOrig_[j]];
        if (fabs(elem2) > eTol_) {
          if (i <= j) {
            addTerm_(oxo, std::make_pair(nbOrig_[i], nbOrig_[j]),
                     coef * elem1 * elem2);
          } else {
            addTerm_(oxo, std::make_pair(nbOrig_[j], nbOrig_[i]),
                     coef * elem1 * elem2);
          }
        }
      }

      for (int j = 0; j < nnbSlack_; ++j) {
        elem2 = slackRow2[nbSlack_[j]];
        if (fabs(elem2) > eTol_) {
          addTerm_(oxs, std::make_pair(nbOrig_[i], nbSlack_[j]),
                   coef * elem1 * elem2);
        }
      }

      addTerm_(cutCoefo, nbOrig_[i], -coef * elem1 * beta2);
    }
  }

  for (int i = 0; i < nnbSlack_; ++i) {
    elem1 = slackRow1[nbSlack_[i]];
    if (fabs(elem1) > eTol_) {
      for (int j = 0; j < nnbOrig_; ++j) {
        elem2 = origRow2[nbOrig_[j]];
        if (fabs(elem2) > eTol_) {
          addTerm_(oxs, std::make_pair(nbOrig_[j], nbSlack_[i]),
                   coef * elem1 * elem2);
        }
      }

      for (int j = 0; j < nnbSlack_; ++j) {
        elem2 = slackRow2[nbSlack_[j]];
        if (fabs(elem2) > eTol_) {
          if (i <= j) {
            addTerm_(sxs, std::make_pair(nbSlack_[i], nbSlack_[j]),
                     coef * elem1 * elem2);
          } else {
            addTerm_(sxs, std::make_pair(nbSlack_[j], nbSlack_[i]),
                     coef * elem1 * elem2);
          }
        }
      }

      addTerm_(cutCoefs, nbSlack_[i], -coef * elem1 * beta2);
    }
  }

  for (int j = 0; j < nnbOrig_; ++j) {
    elem2 = origRow2[nbOrig_[j]];
    if (fabs(elem2) > eTol_) {
      addTerm_(cutCoefo, nbOrig_[j], -coef * beta1 * elem2);
    }
  }

  for (int j = 0; j < nnbSlack_; ++j) {
    elem2 = slackRow2[nbSlack_[j]];
    if (fabs(elem2) > eTol_) {
      addTerm_(cutCoefs, nbSlack_[j], -coef * beta1 * elem2);
    }
  }

  cutConst += coef * beta1 * beta2;

  delete[] origRow1;
  delete[] origRow2;
  delete[] slackRow1;
  delete[] slackRow2;
}

void SimplexQuadCutGen::multiplyBNB_(int b, int nb, double coef,
                                     const double *x, QuadTerm &oxo,
                                     QuadTerm &oxs,
                                     std::map<int, double> &cutCoefo) {
  double *origRow = new double[tabInfo_->ncol];
  double *slackRow = new double[tabInfo_->nrow];
  double elem;
  double beta = x[b];

  // memset(origRow, 0, tabInfo_.ncol * sizeof(double));
  // memset(slackRow, 0, tabInfo_.nrow * sizeof(double));

  lpe_->getBInvARow(basicInd_[b], origRow, slackRow);

  // We assume non-basic slack variables will be zero
  for (int i = 0; i < nnbOrig_; ++i) {
    beta += origRow[nbOrig_[i]] * x[nbOrig_[i]];
  }

  for (int i = 0; i < nnbOrig_; ++i) {
    elem = origRow[nbOrig_[i]];
    if (fabs(elem) > eTol_) {
      if (nbOrig_[i] <= nb) {
        addTerm_(oxo, std::make_pair(nbOrig_[i], nb), -coef * elem);
      } else {
        addTerm_(oxo, std::make_pair(nb, nbOrig_[i]), -coef * elem);
      }
    }
  }

  for (int i = 0; i < nnbSlack_; ++i) {
    elem = slackRow[nbSlack_[i]];
    if (fabs(elem) > eTol_) {
      addTerm_(oxs, std::make_pair(nb, nbSlack_[i]), -coef * elem);
    }
  }

  addTerm_(cutCoefo, nb, coef * beta);

  delete[] origRow;
  delete[] slackRow;
}

void SimplexQuadCutGen::multiplyCB_(int b, double coef, const double *x,
                                    std::map<int, double> &cutCoefo,
                                    std::map<int, double> &cutCoefs,
                                    double &cutConst) {
  double *origRow = new double[tabInfo_->ncol];
  double *slackRow = new double[tabInfo_->nrow];
  double elem;
  double beta = x[b];

  // memset(origRow, 0, tabInfo_->ncol * sizeof(double));
  // memset(slackRow, 0, tabInfo_.nrow * sizeof(double));

  lpe_->getBInvARow(basicInd_[b], origRow, slackRow);

  // We assume non-basic slack variables will be zero
  for (int i = 0; i < nnbOrig_; ++i) {
    beta += origRow[nbOrig_[i]] * x[nbOrig_[i]];
  }

  for (int i = 0; i < nnbOrig_; ++i) {
    elem = origRow[nbOrig_[i]];
    if (fabs(elem) > eTol_) {
      addTerm_(cutCoefo, nbOrig_[i], -coef * elem);
    }
  }

  for (int i = 0; i < nnbSlack_; ++i) {
    elem = slackRow[nbSlack_[i]];
    if (fabs(elem) > eTol_) {
      addTerm_(cutCoefs, nbSlack_[i], -coef * elem);
    }
  }

  cutConst += coef * beta;

  delete[] origRow;
  delete[] slackRow;
}

void SimplexQuadCutGen::multiplyCNB_(int nb, double coef,
                                     std::map<int, double> &cutCoefo) {
  addTerm_(cutCoefo, nb, coef);
}

void SimplexQuadCutGen::multiplyNBNB_(int nb1, int nb2, double coef,
                                      QuadTerm &oxo) {
  if (nb1 <= nb2) {
    addTerm_(oxo, std::make_pair(nb1, nb2), coef);
  } else {
    addTerm_(oxo, std::make_pair(nb2, nb1), coef);
  }
}

int SimplexQuadCutGen::relaxBilTerm_(double coef, bool lower1, bool lower2,
                                     double l1, double u1, double l2, double u2,
                                     double &c1v1, double &c1v2, double &c2v1,
                                     double &c2v2, double &cnst1, double &cnst2,
                                     bool under) {
  int numCuts;
  int variant = env_->getOptions()->findInt("simplex_cut_variant")->getValue();
  if (under) {
    if (coef > eTol_) {
      if (lower1 == lower2) {
        double p1 = lower1 ? l1 : u1;
        double p2 = lower2 ? l2 : u2;
        c1v1 = coef * p2;
        c1v2 = coef * p1;
        cnst1 = -coef * p1 * p2;
        numCuts = 1;
      } else {
        if (variant == 1) {
          c1v1 = coef * l2;
          c1v2 = coef * l1;
          cnst1 = -coef * l1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * u1;
          cnst2 = -coef * u1 * u2;
          numCuts = 2;
        } else {
          c1v1 = coef * (0.5 * l2 + 0.5 * u2);
          c1v2 = coef * (0.5 * l1 + 0.5 * u1);
          cnst1 = -coef * (0.5 * l1 * l2 + 0.5 * u1 * u2);
          numCuts = 1;
        }
      }
    } else {
      if (lower1 == lower2) {
        if (variant == 1) {
          c1v1 = coef * l2;
          c1v2 = coef * u1;
          cnst1 = -coef * u1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * l1;
          cnst2 = -coef * l1 * u2;
          numCuts = 2;
        } else {
          c1v1 = coef * (0.5 * l2 + 0.5 * u2);
          c1v2 = coef * (0.5 * l1 + 0.5 * u1);
          cnst1 = -coef * (0.5 * u1 * l2 + 0.5 * l1 * u2);
          numCuts = 1;
        }
      } else {
        double p1 = lower1 ? l1 : u1;
        double p2 = lower2 ? l2 : u2;
        c1v1 = coef * p2;
        c1v2 = coef * p1;
        cnst1 = -coef * p1 * p2;
        numCuts = 1;
      }
    }
  } else {
    if (coef < -eTol_) {
      if (lower1 == lower2) {
        double p1 = lower1 ? l1 : u1;
        double p2 = lower2 ? l2 : u2;
        c1v1 = coef * p2;
        c1v2 = coef * p1;
        cnst1 = -coef * p1 * p2;
        numCuts = 1;
      } else {
        if (variant == 1) {
          c1v1 = coef * l2;
          c1v2 = coef * l1;
          cnst1 = -coef * l1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * u1;
          cnst2 = -coef * u1 * u2;
          numCuts = 2;
        } else {
          c1v1 = coef * (0.5 * l2 + 0.5 * u2);
          c1v2 = coef * (0.5 * l1 + 0.5 * u1);
          cnst1 = -coef * (0.5 * l1 * l2 + 0.5 * u1 * u2);
          numCuts = 1;
        }
      }
    } else {
      if (lower1 == lower2) {
        if (variant == 1) {
          c1v1 = coef * l2;
          c1v2 = coef * u1;
          cnst1 = -coef * u1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * l1;
          cnst2 = -coef * l1 * u2;
          numCuts = 2;
        } else {
          c1v1 = coef * (0.5 * l2 + 0.5 * u2);
          c1v2 = coef * (0.5 * l1 + 0.5 * u1);
          cnst1 = -coef * (0.5 * u1 * l2 + 0.5 * l1 * u2);
          numCuts = 1;
        }
      } else {
        double p1 = lower1 ? l1 : u1;
        double p2 = lower2 ? l2 : u2;
        c1v1 = coef * p2;
        c1v2 = coef * p1;
        cnst1 = -coef * p1 * p2;
        numCuts = 1;
      }
    }
  }
  return numCuts;
}

int SimplexQuadCutGen::relaxQuadTerms_(SimplexCutVector &iter_cuts,
                                       RelaxationPtr rel, QuadTerm oxo,
                                       QuadTerm oxs, QuadTerm sxs,
                                       std::map<int, double> cutCoefo,
                                       std::map<int, double> cutCoefs,
                                       double cutConst, const double *x,
                                       bool under, double rhs) {
  SimplexCutVector cuts =
      getCutVec_(rel, cutCoefo, cutCoefs, cutConst, under, rhs);
  double c1v1 = 0.0, c1v2 = 0.0, c2v1 = 0.0, c2v2 = 0.0;
  double cnst1 = 0.0, cnst2 = 0.0;
  VarProd vp;
  bool lower1, lower2;
  VariablePtr v1, v2;
  int ncuts;

  // For oxo. Assume that all original variables are at its bounds since they
  // are non-basic
  for (QuadTerm::iterator it = oxo.begin(); it != oxo.end(); ++it) {
    if (fabs(it->second) < eTol_) {
      continue;
    }
    vp = it->first;
    if (vp.first == vp.second) {
      // Original square term
      v1 = rel->getVariable(vp.first);
      lower1 = fabs(x[vp.first] - v1->getLb()) < eTol_ ? true : false;
      relaxSqTerm_(it->second, lower1, v1->getLb(), v1->getUb(), c1v1, cnst1,
                   under);
      addLfTerm_(cuts, v1, c1v1, cnst1);
    } else {
      // Original bilinear term
      v1 = rel->getVariable(vp.first);
      v2 = rel->getVariable(vp.second);
      lower1 = fabs(x[vp.first] - v1->getLb()) < eTol_ ? true : false;
      lower2 = fabs(x[vp.second] - v1->getLb()) < eTol_ ? true : false;
      ncuts = relaxBilTerm_(it->second, lower1, lower2, v1->getLb(),
                            v1->getUb(), v2->getLb(), v2->getUb(), c1v1, c1v2,
                            c2v1, c2v2, cnst1, cnst2, under);
      if (ncuts == 1) {
        addLfTerm_(cuts, v1, v2, c1v1, c1v2, cnst1);
      } else {
        addLfTerm_(cuts, v1, v2, c1v1, c1v2, c2v1, c2v2, cnst1, cnst2);
      }
    }
  }

  // For oxs. Assume non-basic slack variable is always at zero. That is the
  // corresponding constraint is binding
  for (QuadTerm::iterator it = oxs.begin(); it != oxs.end(); ++it) {
    if (fabs(it->second) < eTol_) {
      continue;
    }
    vp = it->first;
    v1 = rel->getVariable(vp.first);
    lower1 = fabs(x[vp.first] - v1->getLb()) < eTol_ ? true : false;
    lower2 = sb_[vp.second].second > eTol_ ? true : false;
    ncuts = relaxBilTerm_(it->second, lower1, lower2, v1->getLb(), v1->getUb(),
                          sb_[vp.second].first, sb_[vp.second].second, c1v1,
                          c1v2, c2v1, c2v2, cnst1, cnst2, under);
    if (ncuts == 1) {
      addLfTerm_(cuts, rel, v1, vp.second, c1v1, c1v2, cnst1);
    } else {
      addLfTerm_(cuts, rel, v1, vp.second, c1v1, c1v2, c2v1, c2v2, cnst1,
                 cnst2);
    }
  }

  // For sxs.
  for (QuadTerm::iterator it = sxs.begin(); it != sxs.end(); ++it) {
    if (fabs(it->second) < eTol_) {
      continue;
    }
    vp = it->first;
    if (vp.first == vp.second) {
      // Slack square term
      lower1 = sb_[vp.first].second > eTol_ ? true : false;
      relaxSqTerm_(it->second, lower1, sb_[vp.first].first,
                   sb_[vp.first].second, c1v1, cnst1, under);
      addLfTerm_(cuts, rel, vp.first, c1v1, cnst1);
    } else {
      // Slack bilinear term
      lower1 = sb_[vp.first].second > eTol_ ? true : false;
      lower2 = sb_[vp.second].second > eTol_ ? true : false;
      ncuts = relaxBilTerm_(it->second, lower1, lower2, sb_[vp.first].first,
                            sb_[vp.first].second, sb_[vp.second].first,
                            sb_[vp.second].second, c1v1, c1v2, c2v1, c2v2,
                            cnst1, cnst2, under);
      if (ncuts == 1) {
        addLfTerm_(cuts, rel, vp.first, vp.second, c1v1, c1v2, cnst1);
      } else {
        addLfTerm_(cuts, rel, vp.first, vp.second, c1v1, c1v2, c2v1, c2v2,
                   cnst1, cnst2);
      }
    }
  }
  iter_cuts.insert(iter_cuts.end(), cuts.begin(), cuts.end());
  return cuts.size();
}

void SimplexQuadCutGen::relaxSqTerm_(double coef, bool atLower, double l,
                                     double u, double &lincoef, double &cnst,
                                     bool under) {
  // We assume coef will be nonzero
  if (under) {
    if (coef > eTol_) {
      if (atLower) {
        lincoef = 2 * l * coef;
        cnst -= l * l * coef;
      } else {
        lincoef = 2 * u * coef;
        cnst -= u * u * coef;
      }
    } else {
      lincoef = (l + u) * coef;
      cnst -= u * l * coef;
    }
  } else {
    if (coef < -eTol_) {
      if (atLower) {
        lincoef = 2 * l * coef;
        cnst -= l * l * coef;
      } else {
        lincoef = 2 * u * coef;
        cnst -= u * u * coef;
      }
    } else {
      lincoef = (l + u) * coef;
      cnst -= u * l * coef;
    }
  }
}

void SimplexQuadCutGen::slackSubstitute_(int slackInd, double coef,
                                         std::map<int, double> &row,
                                         double &rhs) {
  // Get the row from tabInfo_
  int upto = tabInfo_->rowStart[slackInd] + tabInfo_->rowLen[slackInd];
  for (int j = tabInfo_->rowStart[slackInd]; j < upto; ++j) {
    row.insert(
        std::make_pair(tabInfo_->indices[j], coef * tabInfo_->origTab[j]));
  }
  rhs = coef * tabInfo_->rowRhs[slackInd];
}

void SimplexQuadCutGen::sortVariables_() {
  int *basicVars = new int[tabInfo_->nrow];
  std::vector<int> nonbasicslack(tabInfo_->nrow, 0);

  lpe_->getBasics(basicVars);
  basicInd_.clear();

  for (int i = 0; i < tabInfo_->nrow; ++i) {
    if (basicVars[i] < tabInfo_->ncol) {
      basicInd_.insert(std::make_pair(basicVars[i], i));
    } else {
      nonbasicslack[basicVars[i] - tabInfo_->ncol] = 1;
    }
  }

  nnbOrig_ = 0;
  nbOrig_.clear();
  for (int i = 0; i < tabInfo_->ncol; ++i) {
    if (basicInd_.count(i) == 0) {
      nbOrig_.push_back(i);
      ++nnbOrig_;
    }
  }

  nnbSlack_ = 0;
  nbSlack_.clear();
  for (int i = 0; i < tabInfo_->nrow; ++i) {
    if (nonbasicslack[i] == 0) {
      nbSlack_.push_back(i);
      ++nnbSlack_;
    }
  }

  delete[] basicVars;
  nonbasicslack.clear();
}
