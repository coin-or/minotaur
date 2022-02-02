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
}

SimplexQuadCutGen::~SimplexQuadCutGen() {}

CutVector SimplexQuadCutGen::generateCuts(RelaxationPtr rel, const double *x) {
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

  lpe_->enableFactorization();
  sortVariables_();
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
      logger_->msgStream(LogDebug)
          << me_ << "Constraint " << c->getName()
          << " is not defined at this point." << std::endl;
#endif
    }
    qf = c->getFunction()->getQuadraticFunction();
    lf = c->getFunction()->getLinearFunction();
    cutConst = 0.0;

    for (VariablePairGroupConstIterator qit = qf->begin(); qit != qf->end();
         ++qit) {
      v1 = rel->getRelaxationVar(qit->first.first);
      v2 = rel->getRelaxationVar(qit->first.second);
      coef = qit->second;
      if (basicInd_.find(v1->getIndex()) != basicInd_.end()) {
        // v1 is basic
        if (basicInd_.find(v2->getIndex()) != basicInd_.end()) {
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
        if (basicInd_.find(v2->getIndex()) != basicInd_.end()) {
          // v2 is basic
          multiplyBNB_(v2->getIndex(), v1->getIndex(), coef, x, oxo, oxs,
                       cutCoefo);
        } else {
          // v2 is non-basic
          multiplyNBNB_(v1->getIndex(), v2->getIndex(), coef, oxo);
        }
      }
    }

    for (VariableGroupConstIterator lit = lf->termsBegin();
         lit != lf->termsEnd(); ++lit) {
      v1 = rel->getRelaxationVar(lit->first);
      coef = lit->second;
      if (basicInd_.find(v1->getIndex()) != basicInd_.end()) {
        // v1 is basic
        multiplyCB_(v1->getIndex(), coef, x, cutCoefo, cutCoefs, cutConst);
      } else {
        // v1 is non-basic
        multiplyCNB_(v1->getIndex(), coef, cutCoefo);
      }
    }

    relaxQuadTerms_(oxo, oxs, sxs, cutCoefo, cutCoefs, cutConst);
  }
}

void SimplexQuadCutGen::addTerm_(std::map<int, double> &t, int v, double coef) {
  if (fabs(coef) > eTol_) {
    std::map<int, double>::iterator it = t->find(v);
    if (it == t->end()) {
      t.insert(std::make_pair(v, coef));
    } else {
      double nv = it->second + coef;
      if (fabs(nv) < eTol_) {
        t->erase(vp);
      } else {
        it->second = nv;
      }
    }
  }
}

void SimplexQuadCutGen::addTerm_(QuadTerm &t, VarProd vp, double coef) {
  if (fabs(coef) > eTol_) {
    QuadTerm::iterator it = t->find(vp);
    if (it == t->end()) {
      t.insert(std::make_pair(vp, coef));
    } else {
      double nv = it->second + coef;
      if (fabs(nv) < eTol_) {
        t->erase(vp);
      } else {
        it->second = nv;
      }
    }
  }
}

void SimplexQuadCutGen::sortVariables_(RelaxationPtr rel) {
  int *basicVars = new int[tabInfo_.nrow];

  lpe_->enableFactorization();
  lpe_->getBasics(basicVars);

  for (int i = 0; i < tabInfo_.nrow; ++i) {
    if (basicVars[i] < tabInfo_.ncol) {
      basicInd_.insert(basicVars[i], i);
    }
  }

  nnbOrig_ = 0;
  for (int i = 0; i < tabInfo_.ncol; ++i) {
    if (std::find(std::begin(basicVars), std::end(basicVars), i) ==
        std::end(basicVars)) {
      nbOrig_[nnbOrig_] = i;
      ++nnbOrig_;
    }
  }

  nnbSlack_ = 0;
  for (int i = 0; i < tabInfo_.nrow; ++i) {
    if (std::find(std::begin(basicVars), std::end(basicVars),
                  i + tabInfo_.ncol) == std::end(basicVars)) {
      nbSlack_[nnbSlack_] = i;
      ++nnbSlack_;
    }
  }

  delete[] basicVars;
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

void SimplexQuadCutGen::multiplyBB_(int b1, int b2, double coef,
                                    const double *x, QuadTerm &oxo,
                                    QuadTerm &oxs, QuadTerm &sxs,
                                    std::map<int, double> &cutCoefo,
                                    std::map<int, double> &cutCoefs,
                                    double &cutConst) {
  double *origRow1 = new double[tabInfo_.ncol];
  double *origRow2 = new double[tabInfo_.ncol];
  double *slackRow1 = new double[tabInfo_.nrow];
  double *slackRow2 = new double[tabInfo_.nrow];
  double beta1 = x[b1];
  double beta2 = x[b2];
  double elem1, elem2;

  lpe_->getBInvARow(basicInd_[b1], origRow1, slackRow1);
  lpe_->getBInvARow(basicInd_[b2], origRow2, slackRow2);

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

      addTerm_(cutCoefo, nbOrig_[i], -coef * beta2);
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
            addTerm_(sxs, std::make_pair(nbOrig_[i], nbOrig_[j]),
                     coef * elem1 * elem2);
          } else {
            addTerm_(sxs, std::make_pair(nbOrig_[j], nbOrig_[i]),
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

  cutConst += beta1 * beta2;

  delete[] origRow1;
  delete[] origRow2;
  delete[] slackRow1;
  delete[] slackRow2;
}

void SimplexQuadCutGen::multiplyBNB_(int b, int nb, double coef,
                                     const double *x, QuadTerm &oxo,
                                     QuadTerm &oxs,
                                     std::map<int, double> &cutCoefo) {
  double *origRow = new double[tabInfo_.ncol];
  double *slackRow = new double[tabInfo_.nrow];
  double elem;
  double beta = x[b];

  lpe_->getBInvARow(basicInd_[b], origRow, slackRow);

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
    elem = origRow[nbSlack_[i]];
    if (fabs(elem) > eTol_) {
      addTerm_(oxs, std::make_pair(nb, nbSlack_[i]), -coef * elem);
    }
  }

  addTerm_(cutCoefo, nb, coef * beta);

  delete[] origRow;
  delete[] slackRow;
}

void SimplexQuadCutGen::multiplyNBNB_(int nb1, int nb2, double coef,
                                      QuadTerm &oxo) {
  if (nb1 <= nb2) {
    addTerm_(oxo, std::make_pair(nb1, nb2), coef);
  } else {
    addTerm_(oxo, std::make_pair(nb2, nb1), coef);
  }
}

void SimplexQuadCutGen::multiplyCB_(int b, double coef, const double *x,
                                    std::map<int, double> &cutCoefo,
                                    std::map<int, double> &cutCoefs,
                                    double &cutConst) {
  double *origRow = new double[tabInfo_.ncol];
  double *slackRow = new double[tabInfo_.nrow];
  double elem;
  double beta = x[b];

  lpe_->getBInvARow(basicInd_[b], origRow, slackRow);

  for (int i = 0; i < nnbOrig_; ++i) {
    elem = origRow[nbOrig_[i]];
    if (fabs(elem) > eTol_) {
      addTerm_(cutCoefo, nbOrig_[i], -coef * elem);
    }
  }

  for (int i = 0; i < nnbSlack_; ++i) {
    elem = origRow[nbSlack_[i]];
    if (fabs(elem) > eTol_) {
      addTerm_(cutCoefs, nbSlack_[i], -coef * elem);
    }
  }

  cutConst += elem * beta;

  delete[] origRow;
  delete[] slackRow;
}

void SimplexQuadCutGen::multiplyCNB_(int nb, double coef,
                                     std::map<int, double> &cutCoefo) {
  addTerm_(cutCoefo, nb, coef);
}
