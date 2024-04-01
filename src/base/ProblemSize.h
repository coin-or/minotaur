//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file ProblemSize.h
 * \brief Declare structure ProblemSize for storing statistics about the problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURPROBLEMSIZE_H
#define MINOTAURPROBLEMSIZE_H

#include "Types.h"

namespace Minotaur {
  /// ProblemSize stores several statistics on the size of the Problem
  struct ProblemSize {
    /// Number of variables in the Problem.
    UInt vars;

    /// Number of constraints in the Problem.
    UInt cons;

    /// Number of objectives in the Problem.
    UInt objs;

    /// Number of binary variables.
    UInt bins;

    /// Number of integer variables not including binary variables
    UInt fixed;

    /// Number of integer variables not including binary variables
    UInt ints;

    /// Number of continuous variables
    UInt conts;

    /// Number of constraints that have a linear function only.
    UInt linCons;

    /// Number of SOS Type 1 constraints.
    UInt SOS1Cons;

    /// Number of SOS Type 2 constraints.
    UInt SOS2Cons;

    /**
     * Number of constraints that have a bilinear function and optionally, a
     * linear function.
     */
    UInt bilinCons;

    /**
     * Number of constraints that have a multilinear function and optionally, 
     * a linear function.
     */
    UInt multilinCons;

    /**
     * Number of constraints that have a quadratic function and optionally, a
     * linear function.
     */
    UInt quadCons;

    /// Number of constraints that have nonlinear function. 
    UInt nonlinCons;

    /// Number of constraints that have a linear function.
    UInt consWithLin;

    /// Number of constraints that have a bilinear function.
    UInt consWithBilin;

    /// Number of constraints that have a multilinear function.
    UInt consWithMultilin;

    /// Number of constraints that have a quadratic function.
    UInt consWithQuad;

    /// Number of constraints that have a nonlinear function.
    UInt consWithNonlin;

    /// Number of terms in the all linear functions in the constraints.
    UInt linTerms;

    /// Number of terms in the all multilinear functions in the constraints.
    UInt multiLinTerms;

    /// Number of terms in the all quadratic functions in the constraints.
    UInt quadTerms;

    /// Number of terms in linear function in the objectives.
    UInt objLinTerms;

    /// Number of terms in quadratic function in the objectives.
    UInt objQuadTerms;

    /// Type of objective: constant, linear, quadratic ...
    FunctionType objType;

    ///Count the number of aggregation type constraints.
    UInt countAggregation;

    ///Count the number of precedence type constraints.
    UInt countPrecedence;

    ///Count the number of variable bound type constraints.
    UInt countVariableBound;

    ///Count the number of set partitioning type constraints.
    UInt countSetPartitioning;

    ///Count the number of set packing type constraints.
    UInt countSetPacking;

    ///Count the number of set covering type constraints.
    UInt countSetCovering;

    ///Count the number of cardinality type constraints.
    UInt countCardinality;

    ///Count the number of invariant knapsack type constraints.
    UInt countInvariantKnapsack;

    ///Count the number of equation knapsack type constraints.
    UInt countEquationKnapsack;

    ///Count the number of bin packing type constraints.
    UInt countBinPacking;

    ///Count the number of knapsack type constraints.
    UInt countKnapsack;

    ///Count the number of integer knapsack type constraints.
    UInt countIntegerKnapsack;

    ///Count the number of mixed binary type constraints.
    UInt countMixedBinary;

    ///Count the number of constraints with no specific structure.
    UInt countNoSpecificStructure;

  };
  typedef  ProblemSize * ProblemSizePtr;
  typedef  const ProblemSize * ConstProblemSizePtr;
}

#endif
    
// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
