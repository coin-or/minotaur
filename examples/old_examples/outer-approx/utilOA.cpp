//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/*! \brief Definition of utilities for outer approximation with MINOTAUR
 *
 * \author Sven Leyffer, Argonne National Laboratory, 2011.
 */

//! The following header file contains all other required headers
#include "utilOA.h"
#include "BranchAndBound.h"
#include "Constraint.h"
#include "Function.h"
#include "Objective.h"
#include "Option.h"
#include "Solution.h"

////////////////////////////////////////////////////////////////////////////
//! return typical usage of solver
void usage()
{
  std::cout << "usage: ./outer-approx {name-of-.nl-file}\n";
}

////////////////////////////////////////////////////////////////////////////
//! show help for user
void show_help()
{
  std::cout << "Usage:" << std::endl
            << "To show version: outer-approx -v (or --show_version yes) " 
            << std::endl
            << "To show all options: outer-approx -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: outer-approx --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}

////////////////////////////////////////////////////////////////////////////
//! add to AMPL flags
void add_ampl_flags(OptionDBPtr options)
{
  FlagOptionPtr f_option; // some flags provided by user
  f_option = (FlagOptionPtr) new Option<Bool>
    ("AMPL", "-AMPL option tells us to write .sol file for ampl.", 
      true, false);
  options->insert(f_option, true);

  f_option = (FlagOptionPtr) new Option<Bool>
    ("v", "-v option tells us to write version information.", 
      true, false);
  options->insert(f_option, true);

  f_option = (FlagOptionPtr) new Option<Bool>
    ("=", "-= option tells us to write all known options.", 
      true, false);
  options->insert(f_option, true);

  f_option = (FlagOptionPtr) new Option<Bool>
    ("?", "-? option tells us to write help message.", 
      true, false);
  options->insert(f_option, true);
}

////////////////////////////////////////////////////////////////////////////
//! linearize a function at a point x
void linearAt(ProblemPtr minlp, FunctionPtr f, double fval, const double *x, 
    double *c, LinearFunctionPtr *lf)
{
  int n = minlp->getNumVars();
  double *a = new double[n];
  VariableConstIterator vbeg = minlp->varsBegin();
  VariableConstIterator vend = minlp->varsEnd();
  double tol = 1E-9;
  int error=0;

  std::fill(a, a+n, 0.);
  f->evalGradient(x, a, &error);
  *lf = (LinearFunctionPtr) new LinearFunction(a, vbeg, vend, tol); 
  *c  = fval - InnerProduct(x, a, n);
  delete [] a;
}

////////////////////////////////////////////////////////////////////////////
//! check if user needs help
int checkUserOK(OptionDBPtr options, EnvPtr env)
{
  options->findString("interface_type")->setValue("AMPL");
  if (options->findBool("show_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    return 0;
  }
  if ((options->findBool("show_help")->getValue() ||
       options->findFlag("?")->getValue()) ||
      (options->findString("problem_file")->getValue()=="")) {
    show_help();
    return 0;
  }
  if (options->findBool("show_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    env->writeFullVersion(std::cout);
    std::cout << std::endl;
    return 0;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////////
//! initialize the master problem
void initMaster(ProblemPtr minlp, ProblemPtr milp, VariablePtr &objVar, 
		ObjectivePtr objFun, const double *x) 
{
  //! Declaration of Internal Variables ===================================
  //! pointers to manipulate variables & constraints
  VariableConstIterator vbegin, vend;
  VariablePtr vPtr;                           //! variable pointer
  ConstraintPtr c, cnew;                      //! constraint pointer
  LinearFunctionPtr lf = LinearFunctionPtr(); //! linear function ptr = NULL
  FunctionPtr f, fnew;                        //! function pointer

  double act, cval;
  // ======================================================================

  vbegin = minlp->varsBegin();
  vend   = minlp->varsEnd();
  milp->newVariables(vbegin, vend);
  for (ConstraintConstIterator i=minlp->consBegin(); i!=minlp->consEnd(); ++i) {
    c = *i;
    if (c->getFunctionType()==Linear) {
      LinearFunctionPtr new_lf = (LinearFunctionPtr) new LinearFunction();
      lf = c->getLinearFunction();
      for (VariableGroupConstIterator vg_iter = lf->termsBegin(); 
           vg_iter != lf->termsEnd(); ++vg_iter) {
        new_lf->incTerm(milp->getVariable(vg_iter->first->getId()), vg_iter->second);
      }
      f = (FunctionPtr) new Function(new_lf);
      milp->newConstraint(f, c->getLb(), c->getUb(), c->getName());
    }
  }
  if (!objFun) {
    //! do nothing: no need to add an objective, if there's none!
    objVar = VariablePtr();
    std::cout << "WARNING: no objective function" << std::endl;
  } else {
    vPtr   = milp->newVariable(-INFINITY, INFINITY, Continuous, "obj_var");
    objVar = vPtr;
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(vPtr, 1.0);
    f = (FunctionPtr) new Function(lf);
    milp->newObjective(f, objFun->getConstant(), objFun->getObjectiveType());
  }
  milp->write(std::cout);
}

////////////////////////////////////////////////////////////////////////////
//! add a set of linearizations to the master problem
void updateMaster(ProblemPtr minlp, ProblemPtr milp, VariablePtr objVar, 
		  ObjectivePtr objFun, double objfUp, const double *x, int n) 
{
  //! Declaration of Internal Variables ===================================
  //! pointers to manipulate variables & constraints
  VariablePtr vPtr;                           //! variable pointer
  ConstraintPtr c, cnew;                      //! constraint pointer
  LinearFunctionPtr lf = LinearFunctionPtr(); //! linear function ptr = NULL
  FunctionPtr f, fnew;                        //! function pointer

  double act, cval;
  // ======================================================================


  //! add linearizations of nonlinear constraints
  for (ConstraintConstIterator i=minlp->consBegin(); i!=minlp->consEnd(); ++i) {
    c = *i;
    if (c->getFunctionType()!=Linear) {
      act = c->getActivity(x);
      f   = c->getFunction();
      linearAt(minlp, f, act, x, &cval, &lf);
      fnew = (FunctionPtr) new Function(lf);
      if (c->getUb()==INFINITY) {
        cnew = milp->newConstraint(fnew, c->getLb()-cval,  INFINITY, "OA_cutL");
      }
      else if (c->getLb()==-INFINITY) {
        cnew = milp->newConstraint(fnew, -INFINITY, c->getUb()-cval, "OA_cutU");
      }
      else {
        assert(!"OA for nonconvex constraint: STOP");
      }
    }
  }

  //! update upper bound on objective variable
  if (objFun->getObjectiveType()==Minimize)
    milp->changeBound(objVar, -INFINITY, objfUp);
  else
    milp->changeBound(objVar, -objfUp, INFINITY);
  
  //! add linearizations of objective function (ONLY NEEDED IF NONLINEAR)
  if (!objFun) {
    //! do nothing: no need to add an objective, if there's none!
  } else {
    f    = objFun->getFunction();
    act  = objFun->eval(x);
    vPtr = objVar;
    linearAt(minlp, f, act, x, &cval, &lf);
    if (objFun->getObjectiveType()==Minimize) {
      lf->addTerm(vPtr,-1.0);
      fnew = (FunctionPtr) new Function(lf);
      cnew = milp->newConstraint(fnew, -INFINITY, -cval, "OA_obj_cutU");
    }
    else {
      lf->addTerm(vPtr, 1.0);
      fnew = (FunctionPtr) new Function(lf);
      cnew = milp->newConstraint(fnew,  cval,  INFINITY, "OA_obj_cutL");
    }
  }

  std::cout << "MILP Master" << std::endl << "===========" << std::endl;
  std::cout << "New x values" << std::endl << "============" << std::endl;
  for (int j=0; j<n; j++) std::cout << "x(" << j << ") = " << x[j] << std::endl;
  milp->write(std::cout);
}

////////////////////////////////////////////////////////////////////////////
//! set-up and solve NLP(y) for fixed integer variables
void solveNLP(ProblemPtr minlp, FilterSQPEngine &e, double *x, 
	      double &objfNLP, int &feasibleNLP, int n)
{
  EngineStatus status;
  VariablePtr v;                              //! variable pointer

  //! fix integer variables
  for (VariableConstIterator i=minlp->varsBegin(); i!=minlp->varsEnd(); ++i) {
    v = *i;
    if ((v->getType()==Binary)||(v->getType()==Integer)) {
      minlp->changeBound(v, x[v->getId()], x[v->getId()]);
    }
  }
  minlp->write(std::cout);

  //! solve NLP(y) & save solution in x
  status      = e.solve();
  std::copy(e.getSolution()->getPrimal(),e.getSolution()->getPrimal()+n,x);
  objfNLP     = e.getSolutionValue();
  feasibleNLP = status==1;

  std::cout << "NLP-subproblem " << e.getStatusString() 
	    << " obj. value = " << objfNLP << std::endl;
  std::cout << "Solution values" << std::endl << "===============" << std::endl;
  for (int j=0; j<n; j++) std::cout << "x(" << j << ") = " << x[j] << std::endl;

}

////////////////////////////////////////////////////////////////////////////
//! solve MILP master problem  
void solveMaster(EnvPtr env, ProblemPtr milp, double *x, double *objfMIP, 
		 int n)
{
  //! Declaration of Internal Variables ===================================
  LPEnginePtr lp_e;                           //! LP engine 
  NLPEnginePtr nlp_e=NLPEnginePtr();          //! NLP engine = NULL
  QPEnginePtr qp_e=QPEnginePtr();             //! QP engine = NULL
  BabInitPtr bit;
  BranchAndBoundPtr bab;
  SolveStatus bstatus;
  // ======================================================================

  //! create the initializer for branch-and-bound (check problem class etc)
  env->getOptions()->findInt("bnb_log_level")->setValue(6);
  lp_e = (OsiLPEnginePtr) new OsiLPEngine(env);
  bit  = (BabInitPtr) new BabInit(env, milp, lp_e, nlp_e, qp_e);
  bab  = bit->createBab();
  bab->solve();
  bstatus  = bab->getStatus();
  if (bstatus==SolvedOptimal) {
    std::copy(bab->getSolution()->getPrimal(),bab->getSolution()->getPrimal()+n,x);
  }
  *objfMIP = bab->getUb();

  //! output from solve of MILP master problem
  std::cout << "MILP master  " << bstatus << "  lower bound = " << bab->getLb() 
	    << "  upper bound = " << *objfMIP << std::endl;
  std::cout << "Solution values" << std::endl << "===============" << std::endl;
  for (int j=0; j<n; j++) std::cout << "x(" << j << ") = " << x[j] << std::endl;  
  
}
