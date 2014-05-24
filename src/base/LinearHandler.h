//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file LinearHandler.h
 * \brief Declare the LinearHandler class for handling integer and continuous
 * variables.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURLINEARHANDLER_H
#define MINOTAURLINEARHANDLER_H

#include "Handler.h"

namespace Minotaur {

class LinearFunction;
typedef boost::shared_ptr<LinearFunction> LinearFunctionPtr;

/// Store statistics of presolving.
struct LinPresolveStats 
{
  Int iters;   /// Number of iterations (main cycle).
  Double time; /// Total time used in initial presolve.
  Double timeN;/// Total time used in presolveNode.
  Int varDel;  /// Number of variables marked for deletion.
  Int conDel;  /// Number of constraints marked for deletion.
  Int var2Bin; /// Number of variables converted to binary.
  Int var2Int; /// Number of variables converted to integers.
  Int vBnd;    /// Number of times variable-bounds were tightened.
  Int cBnd;    /// Number of times constraint-bounds were tightened.
  Int cImp;    /// Number of times coefficient in a constraint was improved.
  Int bImpl;   /// Number of times a binary variable was converted to 
  /// implied binary.
  Int nMods;   /// Number of changes made in all nodes.
};

/// Options for presolve.
struct LinPresolveOpts {
  Bool doPresolve; /// True if presolve is enabled, false otherwise.

  Bool showStats;  /// True if stats are displayed, false otherwise.

  Int  maxIters;   /// Maximum number of iterations.

  Bool purgeVars;  /// If True, purge fixed variables.

  Bool purgeCons;  /// If True, purge redundant constraints.

  Bool dualFix;    /// If True, do dual cost fixing.

  Bool coeffImp;   /// If True, do coefficient improvement.
}; 


/**
 * An LinearHandler handles variables of a problem. It only checks bounds 
 * and integrality of the variables.
 */
class LinearHandler : public Handler {
public:

  /// Default constructor.
  LinearHandler();

  /// Constructor.
  LinearHandler(EnvPtr env, ProblemPtr problem);

  /// Destroy
  ~LinearHandler();

  // Does nothing.
  void relaxInitFull(RelaxationPtr rel, Bool *is_inf) ;

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, Bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, Bool *is_inf) ;

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  /** 
   * We assume that linear constraints and bound constraints are always
   * satisfied. Always return true.
   */
  Bool isFeasible(ConstSolutionPtr, RelaxationPtr, Bool &)
  {return true;}

  Bool isNeeded() { return true; }

  /**
   * Generate valid cuts using linear constraints.
   */
  void separate(ConstSolutionPtr sol, NodePtr node, 
                RelaxationPtr rel, CutManager *cutman, SolutionPoolPtr s_pool, 
                Bool *sol_found, SeparationStatus *status);

  /// Does nothing.
  virtual void getBranchingCandidates(RelaxationPtr , 
                                      const DoubleVector &, ModVector &, 
                                      BrCandSet &, Bool &) {};

  /// Does nothing.
  virtual ModificationPtr getBrMod(BrCandPtr, DoubleVector &, 
                                   RelaxationPtr, BranchDirection)
  {return ModificationPtr();}; // NULL

  /// Does nothing.
  virtual Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr, 
                               SolutionPoolPtr)
  {return Branches();}; // NULL

  // presolve.
  virtual SolveStatus presolve(PreModQ *pre_mods, Bool *changed);

  // Implement Handler::presolveNode().
  virtual Bool presolveNode(ProblemPtr p, NodePtr node,
                            SolutionPoolPtr s_pool, ModVector &n_mods,
                            ModVector &t_mods);
      

  // Write name
  virtual std::string getName() const;

  /// Return a constant pointer to the presolve options.
  const LinPresolveOpts* getOpts() const;

  /// If true show statistics.
  void setPreOptShowStats(Bool val) {pOpts_->showStats = val;}; 

  /// Maximum number of iterations.
  void setPreOptMaxIters(Int val) {pOpts_->maxIters = val;}; 

  /// If True, purge fixed variables.
  void setPreOptPurgeVars(Bool val) {pOpts_->purgeVars = val;}; 

  /// If True, purge redundant constraints.
  void setPreOptPurgeCons(Bool val) {pOpts_->purgeCons = val;}; 

  void setPreOptDualFix(Bool val) {pOpts_->dualFix = val;}; 

  void setPreOptCoeffImp(Bool val) {pOpts_->coeffImp = val;}; 

  void simplePresolve(ProblemPtr p, ModVector &t_mods, SolveStatus &status);
  /// Write the presolve statistics.
  void writePreStats(std::ostream &out) const;

  // Write statistics.
  void writeStats(std::ostream &out) const;

protected:
  /// Environment.
  EnvPtr env_;

  /// The problem for which the handler was created.
  ProblemPtr problem_;

  /// Log
  LoggerPtr logger_;

  /// Tolerance for checking integrality.
  /**
   * If |round(x) - x| < intTol_, then it is considered to be integer
   * valued.
   */
  const Double intTol_;

  /// Tolerance.
  const Double eTol_;

  /// Infinity. Bounds beyond this number are treated as infinity.
  const Double infty_;

  /// Statistics of presolve.
  LinPresolveStats *pStats_;

  /// Options for presolve.
  LinPresolveOpts *pOpts_;

  /**
   * Linear variables: variables that do not appear in nonlinear
   * functions, both in objective and constraints.
   */
  VarQueue linVars_;


  /// For log.
  static const std::string me_;

  void chkIntToBin_(VariablePtr v);

  void chkSing_(Bool *changed);
  void coeffImp_(Bool *changed);
  void computeImpBounds_(ConstraintPtr c, VariablePtr z, Double zval,
                         Double *lb, Double *ub);

  void delFixedVars_(Bool *changed);

  void dualFix_(Bool *changed);
  void dupRows_(Bool *changed);

  /// check if lb <= ub for all variables and constraints.
  SolveStatus checkBounds_(ProblemPtr p);

  void findLinVars_();

  void findAllBinCons_();
  void fixToCont_();

  void getLfBnds_(LinearFunctionPtr lf, Double *lo, Double *up);
  void getSingLfBnds_(LinearFunctionPtr lf, Double *lo, Double *up);

  SolveStatus linBndTighten_(ProblemPtr p, Bool apply_to_prob, 
                      ConstraintPtr c_ptr, Bool *changed, ModQ *mods, UInt *nintmods);

  void purgeVars_(PreModQ *pre_mods);

  /**
   * \brief Common routine for building relaxation by copying all the linear
   * constraints and variable-bounds from a given problem.
   * 
   * \param[in] p The problem whose relaxation we want to create.
   * \param[in/out] rel The relaxation in which we want to add new variables
   * and constraints.
   * \param [out] is_inf True if problem p is found to be infeasible, false
   * otherwise.
   */
  void relax_(ProblemPtr p, RelaxationPtr rel, Bool *is_inf);

  void substVars_(Bool *changed, PreModQ *pre_mods);

  /// Round the bounds
  void tightenInts_(ProblemPtr p, Bool apply_to_prob, Bool *changed, 
                    ModQ *mods);

  Bool treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, Double mult,
                     Bool *changed);

  void updateLfBoundsFromLb_(ProblemPtr p, Bool apply_to_prob, 
                             LinearFunctionPtr lf, Double lb, Double uu,
                             Bool is_sing, Bool *changed, ModQ *mods,
                             UInt *nintmods);

  void updateLfBoundsFromUb_(ProblemPtr p, Bool apply_to_prob, 
                             LinearFunctionPtr lf, Double ub, Double ll,
                             Bool is_sing, Bool *changed, ModQ *mods,
                             UInt *nintmods);

  SolveStatus varBndsFromCons_(ProblemPtr p, Bool apply_to_prob, Bool *changed, 
                               ModQ *mods, UInt *nintmods);
};
typedef boost::shared_ptr<LinearHandler> LinearHandlerPtr;
typedef boost::shared_ptr<const LinearHandler> ConstLinearHandlerPtr;
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
