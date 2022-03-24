//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file SimplexQuadCutGen.h
 * \brief Define class for generating cuts for quadratic constraints from
 * the simplex tableau
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 */

#ifndef MINOTAURSIMPLEXQUADCUTGEN_H
#define MINOTAURSIMPLEXQUADCUTGEN_H

#include "Constraint.h"
#include "Environment.h"
#include "LPEngine.h"
#include "Problem.h"
#include "Types.h"

namespace Minotaur {
class Relaxation;
typedef Relaxation *RelaxationPtr;
/** \brief A structure to store info from the simplex tableau
 */
struct TableauInfo {
  int ncol;                // Number of columns in the simplex
  int nrow;                // Number of rows in the simplex
  const double *colLower;  // Lower bounds of the variables
  const double *colUpper;  // Upper bounds of the variables
  const double *rowLower;  // Lower bounds of the constraints
  const double *rowUpper;  // Upper bounds of the constraints
  const double *rowRhs;    // Right hand side of constraints
  const double *origTab;   // Original simplex tableau
  const int *rowStart;     // Vector of row starts
  const int *indices;      // Vector of indices
  const int *rowLen;       // Vector of row lengths
};

struct SimplexCut {
  LinearFunctionPtr lf;  // The linear function of the cut
  double lb;             // Lower Bound
  double ub;             // Upper Bound
  UInt numInactive;      // Number of times the cut has remained inactive
};

typedef std::pair<int, int> VarProd;
typedef std::map<VarProd, double> QuadTerm;
typedef std::map<int, std::pair<double, double>> SlackBound;
typedef SimplexCut *SimplexCutPtr;
typedef std::vector<SimplexCutPtr> SimplexCutVector;

class SimplexQuadCutGen {
 public:
  // Default Constructor
  SimplexQuadCutGen();

  // Constructor
  SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe);

  // Destructor
  ~SimplexQuadCutGen();

  // Generate the cuts that violate the given point
  int generateCuts(RelaxationPtr rel, const double *x);

 private:
  // Environment pointer
  EnvPtr env_;

  // Feasibility Tolerance
  double eTol_;

  // Problem for which the cuts are generated
  ProblemPtr p_;

  // LP Engine to access the Simplex tableau
  LPEnginePtr lpe_;

  // For printing msgs
  static const std::string me_;

  // Number of cuts generated
  UInt ncuts_;

  // Number of iterations of cut generation
  int iter_;

  // All the cuts generated
  SimplexCutVector allCuts_;

  // Basic Tableau Info
  TableauInfo *tabInfo_;

  // Row index of the basic original variables in the tableau
  // Key - index of the original variable which is basic
  // Value - Row index of the corresponding variable
  std::map<int, int> basicInd_;

  // A vector of indices of non-basic original variables
  std::vector<int> nbOrig_;

  // Number of non-basic original variables
  int nnbOrig_;

  // A vector of indices of non-basic slack variables
  std::vector<int> nbSlack_;

  // Number of non-basic slack variables
  int nnbSlack_;

  // Lower and Upper bounds of the slack variables
  SlackBound sb_;

  // \brief Add the generated cuts to the relaxation
  // \param[in] cuts - The vector of cuts to be added to relaxation
  // \param[in] rel - Relaxation to which the cuts needs to be added
  void addCutsToRel_(SimplexCutVector cuts, RelaxationPtr rel);

  // \brief Add relaxed term in the cut lf for a square original term
  // \param[in] cuts - The vector of cuts.
  // \param[in] v - The variable of the squared term
  // \param[in] coef - The coefficient of the term
  // \param[in] cnst - The constant of the relaxation of the square term
  void addLfTerm_(SimplexCutVector cuts, VariablePtr v, double coef,
                  double cnst);

  // \brief Add relaxed term in the cut lf for a square slack term
  // \param[in] cuts - The vector of cuts.
  // \param[in] rel - The relaxation of the problem
  // \param[in] slackInd - The variable of the squared term
  // \param[in] coef - The coefficient of the term
  // \param[in] cnst - The constant of the relaxation of the square term
  void addLfTerm_(SimplexCutVector cuts, RelaxationPtr rel, int slackInd,
                  double coef, double cnst);

  // \brief Add one relaxed term in the cut lf for a bilinear original term
  // \param[in] cuts - The vector of cuts.
  // \param[in] v1 - The first variable of the bilinear term
  // \param[in] v2 - The second variable of the bilinear term
  // \param[in] coef1 - The coefficient of v1
  // \param[in] coef2 - The coefficient of v2
  // \param[in] cnst - The constant of the relaxation of the bilinear term
  void addLfTerm_(SimplexCutVector cuts, VariablePtr v1, VariablePtr v2,
                  double coef1, double coef2, double cnst);

  // \brief Add one relaxed term in the cut lf for a bilinear slack term
  // \param[in] cuts - The vector of cuts.
  // \param[in] rel - The relaxation of the problem
  // \param[in] slackInd1 - The first variable of the bilinear term
  // \param[in] slackInd2 - The second variable of the bilinear term
  // \param[in] coef1 - The coefficient of slack1
  // \param[in] coef2 - The coefficient of slack2
  // \param[in] cnst - The constant of the relaxation of the bilinear term
  void addLfTerm_(SimplexCutVector cuts, RelaxationPtr rel, int slackInd1,
                  int slackInd2, double coef1, double coef2, double cnst);

  // \brief Add one relaxed term in the cut lf for a product of orig and slack
  // \param[in] cuts - The vector of cuts.
  // \param[in] rel - The relaxation of the problem
  // \param[in] orig_v - The original variable of the product
  // \param[in] slackInd2 - The slack variable of the product
  // \param[in] coef1 - The coefficient of orig_v
  // \param[in] coef2 - The coefficient of slack
  // \param[in] cnst - The constant of the relaxation of the product
  void addLfTerm_(SimplexCutVector cuts, RelaxationPtr rel, VariablePtr orig_v,
                  int slackInd, double coef1, double coef2, double cnst);

  // \brief Add two relaxed term in the cut lf for a bilinear original term
  // \param[out] cuts - The vector of cuts.
  // \param[in] v1 - The first variable of the bilinear term
  // \param[in] v2 - The second variable of the bilinear term
  // \param[in] c1v1 - The coefficient of v1 in the first term
  // \param[in] c1v2 - The coefficient of v2 in the first term
  // \param[in] c2v1 - The coefficient of v1 in the second term
  // \param[in] c2v2 - The coefficient of v2 in the second term
  // \param[in] cnst1 - The constant of the relaxation for the first term
  // \param[in] cnst2 - The constant of the relaxation for the second term
  void addLfTerm_(SimplexCutVector &cuts, VariablePtr v1, VariablePtr v2,
                  double c1v1, double c1v2, double c2v1, double c2v2,
                  double cnst1, double cnst2);

  // \brief Add two relaxed term in the cut lf for a product of orig and slack
  // \param[out] cuts - The vector of cuts.
  // \param[in] orig_v - The original variable of the product
  // \param[in] slackInd - The slack variable of the product
  // \param[in] c1v1 - The coefficient of orig_v in the first term
  // \param[in] c1v2 - The coefficient of slack in the first term
  // \param[in] c2v1 - The coefficient of orig_v in the second term
  // \param[in] c2v2 - The coefficient of slack in the second term
  // \param[in] cnst1 - The constant of the relaxation for the first term
  // \param[in] cnst2 - The constant of the relaxation for the second term
  void addLfTerm_(SimplexCutVector &cuts, RelaxationPtr rel, VariablePtr orig_v,
                  int slackInd, double c1v1, double c1v2, double c2v1,
                  double c2v2, double cnst1, double cnst2);

  // \brief Add two relaxed term in the cut lf for a bilinear slack term
  // \param[out] cuts - The vector of cuts.
  // \param[in] slackInd1 - The first variable of the bilinear term
  // \param[in] slackInd2 - The second variable of the bilinear term
  // \param[in] c1v1 - The coefficient of v1 in the first term
  // \param[in] c1v2 - The coefficient of v2 in the first term
  // \param[in] c2v1 - The coefficient of v1 in the second term
  // \param[in] c2v2 - The coefficient of v2 in the second term
  // \param[in] cnst1 - The constant of the relaxation for the first term
  // \param[in] cnst2 - The constant of the relaxation for the second term
  void addLfTerm_(SimplexCutVector &cuts, RelaxationPtr rel, int slackInd1,
                  int slackInd2, double c1v1, double c1v2, double c2v1,
                  double c2v2, double cnst1, double cnst2);

  // \brief Add a linear term for the substituted quadratic
  // \param[out] t - The map in which the term is to be added
  // \param[in] v - The variable
  // \param[in] coef - The coefficient of the variable
  void addTerm_(std::map<int, double> &t, int v, double coef);

  // \brief Add a quadratic term for the substituted quadratic
  // \param[out] t - The map in which the term is to be added
  // \param[in] vp - The product of variables which are to be added
  // \param[in] coef - The coefficient of the product
  void addTerm_(QuadTerm &t, VarProd vp, double coef);

  // Delete tabInfo_. Called in the destructor.
  void delTabInfo_();

  // get basic info of the simplex tableau from the lp solver
  void getBasicInfo_();

  // \brief Get the initial cut vector for the current iteration
  // \param[in] rel - The relaxation of the problem
  // \param[in] cutCoefo - The original linear terms in the substituted
  // quadratic \param[in] cutCoefs - The slack linear terms in the substituted
  // quadratic \param[in] cutConst - The constant in the substituted quadratic
  // \param[in] under - true if we need to underestimate the quad; false o/w
  // \param[in] rhs - rhs of the constraint for which the substitution is done
  SimplexCutVector getCutVec_(RelaxationPtr rel, std::map<int, double> cutCoefo,
                              std::map<int, double> cutCoefs, double cutConst,
                              bool under, double rhs);

  // \brief Get the second cut for product of original variables
  SimplexCutPtr getNewCut_(SimplexCutPtr cut, VariablePtr v1, VariablePtr v2,
                           double coef1, double coef2, double cnst);

  // \brief Get the second cut for product of original and slack variables
  SimplexCutPtr getNewCut_(SimplexCutPtr cut, RelaxationPtr rel,
                           VariablePtr orig_v, int slackInd, double coef1,
                           double coef2, double cnst);

  // \brief Get the second cut for product of slack variables
  SimplexCutPtr getNewCut_(SimplexCutPtr cut, RelaxationPtr rel, int slackInd1,
                           int slackInd2, double coef1, double coef2,
                           double cnst);

  // get the bounds on the slack variables
  void getSlackBounds_();

  // get bounds on the linear terms. Called for calculating slack bounds.
  void getTermBounds_(double coef, double vlb, double vub, double &lb,
                      double &ub);

  // \brief multiply two basic variables coef*b1*b2
  // \param[in] b1 - index of the first basic variable
  // \param[in] b2 - index of the second basic variable
  // \param[in] coef - Coefficient for the term
  // \param[in] x - current lp point
  // \param[out] oxo - the product map for original times original variables
  // \param[out] oxs - the product map for original times slack variables
  // \param[out] sxs - the product map for slack times slack variables
  // \param[out] cutCoefo - the linear terms for original variables
  // \param[out] cutCoefs - the linear terms for slack variables
  // \param[out] cutConst - the constant of the substituted quadratic
  void multiplyBB_(int b1, int b2, double coef, const double *x, QuadTerm &oxo,
                   QuadTerm &oxs, QuadTerm &sxs,
                   std::map<int, double> &cutCoefo,
                   std::map<int, double> &cutCoefs, double &cutConst);

  // \brief multiply a basic and a non-basic variable coef*b*nb
  // \param[in] b - index of the basic variable
  // \param[in] nb - index of the non-basic variable
  // \param[in] coef - Coefficient for the term
  // \param[in] x - current lp point
  // \param[out] oxo - the product map for original times original variables
  // \param[out] oxs - the product map for original times slack variables
  // \param[out] cutCoefo - the linear terms for original variables
  void multiplyBNB_(int b, int nb, double coef, const double *x, QuadTerm &oxo,
                    QuadTerm &oxs, std::map<int, double> &cutCoefo);

  // \brief multiply a coefficient to a basic variable (linear term)
  // \param[in] b - index of the basic variable
  // \param[in] coef - Coefficient for the term
  // \param[in] x - current lp point
  // \param[out] cutCoefo - the linear terms for original variables
  // \param[out] cutCoefs - the linear terms for slack variables
  // \param[out] cutConst - the constant of the substituted quadratic
  void multiplyCB_(int b, double coef, const double *x,
                   std::map<int, double> &cutCoefo,
                   std::map<int, double> &cutCoefs, double &cutConst);

  // \brief multiply a coefficient to a non-basic variable (linear term)
  // \param[in] nb - index of the non-basic variable
  // \param[in] coef - Coefficient for the term
  // \param[out] cutCoefo - the linear terms for original variables
  void multiplyCNB_(int nb, double coef, std::map<int, double> &cutCoefo);

  // \brief multiply two non-basic variables coef*nb1*nb2
  // \param[in] nb1 - index of the first non-basic variable
  // \param[in] nb2 - index of the second non-basic variable
  // \param[in] coef - Coefficient for the term
  // \param[out] oxo - the product map for original times original variables
  void multiplyNBNB_(int nb1, int nb2, double coef, QuadTerm &oxo);

  int relaxBilTerm_(double coef, bool lower1, bool lower2, double l1, double u1,
                    double l2, double u2, double &c1v1, double &c1v2,
                    double &c2v1, double &c2v2, double &cnst1, double &cnst2,
                    bool under);

  int relaxQuadTerms_(SimplexCutVector &cuts, RelaxationPtr rel, QuadTerm oxo,
                      QuadTerm oxs, QuadTerm sxs,
                      std::map<int, double> cutCoefo,
                      std::map<int, double> cutCoefs, double cutConst,
                      const double *x, bool under, double rhs);

  void relaxSqTerm_(double coef, bool atLower, double l, double u,
                    double &lincoef, double &cnst, bool under);

  void slackSubstitute_(int slackInd, double coef, std::map<int, double> &row,
                        double &rhs);

  void sortVariables_();
};
typedef SimplexQuadCutGen *SimplexQuadCutGenPtr;
}  // namespace Minotaur
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
