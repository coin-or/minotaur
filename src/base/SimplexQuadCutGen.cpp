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

#include <array>
#include <iterator>

#include "Constraint.h"
#include "Cut.h"
#include "Environment.h"
#include "LPEngine.h"
#include "LinearFunction.h"
#include "Problem.h"
#include "QuadraticFunction.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

SimplexQuadCutGen::SimplexQuadCutGen() {}

SimplexQuadCutGen::SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe)
    : env_(env), eTol_(1e-6), p_(p), lpe_(lpe) {
  ncuts_ = 0;
  findQuadCons_();  // required??
}

SimplexQuadCutGen::~SimplexQuadGen() {}

CutVector SimplexQuadCutGen::generateCuts(RelaxationPtr rel, const double *x) {
  UInt nvars = rel->getNumVars(), ncons = rel->getNumCons();
  int *basic = new int[ncons];
  ConstraintPtr c;
  FunctionType ftype;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;
  VariablePtr v1, v2;
  double coef, val1, val2, act;
  auto b1, b2;
  double *quad = new double[(nvars * (nvars + 1)) / 2];
  double *lin = new double[nvars];
  double cnst;
  int error = 0;
  std::map<UInt, UInt> nb_index;

  lpe_->enableFactorization();
  lpe_->getBasics(basic);

  fillNonBasicIndex_(rel, basic, nb_index);  // required??

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
       ++cit) {
    c = *cit;
    ftype = c->getFunction()->getType();
    act = c->getActivity(x, &error);
    if (error == 0) {
      if (act <= c->getUb() + eTol_ && act >= c->getLb() - eTol_) {
        continue;
      }
    } else {
#if SPEW
      logger_->msgStream(LogDebug)
          << me_ << "Constraint " << c->getName()
          << " is not defined at this point." << std::endl;
#endif
    }
    // Linear constraints should be satisfied thus no cuts will be generated
    assert(ftype == Quadratic || ftype == Bilinear);
    qf = c->getFunction()->getQuadraticFunction();
    lf = c->getFunction()->getLinearFunction();
    cnst = 0.0;
    {
      UInt quad_ind = 0;
      for (UInt i = 0; i < nvars; ++i) {
        lin[i] = 0.0;
        for (UInt j = 0; j <= i; ++j) {
          quad[quad_ind++] = 0.0;
        }
      }
    }
    // get expression for qf
    for (VariablePairGroupConstIterator qit = qf->begin(); qit != qf->end();
         ++qit) {
      v1 = rel->getRelaxationVar(qit->first.first);
      v2 = rel->getRelaxationVar(qit->first.second);
      coef = qit->second;
      b1 = std::find(std::begin(basic), std::end(basic),
                     static_cast<int>(v1->getIndex()));
      b2 = std::find(std::begin(basic), std::end(basic),
                     static_cast<int>(v2->getIndex()));
      if (b1 != std::end(basic)) {
        val1 = x[v1->getIndex()];
        if (b2 != std::end(basic)) {
          val2 = x[v2->getIndex()];
          getExpression_(std::distance(basic, b1), std::distance(basic, b2),
                         val1, val2, nb_index, coef, quad, lin, &cnst, true,
                         true);
        } else {
          val2 = 0.0;
          getExpression_(std::distance(basic, b1), v2->getIndex(), val1, val2,
                         nb_index, coef, quad, lin, &cnst, true, false);
        }
      } else {
        val1 = 0.0;
        if (b2 != std::end(basic)) {
          val2 = x[v2->getIndex()];
          getExpression_(v1->getIndex(), std::distance(basic, b2), val1, val2,
                         nb_index, coef, quad, lin, &cnst, false, true);
        } else {
          val2 = 0.0;
          getExpression_(v1->getIndex(), v2->getIndex(), val1, val2, nb_index,
                         coef, quad, lin, &cnst, false, false);
        }
      }
    }

    // get expession for lf
    for (VariableGroupConstIterator lit = lf->termsBegin();
         lit != lf->termsEnd(); ++lit) {
      v1 = rel->getRelaxationVar(lit->first);
      coef = lit->second;
      val1 = x[v1->getIndex()];
      b1 = std::find(std::begin(basic), std::end(basic),
                     static_cast<int>(v1->getIndex()));
      if (b1 != std::end(basic)) {
        getExpression_(std::distance(basic, b1), val1, nb_index, coef, lin,
                       &cnst, true);
      } else {
        getExpression_(v1->getIndex(), val1, nb_index, coef, lin, &cnst, false);
      }
    }

    if (act > c->getUb() + eTol_) {
      // get underestimating cuts for the constraint
      getCutsForUb_(quad, lin, cnst, c->getUb());
    }
    if (act < c->getLb() - eTol_) {
      // get overestimating cuts for the constraint
      getCutsForLb_(quad, lin, cnst, c->getLb());
    }
  }
}

void SimplexQuadCutGen::fillNonBasicIndex_(RelaxationPtr rel, int &basic,
                                           std::map<UInt, UInt> &nb_index) {
  VariablePtr v;
  auto b;
  UInt index = 0;
  int j = rel->getNumVars();
  for (VariableConstIterator vit = rel->varsBegin(); vit != rel->varsEnd();
       ++vit) {
    v = *vit;
    b = std::find(std::begin(basic), std::end(basic),
                  static_cast<int>(v->getIndex()));
    if (b1 == std::end(basic)) {
      nb_index.insert(std::pair<UInt, UInt>(v1->getIndex(), index++));
    }
  }

  for (ConstraintConstIterator cit = rel->consBegin(); cit != rel->consEnd();
       ++cit) {
    c = *cit;
    b1 = std::find(std::begin(basic), std::end(basic), j++);
    if (b1 == std::end(basic)) {
      nb_index.insert(std::pair<UInt, UInt>(j, index++));
    }
  }
}

void SimplexQuadGen::sortVariables_(RelaxationPtr rel) {
  int *basicVars = new int[tabInfo_.nrow];

  lpe_->enableFactorization();
  lpe_->getBasics(basicVars);

  for (int i = 0; i < tabInfo_.nrow; ++i) {
    if (basicVars[i] < tabInfo_.ncol) {
      basicInd_.insert(basicVars[i], i);
    }
  }

  int j = 0;
  for (int i = 0; i < tabInfo_.ncol; ++i) {
    if (std::find(std::begin(basicVars), std::end(basicVars), i) ==
        std::end(basicVars)) {
      nbOrig_[j] = i;
      ++j;
    }
  }

  j = 0;
  for (int i = 0; i < tabInfo_.nrow; ++i) {
    if (std::find(std::begin(basicVars), std::end(basicVars),
                  i + tabInfo_.ncol) == std::end(basicVars)) {
      nbSlack_[j] = i;
      ++j;
    }
  }
}

void SimplexQuadCutGen::findQuadCons_() {
  ConstraintPtr c;
  FunctionPtr f;
  FunctionType ftype;

  for (ConstraintConstIterator cit = p_->consBegin(); cit != p_->consEnd();
       ++cit) {
    c = *cit;
    f = c->getFunction();
    ftype = f->getType();
    if (ftype == Quadratic || ftype == Bilinear) {
      qcons_.push_back(c);
    }
  }
}

void SimplexQuadCutGen::getBasicInfo_() {
  if (!lpe_->IsOptimalBasisAvailable()) {
    logger_->msgStream(LogDebug)
        << me_ << "Solver provided no Optimal Basis" << std::endl;
  }
  tabInfo_.ncol = lpe_->getNumCols();
  tabInfo_.nrow = lpe_->getNumRows();
  tabInfo_.colLower = lpe_->getColLower();
  tabInfo_.colUpper = lpe_->getColUpper();
  tabInfo_.rowLower = lpe_->getRowLower();
  tabInfo_.rowUpper = lpe_->getRowUpper();
  // tabInfo_.rowRhs = lpe_-.>getRightHandSide();
  tabInfo_.rowActivity = lpe_->getRowActivity();
  tabInfo_->origTab = lpe_->getOriginalTableau();
  tabInfo_->rowStart = lpe_->getRowStarts();
  tabInfo_->indices = lpe_->getIndicesofVars();
  tabInfo_->rowLen = lpe_->getRowLength();
}

void SimplexQuadCutGen::getExpression_(UInt ind, double beta,
                                       std::map<UInt, UInt> &nb_index,
                                       double coef, double *lin, double *cnst,
                                       bool basic) {
  // This function returns an expression for a linear term
  UInt size = nb_index.size();
  double *alpha = new double[size];
  // get alpha

  if (basic) {
    for (UInt i = 0; i < size; ++i) {
      lin[i] += coef * alpha[i];
    }
    *cnst += coef * beta;
  } else {
    lin[nb_index[ind]] += coef;
  }
  delete[] alpha;
}

void SimplexQuadCutGen::getExpression_(UInt ind1, UInt ind2, double beta1,
                                       double beta2,
                                       std::map<UInt, UInt> &nb_index,
                                       double coef, double *quad, double *lin,
                                       double *cnst, bool basic1, bool basic2) {
  UInt size = nb_index.size();
  double *alpha1 = new double[size];
  double *alpha2 = new double[size];

  // get both alphas from a different function by calling the LP solver

  if (basic1) {
    if (basic2) {
      multiplyBB_(coef, alpha1, aplha2, beta1, beta2, quad, lin, cnst, size);
    } else {
      multiplyBNB_(coef, alpha1, beta1, nb_index[ind2], quad, lin, size);
    }
  } else {
    if (basic2) {
      multiplyBNB_(coef, alpha2, beta2, nb_index[ind1], quad, lin, size);
    } else {
      multiplyNBNB_(coef, nb_index[ind1], nb_index[ind2], quad);
    }
  }
  delete[] alpha1;
  delete[] alpha2;
}

void SimplexQuadCutGen::multiplyBB_(double coef, double *alpha1, double *alpha2,
                                    double beta1, double beta2, double *quad,
                                    double *lin, double *cnst, UInt size) {
  for (UInt i = 0; i < size; ++i) {
    for (UInt j = 0; j < size; ++j) {
      if (j > i) {
        quad[(j * (j + 1)) / 2 + i] += coef * alpha1[i] * alpha2[j];
      } else {
        quad[(i * (i + 1)) / 2 + j] += coef * alpha1[i] * alpha2[j];
      }
    }
    lin[i] += coef * (beta1 * alpha2[i] + beta2 * alpha1[i]);
  }
  *cnst += coef * beta1 * beta2;
}

void SimplexQuadCutGen::multiplyBNB_(double coef, double *alpha1, double beta1,
                                     UInt nb_ind, double *quad, double *lin,
                                     UInt size) {
  for (UInt i = 0; i < size; ++i) {
    if (nb_ind > i) {
      quad[(nb_ind * (nb_ind + 1)) / 2 + i] += coef * alpha1[i];
    } else {
      quad[(i * (i + 1)) / 2 + nb_ind] += coef * alpha1[i];
    }
  }
  lin[nb_ind] += coef * beta1;
}

void SimplexQuadCutGen::multiplyNBNB_(double coef, double ind1, double ind2,
                                      double *quad) {
  if (ind1 > ind2) {
    quad[(ind1 * (ind1 + 1)) / 2 + ind2] += coef;
  } else {
    quad[(ind2 * (ind2 + 1)) / 2 + ind1] += coef;
  }
}
