/*
	Example: CGraph create using Quadratic and Linear Functions
	
	Vars:

	x1	x2	x3

	H:

	1	2	3
	4	5	6
	7	8	9

	f:

	1	2	3

	Output:

	(((0.5 * (x1)^2) + (6 * (x1 * x2)) + (10 * (x1 * x3)) + (2.5 * (x2)^2)
	+ (14 * (x2 * x3)) + (4.5 * (x3)^2)) + ((1 * x1) + (2 * x2) + (3 * x3)))	
*/
#include <cmath>
#include <iostream>
#include <iomanip>

#include "CGraph.h"
#include "CNode.h"
#include "Problem.h"
#include "Environment.h"
#include "Variable.h"
#include "LinearFunction.h"
#include "QuadraticFunction.h"

using namespace Minotaur;

int main()
{
	double H[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	double f[] = {1, 2, 3};
	EnvPtr env = new Environment();
	ProblemPtr p = (ProblemPtr) new Problem(env);

	VariablePtr x1 = (VariablePtr) (p->newVariable(-INFINITY, +INFINITY, Integer, "x1"));
	VariablePtr x2 = (VariablePtr) (p->newVariable(-INFINITY, +INFINITY, Integer, "x2"));
	VariablePtr x3 = (VariablePtr) (p->newVariable(-INFINITY, +INFINITY, Integer, "x3"));

	VariableConstIterator vbeg = p->varsBegin();
	VariableConstIterator vend = p->varsEnd();

	QuadraticFunctionPtr qf = (QuadraticFunctionPtr) new QuadraticFunction(H, vbeg, vend);
	LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction(f, vbeg, vend, 1e-9);

	CGraphPtr cg = (CGraphPtr) new CGraph();

	// Handle linear terms
	CNode* lf_out = nullptr;
	if (lf) {
		// Assuming `lf` is a LinearFunction, iterate over its terms.
		for (VariableGroupConstIterator it = lf->termsBegin(); it != lf->termsEnd(); ++it) {
			CNode* var_node = cg->newNode(it->first);  // Create a node for the variable
			CNode* coef_node = cg->newNode(it->second);  // Create a node for the coefficient
			CNode* term_node = cg->newNode(OpMult, coef_node, var_node);  // Multiply coefficient and variable
			if (!lf_out) {
				lf_out = term_node;  // Initialize first term
			} else {
				lf_out = cg->newNode(OpPlus, lf_out, term_node);  // Sum up the terms
			}
		}
	}

	// Handle quadratic terms
	CNode* qf_out = nullptr;
	if (qf) {
		// Iterate over the quadratic terms using begin() and end() iterators
		for (VariablePairGroupConstIterator it = qf->begin(); it != qf->end(); ++it) {
			ConstVariablePtr v1 = it->first.first;  // First variable in the pair
			ConstVariablePtr v2 = it->first.second;  // Second variable in the pair
			double coef = it->second;  // Coefficient of the term

			CNode* var1_node = cg->newNode(v1);  // Create node for the first variable
			CNode* var2_node = cg->newNode(v2);  // Create node for the second variable
			CNode* coef_node = cg->newNode(coef);  // Create node for the coefficient
			CNode* term_node = cg->newNode(OpMult, var1_node, var2_node);  // Multiply the two variables
			CNode* quad_term = cg->newNode(OpMult, coef_node, term_node);  // Multiply by the coefficient

			if (!qf_out) {
				qf_out = quad_term;  // Initialize first term
			} else {
				qf_out = cg->newNode(OpPlus, qf_out, quad_term);  // Sum up the terms
			}
		}
	}

	// Combine linear and quadratic terms
	CNode* out = nullptr;
	if (lf_out && qf_out) {
		out = cg->newNode(OpPlus, lf_out, qf_out);
	} else if (lf_out) {
		out = lf_out;
	} else if (qf_out) {
		out = qf_out;
	}

	if (out) {
		cg->setOut(out);
	}

	cg->finalize();



	cg->write(std::cout);
	std::cout<<std::endl;

	return 0;
}
