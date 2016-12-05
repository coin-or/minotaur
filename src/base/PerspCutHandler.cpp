//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.cpp
 * \brief Declare the KnapCovHandler class for handling knapsack cover 
 * constraints. It generates the cuts whenever they are needed. 
 * \author Meenarli Sharma, Indian Institute of Technology
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include "PerspCutHandler.h"
#include "Option.h"
#include "Node.h"
#include "CutManager.h"
#include "Logger.h"
#include "Function.h"
#include "LinearFunction.h"
#include "PerspCon.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string PerspCutHandler::me_ = "PerspCutHandler: ";

PerspCutHandler::PerspCutHandler()
  : env_(EnvPtr()),
    minlp_(ProblemPtr()),
    //stats_(0),
    isFeas_(true),
    solAbsTol_(1e-5),
    solRelTol_(1e-5)
{
  // Logger is an abstract class, find a way to make this work.
  // looger_ = (LoggerPtr) new Logger();
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  numCuts_ = 0;
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}

PerspCutHandler::PerspCutHandler(EnvPtr env, ProblemPtr minlp)
  : env_(env),
    minlp_(minlp),
    //stats_(0),
    isFeas_(true),
    solAbsTol_(1e-5),
    solRelTol_(1e-5)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  numCuts_ = 0;
  // Initialize logger.
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("handler_log_level")->getValue());
  // Initialize statistics.
  //stats_ = new PCStats();
  //stats_->cuts = 0;
  //stats_->time = 0.0;
  //Identifying constraints amenable to perspective reformulation in 
  //the problem
}

void PerspCutHandler::perspList()
{
  persplist_ = (PerspConPtr) new PerspCon(minlp_, env_); 
  persplist_ ->generateList();; 
  cons_ = persplist_->getPerspCons();
  binvar_ = persplist_->getConsBinVar();
  return;
}

PerspCutHandler::~PerspCutHandler()
{
  //if (stats_) {
    ////writeStats(logger_->MsgStream(LogInfo));
    //delete stats_;
  //}
  env_.reset();
  minlp_.reset();
}

bool PerspCutHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &, 
                                 double &)
{
  isFeas_=true;
  if (cons_.size()>=1) {
    // Get primal solution.
    const double *x = sol->getPrimal();
    // Evaluation of constraint for a given solution.
    double act = 0.0;
    int error = 0;
    // Now, we check all perspective cut constraints if the current 
    // relaxation solution violates any of them.
    // Vector of perspective constraints.
    // Temporary constraint holder.
    ConstConstraintPtr c;
    for (UInt it=0; it!=cons_.size(); ++it) {
      c = cons_[it];
      act = c->getActivity(x, &error);
      if(error==0) {
        if( (act > c->getUb() + solAbsTol_ &&  
             act > c->getUb() + (fabs(c->getUb())*solRelTol_))  ||  
            ( act < c->getLb() - solAbsTol_ &&  
              act < c->getLb() - (fabs(c->getLb())*solRelTol_)) ) { 
#if SPEW

          logger_->msgStream(LogDebug)
            << me_ << "constraint not feasible" << std::endl
            << me_;
          c->write(logger_->msgStream(LogDebug2));
          logger_->msgStream(LogDebug)<< me_ << "activity = " << act << std::endl;
#endif
          isFeas_ = false;
          return false;
        }   
      }   else {
        logger_->msgStream(LogError) << me_ 
          << "Constraint not defined at this point"<< std::endl;
        isFeas_ = false;
        return false;
      }   
    }
  }
  return true;
}

void PerspCutHandler::separate(ConstSolutionPtr sol, NodePtr n,
                               RelaxationPtr rel, CutManager * ,
                               SolutionPoolPtr, bool *,
                               SeparationStatus * status)
{
  //MS:These two variables are created for the sake of isFeasible funtion

  //generating perspective cuts only for root node
  if (n->getId() == 0) {
  bool binint;
  double binsol, act, lpvio;
  ConstConstraintPtr c;
  ConstraintPtr newc;
  UInt binindex, cutcount = 0;
  const double * x = sol->getPrimal();
  int error = 0;
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  FunctionPtr f;

  PerspCutGeneratorPtr pcg; 
  for (UInt it=0; it!=cons_.size(); ++it) {
    c = cons_[it];
    binindex = binvar_[it]->getIndex();
    binsol = x[binindex];
    //PC is not generated if binary variable of the
    // constraint has value 0 or 1
      
    binint = fabs(floor(binsol+0.5)-binsol);
    if (binint < intTol_) {
      continue;
    }
    act = c->getActivity(x, &error);
    if(error==0) {
      if( (act > c->getUb() + solAbsTol_ &&  
           act > c->getUb() + (fabs(c->getUb())*solRelTol_))  ||  
          ( act < c->getLb() - solAbsTol_ &&  
            act < c->getLb() - (fabs(c->getLb())*solRelTol_)) ) { 
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "constraint not feasible" 
          << std::endl<< me_;
        c->write(logger_->msgStream(LogDebug2));
        logger_->msgStream(LogDebug)<< me_ << "activity = " << act << std::endl;
#endif

        pcg = (PerspCutGeneratorPtr) new PerspCutGenerator(rel->getNumVars(), 
                                                       sol, c, binvar_[it]);
        pcg->generateCut();
        lf = pcg->getPFunction();
        lpvio = std::max(lf->eval(x), 0.0);
        // Adding cut only if violates existing solution
        if (lpvio > solAbsTol_) {
          f = (FunctionPtr) new Function(lf);
          newc = rel->newConstraint(f, -INFINITY, 0.0, "perspective_cut");
          numCuts_++;
          cutcount++;
          f.reset();
        }
#if SPEW
        logger_->msgStream(LogDebug) << me_ <<" Perspective cut: " << std::endl
          << std::setprecision(9);
        newc->write(logger_->msgStream(LogDebug));
#endif
        pcg.reset();  
        newc.reset();
      } else {
#if SPEW
        logger_->msgStream(LogDebug) << me_ 
          << "constraint is feasible so not considered for perspective cut" << std::endl
          << me_;
        c->write(logger_->msgStream(LogDebug2));
        logger_->msgStream(LogDebug)<< me_ << "activity = " << act << std::endl;
#endif       
        continue;
      }
    }   else {
      logger_->msgStream(LogError) << me_ 
        << "Constraint not defined at this point"<< std::endl;
      continue;
    }   
  }
  
  if (cutcount >= 1) {
    *status = SepaResolve;
  }
 }
  return;
}

std::string PerspCutHandler::getName() const
{
  return "PerspCutHandler (Perspective cuts)";
}

void PerspCutHandler::writeStats(std::ostream &out) const
{
  if (cons_.size() > 0) {
    // Problem not amenable to perspective reformulation
    out << me_ << "Is problem amenable to perspective reformulation: 1" << std::endl;
    out << me_ << "No. of perspective amenable constraints: "<< cons_.size() << std::endl;
    out << me_ << "number of perspective cuts added = " << numCuts_ << std::endl;
  return;
  } else {
    // Problem amenable to perspective reformulation
    out << me_ << "Is problem amenable to perspective reformulation: 0" << std::endl;
    return;
  
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
