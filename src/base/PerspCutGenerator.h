//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutGenerator.h 
 * \Declare PerspCutGenerator class . 
 * \author Meenarli Sharma, Indian Institute of Technology Bombay 
 */

#ifndef MINOTAURPERSPCUTGENERATOR_H
#define MINOTAURPERSPCUTGENERATOR_H

#include "CutManager.h"
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
  PerspCutGenerator(EnvPtr env, ProblemPtr problem);

  /// Destructor.
  ~PerspCutGenerator();

  /// Added perspective cut to a constraint at a given point

  bool addPC(RelaxationPtr rel, ConstSolutionPtr sol, UInt i, 
                              const double *ptToCut, double *prPt, bool isObj,
                              VariablePtr objVar, CutManagerPtr cutMan);

  /// Returns name of the handler.
  std::string getName() const;

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
   * Given point x violating PR amenable constraint give a feasible point 
   * using bisection. 
   */
  bool bisecPt(const double *x, double * newPt, UInt i,
                                bool isObj, double relVal);

  void cvxCombPt(RelaxationPtr rel, const double * y, UInt it);
 

  bool changeVarForm(VariablePtr v, double xc, double sb,                                   UInt itn, double *newPt);
  /**
   * Given outer-approximatio to a violated PR constraint
   */
  void linearAt(RelaxationPtr rel, const double *x, const double *y, double *c, LinearFunctionPtr *lf, UInt itn, int *error);

  /// Return vector of perspective amenable constraint
  std::vector<prCons> getPRCons() {return prCons_;};
  
  prObj getPRObj() {return prObj_;};
  
  ///// Return vector of binary variables of perspective amenable constraints
  //std::vector<VariablePtr> getPRBinVar() {return binvar_;};

  //std::vector<UInt> getPRStruct() {return sType_;};
 
  /// Check feasibility of PR amenable constraints at the given solution.
  bool isFeasible(const double *x, UInt i, bool isObj, double relVal);

  /// Generate variables for PR reformulation
  bool prVars(const double *x, double *prPt, UInt itn, bool isObj);

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

  bool isObj_;

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
