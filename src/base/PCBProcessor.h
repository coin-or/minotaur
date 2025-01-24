//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file PCBProcessor.h
 * \brief Define the derived class of NodeProcessor that solves LP
 * or NLP relaxations.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPCBPROCESSOR_H
#define MINOTAURPCBPROCESSOR_H

#include "NodeProcessor.h"
#include "Types.h"

namespace Minotaur
{

class CutManager;
//class Problem;

struct NodeStats {
  UInt bra;     /// Number of times relaxation became infeasible
  UInt inf;     /// Number of times relaxation became infeasible
  UInt opt;     /// Number of times relaxation gave optimal feasible solution
  UInt prob;    /// Number of times problem ocurred in solving
  UInt proc;    /// Number of nodes processed
  UInt ub;      /// Number of nodes pruned because of bound
  UInt tol_err; /// Number of nodes for which fixNodeErr was called
};

/**
   * \brief Default node processor used in solver for now.
   *
   * PCBProcessor is a derived class of NodeProcessor. It is meant to solve LPs
   * at each node. It is used in a simple MILP branch-and-bound. 
   * As a stop-gap measure, it is being used for MINLP branch-and-bound as
   * well.
   */
class PCBProcessor : public NodeProcessor
{

public:
  /// Default constructor
  PCBProcessor() { }

  /// Constructor with a given engine.
  PCBProcessor(EnvPtr env, EnginePtr engine, HandlerVector handlers_);

  /// Destroy
  ~PCBProcessor();

  // Add a heuristic.
  void addHeur(HeurPtr h);

  // True if a new solution was found while processing this node.
  bool foundNewSolution();

  // Find branches that will be used to branch at this node.
  Branches getBranches();

  // Get warm-start information.
  WarmStartPtr getWarmStart();

  // Implement NodeProcessor::process().
  void process(NodePtr node, RelaxationPtr rel, SolutionPoolPtr s_pool);

  void setCutManager(CutManager* cutman);

  // write statistics. Base class method.
  void writeStats(std::ostream& out) const;

  // write statistics. Base class method.
  void writeStats() const;

private:
  /// Branches found by this processor for this node
  Branches branches_;

  /**
       * If true, we continue to search, if engine reports error. If false,
       * we assume that the relaxation is infeasible when engine returns error.
       */
  bool contOnErr_;

  /// The cut manager.
  CutManager* cutMan_;

  /// If lb is greater than cutOff_, we can prune this node.
  double cutOff_;

  /// Engine used to process the relaxation
  EnginePtr engine_;

  /// Status of the engine
  EngineStatus engineStatus_;

  /// Pointer to environment
  EnvPtr env_;

  /// All the handlers that are used for this processor
  HandlerVector handlers_;

  /// Heuristics that can be called at each node.
  HeurVector heurs_;

  /// The handler which reports the infeasibility of a node.
  HandlerPtr infHand_;

  /// Log
  LoggerPtr logger_;

  /// For logging.
  static const std::string me_;

  /// How frequently should node-presolve be called? If 1, then call at
  /// all nodes. If 0, then never. If 4, then every fourth node, etc.
  int presFreq_;

  /// How many new solutions were found by the processor.
  UInt numSolutions_;

  /// Absolute tolerance for pruning a node on basis of bounds.
  double oATol_;

  /// Relative tolerance for pruning a node on basis of bounds.
  double oRTol_;

  /// Relaxation that is processed by this processor.
  RelaxationPtr relaxation_;

  /// Statistics
  NodeStats stats_;

  /// Warm-start information for start processing the children
  WarmStartPtr ws_;

  /**
       * Check if the solution is feasible to the original problem. 
       * In case it is feasible, we can store the solution and update the
       * upper bound. Additionally, if the solution is optimal for the
       * current node, then the node can be pruned.
       */
  virtual bool isFeasible_(NodePtr node, ConstSolutionPtr sol,
                           SolutionPoolPtr s_pool, bool& should_prune);

  /// Presolve a node.
  virtual bool presolveNode_(NodePtr node, SolutionPoolPtr s_pool);

  /// Solve the relaxation.
  virtual void solveRelaxation_();

  /**
       * Check if a node can be pruned either because the relaxation is
       * infeasible or because the cost is too high.
       */
  virtual bool shouldPrune_(NodePtr node, double solval,
                            SolutionPoolPtr s_pool);

  /**
       * Remove optional cuts which are not required.
       */
  void removeCuts_(ConstSolutionPtr sol);

  /// Separate the given point from the node relaxation
  /**
       * Call each handler and generate cuts. Cuts could be local or global.
       * Local cuts must be added to the node (not implemented yet). Global
       * cuts must be added to a global pool (not implemented yet). Separation
       * status is SepPrune if there is no further need to solve the
       * subproblem. It is SepResolve if the relaxation needs to be resolved.
       */
  void separate_(ConstSolutionPtr sol, NodePtr node, SolutionPoolPtr s_pool,
                 SeparationStatus* status);

  // Implement NodeProcessor::tightenBounds_()
  virtual void tightenBounds_(NodePtr node, SolutionPoolPtr s_pool,
                              ConstSolutionPtr sol, SeparationStatus* status);
};

typedef PCBProcessor* PCBProcessorPtr;

} // namespace Minotaur
#endif

