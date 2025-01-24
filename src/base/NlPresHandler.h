//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file NlPresHandler.h
 * \brief Declare the NlPresHandler class for try some NLP presolve ideas.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURNLPRESHANDLER_H
#define MINOTAURNLPRESHANDLER_H

#include "Handler.h"
#include "VarBoundMod.h"
#include "QuadraticFunction.h"

namespace Minotaur {

class CGraph;
class CNode;
class PreAuxVars;
typedef CGraph* CGraphPtr;
typedef PreAuxVars* PreAuxVarsPtr;


/// Store statistics of presolving.
struct NlPresStats 
{
  int iters;    /// Number of iterations (main cycle).
  double time;  /// Total time used in initial presolve.
  double timeN; /// Total time used in node presolves.
  int varDel;   /// Number of variables marked for deletion.
  int conDel;   /// Number of constraints marked for deletion.
  int conRel;   /// Number of times constraint bounds relaxed.
  int infBnds;  /// No. of times infeasible implied bounds detected.
  int pRefs;    /// Number of perspective reformulations
  int vBnd;     /// Number of times variable-bounds were tightened.
  int cBnd;     /// Number of times constraint-bounds were tightened.
  int cImp;     /// Number of times coefficient in a constraint was improved.
  int nMods;    /// Number of changes in nodes.
  int qCone;    /// Number of times a quadratic constraint changed to a
                /// conic constraint.
};


/// Options for presolve.
struct NlPresOpts {
  bool doPresolve; /// True if presolve is enabled, false otherwise.
  bool showStats;  /// True if stats are displayed, false otherwise.
  int  maxIters;   /// Maximum number of iterations.
  bool coeffImp;   /// If True, do coefficient improvement.
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
  void relaxInitFull(RelaxationPtr , SolutionPool *, bool *) {};

  // Does nothing.
  void relaxInitInc(RelaxationPtr , SolutionPool *, bool *) {};

  // Does nothing.
  void relaxNodeFull(NodePtr , RelaxationPtr , bool *) {};

  // Does nothing.
  void relaxNodeInc(NodePtr , RelaxationPtr , bool *) {};

  /** 
   * We assume that nonlinear presolve handler is never called for checking
   * feasibility. Always return true.
   */
  bool isFeasible(ConstSolutionPtr, RelaxationPtr, bool &, double &)
  {return true;}

  /**
   * Generate valid cuts using linear constraints.
   */
  void separate(ConstSolutionPtr , NodePtr , RelaxationPtr , CutManager *,
                SolutionPoolPtr , ModVector &, ModVector &, bool *,
                SeparationStatus *) {};

  /// Does nothing.
  void getBranchingCandidates(RelaxationPtr, const DoubleVector &,
                              ModVector &, BrVarCandSet &, BrCandVector &,
                              bool &) {};

  /// Does nothing.
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, 
                           RelaxationPtr, BranchDirection)
  {return ModificationPtr();}; // NULL

  /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr, 
                               SolutionPoolPtr)
  {return Branches();}; // NULL

  // presolve.
  SolveStatus presolve(PreModQ *pre_mods, bool *changed, Solution **sol);

  // Implement Handler::presolveNode().
  bool presolveNode(RelaxationPtr p, NodePtr node, SolutionPoolPtr s_pool,
                    ModVector &p_mods, ModVector &rmods);

  // Write name
  std::string getName() const;

  void simplePresolve(ProblemPtr p, SolutionPoolPtr s_pool,
                      ModVector &t_mods, SolveStatus &status);
  /**
   * \brief Write statistics about presolve. 
   * \param [in] out The output stream to which statistics are printed.
   */
  void writePreStats(std::ostream &out) const;

  // base class method.
  void writeStats(std::ostream &out) const;







  /**
   * Bounds for unvariate case, x>0 for ax^2 +bx >= c 
   */
  void x_bound_with_c(double a,double b,double c, double *l,
                                    double *u, double *l2);


  /**
   * Bounds for unvariate case, x>0, b=[blb,bub], for ax^2 +bx >= c 
   */

  //void x_bound_range_with_c(double a,double blb,double bub, double c, double *l,
                                  //double *u, int infeasibility);


  /**
   * get set of variables having only linear terms in fucntion 
   */

 void linear_var_set(VariableSet lfvars, FunctionPtr f,VariableSet *linear_terms);

  /**
   * Bounds for variables with only linear terms in function 
   */

  void lin_var_bound(VarBoundModVector lfmod, LinearFunctionPtr lf, QuadraticFunctionPtr qf, 
                              VariableSet qfvars,  VariableSet linear_terms, double cl, double cu, int &change);



  /**
   * Bounds for variables with only linear terms in function other than the variable given as input to this function
   * to be used for above function, for getting linear var bound
   * 
   */

  void get_other_linear_var_bounds(VariablePtr v, double *lflb, double *lfub, 
                                            VariableSet linear_tSerms, LinearFunctionPtr lf);



   /**
   * Bounds for quadratic variable terms 
   */

  void quad_var_bound(VarBoundModVector qfmod, LinearFunctionPtr lf, QuadraticFunctionPtr qf, 
                                  VariableSet qfvars,  VariableSet linear_terms, double cl, double cu,
                                  int &change);




   /**
   * Bounds for quadratic variable terms other than the variable given as input to this function  to be used for
   *  above function, for getting quad var bound
   */
  void get_other_quad_var_bounds(VariablePtr v, double *qflb, double *qfub, VariableSet qfvars, 
                                        QuadraticFunctionPtr qf, LinearFunctionPtr lf);

   /**
   * Bounds for bilinear variable terms other than the variable given as input to this function  to be used for
   *  above function, for getting quad var bound
   */
  void bilinear_bounds(VariablePtr v, double *qflb1, double *qfub1, VariableSet qfvars,
                                        QuadraticFunctionPtr qf);



   /**
   * Bounds for a quadratic function : x>0 for ax^2 +bx  
   */
  void quad_bound(VariablePtr v, double a,double b, double *l,
                                     double *u);


  /**
   * Bounds for a quadratic function : x>0, b=[blb,bub]  for ax^2 +bx  
   */                       
  void quad_bound_range(VariablePtr v, double a,double blb, double bub, double *l,
                                     double *u);





private:
  /// Should we try perspective reformulation?
  bool doPersp_;

  /// Should we try perspective reformulation?
  bool doQuadCone_;

  /// Environment.
  EnvPtr env_;

  /// Tolerance for checking feasibility etc.
  double eTol_;

  /// Log manager
  LoggerPtr logger_;
 
  /// Problem that will be presolved.
  ProblemPtr p_;

  NlPresStats stats_;

  /// Tolerance for checking zero.
  double zTol_;

  /// Who am I?
  static const std::string me_;

  void bin2Lin_(ProblemPtr p, PreModQ *mods, bool *changed);
  void bin2LinF_(ProblemPtr p, LinearFunctionPtr lf,
                 UInt nz, const UInt *irow, const UInt *jcol,
                 const double *values, PreAuxVarsPtr mods);

  bool canBin2Lin_(ProblemPtr p, UInt nz, const UInt *irow,
                   const UInt *jcol, const double *values);
  void chkRed_(ProblemPtr p, bool apply_to_prob, bool *changed,
               ModQ *mods, SolveStatus &status);
  void  coeffImpr_(bool *changed);
  void  computeImpBounds_(ConstraintPtr c, VariablePtr z, 
                          double zval, double *lb, double *ub);

  void copyBndsFromRel_(RelaxationPtr rel, ModVector &p_mods);

  /**
   * Check if the nonlinear function is a constant or a linear function
   * If so, simplify the constraint. Constraint may become infeasible or
   * redundant after the modification
   */
  void fixedNl_(ProblemPtr p, bool purge_cons, bool *changed,
                ModQ *, SolveStatus &status);
  /**
   * Fix any binary variables in the objective function based on the
   * incumbent soluion value.
   */
  void fixObjBins_(ProblemPtr p, double ub, bool *changed, ModQ *mods,
                   SolveStatus &status);

  void perspMod_(ConstraintPtr c, VariablePtr z);
  void perspRef_(ProblemPtr p, PreModQ *mods, bool *changed);
  void quadConeRef_(ProblemPtr p, PreModQ *mods, bool *changed);
  void varBndsFromCons_(ProblemPtr p, bool apply_to_prob, bool *changed,
                        ModQ* mods, SolveStatus &status);
};
typedef NlPresHandler* NlPresHandlerPtr;
}
#endif

