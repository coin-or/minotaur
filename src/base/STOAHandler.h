// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file STOAHandler.h
 * \Briefly declare a derived class of Handler that handles convex nonlinear constraints
 * of a problem for Single Tree Outer Approximation algorithm.
 * \Authors Meenarli Sharma and Prashant Palkar, Indian Institute of
 * Techonology Bombay.
 */

#ifndef MINOTAURSTOAHANDLER_H
#define MINOTAURSTOAHANDLER_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "MILPEngine.h"
#include "Problem.h"
#include "Function.h"
#include "Timer.h"

namespace Minotaur {

struct STOAStats {
  size_t nlpS;      /// Number of nlps solved.
  size_t nlpF;      /// Number of nlps feasible.
  size_t nlpI;      /// Number of nlps infeasible.
  size_t nlpIL;     /// Number of nlps hits engine iterations limit.
  size_t cuts;      /// Number of cuts added to the LP.
};

/**
 * \brief Handler for convex constraints, based on quesada-grossmann
 * algorithm.
 *
 * STOAHandler is a derived class of Handler. It adds cuts generated
 * by solving an NLP whenever an integer (but infeasible) solution of LP relaxation is found.
 */
class STOAHandler : public Handler {

private: 
  /// Pointer to environment.
  EnvPtr env_;

  /// Tolerance for checking integrality (should be obtained from env).
  double intTol_;

  /// Log.
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// Pointer to original problem.
  ProblemPtr minlp_;

    /**
     * \brief Timer for keeping track of time.
     *
     * The user or the environment from which branch-and-bound is called can
     * set up the timer and even start it before sending it to
     * branch-and-bound.
     */
  Timer *timer_;

    /// Vector of constraints.
  ConstraintVector nlCons_;

  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;
  
  /// MILP Engine used to solve the MILP relaxations.
  MILPEnginePtr milpe_;

  /// Modifications done to NLP before solving it.
  //std::stack<Modification *> nlpMods_;
  
  /// Status of the NLP/QP engine.
  EngineStatus nlpStatus_;

  /**
   * The variable corresponding to the objective function. It is a part of
   * all linearizations of the objective function and it appears in the
   * objective.
   */
  VariablePtr objVar_;

  /// Nonlinearity status of objective function. 1 if nonlinear 0 otherwise.
  bool oNl_;
  
  double newUb_;

  /// Pointer to relaxation of the problem.
  RelaxationPtr rel_;

  SolutionPoolPtr solPool_;
 
  /// Value of objective in relaxation solution
  //double relobj_;

  /// Absolute tolerance for constraint feasibility.
  double solAbsTol_;

  /// Relative tolerance for constraint feasibility.
  double solRelTol_;

  /// Absolute tolerance for pruning a node.
  double npATol_;

  /// Relative tolerance for pruning a node.
  double npRTol_;

  /// Callback calls
  UInt numCalls_;

  /// Total wall clock time spent in callbacks
  double cbtime_;

  /// Statistics.
  STOAStats *stats_;


public:
  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  STOAHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe, MILPEnginePtr milpe, SolutionPoolPtr solPool); 

  /// Destroy.
  ~STOAHandler();
   
 /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr,
                       SolutionPoolPtr)
  {return Branches();};

  /// Return number of callbacks till now
  UInt getNumCalls() {return numCalls_;}

  /// Return the time taken in callbacks till now
  double getCbTime() {return cbtime_;}

  /// Set the time taken in callbacks till now
  void setCbTime(double timeval) {cbtime_ = timeval;}

  /// Does nothing.
  void getBranchingCandidates(RelaxationPtr, 
                              const DoubleVector &, ModVector &,
                              BrVarCandSet &, BrCandVector &, bool &) {};

  /// Does nothing.
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                           BranchDirection)
  {return ModificationPtr();};

  // Base class method. 
  std::string getName() const;

  // Base class method. Check if x is feasible. x has to satisfy integrality
  // and also nonlinear constraints.
  bool isFeasible(ConstSolutionPtr, RelaxationPtr, 
                  bool &, double &) {return 0;};

  //bool isFeas(const double* x);

  /// Does nothing.
  SolveStatus presolve(PreModQ *, bool *, Solution **) {return Finished;};

  /// Does nothing.
  bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  /// Does nothing.
  void postsolveGetX(const double *, UInt, DoubleVector *) {};

  // Base class method. calls relax_().
  void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) {};

  // Base class method. calls relax_().
  void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *);

  // Base class method. Does nothing.
  void relaxNodeFull(NodePtr, RelaxationPtr, bool *) {};

  // Base class method. Does nothing.
  void relaxNodeInc(NodePtr, RelaxationPtr, bool *) {};

 
  // Base class method. Find cuts.
  void separate(ConstSolutionPtr, NodePtr, RelaxationPtr, 
                CutManager *, SolutionPoolPtr, ModVector &, 
                ModVector &, bool *, SeparationStatus *) {};
 
  //void solveMILP(double* objfLb, double* objfUb,
                 //SolutionPoolPtr solPool, CutManager* cutMan);
  void solveMILP(double* objfLb, ConstSolutionPtr* sol,
                 SolutionPoolPtr solPool, CutManager* cutMan);
  // Show statistics.
  void writeStats(std::ostream &out) const;

  // Return the relaxation pointer
  RelaxationPtr getRel() {return rel_;}

  // Return true if the fixed NLP was feasible
  //bool fixedNLP(const double *lpx, const double * nlpx);
  bool fixedNLP(const double *lpx, EnginePtr &nlpe, ProblemPtr &minlp);

  // Return the upper bound obtained from an integer solution
  double newUb(std::vector<UInt> *varIdx,
                        std::vector<double>* varCoeff);

  //void cutIntSol(const double *lpx, double objVal);

  /// Iterate over constraints. Returns the 'begin' iterator.
  ConstraintConstIterator consBegin() const { return nlCons_.begin(); }

  /// Iterate over constraints. Returns the 'end' iterator.
  ConstraintConstIterator consEnd() const { return nlCons_.end(); }


  void OACutToCons(const double *lpx, ConstraintPtr con, double* rhs,
                   std::vector<UInt> *varIdx, std::vector<double>* varCoeff,
                   EnginePtr nlpe);

  void OACutToObj(const double *lpx, double* rhs, std::vector<UInt> *varIdx,
                  std::vector<double>* varCoeff, double ub, EnginePtr nlpe);

private:
  /**
   * Find the linearization of nonlinear functions at point x* and add
   * them to the relaxation only (not to the lp engine)
   */
  void addInitLinearX_(const double *x);

  /**
   * Add cuts to cut out an integer point not satisfied by nonlinear
   * constraints or objective.
   */
  //void cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan, 
                  //SolutionPoolPtr s_pool, bool *sol_found, 
                  //SeparationStatus *status);
  /**
   * Fix integer constrained variables to integer values in x. Called
   * before solving NLP.
   */
  void fixInts_(const double *x, ProblemPtr minlp, std::stack<Modification *> &nlpMods);
  
   /**
   * Solve the NLP relaxation of the MINLP and add linearizations about
   * the optimal point. isInf is set to true if the relaxation is found
   * infeasible. Throw an assert if the relaxation is unbounded.
   */
  void initLinear_(bool *isInf);

  /** 
   * When the objective function is nonlinear, we need to replace it with
   * a single variable.
   */
  void linearizeObj_();


  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf, int *error);

  /**
   * Check which nonlinear constraints are violated at the LP solution and
   * add OA cuts. Return number of OA cuts added.
   */
  void cutToCons_(ConstraintPtr con, const double *nlpx, const double *lpx, double* rhs, std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff);
  
  void addCut_(const double *nlpx, const double *lpx, ConstraintPtr con, double* rhs, std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff);

  void objCutAtLpSol_(const double *lpx, double* rhs,
                             std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff, double relobj);

  void consCutAtLpSol_(ConstraintPtr con, const double *lpx, double* rhs,
                             std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff);

  /**
   * Check if objective is violated at the LP solution and
   * add OA cut.
   */
  void cutToObj_(const double *nlpx, const double *lpx,
                 double* rhs, std::vector<UInt> *varIdx,
                            std::vector<double>* varCoeff, double relobj);

  /**
   * Create the initial relaxation. It is called from relaxInitFull and
   * relaxInitInc functions.
   */
  void relax_(bool *is_inf);

  /// Solve the nlp.
  void solveNLP_();

  /// Undo the changes done in fixInts_().
  void unfixInts_(ProblemPtr minlp, std::stack<Modification *> &nlpMods);

  /**
   * Update the upper bound. XXX: Needs proper integration with
   * Minotaur's Handler design. 
   */
  void updateUb_(SolutionPoolPtr s_pool, double *nlp_val, bool *sol_found);

  };

  typedef STOAHandler* STOAHandlerPtr;
}
#endif

