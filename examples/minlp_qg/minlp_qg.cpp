//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
//

// Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

/*
Mixed Integer Non-Linear Program
################################################
var x1 >= 0 <= 1  binary ;
var x2 >= -10 <= 10 ;
minimize obj: + 1*x1*x1 + 1*x2*x2  - 0;

subject to cons0:  +  nonlinear function  <= 1;

subject to cons1: 1 <= + 1*x1 + 1*x2 ;
#################################################
Where nonlinear function = x1 * x2^2
#################################################
*/
#include <iostream>
#include <cmath>
#include "MinotaurConfig.h"
#include "Environment.h"
#include "Problem.h"
#include "QG.h"
#include "NonlinearFunction.h"

using namespace Minotaur;

// Custom nonlinear function: x1 * x2^2
class CustomNonlinearFunction : public NonlinearFunction {
public:
    // Evaluate the function: f(x1, x2) = x1 * x2^2
    double eval(const double* x, int* error) override {
        *error = 0;
        return x[0] * std::pow(x[1], 2);
    }

    // Gradient of the function
    void evalGradient(const double* x, double* grad_f, int* error) override {
        *error = 0;
        grad_f[0] = std::pow(x[1], 2);   // df/dx1 = x2^2
        grad_f[1] = 2 * x[0] * x[1];     // df/dx2 = 2*x1*x2
    }

    // Hessian is not needed for this example, so just leave empty
    void evalHessian(double, const double*, const LTHessStor*, double*, int*) override {}

    // Other virtual functions that need to be implemented
    void fillHessStor(LTHessStor*) override {}
    void fillJac(const double*, double*, int*) override {}
    void finalHessStor(const LTHessStor*) override {}
    void getVars(VariableSet*) override {}
    NonlinearFunctionPtr cloneWithVars(VariableConstIterator, int*) const override { return nullptr; }
    void multiply(double) override {}
    void prepJac(VarSetConstIter, VarSetConstIter) override {}
};

int main() {
    // Create environment
    EnvPtr env = new Environment();
    env->getOptions()->findBool("use_native_cgraph")->setValue(true);

    // Create the problem instance
    ProblemPtr problem = new Problem(env);

    // Define decision variables
    VariablePtr x1 = problem->newVariable(0, 1, Binary, "x1"); // Binary variable
    VariablePtr x2 = problem->newVariable(-10, 10, Continuous, "x2"); // Continuous variable

    // Define the objective function: Minimize x1^2 + x2^2
    QuadraticFunctionPtr qf = new QuadraticFunction();
    qf->addTerm(x1, x1, 1.0);  // x1^2
    qf->addTerm(x2, x2, 1.0);  // x2^2

    FunctionPtr obj_f = new Function(nullptr, qf);
    problem->newObjective(obj_f, 0.0, Minimize);

    // Add nonlinear constraint: x1 * x2^2 <= 1
    NonlinearFunctionPtr nlf = new CustomNonlinearFunction(); // Custom nonlinear function
    FunctionPtr nl_con_f = (FunctionPtr)new Function(nullptr, nullptr, nlf);
    problem->newConstraint(nl_con_f, -INFINITY, 1.0);  // Constraint: x1 * x2^2 <= 1

    // Add linear constraint: x1 + x2 >= 1
    LinearFunctionPtr lf = new LinearFunction();
    lf->addTerm(x1, 1.0);
    lf->addTerm(x2, 1.0);
    FunctionPtr lin_con_f = new Function(lf);
    problem->newConstraint(lin_con_f, 1.0, INFINITY);  // Constraint: x1 + x2 >= 1

    // Print initial values
    std::cout << "===========================Before Solving============================\n";
    std::cout << "Initial values:" << std::endl;
    std::cout << "x1 = " << x1->getInitVal() << std::endl;
    std::cout << "x2 = " << x2->getInitVal() << std::endl;

    problem->write(std::cout);

    std::cout << "===========================After Solving============================\n";

    // Solve the problem using QG
    QG qg(env);
    qg.solve(problem);
    qg.getSolution();

    std::cout << "=====================================================================\n";

    return 0;
}
