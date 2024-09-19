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
var x1 >= -100 <= 10  integer ;
var x2 >= -100 <= 10 ;
var x3 >= -100 <= 10  integer ;
var x4 >= -100 <= 10 ;
var x5 >= -100 <= 10  integer ;
var x6 >= -100 <= 10 ;
var x7 >= -100 <= 10  integer ;
var x8 >= -100 <= 10 ;
var x9 >= -100 <= 10  integer ;
var x10 >= -100 <= 10 ;
minimize obj: - 5*x1 - 6*x2 - 7*x3 - 8*x4 - 9*x5 - 10*x6 - 11*x7 - 12*x8 - 13*x9 - 14*x10  - 0;

subject to cons0: 0 <= + 1*x1 + 2*x2 + 3*x3 + 4*x4 + 5*x5 + 6*x6 + 7*x7 + 8*x8 + 9*x9 + 10*x10  <= 100;

subject to cons1: 0 <= + 2*x1 + 4*x2 + 6*x3 + 8*x4 + 10*x5 + 12*x6 + 14*x7 + 16*x8 + 18*x9 + 20*x10  <= 110;

subject to cons2: 0 <= + 3*x1 + 6*x2 + 9*x3 + 12*x4 + 15*x5 + 18*x6 + 21*x7 + 24*x8 + 27*x9 + 30*x10  <= 120;

subject to cons3: 0 <= + 4*x1 + 8*x2 + 12*x3 + 16*x4 + 20*x5 + 24*x6 + 28*x7 + 32*x8 + 36*x9 + 40*x10  <= 130;

subject to cons4: 0 <= + 5*x1 + 10*x2 + 15*x3 + 20*x4 + 25*x5 + 30*x6 + 35*x7 + 40*x8 + 45*x9 + 50*x10  <= 140;

subject to cons5: 0 <= + 6*x1 + 12*x2 + 18*x3 + 24*x4 + 30*x5 + 36*x6 + 42*x7 + 48*x8 + 54*x9 + 60*x10  <= 150;

subject to cons6: 0 <= + 7*x1 + 14*x2 + 21*x3 + 28*x4 + 35*x5 + 42*x6 + 49*x7 + 56*x8 + 63*x9 + 70*x10  <= 160;

subject to cons7: 0 <= + 8*x1 + 16*x2 + 24*x3 + 32*x4 + 40*x5 + 48*x6 + 56*x7 + 64*x8 + 72*x9 + 80*x10  <= 170;

subject to cons8: 0 <= + 9*x1 + 18*x2 + 27*x3 + 36*x4 + 45*x5 + 54*x6 + 63*x7 + 72*x8 + 81*x9 + 90*x10  <= 180;

subject to cons9: 0 <= + 10*x1 + 20*x2 + 30*x3 + 40*x4 + 50*x5 + 60*x6 + 70*x7 + 80*x8 + 90*x9 + 100*x10  <= 190;
*/
#include <iostream>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "Problem.h"
#include "Bnb.h"

using namespace Minotaur;

int main()
{
    // Create environment
    EnvPtr env = (EnvPtr)new Environment();

    // Create the problem instance
    ProblemPtr problem = (ProblemPtr)new Problem(env);
    std::vector<VariablePtr> vars(10);
    for (int i = 0; i < 10; ++i) {
        if(i%2)
            vars[i] = problem->newVariable(-100.0, 10.0, Continuous, "x" + std::to_string(i + 1));// xi is an Continuous
        else
            vars[i] = problem->newVariable(-100.0, 10.0, Integer, "x" + std::to_string(i + 1));// xi is an Integer
    } 

    // Define the objective function: Maximize 5*x1 + 6*x2 + ... + 14*x10
    LinearFunctionPtr lf = new LinearFunction();
    for (int i = 0; i < 10; ++i) {
        lf->addTerm(vars[i], 5.0 + i);  // The coefficient increases with i
    }

    FunctionPtr f = new Function(lf);
    ObjectiveType otyp = Maximize;
    problem->newObjective(f, 0, otyp);

    // Add 10 constraints: random linear constraints
    for (int i = 0; i < 10; ++i) {
        LinearFunctionPtr constraint_lf = new LinearFunction();
        for (int j = 0; j < 10; ++j) {
            constraint_lf->addTerm(vars[j], (i + 1) * (j + 1));  // Example: i*j as coefficients
        }
        FunctionPtr cf = new Function(constraint_lf);
        problem->newConstraint(cf, 0.0, 100.0 + i * 10);  // RHS varies with i
    }

    // Output initial values
    std::cout << "===========================Before Solving============================" << std::endl;
    std::cout << "Initial values:" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << "x" << i + 1 << " = " << vars[i]->getInitVal() << std::endl;
    }
    problem->write(std::cout);
    std::cout << "===========================After Solving============================" << std::endl;
    
    // Solve the problem using Bnb
   
    Bnb bnb(env);
    bnb.solve(problem);
    bnb.getSolution();
    std::cout << "=====================================================================\n";
}
