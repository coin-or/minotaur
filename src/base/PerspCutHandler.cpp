//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.cpp
 * \brief Declare PerspCutHandler class to generate perspective cuts.
 * \author Meenarli Sharma, Indian Institute of Technology Bombay
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include "PerspCutHandler.h"
#include "LinearFunction.h"
#include "CutManager.h"
#include "Environment.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "Constraint.h"
#include "Function.h"
#include "Solution.h"
#include "Option.h"
#include "Logger.h"
#include "Node.h"

using namespace Minotaur;

typedef std::vector<ConstraintPtr>::const_iterator CCIter;
const std::string PerspCutHandler::me_ = "PerspCutHandler: ";

PerspCutHandler::PerspCutHandler()
  : env_(EnvPtr()),  minlp_(ProblemPtr()), numCuts_(0), cons_(0), binvar_(0),
  sType_(0), nviv_(0), lviv_(0), indi_(1)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
}


PerspCutHandler::PerspCutHandler(EnvPtr env, ProblemPtr minlp,
                                 std::vector<ConstConstraintPtr> cpr,
                                 std::vector<ConstVariablePtr> bpr,
                                 std::vector<char> spr,
                                 std::vector<std::vector<double> > nviv,
                                 std::vector<std::vector<double> > lviv,
                                 bool indi)
  : env_(env), minlp_(minlp), numCuts_(0), cons_(cpr), binvar_(bpr),
  sType_(spr), nviv_(nviv), lviv_(lviv), indi_(indi)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  solAbsTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
  solRelTol_ = env_->getOptions()->findDouble("solRel_tol")->getValue();
  logger_ = (LoggerPtr) new Logger((LogLevel)env->getOptions()->
                                   findInt("handler_log_level")->getValue());
}


bool PerspCutHandler::generateCut(RelaxationPtr rel, ConstSolutionPtr s,
                                  UInt itn)
{
  int  error = 0, errorgr = 0;
  const double * x = s->getPrimal();
  ConstVariablePtr bp = rel->getRelaxationVar(binvar_[itn]);
  UInt i, binindex = bp->getIndex(), nvars = minlp_->getNumVars();
 
  ConstConstraintPtr cp = cons_[itn];
  FunctionPtr f = cp->getFunction();
  double * cgrad = new double[nvars], * y = new double[nvars];
 
  // Generate solution y=(x/z,1), x are continuous and z is binary variable
  std::fill(y, y + nvars, 0);
  y = prVars(s, nvars, itn);
 
  std::fill(cgrad, cgrad + nvars, 0);
  f->evalGradient(y, cgrad, &errorgr);
 
  if (errorgr == 0 && error ==0) {
    UInt varindex = 0;
    FunctionPtr fnew;
    ConstraintPtr newc;
    ConstVariablePtr v;
    double newBound = 0;
    double conseval, lpvio;
    const LinearFunctionPtr lfn = cp->getLinearFunction();
    const QuadraticFunctionPtr qfn = cp->getQuadraticFunction();
    const NonlinearFunctionPtr nlfn = cp->getNonlinearFunction();
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();

    if (sType_[itn] == '2') {
      for (VariableGroupConstIterator vt=lfn->termsBegin(); vt!=lfn->termsEnd();
           ++vt) {
        v = rel->getRelaxationVar(vt->first);
        lf->addTerm(v, vt->second);
      }
      conseval = nlfn->eval(y, &error);
    } else {
      conseval = f->eval(y, &error);
    }

    i = 0;
    if (nlfn) {
      for (VarSetConstIterator vt=nlfn->varsBegin(); vt!=nlfn->varsEnd();
           ++vt) {
        v = rel->getRelaxationVar(*vt);
        varindex = v->getIndex();
        lf->addTerm(v, cgrad[varindex]);
        conseval += (cgrad[varindex] *((nviv_[itn][i]-x[varindex])/x[binindex])); 
        newBound += (cgrad[varindex] *nviv_[itn][i]); 
        i++;
      }
    } else if (qfn) {
      for (VarIntMapConstIterator vt=qfn->varsBegin(); vt!=qfn->varsEnd(); ++vt) {
        v = rel->getRelaxationVar(vt->first);
        varindex = v->getIndex();
        lf->addTerm(v, cgrad[varindex]);
        conseval += (cgrad[varindex] 
                     *((nviv_[itn][i]-x[varindex])/x[binindex]));
        newBound += (cgrad[varindex] *nviv_[itn][i]); 
        i++;
      }
    }

    if (lfn && (sType_[itn] == '1')) {
      i = 0;
      for (VariableGroupConstIterator vt=lfn->termsBegin(); vt!=lfn->termsEnd();
           ++vt) {
        v = rel->getRelaxationVar(vt->first);
        varindex = v->getIndex();
        if(varindex != binindex){
          lf->addTerm(v, cgrad[varindex]);
          conseval += (cgrad[varindex] 
                       *((lviv_[itn][i]-x[varindex])/x[binindex]));
          newBound += (cgrad[varindex] *lviv_[itn][i]); 
          i++;
        }
      }
    }   
    
    if (cp->getUb() != +INFINITY) {
      conseval-=(cp->getUb());
      lf->addTerm(bp, conseval);
      lpvio = std::max(lf->eval(s->getPrimal()) - newBound, 0.0);
    } else {
      conseval-=(cp->getLb());
      lf->addTerm(bp, conseval);
      lpvio = std::max(newBound - lf->eval(s->getPrimal()), 0.0);
    }
 
    if (lpvio > solAbsTol_) {
      fnew = (FunctionPtr) new Function(lf);
      if(cp->getUb() != +INFINITY) {
        newc = rel->newConstraint(fnew, -INFINITY, newBound, "perspective_cut");
      } else {
        newc = rel->newConstraint(fnew, newBound, +INFINITY, "perspective_cut");
      }
#if SPEW
      logger_->msgStream(LogDebug) << me_ <<" Perspective cut: " << std::endl
        << std::setprecision(9);
      newc->write(logger_->msgStream(LogDebug));
#endif
      delete [] y;
      delete [] cgrad;
      return true;
    }
  } else {
    logger_->msgStream(LogError) << me_ <<" Gradient or nonlinear part of the"
      <<  " function is not defined at the current point" 
      <<std::endl;
  }
  
  delete [] y;
  delete [] cgrad;
  return false;
}


std::string PerspCutHandler::getName() const
{
  return "PerspCutHandler (Perspective Reformulation)";
}


bool PerspCutHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr, 
                                 bool &, double & )
{
  int error=0;
  UInt binindex, nvars = minlp_->getNumVars();
  FunctionPtr f;
  double act, sb;
  ConstConstraintPtr c;
  const double *x = sol->getPrimal();
  double * y = new double[nvars];

  for (UInt it= 0; it!=cons_.size(); ++it) { 
    act = 0;
    binindex = binvar_[it]->getIndex();
    sb = x[binindex];
    if (sb < intTol_) {
      continue;
    }
      
    c = cons_[it];
    f = c->getFunction();
    std::fill(y, y + nvars, 0);
    y = prVars(sol, nvars, it);
    act = sb * (f->eval(y, &error));
    
    if (error==0) {
      if ((act > (c->getUb())*sb + solAbsTol_ && 
           act > (c->getUb()*sb) + (fabs((c->getUb())*sb)*solRelTol_))  ||
          (act < (c->getLb())*sb - solAbsTol_ && 
            act < (c->getLb())*sb - (fabs((c->getLb())*sb)*solRelTol_))) {
#if SPEW
        logger_->msgStream(LogDebug) 
          << me_ << "constraint not feasible" << std::endl
          << me_;
        c->write(logger_->msgStream(LogDebug));
        logger_->msgStream(LogDebug) 
          << me_ << "activity = " << act << std::endl;  
#endif
        delete [] y;
        return false;
      }
    } else {
      logger_->msgStream(LogError) << me_ 
        << "Constraint not defined at this point"<< std::endl;
      delete [] y;
      return false;
    }
  }
  delete [] y;
#if SPEW
  logger_->msgStream(LogDebug) << me_ 
    << "All perspective amenable constraints looks feasible" << std::endl;
#endif
  return true; 
}


double * PerspCutHandler::prVars(ConstSolutionPtr sol, UInt nvars, UInt itn)
{
  double sb;
  VariablePtr v;
  UInt i, binindex, indexvar;
  ConstConstraintPtr c = cons_[itn];
  const double *x = sol->getPrimal();
  const LinearFunctionPtr lf = c->getLinearFunction();
  const QuadraticFunctionPtr qf = c->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = c->getNonlinearFunction();
  
  // Generate solution y=(x/z,1), x are continuous and z is binary variable
  double * y = new double[nvars];
  std::fill(y, y + nvars, 0);
  
  binindex = binvar_[itn]->getIndex();
  sb = x[binindex];
  i = 0;
  if (nlf) {
    for (VarSetConstIterator vt=nlf->varsBegin(); vt!=nlf->varsEnd(); ++vt) {
      v = *vt;
      indexvar = v->getIndex();
      y[indexvar] = (x[indexvar] - (1-sb)*nviv_[itn][i])/sb;
      i++;
    }
  } else if (qf) {
    for (VarIntMapConstIterator vt=qf->varsBegin(); vt!=qf->varsEnd(); ++vt) {
      v = vt->first;
      indexvar = v->getIndex();
      y[indexvar] = (x[indexvar] - (1-sb)*nviv_[itn][i])/sb;
      i++;
    }
  }

  if (lf) {
    i = 0;
    for (VariableGroupConstIterator vt=lf->termsBegin(); vt!=lf->termsEnd(); ++vt) {
      v = vt->first;
      indexvar = v->getIndex();
      if(indexvar != binindex){
        if (sType_[itn] == '1') {
          y[indexvar] = (x[indexvar]- (1-sb)*lviv_[itn][i])/sb;
        } else {
          y[indexvar] = (x[indexvar])/sb;
        }
        i++;
      }
    }
    y[binindex] = 1;
  }   
  return y; 
}


void PerspCutHandler::separate(ConstSolutionPtr sol, NodePtr n,
                               RelaxationPtr rel, CutManager * ,
                               SolutionPoolPtr, bool *,
                               SeparationStatus * status)

{
  // Generating perspective cuts only at root node
  //if (n->getId() == 0) {
    int error=0;
    bool newcut;
    FunctionPtr f;
    double act, sb;
    VariablePtr v, v1;
    ConstraintPtr newc;
    ConstConstraintPtr c;
    const double *x = sol->getPrimal();
    UInt binindex, cutsCount = 0, nvars = minlp_->getNumVars();
    double * y = new double[nvars];
 
    //PC is not generated if binary variable of the
    // constraint has value 0
    for (UInt it=0; it!=cons_.size(); ++it) {
      act = 0;
      binindex = binvar_[it]->getIndex();
      sb = x[binindex];
      
      if (fabs(sb) < intTol_) {
        continue;
      } else if ((fabs(1 - sb) < intTol_) && indi_ == 0) {
        continue;
      }
        
      c = cons_[it];
      f = c->getFunction();
      std::fill(y, y + nvars, 0);
      y = prVars(sol, nvars, it);
      act = sb * (f->eval(y, &error));
      
      if (error==0) {
        if ((act > (c->getUb())*sb + solAbsTol_ && 
             act > (c->getUb()*sb) + (fabs((c->getUb())*sb)*solRelTol_))  ||
            (act < (c->getLb())*sb - solAbsTol_ && 
              act < (c->getLb())*sb - (fabs((c->getLb())*sb)*solRelTol_))) {
#if SPEW
          logger_->msgStream(LogDebug) 
            << me_ << "constraint not feasible" << std::endl << me_;
          c->write(logger_->msgStream(LogDebug));
          logger_->msgStream(LogDebug) << me_ << "activity = " << act 
            << std::endl;  
#endif
          newcut = generateCut(rel, sol, it);
          if (newcut) {
            numCuts_++;
            cutsCount++;
          }    
        }
      } else {
        logger_->msgStream(LogError) << me_ 
          << "Constraint not defined at this point"<< std::endl;
      }
    }  
   
    if (cutsCount > 0) {
      *status = SepaResolve;
    }
    delete [] y;
  //}
  return;
}


void PerspCutHandler::writeStats(std::ostream &out) const
{
  if (cons_.size() > 0) {
      out << me_ << "No. of perspective cuts added = " << numCuts_ << std::endl;
  }
  return;
}
 

PerspCutHandler::~PerspCutHandler()
{
  env_.reset();
  minlp_.reset();
  logger_.reset();
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
