//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

// Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

/*
======= Knapsack problem==============
var x1 >= 0 <= 1  binary ;
var x2 >= 0 <= 1  binary ;
var x3 >= 0 <= 1  binary ;
maximize obj: + 80*x1 + 20*x2 + 100*x3  - 0;

subject to cons0: 0 <= + 30*x1 + 10*x2 + 40*x3  <= 40;
*/
#include <iostream>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "Problem.h"
#include "Bnb.h"

using namespace Minotaur;

int main() {
    // Create environment

    EnvPtr env = (EnvPtr)new Environment();

    // Create the problem instance
    ProblemPtr problem = new Problem(env);

    // Define variables
    VariablePtr x1 = problem->newVariable(0, 1, Binary, "x1"); //Binary Variable
    VariablePtr x2 = problem->newVariable(0, 1, Binary, "x2"); //Binary Variable
    VariablePtr x3 = problem->newVariable(0, 1, Binary, "x3"); //Binary Variable

    // Define the objective function: Maximize 80*x1 + 20*x2 + 100*x3
    LinearFunctionPtr obj_lf = new LinearFunction();
    obj_lf->addTerm(x1, 80);
    obj_lf->addTerm(x2, 20);
    obj_lf->addTerm(x3, 100);
    FunctionPtr obj_f = new Function(obj_lf);
    problem->newObjective(obj_f, 0.0, Maximize);

    // Add constraint: 0 <= 30*x1 <= 40
    LinearFunctionPtr lf1 = new LinearFunction();
    lf1->addTerm(x1, 30.0);
    lf1->addTerm(x2, 10.0);
    lf1->addTerm(x3, 40.0);
    FunctionPtr cf1 = new Function(lf1);
    problem->newConstraint(cf1, 0.0, 40);


    // Print initial values
    std::cout << "===========================Before Solving============================\n";
    problem->write(std::cout);

    std::cout << "===========================After Solving============================\n";

    // Solve the problem using Bnb
   
    Bnb bnb(env);
    bnb.solve(problem);
    
    std::cout << "=====================================================================\n";
    //std::cout << "status = " << bnb.getStatus() << std::endl;

    //DoubleVector s = bnb.getSolution();
    
    
    delete problem;
    delete env;
    return 0;
}
