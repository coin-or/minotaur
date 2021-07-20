//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.h
 * \brief Declare PerspCutHandler class for perspective reformulation (PR).
 * It detects constraints amenable to perspective reformulation and generates
 * perspective cuts whenever needed. 
 * \author Meenarli Sharma, Indian Institute of Technology
 */

#ifndef MINOTAURPERSPCUTHANDLER_H
#define MINOTAURPERSPCUTHANDLER_H

#include "Handler.h"
#include "Problem.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"

namespace Minotaur {
//MS: constraint form expected is f(x) <= b
  class PerspCutHandler;
  typedef PerspCutHandler* PerspCutHandlerPtr;
  typedef const PerspCutHandler* ConstPerspCutHandlerPtr;
 
  struct PRStats {
    size_t imprvPt;   /// Number of points feasible to original nonlinear constraint but not the PR constraint.
    size_t infPt;   /// Number of points infeasible to original constraint.
    size_t cuts;      /// Number of cuts added to the LP.
  }; 

  class PerspCutHandler : public Handler {
  public:

  /// Default constructor.
  PerspCutHandler();
  
  /// Constructor.
  PerspCutHandler(EnvPtr env, ProblemPtr problem);

  /// Destructor.
  ~PerspCutHandler();

  /// Does nothing.
  virtual Branches getBranches(BrCandPtr, DoubleVector &,
                               RelaxationPtr, SolutionPoolPtr)
    {return Branches();}; 

  // Does nothing.
  virtual void getBranchingCandidates(RelaxationPtr,
                                      const DoubleVector &, ModVector &,
                                      BrVarCandSet &, BrCandVector &,
                                      bool &) {};

  /// Does nothing.
  virtual ModificationPtr getBrMod(BrCandPtr, DoubleVector &,
                                   RelaxationPtr, BranchDirection)
    {return ModificationPtr();};

  /// Returns name of the handler.
  std::string getName() const;

    /// Does nothing.
  SolveStatus presolve(PreModQ *, bool *, Solution **sol) {return Finished;};

  /// Does nothing.
  virtual bool presolveNode(RelaxationPtr, NodePtr,
                            SolutionPoolPtr, ModVector &,
                            ModVector &) {return false;};

  /// Does nothing.
  void relaxInitFull(RelaxationPtr, bool * ) {};
  
  /// Does nothing.
  void relaxInitInc(RelaxationPtr, bool * ) {};
  
  /// Does nothing.
  void relaxNodeFull(NodePtr, RelaxationPtr, bool * ) {};

  /// Does nothing.
  void relaxNodeInc(NodePtr, RelaxationPtr, bool * ) {};

  /// Separates current solution. 
   void separate(ConstSolutionPtr sol, NodePtr node, 
                RelaxationPtr rel, CutManager *cutman, SolutionPoolPtr s_pool, 
                ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                SeparationStatus *status);


  /**
   * Add Perspective cut (PCut) to a violated PR amenable constraint whose associated
   * binary variable in integer at the given point (nlpx). 
   */
  void addCut(RelaxationPtr rel, const double *nlpx, const double *lpx, UInt it,
              SeparationStatus * pcStatus);

  /**
   * Add PCut to violated PR amenable constraints whose associated
   * binary variable in integer at the given point (nlpx). 
   */
  void atIntPt(RelaxationPtr rel , const double * nlpx, 
                 const double *lpx, SeparationStatus * pcStatus, CutManager *);
  
  /**
   * Add OA cut to original constraint of a violated PR constraint whose
   *  associated binary variable has value 0.
   */
  void linearAt(RelaxationPtr rel, FunctionPtr f, double fval,
                const double *x, double *c, LinearFunctionPtr *lf,
                int *error);
  /**
   * Add PCut to a PR amenable constraint whose associated
   * binary variable in integer at the given point (nlpx). 
   */
  void getCutFun(RelaxationPtr rel,LinearFunctionPtr *lf, UInt it, const double *x, 
               double *c, int *error);

  /// Add PCut torelaxation 
  void addCutToRel(RelaxationPtr rel, LinearFunctionPtr lf, double c);
 
  /**
   * Add PCut to a PR amenable constraint. At point (x) original 
   * constraint is known to be satisfied but not necessarily PR constraint. If PR
   * constraint is satisfied at x then PCut is added at x.
   * Otherwise, a feasible point to add PCut is obtained using bisection. 
   */ 
  //void addCutOriFeas(RelaxationPtr rel , UInt it, const double *x);

  /// Iterate over PR amenable constraints to add PCut;
  void oriFeasPt(RelaxationPtr rel , const double *x);

  void oriInfeasPt(RelaxationPtr rel, const double *x);

  /**
   * Add PCut to a PR amenable constraint. If both the original and
   * PR amenable constraints are satisfied at x, PCut is added at x. If only
   * former is satisfied, then a point feasible to latter is obtained using
   * bisection at which PCut is then added. If both the constraints are not
   * satisfied then first a point feasible to original constraint is obtained
   * using which a point satisfying PR amneable constraint is obtained using
   * bisection, at which PCut is added. 
   */ 
  //void addPCut(RelaxationPtr rel , UInt it, const double *x);

  void atPtx(RelaxationPtr rel, const double *x);
  
  void cutToCons(RelaxationPtr rel, const double *x, UInt it);
  /**
   * Determine a point feasible to orginal constraint when the given point (x) 
   * violates both original and PR reformulated constraints.
   */
  //void feasBisecPt(const double *x, double *y, UInt it);

  /**
   * Give variable values when associated binary variable is 0 for a PR 
   * amenable constraint.
   */ 
  //void vertexPt(double * y, UInt it);

  /**
   * Find a point (w1) feasible to original constraint using bisection method 
   * on points x and w, where former in feasible and latter is infeasible to
   * the original constraint. 
   */
  //void bisecPtOri(const double *x, double * w, double * w1, UInt it);


  /**
   * Given point x violating PR amenable constraint give a feasible point 
   * using bisection. 
   */
  void bisecPt(const double *x, double * y, UInt it);
  
  void cvxCombPt(RelaxationPtr rel, const double * y, UInt it);
 
  double changeVarForm(VariablePtr v, double xc, double xb, UInt itn,
                       int *error);
  /**
   * Given outer-approximatio to a violated PR constraint
   */
  void linearAt(RelaxationPtr rel, const double *y, double *c, LinearFunctionPtr *lf, UInt itn, int *error);

  /// Return vector of perspective amenable constraint
  std::vector<ConstraintPtr> getPRCons() {return cons_;};
  
  ///// Return vector of binary variables of perspective amenable constraints
  //std::vector<VariablePtr> getPRBinVar() {return binvar_;};

  //std::vector<UInt> getPRStruct() {return sType_;};
 
  /// Check feasibility of PR amenable constraints at the given solution.
  bool isFeasible(ConstSolutionPtr, RelaxationPtr, bool &, double &);

  /// Generate variables for PR reformulation
  void prVars(const double *x, double *y,  UInt itn, int *error);

  void findPRCons();

  /// Return 1 if there are PR amenable constraints in the problem
  bool getStatus();


  /// Writes statistics.
  void writeStats(std::ostream &out) const;
  
private:
  
  /// Environment.
  EnvPtr env_;
  
  /// Problem for which the handler is created.
  ProblemPtr minlp_;
  
  /// Log.
  LoggerPtr logger_;
 
  /// Tolerance for accepting a new solution value: absolute threshold.
  double solAbsTol_;
  
  /// Tolerance for accepting a new solution value: relative threshold.
  double solRelTol_;
  
  /// Tolerance for checking integrality.
  double intTol_;

  /// Convex combination of feasible point
  double* cvxPt;

  /// Statistics.
  PRStats *stats_;

  /// For log:
  static const std::string me_;
  
  /// Vector of perspective constraint pointers
  std::vector<ConstraintPtr> cons_;
   
  /// Vector of reformulated perspective constraint pointers
  std::vector<ConstraintPtr> prcons_;
  
  /// Vector of pointers to binary variables of perspective constraints
  std::vector<VariablePtr> binvar_;

  /// Vector of pointers to structure type of perspective constraints
  std::vector<int> sType_;
  
  /// Vector of pointers to values to which variables in the nonlinear part of
  /// the constraints amenable to PR are fixed to
  std::vector<VariableGroup> nviv_;
 
 /// Vector of pointers to values to which variables in the linear part of
 // the constraints amenable to PR are fixed to 
  std::vector<VariableGroup> lviv_;
  };
}

#endif // MINOTAURPERSPCUTHANDLER_H

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
