// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file ParBndProcessor.h
 * \brief Define simple node-processor for parallel branch-and-bound
 * \author Prashant Palkar, IIT Bombay
 */

#ifndef MINOTAURPARBNDPROCESSOR_H
#define MINOTAURPARBNDPROCESSOR_H

#include "NodeProcessor.h"
#include "Types.h"

namespace Minotaur {

  class Solution;
  class Engine;
  typedef Engine* EnginePtr;
  typedef const Solution* ConstSolutionPtr;

  struct ParBPStats {
    UInt bra;    /// Number of times relaxation became infeasible
    UInt inf;    /// Number of times relaxation became infeasible
    UInt opt;    /// Number of times relaxation gave optimal feasible solution
    UInt prob;   /// Number of times problem ocurred in solving
    UInt proc;   /// Number of nodes processed
    UInt ub;     /// Number of nodes pruned because of bound
  };

  /**
   * \brief Simple node-processor for branch-and-bound.
   *
   * BndProcessor is a derived class of NodeProcessor. It is meant to solve
   * relaxations at each node. It performs only pruning and branching in a
   * node. Does not call any presolving, cutting, or heuristic search.
   */
  class ParBndProcessor : public NodeProcessor {

    friend class ParBranchAndBound;

  public:
    /// Default constructor
    ParBndProcessor();

    /// Constructor with a given engine.
    ParBndProcessor(EnvPtr env, EnginePtr engine, HandlerVector handlers_);

    /// Destroy
    ~ParBndProcessor();

    // True if a new solution was found while processing this node.
    bool foundNewSolution(); 

    // Find branches that will be used to branch at this node.
    Branches getBranches();

    // Get the recent solution from the engine.
    ConstSolutionPtr getSolution();

    // Get warm-start information.
    WarmStartPtr getWarmStart();

    // Implement NodeProcessor::process().
    void process(NodePtr node, RelaxationPtr rel, 
                 SolutionPoolPtr s_pool);

    /** \brief Process a node. 
     *
     * \param [in] init is true if pseudocosts have been initialized
    */  
    void process(NodePtr node, RelaxationPtr rel, 
                 SolutionPoolPtr s_pool, bool init);

    // write statistics. Base class method.
    void writeStats(std::ostream &out) const; 

    // write statistics. Base class method.
    void writeStats() const; 

  protected:

    /// Branches found by this processor for this node
    Branches branches_;

    /**
     * If true, we continue to search, if engine reports error. If false,
     * we assume that the relaxation is infeasible when engine returns error.
     */
    bool contOnErr_;

    /// If lb is greater than cutOff_, we can prune this node.
    double cutOff_;

    /// Engine used to process the relaxation
    EnginePtr engine_;

    /// Status of the engine
    EngineStatus engineStatus_;

    /// All the handlers that are used for this processor
    HandlerVector handlers_;

    /// Log
    LoggerPtr logger_;

    /// For logging.
    static const std::string me_;

    /// How many new solutions were found by the processor.
    UInt numSolutions_;

    /// Relaxation that is processed by this processor.
    RelaxationPtr relaxation_;

    /// Statistics
    ParBPStats stats_;

    /// Warm-start information for start processing the children
    WarmStartPtr ws_;

    /**
     * Check if the solution is feasible to the original problem. 
     * In case it is feasible, we can store the solution and update the
     * upper bound. Additionally, if the solution is optimal for the
     * current node, then the node can be pruned.
     */
    virtual bool isFeasible_(NodePtr node, ConstSolutionPtr sol, 
                             SolutionPoolPtr s_pool, bool &should_prune);

    /// Solve the relaxation.
    virtual void solveRelaxation_();

    /**
     * Check if a node can be pruned either because the relaxation is
     * infeasible or because the cost is too high.
     */
    virtual bool shouldPrune_(NodePtr node, double solval, 
                              SolutionPoolPtr s_pool);

  };

  typedef ParBndProcessor* ParBndProcessorPtr;

}
#endif

