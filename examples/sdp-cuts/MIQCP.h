/*
 * MIQCP.h
 *
 *  Created on: Jun 26, 2014
 *      Author: hongbodong
 */

#ifndef MIQCP_H_
#define MIQCP_H_

#include <cmath> // for INFINITY
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>

//#include "MinotaurConfig.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "Function.h"
//#include "IpoptEngine.h"
//#include "Option.h"
#include "LinearFunction.h"
#include "Problem.h"
#include "QuadraticFunction.h"
#include "Solution.h"
#include "Timer.h"
#include "Constraint.h"
//#include "HessianOfLag.h"
#include "Eigen.h"
#include <boost/scoped_array.hpp>

typedef enum
{
  ProbTypeBoxQP
} MINLPType;

typedef enum
{
  Perturbe,
  Linearize
} CutMethod;


namespace Minotaur
{
  class QuadCons;
  class MIQCP;
  typedef boost::shared_ptr<QuadCons> QuadConsPtr;

  class QuadCons
  {
  public:
    ///Default constructor
    QuadCons();
    QuadCons(ProblemPtr prob, FunctionPtr f, Double rhs);
    QuadCons(ProblemPtr prob, FunctionPtr f, Double rhs, std::string name);
    ~QuadCons();

    // Evaluate lower envelope of x^2
    static Double evalLenv(ConstVariablePtr x, Double val);

    // Evaluate upper envelope of x^2
    static Double evalUenv(ConstVariablePtr x, Double val);

    //Generate a convex quadratic relaxed constraint
    std::pair<FunctionPtr, Double> genCvxQuad(ConstSolutionPtr sol, CutMethod mtd);

    Double getMinEig();

    // Clone the quadratic part
    QuadraticFunctionPtr cloneQuadF();

    // Clone the linear part
    LinearFunctionPtr cloneLinF();

    // Get the constraint upper bound
    Double getRHS() {return _rhs;};

  private:
    // Eigenvalue/vectors of the original Hessian matrix
    EigenPtr _eigs;

    // Min/Max eigenvalue of the original Hessian
    Double _mineig;
    Double _maxeig;

    // Function Pointer
    FunctionPtr _f;

    // Right hand side of the constraint, together with _f,
    // the constraint is _f(x) <= _rhs
    Double _rhs;

    // Whatever you want to name it
    std::string _name;

    //variables in the quadratic function. Order corresponds to
    //the ordering in _HessPtr.
    VarIntMap _order;

    // Scale factor for the stored Hessian matrix
    Double _HessScale;
    // Stores the rescaled Hessian matrix.
    // _HessScale*_HessScaledPtr[][] is the original Hessian matrix
    Double* _HessScaledPtr;

    // Associated problem. Used to access variables.
    ProblemPtr _prob;
  };

  class MIQCP
  {
  public:
    // Create _prob with variables, objective and all constraints
    // except nonconvex quadratic constraints.
    // Fill _ncvxRows corresponding to them.
    MIQCP(MINLPType minlptype, std::string filename);

    // Solve _prob, write _sol and _engstatus
    void solveRelax();

    // Print solution the in _sol
    void writeSol();

    // For each nonconvex quadratic constraint,
    // try to generate and add new convex constraints to separate _sol
    bool separateByDiagPerturbation();

    // Write the problem, for debugging
    void writeProblem() {_prob->write(std::cout);};

    Double getTime(Int &err) {return _env->getTime(err);}

    Double getObj(Int &err) {return _prob->getObjValue(_sol->getPrimal(),&err);}

  private:
    /////////////////////////////////////////////////////////////////
    // The following functions are called by the constructor according
    // to the value of minlptype. Read in problem, put all nonconvex
    // quadratic constraints in _ncvxRows, and all other things in _prob

    // If minlptype=ProbTypeBoxQP
    void init_boxqp(std::string filename);
    /////////////////////////////////////////////////////////////////

    // Take all nonconvex quadratic constraints, construct eigenvalue
    // perturbation relaxations, and add to _prob.
    void initRelax();

    // Contains all the nonconvex quadratic constraints
    std::vector<QuadConsPtr> _ncvxRows;

    ProblemPtr _prob;

    ConstSolutionPtr _sol;

    EngineStatus _engstatus;

    std::string _me;

    MINLPType _type;

    EnvPtr _env;

    // Not used for now.
    HandlerVector _handlers;
  };
}


#endif /* MIQCP_H_ */
