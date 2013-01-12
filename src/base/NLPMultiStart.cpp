// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2013 The MINOTAUR Team.
// 

/**
 * \file NLPMultiStart.cpp
 * \brief Multi start heuristic for continuous NLPs
 * \author Jayash Koshal, Argonne National Laboratory
 */

#include <cmath> // for INFINITY

#include "MinotaurConfig.h"
#include "Engine.h"
#include "Variable.h"
#include "Environment.h"
#include "NLPMultiStart.h"
#include "Logger.h"
#include "Node.h"
#include "Operations.h"
#include "Option.h"
#include "SolutionPool.h"
#include "Timer.h"
#include <iomanip>
#include <algorithm>
#include <cstdlib>
using namespace Minotaur;

//#define SPEW 1

const std::string NLPMultiStart::me_ = "NLP Multi-Start Heuristic: ";

NLPMultiStart::NLPMultiStart(EnvPtr env, ProblemPtr p, EnginePtr e)
: e_(e),
  env_(env),
  p_(p),
  stats_(NULL)
{
  VariablePtr variable;
  UInt vars      = p_->getNumVars();
  distBound_     = 0;
  for (UInt i=0; i<vars; ++i) {
    variable = p_->getVariable(i);
    distBound_ += pow(variable->getUb()-variable->getLb(),2);
  }
  distBound_ = (distBound_ >= INFINITY) ? sqrt(vars) : sqrt(distBound_);
  distBound_ /= 2;
  logger_ = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
      findInt("MSheur_log_level")->getValue());
#if SPEW
  logger_->MsgStream(LogDebug) << me_ 
    << "Dist bound_ " << distBound_ << std::endl;
#endif


  random_                   = new Double[p->getNumVars()];  

  //MSHeurStats stats_
  stats_                    = new MSHeurStats();
  stats_->numNLPs           = 0;
  stats_->numInfeas         = 0;
  stats_->numImprove        = 0;
  stats_->numBadstatus      = 0;
  stats_->time              = 0;
  stats_->iterations        = 0;
  stats_->bestObjValue      = INFINITY;

}


NLPMultiStart::~NLPMultiStart(){
  delete [] random_;
  delete stats_;
}


void NLPMultiStart::constructInitial_(Double* pointa, const Double* pointb,
    Double rho, UInt vars)
{
  Double dist;
  VariablePtr variable;
  Double norm;
  VariableConstIterator v_iter;

  for (UInt i=0; i<vars; ++i) {
    random_[i] = rand()/Double(RAND_MAX) - 0.5;
  }
  norm = sqrt(InnerProduct(random_, random_, vars)); 
  for (UInt i=0; i<vars; ++i) {
    random_[i] /= norm;
  }

#if SPEW
    logger_->MsgStream(LogDebug) << me_ << "rho = " << rho << std::endl;
#endif

  // find how far we need to go.
  dist  = getDistance(pointa, pointb, vars);
  if (dist <= distBound_) {
    dist = distBound_;
  }
  dist *= rho;

  for (v_iter=p_->varsBegin(); v_iter!=p_->varsEnd(); ++v_iter, 
      ++pointa, ++pointb, ++random_) {
    variable = *v_iter;
    // find a point in a random direction outside the ball
    // centered around x* with radius = ||x*-x||
    *pointa = std::max(std::min(*pointb + (*random_) * dist, 
          variable->getUb()), variable->getLb());
  }
  random_ -= vars;

#if SPEW
  logger_->MsgStream(LogDebug2)
    << me_ << "distance to new point = " 
    <<  getDistance(pointa-vars, pointb-vars, vars)
    << std::endl;
#endif 
}


void NLPMultiStart::solve(NodePtr, RelaxationPtr, 
    SolutionPoolPtr s_pool)
{
  ConstSolutionPtr sol; 
  EngineStatus status;
  UInt heur_bound                = 50; // number of times a Heuristic should run
  UInt unchanged_obj_count_limit = 20;
  Double obj_tol                 = 1e-6; 
  Double rho_initial             = 1.1;// amplification factor
  Double rho                     = rho_initial;
  Timer *timer                   = env_->getNewTimer();
  UInt vars                      = p_->getNumVars();
  Double* prev_feasible          = new Double[vars];
  Double* initial_point          = new Double[vars];

  for (UInt i=0; i <vars;++i){
    initial_point[i] = rand()/Double (RAND_MAX);
  }
  srand(1);
  timer->start();
  for (UInt i=0, unchanged_obj_count=0; i<heur_bound && unchanged_obj_count< 
      unchanged_obj_count_limit; ++i) {
    e_->clear();
    // XXX: ashu to bring this out of the loop.
    e_->load(p_);
    p_->setInitialPoint(initial_point);
    status = e_->solve();
    ++(stats_->numNLPs);
    sol = e_->getSolution();
    if (sol->getObjValue() < stats_->bestObjValue - obj_tol) {
      stats_->bestObjValue = sol->getObjValue();
      rho = rho_initial;
      unchanged_obj_count = 0;
      s_pool->addSolution(sol);
      constructInitial_(initial_point, sol->getPrimal(), rho, vars);
      std::copy(sol->getPrimal(), sol->getPrimal() + vars, prev_feasible); 
      ++(stats_->numImprove);
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "Better solution " 
        << stats_->bestObjValue << std::endl;
#endif
    } else if ((ProvenInfeasible==status || ProvenLocalInfeasible==status ||
        ProvenObjectiveCutOff==status || ProvenFailedCQInfeas==status || 
        FailedInfeas==status) || ((FailedFeas==status || 
        ProvenFailedCQFeas==status || ProvenLocalOptimal==status) && 
        stats_->bestObjValue <= sol->getObjValue() + obj_tol)) {
      rho *= 1.07;
      ++unchanged_obj_count;
      // use previously found feasible solution
      constructInitial_(initial_point, prev_feasible, rho, vars);
      ++(stats_->numInfeas);
#if SPEW
      logger_->MsgStream(LogDebug) << me_ 
        << "Engine status = " << status << std::endl
        << me_
        << "Optimal solution no better than best known " << sol->getObjValue()  
        << std::endl;
#endif
    } else if (status == ProvenUnbounded) { 
      rho *= 0.9;
      ++unchanged_obj_count;
      ++(stats_->numBadstatus);
      // use previously found feasible solution
      constructInitial_(initial_point, prev_feasible, rho, vars);       
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "Unbounded." << std::endl;
#endif
    } else { 
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << "Solution found is not optimal" 
        << " solution value = " <<  sol->getObjValue() << std::endl; 
#endif
    }
    stats_->time = timer->query();
  }
  if (timer) {
    delete timer;
  }
  if (initial_point) {
    delete [] initial_point;
  }
  if (prev_feasible) {
    delete [] prev_feasible;
  }
}


void NLPMultiStart::writeStats()
{
  logger_->MsgStream(LogInfo)
    << me_ << " number of nlps solved                 = " 
    << stats_->numNLPs << std::endl
    << me_ << " number of Infeasible solves           = " 
    << stats_->numInfeas << std::endl
    << me_ << " number of Improvements in objective   = " 
    << stats_->numImprove << std::endl
    << me_ << " number of Bad status(unbounded etc)   = " 
    << stats_->numNLPs << std::endl
    << me_ << " total time taken                      = " 
    << stats_->time << std::endl
    << me_ << " number of iterations                  = " 
    << stats_->iterations << std::endl
    << me_ << " Best Objective Value                  = " 
    << stats_->bestObjValue << std::endl;
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
