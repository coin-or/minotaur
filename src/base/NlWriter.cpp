//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file NlWriter.cpp
 * \brief write an nl file
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "MinotaurConfig.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "Jacobian.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "NlWriter.h"
#include "NonlinearFunction.h"
#include "Objective.h"
#include "Option.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "Variable.h"

// TODO: Implement obj/constraint writing when both nlf and qf are available.
// TODO: Test linear objective with constant

using namespace Minotaur;

const std::string NlWriter::me_ = "NlWriter: "; 

NlWriter::NlWriter(EnvPtr env) 
: env_(env)
{
}


NlWriter::~NlWriter()
{
}


int NlWriter::write(ProblemPtr p, const std::string fname)
{
  std::ofstream of;

  p->prepareForSolve();
  of.open(fname.c_str());
  if (!of.is_open()) {
    env_->getLogger()->errStream() << me_ << "could not open file " << fname
                                   << " for writing" << std::endl;
    return 1;
  } 

  header_(p, of);
  co_(p, of); // sections C, O for constraints and objective
  rb_(p, of); // sections r, b for bounds
  kjg_(p,of); // sections k, G, J for sparsity and linear part

  of.close();

  return 0;
}


int NlWriter::exp_(std::ofstream &of, QuadraticFunctionPtr qf, NonlinearFunctionPtr nlf)
{
  int err = 0;
  std::string str;
  if (nlf && qf) {
    env_->getLogger()->errStream() << me_ << "writing qf not implemented yet!"
                                   << std::endl;
  } else if (nlf) {
    str = nlf->getNlString(&err);
    if (0 == err) {
      of << str;
    } 
  } else if (qf) {
    env_->getLogger()->errStream() << me_ << "writing qf not implemented yet!"
                                   << std::endl;
  } else {
    of << "n0" << std::endl;
  }
  return err;
}


int NlWriter::co_(ProblemPtr p, std::ofstream &of)
{
  ConstConstraintPtr c;
  ObjectivePtr obj;
  int cnt;
  QuadraticFunctionPtr qf;
  NonlinearFunctionPtr nlf;
  int err=0;
  std::string str;

  // start with constraints
  cnt = 0;
  for (ConstraintConstIterator citer=p->consBegin(); citer != p->consEnd();
       ++citer,++cnt) {
    c = *citer;
    of << "C" << cnt << std::endl;
    qf = c->getQuadraticFunction();
    nlf = c->getNonlinearFunction();
    err = exp_(of, qf, nlf);
    if (0!=err) {
      env_->getLogger()->errStream() << me_ << "error in writing constraint "
                                     << c->getName() << std::endl;
      return err;
    }
  }

  // objective
  obj = p->getObjective();
  if (obj) {
    int sense = (obj->getObjectiveType() == Minimize)? 0 : 1;
    of << "O0 " << sense << std::endl;
    qf = obj->getQuadraticFunction();
    nlf = obj->getNonlinearFunction();
    err = exp_(of, qf, nlf);
    if (0!=err) {
      env_->getLogger()->errStream() << me_ << "error in writing objective"
                                     << std::endl;
    }
  }

  return err;
}


int NlWriter::header_(ProblemPtr p, std::ofstream &of)
{
// g3 0 1 0        # problem Convex_MINLP
//  80 111 1 0 40  # vars, constraints, objectives, ranges, eqns
//  10 0   # nonlinear constraints, objectives
//  0 0    # network constraints: nonlinear, linear
//  10 0 0 # nonlinear vars in constraints, objectives, both
//  0 0 0 1        # linear network variables; functions; arith, flags
//  20 0 0 0 0     # discrete variables: binary, integer, nonlinear (b,c,o)
//  230 10 # nonzeros in Jacobian, gradients
//  0 0    # max name lengths: constraints, variables
//  0 0 0 0 0      # common exprs: b,c,o,c1,o1

  int nobj =  0;
  int nrngs = 0;
  int neqns = 0;
  int nlcs  = 0;
  int nlo   = 0;  
  int nlvc  = 0;
  int nlvo  = 0;  // no. of nonlinear vars in obj
  int vo    = 0;  // total no. of vars in obj
  int nlvb  = 0;
  int binl  = 0;  // no. of linear binary variables
  int intl  = 0;  // no. of linear integer variables
  int intb  = 0;  // no. of nonlinear disc vars in both constr and obj
  int intc  = 0;  // no. of nonlinear disc vars in constr and not in obj
  int into  = 0;  // no. of nonlinear disc vars in obj and not in constr
  UInt clen = 0;  // max size of constraint names
  UInt vlen = 0;  // max size of variable names
  bool nl_in_o, nl_in_c;
  const double eps = 1e-6;
  ConstConstraintPtr c;
  ConstVariablePtr v;
  ObjectivePtr o;
  FunctionPtr ofun = FunctionPtr(); // NULL


  // count the number of ranges, equations and number of nonlinear
  // constraints
  for (ConstraintConstIterator citer=p->consBegin(); citer != p->consEnd();
       ++citer) {
    c = *citer;
    if (c->getName().length()>clen) {
      clen = c->getName().length();
    }
    if (fabs(c->getUb() - c->getLb()) < eps) {
      ++neqns;
    } else if (c->getUb() < INFINITY && c->getLb() > -INFINITY) {
      ++nrngs;
    }
    if (c->getFunctionType() != Constant
        && c->getFunctionType() != Linear) {
      ++nlcs;
    }
  }

  o = p->getObjective();
  if (o) {
    ofun = o->getFunction();

    if (ofun) {
      nobj = 1;
      vo = ofun->getNumVars();
    }

    // count number of nonlinear objectives
    if (ofun && o->getFunctionType() != Constant && 
        o->getFunctionType() != Linear) {
      nlo = 1;
    }
  }

  of << "g3 0 1 0        # problem nl_by_minotaur" << std::endl;
  of << " " << p->getNumVars() << " " << p->getNumCons() << " " << nobj
    << " " << nrngs << " " << neqns
    << "    # vars, constraints, objectives, ranges, eqns" << std::endl;
  of << " " << nlcs << " " << nlo
    << "    # nonlinear constraints, objectives" << std::endl;
  of << " 0 0    # network constraints: nonlinear, linear" << std::endl;

  // count variable types
  for (VariableConstIterator viter=p->varsBegin(); viter != p->varsEnd();
       ++viter) {
    v = *viter;
    if (v->getName().length()>vlen) {
      vlen = v->getName().length();
    }
    if (Linear == v->getFunType() || Constant == v->getFunType()) {
      if (Binary == v->getType()) {
        ++binl;
      } else if (Integer == v->getType()) {
        ++intl;
      }
      continue;
    }

    nl_in_o = false;
    nl_in_c = false;
    if (nlo>0 && !ofun->isLinearIn(v)) {
      ++nlvo;
      nl_in_o = true;
    } 

    for (ConstrSet::iterator cit=v->consBegin(); cit!=v->consEnd(); ++cit) {
      if (Linear != (*cit)->getFunction()->getType() && 
          Constant != (*cit)->getFunction()->getType()) {
        nl_in_c = true;
        ++nlvc;
        break;
      }
    }

    if (nl_in_c && nl_in_o) {
      ++nlvb;
      if (Integer == v->getType() || Binary == v->getType()) {
        ++intb;
      }
    } else if (nl_in_c && !nl_in_o &&
               (Integer == v->getType() || Binary == v->getType())) {
        ++intc;
    } else if (!nl_in_c && nl_in_o &&
               (Integer == v->getType() || Binary == v->getType())) {
        ++into;
    }
  }

  of << " " << nlvc << " " << nlvo << " " << nlvb
    << "    # nonlinear vars in constraints, objectives, both" << std::endl;

  // not clear what arith and flags are. Values based on a random sample
  // of .nl files
  of << " 0 0 0 1        # linear network variables; functions; arith, flags" << std::endl;

  of << " " << binl << " " << intl << " " << intb << " " << intc << " " 
     << into << "     # discrete variables: binary, integer, nonlinear (b,c,o)" << std::endl;

  of << " " << p->getJacobian()->getNumNz() << " " << vo
     << "        # nonzeros in Jacobian, gradients" << std::endl;


  of << " " << clen << " " << vlen
     << "    # max name lengths: constraints, variables" << std::endl;
  of << " 0 0 0 0 0      # common exprs: b,c,o,c1,o1" << std::endl;

  return 0;
}


int NlWriter::kjg_(ProblemPtr p, std::ofstream &of)
{
  ConstConstraintPtr c;
  FunctionPtr f;
  LinearFunctionPtr lf;
  int nz = p->getJacobian()->getNumNz();
  UInt *rinds = new UInt[nz];
  UInt *cinds = new UInt[nz];
  UInt *ccnt  = new UInt[p->getNumVars()];
  int tnz;
  ObjectivePtr obj;


  of << "k" << p->getNumVars()-1 << std::endl;
  // we need to count appearance of each variable. 
  for (UInt i=0; i<p->getNumVars(); ++i) {
    ccnt[i] = 0;
  }

  if (nz>0) {
    p->getJacobian()->fillRowColIndices(rinds, cinds);

    for (int i=0; i<nz; ++i) {
      ++ccnt[cinds[i]];
    }
  }

  tnz = 0;
  for (UInt i=0; i<p->getNumVars()-1; ++i) {
    of << ccnt[i]+tnz << std::endl;
    tnz = tnz+ccnt[i];
  }

  // Now write J
  tnz = 0;
  for (ConstraintConstIterator citer=p->consBegin(); citer != p->consEnd();
       ++citer) {
    c = *citer;
    f = c->getFunction();
    if (!f) {
      continue;
    }
    of << "J" << c->getIndex() << " " << f->getNumVars() << std::endl;
    lf = c->getLinearFunction();
    if (lf) {
      for (VarSetConstIter it=f->varsBegin(); it!=f->varsEnd(); ++it) {
        of << (*it)->getIndex() << " " << lf->getWeight(*it) << std::endl;
      }
    } else {
      for (VarSetConstIter it=f->varsBegin(); it!=f->varsEnd(); ++it) {
        of << (*it)->getIndex() << " " << 0 << std::endl;
      }
    }
  }

  // write G
  obj = p->getObjective();
  if (obj && obj->getFunction()) {
    f = obj->getFunction();
    of << "G0" << " " << f->getNumVars() << std::endl;
    lf = obj->getLinearFunction();
    if (lf) {
      for (VarSetConstIter it=f->varsBegin(); it!=f->varsEnd(); ++it) {
        of << (*it)->getIndex() << " " << lf->getWeight(*it) << std::endl;
      }
    } else {
      for (VarSetConstIter it=f->varsBegin(); it!=f->varsEnd(); ++it) {
        of << (*it)->getIndex() << " " << 0 << std::endl;
      }
    }
  }

  delete [] rinds;
  delete [] cinds;
  delete [] ccnt;
  return 0;
}


int NlWriter::rb_(ProblemPtr p, std::ofstream &of)
{
  ConstConstraintPtr c;
  ConstVariablePtr v;
  double lb, ub;

  if (p->getNumCons()>0) { 
    of << "r" << std::endl;
  }

  for (ConstraintConstIterator citer=p->consBegin(); citer != p->consEnd();
       ++citer) {
    c = *citer;
    lb = c->getLb();
    ub = c->getUb();
    if (lb > -INFINITY && ub < INFINITY && lb==ub) {
      of << 4 << " " << lb << std::endl;
    } else if (lb > -INFINITY && ub < INFINITY) {
      of << 0 << " " << lb << " " << ub << std::endl;
    } else if (ub < INFINITY) {
      of << 1 << " " << ub << std::endl;
    } else if (lb > -INFINITY) {
      of << 2 << " " << lb << std::endl;
    } else {
      of << 3 << std::endl;
    }
  }

  of << "b" << std::endl;
  for (VariableConstIterator viter=p->varsBegin(); viter != p->varsEnd();
       ++viter) {
    v = *viter;
    lb = v->getLb();
    ub = v->getUb();
    if (lb > -INFINITY && ub < INFINITY) {
      of << 0 << " " << lb << " " << ub << std::endl;
    } else if (ub < INFINITY) {
      of << 1 << " " << ub << std::endl;
    } else if (lb > -INFINITY) {
      of << 2 << " " << lb << std::endl;
    } else {
      of << 3 << std::endl;
    }
  }
  return 0;
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
