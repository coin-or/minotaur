//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file NlPresHandler.h
 * \brief Declare the NlPresHandler class for try some NLP presolve ideas.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURNLPRESHANDLER_H
#define MINOTAURNLPRESHANDLER_H

#include "Handler.h"

namespace Minotaur {

class CGraph;
class CNode;
class PreAuxVars;
typedef boost::shared_ptr<CGraph> CGraphPtr;
typedef boost::shared_ptr<PreAuxVars> PreAuxVarsPtr;


/// Store statistics of presolving.
struct NlPresStats 
{
  Int iters;   /// Number of iterations (main cycle).
  Double time; /// Total time used in initial presolve.
  Int varDel;  /// Number of variables marked for deletion.
  Int conDel;  /// Number of constraints marked for deletion.
  Int vBnd;    /// Number of times variable-bounds were tightened.
  Int cBnd;    /// Number of times constraint-bounds were tightened.
  Int cImp;    /// Number of times coefficient in a constraint was improved.
  Int nMods;   /// Number of changes in nodes.
};


/// Options for presolve.
struct NlPresOpts {
  Bool doPresolve; /// True if presolve is enabled, false otherwise.
  Bool showStats;  /// True if stats are displayed, false otherwise.
  Int  maxIters;   /// Maximum number of iterations.
  Bool coeffImp;   /// If True, do coefficient improvement.
}; 


/**
 * A NlPresHandler presolves nonlinear constraints. Experimental.
 */
class NlPresHandler : public Handler {
public:

  /// Default constructor.
  NlPresHandler();

  /// Constructor.
  NlPresHandler(EnvPtr env, ProblemPtr p);

  /// Destroy
  ~NlPresHandler();

  // Does nothing.
  void relaxInitFull(RelaxationPtr , Bool *) {};

  // Does nothing.
  void relaxInitInc(RelaxationPtr , Bool *) {};

  // Does nothing.
  void relaxNodeFull(NodePtr , RelaxationPtr , Bool *) {};

  // Does nothing.
  void relaxNodeInc(NodePtr , RelaxationPtr , Bool *) {};

  /** 
   * We assume that nonlinear presolve handler is never called for checking
   * feasibility. Always return true.
   */
  Bool isFeasible(ConstSolutionPtr, RelaxationPtr, Bool &)
  {return true;}

  /**
   * Generate valid cuts using linear constraints.
   */
  void separate(ConstSolutionPtr , NodePtr , RelaxationPtr , CutManager *,
                SolutionPoolPtr , Bool *, SeparationStatus *) {};

  /// Does nothing.
  void getBranchingCandidates(RelaxationPtr , 
                                      const DoubleVector &, ModVector &, 
                                      BrCandSet &, Bool &) {};

  /// Does nothing.
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, 
                           RelaxationPtr, BranchDirection)
  {return ModificationPtr();}; // NULL

  /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr, 
                               SolutionPoolPtr)
  {return Branches();}; // NULL

  // presolve.
  SolveStatus presolve(PreModQ *pre_mods, Bool *changed);

  // Implement Handler::presolveNode().
  Bool presolveNode(ProblemPtr p, NodePtr node, SolutionPoolPtr s_pool,
                    ModVector &n_mods, ModVector &t_mods);

  // Write name
  std::string getName() const;

  /**
   * \brief Write statistics about presolve. 
   * \param [in] out The output stream to which statistics are printed.
   */
  void writePreStats(std::ostream &out) const;

  // base class method.
  void writeStats(std::ostream &out) const;
private:
  /// Environment.
  EnvPtr env_;

  /// Tolerance for checking feasibility etc.
  Double eTol_;

  /// Log manager
  LoggerPtr logger_;
 
  /// Problem that will be presolved.
  ProblemPtr p_;

  NlPresStats stats_;

  /// Who am I?
  static const std::string me_;

  void bin2Lin_(ProblemPtr p, PreModQ *mods, Bool *changed);
  void bin2LinF_(ProblemPtr p, LinearFunctionPtr lf,
                 UInt nz, const UInt *irow, const UInt *jcol,
                 const Double *values, PreAuxVarsPtr mods);

  Bool canBin2Lin_(ProblemPtr p, UInt nz, const UInt *irow,
                   const UInt *jcol, const Double *values);
  void  chkRed_(Bool *changed);
  void  coeffImpr_(Bool *changed);
  void  computeImpBounds_(ConstraintPtr c, VariablePtr z, 
                          Double zval, Double *lb, Double *ub);
  SolveStatus varBndsFromCons_(Bool *changed);
};
typedef boost::shared_ptr<NlPresHandler> NlPresHandlerPtr;
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
