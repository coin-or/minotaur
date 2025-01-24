//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
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

namespace Minotaur
{
class Relaxation;
class Timer;
typedef Relaxation* RelaxationPtr;
/** \brief A structure to store info from the simplex tableau
 */
struct TableauInfo {
  int ncol;               // Number of columns in the simplex
  int nrow;               // Number of rows in the simplex
  const double* colLower; // Lower bounds of the variables
  const double* colUpper; // Upper bounds of the variables
  const double* rowLower; // Lower bounds of the constraints
  const double* rowUpper; // Upper bounds of the constraints
  const double* rowRhs;   // Right hand side of constraints
  const double* origTab;  // Original simplex tableau
  const int* rowStart;    // Vector of row starts
  const int* indices;     // Vector of indices
  const int* rowLen;      // Vector of row lengths
};

struct CutStats {
  UInt gencuts;   // Number of cuts generated
  UInt cutsadded; // Number of cuts added
  UInt numrounds; // Number of rounds of cutting
  double time;    // Time taken in cut generation
};

struct SimplexCut {
  double* coef; // The linear function of the cut
  double lb;    // Lower Bound
  double ub;    // Upper Bound
  double depth; // Depth of cut
  SimplexCut() { }
  ~SimplexCut()
  {
    if(coef) {
      delete[] coef;
    }
  }
  bool operator<(const SimplexCut& cut) const
  {
    return depth < cut.depth;
  }
  bool operator>(const SimplexCut& cut) const
  {
    return depth > cut.depth;
  }
};

typedef std::map<int, std::pair<double, double>> SlackBound;
typedef SimplexCut* SimplexCutPtr;
typedef std::vector<SimplexCutPtr> SimplexCutVector;

class SimplexQuadCutGen
{
public:
  // Default Constructor
  SimplexQuadCutGen();

  // Constructor
  SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe, double ub);

  // Destructor
  ~SimplexQuadCutGen();

  // disable factorization
  void disableFactorization();

  // Check whether cutting needs to be done or not
  bool doCutting(double curlb);

  // Generate the cuts that violate the given point
  int generateCuts(RelaxationPtr rel, ConstSolutionPtr sol,
                   ConstraintVector& added);

  // get preprocessing info from the simplex tableau
  void preprocessSimplexTab();

  void writeStats(std::ostream& out) const;

private:
  // Average sparsity of the relaxation. This is computed as the total number of
  // nonzeros in the coefficient matrix divided by the number of constraints.
  double avgSparsity_;

  // Bounds after each round of cutting, will have max size of nrounds_
  std::vector<int> bounds_;

  // Environment pointer
  EnvPtr env_;

  // Feasibility Tolerance
  double eTol_;

  // Problem for which the cuts are generated
  ProblemPtr p_;

  // LP Engine to access the Simplex tableau
  LPEnginePtr lpe_;

  // max number of terms allowed in the cut
  UInt maxTerms_;

  // For printing msgs
  static const std::string me_;

  // Number of cuts generated
  UInt ncuts_;

  // Number of rounds after which improvement is checked
  UInt nrounds_;

  // Maximum cuts to add during an iteration
  UInt maxCuts_;

  // Maximum rounds after which not cutting will be done
  UInt maxrounds_;

  // Minimum allowed depth of cut for a cut to be added
  double minDepth_;

  // Minimum change in the bound for more cuts to be added
  double minChangeFrac_;

  // Cut statistics
  CutStats stats_;

  // To keep track of time
  const Timer* timer_;

  // Basic Tableau Info
  TableauInfo* tabInfo_;

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

  // Upper bound at the current node
  double ub_;

  // variant we are solving
  int variant_;

  // \brief Add the generated cuts to the relaxation
  // \param[in] cuts - The vector of cuts to be added to relaxation
  // \param[in] rel - Relaxation to which the cuts needs to be added
  // \param[in] x - The current LP solution
  void addCutsToRel_(SimplexCutVector cuts, RelaxationPtr rel, const double* x,
                     UInt& ncuts, ConstraintVector& added);

  // Calculate the depth of cut from the current point
  void calcDepth_(SimplexCutVector cuts, const double* x);

  // Delete tabInfo_. Called in the destructor.
  void delTabInfo_();

  // get basic info of the simplex tableau from the lp solver
  void getBasicInfo_();

  double getMin_(SimplexCutPtr cut, double rhs);

  // get the bounds on the slack variables
  void getSlackBounds_();

  // get bounds on the linear terms. Called for calculating slack bounds.
  void getTermBounds_(double coef, double vlb, double vub, double& lb,
                      double& ub);

  void relaxConsBNB_(SimplexCutVector& cuts, ConstraintPtr c, const double* x,
                     RelaxationPtr rel);

  int relaxTermBNB_(double coef, bool lower, double bl, double bu, double nl,
                    double nu, double& cb, double& cn, double& cnst,
                    bool under);

  void slackSubstitute_(int slackInd, double coef, std::map<int, double>& row,
                        double& rhs);

  void substituteAndRelax_(RelaxationPtr rel, const double* x,
                           VariablePtr bkeep, VariablePtr bsubst, double beta,
                           double* coefs, double& cutConst, bool under);

  void sortVariables_();
};
typedef SimplexQuadCutGen* SimplexQuadCutGenPtr;
} // namespace Minotaur
#endif

