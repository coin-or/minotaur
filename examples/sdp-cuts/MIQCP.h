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
#include <stdlib.h>
#include <algorithm>

//#include "MinotaurConfig.h"

#include "Environment.h"
#include "FilterSQPEngine.h"
#include "Function.h"
#include "Objective.h"
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
#include <boost/shared_array.hpp>



typedef enum
{
  ProbTypeBoxQP,
  AMPLstubInput
} MINLPType;

typedef enum
{
  InitialRelax,
  Perturbe,
  Linearize
} CutMethod;

void printVec(double* vec, int n);
void printMat(double* mat, int n);

namespace Minotaur
{
  class QuadCons;
  class MIQCP;
  class VarMap;
  typedef boost::shared_ptr<QuadCons> QuadConsPtr;
  typedef boost::shared_ptr<VarMap> VarMapPtr;
  typedef const boost::shared_ptr<VarMap> ConstVarMapPtr;
  typedef boost::shared_array<Double> DoubleArray;

  // This class stores the mapping/linking between variables, such as
  // (1) original variables (in the original problem object)
  //  and the relaxed variables (in the relaxed problem object);
  // (2) relaxed variables and their lifted (squares) copy.
  class VarMap
  {
  public:
    // Default constructor
    VarMap();

    // Establish the link between the original variable and relax variable
    void LinkOriRelax(ConstVariablePtr ori, ConstVariablePtr relax);

    // Establish the link between relaxed variable and squared lifting variable
    void LinkRelaxSqr(ConstVariablePtr relax, ConstVariablePtr sqrvar);

    ConstVariablePtr getRelaxFromOri(ConstVariablePtr ori) const;

    ConstVariablePtr getSqrFromRelax(ConstVariablePtr ori) const;

    ConstVariablePtr getOriFromRelax(ConstVariablePtr ori) const;

    void write(std::ostream& out) const;
  private:
    VarVarMap _ori2relax;
    VarVarMap _relax2ori;
    VarVarMap _relax2sqr;
  };

  // This class stores one nonconvex quadratic constraint (a nonconvex row) in the original problem
  class QuadCons
  {
  public:
    ///Default constructor
    QuadCons();
    QuadCons(ProblemPtr prob, FunctionPtr f, Double rhs);

    // [in] prob -- the (relaxed) problem
    // [in] f    -- f(x)<=rhs
    // [in] rhs  -- f(x)<=rhs
    // [in] name -- row name
    QuadCons(ProblemPtr prob, FunctionPtr f, Double rhs, std::string name);
    ~QuadCons();

    // Evaluate lower envelope of x^2
    static Double evalLenv(ConstVariablePtr x, Double val);

    // Evaluate upper envelope of x^2
    static Double evalUenv(ConstVariablePtr x, Double val);

    //Generate a convex quadratic relaxed constraint
    std::pair<FunctionPtr, Double> genCvxQuad(CutMethod mtd, ConstSolutionPtr sol,
        VarMapPtr varmap);

    Double getMinEig();

    // Clone the quadratic part
    QuadraticFunctionPtr cloneQuadF();

    // Clone the linear part
    LinearFunctionPtr cloneLinF();

    // Get the constraint upper bound
    Double getRHS() {return _rhs;};

    // Get the constraint upper bound
    const QuadraticFunctionPtr getQuadF() const {return _f->getQuadraticFunction();};

    // Whether the quadratic part has a certain variable
    bool hasVarInQuad(ConstVariablePtr var) const;

  private:
    // Eigenvalue/vectors of the original Hessian matrix
    EigenPtr _eigs;

    // Min/Max eigenvalue of the original Hessian
    Double _mineig;
    Double _maxeig;

    // Function Pointer, use relaxed variables
    FunctionPtr _f;

    // Right hand side of the constraint, together with _f,
    // the constraint is _f(x) <= _rhs
    Double _rhs;

    // Whatever you want to name it
    std::string _name;

    //variables in the quadratic function. Order corresponds to
    //the ordering in _HessPtr. takes values 0 ~ (n-1).
    VarIntMap _order;

    // Scale factor for the stored Hessian matrix
    Double _HessScale;
    // Stores the rescaled Hessian matrix.
    // _HessScale*_HessScaledPtr[][] is the original Hessian matrix
    //Double* _HessScaledPtr;
    DoubleArray _HessScaledPtr;

    // Rescaled initial d, usually taken as -1.1*(most negative eig)
    //Double* _initd;
    DoubleArray _initScaled;

    // Stores the matrix inv(_HessScaledPtr+diag(_initd))
    //Double* _InvHessPlusInitd;
    DoubleArray _InvHessPlusInitd;

    // Associated problem. Used to access variables.
    ProblemPtr _prob;
  };

  // Problem class MIQCP
  class MIQCP
  {
  public:
    // Constructor from a problem class
    // MIQCP(ProblemPtr p);

    // Create _prob with variables, objective and all constraints
    // except nonconvex quadratic constraints.
    // Fill _ncvxRows corresponding to them.
    // After calling initRelax(), set up the engine.
    MIQCP(MINLPType minlptype, std::string filein);

    // Solve the problem stored in engine, write _sol, _engstatus and
    // [out] comptime
    void solveEngRelax(Double & comptime, bool useInitial=false);

    EngineStatus getEngStatus() {return _engstatus;};

    // Print solution the in _sol
    void writeSol(bool ifwriteprimal=false);

    // For each nonconvex quadratic constraint,
    // try to generate and add new convex constraints to separate _sol
    bool separateByDiagPerturbation(Double& septime);

    // Write the problem, for debugging
    void writeProblem(bool ifOrig=true, bool ifrelax=true) {_orig_prob->write(std::cout); _prob->write(std::cout);};

    Double getTime(Int &err) {return _env->getTime(err);};

    Double getObj(Int &err) {return _prob->getObjValue(_sol->getPrimal(),&err);};

  private:
    /////////////////////////////////////////////////////////////////
    // The following functions are called by the constructor according
    // to the value of minlptype. Read in problem, put all nonconvex
    // quadratic constraints in _ncvxRows, and all other things in _prob

    // If minlptype=ProbTypeBoxQP
    void init_boxqp(std::string filein);
    void init_ampl(std::string filein);
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // This function complete the following tasks:
    // 1. For all integer variable that appears in some nonconvex
    //    quadratic constraints, create its lifted square variables;
    //    Add the 2-dimensional convex hull constraints for each pair;
    //
    // 2. Take all nonconvex quadratic constraints, construct eigenvalue
    //    perturbation relaxations, and add to _prob.
    //
    // This function should be called in constructors.
    void initRelax();
    /////////////////////////////////////////////////////////////////

    // Add constraints according to two-dimensional convex hull
    void add2dimCvxHull(ConstVariablePtr x, ConstVariablePtr xsqr);

    // Contains all the nonconvex quadratic constraints
    std::vector<QuadConsPtr> _ncvxRows;

    // Original nonconvex problem (w. integral vars)
    // Use _prob whenever possible. Currently I can only think of the
    // need of _orig_prob when original variables types are of interests
    ProblemPtr _orig_prob;

    // Relaxed problem.
    ProblemPtr _prob;

    // Stores the mapping/linkage between variables, such as
    // (1) original variables (in the original problem object)
    //  and the relaxed variables (in the relaxed problem object);
    // (2) relaxed variables and their lifted (squares) copy.
    VarMapPtr _varmap;

    ConstSolutionPtr _sol;

    EngineStatus _engstatus;

    EnginePtr _engine;

    EnvPtr _env;

    std::string _me;

    MINLPType _type;

    // Not used for now.
    HandlerVector _handlers;
  };
}


#endif /* MIQCP_H_ */
