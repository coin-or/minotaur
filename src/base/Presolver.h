// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 
/**
 * \file Presolver.h
 * \brief Declare Presolver class for presolving.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURPRESOLVER_H
#define MINOTAURPRESOLVER_H

#include "Types.h"

namespace Minotaur {

  class   Solution;
  class   PreMod;
  typedef PreMod* PreModPtr;
  typedef Solution* SolutionPtr;
  typedef std::deque<PreModPtr> PreModQ;
  typedef PreModQ::iterator PreModQIter;
  typedef PreModQ::reverse_iterator PreModQRIter;
  typedef PreModQ::const_iterator PreModQConstIter;

   /**
    * A Presolver is used to modify a problem such that it becomes simplified
    * or easier to solve. A Presolver, in its default form, will create a copy
    * of the problem. A user will need to specifically tell presolver not to
    * copy a problem. It should maintain a
    * list of changes made to the original problem.
  
    * The Presolver may also be able to detect infeasibility, optimality,
    * feasibility or unboundedness. A presolver in this way, is very much a
    * solver.
    */
  class Presolver {
  public:
    /// Default constructor.
    Presolver ();

    /// Constructor for a given problem.
    Presolver (ProblemPtr problem, EnvPtr env, HandlerVector handlers);

    /// Destroy.
    virtual ~Presolver();

    /// Default presolve.
    virtual void presolve() {};

    /// get solution if any
    virtual SolutionPtr  getSolution() {return sol_;};

    virtual SolveStatus getStatus();
    /**
     * standardize is called before solving any problem even when
     * presolve is disabled. This method is necessary to standardize the
     * problem:
     * convert maximize to minimize, 
     * add a new variable for the objective function,
     * ...
     */
    virtual void standardize();

    virtual SolveStatus solve();

    /// Search and remove any duplicate rows and columns from the problem.
    virtual void removeDuplicates() {};

    /**
     * Translate a given x into solution of the original problem.
     * The space for newx needs to be allocated.
     */
    virtual void getX(const double *x, DoubleVector *newx);

    /** 
     * Construct a solution for the original problem from that of the
     * presolved problem. 
     */
    SolutionPtr getPostSol(SolutionPtr s);

  protected:

    /// Environment.
    EnvPtr env_;

    /// Tolerance for checking feasibility.
    double eTol_;

    /// Handlers used to presolve the problem.
    HandlerVector handlers_;

    /// A value in [z-intTol_, z+intTol_], z integer, will be treated as z.
    double intTol_;

    /// Log manager.
    LoggerPtr logger_;

    /// For logging
    static const std::string me_;

    /// A queue of presolve-modifications required for post-solve.
    PreModQ mods_;

    /*
     * The problem being presolved. Only one problem may be presolved by one
     * Presolver.
     */
    ProblemPtr problem_;

    /// Pointer to optimal solution, if one found
    SolutionPtr sol_;

    /// Status.
    SolveStatus status_;

    /// Remove objective function, if it is zero or constant.
    void removeEmptyObj_();

    /// convert to minimization problem.
    void minimizify_();

    /**
     * Replace the objective with a linear function. The problem remains
     * equivalent to the one before the function is called.
     */
    void linearizeObjective_();

    /**
     * Some interfaces (like AMPL) and the users may specify binary
     * variables as integer variables. This function converts such variables
     * to binary.
     */
    void ifIntsAreBins_();

    /*
     * Convert constraints of the form g(x) >= c to -g(x) <= -c. Do not
     * change equality or range constraints.
     */
    void standardizeConstraints_();

  };

  typedef Presolver* PresolverPtr;
}
#endif

