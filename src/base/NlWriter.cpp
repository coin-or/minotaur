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
#include "Logger.h"
#include "NlWriter.h"
#include "Option.h"
#include "Problem.h"
#include "Objective.h"
#include "ProblemSize.h"
#include "Variable.h"

#define SPEW 1

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
    of.open(fname.c_str());
    if (!of.is_open()) {
      std::cerr << "cannot open nl file " << fname << " for writing";
      return 1;
    } 

    writeHeader_(p, of);
    of.close();

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

    int nobj = (p->getObjective()) ? 1 : 0;
    int nrngs = 0;
    int neqns = 0;
    int nlcs  = 0;
    int nlo   = 0;
    int nlvc  = 0;
    int nlvo  = 0;
    int nlvb  = 0;
    bool nl_in_o, nl_in_c;
    const double eps = 1e-6;
    ConstConstraintPtr c;
    ConstVariablePtr v;
    ObjectivePtr o;
    FunctionPtr ofun;


    // count the number of ranges, equations and number of nonlinear
    // constraints
    for (ConstraintConstIterator citer=p->consBegin(); citer != p->consEnd();
         ++citer) {
      c = *citer;
      if (c->getUb() < INFINITY && c->getLb() > -INFINITY) {
        ++nrngs;
      }
      if (fabs(c->getUb() - c->getLb()) < eps) {
        ++neqns;
      }
      if (c->getFunctionType() != Constant
          && c->getFunctionType() != Linear) {
        ++nlcs;
      }
    }

    o = p->getObjective();
    ofun = o->getFunction();

    // count number of nonlinear objectives
    if (nobj>0 && o->getFunctionType() != Constant && 
        o->getFunctionType() != Linear) {
      nlo = 1;
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
      if (Linear==v->getFunType() || Constant==v->getFunType()) {
        continue;
      }
      nl_in_o = false;
      if (nlo>0 && (false == ofun->isLinearIn(v))) {
        nl_in_o = true;
      } else {
        ++nlvc; // v is linear in obj but nonlinear in constraints
        continue;
      }
      nl_in_c = false;
    }

    // to be removed
    of << " " << nl_in_o << " " << nl_in_c << std::endl;

    of << " " << nlvc << " " << nlvo << " " << nlvb
       << "    # nonlinear vars in constraints, objectives, both" << std::endl;

    of << " 0 0 0 1        # linear network variables; functions; arith, flags" << std::endl;
    of << " 20 0 0 0 0     # discrete variables: binary, integer, nonlinear (b,c,o)" << std::endl;
    of << " 230 10 # nonzeros in Jacobian, gradients" << std::endl;

    of << " 0 0    # max name lengths: constraints, variables" << std::endl;
    of << " 0 0 0 0 0      # common exprs: b,c,o,c1,o1" << std::endl;

    of.close();
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
