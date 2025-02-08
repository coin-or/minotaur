//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file OsiLPEngine.cpp
 * \brief Implement an interface to the OSI-LP solver.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#if MNTROSICLP
#include "coin/OsiClpSolverInterface.hpp"
#endif

#if MNTROSIGRB
#include "coin/OsiGrbSolverInterface.hpp"
#endif
#include "coin/CoinPackedMatrix.hpp"
#include "coin/CoinWarmStart.hpp"
#include "coin/CoinWarmStartDual.hpp"

#undef F77_FUNC_
#undef F77_FUNC

#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "HessianOfLag.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "MinotaurConfig.h"
#include "Objective.h"
#include "Option.h"
#include "OsiLPEngine.h"
#include "Problem.h"
#include "Solution.h"
#include "Timer.h"
#include "Variable.h"

using namespace Minotaur;

//#define SPEW 1

const std::string OsiLPEngine::me_ = "OsiLPEngine: ";

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

OsiLPWarmStart::OsiLPWarmStart()
  : coinWs_(0),
    mustDelete_(true)
{
}

OsiLPWarmStart::~OsiLPWarmStart()
{
  if (coinWs_ && mustDelete_) {
    delete coinWs_;
    coinWs_ = 0;
  }
}

bool OsiLPWarmStart::hasInfo()
{
  if (coinWs_) {
    return true;
  } else {
    return false;
  }
}

CoinWarmStart *OsiLPWarmStart::getCoinWarmStart() const
{
  return coinWs_;
}

void OsiLPWarmStart::setCoinWarmStart(CoinWarmStart *coin_ws,
                                      bool must_delete)
{
  if (coinWs_ && mustDelete_) {
    delete coinWs_;
    coinWs_ = 0;
  }

  coinWs_ = coin_ws;
  mustDelete_ = must_delete;
}

void OsiLPWarmStart::setDualWarmStart(int size, const double *dual)
{
  CoinWarmStart *coin_ws = (CoinWarmStart *)new CoinWarmStartDual(size, dual);
  setCoinWarmStart(coin_ws, true);
}

void OsiLPWarmStart::write(std::ostream &) const
{
  assert(!"implement me!");
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

OsiLPEngine::OsiLPEngine(EnvPtr env)
  : bndChanged_(true),
    consChanged_(true),
    env_(env),
    maxIterLimit_(10000),
    objChanged_(true),
    pickLPMeth_(true),
    problem_(0),
    sol_(0),
    strBr_(false)
{
#if USE_OSILP
#else
#error Need to set USE_OSILP
#endif
  std::string etype = env_->getOptions()->findString("lp_engine")->getValue();

  logger_ = env_->getLogger();
  pickLPMeth_ = env_->getOptions()->findBool("set_lp_method")->getValue();
  eName_ = OsiUndefEngine;
  if (etype == "OsiClp") {
    eName_ = OsiClpEngine;
  } else if (etype == "OsiGrb") {
    eName_ = OsiGrbEngine;
  }
  osilp_ = newSolver_(eName_);
  assert(osilp_);
  stats_ = new OsiLPStats();
  stats_->calls = 0;
  stats_->strCalls = 0;
  stats_->time = 0;
  stats_->strTime = 0;
  stats_->iters = 0;
  stats_->strIters = 0;

  timer_ = env->getNewTimer();

#if MNTROSICLP
  if (OsiClpEngine == eName_) {
    OsiClpSolverInterface *osiclp =
        (OsiClpSolverInterface *)(dynamic_cast<OsiClpSolverInterface *>(
            osilp_));
    osiclp->setupForRepeatedUse();
  }
#endif
}

OsiLPEngine::~OsiLPEngine()
{
  delete osilp_;
  delete stats_;
  delete timer_;
  if (problem_) {
    problem_->unsetEngine();
    problem_ = 0;
  }
  if (sol_) {
    delete sol_;
  }
}

void OsiLPEngine::addConstraint(ConstraintPtr con)
{
  LinearFunctionPtr lf = con->getLinearFunction();
  int nz = static_cast<int>(lf->getNumTerms());
  int *cols = new int[nz];
  double *elems = new double[nz];
  VariableGroupConstIterator it;
  int i = 0;

  for (it = lf->termsBegin(); it != lf->termsEnd(); ++it, ++i) {
    cols[i] = it->first->getIndex();
    elems[i] = it->second;
  }

  osilp_->addRow(nz, cols, elems, con->getLb(), con->getUb());
  // assert(!"implement me!");
  delete[] cols;
  delete[] elems;
  consChanged_ = true;
}

void OsiLPEngine::changeBound(ConstraintPtr cons, BoundType lu,
                              double new_val)
{
  if (Upper == lu) {
    osilp_->setRowUpper(cons->getIndex(), new_val);
  } else {
    osilp_->setRowLower(cons->getIndex(), new_val);
  }
  bndChanged_ = true;
}

void OsiLPEngine::changeBound(VariablePtr var, BoundType lu, double new_val)
{
  // XXX: need a better map than the following for mapping variables to
  // indices and vice versa
  int col = var->getIndex();
  switch (lu) {
  case Lower:
    osilp_->setColLower(col, new_val);
    break;
  case Upper:
    osilp_->setColUpper(col, new_val);
    break;
  default:
    break;
  }
  bndChanged_ = true;
}

void OsiLPEngine::changeBound(VariablePtr var, double new_lb, double new_ub)
{
  int col = var->getIndex();
  osilp_->setColBounds(col, new_lb, new_ub);
  bndChanged_ = true;
}

#if MNTROSICLP
void OsiLPEngine::changeConstraint(ConstraintPtr c, LinearFunctionPtr lf,
                                   double lb, double ub)
#else
void OsiLPEngine::changeConstraint(ConstraintPtr, LinearFunctionPtr, double,
                                   double)
#endif
{
  if (eName_ == OsiClpEngine) {
#if MNTROSICLP
    // OsiLPInterface does not have a modifyCoefficient function. So we have
    // to downcast it to OsiClpSolverInterface. XXX: Clean this code by
    // creating a map from constraints in problem, to those in engine. Then
    // use deleteRow and addRow, instead of modifyCoefficient().
    OsiClpSolverInterface *osiclp =
        (OsiClpSolverInterface *)(dynamic_cast<OsiClpSolverInterface *>(
            osilp_));
    int row = c->getIndex();
    ConstLinearFunctionPtr clf = c->getFunction()->getLinearFunction();

    // first zero out all the existing coefficients in the row.
    for (VariableGroupConstIterator it = clf->termsBegin();
         it != clf->termsEnd(); ++it) {
      osiclp->modifyCoefficient(row, it->first->getIndex(), 0.0);
    }

    // assign new coefficients in the row.
    for (VariableGroupConstIterator it = lf->termsBegin();
         it != lf->termsEnd(); ++it) {
      osiclp->modifyCoefficient(row, it->first->getIndex(), it->second);
    }
    osiclp->setRowUpper(row, ub);
    osiclp->setRowLower(row, lb);
    consChanged_ = true;
#endif
  } else {
    assert(!"implement me!");
  }
}

void OsiLPEngine::changeConstraint(ConstraintPtr, NonlinearFunctionPtr)
{
  assert(!"Cannot change a nonlinear function in OsiLPEngine");
}

void OsiLPEngine::changeObj(FunctionPtr f, double)
{
  LinearFunctionPtr lf = (f) ? f->getLinearFunction() : 0;
  double *obj = new double[problem_->getNumVars()];
  std::fill(obj, obj + problem_->getNumVars(), 0.0);
  if (lf) {
    for (VariableGroupConstIterator it = lf->termsBegin();
         it != lf->termsEnd(); ++it) {
      obj[it->first->getIndex()] = it->second;
    }
  }
  osilp_->setObjective(obj);
  objChanged_ = true;
  delete[] obj;
}

void OsiLPEngine::clear()
{
  if (osilp_) {
    osilp_->reset();
    osilp_->setHintParam(OsiDoReducePrint);
    osilp_->messageHandler()->setLogLevel(0);
  }
  if (problem_) {
    problem_->unsetEngine();
    // problem_.reset();
    problem_ = 0;
  }
}

// void OsiLPEngine::disableFactorization() {
//  osilp_->disableFactorization();
//}

void OsiLPEngine::disableStrBrSetup()
{
#if SPEW
  logger_->msgStream(LogDebug2)
      << me_ << "disabling strong branching." << std::endl;
#endif
  strBr_ = false;
}

EnginePtr OsiLPEngine::emptyCopy()
{
  return (OsiLPEnginePtr) new OsiLPEngine(env_);
}

void OsiLPEngine::enableStrBrSetup()
{
#if SPEW
  logger_->msgStream(LogDebug2)
      << me_ << "enabling strong branching." << std::endl;
#endif
  strBr_ = true;
}

int OsiLPEngine::getIterationCount()
{
  return osilp_->getIterationCount();
}

void OsiLPEngine::fillStats(std::vector<double> &lpStats)
{
  if (lpStats.size()) {
    lpStats[0] += stats_->calls;
    lpStats[1] += stats_->strCalls;
    lpStats[2] += stats_->time;
    lpStats[3] += stats_->strTime;
    lpStats[4] += stats_->iters;
    lpStats[5] += stats_->strIters;
  }
}

void OsiLPEngine::disableFactorization()
{
  osilp_->disableFactorization();
}

void OsiLPEngine::enableFactorization()
{
  osilp_->enableFactorization();
}

void OsiLPEngine::getBasics(int *index)
{
  osilp_->getBasics(index);
}

void OsiLPEngine::getBInvARow(int row, double *z, double *slack)
{
  osilp_->getBInvARow(row, z, slack);
}

void OsiLPEngine::getBasisStatus(int *cstat, int *rstat)
{
  osilp_->getBasisStatus(cstat, rstat);
}

const double *OsiLPEngine::getColLower()
{
  return osilp_->getColLower();
}

const double *OsiLPEngine::getColUpper()
{
  return osilp_->getColUpper();
}

const double *OsiLPEngine::getRowLower()
{
  return osilp_->getRowLower();
}

const double *OsiLPEngine::getRowUpper()
{
  return osilp_->getRowUpper();
}

const double *OsiLPEngine::getRightHandSide()
{
  return osilp_->getRightHandSide();
}

int OsiLPEngine::getNumCols()
{
  return osilp_->getNumCols();
}

int OsiLPEngine::getNumRows()
{
  return osilp_->getNumRows();
}

const double *OsiLPEngine::getRowActivity()
{
  return osilp_->getRowActivity();
}

const double *OsiLPEngine::getOriginalTableau()
{
  return osilp_->getMatrixByRow()->getElements();
}

const int *OsiLPEngine::getRowStarts()
{
  return osilp_->getMatrixByRow()->getVectorStarts();
}

const int *OsiLPEngine::getIndicesofVars()
{
  return osilp_->getMatrixByRow()->getIndices();
}

const int *OsiLPEngine::getRowLength()
{
  return osilp_->getMatrixByRow()->getVectorLengths();
}

std::string OsiLPEngine::getName() const
{
  return "OsiLP";
}

double OsiLPEngine::getSolutionValue()
{
  return sol_->getObjValue();
}

ConstSolutionPtr OsiLPEngine::getSolution()
{
  return sol_;
}

// OsiSolverInterface * OsiLPEngine::getSolver()
//{
//  return osilp_;
//}

EngineStatus OsiLPEngine::getStatus()
{
  return status_;
}

WarmStartPtr OsiLPEngine::getWarmStartCopy()
{
  // create a new copy of warm-start information from osilp_
  CoinWarmStart *coin_copy = osilp_->getWarmStart();

  OsiLPWarmStartPtr ws = new OsiLPWarmStart();
  // save it. It is our responsibility to free it.
  ws->setCoinWarmStart(coin_copy, true);

  return ws;
}

bool OsiLPEngine::IsOptimalBasisAvailable()
{
  return osilp_->optimalBasisIsAvailable();
}

void OsiLPEngine::load(ProblemPtr problem)
{
  problem_ = problem;
  int numvars = static_cast<int>(problem->getNumVars());
  int numcons = static_cast<int>(problem->getNumCons());
  int i, j;
  double obj_sense = 1.;
  CoinPackedMatrix *r_mat;
  double *conlb, *conub, *varlb, *varub, *obj;
  double *value;
  int *index;
  CoinBigIndex *start;

  ConstraintConstIterator c_iter;
  VariableConstIterator v_iter;

  conlb = new double[numcons];
  conub = new double[numcons];
  varlb = new double[numvars];
  varub = new double[numvars];

  VariableGroupConstIterator it;
  /* map the variables in this constraint to the function type (linear here)
   */

  // XXX Need to count the number of nnz in the problem
  //     -- maybe add it to class later
  LinearFunctionPtr lin;
  int nnz = 0;
  for (c_iter = problem->consBegin(); c_iter != problem->consEnd();
       ++c_iter) {
    // XXX Don't want assert here, but not sure of eventually calling sequence
    //     and assumptions
    assert((*c_iter)->getFunctionType() == Linear);
    lin = (*c_iter)->getLinearFunction();
    nnz += static_cast<int>(lin->getNumTerms());
  }

  index = new int[nnz];
  value = new double[nnz];
  start = new CoinBigIndex[numcons + 1];

  i = 0;
  j = 0;
  start[0] = 0;
  for (c_iter = problem->consBegin(); c_iter != problem->consEnd();
       ++c_iter) {
    conlb[i] = (*c_iter)->getLb();
    conub[i] = (*c_iter)->getUb();
    lin = (*c_iter)->getLinearFunction();
    for (it = lin->termsBegin(); it != lin->termsEnd(); ++it) {
      ConstVariablePtr vPtr = it->first;
      index[j] = vPtr->getIndex();
      value[j] = it->second;
      ++j;
    }
    ++i;
    start[i] = j;
  }

  i = 0;
  for (v_iter = problem->varsBegin(); v_iter != problem->varsEnd();
       ++v_iter, ++i) {
    varlb[i] = (*v_iter)->getLb();
    varub[i] = (*v_iter)->getUb();
  }

  if (problem->getObjective()) {
    lin = problem->getObjective()->getLinearFunction();
    if (problem->getObjective()->getObjectiveType() == Minotaur::Maximize) {
      obj_sense = -1.;
    }
  } else {
    lin = 0;
  }
  obj = new double[numvars];
  i = 0;
  if (lin) {
    for (v_iter = problem->varsBegin(); v_iter != problem->varsEnd();
         ++v_iter, ++i) {
      obj[i] = obj_sense * lin->getWeight(*v_iter);
    }
  } else {
    memset(obj, 0, numvars * sizeof(double));
  }

  r_mat = new CoinPackedMatrix(false, numvars, numcons, nnz, value, index,
                               start, NULL);
  osilp_->loadProblem(*r_mat, varlb, varub, obj, conlb, conub);

  if (sol_) {
    delete sol_;
    sol_ = 0;
  }
  sol_ = (SolutionPtr) new Solution(1E20, 0, problem_);

  objChanged_ = true;
  bndChanged_ = true;
  consChanged_ = true;
  delete r_mat;
  delete[] index;
  delete[] value;
  delete[] start;
  delete[] conlb;
  delete[] conub;
  delete[] varlb;
  delete[] varub;
  delete[] obj;

  // osilp_->writeLp("stub");
  // exit(0);
  problem->setEngine(this);
}

void OsiLPEngine::loadFromWarmStart(const WarmStartPtr ws)
{
  ConstOsiLPWarmStartPtr ws2 = dynamic_cast<const OsiLPWarmStart *>(ws);
  assert(ws2);
  CoinWarmStart *coin_ws = ws2->getCoinWarmStart();
  osilp_->setWarmStart(coin_ws);
}

void OsiLPEngine::loadDualWarmStart(int size, double *dualVec)
{
  OsiLPWarmStartPtr ws = (OsiLPWarmStartPtr) new OsiLPWarmStart();
  ws->setDualWarmStart(size, dualVec);
  loadFromWarmStart(ws);
  delete ws;
}

void OsiLPEngine::negateObj()
{
  UInt n = static_cast<int>(problem_->getNumVars());
  double *obj = new double[n];
  const double *old_obj = osilp_->getObjCoefficients();
  std::copy(old_obj, old_obj + n, obj);
  osilp_->setObjective(obj);
  objChanged_ = true;
  delete[] obj;
}

OsiSolverInterface *OsiLPEngine::newSolver_(OsiLPEngineName ename)
{
  OsiSolverInterface *si = 0;
  switch (ename) {
  case (OsiClpEngine):
#if MNTROSICLP
    si = new OsiClpSolverInterface();
    si->setHintParam(OsiDoReducePrint);
    si->messageHandler()->setLogLevel(0);
#else
    logger_->errStream() << me_ << "Minotaur is not compiled with OsiClp!"
                         << std::endl;
#endif
    break;
  case (OsiGrbEngine):
#if MNTROSIGRB
    si = new OsiGrbSolverInterface();
#else
    logger_->errStream() << me_ << "Minotaur is not compiled with OsiGrb!"
                         << std::endl;
#endif
    break;
  default:
    break;
  }
  return si;
}

void OsiLPEngine::removeCons(std::vector<ConstraintPtr> &delcons)
{
  int num = static_cast<int>(delcons.size());
  int *inds = new int[num];
  for (int i = 0; i < num; ++i) {
    inds[i] = delcons[i]->getIndex();
  }
  osilp_->deleteRows(num, inds);
  consChanged_ = true;
}

void OsiLPEngine::resetIterationLimit()
{
  OsiIntParam key = OsiMaxNumIteration;
  osilp_->setIntParam(key, maxIterLimit_);
}

void OsiLPEngine::setIterationLimit(int limit)
{
  OsiIntParam key = OsiMaxNumIteration;
  osilp_->setIntParam(key, limit);
}

EngineStatus OsiLPEngine::solve()
{
  double off = 0;

  if (problem_->getObjective()) {
    off = problem_->getObjective()->getConstant();
  }
  timer_->start();
  if (true == pickLPMeth_) {
    if (true == objChanged_ && false == bndChanged_ &&
        false == consChanged_) {
      osilp_->setHintParam(OsiDoDualInResolve, false);
    } else {
      osilp_->setHintParam(OsiDoDualInResolve, true);
    }
  }

  stats_->calls += 1;
#if SPEW
  logger_->msgStream(LogDebug2)
      << me_ << "in call number " << stats_->calls << std::endl;
#endif

  if (1 == stats_->calls) {
    osilp_->initialSolve();
  } else if (true == pickLPMeth_) {
    osilp_->resolve();
  } else {
    Problem *p = problem_;
    clear();
    load(p);
    osilp_->initialSolve();
  }

  if (osilp_->isProvenOptimal()) {
    status_ = ProvenOptimal;
    sol_->setPrimal(osilp_->getStrictColSolution());
    sol_->setObjValue(osilp_->getObjValue() + off);
    sol_->setDualOfCons(osilp_->getRowPrice());
    sol_->setDualOfVars(osilp_->getReducedCost());
  } else if (osilp_->isProvenPrimalInfeasible()) {
    status_ = ProvenInfeasible;
    sol_->setObjValue(INFINITY);
    sol_->setDualOfCons(osilp_->getRowPrice());
    sol_->setDualOfVars(osilp_->getReducedCost());
  } else if (osilp_->isProvenDualInfeasible()) {
    status_ = ProvenUnbounded;  // primal is not infeasible but dual is.
    sol_->setObjValue(-INFINITY);
    sol_->setDualOfCons(osilp_->getRowPrice());
    sol_->setDualOfVars(osilp_->getReducedCost());
  } else if (osilp_->isIterationLimitReached()) {
    status_ = EngineIterationLimit;
    sol_->setPrimal(osilp_->getStrictColSolution());
    sol_->setObjValue(osilp_->getObjValue() + off);
    sol_->setDualOfCons(osilp_->getRowPrice());
    sol_->setDualOfVars(osilp_->getReducedCost());
  } else if (osilp_->isAbandoned()) {
    status_ = EngineError;
    sol_->setObjValue(INFINITY);
  } else if (osilp_->isPrimalObjectiveLimitReached() ||
             osilp_->isDualObjectiveLimitReached()) {
    status_ = ProvenObjectiveCutOff;
    sol_->setPrimal(osilp_->getStrictColSolution());
    sol_->setObjValue(osilp_->getObjValue() + off);
    sol_->setDualOfCons(osilp_->getRowPrice());
    sol_->setDualOfVars(osilp_->getReducedCost());
  } else {
    status_ = EngineUnknownStatus;
    sol_->setObjValue(INFINITY);
  }

  stats_->iters += osilp_->getIterationCount();
  stats_->time += timer_->query();
  if (strBr_) {
    ++(stats_->strCalls);
    stats_->strIters += osilp_->getIterationCount();
    stats_->strTime += timer_->query();
  }

#if SPEW
  logger_->msgStream(LogDebug2)
      << me_ << "status = " << status_ << std::endl
      << me_ << "solution value = " << sol_->getObjValue() << std::endl
      << me_ << "iterations = " << osilp_->getIterationCount() << std::endl;
#endif
  timer_->stop();
  if (true == pickLPMeth_ && true == objChanged_ && false == bndChanged_ &&
      false == consChanged_) {
    osilp_->setHintParam(OsiDoDualInResolve, true);
  }
  bndChanged_ = false;
  consChanged_ = false;
  objChanged_ = false;

  return status_;
}

void OsiLPEngine::writeLP(const char *filename) const
{
  osilp_->writeLp(filename);
}

void OsiLPEngine::writeStats(std::ostream &out) const
{
  if (stats_) {
    std::string me = "OsiLP: ";
    out << me << "total calls            = " << stats_->calls << std::endl
        << me << "strong branching calls = " << stats_->strCalls << std::endl
        << me << "total time in solving  = " << stats_->time << std::endl
        << me << "time in str branching  = " << stats_->strTime << std::endl
        << me << "total iterations       = " << stats_->iters << std::endl
        << me << "strong br iterations   = " << stats_->strIters << std::endl;
  }
}
