//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
//

// Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

/*
=======================================================================
Quadratically Constrained Quadratic Program (QCQP) 
var x1 >= -10 <= 10 ;
var x2 >= -10 <= 10 ;
maximize obj: + 0.5*x1*x1 + 0.5*x2*x2  - 0;

subject to cons0: + 0.5*x1*x1 + 1*x1*x2 - 1*x2*x2  <= 1;
=========================================================================
*/
#include <iostream>
#include <cmath>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "Problem.h"
#include "Glob.h"

using namespace Minotaur;

int main() {
    // Create environment
    EnvPtr env = new Environment();
    env->getOptions()->findBool("use_native_cgraph")->setValue(true);

    // Create the problem instance
    ProblemPtr problem = new Problem(env);

    // Define decision variables
    VariablePtr x1 = problem->newVariable(-10, 10, Continuous, "x1"); // Continuous variable
    VariablePtr x2 = problem->newVariable(-10, 10, Continuous, "x2"); // Continuous variable

    // Define the objective function: Maximize 0.5*x1^2 + 0.5*x2^2
    QuadraticFunctionPtr qf = new QuadraticFunction();
    qf->addTerm(x1, x1, 0.5);  // 0.5*x1^2
    qf->addTerm(x2, x2, 0.5);  // 0.5*x2^2

    FunctionPtr obj_f = new Function(nullptr, qf);
    problem->newObjective(obj_f, 0.0, Maximize);

    // Add constraint: 0.5*x1^2 + x1*x2 - x2^2 <= 1
    QuadraticFunctionPtr qcf = new QuadraticFunction();
    qcf->addTerm(x1, x1, 0.5);  // 0.5*x1^2
    qcf->addTerm(x1, x2, 1.0);   // x1*x2
    qcf->addTerm(x2, x2, -1.0);  // -x2^2

    FunctionPtr con_f = new Function(nullptr, qcf);
    problem->newConstraint(con_f, -INFINITY, 1.0);  // Constraint: 0.5*x1^2 + x1*x2 - x2^2 <= 1

    // Print initial values
    std::cout << "===========================Before Solving============================\n";
    std::cout << "Initial values:" << std::endl;
    std::cout << "x1 = " << x1->getInitVal() << std::endl;
    std::cout << "x2 = " << x2->getInitVal() << std::endl;

    problem->write(std::cout);

    std::cout << "===========================After Solving============================\n";

    // Solve the problem using Glob
    Glob glob(env);
    glob.solve(problem);
    glob.getSolution();

    std::cout << "=====================================================================\n";

    return 0;
}
