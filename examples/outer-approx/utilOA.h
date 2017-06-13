//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2011 The MINOTAUR Team
// 

/*! \brief Header of utilities for outer approximation with MINOTAUR
 *
 * \author Sven Leyffer, Argonne National Laboratory, 2011.
 */

#include <iostream>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "OsiLPEngine.h"
#include "AMPLInterface.h"
#include "Problem.h"
#include "Timer.h"
#include "MinotaurDeconfig.h"
#include "NLPRelaxation.h"
#include "Operations.h"
#include "BabInit.h"
#include "Variable.h"

using namespace Minotaur;

//! return typical usage of OA solver
void usage();

//! show help for user
void show_help();

//! add to AMPL flags
void add_ampl_flags(OptionDBPtr options);

//! linearize a function at a point x
void linearAt(ProblemPtr minlp, FunctionPtr f, double fval, double *x, 
	      double *c, LinearFunctionPtr *lf);

//! check if user needs help
int checkUserOK(OptionDBPtr options, EnvPtr env);

//! initialize the master problem
void initMaster(ProblemPtr minlp, ProblemPtr milp, VariablePtr &objVar, 
		ObjectivePtr objFun, const double *x);

//! add a set of linearizations to the master problem
void updateMaster(ProblemPtr minlp, ProblemPtr milp, VariablePtr objVar, 
		  ObjectivePtr objFun, double objfUp, const double *x, int n);

//! set-up and solve NLP(y) for fixed integer variables
void solveNLP(ProblemPtr minlp, FilterSQPEngine &e, double *x, 
	      double &objfNLP, int &feasibleNLP, int n);

//! solve MILP master problem  
void solveMaster(EnvPtr env, ProblemPtr milp, double *x, 
		 double *objfMIP, int n);

