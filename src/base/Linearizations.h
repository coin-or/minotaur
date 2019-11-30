// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/**
 * \file Linearizations.h
 * \Briefly declare a class for generating extra linearizations in linearization
 *  based methods.
 * \Author Meenarli Sharma, Indian Institute of Techonology Bombay.
 */

#ifndef MINOTAURLINEARIZATIONS_H
#define MINOTAURLINEARIZATIONS_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "Problem.h"
#include "Function.h"

#include "Solution.h"

namespace Minotaur {

  struct LinStats {
  size_t rs1Cuts; /// Number of cuts in root scheme 1.
  size_t rs2Cuts; /// Number of cuts in root scheme 2.
  size_t rs3Cuts; /// Number of cuts in root scheme 3 version 1.
  size_t rs4Cuts; /// Number of cuts in root scheme 3 version 2.
  size_t rgs1Cuts; /// Number of cuts in root gen scheme 1.
  size_t rgs2Cuts; /// Number of cuts in root gen scheme 2.
};

class Linearizations {

private: 
  /// Pointer to environment.
  EnvPtr env_;

  /// Pointer to relaxation of the problem.
  RelaxationPtr rel_;

  /// Pointer to original problem.
  ProblemPtr minlp_;

  /// Tolerance for checking integrality (should be obtained from env).
  double intTol_;

  /// Log.
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// Vector of nonlinear constraints.
  std::vector<ConstraintPtr> nlCons_;

  /// LP Engine used in root linearization scheme 3
  //EnginePtr lpe_;

  /// NLP/QP Engine used to approximate the center of the feasible region
  EnginePtr nlpe_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;

  /// Status of the NLP/QP engine.
  EngineStatus nlpStatus_;

  /**
   * The variable corresponding to the objective function. It is a part of
   * all linearizations of the objective function and it appears in the
   * objective.
   */
  //VariablePtr objVar_;

  /// Nonlinearity status of objective function. 1 if nonlinear 0 otherwise.
  //bool oNl_;

  /// Parameter for root lin scheme 1
  double rs1_;
  
  /// Parameters for root lin scheme 2
  double rs2Per_;

  double rs2NbhSize_;
  
  /// Parameter for root lin scheme 3
  UInt rs3_;
  
  /// Parameter for root lin scheme 1 for general problem
  bool rgs1_;
  
  /// Parameter for root lin scheme 2 for general problem
  double rsg2Per_;
  //bool rgs2_;

  /// Value of objective in relaxation solution
  //double relobj_; 
  
  /// Absolute tolerance for constraint feasibility.
  double solAbsTol_;
  
  /// Approximation of the center of the feasible region
  double * solC_; 
 
  /// Solution of root NLP
  const double * nlpx_; 

  /// Relative tolerance for constraint feasibility.
  double solRelTol_;

  /// Absolute tolerance for pruning a node.
  double objATol_;

  /// Relative tolerance for pruning a node.
  double objRTol_;

  // populate this if any general scheme is on 
  std::vector<VariablePtr> varPtrs_; 

  /// Statistics.
  LinStats *stats_;

  public:
  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  Linearizations(EnvPtr env, RelaxationPtr rel,
                               ProblemPtr minlp, std::vector<ConstraintPtr> nlCons); 
  
  /// Destroy.
  ~Linearizations();
   
  /// Root linearization schemes
  void rootLinearizations(const double * nlpx);

  double * getCenter() {return solC_;}
  
  /// Find approximate center of the feasible region
  void findCenter();

  double maxVio(const double *x, int &index);
  /**
   * Add linearizatios by performing line search between center of the
   * feasible region and the root LP solution - root linearization scheme 3
   */
  void rootLinScheme3(EnginePtr lpe_, VariablePtr objVar,
                                    SeparationStatus *status);

  //void setLpEngine(EnginePtr lpe) {lpe_ = lpe;};
  
  void setNlpEngine(EnginePtr nlpe) {nlpe_ = nlpe;};

  /// Show statistics.
  void writeStats(std::ostream &out) const;


private:
 
  /// Add linearization in root linearization scheme 1 
  bool addCutAtRoot_(double *x, ConstraintPtr con, UInt &newConId);

  /// Add new cut in case of root linearization scheme 1
  bool addNewCut_(double *b1, ConstraintPtr con, 
                  UInt &newConId);

  void boundingVar_(double &varbound, UInt &pos, 
                                  double *lastDir,
                                  std::vector<double > &alphaSign);
  void boundingVar_(double &varbound,
                                  UInt vIdx, VariablePtr fixVar, double coeff,
                                 double fixCoeff, std::vector<int > &alphaSign, std::vector<UInt > &varIdx);

  /// Find intersection of two linearizations in root linearization scheme 1  
  bool findIntersectPt_(std::vector<UInt > newConsId, VariablePtr vl,
                       VariablePtr vnl, double * iP);

  bool findBoundaryPt_(const double *xOut, const double *xIn,
                                     double *x, 
                                     std::vector<ConstraintPtr> &vioCons);

  void setStepSize_(double &varbound, double &alpha,
                                   UInt vIdx, double val,
                                   double boundSign);

  void solveNLP_();

  void foundLinPt_(UInt vIdx, std::vector<UInt> varIdx, UInt pos, std::vector<double> alphaSign, double varBound, double *xOut, bool isAllOne);

  /**
   * Insert a new point in the candidate list for adding linearization in
   * case of root linearization scheme 1
  */
  void insertNewPt_(UInt j, UInt k, std::vector<double > & xc, 
                    std::vector<double> & yc, ConstraintPtr con, 
                    VariablePtr vl, VariablePtr vnl, bool & shouldCont);

  /// Check feasibility of sol 
  //bool isFeas_(ConstSolutionPtr sol);
 
  bool boundaryPt_(const double *xOut,
                                     std::vector<UInt > &varConsPos,
                                   std::vector<double* > & lastGrad);
  bool findLinPoint_(double *xOut, 
                                   std::vector<UInt > varConsPos,
                                   std::vector<double* > & lastGrad);


  void exploreDir_(double *xOut, std::vector<UInt > vIdx,
                                 std::vector<UInt > varConsPos, 
                                std::vector<double *> nlconsGrad,
                                std::vector<int> alpha,
                                double &vbnd);


  bool genLin_(double *x, std::vector<UInt > vioConsPos,
                                     std::vector<double *> &lastGrad);

  std::vector<UInt > isFeas_(double *x, std::vector<UInt > varConsPos,
                             bool &foundActive, bool &foundVio);

  void varCons_(std::vector<UInt > varPos, std::vector<UInt > &varConsPos);
  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf, int *error);

  /// Line search in root linearization scheme 3

  bool lineSearchPt_(const double *xIn, const double *xOut, double* x, ConstraintPtr con, double &nlpact);
   
  /// Compute variable in the linear part 
  bool linPart_(double *b1, UInt vlIdx, ConstraintPtr con, 
                  double linVarCoeff, double nVarCoeff);

  UInt minChange_(double diff1, double diff2, double &alpha,
                  double &varbound, UInt vIdx, UInt fixIdx,
                  double coeff, double fixCoeff);

  void newPoint_(bool isAllOne,
                               std::vector<UInt> varIdx, double *xOut, std::vector<double> alphaSign);

  void populateVarIdx(VariablePtr v, std::vector<UInt > &varIdx,
                                      double &vMinUb, double &vMaxLb,
                                      UInt &lIdx, UInt &uIdx);
   /**
   * Add linerizations to constraints with exactly one var in the nonlinear
   * part - root linearization scheme 1
   */
  void rootLinScheme1_(ConstraintPtr con, double linTermCoeff, UInt vlIdx,
                       UInt vnIdx, double extraCoeff);
  
  /**
   * Add linearizations in the neighborhood of the root nonlinear relaxation
   * solution - root linearization scheme 2
   */
  void rootLinScheme2_(ConstraintPtr con, 
                       double lVarCoeff,
                            UInt vlIdx, UInt vnIdx);

  /**
   * Find points with reasoanble difference in curvature to add linearizaion
   * in root linearization scheme 2
   */
  void rScheme2Cut_(ConstraintPtr con, double &delta,
                                double linTermCoeff, double &lastSlope,
                                UInt vnIdx, double * npt, double * grad);

  void rootLinGenScheme1_();
  
  void rootLinGenScheme2_();

  void search_(double varbound, UInt vIdx, double val, 
               std::vector<UInt > varIdx, 
                             double *xOut,
                             std::vector<double > &alphaSign,
                             UInt pos, bool isAllOne);


  bool shouldStop_(EngineStatus eStatus);

  bool cutAtLineSearchPt_(const double *xIn, const double *xOut, double* xNew,
                                        ConstraintPtr con);

  /// Find nonlinear constraint with only one variable in the nonlinear part
  bool uniVarNlFunc_(ConstraintPtr con, double &linTermCoeff, UInt & vlIdx,
                   UInt & vnIdx, double &extraCoeff);

  void varsInNonlinCons_();
  };

  typedef Linearizations* LinearizationsPtr;
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
