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

#include "Cut.h"
#include "Environment.h"
#include "LPEngine.h"
#include "Problem.h"
#include "Types.h"

namespace Minotaur {
/** \brief A structure to store info from the simplex tableau
 */
struct TableauInfo {
  int ncol;                   // Number of columns in the simplex
  int nrow;                   // Number of rows in the simplex
  const double *colLower;     // Lower bounds of the variables
  const double *colUpper;     // Upper bounds of the variables
  const double *rowLower;     // Lower bounds of the constraints
  const double *rowUpper;     // Upper bounds of the constraints
  const double *rowRhs;       // Right hand side of constraints
  const double *rowActivity;  // Constraints Activity
  const double *origTab;      // Original simplex tableau
  const int *rowStart;        // Vector of row starts
  const int *indices;         // Vector of indices
  const int *rowLen;          // Vector of row lengths
}

typedef std::pair<int, int>
    VarProd;
typedef std::map<VarProd, double> QuadTerm;
typedef std::map<int, std::pair<double, double>> SlackBound;

class SimplexQuadCutGen {
 public:
  // Default Constructor
  SimplexQuadCutGen();

  // Constructor
  SimplexQuadCutGen(EnvPtr env, ProblemPtr p, LPEnginePtr lpe);

  // Destructor
  ~SimplexQuadCutGen();

  // Generate the cuts that violate the given point
  CutVector generateCuts(RelaxationPtr rel, const double *x);

 private:
  // Environment pointer
  EnvPtr env_;

  // Feasibility Tolerance
  double eTol_;

  // Problem for which the cuts are generated
  ProblemPtr p_;

  // LP Engine to access the Simplex tableau
  LPEnginePtr lpe_;

  // Number of cuts generated
  UInt ncuts_;

  // Nonlinear Constraints in the problem
  ConstraintVector qcons_;

  // Basic Tableau Info
  TableauInfo tabInfo_;

  // Row index of the basic original variables in the tableau
  // Key - index of the original variable which is basic
  // Value - Row index of the corresponding variable
  std::map<int, int> basicInd_;

  // An array of indices of non-basic original variables
  int *nbOrig_;

  // Number of non-basic original variables
  int nnbOrig_;

  // An array of indices of non-basic slack variables
  int *nbSlack_;

  // Number of non-basic slack variables
  int nnbSlack_;

  // Lower and Upper bounds of the slack variables
  SlackBound sb_;

  // Finds the non basic variables indices
  fillNonBasicIndex_(RelaxationPtr rel, int &basic,
                     std::map<UInt, UInt> &nb_index);

  // Save the nonlinear constraints of the problem
  void findQuadCons_();
}
}  // namespace Minotaur
