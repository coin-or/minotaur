//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

// Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

/*
Quadratic Program Example
==============================================
var x1 >= -10 <= 10  integer ;
var x2 >= -10 <= 10 ;
minimize obj: + 1*x1*x1 - 1*x2*x2  - 0;

subject to cons0: 1 <= + 1*x1 + 1*x2 ;
===============================================
*/
#include <iostream>
#include <cmath>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "Problem.h"
#include "Bnb.h"

using namespace Minotaur;

int main() {
    // Create environment
    EnvPtr env = new Environment();
    env->getOptions()->findBool("use_native_cgraph")->setValue(true);

    // Create the problem instance
    ProblemPtr problem = new Problem(env);

    // Define decision variables
    VariablePtr x1 = problem->newVariable(-10, 10, Integer, "x1"); // Integer variable
    VariablePtr x2 = problem->newVariable(-10, 10, Continuous, "x2"); // Continuous variable

    // Define the objective function: Minimize x1^2 + x2^2
    QuadraticFunctionPtr qf = new QuadraticFunction();
    qf->addTerm(x1, x1, 1.0);  // x1^2
    qf->addTerm(x2, x2, 1.0);  // x2^2

    FunctionPtr obj_f = new Function(nullptr, qf);
    problem->newObjective(obj_f, 0.0, Maximize);

    // Add constraint: x1 + x2 >= 1
    LinearFunctionPtr lf = new LinearFunction();
    lf->addTerm(x1, 1.0);
    lf->addTerm(x2, 1.0);
    FunctionPtr con_f = new Function(lf);
    problem->newConstraint(con_f, 1.0, INFINITY); // Constraint: x1 + x2 >= 1

    // Print initial values
    std::cout << "===========================Before Solving============================\n";
    std::cout << "Initial values:" << std::endl;
    std::cout << "x1 = " << x1->getInitVal() << std::endl;
    std::cout << "x2 = " << x2->getInitVal() << std::endl;

    problem->write(std::cout);

    std::cout << "===========================After Solving============================\n";

    // Solve the problem using BnB
    Bnb bnb(env);
    bnb.solve(problem);
    bnb.getSol();

    std::cout << "=====================================================================\n";

    return 0;
}
