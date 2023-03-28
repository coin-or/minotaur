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

const std::string SimplexQuadCutGen::me_ = "SimplexQuadCutGen";

SimplexQuadCutGen::SimplexQuadCutGen() {}

SimplexQuadCutGen::SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe)
    : env_(env), eTol_(1e-8), p_(p), lpe_(lpe), tabInfo_(0) {
  ncuts_ = 0;
  minDepth_ = 1e-3;
  iter_ = 0;
  variant_ = env->getOptions()->findInt("simplex_cut_variant")->getValue();
  allCuts_.clear();
  basicInd_.clear();
  nbOrig_.clear();
  nbSlack_.clear();
  sb_.clear();
  timer_ = env->getTimer();
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

int SimplexQuadCutGen::generateCuts(RelaxationPtr rel, ConstSolutionPtr sol) {
  if (iter_ >= env_->getOptions()->findInt("simplex_cut_rounds")->getValue()) {
    return 0;
  }
  ConstraintPtr c;
  FunctionType ftype;
  double act, cutConst;
  int error = 0;
  // oxo - product of two original non-basic variables
  SubstQuadPtr oxo = (SubstQuadPtr) new SubstQuad();
  // oxs - product of an original and a slack non-basic variables
  SubstQuadPtr oxs = (SubstQuadPtr) new SubstQuad();
  // sxs - product of two slack non-basic variables
  SubstQuadPtr sxs = (SubstQuadPtr) new SubstQuad();
  // cutCoefo - Coefficient of cut for original variables
  // cutCoefs - Coefficient of cut for slack variables
  double *cutCoefo = 0;
  double *cutCoefs = 0;
  int ncuts, iter_cuts = 0;
  SimplexCutVector cuts;
  double stime = timer_->query();
  std::map<int, int> countInf;

  ++iter_;
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << " : Round " << iter_ << std::endl;
  preprocessSimplexTab();
  if (variant_ >= 7) {
    std::map<int, int>::iterator countInfIt1, countInfIt2;
    for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
         ++cit) {
      c = *cit;
      ftype = c->getFunction()->getType();
      if (ftype != Quadratic && ftype != Bilinear) {
        continue;
      }
      act = c->getActivity(sol->getPrimal(), &error);
      if (error == 0) {
        if (act <= c->getUb() + eTol_ && act >= c->getLb() - eTol_) {
          continue;
        }
      } else {
#if SPEW
        env_->getLogger()->msgStream(LogDebug)
            << me_ << "Constraint " << c->getName()
            << " is not defined at this point." << std::endl;
#endif
      }
      VariablePtr v1, v2;
      QuadraticFunctionPtr qf;

      qf = c->getFunction()->getQuadraticFunction();

      // We assume the constraint here is of the form y = x1x2 or y = x1^2
      v1 = rel->getRelaxationVar(qf->begin()->first.first);
      v2 = rel->getRelaxationVar(qf->begin()->first.second);
      if (v1->getIndex() == v2->getIndex()) {
        countInfIt1 = countInf.find(v1->getIndex());
        if (countInfIt1 == countInf.end()) {
          countInf.insert({v1->getIndex(), 1});
        } else {
          ++(countInfIt1->second);
        }
      } else {
        countInfIt1 = countInf.find(v1->getIndex());
        countInfIt2 = countInf.find(v2->getIndex());
        if (countInfIt1 == countInf.end()) {
          countInf.insert({v1->getIndex(), 1});
        } else {
          ++(countInfIt1->second);
        }
        if (countInfIt2 == countInf.end()) {
          countInf.insert({v2->getIndex(), 1});
        } else {
          ++(countInfIt2->second);
        }
      }
    }
  }
  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
       ++cit) {
    c = *cit;
    ftype = c->getFunction()->getType();
    if (ftype != Quadratic && ftype != Bilinear) {
      // Linear constraints should be satisfied thus no cuts will be generated
      continue;
    }
    act = c->getActivity(sol->getPrimal(), &error);
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
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << " : Generating cuts for the constraint " << c->getName()
        << std::endl;
    cutConst = 0.0;
    if (variant_ >= 6) {
      int count = 0;
      cutCoefo = new double[tabInfo_->ncol];
      memset(cutCoefo, 0, tabInfo_->ncol * sizeof(double));
      while (count < 2) {
        ++count;
        if (count == 2) {
          oxo->ind1.clear();
          oxo->ind2.clear();
          oxo->val.clear();
          oxs->ind1.clear();
          oxs->ind2.clear();
          oxs->val.clear();
          memset(cutCoefo, 0, tabInfo_->ncol * sizeof(double));
        }
        if (variant_ == 6) {
          getQuadraticBNB(c, sol->getPrimal(), rel, oxo, oxs, cutCoefo,
                          cutConst, count);
        } else {
          ++count;
          getQuadraticBNB(c, sol->getPrimal(), rel, oxo, oxs, cutCoefo,
                          cutConst, countInf);
        }
        if (act > c->getUb() + eTol_) {
          ncuts = relaxQuadTermsBNB_(cuts, rel, oxo, oxs, cutCoefo, cutConst,
                                     sol, true, c->getUb());
          if (ncuts == 0) {
#if SPEW
            env_->getLogger()->msgStream(LogDebug)
                << me_ << "Constraint " << c->getName()
                << " is infeasible at this point yet no cuts generated"
                << std::endl;
#endif
          } else {
            iter_cuts += ncuts;
          }
        } else {
          ncuts = relaxQuadTermsBNB_(cuts, rel, oxo, oxs, cutCoefo, cutConst,
                                     sol, false, c->getLb());
          if (ncuts == 0) {
#if SPEW
            env_->getLogger()->msgStream(LogDebug)
                << me_ << "Constraint " << c->getName()
                << " is infeasible at this point yet no cuts generated"
                << std::endl;
#endif
          } else {
            iter_cuts += ncuts;
          }
        }
      }
    } else {
      cutCoefo = new double[tabInfo_->ncol];
      cutCoefs = new double[tabInfo_->nrow];
      memset(cutCoefo, 0, tabInfo_->ncol * sizeof(double));
      memset(cutCoefs, 0, tabInfo_->nrow * sizeof(double));
      getQuadratic(c, sol->getPrimal(), rel, oxo, oxs, sxs, cutCoefo, cutCoefs,
                   cutConst);

      if (act > c->getUb() + eTol_) {
        ncuts = relaxQuadTerms_(cuts, rel, oxo, oxs, sxs, cutCoefo, cutCoefs,
                                cutConst, sol, true, c->getUb());
        if (ncuts == 0) {
#if SPEW
          env_->getLogger()->msgStream(LogDebug)
              << me_ << "Constraint " << c->getName()
              << " is infeasible at this point yet no cuts generated"
              << std::endl;
#endif
        } else {
          iter_cuts += ncuts;
        }
      } else {
        ncuts = relaxQuadTerms_(cuts, rel, oxo, oxs, sxs, cutCoefo, cutCoefs,
                                cutConst, sol, false, c->getLb());
        if (ncuts == 0) {
#if SPEW
          env_->getLogger()->msgStream(LogDebug)
              << me_ << "Constraint " << c->getName()
              << " is infeasible at this point yet no cuts generated"
              << std::endl;
#endif
        } else {
          iter_cuts += ncuts;
        }
      }
    }
  }
  disableFactorization();
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << ": No. of cuts generated: " << iter_cuts << std::endl;
  addCutsToRel_(cuts, rel, sol->getPrimal(), iter_cuts);
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << ": No. of cuts added: " << iter_cuts << std::endl;
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << ": Time taken in cut generation: " << timer_->query() - stime
      << std::endl;
  ncuts_ += iter_cuts;
  if (oxo) {
    delete oxo;
  }
  if (oxs) {
    delete oxs;
  }
  if (sxs) {
    delete sxs;
  }
  if (cutCoefo) {
    delete[] cutCoefo;
  }
  if (cutCoefs) {
    delete[] cutCoefs;
  }
  return iter_cuts;
}

double SimplexQuadCutGen::getSlackLb(int s) { return sb_[s].first; }

double SimplexQuadCutGen::getSlackUb(int s) { return sb_[s].second; }

void SimplexQuadCutGen::preprocessSimplexTab() {
  lpe_->enableFactorization();
  getBasicInfo_();
  sortVariables_();
  getSlackBounds_();
}

void SimplexQuadCutGen::getAffineFnForSlack(RelaxationPtr rel, int s,
                                            LinearFunctionPtr &lf, double &d) {
  std::map<int, double> row;

  slackSubstitute_(s, 1.0, row, d);
  for (std::map<int, double>::iterator it = row.begin(); it != row.end();
       ++it) {
    lf->addTerm(rel->getVariable(it->first), -it->second);
  }
}

void SimplexQuadCutGen::disableFactorization() { lpe_->disableFactorization(); }

void SimplexQuadCutGen::getQuadraticBNB(ConstraintPtr c, const double *x,
                                        RelaxationPtr rel, SubstQuadPtr &oxo,
                                        SubstQuadPtr &oxs, double *cutCoefo,
                                        double &cutConst,
                                        std::map<int, int> &countInf) {
  VariablePtr v1, v2;
  double coef;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;

  qf = c->getFunction()->getQuadraticFunction();
  lf = c->getFunction()->getLinearFunction();
  cutConst = 0.0;

  assert(qf);

  // We assume the constraint here is of the form y = x1x2 or y = x1^2
  v1 = rel->getRelaxationVar(qf->begin()->first.first);
  v2 = rel->getRelaxationVar(qf->begin()->first.second);
  coef = qf->begin()->second;
  if (v1->getIndex() == v2->getIndex()) {
    multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs, cutCoefo);
  } else {
    double *origRow1 = new double[tabInfo_->ncol];
    double *slackRow1 = new double[tabInfo_->nrow];
    lpe_->getBInvARow(basicInd_[v1->getIndex()], origRow1, slackRow1);
    double *origRow2 = new double[tabInfo_->ncol];
    double *slackRow2 = new double[tabInfo_->nrow];
    lpe_->getBInvARow(basicInd_[v2->getIndex()], origRow2, slackRow2);
    int c1 = 0, c2 = 0;
    double elem;
    for (int i = 0; i < nnbOrig_; ++i) {
      elem = origRow1[nbOrig_[i]];
      if (fabs(elem) > eTol_) {
        ++c1;
      }
      elem = origRow2[nbOrig_[i]];
      if (fabs(elem) > eTol_) {
        ++c2;
      }
    }
    for (int i = 0; i < nnbSlack_; ++i) {
      elem = slackRow1[nbSlack_[i]];
      if (fabs(elem) > eTol_) {
        ++c1;
      }
      elem = slackRow2[nbSlack_[i]];
      if (fabs(elem) > eTol_) {
        ++c2;
      }
    }

    if (variant_ == 7) {
      if (c1 < c2) {
        multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs,
                     cutCoefo);
      } else if (c1 == c2) {
        if (countInf[v1->getIndex()] <= countInf[v2->getIndex()]) {
          multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        } else {
          multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        }
      } else {
        multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs,
                     cutCoefo);
      }
    }
    if (variant_ == 8) {
      if (countInf[v1->getIndex()] < countInf[v2->getIndex()]) {
        multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs,
                     cutCoefo);
      } else if (countInf[v1->getIndex()] == countInf[v2->getIndex()]) {
        if (c1 <= c2) {
          multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        } else {
          multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        }
      } else {
        multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs,
                     cutCoefo);
      }
    }
  }

  if (lf) {
    for (VariableGroupConstIterator lit = lf->termsBegin();
         lit != lf->termsEnd(); ++lit) {
      v1 = rel->getRelaxationVar(lit->first);
      coef = lit->second;
      multiplyCNB_(v1->getIndex(), coef, cutCoefo);
    }
  }
}

void SimplexQuadCutGen::getQuadraticBNB(ConstraintPtr c, const double *x,
                                        RelaxationPtr rel, SubstQuadPtr &oxo,
                                        SubstQuadPtr &oxs, double *cutCoefo,
                                        double &cutConst, int &count) {
  VariablePtr v1, v2;
  double coef;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;

  qf = c->getFunction()->getQuadraticFunction();
  lf = c->getFunction()->getLinearFunction();
  cutConst = 0.0;

  assert(qf);

  // We assume the constraint here is of the form y = x1x2 or y = x1^2
  v1 = rel->getRelaxationVar(qf->begin()->first.first);
  v2 = rel->getRelaxationVar(qf->begin()->first.second);
  coef = qf->begin()->second;
  if (v1->getIndex() == v2->getIndex()) {
    ++count;
  }
  if (count == 1) {
    // Multiply v1*NB2
    multiplyBNB_(v1->getIndex(), v2->getIndex(), coef, x, oxo, oxs, cutCoefo);
  } else {
    // Multiply NB1*v2
    multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs, cutCoefo);
  }

  if (lf) {
    for (VariableGroupConstIterator lit = lf->termsBegin();
         lit != lf->termsEnd(); ++lit) {
      v1 = rel->getRelaxationVar(lit->first);
      coef = lit->second;
      multiplyCNB_(v1->getIndex(), coef, cutCoefo);
    }
  }
}

void SimplexQuadCutGen::getQuadratic(ConstraintPtr c, const double *x,
                                     RelaxationPtr rel, SubstQuadPtr &oxo,
                                     SubstQuadPtr &oxs, SubstQuadPtr &sxs,
                                     double *cutCoefo, double *cutCoefs,
                                     double &cutConst) {
  VariablePtr v1, v2;
  double coef;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;

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
}

void SimplexQuadCutGen::addCutsToRel_(SimplexCutVector cuts, RelaxationPtr rel,
                                      const double *x, int &ncuts) {
  SimplexCutPtr cut;
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr c;
  double minelem;

  calcDepth_(cuts, x);
  if (variant_ == 3) {
    cuts = getTopCuts_(cuts, 1);
    ncuts = cuts.size();
  }

  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    // both lower and upper bounds must not be finite
    assert(!(cut->lb > -INFINITY && cut->ub < INFINITY));
    if (cut->depth < minDepth_) {
#if SPEW
      env_->getLogger()->msgStream(LogDebug)
          << me_ << " Cut not added since the depth of cut is very small"
          << std::endl;
#endif
      delete cut;
      --ncuts;
      continue;
    }
    lf = (LinearFunctionPtr) new LinearFunction(cut->coef, rel->varsBegin(),
                                                rel->varsEnd(), 1e-9);
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "Depth of cut = " << std::fixed << std::setprecision(6)
        << cut->depth << std::endl;
    if (cut->lb > -INFINITY) {
      if (fabs(cut->lb) > 1e-3) {
        minelem = getMin_(cut, fabs(cut->lb));
        lf->multiply(1 / minelem);
        cut->lb = cut->lb / minelem;
      }
    } else {
      if (fabs(cut->ub) > 1e-3) {
        minelem = getMin_(cut, fabs(cut->ub));
        lf->multiply(1 / minelem);
        cut->ub = cut->ub / minelem;
      }
    }
    f = (FunctionPtr) new Function(lf);
    c = rel->newConstraint(f, cut->lb, cut->ub);
#if SPEW
    env_->getLogger()->msgStream(LogDebug)
        << me_ << " added new cut. Depth of cut = " << cut->depth << std::endl;
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
    cut->coef[v->getIndex()] += coef;
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst;
    }
  }
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, int slackInd,
                                   double coef, double cnst) {
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
      cut->coef[itrow->first] -= itrow->second;
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

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, VariablePtr v1,
                                   VariablePtr v2, double coef1, double coef2,
                                   double cnst) {
  SimplexCutPtr cut;
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cut->coef[v1->getIndex()] += coef1;
    cut->coef[v2->getIndex()] += coef2;
    if (cut->lb > -INFINITY) {
      cut->lb -= cnst;
    }
    if (cut->ub < INFINITY) {
      cut->ub -= cnst;
    }
  }
}

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, int slackInd1,
                                   int slackInd2, double coef1, double coef2,
                                   double cnst) {
  SimplexCutPtr cut;
  std::map<int, double> row1, row2;
  double rhs1 = 0.0, rhs2 = 0.0;

  slackSubstitute_(slackInd1, coef1, row1, rhs1);
  slackSubstitute_(slackInd2, coef2, row2, rhs2);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    for (std::map<int, double>::iterator itrow = row1.begin();
         itrow != row1.end(); ++itrow) {
      cut->coef[itrow->first] -= itrow->second;
    }
    for (std::map<int, double>::iterator itrow = row2.begin();
         itrow != row2.end(); ++itrow) {
      cut->coef[itrow->first] -= itrow->second;
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

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector cuts, VariablePtr orig_v,
                                   int slackInd, double coef1, double coef2,
                                   double cnst) {
  SimplexCutPtr cut;
  std::map<int, double> row;
  double rhs = 0.0;

  slackSubstitute_(slackInd, coef2, row, rhs);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cut->coef[orig_v->getIndex()] += coef1;
    for (std::map<int, double>::iterator itrow = row.begin();
         itrow != row.end(); ++itrow) {
      cut->coef[itrow->first] -= itrow->second;
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
    cut->coef[v1->getIndex()] += c1v1;
    cut->coef[v2->getIndex()] += c1v2;
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

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector &cuts, VariablePtr orig_v,
                                   int slackInd, double c1v1, double c1v2,
                                   double c2v1, double c2v2, double cnst1,
                                   double cnst2) {
  SimplexCutPtr cut;
  SimplexCutVector cuts_new;
  std::map<int, double> row;
  double rhs = 0.0;

  slackSubstitute_(slackInd, c1v2, row, rhs);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cuts_new.push_back(getNewCut_(cut, orig_v, slackInd, c2v1, c2v2, cnst2));
    cut->coef[orig_v->getIndex()] += c1v1;
    for (std::map<int, double>::iterator itrow = row.begin();
         itrow != row.end(); ++itrow) {
      cut->coef[itrow->first] -= itrow->second;
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

void SimplexQuadCutGen::addLfTerm_(SimplexCutVector &cuts, int slackInd1,
                                   int slackInd2, double c1v1, double c1v2,
                                   double c2v1, double c2v2, double cnst1,
                                   double cnst2) {
  SimplexCutPtr cut;
  SimplexCutVector cuts_new;
  std::map<int, double> row1, row2;
  double rhs1 = 0.0, rhs2 = 0.0;

  slackSubstitute_(slackInd1, c1v1, row1, rhs1);
  slackSubstitute_(slackInd2, c1v2, row2, rhs2);
  for (SimplexCutVector::iterator it = cuts.begin(); it != cuts.end(); ++it) {
    cut = (*it);
    cuts_new.push_back(
        getNewCut_(cut, slackInd1, slackInd2, c2v1, c2v2, cnst2));
    for (std::map<int, double>::iterator itrow = row1.begin();
         itrow != row1.end(); ++itrow) {
      cut->coef[itrow->first] -= itrow->second;
    }
    for (std::map<int, double>::iterator itrow = row2.begin();
         itrow != row2.end(); ++itrow) {
      cut->coef[itrow->first] -= itrow->second;
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

void SimplexQuadCutGen::calcDepth_(SimplexCutVector cuts, const double *x) {
  double norm = 0;
  SimplexCutPtr cut;

  for (SimplexCutVector::iterator vit = cuts.begin(); vit != cuts.end();
       ++vit) {
    cut = (*vit);
    norm = sqrt(InnerProduct(cut->coef, cut->coef, tabInfo_->ncol));

    if (cut->lb > -INFINITY) {
      cut->depth =
          (cut->lb - InnerProduct(x, cut->coef, tabInfo_->ncol)) / norm;
    } else {
      cut->depth =
          (InnerProduct(x, cut->coef, tabInfo_->ncol) - cut->ub) / norm;
    }
  }
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

SimplexCutVector SimplexQuadCutGen::getCutVec_(double *cutCoefo,
                                               double *cutCoefs,
                                               double cutConst, bool under,
                                               double rhs) {
  SimplexCutVector cuts;
  SimplexCutPtr cut = (SimplexCutPtr) new SimplexCut();
  std::map<int, double> row;
  double rowrhs = 0.0;

  cut->coef = new double[tabInfo_->ncol];
  memcpy(cut->coef, cutCoefo, tabInfo_->ncol * sizeof(double));

  if (cutCoefs) {
    for (int i = 0; i < nnbSlack_; ++i) {
      if (fabs(cutCoefs[nbSlack_[i]]) < eTol_ ||
          sb_[nbSlack_[i]].second - sb_[nbSlack_[i]].first < eTol_) {
        continue;
      }
      slackSubstitute_(nbSlack_[i], cutCoefs[nbSlack_[i]], row, rowrhs);

      for (std::map<int, double>::iterator itrow = row.begin();
           itrow != row.end(); ++itrow) {
        cut->coef[itrow->first] -= itrow->second;
      }
      cutConst += rowrhs;
    }
  }

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

double SimplexQuadCutGen::getMin_(SimplexCutPtr cut, double rhs) {
  double minelem = INFINITY;
  for (int i = 0; i < tabInfo_->ncol; ++i) {
    if (fabs(cut->coef[i]) > eTol_ && fabs(cut->coef[i]) < minelem) {
      minelem = fabs(cut->coef[i]);
    }
  }
  if (rhs < minelem) {
    minelem = rhs;
  }
  return minelem;
}

SimplexCutPtr SimplexQuadCutGen::getNewCut_(SimplexCutPtr cut, VariablePtr v1,
                                            VariablePtr v2, double coef1,
                                            double coef2, double cnst) {
  SimplexCutPtr new_cut = (SimplexCutPtr) new SimplexCut();
  new_cut->coef = new double[tabInfo_->ncol];
  memcpy(new_cut->coef, cut->coef, tabInfo_->ncol * sizeof(double));
  new_cut->coef[v1->getIndex()] += coef1;
  new_cut->coef[v2->getIndex()] += coef2;
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
                                            VariablePtr orig_v, int slackInd,
                                            double coef1, double coef2,
                                            double cnst) {
  SimplexCutPtr new_cut = (SimplexCutPtr) new SimplexCut();
  std::map<int, double> row;
  double rhs = 0.0;

  slackSubstitute_(slackInd, coef2, row, rhs);
  new_cut->coef = new double[tabInfo_->ncol];
  memcpy(new_cut->coef, cut->coef, tabInfo_->ncol * sizeof(double));
  new_cut->coef[orig_v->getIndex()] += coef1;
  for (std::map<int, double>::iterator itrow = row.begin(); itrow != row.end();
       ++itrow) {
    new_cut->coef[itrow->first] -= itrow->second;
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

SimplexCutPtr SimplexQuadCutGen::getNewCut_(SimplexCutPtr cut, int slackInd1,
                                            int slackInd2, double coef1,
                                            double coef2, double cnst) {
  SimplexCutPtr new_cut = (SimplexCutPtr) new SimplexCut();
  std::map<int, double> row1, row2;
  double rhs1 = 0.0, rhs2 = 0.0;

  slackSubstitute_(slackInd1, coef1, row1, rhs1);
  slackSubstitute_(slackInd2, coef2, row2, rhs2);
  new_cut->coef = new double[tabInfo_->ncol];
  memcpy(new_cut->coef, cut->coef, tabInfo_->ncol * sizeof(double));
  for (std::map<int, double>::iterator itrow = row1.begin();
       itrow != row1.end(); ++itrow) {
    new_cut->coef[itrow->first] -= itrow->second;
  }
  for (std::map<int, double>::iterator itrow = row2.begin();
       itrow != row2.end(); ++itrow) {
    new_cut->coef[itrow->first] -= itrow->second;
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

SimplexCutVector SimplexQuadCutGen::getTopCuts_(SimplexCutVector cuts,
                                                UInt num) {
  if (cuts.size() <= num) {
    return cuts;
  }
  SimplexCutVector::iterator it = cuts.begin();
  for (UInt i = 0; i < num; ++i) {
    ++it;
  }
  std::nth_element(cuts.begin(), ++it, cuts.end(),
                   [](const SimplexCutPtr lhs, const SimplexCutPtr rhs) {
                     return (*lhs) > (*rhs);
                   });
  cuts.erase(cuts.begin() + num, cuts.end());
  return cuts;
}

void SimplexQuadCutGen::addMatrix_(SubstQuadPtr &m1, SubstQuadPtr &m2) {
  // m1 := m1 + m2
  std::vector<int>::size_type size1 = m1->ind1.size(), size2 = m2->ind1.size();
  std::vector<int>::size_type pos1 = 0, pos2 = 0;
  while (pos1 < size1 && pos2 < size2) {
    if (m1->ind1[pos1] > m2->ind1[pos2] ||
        (m1->ind1[pos1] == m2->ind1[pos2] && m1->ind2[pos1] > m2->ind2[pos2])) {
      m1->ind1.insert(m1->ind1.begin() + pos1, m2->ind1[pos2]);
      m1->ind2.insert(m1->ind2.begin() + pos1, m2->ind2[pos2]);
      m1->val.insert(m1->val.begin() + pos1, m2->val[pos2]);
      ++pos1;
      ++size1;
      ++pos2;
    } else if (m1->ind1[pos1] < m2->ind1[pos2] ||
               (m1->ind1[pos1] == m2->ind1[pos2] &&
                m1->ind2[pos1] < m2->ind2[pos2])) {
      ++pos1;
    } else {
      double addval = m1->val[pos1] + m2->val[pos2];
      if (fabs(addval) < eTol_) {
        m1->ind1.erase(m1->ind1.begin() + pos1);
        m1->ind2.erase(m1->ind2.begin() + pos1);
        m1->val.erase(m1->val.begin() + pos1);
        ++pos2;
        --size1;
      } else {
        m1->val[pos1] = addval;
        ++pos1;
        ++pos2;
      }
    }
  }
  if (pos2 < size2) {
    m1->ind1.insert(m1->ind1.end(), m2->ind1.begin() + pos2, m2->ind1.end());
    m1->ind2.insert(m1->ind2.end(), m2->ind2.begin() + pos2, m2->ind2.end());
    m1->val.insert(m1->val.end(), m2->val.begin() + pos2, m2->val.end());
  }
}

void SimplexQuadCutGen::multiplyBB_(int b1, int b2, double coef,
                                    const double *x, SubstQuadPtr &oxo,
                                    SubstQuadPtr &oxs, SubstQuadPtr &sxs,
                                    double *cutCoefo, double *cutCoefs,
                                    double &cutConst) {
  double *origRow1 = new double[tabInfo_->ncol];
  double *origRow2 = new double[tabInfo_->ncol];
  double *slackRow1 = new double[tabInfo_->nrow];
  double *slackRow2 = new double[tabInfo_->nrow];
  double beta1 = x[b1];
  double beta2 = x[b2];
  std::vector<int> nzOrig, nzSlack;
  std::vector<bool> inOrig1, inOrig2, inSlack1, inSlack2;
  SubstQuadPtr tempoxo = (SubstQuadPtr) new SubstQuad();
  SubstQuadPtr tempoxs = (SubstQuadPtr) new SubstQuad();
  SubstQuadPtr tempsxs = (SubstQuadPtr) new SubstQuad();
  int a, b;

  lpe_->getBInvARow(basicInd_[b1], origRow1, slackRow1);
  lpe_->getBInvARow(basicInd_[b2], origRow2, slackRow2);

  // We assume non-basic slack variables will be zero
  for (int i = 0; i < nnbOrig_; ++i) {
    bool in1 = fabs(origRow1[nbOrig_[i]]) > eTol_;
    bool in2 = fabs(origRow2[nbOrig_[i]]) > eTol_;
    if (in1 || in2) {
      nzOrig.push_back(nbOrig_[i]);
      inOrig1.push_back(in1);
      inOrig2.push_back(in2);
      beta1 += origRow1[nbOrig_[i]] * x[nbOrig_[i]];
      beta2 += origRow2[nbOrig_[i]] * x[nbOrig_[i]];
    }
  }

  for (int i = 0; i < nnbSlack_; ++i) {
    bool in1 = fabs(slackRow1[nbSlack_[i]]) > eTol_;
    bool in2 = fabs(slackRow2[nbSlack_[i]]) > eTol_;
    if (in1 || in2) {
      nzSlack.push_back(nbSlack_[i]);
      inSlack1.push_back(in1);
      inSlack2.push_back(in2);
    }
  }

  // We assume that the coef is always nonzero
  // oxo
  a = 0;
  for (std::vector<int>::iterator it1 = nzOrig.begin(); it1 != nzOrig.end();
       ++it1, ++a) {
    int i = *it1;
    if (inOrig1[a] && inOrig2[a]) {
      tempoxo->ind1.push_back(i);
      tempoxo->ind2.push_back(i);
      tempoxo->val.push_back(coef * origRow1[i] * origRow2[i]);
    }
    b = a + 1;
    for (std::vector<int>::iterator it2 = it1 + 1; it2 != nzOrig.end();
         ++it2, ++b) {
      int j = *it2;
      double val = 0.0;
      if (inOrig1[a] && inOrig2[b]) {
        val += coef * origRow1[i] * origRow2[j];
      }
      if (inOrig2[a] && inOrig1[b]) {
        val += coef * origRow2[i] * origRow1[j];
      }
      if (fabs(val) > eTol_) {
        tempoxo->ind1.push_back(i);
        tempoxo->ind2.push_back(j);
        tempoxo->val.push_back(val);
      }
    }
    cutCoefo[i] -= coef * origRow1[i] * beta2;
    cutCoefo[i] -= coef * origRow2[i] * beta1;
  }
  // oxs
  a = 0;
  for (std::vector<int>::iterator it1 = nzOrig.begin(); it1 != nzOrig.end();
       ++it1, ++a) {
    int i = *it1;
    b = 0;
    for (std::vector<int>::iterator it2 = nzSlack.begin(); it2 != nzSlack.end();
         ++it2, ++b) {
      int j = *it2;
      double val = 0.0;
      if (inOrig1[a] && inSlack2[b]) {
        val += coef * origRow1[i] * slackRow2[j];
      }
      if (inOrig2[a] && inSlack1[b]) {
        val += coef * origRow2[i] * slackRow1[j];
      }
      if (fabs(val) > eTol_) {
        tempoxs->ind1.push_back(i);
        tempoxs->ind2.push_back(j);
        tempoxs->val.push_back(val);
      }
    }
  }
  // sxs
  a = 0;
  for (std::vector<int>::iterator it1 = nzSlack.begin(); it1 != nzSlack.end();
       ++it1, ++a) {
    int i = *it1;
    if (inSlack1[a] && inSlack2[a]) {
      tempsxs->ind1.push_back(i);
      tempsxs->ind2.push_back(i);
      tempsxs->val.push_back(coef * slackRow1[i] * slackRow2[i]);
    }
    b = a + 1;
    for (std::vector<int>::iterator it2 = it1 + 1; it2 != nzSlack.end();
         ++it2, ++b) {
      int j = *it2;
      double val = 0.0;
      if (inSlack1[a] && inSlack2[b]) {
        val += coef * slackRow1[i] * slackRow2[j];
      }
      if (inSlack2[a] && inSlack1[b]) {
        val += coef * slackRow2[i] * slackRow1[j];
      }
      if (fabs(val) > eTol_) {
        tempsxs->ind1.push_back(i);
        tempsxs->ind2.push_back(j);
        tempsxs->val.push_back(val);
      }
    }
    cutCoefs[i] -= coef * slackRow1[i] * beta2;
    cutCoefs[i] -= coef * slackRow2[i] * beta1;
  }
  // beta1 x beta2
  cutConst += coef * beta1 * beta2;
  addMatrix_(oxo, tempoxo);
  addMatrix_(oxs, tempoxs);
  addMatrix_(sxs, tempsxs);

  nzOrig.clear();
  inOrig1.clear();
  inOrig2.clear();
  nzSlack.clear();
  inSlack1.clear();
  inSlack2.clear();
  delete[] origRow1;
  delete[] origRow2;
  delete[] slackRow1;
  delete[] slackRow2;
  delete tempoxo;
  delete tempoxs;
  delete tempsxs;
}

void SimplexQuadCutGen::multiplyBNB_(int b, int nb, double coef,
                                     const double *x, SubstQuadPtr &oxo,
                                     SubstQuadPtr &oxs, double *cutCoefo) {
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

  SubstQuadPtr tempoxo = (SubstQuadPtr) new SubstQuad();
  for (int i = 0; i < nnbOrig_; ++i) {
    elem = origRow[nbOrig_[i]];
    if (fabs(elem) > eTol_) {
      // Since nbOrig is sorted once the if condition becomes false it will be
      // false till the end of loop. Thus the tempoxo will still remain
      // sorted.
      if (nbOrig_[i] <= nb) {
        tempoxo->ind1.push_back(nbOrig_[i]);
        tempoxo->ind2.push_back(nb);
        tempoxo->val.push_back(-coef * elem);
      } else {
        tempoxo->ind2.push_back(nbOrig_[i]);
        tempoxo->ind1.push_back(nb);
        tempoxo->val.push_back(-coef * elem);
      }
    }
  }
  addMatrix_(oxo, tempoxo);
  delete tempoxo;

  SubstQuadPtr tempoxs = (SubstQuadPtr) new SubstQuad();
  for (int i = 0; i < nnbSlack_; ++i) {
    elem = slackRow[nbSlack_[i]];
    if (fabs(elem) > eTol_) {
      tempoxs->ind1.push_back(nb);
      tempoxs->ind2.push_back(nbSlack_[i]);
      tempoxs->val.push_back(-coef * elem);
    }
  }
  addMatrix_(oxs, tempoxs);
  delete tempoxs;

  cutCoefo[nb] += coef * beta;

  delete[] origRow;
  delete[] slackRow;
}

void SimplexQuadCutGen::multiplyCB_(int b, double coef, const double *x,
                                    double *cutCoefo, double *cutCoefs,
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
      cutCoefo[nbOrig_[i]] -= coef * elem;
    }
  }

  for (int i = 0; i < nnbSlack_; ++i) {
    elem = slackRow[nbSlack_[i]];
    if (fabs(elem) > eTol_) {
      cutCoefs[nbSlack_[i]] -= coef * elem;
    }
  }

  cutConst += coef * beta;

  delete[] origRow;
  delete[] slackRow;
}

void SimplexQuadCutGen::multiplyCNB_(int nb, double coef, double *cutCoefo) {
  cutCoefo[nb] += coef;
}

void SimplexQuadCutGen::multiplyNBNB_(int nb1, int nb2, double coef,
                                      SubstQuadPtr &oxo) {
  SubstQuadPtr tempoxo = (SubstQuadPtr) new SubstQuad();
  if (nb1 <= nb2) {
    tempoxo->ind1.push_back(nb1);
    tempoxo->ind2.push_back(nb2);
    tempoxo->val.push_back(coef);
  } else {
    tempoxo->ind1.push_back(nb2);
    tempoxo->ind2.push_back(nb1);
    tempoxo->val.push_back(coef);
  }
  addMatrix_(oxo, tempoxo);
  delete tempoxo;
}

int SimplexQuadCutGen::relaxBilTerm_(double coef, bool lower1, bool lower2,
                                     double l1, double u1, double l2, double u2,
                                     double w1, double w2, double &c1v1,
                                     double &c1v2, double &c2v1, double &c2v2,
                                     double &cnst1, double &cnst2, bool under) {
  int numCuts;

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
        if (variant_ == 1 || variant_ == 3) {
          c1v1 = coef * l2;
          c1v2 = coef * l1;
          cnst1 = -coef * l1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * u1;
          cnst2 = -coef * u1 * u2;
          numCuts = 2;
        } else {
          if (lower2) {
            std::swap(w1, w2);
          }
          if (variant_ == 4) {
            w1 = fabs(l1) + fabs(l2) < fabs(u1) + fabs(u2) ? 1 : 0;
            w2 = fabs(l1) + fabs(l2) < fabs(u1) + fabs(u2) ? 0 : 1;
          }
          if (variant_ == 5) {
            w1 = 0.5;
            w2 = 0.5;
          }
          c1v1 = coef * (w1 * l2 + w2 * u2);
          c1v2 = coef * (w1 * l1 + w2 * u1);
          cnst1 = -coef * (w1 * l1 * l2 + w2 * u1 * u2);
          numCuts = 1;
        }
      }
    } else {
      if (lower1 == lower2) {
        if (variant_ == 1 || variant_ == 3) {
          c1v1 = coef * l2;
          c1v2 = coef * u1;
          cnst1 = -coef * u1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * l1;
          cnst2 = -coef * l1 * u2;
          numCuts = 2;
        } else {
          if (!lower1) {
            std::swap(w1, w2);
          }
          if (variant_ == 4) {
            w1 = fabs(l1) + fabs(u2) < fabs(u1) + fabs(l2) ? 1 : 0;
            w2 = fabs(l1) + fabs(u2) < fabs(u1) + fabs(l2) ? 0 : 1;
          }
          if (variant_ == 5) {
            w1 = 0.5;
            w2 = 0.5;
          }
          c1v1 = coef * (w1 * u2 + w2 * l2);
          c1v2 = coef * (w1 * l1 + w2 * u1);
          cnst1 = -coef * (w1 * l1 * u2 + w2 * u1 * l2);
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
        if (variant_ == 1 || variant_ == 3) {
          c1v1 = coef * l2;
          c1v2 = coef * l1;
          cnst1 = -coef * l1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * u1;
          cnst2 = -coef * u1 * u2;
          numCuts = 2;
        } else {
          if (lower2) {
            std::swap(w1, w2);
          }
          if (variant_ == 4) {
            w1 = fabs(l1) + fabs(l2) < fabs(u1) + fabs(u2) ? 1 : 0;
            w2 = fabs(l1) + fabs(l2) < fabs(u1) + fabs(u2) ? 0 : 1;
          }
          if (variant_ == 5) {
            w1 = 0.5;
            w2 = 0.5;
          }
          c1v1 = coef * (w1 * l2 + w2 * u2);
          c1v2 = coef * (w1 * l1 + w2 * u1);
          cnst1 = -coef * (w1 * l1 * l2 + w2 * u1 * u2);
          numCuts = 1;
        }
      }
    } else {
      if (lower1 == lower2) {
        if (variant_ == 1 || variant_ == 3) {
          c1v1 = coef * l2;
          c1v2 = coef * u1;
          cnst1 = -coef * u1 * l2;
          c2v1 = coef * u2;
          c2v2 = coef * l1;
          cnst2 = -coef * l1 * u2;
          numCuts = 2;
        } else {
          if (!lower1) {
            std::swap(w1, w2);
          }
          if (variant_ == 4) {
            w1 = fabs(l1) + fabs(u2) < fabs(u1) + fabs(l2) ? 1 : 0;
            w2 = fabs(l1) + fabs(u2) < fabs(u1) + fabs(l2) ? 0 : 1;
          }
          if (variant_ == 5) {
            w1 = 0.5;
            w2 = 0.5;
          }
          c1v1 = coef * (w1 * u2 + w2 * l2);
          c1v2 = coef * (w1 * l1 + w2 * u1);
          cnst1 = -coef * (w1 * l1 * u2 + w2 * u1 * l2);
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

int SimplexQuadCutGen::relaxTermBNB_(double coef, bool lower, double bl,
                                     double bu, double nl, double nu,
                                     double &cb, double &cn, double &cnst,
                                     bool under) {
  if (under) {
    if (coef > eTol_) {
      if (lower) {
        cb = coef * nl;
        cn = coef * bl;
        cnst = -coef * bl * nl;
      } else {
        cb = coef * nu;
        cn = coef * bu;
        cnst = -coef * bu * nu;
      }
    } else {
      if (lower) {
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
    if (coef < eTol_) {
      if (lower) {
        cb = coef * nl;
        cn = coef * bl;
        cnst = -coef * bl * nl;
      } else {
        cb = coef * nu;
        cn = coef * bu;
        cnst = -coef * bu * nu;
      }
    } else {
      if (lower) {
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

int SimplexQuadCutGen::relaxQuadTermsBNB_(SimplexCutVector &iter_cuts,
                                          RelaxationPtr rel, SubstQuadPtr oxo,
                                          SubstQuadPtr oxs, double *cutCoefo,
                                          double cutConst, ConstSolutionPtr sol,
                                          bool under, double rhs) {
  // Whenever this function is called only one cut must be generated. This is
  // unlike its counterpart for other variants below.
  double *cutCoefs = 0;
  SimplexCutVector cuts = getCutVec_(cutCoefo, cutCoefs, cutConst, under, rhs);
  bool lower;
  VariablePtr b, nb;
  int ncuts;
  const double *x = sol->getPrimal();
  double cb, cn, cnst;
  std::vector<int>::size_type oxosize = oxo->ind1.size(),
                              oxssize = oxs->ind1.size();

  for (std::vector<int>::size_type i = 0; i < oxosize; ++i) {
    if (fabs(oxo->val[i]) < eTol_) {
      continue;
    }
    if (std::find(nbOrig_.begin(), nbOrig_.end(), oxo->ind1[i]) !=
        nbOrig_.end()) {
      nb = rel->getVariable(oxo->ind1[i]);
      b = rel->getVariable(oxo->ind2[i]);
    } else {
      b = rel->getVariable(oxo->ind1[i]);
      nb = rel->getVariable(oxo->ind2[i]);
    }
    lower = fabs(x[nb->getIndex()] - nb->getLb()) < eTol_ ? true : false;
    ncuts = relaxTermBNB_(oxo->val[i], lower, b->getLb(), b->getUb(),
                          nb->getLb(), nb->getUb(), cb, cn, cnst, under);
    addLfTerm_(cuts, b, nb, cb, cn, cnst);
  }

  for (std::vector<int>::size_type i = 0; i < oxssize; ++i) {
    if (fabs(oxs->val[i]) < eTol_) {
      continue;
    }
    b = rel->getVariable(oxs->ind1[i]);
    double sub = sb_[oxs->ind2[i]].second;
    lower = sub > eTol_ ? true : false;
    ncuts = relaxTermBNB_(oxs->val[i], lower, b->getLb(), b->getUb(),
                          sb_[oxs->ind2[i]].first, sub, cb, cn, cnst, under);
    addLfTerm_(cuts, b, oxs->ind2[i], cb, cn, cnst);
  }
  iter_cuts.insert(iter_cuts.end(), cuts.begin(), cuts.end());
  return cuts.size();
}

int SimplexQuadCutGen::relaxQuadTerms_(SimplexCutVector &iter_cuts,
                                       RelaxationPtr rel, SubstQuadPtr oxo,
                                       SubstQuadPtr oxs, SubstQuadPtr sxs,
                                       double *cutCoefo, double *cutCoefs,
                                       double cutConst, ConstSolutionPtr sol,
                                       bool under, double rhs) {
  SimplexCutVector cuts = getCutVec_(cutCoefo, cutCoefs, cutConst, under, rhs);
  double c1v1 = 0.0, c1v2 = 0.0, c2v1 = 0.0, c2v2 = 0.0;
  double cnst1 = 0.0, cnst2 = 0.0;
  bool lower1, lower2;
  VariablePtr v1, v2;
  int ncuts;
  double w1 = 0.0, w2 = 0.0;
  const double *x = sol->getPrimal();
  const double *dualVars = sol->getDualOfVars();
  const double *dualCons = sol->getDualOfCons();
  std::vector<int>::size_type oxosize = oxo->ind1.size(),
                              oxssize = oxs->ind1.size(),
                              sxssize = sxs->ind1.size();

  // For oxo. Assume that all original variables are at its bounds since
  // they are non-basic
  for (std::vector<int>::size_type i = 0; i < oxosize; ++i) {
    if (fabs(oxo->val[i]) < eTol_) {
      continue;
    }
    if (oxo->ind1[i] == oxo->ind2[i]) {
      // Original square term
      v1 = rel->getVariable(oxo->ind1[i]);
      lower1 = fabs(x[oxo->ind1[i]] - v1->getLb()) < eTol_ ? true : false;
      relaxSqTerm_(oxo->val[i], lower1, v1->getLb(), v1->getUb(), c1v1, cnst1,
                   under);
      addLfTerm_(cuts, v1, c1v1, cnst1);
    } else {
      // Original bilinear term
      v1 = rel->getVariable(oxo->ind1[i]);
      v2 = rel->getVariable(oxo->ind2[i]);
      lower1 = fabs(x[oxo->ind1[i]] - v1->getLb()) < eTol_ ? true : false;
      lower2 = fabs(x[oxo->ind2[i]] - v2->getLb()) < eTol_ ? true : false;
      if (variant_ == 2) {
        if (fabs(dualVars[oxo->ind1[i]]) + fabs(dualVars[oxo->ind2[i]]) <
            eTol_) {
          w1 = 0.5;
          w2 = 0.5;
        } else {
          w1 = fabs(dualVars[oxo->ind1[i]]) /
               (fabs(dualVars[oxo->ind1[i]]) + fabs(dualVars[oxo->ind2[i]]));
          w2 = fabs(dualVars[oxo->ind2[i]]) /
               (fabs(dualVars[oxo->ind1[i]]) + fabs(dualVars[oxo->ind2[i]]));
        }
      }
      ncuts = relaxBilTerm_(oxo->val[i], lower1, lower2, v1->getLb(),
                            v1->getUb(), v2->getLb(), v2->getUb(), w1, w2, c1v1,
                            c1v2, c2v1, c2v2, cnst1, cnst2, under);
      if (ncuts == 1) {
        addLfTerm_(cuts, v1, v2, c1v1, c1v2, cnst1);
      } else {
        addLfTerm_(cuts, v1, v2, c1v1, c1v2, c2v1, c2v2, cnst1, cnst2);
      }
    }
  }

  // For oxs. Assume non-basic slack variable is always at zero. That is the
  // corresponding constraint is binding
  for (std::vector<int>::size_type i = 0; i < oxssize; ++i) {
    if (fabs(oxs->val[i]) < eTol_) {
      continue;
    }
    double slb = sb_[oxs->ind2[i]].first;
    double sub = sb_[oxs->ind2[i]].second;
    if (sub - slb < eTol_) {
      continue;
    }
    v1 = rel->getVariable(oxs->ind1[i]);
    lower1 = fabs(x[oxs->ind1[i]] - v1->getLb()) < eTol_ ? true : false;
    lower2 = sub > eTol_ ? true : false;
    if (variant_ == 2) {
      if (fabs(dualVars[oxs->ind1[i]]) + fabs(dualCons[oxs->ind2[i]]) < eTol_) {
        w1 = 0.5;
        w2 = 0.5;
      } else {
        w1 = fabs(dualVars[oxs->ind1[i]]) /
             (fabs(dualVars[oxs->ind1[i]]) + fabs(dualCons[oxs->ind2[i]]));
        w2 = fabs(dualCons[oxs->ind2[i]]) /
             (fabs(dualVars[oxs->ind1[i]]) + fabs(dualCons[oxs->ind2[i]]));
      }
    }
    ncuts = relaxBilTerm_(oxs->val[i], lower1, lower2, v1->getLb(), v1->getUb(),
                          slb, sub, w1, w2, c1v1, c1v2, c2v1, c2v2, cnst1,
                          cnst2, under);
    if (ncuts == 1) {
      addLfTerm_(cuts, v1, oxs->ind2[i], c1v1, c1v2, cnst1);
    } else {
      addLfTerm_(cuts, v1, oxs->ind2[i], c1v1, c1v2, c2v1, c2v2, cnst1, cnst2);
    }
  }

  // For sxs.
  for (std::vector<int>::size_type i = 0; i < sxssize; ++i) {
    if (fabs(sxs->val[i]) < eTol_) {
      continue;
    }
    double slb1 = sb_[sxs->ind1[i]].first;
    double sub1 = sb_[sxs->ind1[i]].second;
    double slb2 = sb_[sxs->ind2[i]].first;
    double sub2 = sb_[sxs->ind2[i]].second;
    if (sub1 - slb1 < eTol_) {
      continue;
    }
    if (sub2 - slb2 < eTol_) {
      continue;
    }
    if (sxs->ind1[i] == sxs->ind2[i]) {
      // Slack square term
      lower1 = sub1 > eTol_ ? true : false;
      relaxSqTerm_(sxs->val[i], lower1, slb1, sub1, c1v1, cnst1, under);
      addLfTerm_(cuts, sxs->ind1[i], c1v1, cnst1);
    } else {
      // Slack bilinear term
      lower1 = sub1 > eTol_ ? true : false;
      lower2 = sub2 > eTol_ ? true : false;
      if (variant_ == 2) {
        if (fabs(dualCons[sxs->ind1[i]]) + fabs(dualCons[sxs->ind2[i]]) <
            eTol_) {
          w1 = 0.5;
          w2 = 0.5;
        } else {
          w1 = fabs(dualCons[sxs->ind1[i]]) /
               (fabs(dualCons[sxs->ind1[i]]) + fabs(dualCons[sxs->ind2[i]]));
          w2 = fabs(dualCons[sxs->ind2[i]]) /
               (fabs(dualCons[sxs->ind1[i]]) + fabs(dualCons[sxs->ind2[i]]));
        }
      }
      ncuts =
          relaxBilTerm_(sxs->val[i], lower1, lower2, slb1, sub1, slb2, sub2, w1,
                        w2, c1v1, c1v2, c2v1, c2v2, cnst1, cnst2, under);
      if (ncuts == 1) {
        addLfTerm_(cuts, sxs->ind1[i], sxs->ind2[i], c1v1, c1v2, cnst1);
      } else {
        addLfTerm_(cuts, sxs->ind1[i], sxs->ind2[i], c1v1, c1v2, c2v1, c2v2,
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
  row.clear();
  for (int j = tabInfo_->rowStart[slackInd]; j < upto; ++j) {
    row.insert(std::make_pair(tabInfo_->indices[j], coef*tabInfo_->origTab[j]));
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
