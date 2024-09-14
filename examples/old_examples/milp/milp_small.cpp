//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
//

// Contributer: Sumanto Kar(jeetsumanto123@gmail.com)
/*
===================================================
Mixed Integer Linear Programming Example
===================================================
var x1 >= -100 <= 10 ;
var x2 >= -100 <= 10 ;
var x3 >= -100 <= 10  integer ;
minimize obj: - 5*x1 - 6*x2 - 7*x3  - 0;

subject to cons0: 0 <= + 1*x1 - 2*x2 + 3*x3  <= 15;

subject to cons1: 0 <= + 2*x1 + 1*x2  <= 10;
=================================================
*/
#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "IpoptEngine.h"
#include "LinearHandler.h"
#include "PCBProcessor.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "OsiLPEngine.h"
#include "Problem.h"
#include "QGHandler.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"
#include "AMPLInterface.h"

using namespace Minotaur;

int main() {
    // Create an environment for the problem
    EnvPtr env = new Environment();
    env->getOptions()->findBool("use_native_cgraph")->setValue(true);
    env->getOptions()->findBool("expand_quad")->setValue(false);

    if (!env) {
        std::cerr << "Failed to create environment" << std::endl;
        return 1;
    }

    // Create the problem instance
    ProblemPtr problem = new Problem(env);
    problem->setNativeDer();

    if (!problem) {
        std::cerr << "Failed to create problem" << std::endl;
        return 1;
    }

    // Create variables: x1 (continuous), x2 (continuous), x3 (integer)
    VariablePtr x1 = problem->newVariable(-100.0, 10.0, Continuous, "x1");
    VariablePtr x2 = problem->newVariable(-100.0, 10.0, Continuous, "x2");
    VariablePtr x3 = problem->newVariable(-100.0, 10.0, Integer, "x3");  // x3 is integer

    problem->setInitVal(x1, -18.0);

    // Define the objective function: Maximize 5*x1 + 6*x2 + 7*x3
    LinearFunctionPtr lf = new LinearFunction();
    lf->addTerm(x1, 5.0);
    lf->addTerm(x2, 6.0);
    lf->addTerm(x3, 7.0);

    FunctionPtr f = new Function(lf);
    ObjectiveType otyp = Maximize;
    problem->newObjective(f, 0, otyp);

    // Add constraint: x1 - 2*x2 + 3*x3 <= 15
    LinearFunctionPtr constraint1_lf = new LinearFunction();
    constraint1_lf->addTerm(x1, 1.0);
    constraint1_lf->addTerm(x2, -2.0);
    constraint1_lf->addTerm(x3, 3.0);
    FunctionPtr cf1 = new Function(constraint1_lf);
    problem->newConstraint(cf1, 0.0, 15.0);

    // Add another constraint: 2*x1 + x2 <= 10
    LinearFunctionPtr constraint2_lf = new LinearFunction();
    constraint2_lf->addTerm(x1, 2.0);
    constraint2_lf->addTerm(x2, 1.0);
    FunctionPtr cf2 = new Function(constraint2_lf);
    problem->newConstraint(cf2, 0.0, 10.0);

    // Setup branch-and-bound
    HandlerVector handlers;
    double objsense = 1.0;
    if (Maximize == problem->getObjective()->getObjectiveType()) {
        problem->negateObj();
        objsense = -1.0;
    }

    // Create branch-and-bound object
    BranchAndBound* bab = new BranchAndBound(env, problem);
    EnginePtr nlp_e = (IpoptEnginePtr)new IpoptEngine(env);
    EnginePtr e = (OsiLPEnginePtr)new OsiLPEngine(env);

    // Setup handlers
    IntVarHandlerPtr v_hand = (IntVarHandlerPtr)new IntVarHandler(env, problem);
    LinearHandlerPtr l_hand = (LinearHandlerPtr)new LinearHandler(env, problem);
    l_hand->setModFlags(false, true);
    handlers.push_back(v_hand);
    handlers.push_back(l_hand);

    // Setup engine for solving relaxations and branching
    ReliabilityBrancherPtr rel_br = (ReliabilityBrancherPtr)new ReliabilityBrancher(env, handlers);
    rel_br->setEngine(e);

    // Node processor
    NodeProcessorPtr nproc = (PCBProcessorPtr)new PCBProcessor(env, e, handlers);
    nproc->setBrancher(rel_br);
    bab->setNodeProcessor(nproc);

    // Node relaxer
    NodeIncRelaxerPtr nr = (NodeIncRelaxerPtr)new NodeIncRelaxer(env, handlers);
    nr->setEngine(e);
    nr->setModFlag(false);
    bab->setNodeRelaxer(nr);
    bab->shouldCreateRoot(true);

    // Print initial values
    std::cout << "===========================Before Solving============================\n";
    std::cout << "Initial values:" << std::endl;
    std::cout << "x1 = " << x1->getInitVal() << std::endl;
    std::cout << "x2 = " << x2->getInitVal() << std::endl;
    std::cout << "x3 = " << x3->getInitVal() << std::endl;

    problem->write(std::cout);

    std::cout << "===========================After Solving============================\n";

    // Start solving
    bab->solve();
    bab->writeStats(std::cout);

    // Print solution
    std::cout << "Best solution value = " << objsense * bab->getUb() << std::endl;
    std::cout << "Status of branch-and-bound: " << getSolveStatusString(bab->getStatus()) << std::endl;

    // Cleanup
    delete bab;

    return 0;
}
