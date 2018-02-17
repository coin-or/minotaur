//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
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
#include "Logger.h"
#include "NlWriter.h"
#include "NonlinearFunction.h"
#include "Option.h"
#include "Problem.h"
#include "Objective.h"
#include "ProblemSize.h"
#include "Variable.h"

#define SPEW 1

// TODO: Implement obj/constraint writing when both nlf and qf are available.

using namespace Minotaur;

const std::string NlWriter::me_ = "NlWriter: "; 

NlWriter::NlWriter() 
{
}


NlWriter::~NlWriter()
{
}


int NlWriter::write(ProblemPtr p, const std::string fname)
{
  std::ofstream of;

  p->prepareForSolve();
  p->write(std::cout);
  of.open(fname.c_str());
  if (!of.is_open()) {
    std::cerr << "cannot open nl file " << fname << " for writing";
    return 1;
  } 

  writeHeader_(p, of);
  writeCO_(p, of); // sections C, O of constraints and objective

  of.close();

  return 0;
}


int NlWriter::writeCO_(ProblemPtr p, std::ofstream &of)
{
  ConstConstraintPtr c;
  ConstVariablePtr v;
  int cnt;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;
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
    if (nlf && qf) {
      assert(!"not implemented yet!");
    } else if (nlf) {
      str = nlf->getNlString(&err);
      of << str;
    } else if (qf) {
    } else {
      of << "n0" << std::endl;
    }
  }

  return 0;
}


int NlWriter::writeHeader_(ProblemPtr p, std::ofstream &of)
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
