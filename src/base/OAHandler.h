// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file OAHandler.h
 * \Briefly declare a derived class of Handler that handles convex nonlinear constraints
 * of a problem for the Outer Approximation algorithm.
 * \Authors Meenarli Sharma and Prashant Palkar, Indian Institute of
 * Techonology Bombay.
 */

#ifndef MINOTAUROAHANDLER_H
#define MINOTAUROAHANDLER_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "Problem.h"
#include "Function.h"
#include "Timer.h"

namespace Minotaur {

struct OAStats {
  size_t cuts;      /// Number of cuts added to the MILP.
  size_t nlpS;      /// Number of nlps solved.
  size_t nlpF;      /// Number of nlps feasible.
  size_t nlpI;      /// Number of nlps infeasible.
  size_t nlpIL;     /// Number of nlps hits engine iterations limit.
  size_t milpS;      /// Number of milps solved.
  size_t milpIL;     /// Number of milps hits engine iterations limit.
}; 


/**
 * \brief Handler for convex constraints, based on quesada-grossmann
 * algorithm.
 *
 * OAHandler is a derived class of Handler. It adds cuts generated
 * by solving an NLP whenever an integer (but infeasible) solution of LP relaxation is found.
 */
class OAHandler : public Handler {

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
  std::vector<ConstraintPtr> nlCons_;

  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;
  
  /// MILP Engine used to solve the MILP relaxations.
  MILPEnginePtr milpe_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;
  
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

  /// Pointer to relaxation of the problem.
  RelaxationPtr rel_;
 
  /// Value of objective in relaxation solution
  double relobj_; 

  /// Absolute tolerance for constraint feasibility.
  double solAbsTol_;

  /// Relative tolerance for constraint feasibility.
  double solRelTol_;

  /// Absolute tolerance for pruning a node.
  double objATol_;

  /// Relative tolerance for pruning a node.
  double objRTol_;

  /// Statistics.
  OAStats *stats_;


public:
  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  OAHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe, MILPEnginePtr milpe); 

  /// Destroy.
  ~OAHandler();
   
 /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr,
                       SolutionPoolPtr)
  {return Branches();};

  /// Does nothing.
  void getBranchingCandidates(RelaxationPtr, 
                              const DoubleVector &, ModVector &,
                              BrVarCandSet &, BrCandVector &, bool &) {};

  std::vector<ConstraintPtr> getNonlinCons() {return nlCons_;}

  VariablePtr getObjVar() {return objVar_;}

  bool getObjType() { return oNl_;}

  /// Returns the MILP engine.
  MILPEnginePtr getMILPEngine() {return milpe_;};

  /// Does nothing.
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                           BranchDirection)
  {return ModificationPtr();};

       
  // Base class method. 
  std::string getName() const;

  // Base class method. Check if x is feasible. x has to satisfy integrality
  // and also nonlinear constraints.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool & should_prune, double &inf_meas);

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
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, ModVector &, 
                ModVector &, bool *sol_found, SeparationStatus *status);
 
  //void solveMILP(double* objfLb, double* objfUb,
                 //SolutionPoolPtr solPool, CutManager* cutMan);
  void solveMILP(double* objfLb, ConstSolutionPtr* sol,
                 SolutionPoolPtr solPool, CutManager* cutMan,
                 SolveStatus &status);
  // Show statistics.
  void writeStats(std::ostream &out) const;

  /// Set the nonlinear constraints
  void setNonlinCons(std::vector<ConstraintPtr> nlCons) {nlCons_ = nlCons;}

  void setObjVar(VariablePtr objvar) {objVar_ = objvar;}

  void setObjType(bool oNl) {oNl_ = oNl;}

  void setRelaxation(RelaxationPtr rel) {rel_ = rel;}

  /// Set the relaxation
  void setRelObj(double val) {relobj_ = val;}

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
  void cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan, 
                  SolutionPoolPtr s_pool, bool *sol_found, 
                  SeparationStatus *status);

  /**
   * Fix integer constrained variables to integer values in x. Called
   * before solving NLP.
   */
  void fixInts_(const double *x);
  
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
  void cutToCons_(const double *nlpx, const double *lpx, CutManager *,
                    SeparationStatus *status);

  void cutToConsInf_(const double *nlpx, const double *lpx, CutManager *,
                    SeparationStatus *status);

  /// Add OA cut to a violated constraint.   
  void addCut_(const double *nlpx, const double *lpx, ConstraintPtr con, 
               CutManager *cutman, SeparationStatus *status);

  void addCutInf_(ConstraintPtr con, const double *nlpx, const double *lpx, 
               CutManager *cutman, SeparationStatus *status);

  /// OA cut at the LP solution
  void cutsAtLpSol_(const double *lpx, CutManager *cutman,
                    SeparationStatus *status);


  /**
   * Check if objective is violated at the LP solution and
   * add OA cut.
   */
  void cutToObj_(const double *nlpx, const double *lpx, CutManager *,
                   SeparationStatus *status);

   /**
   * Create the initial relaxation. It is called from relaxInitFull and
   * relaxInitInc functions.
   */
  void relax_(bool *is_inf);

  /// Solve the nlp.
  void solveNLP_();

  /// Undo the changes done in fixInts_().
  void unfixInts_();

  /**
   * Update the upper bound. XXX: Needs proper integration with
   * Minotaur's Handler design. 
   */
  void updateUb_(SolutionPoolPtr s_pool, double nlp_val, bool *sol_found);

  };

  typedef OAHandler* OAHandlerPtr;
}
#endif

