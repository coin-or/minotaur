// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file MsProcessor.h
 * \brief Define multi-start node-processor for branch-and-bound
 * \author Prashant Palkar, IIT Bombay
 */

#ifndef MINOTAURMSPROCESSOR_H
#define MINOTAURMSPROCESSOR_H

#include "NodeProcessor.h"
#include "Types.h"

namespace Minotaur {

  class Engine;
  class Problem;
  class Solution;
  typedef const Solution* ConstSolutionPtr;

  struct MBPStats {
    UInt bra;    /// Number of times relaxation became infeasible
    UInt inf;    /// Number of times relaxation became infeasible
    UInt opt;    /// Number of times relaxation gave optimal feasible solution
    UInt prob;   /// Number of times problem ocurred in solving
    UInt proc;   /// Number of nodes processed
    UInt ub;     /// Number of nodes pruned because of bound
  };

  /**
   * \brief Simple multi-start node-processor for branch-and-bound.
   *
   * MsProcessor is a derived class of NodeProcessor. It is meant to solve
   * a relaxation multiple times at each node using different starting points.
   * It performs only pruning and branching in a node. Does not call any
   * presolving or cutting.
   */
  class MsProcessor : public NodeProcessor {

  public:
    /// Default constructor
    MsProcessor(EnvPtr env);

    /// Constructor with a given engine.
    MsProcessor(EnvPtr env, EnginePtr engine, HandlerVector handlers_);

    /// Destroy
    ~MsProcessor();

    // True if a new solution was found while processing this node.
    bool foundNewSolution(); 

    // Find branches that will be used to branch at this node.
    Branches getBranches();

    // Get warm-start information.
    WarmStartPtr getWarmStart();

    // Implement NodeProcessor::process().
    void process(NodePtr node, RelaxationPtr rel, 
                 SolutionPoolPtr s_pool);

    // Get a corner of the variable space box randomly
    double* getBoxCorner(UInt n, RelaxationPtr rel1, 
                         int threadid, int K);

    // Get a corner of the variable space box randomly
    double* getFarBoxCorner(UInt n, RelaxationPtr rel1, 
                            int threadid, double* prev_opt, int K);


    // Generate a random starting point within variable bounds (for relaxations)
    double* getStartPointScheme1(UInt n, RelaxationPtr rel1);

    /**
     * Generate a (re)starting point preferably within variable bounds outside a
     * radius (for relaxations)
     */ 
    double* getStartPointScheme2(UInt n, RelaxationPtr rel1, int threadid,
                                 double radius, int numsols, 
                                 double* prev_start_point);

    /**
     * Generate a (re)starting point preferably within variable bounds outside a
     * radius (for relaxations) in a conjugate direction
     */ 
    double* getStartPointScheme4(UInt n, RelaxationPtr rel1, int threadid,
                                 double radius, int numsols, 
                                 double* prev_start_point, 
                                 double* prev_opt, double costhetalim);

    /**
     * Generate a starting point as the convex combination of prev optimal
     * and the farthest box corner from that point.
     */ 
    double* getStartPointScheme5(UInt n, RelaxationPtr rel1, int threadid, 
                                 double radius, double* prev_start_point,
                                 double* prev_opt, int K, double lambda);

    /**
     * Generate a starting point and take a direction as the linear
     * combination of active constraint gradients at that point; choose step
     * length based on the function decrease along this direction and
     * average constraint violation.
     */    
    //double* getStartPointScheme6(UInt n, RelaxationPtr rel1, int threadid,
                                 //double radius, int numsols, 
                                 //double* prev_start_point,
                                 //double* prev_opt, int K, double lambda);

    // Generate initial point based on variable fixing
    double* genInitialPoint(UInt n, RelaxationPtr rel);

    // Calculating inner product of two vectors
    double InnerProduct(double b[], double c[], UInt n);

    // Calculating Euclidean norm
    double ENorm(double b[], UInt n);

    // Calculate the Euclidean distance between two vectors
    double EDist(double b[], double c[], UInt n);

    // Testing parallelism
    void par(); 	      

    // write statistics. Base class method.
    void writeStats(std::ostream &out) const; 

    // write statistics. Base class method.
    void writeStats() const; 

  protected:

    /// Modifications done to NLP before solving it.
    std::stack<Modification *> relMods_;

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

    /// Environment pointer
    EnvPtr env_;

    /// All the handlers that are used for this processor
    HandlerVector handlers_;

    /// Log
    LoggerPtr logger_;

    /// For logging.
    static const std::string me_;

    /// How many new solutions were found by the processor.
    UInt numSolutions_;

    /// Number of restarts to improve the first intial point
    UInt numRestarts_;

    /// Number of processing cores to be used by the processor
    UInt numThreads_;

    /// Relaxation that is processed by this processor.
    RelaxationPtr relaxation_;

    /// Scheme id for generating initial point
    UInt schemeId_;

/// Statistics
    MBPStats stats_;

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

    /// Solve the relaxation.
    virtual void solveRelaxation_(EnginePtr e1);


    /**
     * Check if a node can be pruned either because the relaxation is
     * infeasible or because the cost is too high.
     */
    virtual bool shouldPrune_(NodePtr node, double solval, 
                              SolutionPoolPtr s_pool);

  };

  typedef MsProcessor* MsProcessorPtr;

}
#endif

