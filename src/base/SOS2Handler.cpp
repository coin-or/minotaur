//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2013 The MINOTAUR Team.
//

/**
 * \file SOS2Handler.cpp
 * \brief Declare the SOS2Handler class for handling SOS type II constraints.
 * It checks integrality and provides branching candidates. Does
 * not do any presolving and cut-generation.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iomanip>

#include "MinotaurConfig.h"
#include "BrCand.h"
#include "Branch.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "Logger.h"
#include "LinMods.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "SOS.h"
#include "SOS2Handler.h"
#include "SOSBrCand.h"
#include "Variable.h"

//#define SPEW 1

using namespace Minotaur;
const std::string SOS2Handler::me_ = "SOS2Handler: ";

SOS2Handler::SOS2Handler(EnvPtr env, ProblemPtr problem)
  : env_(env)
{
  logger_ = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
                                   findInt("handler_log_level")->getValue());
  //logger_ = (LoggerPtr) new Logger(LogDebug2);
  zTol_   = 1e-6;

  if (false == env_->getOptions()->findBool("modify_rel_only")->getValue()
      &&  (0 < problem_->getNumSOS2() + problem_->getNumSOS2())) {
    logger_->ErrStream() << "Can not handle SOS constraints if "
                         << "modify_rel_only flag is false" 
                         << std::endl;
    assert(!"error initializing sos constraint");
  }
  problem_  = problem;
}


SOS2Handler::~SOS2Handler()
{
  problem_.reset();
  env_.reset();
  logger_.reset();
}


Bool SOS2Handler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel,
                             Bool &)
{
  SOSPtr sos;
  bool is_feas = true;
  const Double* x = sol->getPrimal();

  for (SOSConstIterator siter=rel->sos2Begin(); siter!=rel->sos2End();
       ++siter) {
    sos = *siter;
    is_feas = isXFeasible_(x, sos);
    if (false==is_feas) {
      //is_inf = isInf_(sos);
    }
    break;
  }
#if SPEW
    logger_->MsgStream(LogDebug) << me_ << "isFeasible "
                                 << sos->getName() << " is_feas = " << is_feas
                                 << std::endl;
#endif
  return is_feas;
}


Bool SOS2Handler::isXFeasible_(const Double *x, SOSPtr sos)
{
  bool new_nz_allowed = true;
  bool is_feas = true;
  int nz = 0;
  double xval;

  for (VariableConstIterator viter = sos->varsBegin(); viter!=sos->varsEnd();
       ++viter) {
    xval = x[(*viter)->getIndex()];
    if (xval>zTol_) {
      ++nz;
      if (false==new_nz_allowed) {
        is_feas = false;
        break;
      } else if (2==nz) {
        new_nz_allowed = false;
      }
    } else if (nz>0) {
      new_nz_allowed = false;
    }
  }
  return is_feas;
}

#if 0
{
  // Find two non-adjacent variables that are fixed to nonzero values. If
  // found, then this node can be pruned.
  nz = 0;
  for (VariableConstIterator viter = sos->varsBegin();
       viter!=sos->varsEnd(); ++viter) {
    if ((*viter)->getLb()>zTol_ || (*viter)->getUb()<-zTol_) {
      if (2==nz) {
        is_feas = false;
        *is_inf = true;
        break;
      } else {
        ++nz;
      }
    } else if (nz>0) {
      nz = 2;
    }
  }
}
#endif


Branches SOS2Handler::getBranches(BrCandPtr cand, DoubleVector &, 
                                  RelaxationPtr, SolutionPoolPtr)
{
  SOSBrCandPtr scand = boost::dynamic_pointer_cast <SOSBrCand> (cand);
  LinModsPtr mod;
  VarBoundModPtr bmod;

  BranchPtr branch1, branch2;
  Branches branches = (Branches) new BranchPtrVector();

  mod = (LinModsPtr) new LinMods();
  for (VariableConstIterator vit=scand->lVarsBegin();vit!=scand->lVarsEnd();
       ++vit) {
    bmod = (VarBoundModPtr) new VarBoundMod(*vit, Upper, 0.0);
    mod->insert(bmod);
  }
  branch1 = (BranchPtr) new Branch();
  branch1->addMod(mod);
  branch1->setActivity(scand->getLSum());

  mod = (LinModsPtr) new LinMods();
  for (VariableConstIterator vit=scand->rVarsBegin();vit!=scand->rVarsEnd();
       ++vit) {
    bmod = (VarBoundModPtr) new VarBoundMod(*vit, Upper, 0.0);
    mod->insert(bmod);
  }
  branch2 = (BranchPtr) new Branch();
  branch2->addMod(mod);
  branch2->setActivity(scand->getRSum());

  branches->push_back(branch1);
  branches->push_back(branch2);
  return branches;
}


void SOS2Handler::getBranchingCandidates(RelaxationPtr rel, 
                                         const std::vector< Double > &x,
                                         ModVector &mods, BrCandSet & cands,
                                         Bool & is_inf)
{
  int lnz, nv, nnz, nspos;
  double lsum, rsum, nzsum;
  const double *xa = &(x[0]);
  SOSPtr sos;
  SOSConstIterator siter;
  VariablePtr var;
  VariableConstIterator viter, viter2;
  VarVector lvars, rvars;
  SOSBrCandPtr br_can;
  VarBoundModPtr bmod;

  for (siter=rel->sos2Begin(); siter!=rel->sos2End(); ++siter) {
    sos = *siter;
    if (isXFeasible_(xa, sos)) {
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << sos->getName() << " is not a "
                                   << " branching candidate." << std::endl;
#endif
      continue;
    }

    lvars.clear();
    rvars.clear();

    getSumN_(sos, x, &nzsum, &nnz, &nv, &nspos);
    // TODO: this can be avoided if the node processor is better designed.
    if (2==nspos) {
      // If two vars have bounds away from zero, then all other vars can be
      // fixed to zero.  Send this mod back. 
      for (viter=sos->varsBegin(); viter!=sos->varsEnd(); ++viter) {
        if ((*viter)->getLb()>zTol_) {
          ++viter; ++viter;
          break;
        }
        bmod = (VarBoundModPtr) new VarBoundMod((*viter), Upper, 0.0);
        mods.push_back(bmod);
      }
      for (; viter!=sos->varsEnd(); ++viter) {
        bmod = (VarBoundModPtr) new VarBoundMod((*viter), Upper, 0.0);
        mods.push_back(bmod);
      }
      cands.clear();
      return;
    } else if (1==nspos) {
      // If one var has bounds away from zero, then only one of its neighbors
      // can be nonzero. Others must be zero. 
      // case 1: First unfixed var is bounded away from zero. Then all vars
      // except first two must be fixed to zero.
      viter = sos->varsBegin();
      for (viter=sos->varsBegin(); viter!=sos->varsEnd(); ++viter) {
        if ((*viter)->getLb()>zTol_) {
          ++viter;
          for (; viter!=sos->varsEnd(); ++viter) {
            bmod = (VarBoundModPtr) new VarBoundMod((*viter), Upper, 0.0);
            mods.push_back(bmod);
          }
          cands.clear();
          return;
        } else if ((*viter)->getUb()>zTol_) {
          break;
        }
      }

      // case 2: Last var is bounded away from zero. Then all vars except
      // last two must be fixed to zero.
      viter = sos->varsEnd();
      --viter;
      for ( ; viter!=sos->varsBegin(); --viter) {
        if ((*viter)->getLb()>zTol_) {
          --viter;
          for (; viter!=sos->varsBegin(); --viter) {
            bmod = (VarBoundModPtr) new VarBoundMod((*viter), Upper, 0.0);
            mods.push_back(bmod);
          }
          bmod = (VarBoundModPtr) new VarBoundMod((*viter), Upper, 0.0);
          mods.push_back(bmod);
          cands.clear();
          return;
        } else if ((*viter)->getUb()>zTol_) {
          break;
        }
      }

      // case 3: A var in the middle is bounded away from zero.
      for (viter=sos->varsBegin(); viter!=sos->varsEnd(); ++viter) {
        if ((*viter)->getLb()>zTol_) {
          break;
        }
      }
      // If we have 0 nz 0 0 ? var ? 0 0 nz 0 
      // Again, return a mod.
      if (nnz>3 || 
          x[(*(viter-1))->getIndex()]<zTol_ ||
          x[(*(viter+1))->getIndex()]<zTol_) {
        viter2 = viter; --viter2; --viter2;
        for (; viter2!=sos->varsBegin(); --viter2) {
          if ((*viter2)->getUb()>zTol_) {
            bmod = (VarBoundModPtr) new VarBoundMod((*viter2), Upper, 0.0);
            mods.push_back(bmod);
          }
        }
        if ((*viter2)->getUb()>zTol_) {
          bmod = (VarBoundModPtr) new VarBoundMod((*viter2), Upper, 0.0);
          mods.push_back(bmod);
        }
        viter2 = viter; ++viter2; ++viter2;
        for (; viter2!=sos->varsEnd(); ++viter2) {
          if ((*viter2)->getUb()>zTol_) {
            bmod = (VarBoundModPtr) new VarBoundMod((*viter2), Upper, 0.0);
            mods.push_back(bmod);
          }
        }
        cands.clear();
        return;
      } else {
        // branch around viter.
        // TODO: double check.
        lsum = 0.;
        rsum = 0.;
        for (viter2=sos->varsBegin(); viter2!=viter; ++viter2) {
          if ((*viter2)->getUb()>zTol_) {
            lvars.push_back(*viter2);
            lsum += x[(*viter2)->getIndex()];
          }
        }
        viter2=viter;
        ++viter2;
        for (;viter2!=sos->varsEnd(); ++viter2) {
          if ((*viter2)->getUb()>zTol_) {
            rsum += x[(*viter2)->getIndex()];
            rvars.push_back(*viter2);
          }
        }
      }
    }

    // Most common case, nspos = 0.
    // We want equal distribution of unfixed variables in the branches. We
    // also want each branch to cut off the current solution.
    //
    // We will pick the branching-variable v0, such that there is atleast one
    // nonzero x on either side and the number of unfixed variables on either
    // side is as close as possible. We will not care about the magnitude of
    // violation (or fractionality) in each branch.
    if (lvars.size() + rvars.size()<1) {
      lnz = 0; // number of vars in left branch that have nonzero value in sol.
      lsum = 0;
      rsum = 0;
      for (viter=sos->varsBegin(); viter!=sos->varsEnd(); ++viter) {
        var = *viter;
        if (lnz>0 && lvars.size() >= floor(nv/2)) {
          break;
        } else if (lnz == nnz-1 && x[(*(viter+1))->getIndex()]>zTol_) {
          break;
        } else if (lnz == nnz-2 && x[var->getIndex()]>zTol_ &&
                   x[(*(viter+1))->getIndex()]>zTol_) {
          break;
        } else if (var->getUb()>zTol_) {
          lvars.push_back(var);
          if (x[var->getIndex()]>zTol_) {
            ++lnz;
            lsum += x[var->getIndex()];
          }
        }
      }
      ++viter;
      for (; viter!=sos->varsEnd(); ++viter) {
        var = *viter;
        if (var->getUb()-var->getLb()>zTol_) {
          rvars.push_back(*viter);
          rsum += x[var->getIndex()];
        }
      }
    }

    br_can = (SOSBrCandPtr) new SOSBrCand(sos, lvars, rvars, lsum,
                                          rsum);
    br_can->setDir(DownBranch);
    br_can->setScore(20.0*(lvars.size())*(rvars.size()));
      
    if(cands.insert(br_can).second == false) {
      std::cout << "trouble ehere\n";
    }

#if SPEW
    logger_->MsgStream(LogDebug) << me_ << sos->getName() << " is a "
                                 << " branching candidate." << std::endl
                                 << me_ << "left branch has variables ";
    for (viter = lvars.begin(); viter!=lvars.end(); ++viter) {
      logger_->MsgStream(LogDebug) << (*viter)->getName() << " ";
    }
    logger_->MsgStream(LogDebug) << std::endl
                                 << me_ << "left sum = " << lsum
                                 << std::endl
                                 << me_ << "right branch has variables ";
    for (viter = rvars.begin(); viter!=rvars.end(); ++viter) {
      logger_->MsgStream(LogDebug) << (*viter)->getName() << " ";
    }
    logger_->MsgStream(LogDebug) << std::endl
                                 << me_ << "right sum = " << nzsum - lsum
                                 << std::endl;
#endif

  }
  is_inf = false;
}


ModificationPtr SOS2Handler::getBrMod(BrCandPtr cand, DoubleVector &,
                                      RelaxationPtr , BranchDirection dir) 
{
  LinModsPtr mod = (LinModsPtr) new LinMods();
  SOSBrCandPtr scand = boost::dynamic_pointer_cast <SOSBrCand> (cand);
  VarBoundModPtr bmod;

  if (dir==DownBranch) {
    for (VariableConstIterator vit = scand->lVarsBegin();
         vit!=scand->lVarsEnd(); ++vit) {
      bmod = (VarBoundModPtr) new VarBoundMod((*vit), Upper, 0.0);
      mod->insert(bmod);
    }
  } else {
    for (VariableConstIterator vit = scand->rVarsBegin();
         vit!=scand->rVarsEnd(); ++vit) {
      bmod = (VarBoundModPtr) new VarBoundMod((*vit), Upper, 0.0);
      mod->insert(bmod);
    }
  }
  return mod;
}


std::string SOS2Handler::getName() const
{
  return "SOS2Handler (Handling SOS2 constraints).";
}


void SOS2Handler::getSumN_(SOSPtr sos, const DoubleVector &x, double *sum,
                           int *nnz, int *nv, int *nspos)
{
  double xval;
  VariablePtr var;

  *sum = 0.0;
  *nnz = 0;
  *nv = 0;
  *nspos = 0;

  for (VariableConstIterator viter = sos->varsBegin(); viter!=sos->varsEnd();
       ++viter) {
    var = *viter;
    xval = x[var->getIndex()];
    if (xval>zTol_) {
      (*sum) += xval;
      ++(*nnz);
    }
    if (var->getUb()>zTol_) {
      ++(*nv);
    }
    if (var->getLb()>zTol_) {
      ++(*nspos);
    }
    //std::cout << x[(*viter)->getIndex()] << std::endl;
  }
}


Double SOS2Handler::getTol() const
{
  return zTol_;
}


Bool SOS2Handler::isNeeded()
{
  if (problem_ &&
      false == env_->getOptions()->findBool("ignore_SOS2")->getValue()) {
    problem_->calculateSize();
    if (problem_->getSize()->SOS2Cons > 0) {
      return true;
    }
  }
#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "SOS2 Handler not required"
                               << std::endl;
#endif
  return false;
}


void SOS2Handler::relaxInitFull(RelaxationPtr, Bool *is_inf)
{
  *is_inf = false;
}


void SOS2Handler::relaxInitInc(RelaxationPtr , Bool *is_inf)
{
  *is_inf = false;
}


void SOS2Handler::relaxNodeFull(NodePtr , RelaxationPtr, Bool *is_inf)
{
  *is_inf = false;
}


void SOS2Handler::relaxNodeInc(NodePtr , RelaxationPtr , Bool *is_inf)
{
  *is_inf = false;
}


void SOS2Handler::separate(ConstSolutionPtr, NodePtr , RelaxationPtr,
                           CutManager *, SolutionPoolPtr, Bool *,
                           SeparationStatus *status)
{
  *status = SepaNone;
}


void SOS2Handler::setTol(Double tol)
{
  zTol_ = tol;
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
