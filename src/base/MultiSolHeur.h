//
//     Minotaur -- It's only 1/2 bull  
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//
/** 
 * \file MultiSolHeur.h
 * \brief Declare the class MultiSolHeur derived from base class Heuristic.
 * \author Meenarli Sharma, Prashant Palkar, Indian Institute of Technology
 * Bombay
 */

#ifndef MULTISOLHEUR_H
#define MULTISOLHEUR_H

#include "CutManager.h"
#include "Heuristic.h"
#include "MILPEngine.h"
#include "Relaxation.h"
#include "Solution.h"
#include "Types.h"

namespace Minotaur {
  class Engine;
  class Problem;

  /// statistics for heuristic
  struct MultiSolHeurStats {
    UInt milpS;        /// Number of MILPs solved in the heuristic
    UInt nlpS;         /// Number of NLPs solved in the heuristic
    UInt nlpLim;        /// Number of NLPs hit engine limit
    UInt nlpInf;       /// Number of infeasible NLPs
    UInt nlpOthr;      /// Number of NLPs other than opt and inf
    UInt cuts;         /// Number of cuts generated
    double time;       /// Total time taken by the heuristic
  };

  class MultiSolHeur : public Heuristic {

  public:

    /// default constructor
    MultiSolHeur();

    /// constructor for derived class
    MultiSolHeur(EnvPtr env, ProblemPtr p, EnginePtr nlpe, MILPEnginePtr milpe);
    /// default destructor
    virtual ~MultiSolHeur();

    /// call to the heuristic and find solutions with value less than or equal
    //to ub
    void solveAtSol(ConstSolutionPtr sol, RelaxationPtr rel, SolutionPoolPtr sPool);

    void solve(NodePtr, RelaxationPtr, SolutionPoolPtr) {};

    /// write statistic to the logger
    void writeStats(std::ostream &out) const;

  private:

    /// Message name for the heuristic
    const static std::string me_;
    
    /// Pointer to the environment
    EnvPtr env_;

    /// Pointer to the problem being solved
    ProblemPtr minlp_;
    
    /// Pointer to the relaxation problem being solved
    ProblemPtr miqp_;

    /// Pointer to the NLP engine to be used to solve the problem
    EnginePtr nlpe_;
    
    /// Pointer to the MILP engine to be used to solve the problem
    MILPEnginePtr milpe_;
    
    /// Solution pool returned by the heuristic
    //SolutionPoolPtr sPool_;

    /// Pointer to the logger
    LoggerPtr logger_;

    /// Best known upper bound
    double ub_;
    
    /// Best known lower bound
    double lb_;
  
    VariablePtr objVar_;

    SolutionPtr bestSol_;
  
    bool newSolFound_;

    /// Tolerance for a number to be considered as an integer
    double intTol_;

    double objATol_;
    
    double solATol_;

    double objRTol_;
    
    double solRTol_;

    /// Vector of nonlinear constraints.
    std::vector<ConstraintPtr> nlCons_;

    /// Approximation of the center of the feasible region
    double * xC_;

    /// Statistics for the Feasibility Pump heuristic
    MultiSolHeurStats* stats_;

    /// Timer of the heuristic
    Timer* timer_;
  
  
    /// Modifications done to rel or NLP before solving it.
  
    std::stack<Modification *> mods_; 
    
    void cutToCons_(const double *nlpx, const double *lpx,
                           SeparationStatus *status);

    void cutsAtLpSol_(const double *lpx, SeparationStatus *status);

    void addCut_(const double *nlpx, const double *lpx,
                        ConstraintPtr con,
                        SeparationStatus *status);

    void linearAt_(FunctionPtr f, double fval, const double *x,
                          double *c, LinearFunctionPtr *lf, int *error);

    FunctionPtr newObj_(const double * x0, const double * x);
    /**
   
     * Fix integer constrained variables to integer values in x. Called
     *  before solving NLP.
     */
    void relMods_(const double *x);

    void undoMods_();

    //// Solve restricted MILP
    void solveMILP(SolveStatus &status);
    //void solveMILP(RelaxationPtr rel, SolveStatus &status);

    void findCenter_();

    void findSol_(ConstSolutionPtr sol, SolutionPoolPtr sPool,
                            bool &isTerm,
                            //double oldBest, bool &isTerm,
                            std::vector<SolutionPtr> & infSols,
                            std::vector<SolutionPtr> & limSols);
    void ifOnlyNonlinObj_();  

    void ifNonlinCons_();
    
    void improveSol_(const double * x0);

    void itrMilpSolve_(SolutionPoolPtr sPool);

    void fixInts_(const double *x);

    void newMIQP_(RelaxationPtr rel, const double * x0,
                            SolutionPoolPtr sPool);

    void solveNLP_();

    void unfixInts_();

  };

  typedef MultiSolHeur* MultiSolHeurPtr;

}

#endif

