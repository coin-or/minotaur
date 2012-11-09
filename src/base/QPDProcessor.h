// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2012 The MINOTAUR Team.
// 

/**
 * \file QPDProcessor.h
 * \brief Define the derived class of QPDProcessor that solves QPs and NLP
 * relaxations.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURQPDPROCESSOR_H
#define MINOTAURQPDPROCESSOR_H

#include "NodeProcessor.h"

namespace Minotaur {

  class Engine;
  class NonlinearFunction;
  class Problem;
  class Solution;
  typedef boost::shared_ptr<Engine> EnginePtr;
  typedef boost::shared_ptr<NonlinearFunction> NonlinearFunctionPtr;
  typedef boost::shared_ptr<Problem> ProblemPtr;
  typedef boost::shared_ptr<Solution> SolutionPtr;
  typedef boost::shared_ptr<const Solution> ConstSolutionPtr;

  struct QPDStats {
    UInt bra;    /// Number of times relaxation became infeasible
    UInt cuts;   /// Number of cuts added
    UInt inf;    /// Number of times relaxation became infeasible
    UInt nlp;    /// Number of NLPs solved
    UInt nlpI;   /// Number of NLP-solves infeasible
    UInt nlpU;   /// Number of NLP-solves feasible
    UInt opt;    /// Number of times relaxation gave optimal feasible solution
    UInt prob;   /// Number of times problem ocurred in solving
    UInt proc;   /// Number of nodes processed
    UInt sep;    /// Number of times qp-point separated
    UInt ub;     /// Number of nodes pruned because of bound
  };

  /**
   * QPDProcessor is a derived class of NodeProcessor. It is meant to solve QPs
   * at each node. Sometimes it also solves NLPs.
   */
  class QPDProcessor : public NodeProcessor {

    public:
      /// Default constructor
      QPDProcessor();

      /// Create the processor using options from an environment.
      QPDProcessor(EnvPtr env, ProblemPtr p, EnginePtr e, EnginePtr qe,
                   HandlerVector &handlers);

      /// Destroy
      ~QPDProcessor();
      
      // Add a heuristic.
      void addHeur(HeurPtr h);

      // True if a new solution was found while processing this node.
      Bool foundNewSolution();

      // Find branches that will be used to branch at this node.
      Branches getBranches();

      // Get warm-start information.
      WarmStartPtr getWarmStart();

      // Implement NodeProcessor::process().
      void process(NodePtr node, RelaxationPtr rel, 
                   SolutionPoolPtr s_pool);

      // Process root node.
      void processRootNode(NodePtr node, RelaxationPtr rel, 
                           SolutionPoolPtr s_pool);

      // write statistics. Base class method.
      void writeStats(std::ostream &out) const; 

      // write statistics to logger. Base class method.
      void writeStats() const {}; 

    private:
      /// Branches found by this processor for this node.
      Branches branches_;

      /// Engine used to process the NLP relaxation.
      EnginePtr e_;

      /// Pointer to environment.
      EnvPtr env_;

      VariablePtr eta_;

      VariablePtr etaL_;

      /// Vector of all handlers.
      HandlerVector handlers_;

      /// Log
      Logger *logger_;

      /* 
       * If Filter-SQP is used, the duals are negative of what QP from Hessian
       * evaluation expects. If this flag is on, the duals are negated before
       * creating QP.
       */
      Bool negDuals_;

      /// Name. For logging.
      static const std::string me_;

      /// Vector of nonlinear constraints.
      std::vector<ConstConstraintPtr> nlCons_;

      /// No. of solutions found while processing this node.
      UInt numSolutions_;

      /// Pointer to original problem.
      ProblemPtr p_;

      /// Relaxation that is processed by this processor.
      RelaxationPtr qp_;

      /// Engine used to process the QP. 
      EnginePtr qpe_;

      const Double solAbsTol_;

      const Double solRelTol_;

      /**
       * If QP-diving is used with Filter-SQP as nlp-engine, then we need to
       * resolve QP after each call to nlp solve. Otherwise, bqpd gets in
       * trouble.
       */
      Bool solveQPafNLP_;

      /// Statistics
      QPDStats stats_;

      /// To impose upper bound.
      ConstraintPtr ubCon_;

      /// Warm-start information for start processing the children.
      WarmStartPtr ws_;

      Bool boundTooFar_(ConstSolutionPtr sol, NodePtr node,
                        Double best) ;

      void getLin_(FunctionPtr f, const Double *x, UInt n,
                   VariableConstIterator vbeg, VariableConstIterator vend, 
                   LinearFunctionPtr &lf, Double &val);

      void getObjLin_(NonlinearFunctionPtr nlf, const Double *x,
                      UInt n, VariableConstIterator vbeg,
                      VariableConstIterator vend, 
                      LinearFunctionPtr &lf, Double &val);

      void fixInts_(const Double *x, std::stack<Modification *> *nlp_mods);

      void chkObjVio_(Double vio, Double etaval, Bool &large_vio);

      void chkVio_(NodePtr node, Double *vio, Double &tvio, Double &maxvio,
                   Bool &large_vio);

      Bool isHFeasible_(ConstSolutionPtr sol, Bool &should_prune);
      Bool isHFeasible2_(ConstSolutionPtr sol, Bool &ishigh, Bool &should_prune);
      void chkObjVio2_(const Double *qpx, NodePtr node, Double best, Bool &hiobjd, Bool &hietavio);
      Bool isNLPFeasible2_(const Double *qpx, Double *vio, NodePtr node, Bool &hicvio);
      Bool isLargeCVio_(ConstConstraintPtr c, Double vio, UInt depth);

      //Bool isQPFeasible_(NodePtr node, ConstSolutionPtr sol, 
      //                  SolutionPoolPtr s_pool, Bool &should_prune);

      Bool isNLPFeasible_(ConstSolutionPtr sol, Double *vio);

      SolutionPtr nlpSol2Qp_(ConstSolutionPtr sol);

      void OAFromPoint_(const Double *x, ConstSolutionPtr sol,
                        SeparationStatus *status); 
      
      Bool presolveNode_(NodePtr node, SolutionPoolPtr s_pool);

      void processNLP_(NodePtr node, ConstSolutionPtr &sol,
                       ConstSolutionPtr qpsol,
                       SolutionPoolPtr s_pool, Bool &should_prune);

      void processQP_(UInt iter, NodePtr node, ConstSolutionPtr &sol,
                      SolutionPoolPtr s_pool, Bool &should_prune,
                      Bool &should_resolve);

      void processQP2_(UInt iter, NodePtr node, ConstSolutionPtr &sol,
                       SolutionPoolPtr s_pool, Bool &should_prune,
                       Bool &should_resolve);

      void saveSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool,
                    NodePtr node);
      void saveQPSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool,
                      NodePtr node);

      void separate_(Bool is_nec, ConstSolutionPtr sol, const Double *vio,
                     ConstSolutionPtr nlp_sol, NodePtr node,
                     SolutionPoolPtr s_pool, SeparationStatus *status);

      void separateB_(ConstSolutionPtr sol, ConstSolutionPtr nlp_sol,
                      Double *vio, NodePtr node, SolutionPoolPtr s_pool,
                      SeparationStatus *status);

      void separateC_(ConstSolutionPtr sol, ConstSolutionPtr nlp_sol,
                      Double *vio, NodePtr node, SolutionPoolPtr s_pool,
                      SeparationStatus *status);

      void separateO_(ConstSolutionPtr sol, ConstSolutionPtr nlp_sol,
                      Double *vio, NodePtr node, SolutionPoolPtr s_pool,
                      SeparationStatus *status);

      void separateECP_(ConstSolutionPtr sol, const Double *vio,
                        NodePtr node, SolutionPoolPtr ,
                        SeparationStatus *status);

      void separateObj_(ConstSolutionPtr sol, ConstSolutionPtr nlp_sol,
                        Double vio, SeparationStatus *status);

      void setupQP_(ConstSolutionPtr sol);

      /**
       * Check if a node can be pruned either because the relaxation is
       * infeasible or because the cost is too high.
       */
      Bool shouldPrune_(NodePtr node, EngineStatus nlp_status, 
                        ConstSolutionPtr sol, SolutionPoolPtr s_pool);

      /**
       * Check if a node can be pruned after solving QP.
       */
      Bool shouldPruneQP_(NodePtr node, EngineStatus nlp_status, 
                          ConstSolutionPtr sol, SolutionPoolPtr s_pool);

      Bool shouldSep_(Bool is_nec, Double vio, ConstConstraintPtr c);
      Bool shouldSepObj_(Bool is_nec, Double vio, Double etaval);

      /// Solve original nlp.
      void solveNLP_(ConstSolutionPtr &sol, EngineStatus &nlp_status);

      void solveQP_(ConstSolutionPtr &sol, EngineStatus &qp_status);

      SolutionPtr translateSol_(ConstSolutionPtr sol);

      void unfixInts_(std::stack<Modification *> *nlp_mods);

      void updateObjCons_(ConstSolutionPtr sol);

      void updateUb_(SolutionPoolPtr s_pool, Double *nlpval);
  };

  typedef boost::shared_ptr <QPDProcessor> QPDProcessorPtr;

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
