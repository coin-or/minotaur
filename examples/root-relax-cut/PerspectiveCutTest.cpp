//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 -- 2010 The MINOTAUR Team.
//

/**
 * \file RelaxCutResolve.cpp
 * \brief The main function for solving relaxation, generating cuts, adding cuts
 * and resolving the modified problem.
 * \author Serdar Yildiz, Argonne National Laboratory
 */

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <ostream>
using std::ostream;
#include <fstream>
using std::ofstream;
using std::fstream;
using std::ifstream;
#include <string>
using std::string;
#include <cmath>
using std::fabs;
#include <iomanip>
#include <vector>
#include <cstdio>

#include "Environment.h"
#include "Option.h"
#include "Problem.h"
#include "FilterSQPEngine.h"
#include "AMPLInterface.h"
#include "Timer.h"
#include "AMPLJacobian.h"
#include "AMPLHessian.h"
#include "Types.h"
#include "PerspCutGenerator.h"
#include "Objective.h"
#include "Presolver.h"
#include "LinearHandler.h"
#include "NlPresHandler.h"

using namespace Minotaur;

# define WARMSTART 1
# define PRESOLVE 1

Bool createPres(EnvPtr env, ProblemPtr p, Size_t ndefs, HandlerVector &handlers,
		PresolverPtr& pres);

typedef boost::shared_ptr<LinearHandler> LinHandlerPtr;

int main()
{
  // Generate output.
  ofstream output;
  output.open("numpersp.txt");
  // Generate input.
  ifstream input;
  input.open("list.txt");
  // Check if input is opened successfully.
  if (input.is_open() == false) {
    cerr  << "Input file read error." << endl;
    output << "Input file read error." << endl;
    exit(0);
  }

  /****************************************************************************/
  // Headers for output data.
  output << "Statsitics of perspective cuts applied to root relaxation." << endl;
  output << "problem " << "vars " << "cons " << "lincons " << "perspcons "
	 << "perspnotcons " << "perspconsidered "
	 << "totalcuts " << "cuts " << "violcuts " << "initobj " << "endobj "
	 << "gapclosed " << "timeinit " << "timecut " << "timemod " << endl;
  /****************************************************************************/

  // loop tp test all problems in list.txt.
  while(input.good()) {
    // problem name.
    string pname;
    getline(input, pname);
    // At the end of file just exit from loop.
    if (pname.empty()) {
      break;
    }
    
    cout << "Problem considered is: " << pname << endl;
    
    // Real stuff begins.
    // AMPL interface, jacobian and hessian.
    MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();
    JacobianPtr jPtr;
    HessianOfLagPtr hPtr;

    // environment, timers and options.
    EnvPtr env = (EnvPtr) new Environment();
    OptionDBPtr options;
    
    // problem to be solved.
    ProblemPtr minlp;
    
    // solver pointers, including status.
    FilterSQPEngine e(env);
    EngineStatus status;

    // Presolver.
    PresolverPtr pres;

    // give parameters.
    UInt argc2 = 2;
    std::string arg1 = "bnb";
    std::string arg2 = pname;
    char** argv2 = new char* [2];
    argv2[0] = &arg1[0];
    argv2[1] = &arg2[0];

    // Default options.
    if (PRESOLVE == 1) {
      env->getOptions()->findBool("presolve")->setValue(true);
      // Nonlinear presolver gives error.
      env->getOptions()->findBool("nl_presolve")->setValue(true);
    } else {
      env->getOptions()->findBool("presolve")->setValue(false);
      env->getOptions()->findBool("nl_presolve")->setValue(false);
    }
    env->getOptions()->findBool("use_native_cgraph")->setValue(true);
    // parse options.
    env->readOptions(argc2, argv2);
    options = env->getOptions();
    options->findString("interface_type")->setValue("AMPL");

    // read minlp by using AMPL.
    iface = (MINOTAUR_AMPL::AMPLInterfacePtr) new MINOTAUR_AMPL::AMPLInterface(env);
    minlp = iface->readInstance(pname);
    
    // Get timer.
    Timer * timer = env->getNewTimer();

    if (PRESOLVE == 0) {
      HandlerVector handlers;
      // Use presolver to standardize problem.
      pres = (PresolverPtr) new Presolver(minlp, env, handlers);
      pres->standardize();
    }

    // Prepare for solution.
    minlp->calculateSize();
    minlp->prepareForSolve();
    
    minlp->setJacobian(jPtr);
    minlp->setHessian(hPtr);
    minlp->setNativeDer();


    // Do presolve
    ProblemPtr tinst;
    if (PRESOLVE == 1) {
      tinst = minlp;
      HandlerVector handlers;
      createPres(env, tinst, iface->getNumDefs(), handlers, pres);
      pres->standardize();
      pres->solve();
      handlers.clear();
      minlp=tinst;
    }

    // Check if problem is maximize or minimize. We do not consider maximization problems.
    if(minlp->getObjective()->getObjectiveType() != Minimize) {
      cerr << "Problem is a maximization problem!" << endl;
      continue;
    }

    // Prepare for solution.
    minlp->calculateSize();
    minlp->prepareForSolve();
    
    minlp->setJacobian(jPtr);
    minlp->setHessian(hPtr);
    minlp->setNativeDer();

    

    /****************************************************************************/
    // Given problem statistics.
    // Number of variables.
    UInt numvars = minlp->getNumVars();
    // number of constraints.
    UInt numcons = minlp->getNumCons();
    // linear constraints.
    UInt numlin = minlp->getNumLinCons();
    /****************************************************************************/

    // set option for engine to resolve to solve NLP repeatedly.
    // Probably does nothing.
    e.setOptionsForRepeatedSolve();

    // load problem.
    e.load(minlp);

    // solve problem.
    timer->start();
    status = e.solve();

    /****************************************************************************/
    // Solution time of relaxation.
    Double timeinit = timer->query();
    timer->stop();
    // Solution objective value.
    Double initobj = e.getSolutionValue();
    /****************************************************************************/
    
    std:: cout << "Relaxation objective value = " << initobj << endl;

    // Get solution from engine.
    ConstSolutionPtr sol = e.getSolution();

    // Serdar!!! Change solution values temporarily just to test all functions completeley.
    // Double * tempsolvalues = new Double[numvars];
    // for (UInt i=0; i<numvars; ++i) {
    //   tempsolvalues[i] = (i+0.2) * (i%3 + 0.3) / Double(i*(i+1)+1.2) + (i+0.1) * (i%2+0.2) / Double(i*(i+1)+1.4);
    // }
    // const Double * consttempsolvalues = tempsolvalues;
    // sol = (SolutionPtr) new Solution(initobj, consttempsolvalues, minlp);
    
    
    // Construct relaxation.
    RelaxationPtr rel = (RelaxationPtr) new Relaxation(minlp);

    // Time for cut generation.
    timer->start();
    // Generate perspective cuts.
    PerspCutGeneratorPtr perspgen =
      (PerspCutGeneratorPtr) new PerspCutGenerator(rel, sol, env);
    
    /****************************************************************************/
    Double timecut = timer->query();
    timer->stop();
    /****************************************************************************/
    
    // Get statistics of cut generation.
    ConstPerspGenStatsPtr perspstats = perspgen->getStats();

    /****************************************************************************/
    // Perspective cut generator statistics.
    // Perspective constraints.
    UInt numperspcons = perspstats->perspcons;
    // Perspective constarints which are not generated since 
    // the binary variable has integral value.
    UInt numperspnotcons = perspstats->perspnotcons;
    // Perspective constraints used for cut generation.
    UInt numperspconsidered = perspstats->perspconsidered;
    // Perspective cuts including duplicates.
    UInt totalperspcuts = perspstats->totalcuts;
    // cuts without duplicates.
    UInt nonduplicatepersp = perspstats->cuts;
    // violated cuts.
    UInt violperspcuts = perspstats->violated;
    /****************************************************************************/

    // Get violated cuts from generator.
    CutVector perspcuts = perspgen->getViolatedCutList();    
    // Iterators for cuts.
    CutVectorConstIter it;
    CutVectorConstIter begin = perspcuts.begin();
    CutVectorConstIter end   = perspcuts.end();
    // Apply cuts to the problem.
    // Violation list.
    DoubleVector perspviols = perspgen->getViolList();
    UInt curpersp = 0;
    Double maxviol = 0.0;
    ConstConstraintPtr newcons;
    CutPtr perspcut;
    FunctionPtr cutf;
    Double cutlb = 0.0;
    Double cutub = 0.0;
    Double curviol = 0.0;
    for (it=begin; it!=end; ++it) {
      curviol = perspviols[curpersp];
      cout << "Violation obtained from constraint " << curpersp << " is: " << curviol << endl;
      if (maxviol < curviol) {
	maxviol = curviol;
      }
      perspcut = *it;
      cutf = perspcut->getFunction();
      cutlb = perspcut->getLb();
      cutub = perspcut->getUb();
      newcons = rel->newConstraint(cutf, cutlb, cutub);
      curpersp += 1;
    }
    
    /****************************************************************************/
    // Solution time of persp cuts added problem.
    Double timemod = 0.0;
    // Objective value after adding persp cuts.
    Double endobj = 0.0;
    // Gap closed by adding persp cuts.
    Double gappersp = 0.0;
    /****************************************************************************/

    // If any violated cuts added.
    if (numperspcons >= 1) {
      // Reload problem  to engine.
      e.clear();
      const Double * xupdated;
      if (WARMSTART == 1) {
	// Warmstart starts.
	// Set initial point as the solution of previous solution.
	xupdated = sol->getPrimal();
	rel->setInitialPoint(xupdated);
	// Warmstart ends.
      }
      // Load the modified problem.
      e.load(rel);
      if (WARMSTART == 1) {
	// Serdar! Before resolve, we set the initial primal and dual solutions.
	SolutionPtr solupdated = (SolutionPtr) new Solution(initobj, xupdated, rel);
	// Create new dual solution.
	const Double * dualofvars = sol->getDualOfVars();
	solupdated->setDualOfVars(dualofvars);
	const Double * initdualofcons = sol->getDualOfCons();
	// UInt numconsupdated = numcons + violperspcuts;
	UInt numconsupdated = rel->getNumCons();
	Double * dualofcons = new Double[numconsupdated];
	memcpy(dualofcons, initdualofcons, numcons*sizeof(Double));
	//std::fill(dualofcons+sizeof(Double)*numcons, dualofcons 
	//	+ sizeof(Double)*numconsupdated, 0);
	for (UInt indexx = numcons; indexx <numconsupdated; ++indexx) {
	  dualofcons[indexx] = 0.0;
	}
	solupdated->setDualOfCons(dualofcons);
	FilterWSPtr warmstart = (FilterWSPtr) new FilterSQPWarmStart();
	warmstart->setPoint(solupdated);
	e.loadFromWarmStart(warmstart);

	delete [] dualofcons;
      }
	
      // Solution time after adding persp cuts to relaxation.
      timer->start();
      // Resolve problem.
      e.solve();

      /****************************************************************************/
      // Solution time of persp cuts added problem.
      timemod = timer->query();
      timer->stop();
      // Objective value after adding persp cuts.
      endobj = e.getSolutionValue();
      // Gap closed by using perspective cuts.
      gappersp = (endobj-initobj)/fabs(initobj) * 100; 
      /****************************************************************************/
    } else {
      /****************************************************************************/
      // Solution time  of perspective cuts added problem.
      timemod = timeinit;
      // Objective value after adding perspective cuts.
      endobj = initobj;
      gappersp = 0.0;
      /****************************************************************************/
    } // end of else.
    
    cout << "Objective value of relaxation after adding cuts: " << endobj << endl;
    
    cout << pname << " " << numvars << " " << numcons << " " << numlin 
	 << " " << numperspcons << " " << numperspnotcons
	 << " " << numperspconsidered << " " << totalperspcuts
	 << " " << nonduplicatepersp << " " << violperspcuts 
	 << std::fixed << std::setprecision(2)
	 << " " << initobj << " " << endobj << " " << gappersp
	 << " " << timeinit << " " << timecut 
	 << " " << timemod << endl;

    //    if (numperspcons >= 1) {
      // Save output data.
      output << pname << " " << numvars << " " << numcons << " " << numlin 
	     << " " << numperspcons << " " << numperspnotcons
	     << " " << numperspconsidered << " " << totalperspcuts
	     << " " << nonduplicatepersp << " " << violperspcuts 
	     << std::fixed << std::setprecision(2)
	     << " " << initobj << " " << endobj << " " << gappersp
	     << " " << timeinit << " " << timecut 
	     << " " << timemod << endl;
      // }

    // delete [] tempsolvalues;
    delete iface;
    delete [] argv2;

  } // end of while loop for all problems.
  
  output.close();
  input.close();
  
  return 0;
}

// Generate and give options for presolver.
Bool createPres(EnvPtr env, ProblemPtr p, Size_t ndefs, HandlerVector &handlers,
		PresolverPtr& pres)
{
  // Create handlers for presolve.
  pres = PresolverPtr(); // NULL
  const std::string me("rootcut: ");
  p->calculateSize();
  LinHandlerPtr lhandler = (LinHandlerPtr) new LinearHandler(env, p);
  handlers.push_back(lhandler);
  if (p->isQP() || p->isQuadratic() || p->isLinear() ||
      env->getOptions()->findBool("use_native_cgraph")->getValue() == true ) {
    lhandler->setPreOptPurgeVars(true);
    lhandler->setPreOptPurgeCons(true);
    lhandler->setPreOptCoeffImp(true);
  } else {
    lhandler->setPreOptPurgeVars(false);
    lhandler->setPreOptPurgeCons(false);
    lhandler->setPreOptCoeffImp(false);
  }
  if (ndefs > 0) {
    lhandler->setPreOptDualFix(false);
  } else {
    lhandler->setPreOptDualFix(true);
  }
  if (!p->isLinear() &&
      env->getOptions()->findBool("use_native_cgraph")->getValue() == true &&
      env->getOptions()->findBool("nl_presolve")->getValue() == true) {
    NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
    handlers.push_back(nlhand);
  }
  
  pres = (PresolverPtr) new Presolver(p, env, handlers);

  return true;
}

