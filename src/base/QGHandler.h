// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/**
 * \file QGHandler.h
 * \Briefly declare a derived class of Handler that handles convex nonlinear constraints
 * of a problem by using Quesada-Grossmann algorithm.
 * \Authors Ashutosh Mahajan and Meenarli Sharma, Indian Institute of
 * Techonology Bombay.
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
  size_t nlpS;      /// Number of nlps solved.
  size_t nlpF;      /// Number of nlps feasible.
  size_t nlpI;      /// Number of nlps infeasible.
  size_t cuts;      /// Number of cuts added to the LP.
}; 


/**
 * \brief Handler for convex constraints, based on quesada-grossmann
 * algorithm.
 *
 * QGHandler is a derived class of Handler. It adds cuts generated
 * by solving an NLP whenever an integer (but infeasible) solution of LP relaxation is found.
 */
class QGHandler : public Handler {

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

  /// Vector of constraints.
  std::vector<ConstraintPtr> nlCons_;

  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;
  
  /// Modifications done to perspective amenable constraints.
  std::stack<Modification *> prMods_;

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

  /// Is the objective function nonlinear?
  bool oNl_;

  /// Pointer to original problem.
  RelaxationPtr rel_;
 
  double relobj_; 

  /// Tolerance for accepting a new solution value: absolute threshold.
  double solAbsTol_;

  /// Tolerance for accepting a new solution value: relative threshold.
  double solRelTol_;

  /// Statistics.
  QGStats *stats_;

  /// Vector of perspective constraint pointers
  std::vector<ConstConstraintPtr> cpr_;
  
  /* Vector of pointers to binary variables associated with constraints
   * amenable to PR.
   */
  std::vector<ConstVariablePtr> bpr_;
  
  /// Vector of struture types of constraints amenable to PR.
  std::vector<char> spr_;
  
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
  
  QGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe,
            std::vector<ConstConstraintPtr> conpr, 
            std::vector<ConstVariablePtr> bpr, std::vector<char> spr);
  
  /// Destroy.
  ~QGHandler();
   
 /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr,
                       SolutionPoolPtr)
  {return Branches();};

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
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool & should_prune, double &inf_meas);

  /// Does nothing.
  SolveStatus presolve(PreModQ *, bool *) {return Finished;};

  /// Does nothing.
  bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  /// Does nothing.
  void postsolveGetX(const double *, UInt, DoubleVector *) {};

  // Base class method. calls relax_().
  void relaxInitFull(RelaxationPtr rel, bool *is_inf);

  // Base class method. calls relax_().
  void relaxInitInc(RelaxationPtr rel, bool *is_inf);

  // Base class method. Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // Base class method. Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf);

 
  // Base class method. Find cuts.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, bool *sol_found,
                SeparationStatus *status);
 
  // Show statistics.
  void writeStats(std::ostream &out) const;

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
  void cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                  bool *sol_found, SeparationStatus *status);

  /**
   * Fix integer constrained variables to integer values in x. Called
   * before solving NLP.
   */
  void fixInts_(const double *x);
  
   /**
   * Modify, before solving NLP, constraints amenable to perspective 
   * reformulation if the associated binary variable takes zero value.
   */ 
  void fixCons_(ConstConstraintPtr c);

 /**
   * Solve the NLP relaxation of the MINLP and add linearizations about
   * the optimal point. isInf is set to true if the relaxation is found
   * infeasible. Throw an assert if the relaxation is unbounded.
   */
  void initLinear_(bool *isInf);

  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf);

  /** 
   * When the objective function is nonlinear, we need to replace it with
   * a single variable.
   */
  void linearizeObj_();
 

  /// Add all linearizations at point x that violate inf_x.
  int OAFromPoint_(const double *x, const double *inf_x,
                             SeparationStatus *status);

  int OAFromPointInf_(const double *x, const double *inf_x, 
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

  /// Undo the changes done in fixCons_().
  void unfixCons_();

  /**
   * Update the upper bound. XXX: Needs proper integration with
   * Minotaur's Handler design. 
   */
  void updateUb_(SolutionPoolPtr s_pool, double *nlp_val, bool *sol_found);

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
