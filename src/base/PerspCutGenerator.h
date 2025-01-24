//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file PerspCutGenerator.h 
 * \Declare PerspCutGenerator class . 
 * \author Meenarli Sharma, Indian Institute of Technology Bombay 
 */

#ifndef MINOTAURPERSPCUTGENERATOR_H
#define MINOTAURPERSPCUTGENERATOR_H

#include "CutManager.h"
#include "Engine.h" 
#include "Problem.h"
#include "Function.h"
#include "PerspCon.h"
#include "SolutionPool.h"
#include "LinearFunction.h"

namespace Minotaur {
//MS: constraint form expected is f(x) <= b
// MS: make functions private/public
  class PerspCutGenerator;
  typedef PerspCutGenerator* PerspCutGeneratorPtr;
  typedef const PerspCutGenerator* ConstPerspCutGeneratorPtr;
 
  struct PRStats {
    size_t imprvPt;   /// Number of points feasible to original nonlinear constraint but not the PR constraint.
    size_t infPt;   /// Number of points infeasible to original constraint.
    size_t cuts;      /// Number of cuts added to the LP.
  }; 

  class PerspCutGenerator {
  public:

  /// Default constructor.
  PerspCutGenerator();
  
  /// Constructor.
  PerspCutGenerator(EnvPtr env, ProblemPtr problem, EnginePtr nlpe_);

  /// Destructor.
  ~PerspCutGenerator();

  /// Added perspective cut to a constraint at a given point

  bool addPC(RelaxationPtr rel, const double * y, UInt i, 
                              const double *ptToCut, const double *prPt, bool isObj,
                              VariablePtr objVar, CutManagerPtr cutMan, double relVal);

  /// Returns name of the handler.
  std::string getName() const;
  
  UInt getNumCuts() {return stats_->cuts;}

  void fixedValue(double * prPt, bool isObj, UInt i);

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
  //void linearAt(RelaxationPtr rel, FunctionPtr f, double fval,
                //const double *x, double *c, LinearFunctionPtr *lf,
                //int *error);
  /**
   * Add PCut to a PR amenable constraint whose associated
   * binary variable in integer at the given point (nlpx). 
   */
  void getCutFun(RelaxationPtr rel,LinearFunctionPtr *lf, UInt it, const double *x, 
               double *c, int *error);

  /// Add PCut torelaxation 
  bool addCutToRel(RelaxationPtr rel, const double * ptToCut, 
                                    LinearFunctionPtr lf, double c, VariablePtr objVar, double relVal, CutManagerPtr);
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
   * Given point (x,z) violating PR amenable constraint. Also, known that
   * (x,1) satisfies the PR constraint. Perform the bisection to find boundary
   * point. 
   */
  bool bisecPt(double *x, double * newPt, UInt i,
                                bool isObj, double relVal);

  void cvxCombPt(RelaxationPtr rel, const double * y, UInt it);
 
  bool changeVarForm(VariablePtr v, double xc, double sb, UInt itn, double *newPt, bool isObj, bool nlin);
  /**
   * Given outer-approximatio to a violated PR constraint
   */
  void linearAt(RelaxationPtr rel, const double *x, const double *prPt, double *c, LinearFunctionPtr *lf, UInt itn, int *error, bool isObj);

  /// Return vector of perspective amenable constraint
  std::vector<prCons> getPRCons() {return prCons_;};
  
  prObj getPRObj() {return prObj_;};
  
  ///// Return vector of binary variables of perspective amenable constraints
  //std::vector<VariablePtr> getPRBinVar() {return binvar_;};

  //std::vector<UInt> getPRStruct() {return sType_;};
 
  /// Check feasibility of PR amenable constraints at the given solution.
  UInt isFeasible(const double *x, UInt i, bool isObj, double relVal);

  /// Generate variables for PR reformulation. Case in which z = 0 fixes x
  //variables, if z has value 0 then the function returns false; And if z has
  //value 1, then the function returns the same point. Similarly, for z = 1
  //fixing case.
  bool prVars(const double *y, double *prPt, UInt itn, bool isObj);

  void findPRCons();

  bool uniDirZSearch(double *y, double * newPt, UInt i, double zdir);

  /// Return 1 if there are PR amenable constraints in the problem
  bool getStatus();
  

  /// Perform line search using analytical center solC_. This search happens
  //only if solC_ satisfies the PR amenable constraint or (xC, 1) satisfies,
  //where xC is x components of  solC_.
  bool lineSearchAC(double *y, double * newPt, UInt i, bool isObj, double relVal);

  /// Writes statistics.
  void writeStats(std::ostream &out) const;
  
private:

  //void fixBinVars_(std::stack<Modification *> &varMods);
  
  //void unfixBinVars_(std::stack<Modification *> &varMods);
  
  bool lineSearch_(double *y, double * newPt, UInt i, bool isObj, double relVal);
  /// Environment.
  EnvPtr env_;
  
  /// Problem for which the handler is created.
  ProblemPtr minlp_;
 
  EnginePtr nlpe_;
  
  /// Log.
  LoggerPtr logger_;
 
  /// Tolerance for accepting a new solution value: absolute threshold.
  double solAbsTol_;
  
  /// Tolerance for accepting a new solution value: relative threshold.
  double solRelTol_;
  
  /// Absolute tolerance for objective.
  double objAbsTol_;

  /// Relative tolerance for objective.
  double objRelTol_;
  
  /// Tolerance for checking integrality.
  double intTol_;

  /// Convex combination of feasible point
  double* cvxPt;

  /// Statistics.
  PRStats *stats_;

  /// For log:
  static const std::string me_;
  
  /// Vector of perspective constraint pointers
  std::vector<prCons> prCons_;

  prObj prObj_;

  double * solC_;

  //bool isObj_;

  };
}

#endif // MINOTAURPERSPCUTHANDLER_H

