// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2013 The MINOTAUR Team.
// 

/**
 * \file QGHandler.h
 * \brief Declare the derived class of Handler that handles convex constraints
 * of a problem by using Quesada-Grossmann algorithm.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURQGHANDLER_H
#define MINOTAURQGHANDLER_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "Problem.h"
#include "Function.h"

namespace Minotaur {

class WarmStart;
typedef boost::shared_ptr<WarmStart> WarmStartPtr;

struct QGStats {
  Size_t nlpS;      /// Number of nlps solved.
  Size_t nlpF;      /// Number of nlps feasible.
  Size_t nlpI;      /// Number of nlps infeasible.
  Size_t cuts;      /// Number of cuts added to the LP.
}; 


/**
 * \brief Handler for convex constraints, based on quesada-grossmann
 * algorithm.
 *
 * QGHandler is a derived class of VariableHandler. It adds cuts generated
 * by solving an NLP whenever an integer (but infeasible) solution is found.
 */
class QGHandler : public Handler {

public:
  /// Empty constructor.
  QGHandler();

  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe); 

  /// Destroy.
  ~QGHandler();
      
  // Base class method. calls relax_().
  void relaxInitFull(RelaxationPtr rel, Bool *is_inf);

  // Base class method. calls relax_().
  void relaxInitInc(RelaxationPtr rel, Bool *is_inf);

  // Base class method. Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  // Base class method. Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  // Base class method. Check if x is feasible. x has to satisfy integrality
  // and also nonlinear constraints.
  Bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  Bool & is_inf);

  // Base class method. Find cuts.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, Bool *sol_found,
                SeparationStatus *status);

  /// Does nothing.
  void getBranchingCandidates(RelaxationPtr, 
                              const std::vector< Double > &, ModVector &,
                              BrCandSet &, Bool &) {};

  /// Does nothing.
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                           BranchDirection)
  {return ModificationPtr();}; // NULL

  /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr,
                       SolutionPoolPtr)
  {return Branches();}; // NULL
      
  // Base class method. 
  std::string getName() const;

  /// Does nothing.
  SolveStatus presolve(PreModQ *, Bool *) {return Finished;};

  /// Does nothing.
  void postsolveGetX(const Double *, UInt, DoubleVector *) {};

  /// Does nothing.
  Bool presolveNode(ProblemPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};
 
  // Show statistics.
  void writeStats(std::ostream &out) const;

private:
  /// Pointer to environment.
  EnvPtr env_;

  /// Pointer to original problem.
  ProblemPtr minlp_;

  /// Pointer to original problem.
  RelaxationPtr rel_;

  /// Log.
  LoggerPtr logger_;

  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;

  /// Status of the NLP/QP engine.
  EngineStatus nlpStatus_;

  /// Warm-start information for solving NLPs.
  WarmStartPtr nlpWs_;

  /**
   * When the objective function is nonlinear, we need to save it, so
   * that we can add linear approximations.
   */
  FunctionPtr objFun_;

  /**
   * The variable corresponding to the objective function. It is a part of
   * all linearizations of the objective function and it appears in the
   * objective.
   */
  VariablePtr objVar_;

  /// Vector of constraints.
  std::vector<ConstraintPtr> nlCons_;

  /// Is the objective function nonlinear?
  Bool oNl_;

  /**
   * Was this handler used to check the feasibility. We generate cuts
   * only if we checked the feasibility and it is false.
   */
  Bool isFeas_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;

  /**
   * For any linearization constraint that we generate, all 
   * coefficients with absolute value less than it are assumed zero.
   */
  const Double linCoeffTol_;

  /// Statistics.
  QGStats *stats_;

  /// Tolerance for accepting a new solution value: absolute threshold.
  const Double solAbsTol_;

  /// Tolerance for accepting a new solution value: relative threshold.
  const Double solRelTol_;

  /// Tolerance for checking constraint violation.
  Double eTol_;

  /// Tolerance for checking linear cut violation.
  Double eLinTol_;
 
  UInt numCuts_;

  /// Tolerance for checking integrality (should be obtained from env).
  Double intTol_;

  /// For log:
  static const std::string me_;

  /**
   * Solve the NLP relaxation of the MINLP and add linearizations about
   * the optimal point. isInf is set to true if the relaxation is found
   * infeasible. Throw an assert if the relaxation is unbounded.
   */
  void initLinear_(Bool *isInf);

  /**
   * Find the linearization of nonlinear functions at point x* and add
   * them to the relaxation only (not to the lp engine)
   */
  void addInitLinearX_(const Double *x);

  /** 
   * When the objective function is nonlinear, we need to replace it with
   * a single variable.
   */
  void linearizeObj_(RelaxationPtr rel);

  /**
   * Add cuts to cut out an integer point not satisfied by nonlinear
   * constraints or objective.
   */
  void cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                  Bool *sol_found, SeparationStatus *status);

  /**
   * Fix integer constrained variables to integer values in x. Called
   * before solving NLP.
   */
  void fixInts_(const Double *x);

  /// Undo the changes done in fixInts_().
  void unfixInts_();

  /**
   * Create the initial relaxation. It is called from relaxInitFull and
   * relaxInitInc functions.
   */
  void relax_(RelaxationPtr rel, Bool *is_inf);

  /// Solve the nlp.
  void solveNLP_();

  /**
   * Make a copy of the bounds on variables. Called before bounds on NLP
   * are changed.
   */
  void copyLPBounds_(std::stack<Modification *> *mods);

  /// Restore bounds.
  void undoLPBounds_(std::stack<Modification *> *mods);

  /**
   * Update the upper bound. XXX: Needs proper integration with
   * Minotaur's Handler design. 
   */
  void updateUb_(SolutionPoolPtr s_pool, Double *nlp_val, 
                 Bool *sol_found);


  /// Add all linearizations at point x that violate inf_x.
  void OAFromPoint_(const Double *x, ConstSolutionPtr sol, 
                    SeparationStatus *status);

  void OAFromPoint_(const Double *x, const Double *inf_x,
                             SeparationStatus *status);

  Int OAFromPointInf_(const Double *x, const Double *inf_x, 
                    SeparationStatus *status);

  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, Double fval, const Double *x, 
                 Double *c, LinearFunctionPtr *lf);

  void readSol_();

  Int ObjId_;
 
  UInt lastNode_;
  Double *lastSol_;

  const Double *rootSol_;  // The solution to the NLP relaxation at root node
  const Double *incSol_;
  const Double *LPrelSol_;

  Int numvars_;

  // Either dir = 0 if Ub < INFINITY and violated or dir = 1 if Lb > -INFINIT and violated
  void cutXLP_(const Double *x_nlp, const Double *x_lp,
                             ConstraintPtr con, SeparationStatus *status, Bool dir);

  void cutXLP_(const Double *x_nlp, const Double *x_lp,
		Double *x_alpha, ConstraintPtr con, Bool dir);

  void cutXLP_(const Double *x_nlp, const Double *x_lp,
                          Double *x_alpha, FunctionPtr fn, Double ub, Double lb, Bool dir);

  void cutXLPObj_(const Double *x_nlp, const Double *x_inf,
                  Double *x_alpha, FunctionPtr funobj);

  Double relobj_;  
  Bool objCutOff;
};

  typedef boost::shared_ptr <QGHandler> QGHandlerPtr;
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
