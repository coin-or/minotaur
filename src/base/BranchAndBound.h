// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file BranchAndBound.h
 * \brief Declare the BranchAndBound class for branch-and-bound-algorithm,
 * BabStats class for statistics and BabOptions class for options.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURBRANCHANDBOUND_H
#define MINOTAURBRANCHANDBOUND_H

#include "Types.h"
#include "Environment.h"
#include "Brancher.h"
#include "Heuristic.h"
#include "NodeProcessor.h"
#include "NodeRelaxer.h"
#include "Problem.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "TreeManager.h"

namespace Minotaur {

  struct  BabOptions;
  struct  BabStats;
  typedef BabOptions* BabOptionsPtr;


  /**
   * \brief Implement a generic branch-and-bound algorithm on a single cpu. 
   */
  class BranchAndBound {

  public:
    /// Default constructor.
    BranchAndBound();

    /// Constructor for a given Problem and Environment.
    BranchAndBound(EnvPtr env, ProblemPtr problem);

    /// Destroy.
    virtual ~BranchAndBound();

    /** 
     * \brief Add a heuristic that will be called before root node.
     * \param [in] h The heuristic that should be called. This heuristic will
     * be called after all previously added heuristic.
     */
    void addPreRootHeur(HeurPtr h);

    /**
     * \brief Return the percentage gap between the lower and upper bounds. 
     * 
     * Gap percentage is calculated as
     * \f$\frac{u - l}{\left|u\right|+\epsilon} \times 100\f$, where \f$u\f$
     * is the upper bound, \f$l\f$ is the lower bound and \f$\epsilon\f$ is a
     * small constant to avoid division by zero.
     */
    double getPerGap();

    /**
     * \brief Return the lower bound from the search tree.
     *
     * This bound is defined as the minimum of the bounds from all active
     * nodes. It may not be a bound on the optimal solution value.
     */
    double getLb();

    /// Return a pointer to NodeProcessor used in branch-and-bound.
    NodeProcessorPtr getNodeProcessor();

    /// Return a pointer to NodeRelaxer used in branch-and-bound.
    NodeRelaxerPtr getNodeRelaxer(); 

    /*
     * \brief Return solution from the last solve. If no solution was found, return
     * NULL.
     */
    SolutionPtr getSolution();

    /*
     * \brief Return solution from the root node solve. If no solution was found, return
     * NULL.
     */
    //SolutionPtr getRootSolution();

    /// Return the final status.
    SolveStatus getStatus();

    /**
     * \brief Return a pointer to the tree manager. The client can then
     * directly query the TreeManager for its size and other attributes.
     */
    TreeManagerPtr getTreeManager(); 

    /**
     * \brief Return the upper bound for the solution value from the search tree. 
     *
     * This bound may or may not correspond to a feasible solution of the
     * problem. It may be obtained from a feasible solution of a relaxation of 
     * the problem.
     */
    double getUb();

    /// Return number of nodes processed while solving.
    UInt numProcNodes();

    /**
     * \brief Set log level.
     *
     * \param [in] level The desired log level for this class.
     */
    void setLogLevel(LogLevel level);

    /**
     * \brief Set the NodeProcessor that processes each node.
     *
     * \param [in] p The desired node-processor.
     */
    void setNodeProcessor(NodeProcessorPtr p);

    /**
     * \brief Set the NodeRelaxer for setting-up relaxations at each node.
     *
     * \param [in] nr The desired node-relaxer.
     */
    void setNodeRelaxer(NodeRelaxerPtr nr);

    /**
     * \brief Switch to turn on/off root-node creation.
     *
     * Sometimes a client may set up a root node on its own and
     * may not want the default root node.
     * \param [in] b True if root node should be created, false otherwise.
     */
    void shouldCreateRoot(bool b);

    /// Start solving the Problem using branch-and-bound
    void solve();

    /// Return total time taken
    double totalTime();

    /// Write statistics to the ostream out
    void writeStats(std::ostream & out);

    /// Write statistics to the logger
    void writeStats();

  private:
    /// Pointer to the enviroment.
    EnvPtr env_;

    /// Log manager for displaying messages.
    LoggerPtr logger_;

    /// String name used in log messages.
    static const std::string me_;

    /// The processor to process each node.
    NodeProcessorPtr nodePrcssr_;

    /// The relaxer to create a relaxation at each node.
    NodeRelaxerPtr nodeRlxr_;

    /// Options.
    BabOptionsPtr options_;

    /**
     * \brief Heuristics that need to be called before creating and solving the root
     * node.
     */
    HeurVector preHeurs_;

    /// The Problem that is solved using branch-and-bound.
    ProblemPtr problem_;

    /// The TreeManager used to manage the search tree.
    SolutionPoolPtr solPool_;

    /**
     * \brief Statistics about the branch-and-bound (including time, number of
     * iterations etc.)
     */
    BabStats *stats_;

    /// The status of the branch-and-bound algorithm.
    SolveStatus status_;

    /**
     * \brief Timer for keeping track of time.
     *
     * The user or the environment from which branch-and-bound is called can
     * set up the timer and even start it before sending it to
     * branch-and-bound.
     */
    const Timer *timer_;
   
    /// The TreeManager used to manage the search tree.
    TreeManagerPtr tm_;

    /**
     * \brief Process the root node.
     *
     * \param [out] should_prune True if the root node can be pruned.
     * \param [out] should_dive True if we should dive to a child node.
     */
    NodePtr processRoot_(bool *should_prune, bool *should_dive);

    /// Return True if a node can be pruned.
    bool shouldPrune_(NodePtr node);

    /**
     * \brief Check whether the branch-and-bound can stop because of time
     * limit, or node limit or if solved?
     */
    bool shouldStop_();

    /**
     * \brief Display status: number of nodes, bounds, time etc.
     *
     * \param [in] current_uncounted If True, then the number of nodes in the
     * log-message is incremented by one. This may happen when diving: the
     * node being processed is not in the list of active nodes in the tree.
     */
    void showStatus_(bool current_uncounted,bool last_line);

    void showStatusHead_();
  };

  /// Statistics about the branch-and-bound.
  struct BabStats 
  {
    /// Constructor. All data is initialized to zero.
    BabStats();

    /// Number of nodes processed.
    UInt nodesProc;

    /// Total time used in branch-and-bound.
    double timeUsed;

    /// Time of the last log display.
    double updateTime;
  };


  /// Different options and parameters that control branch-and-bound
  struct BabOptions
  {
    /// Default constructor. 
    BabOptions();

    /// Constructor created from options in environment. 
    BabOptions(EnvPtr env);

    /**
     * \brief Should the root be created in branch-and-bound (yes), or the
     * user calls branch-and-bound after creating the root (no)?
     */
    bool createRoot;

    /// Time in seconds between status updates of the progress.
    double logInterval;

    /// Limit on number of nodes processed.
    UInt nodeLimit;

    /**
     * \brief Stop if the percent gap between lower and upper bounds of the
     * objective function falls below this level.
     */
    double perGapLimit;

    /// Limit on number of nodes processed.
    UInt solLimit;

    /// Time limit in seconds for the branch-and-bound.
    double timeLimit;
  };

  typedef BranchAndBound* BranchAndBoundPtr;
}
#endif

