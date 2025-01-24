// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 -- 2011 The MINOTAUR Team.
// 

/**
 * \file quasiQGHandler.h
 * \brief Declare the derived class of Handler that handles convex constraints
 * of a problem by using Quesada-Grossmann algorithm.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURQGHANDLER_H
#define MINOTAURQGHANDLER_H

#include <stack>
#include <fstream>

//#include "CGraph.h"
//#include "CNode.h"
//#include "CutManager.h"
#include "CutMan2.h"
//#include "Engine.h"
//#include "Function.h"
#include "Handler.h"
//#include "Problem.h"
#include "Types.h"
#include "Variable.h"
//#include "Constraint.h"

namespace Minotaur {

//  class SolutionPool;
  class SolutionPool; 
  class quasiACProblem;
  class WarmStart;
  class CutManager;
  class Problem;
  class Engine;
  class Constraint;
  class Logger;
  class LinearFunction;
  class QuadraticFunction;
  class Function;
  class Objective;
  class ProblemSize;
  class CNode;
  class CGraph;
  class Timer;
  class quasiSolutionPool;
  class quasiSolution;

  typedef boost::shared_ptr<SolutionPool> SolutionPoolPtr;
  typedef boost::shared_ptr<WarmStart> WarmStartPtr;
  typedef boost::shared_ptr<quasiACProblem> quasiACProblemPtr;
  typedef boost::shared_ptr<CutManager> CutManagerPtr;
  typedef boost::shared_ptr<Problem> ProblemPtr;
  typedef boost::shared_ptr<Engine> EnginePtr;
  typedef boost::shared_ptr<Constraint> ConstraintPtr;
  typedef boost::shared_ptr<Logger> LoggerPtr;
  typedef boost::shared_ptr<Objective> ObjectivePtr;
  typedef boost::shared_ptr<ProblemSize> ProblemSizePtr;
  typedef boost::shared_ptr<QuadraticFunction> QuadraticFunctionPtr;
  typedef boost::shared_ptr<const ProblemSize> ConstProblemSizePtr;
  typedef boost::shared_ptr<quasiSolution> quasiSolutionPtr;
  typedef std::vector<quasiSolutionPtr>::iterator quasiSolutionIterator;
  typedef boost::shared_ptr<quasiSolutionPool> quasiSolutionPoolPtr;

//  typedef boost::shared_ptr<CNode> CNodePtr;
//  typedef boost::shared_ptr<CGraph> CGraphPtr;  

//  typedef std::vector<CNodePtr>CNodeVector;

struct QGStats {
  Size_t nlpS;      /// Number of nlps solved.
  Size_t nlpP;      /// Number of projection solved.
  Size_t nlpF;      /// Number of nlps feasible.
  Size_t nlpI;      /// Number of nlps infeasible.
  Size_t cuts;      /// Number of cuts added to the LP.
  Size_t looseCuts; /// Number of loose cuts needed to strengthen
  Size_t nlpP1;     /// Number of l1-projection solves.
  Size_t infCuts;   /// Number of so-called infeasible-cuts
  Size_t nlpPC;     /// Number of Continuous projections
  Size_t p2Cuts;    /// Number of l2-cuts added
  Size_t p1Cuts;    /// Number of l1-cuts added
  Size_t CoCuts;    /// Number of continuous-cuts added
  Size_t adCuts;
  double strTime;
  Size_t ecpCalls;
}; 

struct QGNewCut {
  std::vector<double*> coef;
  std::vector<double> rhs;
  double    newUb;
};

/**
 * quasiQGHandler is a derived class of VariableHandler. It adds cuts generated
 * by solving an NLP whenever an integer (but infeasible) solution is found.
 */

class quasiQGHandler : public Handler {

public:
  /// Default constructor.
  quasiQGHandler();

  /// Constructor.
  quasiQGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe); 

  /// Constructor.
  quasiQGHandler(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe, EnginePtr ACnlpe); 

  /// Constructor.
  quasiQGHandler(EnvPtr env, ProblemPtr minlp, ProblemPtr proj_inst, ProblemPtr l1proj_inst, EnginePtr nlpe, EnginePtr proj_nlpe, EnginePtr l1proj_nlpe, EnginePtr ACnlpe);
 
  /// Destroy.
  ~quasiQGHandler();
      
  // calls relax_().
  void relaxInitFull(RelaxationPtr rel, Bool *is_inf);

  // calls relax_().
  void relaxInitInc(RelaxationPtr rel, Bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  // Check if x is feasible. x has to satisfy integrality and also
  // nonlinear constraints.
  Bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  Bool & is_inf);

  // Find cuts.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, CutManager *cutman,
                SolutionPoolPtr s_pool, Bool *sol_found, SeparationStatus *status);

  /// Does nothing.
  virtual void getBranchingCandidates(RelaxationPtr , 
                                      const std::vector< double > &, ModVector &, BrCandSet &, Bool &)
    {};

  /// Does nothing.
  virtual ModificationPtr getBrMod(BrCandPtr, DoubleVector &, 
                                   RelaxationPtr, BranchDirection)
    {return ModificationPtr();}; // NULL

  /// Does nothing.
  virtual Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr, 
                               SolutionPoolPtr)
    {return Branches();}; // NULL
      
  /// Does nothing.
  SolveStatus presolve(PreModQ *, Bool *) {return Finished;};

  /// Does nothing.
  void postsolveGetX(const double *, UInt, DoubleVector *) {};

  /// Does nothing.
  Bool presolveNode(ProblemPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

 
  // Show statistics.
  void writeStats(std::ostream &out) const;

  // Write name
  std::string getName() const;

  Size_t QG_nlpS(){return stats_->nlpS;};
  Size_t QG_nlpP(){return stats_->nlpP;};
  Size_t QG_nlpI(){return stats_->nlpI;};
  Size_t QG_nlpF(){return stats_->nlpF;};
  Size_t QG_cuts(){return stats_->cuts;};
  Size_t QG_looseCuts(){return stats_->looseCuts;};
  Size_t QG_nlpP1(){return stats_->nlpP1;};
  Size_t QG_infCuts(){return stats_->infCuts;};
  Size_t QG_nlpPC(){return stats_->nlpPC;};
  Size_t QG_p2Cuts(){return stats_->p2Cuts;};
  Size_t QG_p1Cuts(){return stats_->p1Cuts;};
  Size_t QG_CoCuts(){return stats_->CoCuts;};
  Size_t QG_adCuts(){return stats_->adCuts;};
  double QG_strTime(){return grdTime_;};
  Size_t QG_Cuts() {return stats_->cuts;}
  Size_t QG_ecpCalls(){return stats_->ecpCalls;}

  void setCutManager(CutMan2Ptr ctMngr){ctMngr_ = ctMngr;}
  void setECP(Bool ecp);//{ECP_ = ecp;}
  void setBetaECP(double beta){betaECP_ = beta;}
  void setTolECP(double tol){tolECP_ = tol;}

  void setPointProbEngine(EnginePtr pntE) { pntE_ = pntE; }

private:
  /// Pointer to environment.
  EnvPtr env_;

  /// Pointer to original problem.
  ProblemPtr minlp_;

  /// Pointer to projection problem.
  ProblemPtr proj_minlp_;

  /// Pointer to l1-projection problem.
  ProblemPtr l1proj_minlp_;

  ///Linear part of the projection objective function
  LinearFunctionPtr proj_lf_;
   
  ///Quadratic part of the projection objective function
  QuadraticFunctionPtr proj_qf_;

  ///FunctionPtr to l1-project objective function
  FunctionPtr l1_proj_;

  ///vector to ConstraintPtr to l1-project linearization
  std::vector<ConstraintPtr>l1_linConst_;
  std::vector<int>IntVarID_;
  /// Pointer to original problem.
  RelaxationPtr rel_;

  /// Pointer to cut manager
  CutMan2Ptr ctMngr_;

  /// Log.
  LoggerPtr logger_;

  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;

  EnginePtr ACnlpe_;
  EnginePtr proj_nlpe_;

  ///Engine used to solve l1-Projection problem
  EnginePtr l1proj_nlpe_;

  /// Status of the NLP/QP engine.
  EngineStatus nlpStatus_;

  EngineStatus projnlpStatus_;
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

  /// Vector of constraints.
  std::vector<ConstraintPtr> nlCons_;

  /// Is the objective function nonlinear?
  Bool oNl_;

  /**
   * Was this handler used to check the feasibility. We generate cuts
   * only if we checked the feasibility and it is false.
   */
  Bool isFeas_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;

  /**
   * For any linearization constraint that we generate, all 
   * coefficients with absolute value less than it are assumed zero.
   */
  const double linCoeffTol_;

  /// Statistics.
  QGStats *stats_;

  /// Tolerance for accepting a new solution value: absolute threshold.
  const double solAbsTol_;

  /// Tolerance for accepting a new solution value: relative threshold.
  const double solRelTol_;

  /// Tolerance for checking constraint violation.
  double eTol_;

  /// Tolerance for checking linear cut violation.
  double eLinTol_;
 
  /// Tolerance for checking integrality (should be obtained from env).
  double intTol_;

  /// For log:
  static const std::string me_;

  /**
   * Solve the NLP relaxation of the MINLP and add linearizations about
   * the optimal point. isInf is set to true if the relaxation is found
   * infeasible. Throw an assert if the relaxation is unbounded.
   */
  void initLinear_(Bool *isInf);

  /**
   * Find the linearization of nonlinear functions at point x* and add
   * them to the relaxation only (not to the lp engine)
   */
  void addInitLinearX_(const double *x);

  /** 
   * When the objective function is nonlinear, we need to replace it with
   * a single variable.
   */
  void linearizeObj_(RelaxationPtr rel);

  /**
   * Add cuts to cut out an integer point not satisfied by nonlinear
   * constraints or objective.
   */
  void cutIntSol_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                  Bool *sol_found, SeparationStatus *status);

  /**
   * Fix integer constrained variables to integer values in x. Called
   * before solving NLP.
   */
  void fixInts_(const double *x);

  /// Undo the changes done in fixInts_().
  void unfixInts_();

  /**
   * Create the initial relaxation. It is called from relaxInitFull and
   * relaxInitInc functions.
   */
  void relax_(RelaxationPtr rel, Bool *is_inf);

  /// Solve the nlp.
  void solveNLP_();

  /// Solve the projection
  void solveProjection_();


  /// Update the coefficients of the linear part of the ProjObjective
  void ProjObjUpdate(const double * x, double& constantTerm);

  /**
   * Make a copy of the bounds on variables. Called before bounds on NLP
   * are changed.
   */
  void copyLPBounds_(std::stack<Modification *> *mods);

  /// Restore bounds.
  void undoLPBounds_(std::stack<Modification *> *mods);

  /**
   * Update the upper bound. XXX: Needs proper integration with
   * Minotaur's Handler design. 
   */
  void updateUb_(SolutionPoolPtr s_pool, double *nlp_val, 
                 Bool *sol_found);


  void updateUb_new_(SolutionPoolPtr s_pool, double *nlp_val, 
                     double *xhat, Bool *sol_found);


  /// Add all linearizations at point x that violate inf_x.
  int OAFromPoint_(const double *x, ConstSolutionPtr sol, 
                    SeparationStatus *status);

  int OAFromPoint_(const double *x, const double *inf_x,
                             SeparationStatus *status);

  int OAFromPointInf_(const double *x, const double *inf_x, 
                    SeparationStatus *status);

  /// Add projection cut at point x
  int ProjCutFromPoint_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, Bool *sol_found,
                         SeparationStatus *status);
  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf);

  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf, double *a);

  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf, double *a,
		 double *k);

  void readSol_();

  double * x_star;

  Bool handleQuasiConvex_;

  UInt num_cuts_;

  double * a_;

  const double * x_pr_;

  const double * x_OA_;

  quasiACProblemPtr qgACP_;

  Bool doACCPM_;
 

  void ACCutsToRelax_(QGNewCut qgnewcut);

  int ObjId_;
 
  UInt last_node_;
  double *last_sol_;
  double lastOptVal_;
  Bool BrootSol_;
  Bool BincSol_;
  Bool BLPrelSol_;
  
  const double *rootSol_;  // The solution to the NLP relaxation at root node
  const double *incSol_;
  const double *LPrelSol_;

  double constCoef_;

  Bool onlyBinary_;  //true if the discrete variables are only binary to add no-good cuts

  void no_good(const double *x);


  int numvars_;
  Bool partialFix_;
  double cuttingPoint_;

  LinearFunctionPtr lf_pure_;
  QuadraticFunctionPtr qf_pure_;

  void solvePartialNLP_(ConstSolutionPtr sol, SolutionPoolPtr s_pool, 
                        Bool *sol_found, SeparationStatus *status);

  // Either dir = 0 if Ub < INFINITY and violated or dir = 1 if Lb > -INFINIT and violated
  void cutXLP_(const double *x_nlp, const double *x_lp,
                             ConstraintPtr con, SeparationStatus *status, Bool dir);

  void cutXLP_(const double *x_nlp, const double *x_lp,
		double *x_alpha, ConstraintPtr con, Bool dir);

  void cutXLP_(const double *x_nlp, const double *x_lp,
                          double *x_alpha, FunctionPtr fn, double ub, double lb, Bool dir);

  void cutXLPObj_(const double *x_nlp, const double *x_inf,
                  double *x_alpha, FunctionPtr funobj);

  int alphaCutProj_(const double *x_OA, const double *x_pr, SeparationStatus *status);

  int pure_project_(const double *x_OA, SeparationStatus *status);
 
  int l1_project_(const double *x_OA, SeparationStatus *status);
  
  double relobj_;  
  Bool objCutOff;

  int numExLin_;	//Number of calls to extra linearizations

  int ExtLinFr_; //Frequency by which extra linearizations are added
  const double *xnlp_;

  void ExtraCuts_(const double *x, const double *xnlp);

  void ExtraCutsCvx_(const double *x, const double *xnlp);

  void strCutsCvx_(ConstSolutionPtr sol, const double *xnlp);

  int ECPCuts_(ConstSolutionPtr sol);

  int strECPCuts_(ConstSolutionPtr sol);
 
  int strCutsQuasi_(ConstSolutionPtr sol, const double *xnlp);

  int strCutsQuasi1_(ConstSolutionPtr sol, const double *xnlp);
  int strCutsQuasi2_(ConstSolutionPtr sol, const double *xnlp);
  
  double zoom_(ConstraintPtr con, double alpha1, double alpha2, double fval, double gnorm,
		const double *x, double *gr);

  double LineSearch_(ConstraintPtr con, const double *x, double *xalpha, double *gr);
  double LineSearch1_(ConstraintPtr con, const double *x, double *xalpha, double *gr);
  void LineSearch2_(ConstraintPtr con, const double *x, double *xalpha, 
		    double *gr, Bool *admit);


//  std::string fname_;
//  std::ofstream mylogfile_;
  std::ofstream mylogfile_;

  double grdTime_;
  double ctmngrTime_;
  Timer *timer_;

  Bool ECP_;
  double betaECP_;
  double tolECP_;

  /// solve continuous relaxation in each node to find 
  /// a point at which ECP cuts is derived
  UInt QuasiECP_ContNLP(ConstSolutionPtr sol);

  /// similar to ContNLP, but NLP engine only does some iterations
  UInt QuasiECP_ContNLPLimited(ConstSolutionPtr sol);

  UInt allLinearAt_(const double *x,const double *xlp);

  ProblemPtr PointProb_;
  QuadraticFunctionPtr qfPoint_;
  /// update the problem of finding a feasible solution
  void updatePointProb_(const double *x);

  void addPointToProb_(const double *x);

  /// create the problem 
  void createPointProb_();
 
  quasiSolutionPoolPtr quasiSol_;

  UInt solvePointProb_(const double *xlp);

  UInt bisectAddCut_(const double *xlp, const double *xnlp);
  UInt bisectAddCut_(ConstraintPtr con,const double *xlp, const double *xnlp);

  UInt QuasiECP_ContProj(ConstSolutionPtr sol);

  EnginePtr pntE_;

  UInt fixFrac(ConstSolutionPtr sol);
};


class quasiSolution {
public:
  quasiSolution();

  quasiSolution(int num,const double *x);

  ~quasiSolution();

  int getSize() {return size_;}

  double * getSolution();// {return x_;}

private:

  double *x_;

  int size_;
};

class quasiSolutionPool {
public:
  quasiSolutionPool();

  quasiSolutionPool(int size);

  ~quasiSolutionPool();

  quasiSolutionIterator quasiSolsBegin() {return quasiSol_.begin(); }

  quasiSolutionIterator quasiSolsEnd() {return quasiSol_.end(); }

  void addSol(const double *x);

  int getNum() {return quasiSol_.size();}

private:

  std::vector<quasiSolutionPtr> quasiSol_;

  int size_;
};

/*
class quasiACProblem{
public:

  quasiACProblem();

  quasiACProblem(EnvPtr env, EnginePtr ACnlpe, RelaxationPtr ACrel, Bool QC, std::vector<ConstraintPtr> nlCons_);

  ~quasiACProblem();


  QGNewCut ACCPM_();


  std::vector<ConstraintPtr> ACCuts(double *x, Bool *objcut, double *rhs);

  void addCut(double *coef, double rhs, Bool IneDir); //IneDir is the direction of the inequality, IneDir = true if <=, false otherwise.

  void updateObjConst(double ub);

private:
   /// Log.
   LoggerPtr logger_;

   /// For log:
   static const std::string me_;

   std::vector <double*> gr_;

   std::vector <double*> rhs_;

   UInt ObjID_;

  ///CGraph to create objective function of Log Barrier function
  //CGraphPtr ACLogBar_;

  ///Pointer to objective function of Analytic Center Problem
  FunctionPtr ACObjFunc_;

  ///Vector of variables added for each linearization of Analytic Center
  CNodeVector AC_Nodes_vars_;

  ///Vector of nodes for log
  CNodeVector AC_Nodes_logs_;
 
  /// Vector of New Linearizations 
  std::vector<ConstraintPtr> nlNewCons_;

  /// Vector of New Variables added for New Linearizations
  std::vector<VariablePtr> nlNewVars_;

  /// Tolerance for checking constraint violation.
  double eTol_;

//   * For any linearization constraint that we generate, all 
//   * coefficients with absolute value less than it are assumed zero.

  const double linCoeffTol_;

  /// Pointer to the Analytic Center Problem
  ProblemPtr quasiACProblem_;

  /// Pointer to original problem.
  RelaxationPtr ACrel_;

  /// Vector of constraints.
  std::vector<ConstraintPtr> nlCons_;

  Bool handleQuasiConvex_;

  EnginePtr AC_Engine_;

  EngineStatus AC_EngStatus_;

//   * Obtain the linear function (lf) and constant (c) from the
//   * linearization of function f at point x.

  CNode *finalNode_;
  CGraphPtr ACLogBar_;


  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf);


  void createLogBarrier_();

  //void createACP_();

  void ACUpdate_(const double *x);


  void updateLogBar_(VariablePtr v);

  void solvequasiACProblem_();

  std::vector<ConstraintPtr> consAC_;


  int numACCPMSolve_;
 
  double newObjBound;
};
*/
  typedef boost::shared_ptr <quasiQGHandler> quasiQGHandlerPtr;
//  typedef boost::shared_ptr <CNode> CNodePtr;
//  typedef std::vector<CNodePtr> CNodePtrVector;
//  typedef boost::shared_ptr <CutManager> CutManagerPtr;
}


#endif




