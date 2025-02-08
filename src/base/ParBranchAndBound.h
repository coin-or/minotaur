// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file ParBranchAndBound.h
 * \brief Declare the default branch-and-bound-algorithm.
 * \author Prashant Palkar, IIT Bombay
 */

#ifndef MINOTAURPARBRANCHANDBOUND_H
#define MINOTAURPARBRANCHANDBOUND_H

#include "Types.h"
#include <sys/time.h>

namespace Minotaur {

  struct  ParBabOptions;
  struct  ParBabStats;
  class   Engine;
  class   NodeProcessor;
  class   NodeRelaxer;
  class   ParNodeIncRelaxer;
  class   ParPCBProcessor;
  class   ParTreeManager;
  class   Problem;
  class   Solution;
  class   SolutionPool;
  class   WarmStart;
  class   Timer;
  typedef Engine* EnginePtr;
  typedef ParBabOptions* ParBabOptionsPtr;
  typedef NodeProcessor* NodeProcessorPtr;
  typedef NodeRelaxer* NodeRelaxerPtr;
  typedef ParNodeIncRelaxer* ParNodeIncRelaxerPtr;
  typedef ParPCBProcessor* ParPCBProcessorPtr;
  typedef Solution* SolutionPtr;
  typedef SolutionPool* SolutionPoolPtr;
  typedef ParTreeManager* ParTreeManagerPtr;
  typedef WarmStart* WarmStartPtr;

  /**
   * \brief Implement a generic parallel branch-and-bound algorithm on a multicore cpu. 
   */
  class ParBranchAndBound {

  public:
    /// Default constructor.
    ParBranchAndBound();

    /// Constructor for a given Problem and Environment.
    ParBranchAndBound(EnvPtr env, ProblemPtr p);

    /// Destroy.
    virtual ~ParBranchAndBound();

    void printsomething();
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

    /// Return the final status.
    SolveStatus getStatus();

    /**
     * \brief Return a pointer to the tree manager. The client can then
     * directly query the ParTreeManager for its size and other attributes.
     */
    ParTreeManagerPtr getTreeManager(); 

    /**
     * \brief Return the upper bound for the solution value from the search tree. 
     *
     * This bound may or may not correspond to a feasible solution of the
     * problem. It may be obtained from a feasible solution of a relaxation of 
     * the problem.
     */
    double getUb();

    /**
     * \brief Map nodes generated in serial and parallel branch-and-bound tree.
     *
     * The logic assumes that a parent node gives rise to only two children
     * and the one created using the left branch is assigned an id first.
     *
     * \param [in] serVec The vector of node descriptions in serial tree.
     * \param [in] parVec The vector of node descriptions in parallel tree.
     */
    std::vector<std::vector<double> > mapSerialOutput (
      std::vector<std::vector<double> >serVec,
      std::vector<std::vector<double> >parVec);

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

    /// Convert a string to integer data type
    int strToInt(std::string str);

    /**
     * \brief Start solving the Problem using branch-and-bound.
     *
     * \param [in] parNodeRelaxer is the array of node relaxers.
     * \param [in] parPCBProcessor is the array of node processors.
     * \param [in] nThreads is the number of threads being used.
     */ 
    void parsolve(ParNodeIncRelaxerPtr parNodeRelaxer[],
                  ParPCBProcessorPtr parPCBProcessor[],
                  UInt nThreads);

    /**
     * \brief Start solving the Problem using parallel branch-and-bound in an
     * opportunistic mode.
     *
     * \param [in] parNodeRelaxer is the array of node relaxers.
     * \param [in] parPCBProcessor is the array of node processors.
     * \param [in] nThreads is the number of threads being used.
     */
    void parsolveOppor(ParNodeIncRelaxerPtr parNodeRelaxer[],
                  ParPCBProcessorPtr parPCBProcessor[],
                  UInt nThreads);

    /**
     * \brief Branch-and-bound solver with reproducibility of results.
     *
     * \param [in] parNodeRelaxer is the array of node relaxers.
     * \param [in] parPCBProcessor is the array of node processors.
     * \param [in] nThreads is the number of threads being used.
     */
    void parsolveSync(ParNodeIncRelaxerPtr parNodeRelaxer[],
                  ParPCBProcessorPtr parPCBProcessor[],
                  UInt nThreads);

    /// Print a two-dimensional vector (customized).
    void print2dvec(std::vector<std::vector<int> > output);

    /// Print a two-dimensional vector (customized).
    void print2dvec(std::vector<std::vector<double> > output);

    /**
     * \brief Read output of branch-and-bound tree generated by a single thread.
     *
     * Each line of the input file must contain the following node information.
     * NodeId ParentId BrVar Lb ThreadId StartTime EndTime TbScore PruneStatus TreeUb
     * Only the first three columns are used for mapping, rest are for
     * completeness of tree information. Root node information is not needed.
     */
    std::vector<std::vector<double> > readSerialOutput(const char* inputFile);

    /// Return total time taken.
    double totalTime();

    /// Write statistics to the ostream out
    void writeStats(std::ostream & out);

    /// Write statistics of parallel algorithm to the ostream out
    void writeParStats(std::ostream & out,
                       ParPCBProcessorPtr parPCBProcessor[]);

    /// Write statistics to the logger
    void writeStats();

    /// Get wall clock time
    double getWallTime() {
      struct timeval time;
      if (gettimeofday(&time,NULL)) {
        // Handle error
        return 0;
      }
      return (double)time.tv_sec + (double)time.tv_usec * .000001;
    }

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
    ParBabOptionsPtr options_;

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
    ParBabStats *stats_;

    /// The status of the branch-and-bound algorithm.
    SolveStatus status_;

    /**
     * \brief Timer for keeping track of time.
     *
     * The user or the environment from which branch-and-bound is called can
     * set up the timer and even start it before sending it to
     * branch-and-bound.
     */
    Timer *timer_;
   
    /// The TreeManager used to manage the search tree.
    ParTreeManagerPtr tm_;

    /**
     * \brief Process the root node.
     *
     * \param [out] should_prune True if the root node can be pruned.
     * \param [out] should_dive True if we should dive to a child node.
     */
    NodePtr processRoot_(bool *should_prune, bool *should_dive);

    /**
     * \brief Process the root node.
     *
     * \param [out] should_prune True if the root node can be pruned.
     * \param [out] should_dive True if we should dive to a child node.
     * \param [out] parNodeRlxr used to create relaxations of nodes.
     * \param [out] nodePrcssr used to solve a node.
     * \param [out] ws information to be used as warmstart for next node.
     */
    void processRoot_(bool *should_prune, bool *should_dive,
                         ParNodeIncRelaxerPtr parNodeRlxr,
                         ParPCBProcessorPtr nodePrcssr, WarmStartPtr ws,
                         NodePtr &node);


    /// Return True if a node can be pruned.
    bool shouldPrune_(NodePtr node);

    /**
     * \brief Check whether the branch-and-bound can stop because of time
     * limit, or node limit or if solved?
     */
    bool shouldStop_();
    
    /**
     * \brief Check whether the branch-and-bound can stop because of time
     * limit, or node limit or if solved?
     *
     * \param [out] wallStartTime is the start time of branch-and-bound.
     * \param [out] treeLb is the lower bound of the branch-and-bound tree.
     */
    bool shouldStopPar_(double wallStartTime, double treeLb);

    /**
     * \brief Display status: number of nodes, bounds, time etc.
     *
     * \param [in] current_uncounted If True, then the number of nodes in the
     * log-message is incremented by one. This may happen when diving: the
     * node being processed is not in the list of active nodes in the tree.
     */
    void showStatus_(bool current_uncounted);

    /**
     * \brief Display status: number of nodes, bounds, time etc.
     *
     * \param [in] current_uncounted If True, then the number of nodes in the
     * log-message is incremented by one. This may happen when diving: the
     * node being processed is not in the list of active nodes in the tree.
     *
     * \param [out] treeLb is the lower bound of the branch-and-bound tree. 
     * \param [out] wallStartTime is the start time of branch-and-bound.
     * \param [out] threadId is the id of a thread in parallel mode (else 0)
     */
    void showParStatus_(UInt current_uncounted, double treeLb,
                        double wallStartTime, UInt threadId);
  };

  /// Statistics about the branch-and-bound.
  struct ParBabStats 
  {
    /// Constructor. All data is initialized to zero.
    ParBabStats();

    /// Number of nodes processed.
    UInt nodesProc;

    /// Total time used in branch-and-bound.
    double timeUsed;

    /// Time of the last log display.
    double updateTime;
  };


  /// Different options and parameters that control branch-and-bound
  struct ParBabOptions
  {
    /// Default constructor. 
    ParBabOptions();

    /// Constructor created from options in environment. 
    ParBabOptions(EnvPtr env);
    
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

  typedef ParBranchAndBound* ParBranchAndBoundPtr;
}
#endif

