//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

// Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

/*
======= Mixed Integer Linear Program ==============
var x1 >= -100 <= 10 ;
var x2 >= -100 <= 10 ;
var x3 >= -100 <= 10  integer ;
maximize obj: + 5*x1 + 6*x2 + 7*x3  - 0;

subject to cons0: 0 <= + 1*x1 - 2*x2 + 3*x3  <= 15;

subject to cons1: 0 <= + 2*x1 + 1*x2  <= 10;
*/
#include <iostream>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "Problem.h"
#include "Bnb.h"

using namespace Minotaur;

int main() {
    // Create environment
    EnvPtr env = new Environment();

    // Create the problem instance
    ProblemPtr problem = new Problem(env);

    // Define variables
    VariablePtr x1 = problem->newVariable(-100.0, 10.0, Continuous, "x1");
    VariablePtr x2 = problem->newVariable(-100.0, 10.0, Continuous, "x2");
    VariablePtr x3 = problem->newVariable(-100.0, 10.0, Integer, "x3");  // Integer variable

    // Define the objective function: Maximize 5*x1 + 6*x2 + 7*x3
    LinearFunctionPtr obj_lf = new LinearFunction();
    obj_lf->addTerm(x1, 5.0);
    obj_lf->addTerm(x2, 6.0);
    obj_lf->addTerm(x3, 7.0);
    FunctionPtr obj_f = new Function(obj_lf);
    problem->newObjective(obj_f, 0, Maximize);

    // Add constraint: 0 <= x1 - 2*x2 + 3*x3 <= 15
    LinearFunctionPtr lf1 = new LinearFunction();
    lf1->addTerm(x1, 1.0);
    lf1->addTerm(x2, -2.0);
    lf1->addTerm(x3, 3.0);
    FunctionPtr cf1 = new Function(lf1);
    problem->newConstraint(cf1, 0.0, 15.0);

    // Add another constraint: 0 <= 2*x1 + x2 <= 10
    LinearFunctionPtr lf2 = new LinearFunction();
    lf2->addTerm(x1, 2.0);
    lf2->addTerm(x2, 1.0);
    FunctionPtr cf2 = new Function(lf2);
    problem->newConstraint(cf2, 0.0, 10.0);

    // Print initial values
    std::cout << "===========================Before Solving============================\n";
    std::cout << "Initial values:" << std::endl;
    std::cout << "x1 = " << x1->getInitVal() << std::endl;
    std::cout << "x2 = " << x2->getInitVal() << std::endl;
    std::cout << "x3 = " << x3->getInitVal() << std::endl;

    problem->write(std::cout);

    std::cout << "===========================After Solving============================\n";

    // Solve the problem using Bnb
   
    Bnb bnb(env);
    bnb.solve(problem);
    bnb.getSolution();
    std::cout << "=====================================================================\n";
    
    return 0;
}
