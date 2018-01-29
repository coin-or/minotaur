//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.cpp
 * \brief declare PerspCutHandler class for handling constraints
 * amenable to perspective reformulation and generating perspective cuts
 *  whenever needed. 
 * \author Meenarli Sharma, Indian Institute of Technology
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include "PerspCutHandler.h"
#include "LinearFunction.h"
#include "CutManager.h"
#include "Function.h"
#include "Solution.h"
#include "PerspCon.h"
#include "Option.h"
#include "Logger.h"
#include "Node.h"

# define SPEW 0
using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string PerspCutHandler::me_ = "PerspCutHandler: ";


PerspCutHandler::PerspCutHandler()
  : env_(EnvPtr()),
    minlp_(ProblemPtr()),
    isFeas_(true),
    numCuts_(0)

{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


PerspCutHandler::PerspCutHandler(EnvPtr env, ProblemPtr minlp)
  : env_(env),
    minlp_(minlp),
    isFeas_(true),
    numCuts_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("handler_log_level")->getValue());
}


LinearFunctionPtr PerspCutHandler::generateCut(UInt rnv, ConstSolutionPtr s,
                                     ConstConstraintPtr cp, ConstVariablePtr bp)
{
 ConstVariablePtr cvp;
 const double * x = s->getPrimal();
 UInt binindex = bp->getIndex(), indexvar = 0;
 double sb = x[binindex], vsol = 0.0;

 FunctionPtr f = cp->getFunction();
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

 // Generate solution y=(x/z,1), x are continuous and z is binary variable
 binindex = bp->getIndex();
 double * y = new double[rnv];
 std::fill(y, y + rnv, 0);
 for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
   cvp = *it;
   indexvar = cvp->getIndex();
     vsol = x[indexvar];
     y[indexvar] = vsol/sb;
 }

 y[binindex] = 1;
 lf = gPCut(rnv, cp, bp, y);
 delete [] y;
 return lf;
}


std::string PerspCutHandler::getName() const
{
  return "PerspCutHandler (Perspective Reformulation)";
}


LinearFunctionPtr PerspCutHandler::gPCut(UInt rnv, ConstConstraintPtr cp,
                                        ConstVariablePtr bp, double * y)
{
  int  error = 0, errorgr = 0;
  double * cgrad = new double[rnv];
  FunctionPtr f = cp->getFunction();
  double conseval = f->eval(y, &error);
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
 
  std::fill(cgrad, cgrad + rnv, 0);
  f->evalGradient(y, cgrad, &errorgr);

  if (errorgr == 0 && error ==0) {
    UInt varindex = 0;
    ConstVariablePtr var ;
    ConstraintPtr newc;
    double gradu =0.0, gradfi = 0.0, ysolvar = 0.0;

    for (VarSetConstIterator it = f->varsBegin(); it!= f->varsEnd(); ++it) {
      var = *it; 
      varindex = var->getIndex();
      if(varindex!=bp->getIndex()){
        gradfi =cgrad[varindex];
        lf->addTerm(var, gradfi);
        ysolvar = y[varindex];
        gradu -= (gradfi * ysolvar); 
      } 
    }
    // In perspective reformulated constraint coefficient of binary variable is
    // included in conseval
    // its coefficient in original constraint minus the upper bound of the constraint
    if(cp->getUb() != +INFINITY){
      gradu-=(cp->getUb());
    } else {
      gradu+=(cp->getLb());
    }
    gradu+=conseval;
    // We can add binary variable to linearization, when we considered all
    // variables except the binary variable so that the linearization
    // coefficient of binary variable is calculated.
    lf->addTerm(bp, gradu);
  } else {
    logger_->msgStream(LogError) << me_ <<" Gradient or nonlinear part of the"
     <<  " function is not defined at the current point" 
     <<std::endl;
  }
  return lf;
}


bool PerspCutHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, bool &, 
                                 double &)
{
  isFeas_=true;
  if (cons_.size()>=1) {
    int error = 0;
    double act = 0.0;
    ConstConstraintPtr c;
    const double *x = sol->getPrimal();
    
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
            << std::endl;
          c->write(logger_->msgStream(LogDebug));
          logger_->msgStream(LogDebug)<< me_ << "activity = "<< act << std::endl;
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

bool PerspCutHandler::perspList()
{
  persplist_ = (PerspConPtr) new PerspCon(minlp_, env_); 
  persplist_ ->generateList(); 

  if(persplist_->getStatus() == true){
    cons_ = persplist_->getPerspCons();
    binvar_ = persplist_->getConsBinVar();
    persplist_->displayInfo(me_);
    return true;
  } else {
    return false;
  }
}


void PerspCutHandler::separate(ConstSolutionPtr sol, NodePtr n,
                               RelaxationPtr rel, CutManager * ,
                               SolutionPoolPtr, bool *,
                               SeparationStatus * status)
{
  // Generating perspective cuts only at root node
  if (n->getId() == 0) {
    bool binint;
    int error = 0;
    double binsol, act, lpvio;
    UInt binindex, cutCount = 0;
    const double * x = sol->getPrimal();

    FunctionPtr f;
    ConstraintPtr newc;
    ConstConstraintPtr c;
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    
    for (UInt it=0; it!=cons_.size(); ++it) {
      c = cons_[it];
      binindex = binvar_[it]->getIndex();
      binsol = x[binindex];
      const double cu = c->getUb();
      const double cl = c->getLb();
      //PC is not generated if binary variable of the
      // constraint has value 0 or 1
      binint = fabs(floor(binsol+0.5)-binsol);
      if (binint < intTol_) {
        continue;
      }
      
      act = c->getActivity(x, &error);
      if(error==0) {
        if( (act > cu + solAbsTol_ &&  
             act > cu + (fabs(cu)*solRelTol_))  ||  
            ( act < cl - solAbsTol_ &&  
              act < cl - (fabs(cl)*solRelTol_)) ) { 
#if SPEW
          logger_->msgStream(LogDebug) << me_ << "Constraint not feasible" 
            << std::endl<< std::endl;
          c->write(logger_->msgStream(LogDebug));
          logger_->msgStream(LogDebug)<< me_ << "activity = " << act << std::endl;
#endif
          
          lf = generateCut( rel->getNumVars(), sol, c, binvar_[it]);
          
          lpvio = std::max(lf->eval(x), 0.0);
          //Cut is added if it violates current solution
          if (lpvio > solAbsTol_) {
            f = (FunctionPtr) new Function(lf);
            if(cu != +INFINITY) {
              newc = rel->newConstraint(f, -INFINITY, 0.0, "perspective_cut");
            } else if(cl != -INFINITY) {
              newc = rel->newConstraint(f, 0.0, +INFINITY, "perspective_cut");
            } else {
#if SPEW   
              logger_->msgStream(LogDebug) << me_ <<" Constraint do not have"
                << " either lower or upper bound" << std::endl;
#endif
            }
            numCuts_++;
            cutCount++;
            f.reset();
            lf.reset();
          }
#if SPEW
          logger_->msgStream(LogDebug) << me_ <<" Perspective cut: " << std::endl
            << std::setprecision(9);
          newc->write(logger_->msgStream(LogDebug));
#endif
          newc.reset();
        } else {
#if SPEW
          logger_->msgStream(LogDebug) << me_ 
            << "Constraint is feasible at the current point"
            << " thus not considered for perspective cut generation."
            << std::endl;
          c->write(logger_->msgStream(LogDebug));
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
    
    if (cutCount >= 1) {
      *status = SepaResolve;
    }
  }
  return;
}


void PerspCutHandler::writeStats(std::ostream &out) const
{
  out << me_ << "No. of constraints amenable to PR: = "<< cons_.size() 
    << std::endl;
  if(cons_.size() > 0){
      out << me_ << "No. of perspective cuts added: = " << numCuts_ << std::endl;
  }
  return;
}
 


PerspCutHandler::~PerspCutHandler()
{
  env_.reset();
  minlp_.reset();
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
