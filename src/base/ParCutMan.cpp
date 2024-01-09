// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2024 The Minotaur Team.
// 

/**
 * \file ParCutMan.cpp
 * \brief Implement the methods of ParCutMan class. 
 * \author Meenarli Sharma, Prashant Palkar, IIT Bombay
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Function.h"
#include "Cut.h"
#include "ParCutMan.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Problem.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

typedef std::list<CutPtr>::const_iterator CCIter;
typedef std::vector<ConstraintPtr>::const_iterator ConstIter;

const std::string ParCutMan::me_ = "ParCutMan: "; 

ParCutMan::ParCutMan()
  : absTol_(1e-6),
    env_(EnvPtr()),   // NULL
    maxDisCutAge_(3),
    maxInactCutAge_(1),
    p_(ProblemPtr())  // NULL
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


ParCutMan::ParCutMan(EnvPtr env, ProblemPtr p)
  : absTol_(1e-6),
    env_(env),
    maxDisCutAge_(1),
    maxInactCutAge_(1),
    p_(p)
{
  logger_ = env->getLogger();
}


ParCutMan::~ParCutMan()
{
  for (CutList::iterator it=pool_.begin(); it != pool_.end(); ++it) {
    delete (*it);
  }
  pool_.clear();
  enCuts_.clear();
}


UInt ParCutMan::getNumCuts() const
{
  return getNumEnabledCuts() + getNumDisabledCuts() + getNumNewCuts();
}


UInt ParCutMan::getNumEnabledCuts() const
{
  return enCuts_.size();
}


UInt ParCutMan::getNumDisabledCuts() const
{
  return disCuts_.size();
}


UInt ParCutMan::getNumNewCuts() const
{
 
  return newCuts_.size();
}


void ParCutMan::postSolveUpdate(ConstSolutionPtr sol, EngineStatus)
{
  UInt n = p_->getNumVars();
  const double *x = new double[n];
  x = sol->getPrimal();
  CutPtr con;
  double viol = 0;
  bool del_const = false;
  CutList cpyrel;
  CutInfo *cinfo;

  int num_dels = 0;
  int err;

  for (CCIter it = enCuts_.begin(); it != enCuts_.end(); ++it) {
    con = *it;
    con->eval(x, &err);
    cinfo = con->getInfo();
    if (viol > -absTol_) {
      ++(cinfo->numActive);
      cinfo->cntSinceActive = 0;
    } else {
      ++(cinfo->numActive);
    }
    if (cinfo->cntSinceActive > maxInactCutAge_ &&
        false == cinfo->neverDelete) {
      addToPool_(con);
      cinfo->cntSinceViol = 0;
      p_->markDelete(con->getConstraint());
      del_const = true;
      num_dels++;
    } else {
      cpyrel.push_back(con);
    }
  }  
  enCuts_.clear();
  enCuts_ = cpyrel;

  if (del_const == true){
    p_->delMarkedCons();
  }

  cpyrel.clear();
}


void ParCutMan::separate(ProblemPtr p, ConstSolutionPtr sol, bool *, UInt *)
{
  UInt n = p->getNumVars();
  const double *x = new double[n];
  x = sol->getPrimal();
  CutPtr con;
  double viol = 0;
  CutList cpypool;
  int err;
  CutInfo *cinfo;
 
  for (CCIter it=pool_.begin(); it != pool_.end(); ++it)
  {
    con = *it;
    err = 0;
    con->eval(x, &err);
    cinfo = con->getInfo();
    if (viol < -absTol_) 
    {
      ++(cinfo->cntSinceViol);
      cpypool.push_back(con);
    } else if (viol > absTol_) {
      addToRel_(con, true);
    } else if (con->getInfo()->cntSinceActive <= maxDisCutAge_ ||
               con->getInfo()->neverDelete==true) {
      cpypool.push_back(con);
    }
  }
  pool_.clear();
  pool_ = cpypool;
  cpypool.clear();
}


std::vector<ConstraintPtr > ParCutMan::getPoolCons()
{
  std::vector<ConstraintPtr > cp;
  for (CCIter it=pool_.begin(); it != pool_.end(); ++it) {
    cp.push_back((*it)->getConstraint());
  }
  return cp;
}


void ParCutMan::addCut(CutPtr c)
{
  addToRel_(c, true);
}


void ParCutMan::addCuts(CutVectorIter cbeg, CutVectorIter cend)
{
  for (CutVectorIter it=cbeg; it!=cend; ++it) {
    addCut(*it);
  }
}


void ParCutMan::addToRel_(CutPtr cut, bool )
{
  enCuts_.push_back(cut);
  ++(cut->getInfo()->numActive);
  cut->getInfo()->cntSinceActive = 0;
}


void ParCutMan::addToPool_(CutPtr cut)
{
  pool_.push_back(cut);
}


void ParCutMan::addCutToPool(CutPtr cut)
{
  pool_.push_back(cut);
}

void ParCutMan::write(std::ostream &out) const
{
  out << me_ << "Nothing to write" << std::endl;
}


void ParCutMan::writeStats(std::ostream &out) const
{
  out << me_ << "No stats availale" << std::endl;
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
