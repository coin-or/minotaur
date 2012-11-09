// A simple file that solves a nonlinear relaxation of a given problem.
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
#include <ctime>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "AMPLInterface.h"
#include "Problem.h"
#include "Timer.h"
#include "MinotaurDeconfig.h"
#include "NLPRelaxation.h"
#include "Variable.h"
#include "AMPLJacobian.h"
#include "AMPLHessian.h"
#include "Types.h"
#include "Option.h"
#include "Relaxation.h"
#include "CoverCutGenerator.h"
#include "Objective.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "CGraph.h"
#include "CNode.h"
#include "OpCode.h"
#include "Function.h"
#include "Presolver.h"

using namespace Minotaur;

# define WARMSTART 1

Bool logobj(LinearFunctionPtr lf, CGraphPtr nlf);

int main() 
{

  // Generate output.
  ofstream output;
  output.open("numknapcov.txt");
  // Generate input.
  ifstream input;
  input.open("list.txt");
  // Check if input is opened succesfully.
  if (input.is_open() == false) {
    cerr   << "Input file read error." << endl;
    output << "Input file read error." << endl;
    exit(0);
  }
  
  /********************************************************************************/
  // Headers for output data.
  output << "Statistics of knapsack cover cuts applied to root relaxation." << endl;
  output << "problem " << "vars " << "cons " << "lincons " << "knapcons " << "knapcov "
	 << "knaps " << "totalcuts " << "cuts " << "violknapcuts " << "initobj " 
	 << "endobj " << "gapclosed " << "timeinit " << "timecut " << "timemod" 
	 <<  endl;
  /********************************************************************************/

  // loop to test all problems in list.txt
  while (input.good()) {
    // problem name
    string pname;
    getline(input, pname);
    // At the end of file just exit from loop.
    if (pname.empty()) {
      break;
    }

    cout << "Problem considered is: " << pname << endl;

    // Real stuff begins.
    // Ampl interface, jacobian and hessian.
    MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();  
    JacobianPtr jPtr;            //! Jacobian read from AMPL
    HessianOfLagPtr hPtr;        //! Hessian read from AMPL
  
    // environment, timers and options:
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

    // Default options
    env->getOptions()->findBool("presolve")->setValue(false);
    env->getOptions()->findBool("use_native_cgraph")->setValue(true);
    env->getOptions()->findBool("nl_presolve")->setValue(false);
    // parse options
    env->readOptions(argc2, argv2);
    options = env->getOptions();
    options->findString("interface_type")->setValue("AMPL");

    // read minlp from AMPL.
    iface = (MINOTAUR_AMPL::AMPLInterfacePtr) new MINOTAUR_AMPL::AMPLInterface(env); 
    minlp = iface->readInstance(pname);

    // Timer is obtained.
    Timer * timer = env->getNewTimer();

    // Nonlinearize objective function.
    Bool MIPCONSIDERED = false;
    if (MIPCONSIDERED ==  true) {
      ObjectivePtr initobjfun = minlp->getObjective();
      if (initobjfun->getObjectiveType() == Maximize) {
    	cerr << "Objective type is Maximize, change it to Minimize." << endl;
    	exit(0);
      }
      LinearFunctionPtr lfinitobj = initobjfun->getLinearFunction();
      // NonlinearFunctionPtr nlfobj = (NonlinearFunctionPtr) new NonlinearFunction();
      CGraphPtr nlfobj = (CGraphPtr) new CGraph();
      logobj(lfinitobj, nlfobj);
      FunctionPtr logobjfun = (FunctionPtr) new Function(nlfobj);      
      ObjectiveType otyp = Minimize;
      minlp->changeObj(logobjfun, 0);
    }
    
    minlp->calculateSize();
    minlp->prepareForSolve();

    // Change format of problem to be suitable for Minotaur.
    HandlerVector handlers;
    // Use presolver to standardize problem.
    //pres = (PresolverPtr) new Presolver(minlp, env, handlers);
    //pres->standardize();

    minlp->calculateSize();
    minlp->prepareForSolve();
  
    minlp->setJacobian(jPtr);
    minlp->setHessian(hPtr);
    minlp->setNativeDer();

    minlp->calculateSize();
    minlp->prepareForSolve();
    minlp->setNativeDer();

    //minlp->write(std::cout);

    /**************************************************************/
    // Given problem statistics .
    // Number of variables.
    UInt numvars = minlp->getNumVars();
    // number of constraints.
    UInt numcons = minlp->getNumCons();
    // linear constraints.
    UInt numlin = minlp->getNumLinCons();
    /*************************************************************/

    // set option for engine to resolve to solve NLP repeatedly.
    // Probbaly does nothing.
    e.setOptionsForRepeatedSolve();

    // load problem.
    e.load(minlp);
      
    // Solve problem.
    timer->start();
    status = e.solve();

    /********************************************************************/
    // Solution time of relaxation.
    Double timeinit = timer->query();
    timer->stop();
    // Solution objective value
    Double initobj = e.getSolutionValue();
    /********************************************************************/

    std::cout << "Relaxation objective value = " << initobj << std::endl; 
  
    // Get solution from engine.
    ConstSolutionPtr sol = e.getSolution();
  
    // Construct relaxation.
    RelaxationPtr rel = (RelaxationPtr) new Relaxation(minlp);
    
    // Time for cut generation.
    timer->start();
    // Generate kanpsack cover cuts.
    CoverCutGeneratorPtr knapgen = 
      (CoverCutGeneratorPtr) new CoverCutGenerator(rel, sol, env);

    /*******************************************************************/
    Double timecut = timer->query();
    timer->stop();
    /*******************************************************************/


    // Get statistics of cut generator.
    ConstCovCutGenStatsPtr knapstats = knapgen->getStats();

    /*******************************************************************/
    // Knapsack cut generator statistics.
    // knapsack constraints.
    UInt numknap = (knapgen->getKnapsackList())->getNumKnaps();
    // knapsacks that has cover sets.
    UInt numknapcov = knapgen->getNumCons();
    // knapsack subproblems solved, i.e number of lifting subproblems solved.
    UInt knaps = knapstats->knaps;
    // cover cuts including duplicates.
    UInt totalcuts = knapstats->totalcuts;
    // cuts without duplicates.
    UInt numknapcuts = knapstats->cuts;
    // violated cuts.
    UInt violknapcuts = knapstats->violated;
    /*******************************************************************/


    std::cout << "Number of knapsack cover cuts to be applied is: " 
	      << knapstats->violated << std::endl;

    // Get the violated cuts from generator.
    CutVector knapcuts = knapgen->getViolatedCutList();

    // Iterators for cuts
    CutVectorConstIter it;
    CutVectorConstIter begin = knapcuts.begin();
    CutVectorConstIter end   = knapcuts.end();

    // Apply the cuts to the problem.
    // Violation list.
    DoubleVector knapviols = knapgen->getViolList();
    UInt curknap = 0;
    Double maxviol = 0.0;
    for (it=begin; it!=end; ++it) {
      std::cout << "Violation obtained from this constraint is: "
		<< knapviols[curknap] << std::endl;
      ConstraintPtr newcons = rel->newConstraint((*it)->getFunction(), (*it)->getLb(), (*it)->getUb());
      if (maxviol < knapviols[curknap]) {
	maxviol = knapviols[curknap];
      }
      // add constraint to engine does not do anything.
      // Thus, we add constraint to the relaxation and reload it to engine.
      // e.addConstraint(newcons);
    }


    /*******************************************************************/
    // Solution time of knapsack cover cuts added problem.
    Double timemod = 0.0;
    // Objective value after adding knapsack cover cuts.
    Double endobj = 0.0;
    // Gap closed by using knapsack cover cuts.
    Double gapknap = 0.0;
    /*******************************************************************/

    if (violknapcuts >= 1) {
      // Reload problem to engine.
      // Check if we should reload the modified problem.
      e.clear();
      const Double * xupdated;
      if (WARMSTART == 1) {
	// Set initial point as the solution of root solution.
	xupdated = sol->getPrimal();
	rel->setInitialPoint(xupdated);
      }

      // Load the modified problem.
      e.load(rel);
    
      // warmstart continues.
      if (WARMSTART == 1) {
	// Before presolve, we set initial primal and 
	// dual solutions as the root solution.
	SolutionPtr solupdated = (SolutionPtr) new Solution(initobj, xupdated, rel);
	// Create new dual solution.
	const Double * dualofvars = sol->getDualOfVars();
	solupdated->setDualOfVars(dualofvars);
	const Double * initdualofcons = sol->getDualOfCons();
	UInt numconsupdated = rel->getNumCons();
	Double * dualofcons = new Double[numconsupdated];
	memcpy(dualofcons, initdualofcons, numcons*sizeof(Double));
	for (UInt indexx = numcons; indexx < numconsupdated; ++indexx) {
	  dualofcons[indexx] = 0.0;
	}
	solupdated->setDualOfCons(dualofcons);
	FilterWSPtr warmstart = (FilterWSPtr) new FilterSQPWarmStart();
	warmstart->setPoint(solupdated);
	e.loadFromWarmStart(warmstart);

	delete [] dualofcons;
      }

      // Solution time after adding knapsack cover cuts to relaxation.
      timer->start();
      // Resolve the problem.
      e.solve();
    
      /*******************************************************************/
      // Solution time of knapsack cover cuts added problem.
      timemod = timer->query();
      timer->stop();
      // Objective value after adding knapsack cover cuts.
      endobj = e.getSolutionValue();
      // Gap closed by using knapsack cover cuts.
      gapknap = (endobj-initobj)/fabs(initobj) * 100;
      /*******************************************************************/
    } else {
      /*******************************************************************/
      // Solution time of knapsack cover cuts added problem.
      timemod = timeinit;
      // Objective value after adding knapsack cover cuts.
      endobj = initobj;
      // Gap closed by using knapsack cover cuts.
      gapknap = 0.0;
      /*******************************************************************/
    }


    std::cout << "Objective value of relaxation after adding cuts: "
	      << endobj << std::endl;

    cout << pname << " " << numvars  << " " << numcons << " " << numlin 
	 << " " << numknap
	 << " " << numknapcov << " " << knaps << " " << totalcuts
	 << " " << numknapcuts << " " << violknapcuts
	 << std::fixed << std::setprecision(2) 
	 << " " << initobj << " " << endobj
	 << " " << gapknap << " " << timeinit << " " << timecut
	 << " " << timemod << endl;

    if (numknap >= 1) {    
    // Save output data.
      output << pname << " " << numvars << " " << numcons << " " << numlin 
	     << " " << numknap
	     << " " << numknapcov << " " << knaps << " " << totalcuts
	     << " " << numknapcuts << " " << violknapcuts
	     << std::fixed << std::setprecision(2) 
	     << " " << initobj << " " << endobj
	     << " " << gapknap << " " << timeinit << " " << timecut
	     << " " << timemod << endl;
    }
      
    delete iface;
    delete [] argv2;
  }

  output.close();
  input.close();

  return 0;
}

Bool logobj(LinearFunctionPtr lf, CGraphPtr nlf)
{
  UInt numvars = lf->getNumTerms();
  // VariableSet vars;
  // lf->getVars(&vars);
  VariableGroup::const_iterator it;
  VariableGroup::const_iterator begin = lf->termsBegin();
  VariableGroup::const_iterator end   = lf->termsEnd();
  // Variable nodes.
  std::map<UInt,CNode*> varnodes;
  std::map<CNode*, CNode*> lognodes;
  std::map<UInt, CNode*> logidnodes;
  std::map<CNode*, CNode*> numnodes;
  std::map<UInt, CNode*> numidnodes;
  std::map<CNode*, CNode*> mulnodes;
  std::map<UInt, CNode*> mulidnodes;
  std::set<CNode*> multnodes;
  std::map<UInt, CNode*> multidnodes;
  CNode** multnodesvec = new CNode*[numvars];
  // Iterate through variables.
  CNode* tempnode;
  CNode* lognode;
  CNode* numnode;
  CNode* mulnode;
  CNode* onenode;
  CNode* sumnode;
  // Create numnode for one.
  UInt index = 0;
  for(it=begin; it!=end; ++it) {
    VariablePtr var = it->first;
    Double varlb = var->getLb();
    if (varlb >= 0) {
      UInt varid = var->getId();
      Double coeff = it->second;
      // Create variable node and add it to a set.
      tempnode = nlf->newNode(var);
      varnodes.insert(std::pair<UInt,CNode*>(varid,tempnode));
      onenode = nlf->newNode(1.0);
      // Create sumnode.      
      sumnode = nlf->newNode(OpPlus, onenode, tempnode);
      // Create log nodes.
      lognode = nlf->newNode(OpLog, &sumnode, 1);
      lognodes.insert(std::pair<CNode* , CNode*>(tempnode, lognode));
      logidnodes.insert(std::pair<UInt, CNode*>(varid, lognode));
      // Create num nodes.
      numnode = nlf->newNode(coeff);
      numnodes.insert(std::pair<CNode*, CNode*>(tempnode, numnode));
      numidnodes.insert(std::pair<UInt, CNode*>(varid, numnode));
      // Create mul nodes.
      mulnode = nlf->newNode(OpMult, numnode, lognode);
      multnodes.insert(mulnode);
      multidnodes.insert(std::pair<UInt, CNode*>(varid,mulnode));
      multnodesvec[index] = mulnode;
      index++;
    }
  }
  
  // This is the root node.
  CNode* sumlistnode = nlf->newNode(OpSumList, multnodesvec, index);  
  nlf->setOut(sumlistnode);
  nlf->finalize();
  
  return true;
}
