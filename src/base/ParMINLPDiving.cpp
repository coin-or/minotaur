//
//     MINOTAUR -- It's only 1/2 bull  
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//
 
/**
 * \file ParMINLPDiving.cpp
 * \brief Define the ParMINLPDiving class for diving heuristics for MINLPs.
 * This is a parallel extension of MINLPDiving heuristic in MINOTAUR.
 * \author Prashant Palkar, IIT Bombay
 *
 * Implements the class ParMINLPDiving.
 */

#include <cmath> // for INFINITY
#include <iomanip>
#if USE_OPENMP
#include <omp.h>
#endif

#include "MinotaurConfig.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "Function.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "ParMINLPDiving.h"
#include "Node.h"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "VarBoundMod.h"
#include "Variable.h"

using namespace Minotaur;

#define SPEW 1

const std::string ParMINLPDiving::me_ = "ParMINLP Diving Heuristic: "; 

ParMINLPDiving::ParMINLPDiving(EnvPtr env, ProblemPtr p, EnginePtr e)
: e_(e), 
  env_(env), 
  //gradientObj_(NULL),
  intTol_(1e-5),
  //lh_(0),
  maxProbs_(env->getOptions()->findInt("divheurMaxProbs")->getValue()),
  maxSol_(2), 
  nSelector_(4),
  p_(p), 
  stats_(NULL), 
  timer_(env_->getNewTimer())
{
  //for (UInt i=0; i<p_->getNumVars(); ++i) {
    //avgDual_.push_back(0.);
  //}
    
  // allocate space for gradient of objective function
  //gradientObj_              = new double[p_->getNumVars()];

  logger_ = env->getLogger();

  //DivingheurStats stats_
  stats_                    = new DivingheurStats();
  for (UInt i=0; i<nSelector_; ++i) {
    stats_->numNLPs[i]      = 0;
    stats_->time[i]         = 0;
    stats_->iterations[i]   = 0;
    stats_->numInfeas[i]    = 0;
    stats_->errors[i]       = 0;
    stats_->numSol[i]       = 0;
  }
  stats_->totalProbs         = 0;
  stats_->totalSol          = 0;
  stats_->numLocal          = 1;
  stats_->best_obj_value    = INFINITY;
  stats_->totalTime         = 0; 
 
#if USE_OPENMP
  numThreads_ = std::min(env_->getOptions()->findInt("threads")->getValue(),
                        omp_get_num_procs());
#else
  numThreads_ = 1;
#endif
   
  numLevels_ = ceil ( ( (double)(maxProbs_*numThreads_*(1 + 3*env_->getOptions()->findBool("divheurLP")->getValue())) )/ (32*2) );
}


ParMINLPDiving::~ParMINLPDiving()
{
  //if (gradientObj_) {
    //delete [] gradientObj_;
  //}
  if (timer_) {
    delete timer_;
  }
  delete stats_;
  //if (lh_) {
    //delete lh_;
  //}
  //lastNodeMods_.clear();
}

void ParMINLPDiving::backtrack_(UInt n_moded, ModVector& lastNodeMods, ProblemPtr p,
                                std::stack<VarBoundModPtr>& mods)
{
  std::stack<VarBoundModPtr> temp;
  VarBoundModPtr varmod;
  VarBoundModPtr temp_varmod;
  VariablePtr variable;
  double old_bound_val;

  //std::cout << "Undoing mods\n"; 
  for (ModVector::reverse_iterator m_iter=lastNodeMods.rbegin(); 
       m_iter!=lastNodeMods.rend(); ++m_iter) {
    //(*m_iter)->write(std::cout);
    (*m_iter)->undoToProblem(p);
  }
  for (UInt i=0; i<n_moded; ++i) {
    varmod = mods.top();
    variable = varmod->getVar();
    old_bound_val = varmod->getNewVal();
    if (varmod->getLU() == Lower) {
      //previously changed bound was lower
      // decrement the old bound value by 1
      temp_varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, 
                                                     old_bound_val - 1);
    } else {
      // previously changed bound was upper
      // increment the old bound value by 1
      temp_varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, 
                                                     old_bound_val + 1);
    }
    temp.push(temp_varmod); // hold the modification in a temp stack 
    mods.pop();
  }

  // put the new modification back in the original stack
  //std::cout << "Applying in backtrack\n";
  while (!temp.empty()) {
    temp_varmod = temp.top();
    temp_varmod->applyToProblem(p);
    //temp_varmod->write(std::cout);
    mods.push(temp_varmod);
    temp.pop();
  }
}


UInt ParMINLPDiving::FracBounds_(UInt numfrac, const double* x, Direction d,
                                 Order o, ProblemPtr p, UIntVector& violated,
                                 std::stack<VarBoundModPtr>& mods,
                                 LinearHandler* lh, ModVector& lastNodeMods,
                                 DoubleVector& score, DoubleVector& avgDual,
                                 double* gradientObj)
{
  VariablePtr variable;
  VarBoundModPtr varmod;
  UInt id;
  double value;
  double new_bound;
  UInt begin      = 0;
  UInt end        = 0;
  //UInt change_wan = (UInt) ceil( (double) numfrac/4);
  UInt change_wan = (UInt) ceil( (double) (numfrac)/numLevels_ );
  UInt changes    = 0;
  ModVector dummy;
  NodePtr node = NodePtr();
  SolutionPoolPtr s_pool = SolutionPoolPtr(); // NULL
  SolveStatus status;

  // get the score of violated variables according to their fractional part
  getScore_(x, Fractional, score, p, avgDual, gradientObj);
  // sort the vector of index according to the fractional part of the variables
  sort_(0,violated.size()-1, score, violated);
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Changing bound of " << change_wan 
    << " variables" << std::endl;
#endif 

  switch (o) {
   case (Least) : // least fractional to be fixed => forward counting
     begin = 0; end = change_wan;
     break;

   case (Most) : // most fractional to be fixed => reverse counting
     begin = numfrac-change_wan; end = numfrac;
     break;

   default:
     assert(!"Order unknown");
  }

  //std::cout << "Applying mods\n";
  for (UInt i=begin; i<end; ++i) {
    id = violated[i];
    variable = p->getVariable(id);
    value = x[id];
    new_bound = rounding_(value, d); // get the new bound with direction
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "value of variable " 
      << variable->getName() << " " << value << std::endl;
    logger_->msgStream(LogDebug) << me_ << "value of new bound " 
      << d << "\t" << new_bound << std::endl;
#endif
    // fix the variable by changing the lower or upper bound to new_bound
    if (d == Floor) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    } else if (d == Ceil) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value < new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value > new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    }

    varmod->applyToProblem(p);
    //varmod->write(std::cout);
    mods.push(varmod); 
    lastNodeMods.push_back(varmod);
    status = Started;
    lh->simplePresolve(p, SolutionPoolPtr(), lastNodeMods, status);
    //after simplePresolve, the next variable may not need fixing

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "changed bound for variable "; 
    variable->write(logger_->msgStream(LogDebug));
    logger_->msgStream(LogDebug) << me_ << "changed bound for index " 
      << id << std::endl << std::endl;
#endif
    ++changes;
  }
  return changes;
}


void ParMINLPDiving::getScore_(const double* x, Scoretype s,
                               DoubleVector& score, ProblemPtr p,
                               DoubleVector& avgDual, double* gradientObj)
{
  VariablePtr variable;
  double value;
  UInt i = 0;
  double fractional;
  ObjectivePtr obj;
  UInt numcons;
  double vl_score;
  int error;
  score.clear(); // remove the score of violated variables from prev sol
  switch (s) {
   case (LexBound)   :
     for (VariableConstIterator v_iter=p->varsBegin();
         v_iter!=p->varsEnd(); ++v_iter, ++i) {
       variable = *v_iter;
       if (variable->getType() == Binary || variable->getType() == Integer) {
         value = x[i];
         fractional = fabs(floor(value+0.5)-value);
         if (fractional > ParMINLPDiving::intTol_) {
           score.push_back(i);
         }
       }
     }
#if SPEW
     logger_->msgStream(LogDebug) << " Lexicographic Scoring " << std::endl;
#endif
     break;

   case (Fractional) : 
     for (VariableConstIterator v_iter=p->varsBegin(); 
         v_iter!=p->varsEnd(); ++v_iter, ++i) {
       variable = *v_iter;
       if (variable->getType() == Binary || variable->getType() == Integer) {
         value = x[i];
         fractional = fabs(floor(value+0.5)-value);
         if (fractional > intTol_) {
           score.push_back(fractional);
         }
       }
     }
#if SPEW
     logger_->msgStream(LogDebug)  << " Fractional Scoring " << std::endl;
#endif
     break;

   case (VectorLength) : 
     obj = p->getObjective();
     std::fill(gradientObj, gradientObj+p->getNumVars(), 0);
     obj->evalGradient(x, gradientObj, &error);
     for (VariableConstIterator v_iter=p->varsBegin(); 
         v_iter!=p->varsEnd(); ++v_iter, ++i) {
       variable = *v_iter;
       if (variable->getType() == Binary || variable->getType() == Integer) {
         value = x[i];
         fractional = fabs(floor(value+0.5)-value);
         if (fractional > intTol_) {
           numcons = variable->getNumCons();
           numcons = (numcons > 0) ? numcons : numcons + 1;
           vl_score = gradientObj[i] * fractional/numcons; 
           score.push_back(vl_score);
         }
       }
     }
#if SPEW
     logger_->msgStream(LogDebug) << " Vector Length Scoring " << std::endl;
#endif
     break;

   case (ReducedCost) :
     score.resize(p->getNumVars());
     std::copy(avgDual.begin(), avgDual.end(), score.begin());
#if SPEW
     logger_->msgStream(LogDebug) << " Reduced Cost Scoring " << std::endl;
#endif
     break;

   default: break;
  }
}


void ParMINLPDiving::implementDive_(int i, const double* x, SolutionPoolPtr s_pool,
                                    ModVector& lastNodeMods, EnginePtr e,
                                    ProblemPtr p, DoubleVector& avgDual,
                                    UIntVector& violated,
                                    std::stack<VarBoundModPtr>& mods,
                                    LinearHandler* lh, DoubleVector& score,
                                    double* gradientObj, 
                                    DivingheurStats* stats)
{
  ConstSolutionPtr sol;
  Direction d;
  Order o;
  EngineStatus status;
  bool solFound = false;
  UInt backtrack     = 0;
  UInt min_vlength   = 5;
  UInt numfrac       = isFrac_(x, violated, p);
  FuncPtr f          = selectHeur_(i, d, o);
  UInt n_moded;

  if (f == &ParMINLPDiving::VectorLength_) {
    if (false==vectorFlag_(min_vlength, p)) { //vector length diving?
      logger_->msgStream(LogInfo) << "VectorLength Diving flag False" 
        << std::endl;
      return; // skip vector length diving
    }
  }

  lastNodeMods.clear();

  n_moded  = (this->*f)(numfrac, x, d, o, p, violated, mods, lh, lastNodeMods,
                        score, avgDual, gradientObj);
  UInt probLimit = numThreads_*maxProbs_*(1 + 3*env_->getOptions()->findBool("divheurLP")->getValue());
  while (stats_->totalProbs < probLimit) {
  //while (stats->totalNLPs < maxNLP_) 
    std::cout << " Heur " << i << " iter " << stats->totalProbs << "\n";
    status = e->solve();
    ++(stats->numNLPs[i/8]);
    ++(stats->totalProbs);
#if USE_OPENMP
#pragma omp critical (stats)
#endif
    ++(stats_->totalProbs);
    if (EngineError == status) {
      e->clear();  // reset the starting point
      e->load(p_);
      ++(stats->errors[i/8]);
    }
    if (status == ProvenLocalOptimal || status == ProvenOptimal 
        || status == ProvenFailedCQFeas || status == FailedFeas) {
      sol = e->getSolution();
      ++(stats->numLocal);
      if (stats_->best_obj_value - 1e-6 < sol->getObjValue()) {
//#if SPEW
        logger_->msgStream(LogInfo) << me_ 
          << "current solution worse than ub. Returning." << std::endl; 
//#endif
        return;
      }
      updateAvgDual_(sol, avgDual, stats);
      x = sol->getPrimal();
      numfrac = isFrac_(x, violated, p); // number of fractional vars in current solution
      if (0==numfrac) {  
        if (env_->getOptions()->findBool("divheurLP")->getValue()) {
          logger_->msgStream(LogInfo) << me_ << "LP feasible point" << std::endl;
          //Solve NLP and add solution if NLP is optimal
          EnginePtr nlpe = nlpe_->emptyCopy();
          ProblemPtr minlp = minlp_->clone();
          nlpe->load(minlp);
          solveNLP(sol, &solFound, minlp, nlpe);
          if (solFound) {
            if (stats_->best_obj_value - 1e-6 < nlpe->getSolution()->getObjValue()) {
//#if SPEW
              logger_->msgStream(LogInfo) << me_ 
                << "current solution worse than ub. Returning." << std::endl;
//#endif
            } else {
              logger_->msgStream(LogInfo) << me_ << "NLP feasible!" << std::endl;
#if USE_OPENMP
#pragma omp critical (solPool)
#endif
              s_pool->addSolution(nlpe->getSolution());
#if USE_OPENMP
#pragma omp critical (stats)
#endif
              stats_->best_obj_value = nlpe->getSolution()->getObjValue();
              ++(stats->numSol[i/8]);
              ++(stats->totalSol);
            }
          } else {
            logger_->msgStream(LogInfo) << me_ << "NLP infeasible" << std::endl;
          }
        } else {
//#if SPEW
          logger_->msgStream(LogInfo) << me_ << "Feasible Solution" << std::endl;
          sol->write(logger_->msgStream(LogDebug2));
//#endif
          logger_->msgStream(LogInfo) << me_ << "Updating the solution value to "
            << sol->getObjValue() << std::endl;
#if USE_OPENMP
#pragma omp critical (stats)
#endif
          stats_->best_obj_value = sol->getObjValue();
#if USE_OPENMP
#pragma omp critical (solPool)
#endif
          s_pool->addSolution(sol);
          ++(stats->numSol[i/8]);
          ++(stats->totalSol);
        }
        return;
      } else {
        // dive further down by rounding variable in current solution "x"
        lastNodeMods.clear();
#if USE_OPENMP
#pragma omp critical (log)
#endif
        logger_->msgStream(LogInfo) << "Diving down" << std::endl;
        n_moded = (this->*f)(numfrac, x, d, o, p, violated, mods, lh,
                             lastNodeMods, score, avgDual, gradientObj);
        backtrack = 0;
      }
    } else if (0 < backtrack) {
      ++(stats->numInfeas[i/8]);
      logger_->msgStream(LogInfo) << "Backtrack unsuccessful" << std::endl;
      return;
    } else {
      // backtrack to the other child of the parent
#if USE_OPENMP
#pragma omp critical (log)
#endif
//#if SPEW
      logger_->msgStream(LogInfo) << me_ << "Backtracking " << std::endl;
//#endif
      ++backtrack;
      backtrack_(n_moded, lastNodeMods, p, mods);
      ++(stats->numInfeas[i/8]);
    }
  }
  logger_->msgStream(LogInfo) << "Problem limit reached" << std::endl;
}


UInt ParMINLPDiving::isFrac_(const double* x, UIntVector& violated, ProblemPtr p)
{
  VariablePtr variable;
  double value;
  UInt i = 0;
  double fractional;
  // remove the violated variables from previous solution
  violated.clear();
  for (VariableConstIterator v_iter=p->varsBegin(); 
      v_iter!=p->varsEnd(); ++v_iter, ++i) {
    variable = *v_iter;
    if (variable->getType() == Binary || variable->getType() == Integer) {
      value = x[i];
      fractional = fabs(floor(value+0.5)-value);
      if (fractional > intTol_) {
        violated.push_back(i);    // variable i is fractional
        //score_.push_back(fractional);
      }
    }
  }
  return violated.size();
}

void ParMINLPDiving::solveNLP(ConstSolutionPtr sol, bool *solFound,
                              ProblemPtr minlp, EnginePtr nlpe)
{
  EngineStatus nlpStatus;
  std::stack<Modification *> nlpMods;

  fixInts(sol->getPrimal(), nlpMods, minlp);

  nlpStatus = nlpe->solve();

  unfixInts(nlpMods, minlp);
  
  switch(nlpStatus) {
  case (ProvenOptimal):
  case (ProvenLocalOptimal):
    *solFound = true;
    break;    
  case (ProvenInfeasible):
  case (ProvenLocalInfeasible): 
  case (ProvenObjectiveCutOff):
  case (EngineIterationLimit):
  case (EngineError):
  case (FailedInfeas):
  case (ProvenUnbounded):
  case (ProvenFailedCQFeas):
  case (EngineUnknownStatus):
  case (ProvenFailedCQInfeas):
  default:
  case (FailedFeas):
    *solFound = false;
    break;
  }
  return;
}

void ParMINLPDiving::fixInts(const double *x,
                             std::stack<Modification *> &nlpMods,
                             ProblemPtr minlp)
{
  double xval;
  VariablePtr v;
  VarBoundMod2 *m = 0;
  for (VariableConstIterator vit=minlp->varsBegin(); vit!=minlp->varsEnd();
       ++vit) {
    v = *vit;
    if (v->getType()==Binary || v->getType()==Integer) {
      xval = x[v->getIndex()];
      xval = floor(xval + 0.5);
      m = new VarBoundMod2(v, xval, xval);
      m->applyToProblem(minlp);
      (nlpMods).push(m);
    }
  }
  return;
}

void ParMINLPDiving::unfixInts(std::stack<Modification *> &nlpMods,
                               ProblemPtr minlp)
{
  Modification *m = 0;
  while((nlpMods).empty() == false) {
    m = (nlpMods).top();
    m->undoToProblem(minlp);
    (nlpMods).pop();
    delete m;
  }
  return;
}

UInt ParMINLPDiving::LexBounds_(UInt numfrac, const double* x, Direction d,
                                Order o, ProblemPtr p, UIntVector& violated,
                                std::stack<VarBoundModPtr>& mods,
                                LinearHandler* lh, ModVector& lastNodeMods,
                                DoubleVector& score, DoubleVector& avgDual,
                                double* gradientObj)
{
  VariablePtr variable;
  VarBoundModPtr varmod;
  UInt id;
  double new_bound;
  double value;
  UInt begin      = 0;
  UInt end        = 0;
  //UInt change_wan = (UInt) ceil( (double) numfrac/4);
  UInt change_wan = (UInt) ceil( (double) (numfrac)/numLevels_ );
  //UInt change_wan = (UInt) ceil( (double) numfrac/env_->getOptions()->findInt("divheurLevel")->getValue());
  UInt changes    = 0;
  ModVector dummy;
  NodePtr node = NodePtr();
  SolutionPoolPtr s_pool = SolutionPoolPtr(); // NULL
  SolveStatus status;

  getScore_(x, ReducedCost, score, p, avgDual, gradientObj);
  std::setprecision(8);
  sort_(0,violated.size()-1, score, violated);
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Changing bound of " << change_wan 
    << " variables" << std::endl;
#endif 
  // set up begin and end for the selection of variable
  switch (o) {
   case (Least) : // least fractional to be fixed => forward counting
     begin = 0; end = change_wan; 
     break;
   
   case (Most) : // most fractional to be fixed => reverse counting
     begin = numfrac-change_wan; end = numfrac;
     break;

   default:
     assert(!"Order unknown");
  }

  // based on begin and end from switch change the bound of variables
  //std::cout << "Applying \n";
  for (UInt i=begin; i<end; ++i) {
    id         = violated[i];
    variable   = p->getVariable(id);
    value      = x[id];
    new_bound = rounding_(value, d); // get the new bound with direction
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "value of variable " 
      << variable->getName() << " " << value << std::endl;
    logger_->msgStream(LogDebug) << me_ << "value of new bound " 
      << d << "\t" << new_bound << std::endl;
#endif
    // fix the variable by changing the lower or upper bound to new_bound
    if (d == Floor) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    } else if (d == Ceil) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value < new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value > new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    }

    varmod->applyToProblem(p);
    //varmod->write(std::cout);
    mods.push(varmod); 
    lastNodeMods.push_back(varmod);
    status = Started;
    lh->simplePresolve(p, SolutionPoolPtr(), lastNodeMods, status);

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "changed bound for variable "; 
    variable->write(logger_->msgStream(LogDebug));
    logger_->msgStream(LogDebug) << std::endl;
    logger_->msgStream(LogDebug) << me_ << "changed bound for index " 
      << id << std::endl << std::endl;
#endif
    ++changes;
  }

  return changes;
}


UInt ParMINLPDiving::ReducedCost_(UInt numfrac, const double* x, Direction d,
                                  Order o, ProblemPtr p, UIntVector& violated,
                                  std::stack<VarBoundModPtr>& mods,
                                  LinearHandler* lh, ModVector& lastNodeMods,
                                  DoubleVector& score, DoubleVector& avgDual,
                                  double* gradientObj)
{
  VariablePtr variable;
  VarBoundModPtr varmod;
  UInt id;
  double new_bound;
  double value;
  UInt begin      = 0;
  UInt end        = 0;
  //UInt change_wan = (UInt) ceil( (double) numfrac/4);
  UInt change_wan = (UInt) ceil( (double) (numfrac)/numLevels_ );
  //UInt change_wan = (UInt) ceil( (double) numfrac/env_->getOptions()->findInt("divheurLevel")->getValue());
  UInt changes    = 0;
  ModVector dummy;
  NodePtr node = NodePtr();
  SolutionPoolPtr s_pool = SolutionPoolPtr(); // NULL
  SolveStatus status;

  getScore_(x, ReducedCost, score, p, avgDual, gradientObj);
  std::setprecision(8);
  sort_(0,violated.size()-1, score, violated);
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Changing bound of " << change_wan 
    << " variables" << std::endl;
#endif 
  // set up begin and end for the selection of variable
  switch (o) {
   case (Least) : // least fractional to be fixed => forward counting
     begin = 0; end = change_wan; 
     break;
   
   case (Most) : // most fractional to be fixed => reverse counting
     begin = numfrac-change_wan; end = numfrac;
     break;

   default:
     assert(!"Order unknown");
  }

  // based on begin and end from switch change the bound of variables
  for (UInt i=begin; i<end; ++i) {
    id         = violated[i];
    variable   = p->getVariable(id);
    value      = x[id];
    new_bound = rounding_(value, d); // get the new bound with direction
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "value of variable " 
      << variable->getName() << " " << value << std::endl;
    logger_->msgStream(LogDebug) << me_ << "value of new bound " 
      << d << "\t" << new_bound << std::endl;
#endif
    // fix the variable by changing the lower or upper bound to new_bound
    if (d == Floor) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    } else if (d == Ceil) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value < new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value > new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    }

    varmod->applyToProblem(p);
    mods.push(varmod); 
    lastNodeMods.push_back(varmod);
    status = Started;
    lh->simplePresolve(p, SolutionPoolPtr(), lastNodeMods, status);

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "changed bound for variable "; 
    variable->write(logger_->msgStream(LogDebug));
    logger_->msgStream(LogDebug) << std::endl;
    logger_->msgStream(LogDebug) << me_ << "changed bound for index " 
      << id << std::endl << std::endl;
#endif
    ++changes;
  }

  return changes;
}


void ParMINLPDiving::restoreBounds_(double* LB_copy, double* UB_copy, UInt vars, ProblemPtr p)
{
  for (UInt i=0; i<vars; ++i, ++LB_copy, ++UB_copy) {
    p->changeBoundByInd(i, Lower, *LB_copy);
    p->changeBoundByInd(i, Upper, *UB_copy);
  }
}


double ParMINLPDiving::rounding_(double value, Direction d)
{
  switch (d) {
       case (Floor)    : return floor(value); 
                         break;
       case (Ceil)     : return ceil(value); 
                         break;
       case (Nearest)  : if (fabs(value-floor(value)) < 0.5) {
                          // floor is the nearest
                          return floor(value);
                         } else {
                           return ceil(value);
                         }
                         break;
       case (Farthest) : if (fabs(value-floor(value)) < 0.5) { 
                           // ceil is the farthest
                           return ceil(value);
                         } else {
                           return floor(value);
                         }
                         break;
      }
  assert(!"unknown case in rounding_");
  return 0;
}


void ParMINLPDiving::saveBounds_(double* LB_copy, double* UB_copy, UInt vars)
{
  VariablePtr variable;
  for (UInt i=0; i<vars; ++i, ++LB_copy, ++UB_copy) {
    variable = p_->getVariable(i);
    *LB_copy = variable->getLb();
    *UB_copy = variable->getUb();
  }
}


ParMINLPDiving::FuncPtr ParMINLPDiving::selectHeur_(int i, Direction &d, Order &o)
{
#if USE_OPENMP
#pragma omp critical (log)
  logger_->msgStream(LogInfo) << "Thread " << omp_get_thread_num() << " i = " << i << " : "
    << getScoreString(i) << " : " << getDirectionString(i) << " : "
    << getOrderString(i) << std::endl;
#endif

  switch (i%4) {
   case 0 : d = Floor;
            break;

   case 1 : d = Ceil;
            break;

   case 2 : d = Nearest;
            break;

   case 3 : d = Farthest;
            break;

   default: d = Ceil;
            break;
  }

  if (i%8 < 4) {
    o = Least;
  } else {
    o = Most;
  }

  if (i < 8) {
    return &ParMINLPDiving::FracBounds_; 
  } else if (i < 16) {
    return &ParMINLPDiving::VectorLength_;
  } else if (i < 24) {
    return &ParMINLPDiving::ReducedCost_;
  } else {
    return &ParMINLPDiving::LexBounds_;
  } 
}


bool ParMINLPDiving::shouldDive_()
{
  VariablePtr variable;
  
  UInt min_num_bint             = 5;
  ConstProblemSizePtr prob_size = p_->getSize();
  UInt num_bin_int              = prob_size->bins + prob_size->ints;
  int option = env_->getOptions()->findBool("pardivheur")->getValue();
  
  switch (option) {

   case 0  : if (num_bin_int <= min_num_bint) {
               return false;
             } else {
               return true;
             }
             break;

   case 1  : return true;
             break;

   default : assert(!"Unknown option for ParMINLPDiving heuristic");
             return false;
             break;
  }

  return false; // if option == -1
}


void ParMINLPDiving::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  bool goAhead = true;
  wallTimeStart_ = getWallTime();
  double wallTimeLimit = env_->getOptions()->findDouble("heur_time_limit")->getValue();
  ConstSolutionPtr sol;
  EngineStatus status;
  const double* x;
  
  int num_method         = 32;
  UInt numvars           = p_->getNumVars();
  double* root_x;
  double* root_copy;
  double* LB_copy;
  double* UB_copy;
  logger_->msgStream(LogInfo) << me_ << "Starting" << std::endl;
  logger_->msgStream(LogInfo) << me_ << "Partitioning levels: " << numLevels_ << std::endl;
  //timer_->start();
  if (!shouldDive_()) {
    logger_->msgStream(LogInfo) << me_ << "Skipping" << std::endl;
    //timer_->stop();
    return;
  }
  root_x             = new double[numvars];
  root_copy          = new double[numvars];
  LB_copy            = new double[numvars];
  UB_copy            = new double[numvars];
  e_->clear();
  e_->load(p_);
  e_->setIterationLimit(7000); // try to run for a loooong time.
  status             = e_->solve();
  sol                = e_->getSolution();
  //stats_->totalTime  = timer_->query();
  stats_->totalTime  = getWallTime() - wallTimeStart_;
  x                  = sol->getPrimal();
  std::copy(x, x + numvars, root_x);
  DoubleVector avgDualR;
  avgDualR.resize(numvars, 0);
  updateAvgDual_(sol, avgDualR, stats_);
  UIntVector violatedR;
  e_->setIterationLimit(200);
  if (status == ProvenLocalInfeasible) {
    logger_->msgStream(LogInfo) << me_ << "Root infeasible" << std::endl;
    logger_->msgStream(LogInfo) << me_ << "Best feasible sol value    = "
      << stats_->best_obj_value << std::endl;
    logger_->msgStream(LogInfo) << me_ << "Total time taken           = " 
      << getWallTime() - wallTimeStart_ << std::endl
      << me_ << "Total problems solved      = " << 1
      << " : total = " << 1 << " : maximum per thread = " << maxProbs_
      << std::endl;

  } else if (isFrac_(root_x, violatedR, p_) == 0) {
    if (env_->getOptions()->findBool("divheurLP")->getValue()) {
      bool solFound = false;
      logger_->msgStream(LogInfo) << me_ << "LP feasible point" << std::endl;
      //Solve NLP and add solution if NLP is optimal
      nlpe_->load(minlp_);
      solveNLP(sol, &solFound, minlp_, nlpe_);
      if (solFound) {
        s_pool->addSolution(nlpe_->getSolution());
        stats_->best_obj_value = nlpe_->getSolution()->getObjValue();
        logger_->msgStream(LogInfo) << me_ << "NLP feasible!" << std::endl;
        //logger_->msgStream(LogInfo) << me_ << "solution value is " 
          //<< stats_->best_obj_value << std::endl;
        logger_->msgStream(LogInfo) << me_ << "Best feasible sol value    = "
          << stats_->best_obj_value << std::endl;
        logger_->msgStream(LogInfo) << me_ << "Total time taken           = " 
          << getWallTime() - wallTimeStart_ << std::endl
          << me_ << "Total problems solved      = " << 1
          << " : total = " << 1 << " : maximum per thread = " << maxProbs_
          << std::endl;
        goAhead = false;
      } else {
        logger_->msgStream(LogInfo) << me_ << "NLP infeasible at root sol" << std::endl;
        logger_->msgStream(LogInfo) << me_ << "Best feasible sol value    = "
          << stats_->best_obj_value << std::endl;
        logger_->msgStream(LogInfo) << me_ << "Total time taken           = " 
          << getWallTime() - wallTimeStart_ << std::endl
          << me_ << "Total problems solved      = " << 1
          << " : total = " << 1 << " : maximum per thread = " << maxProbs_
          << std::endl;
        goAhead = false;
      }
    } else {
      stats_->best_obj_value = sol->getObjValue();
      //logger_->msgStream(LogInfo) << me_ << "solution value is " 
        //<< stats_->best_obj_value << std::endl;
      s_pool->addSolution(sol);
      //if (stats_->best_obj_value < INFINITY) {
        logger_->msgStream(LogInfo) << me_ << "Best feasible sol value    = "
          << stats_->best_obj_value << std::endl;
      //}
      logger_->msgStream(LogInfo) << me_ << "Total time taken           = " 
        << getWallTime() - wallTimeStart_ << std::endl
        << me_ << "Total problems solved      = " << 1
        << " : total = " << 1 << " : maximum per thread = " << maxProbs_
        << std::endl;
 
      goAhead = false;
    }
  } else if (goAhead) {
#if USE_OPENMP
    logger_->msgStream(LogInfo) << "using " << numThreads_ << " out of "
      << omp_get_num_procs() << " processors" << std::endl;
#endif
    //lh_ = new LinearHandler(env_, p_);
    saveBounds_(LB_copy, UB_copy, numvars);

    // CREATING OBJECTS EACH TIME BELOW NEEDS CORRECTION ASAP!!
#if USE_OPENMP
#pragma omp parallel num_threads(numThreads_)
#endif
    {
      //DivingheurStats stats for each thread
      DivingheurStats *stats;
      stats = new DivingheurStats();
      for (UInt j=0; j<nSelector_; ++j) {
        stats->numNLPs[j]      = 0;
        stats->time[j]         = 0;
        stats->iterations[j]   = 0;
        stats->numInfeas[j]    = 0;
        stats->errors[j]       = 0;
        stats->numSol[j]       = 0;
      }
      stats->totalProbs         = 0;
      stats->totalSol          = 0;
      stats->numLocal          = 1;
      stats->best_obj_value    = INFINITY;
      stats->totalTime         = 0; 
    // loop over the methods starts here
#if USE_OPENMP
#pragma omp for
#endif
    //for (int i=0; i<num_method && stats_->totalSol < maxSol_; ++i)
    for (int i=0; i<num_method; ++i) {
      if (getWallTime() - wallTimeStart_ <= wallTimeLimit) {
        logger_->msgStream(LogDebug) << me_<< "diving method "
          << i << std::endl;
        std::copy(root_x, root_x + numvars, root_copy); 
        x = root_copy;
        // Thread specific 
        EnginePtr e =  e_->emptyCopy();
        ProblemPtr p = p_->clone();
        LinearHandler *lh;
        double* gradientObj;
        DoubleVector score;
        UIntVector violated;
        DoubleVector avgDual = avgDualR;
        //avgDual.resize(numvars, 0);
        ModVector lastNodeMods;
        std::stack<VarBoundModPtr> mods;
        e->clear();
        e->load(p);
        e->setIterationLimit(200);
        lh = new LinearHandler(env_, p);
        gradientObj = new double[p->getNumVars()];

        implementDive_(i, x, s_pool, lastNodeMods, e, p, avgDual, violated,
                       mods, lh, score, gradientObj, stats);
        restoreBounds_(LB_copy, UB_copy, numvars, p);
        // clear the stack of modification for this heuristic method
        while (!mods.empty()) {
            mods.pop();
        }
        if ((i+1)%8 == 0) {
#if USE_OPENMP
#pragma omp critical (stats)
#endif  
          //stats_->time[i/8]  = timer_->query();
          stats_->time[i/8]  = getWallTime() - wallTimeStart_;
          //stats->time[i/8]  = timer_->query();
          stats->time[i/8]  = getWallTime() - wallTimeStart_;
          //timer_->stop();
          //timer_->start();
        }
        //if (lh_) {
        //  delete lh_;
        //}
        //lastNodeMods_.clear();
      }
    } // loop over methods ends here
#if USE_OPENMP
#pragma omp critical (stats)
#endif
    writeParStats(logger_->msgStream(LogInfo), stats, getWallTime());
    }
  } else {
    logger_->msgStream(LogInfo) << "Abrupt quit!" <<std::endl;
  }
  e_->resetIterationLimit();
  logger_->msgStream(LogInfo) << me_ << "Over" << std::endl;
  if (root_x){
    delete [] root_x;
  }
  if (LB_copy){
    delete [] LB_copy;
  }
  if (UB_copy){
    delete [] UB_copy;
  }
  if (root_copy){
    delete [] root_copy;
  }
  //timer_->stop();
  exit(1);
}


void ParMINLPDiving::sort_(UInt left, UInt right, DoubleVector& score, UIntVector& violated)
{
  UInt i = left;
  UInt j = right;
  double pivot = score[(left + right)/2];

  while (i <= j) {
    while (score[i] < pivot) {
      ++i;
    }
    while (score[j] > pivot) {
      assert (j!=0); // should never come here if j==0, otherwise overflows.
      --j; 
    }
    if (i <= j) {
      std::swap(score[i],score[j]);
      std::swap(violated[i],violated[j]);
      ++i;
      if (j>0) {
        --j;// should never come here if j==0, otherwise overflows.
      }
    }
  }

  if (left < j) {
    sort_(left,j, score, violated);
  }
  if (i < right) {
    sort_(i,right, score, violated);
  }
}


void ParMINLPDiving::updateAvgDual_(ConstSolutionPtr sol,
                                    DoubleVector& avgDual,
                                    DivingheurStats* stats)
{
  DoubleVector::iterator it;
  UInt iter         = stats->numLocal;
  const double* d   = sol->getDualOfVars();
  for (it=avgDual.begin(); it!=avgDual.end(); ++it, ++d) {
    *it = ((*it)*(iter - 1) + *d)/iter;
  }
}


UInt ParMINLPDiving::VectorLength_(UInt numfrac, const double* x, Direction d,
                                   Order o, ProblemPtr p, UIntVector& violated,
                                   std::stack<VarBoundModPtr>& mods,
                                   LinearHandler* lh, ModVector& lastNodeMods,
                                   DoubleVector& score, DoubleVector& avgDual,
                                   double* gradientObj)
{
  VariablePtr variable;
  VarBoundModPtr varmod;
  UInt id;
  double new_bound;
  double value;
  UInt begin      = 0;
  UInt end        = 0;
  //UInt change_wan = (UInt) ceil( (double) numfrac/4);
  UInt change_wan = (UInt) ceil( (double) (numfrac)/numLevels_ );
  //UInt change_wan = (UInt) ceil( (double) numfrac/env_->getOptions()->findInt("divheurLevel")->getValue());
  UInt changes    = 0;
  ModVector dummy;
  NodePtr node = NodePtr();
  SolutionPoolPtr s_pool = SolutionPoolPtr(); // NULL
  SolveStatus status;

  getScore_(x, VectorLength, score, p, avgDual, gradientObj);
  sort_(0, score.size()-1, score, violated);
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "Changing bound of " << change_wan 
    << " variables" << std::endl;
#endif 
  // set up begin and end for the selection of variable
  switch (o) {
   case (Least) : // least fractional to be fixed => forward counting
     begin = 0; end = change_wan; 
     break;
   
   case (Most) : // most fractional to be fixed => reverse counting
     begin = numfrac-change_wan; end = numfrac;
     break;

   default:
     assert(!"Order unknown");
  }

  // based on begin and end from switch change the bound of variables
  //std::cout << "Applying \n";
  for (UInt i=begin; i<end; ++i) {
    id = violated[i];
    variable = p->getVariable(id);
    value = x[id];
    new_bound = rounding_(value, d); // get the new bound with direction
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "value of variable " 
      << variable->getName() << " " << value << std::endl;
    logger_->msgStream(LogDebug) << me_ << "value of new bound " 
      << d << "\t" << new_bound << std::endl;
#endif
    // fix the variable by changing the lower or upper bound to new_bound
    if (d == Floor) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    } else if (d == Ceil) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value < new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Lower, new_bound);
    } else if (value > new_bound) {
      varmod = (VarBoundModPtr) new VarBoundMod(variable, Upper, new_bound);
    }

    varmod->applyToProblem(p);
    //varmod->write(std::cout);
    mods.push(varmod); 
    lastNodeMods.push_back(varmod);
    status = Started;
    lh->simplePresolve(p, SolutionPoolPtr(), lastNodeMods, status);

#if SPEW
    logger_->msgStream(LogDebug) << me_ << "changed bound for variable "; 
    variable->write(logger_->msgStream(LogDebug));
    logger_->msgStream(LogDebug) << std::endl;
    logger_->msgStream(LogDebug) << me_ << "changed bound for index " 
      << id << std::endl << std::endl;
#endif
    ++changes;
  }

  return changes;
}


bool ParMINLPDiving::vectorFlag_(UInt min_vlength, ProblemPtr p)
{
  UInt num_obj_int      = 0;
  FunctionPtr obj       = p->getObjective()->getFunction();
  VariablePtr variable;
  // get number of binary and integer variables in the objective
  // if num_obj_int is below a threshold then vector length diving is skipped

  for (VarSetConstIterator it= obj->varsBegin(); it!=obj->varsEnd(); ++it) {
    variable = *it;
    if (variable->getType() == Binary || variable->getType() == Integer) {
      ++num_obj_int ;
    }
  }
  if ((p->getObjective()->getFunctionType() == Linear 
        || p->getObjective()->getFunctionType() == Quadratic) 
      && num_obj_int >= min_vlength) {
    return true; 
  } else { 
    return false;
  }
}


void ParMINLPDiving::writeParStats(std::ostream &out, DivingheurStats* stats,
                                   double wallTime) const
{
// write the statistics for ParMINLP heuristic per thread
#if USE_OPENMP
  out << std::endl << me_ << "Thread " << omp_get_thread_num() << std::endl;
#endif
  for (UInt i=0; i<nSelector_; ++i) {
    out << me_ << "heuristic method = " << i 
      << std::endl
      << me_ << "number of problems solved      = " << stats->numNLPs[i] 
      << std::endl
      << me_ << "time taken                     = " << stats->time[i] 
      << std::endl
      << me_ << "number of solutions found      = " << stats->numSol[i] 
      << std::endl
      << me_ << "number of infeasible problems  = " << stats->numInfeas[i] 
      << std::endl
      << me_ << "number of errors               = " << stats->errors[i] 
      << std::endl
      << me_ << "number of iterations           = " << stats->iterations[i] 
      << std::endl << std::endl;
    //stats_->totalTime += stats_->time[i];
    stats_->totalTime = wallTime - wallTimeStart_;
  }
#if USE_OPENMP
#pragma omp master
#endif
  {
  //if (stats_->best_obj_value < INFINITY) {
    logger_->msgStream(LogInfo) << me_ << "Best feasible sol value    = "
      << stats_->best_obj_value << std::endl;
  //}
  logger_->msgStream(LogInfo) << me_ << "Total time taken           = " 
    << stats_->totalTime << std::endl
    << me_ << "Total problems solved      = " << stats->totalProbs
    << " : total = " << stats_->totalProbs << " : maximum per thread = " << maxProbs_
    << std::endl;
  }
}


void ParMINLPDiving::writeStats(std::ostream &out) const
{
// write the statistics for ParMINLP heuristic
  for (UInt i=0; i<nSelector_; ++i) {
    out << me_ << "heuristic method = " << i 
      << std::endl
      << me_ << "number of nlps solved      = " << stats_->numNLPs[i] 
      << std::endl
      << me_ << "time taken                 = " << stats_->time[i] 
      << std::endl
      << me_ << "number of solutions found  = " << stats_->numSol[i] 
      << std::endl
      << me_ << "number of Infeasible NLPs  = " << stats_->numInfeas[i] 
      << std::endl
      << me_ << "number of Errors           = " << stats_->errors[i] 
      << std::endl
      << me_ << "number of iterations       = " << stats_->iterations[i] 
      << std::endl << std::endl;
    stats_->totalTime += stats_->time[i];
  }
  if (stats_->best_obj_value < INFINITY) {
    logger_->msgStream(LogInfo) << me_ << "Best feasible sol value    = "
      << stats_->best_obj_value << std::endl;
  }
  logger_->msgStream(LogInfo) << me_ << "Total time taken           = " 
    << stats_->totalTime << std::endl
    << me_ << "Total NLPs solved          = " << stats_->totalProbs
    << std::endl;
}

std::string ParMINLPDiving::getDirectionString(UInt i)
{
  switch (i%4) {
   case 0:
     return "Floor";
   case 1:
     return "Ceil";
   case 2:
     return "Nearest";
   case 3:
     return "Farthest";
   default:
     return "Unknown rounding direction";
  }
}

std::string ParMINLPDiving::getOrderString(UInt i)
{
  if (i%8 < 4) {
    return "Least Fractional First";
  } else if (i%8 >= 4) {
     return "Most Fractional First";
  } else {
     return "Unknown ordering rule";
  }
}

std::string ParMINLPDiving::getScoreString(UInt i)
{
  switch (i/8) {
   case 0:
     return "Fractional";
   case 1:
     return "VectorLength";
   case 2:
     return "ReducedCost ";
   case 3:
     return "LexBound";
   default:
     return "Unknown scoring rule";
  }
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
