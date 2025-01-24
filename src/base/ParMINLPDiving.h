//
//     Minotaur -- It's only 1/2 bull  
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//
 
/**
 * \file ParMINLPDiving.h
 * \brief Define the class ParMINLPDiving derived from base class Heuristics.
 * This is a parallel extension of MINLPDiving heuristic in MINOTAUR.
 * \author Prashant Palkar, IIT Bombay
 *
 * Declares the class ParMINLPDiving.
 */
 
#ifndef MINOTAURPARMINLPDIVING_H
#define MINOTAURPARMINLPDIVING_H

#include "Heuristic.h"
#include <vector>
#include <stack>
#include <sys/time.h>


namespace Minotaur {
  class LinearHandler;
  class Solution;
  class Timer;
  class VarBoundMod;
  typedef const Solution* ConstSolutionPtr;
  typedef VarBoundMod* VarBoundModPtr;

  /// Direction of rounding
  typedef enum {
    Floor,        /// Round to the smallest integer
    Ceil,         /// Round to the largest integer
    Nearest,      /// Round to the nearest integer
    Farthest      /// Round to the farthest integer
  } Direction;

  /// Order of rounding: least fractional or most fractional
  typedef enum {
    Least,     /// Select the variable with Least fractional part
    Most       /// Select the variable with Most fractional part
  } Order;

  /// Type of score evaluation for fractional variable
  typedef enum {
    Fractional,     /// Score of variable is the fractional part`
    VectorLength,   /// Score of variable is the Vector Length
    LexBound,       /// Score of variable is the index of the variable
    ReducedCost     /// Score of variable is the reduced cost
  } Scoretype;

  /**
   * \brief A statistic struct for ParMINLP Diving heuristic
   */
  struct DivingheurStats {
    UInt numNLPs[4];       /// NLPs solved in each selection method.
    UInt numInfeas[4];     /// Infeasible NLPs solved. 
    UInt errors[4];        /// NLPs that encountered errors.
    UInt numSol[4];        /// Solutions obtained.
    double time[4];        /// Time for each selection method.
    UInt iterations[4];    /// Iterations.
    UInt totalProbs;       /// Problems solved.
    UInt totalSol;         /// Solutions found.
    UInt numLocal;         /// Local optimal solutions obtained.
    double best_obj_value; /// Best objective value of feasible solution.
    double totalTime;      /// Total time taken in Diving heuristics.
  };

  /**
   * \brief Parallel Diving heuristic for MINLPs.
   *
   * A Diving Heuristic used to find solutions for Mixed Integer NLPs 
   * by solving the Relaxed NLP or LP using an appropriate engine. The engine
   * is called once initially to generate a solution  which is rounded and
   * used for diving. 
   */

  class ParMINLPDiving : public Heuristic {
  public:

    /// default constructor
    ParMINLPDiving(EnvPtr env, ProblemPtr p, EnginePtr e);

    /// default destructor
    ~ParMINLPDiving();

    /// call to heuristic
    void solve(NodePtr node, RelaxationPtr rel, SolutionPoolPtr s_pool); 

    /// writing the statistics to the logger
    void writeStats(std::ostream &out) const;

    /// writing the statistics to the logger
    void writeParStats(std::ostream &out, DivingheurStats *stats,
                       double wallTime) const;

    /// Return a string that describes the rounding direction in simple words.
    virtual std::string getDirectionString(UInt i);

    /// Return a string that describes the scoring rule in simple words.
    virtual std::string getScoreString(UInt i);

    /// Return a string that describes the rounding order in simple words.
    virtual std::string getOrderString(UInt i);

    /// Set the alternate (typically NLP) engine pointer
    void setAltEngine(EnginePtr nlpe) { nlpe_ = nlpe; }

    /// Set the original problem pointer
    void setOrigProb(ProblemPtr minlp) { minlp_ = minlp; }

    /// Solve NLP at LP integer feasible solution
    void solveNLP(ConstSolutionPtr sol, bool *solFound,
                             ProblemPtr minlp, EnginePtr nlpe);

    /// Fix integer variables as in x
    void fixInts(const double *x,
                            std::stack<Modification *> &nlpMods,
                            ProblemPtr minlp);

    //Unfix integer variables
    void unfixInts(std::stack<Modification *> &nlpMods,
                              ProblemPtr minlp);

   /// Get wall clock time
   double getWallTime() {
     struct timeval time;
     if (gettimeofday(&time,NULL)) {
       // Handle error
       return 0;
     }
     return (double)time.tv_sec + (double)time.tv_usec * .000001;
   }

  protected:

    const static std::string me_;

    /// Average of dual variables from the previous iterations
    /// which is to be used to reduced cost diving
    //DoubleVector avgDual_;

    /// Engine being used to solve the problems during dive
    EnginePtr e_;

    /// NLP Engine (required only if e_ is LP Engine)
    EnginePtr nlpe_;

    /// Environment
    EnvPtr env_;

    /// Gradient of objective function for vector length diving
    //double* gradientObj_;

    /// If a value is this far from an integer, it is considered
    /// integer. 
    double intTol_;

    /// Mods implied by bound changes in the previous node's presolve. 
    /// We only store mods of one node only.
    //ModVector lastNodeMods_;

    /// Linear Handler for presolving.
    //LinearHandler *lh_;

    /// Logger
    LoggerPtr logger_;

    /// Maximum number of problem-solves allowed for each thread
    UInt maxProbs_;

    /// Maximum number of feasible solution required from heuristic
    UInt maxSol_;

    /** A stack of modification pointer. All modification are stored
     * in stack and for backtracking the stack is unwinded to restore
     * feasibility
     */
    //std::stack<VarBoundModPtr> mods_;

    /// Number of method for selection of variables
    UInt nSelector_;

    /// Number of threads to be used for the heuristic
    UInt numThreads_;

    /// Original problem (required only if p_ is LP relaxation)
    ProblemPtr minlp_;

    /// Problem to be solved
    ProblemPtr p_;

    /// Violated variable fraction part list
    //DoubleVector score_;

    /// Statistics for the heuristic
    DivingheurStats *stats_;

    /// Timer for this heuristic
    Timer* timer_;

    /// violated variable index list
    //UIntVector violated_;

    /// Wall time start
    double wallTimeStart_;

    /// Number of subsets to fix the fractional variables in each round
    UInt numLevels_;
    
    typedef UInt (ParMINLPDiving::*FuncPtr) (UInt numfrac, const double* x,
                                             Direction d, Order o,
                                             ProblemPtr p,
                                             UIntVector& violated,
                                             std::stack<VarBoundModPtr>& mods,
                                             LinearHandler* lh,
                                             ModVector& lastNodeMods,
                                             DoubleVector& score,
                                             DoubleVector& avgDual,
                                             double* gradientObj);

    /**
     * \brief Backtracking method
     *
     * \param[in] n_flipped Number of bound changes made in previous
     * dive
     *
     * All the changes are made by unwinding the modification stack
     * n_flipped number of times
     */
    //void backtrack_(UInt n_flipped);
    void backtrack_(UInt n_moded, ModVector& lastNodeMods, ProblemPtr p,
                    std::stack<VarBoundModPtr>& mods);
    /** 
     * \brief Fractional selection method for fractional variable
     * 
     * \param[in] numfrac Number of fractional variables in current
     * solution
     * \param[in] x Constant pointer to primal solution
     * \param[in] d Direction of rounding
     * \param[in] o Order for selection of fractional variables
     */
    //UInt FracBounds_(UInt numfrac, const double* x, 
                     //Direction d, Order o);


    UInt FracBounds_(UInt numfrac, const double* x, Direction d, Order o,
                     ProblemPtr p, UIntVector& violated,
                     std::stack<VarBoundModPtr>& mods, LinearHandler* lh,
                     ModVector& lastNodesMods, DoubleVector& score,
                     DoubleVector& avgDual, double* gradientObj);
    /**
     * \brief Get the score of solution
     *
     * \param[in] x Const pointer to the primal solution
     * \param[in] s Scoretype
     *
     * The score of the solution is determined based on one of the
     * predefined scoretypes.
     */
    //void getScore_(const double* x, Scoretype s);

    void getScore_(const double* x, Scoretype s, DoubleVector& score,
                   ProblemPtr p, DoubleVector& avgDual,
                   double* gradientObj);
    /**
     * \brief Function to implement diving
     *
     * \param[in] i Index of the method number
     * \param[in] x Const pointer to the root node primal solution
     * \param[in] s_pool Pointer to the solution pool
     *
     * Method for selection of fractional variable as candidate for
     * rouding are chosen for diving. Changes made to the problem are
     * stored in the stack of modification.
     */
    //void implementDive_(int i, const double*x, SolutionPoolPtr s_pool);
    void implementDive_(int i, const double* x, SolutionPoolPtr s_pool,
                        ModVector& lastNodesMods, EnginePtr e, ProblemPtr p,
                        DoubleVector& avgDual, UIntVector& violated,
                        std::stack<VarBoundModPtr>& mods, LinearHandler* lh,
                        DoubleVector& score, double* gradientObj,
                        DivingheurStats* stats);
    /** 
     * \brief The number of fractional variables in current solution
     *
     * \param[in] x Const pointer to primal solution
     *
     * \return Number of fractional variables
     */
    //UInt isFrac_(const double* x);
    UInt isFrac_(const double* x, UIntVector& violated, ProblemPtr p);

    /** 
     * \brief Lexicographic selection method for fractional variable
     * 
     * \param[in] numfrac Number of fractional variables in current
     * solution
     * \param[in] x Constant pointer to primal solution
     * \param[in] d Direction of rounding
     * \param[in] o Order for selection of fractional variables
     */
    UInt LexBounds_(UInt numfrac, const double* x, Direction d, Order o,
                    ProblemPtr p, UIntVector& violated,
                    std::stack<VarBoundModPtr>& mods, LinearHandler* lh,
                    ModVector& lastNodesMods, DoubleVector& score,
                    DoubleVector& avgDual, double* gradientObj);

    /** 
     * \brief Reduced cost diving selection method for fractional variable
     * 
     * \param[in] numfrac Number of fractional variables in current
     * solution
     * \param[in] x Constant pointer to primal solution
     * \param[in] d Direction of rounding
     * \param[in] o Order for selection of fractional variables
     */
    UInt ReducedCost_(UInt numfrac, const double* x, Direction d, Order o,
                      ProblemPtr p, UIntVector& violated,
                      std::stack<VarBoundModPtr>& mods, LinearHandler* lh,
                      ModVector& lastNodesMods, DoubleVector& score,
                      DoubleVector& avgDual, double* gradientObj);
    /**
     * \brief Restore the bounds for the problem
     *
     * \param[in] LB_copy Pointer to saved bounds
     * \param[in] UB_copt Pointer to saved bounds
     * \param[in] vars Number of variables in the problem
     */
    void restoreBounds_(double* LB_copy, double* UB_copy, UInt vars,
                        ProblemPtr p);

    /**
     * \brief Rounding a value in a given direction
     *
     * \param[in] value Fractional value to be rounded
     * \param[in] d Direction to be used for rounding
     *
     * \return rounded value of the fractional variable
     */
    double rounding_(double value, Direction d);

    /** 
     * \brief Save bounds of the problem
     *
     * \param[in] LB_copy Pointer to bounds, space has to be allocated
     * \param[in] UB_copt Pointer to bounds, space has to be allocated
     * \param[in] vars Number of variables in the problem
     */
    void saveBounds_(double* LB_copy, double* UB_copy, UInt vars);

    /**
     * \brief Select the method, ordering and direction.
     *
     * \param[in] i Index of the method number
     * \param     d Reference to direction of rouding
     * \param     o Reference to order for selecting variable
     *
     * \return    FuncPtr Address of the selected Diving method
     */
    FuncPtr selectHeur_(int i, Direction &d, Order &o);

    /** 
     * \brief Function to decide on diving or not
     *
     * return true or false 
     *
     * We decide not to dive if the problem size is large and hessian
     * is dense
     */
    bool shouldDive_();

    /** 
     * \brief Sort the score
     * 
     * \param[in] left Beginning of the container
     * \param[in] right End of the container
     *
     * If the number of non-zero in hessian or number of bin+int <20
     * don't dive
     */
    void sort_(UInt left, UInt right, DoubleVector& score,
               UIntVector& violated);

    /**
     * \brief Update the average of dual multiplier
     * 
     * \param[in] sol Constant Pointer to the solution
     */ 
    void updateAvgDual_(ConstSolutionPtr sol, DoubleVector& avgDual,
                        DivingheurStats* stats);

    /** 
     * \brief Vector Length selection method for fractional variable
     * 
     * \param[in] numfrac Number of fractional variables in current
     * solution
     * \param[in] x Constant pointer to primal solution
     * \param[in] d Direction of rounding
     * \param[in] o Order for selection of fractional variables
     */
    UInt VectorLength_(UInt numfrac, const double* x, Direction d, Order o,
                       ProblemPtr p, UIntVector& violated,
                       std::stack<VarBoundModPtr>& mods, LinearHandler* lh,
                       ModVector& lastNodesMods, DoubleVector& score,
                       DoubleVector& avgDual, double* gradientObj);

    /// Function to decide on vector length diving
    bool vectorFlag_(UInt min_vlength, ProblemPtr p);

  };

  typedef ParMINLPDiving* ParMINLPDivingPtr;
}
#endif

