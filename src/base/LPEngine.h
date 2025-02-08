//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file LPEngine.h
 * \author Ashutosh Mahajan, Argonne National Laboratory.
 * \brief Declare the class LPEngine for solving LPs and getting solution.
 */

#ifndef MINOTAURLPENGINE_H
#define MINOTAURLPENGINE_H

#include "Engine.h"

namespace Minotaur {

/**
 * The LPEengine class is an abstract class for interfacing LP solvers (like
 * OsiLPEngine). A derived class must implement calls to the LP solver for
 * the methods described here.
 *
 * \todo  add more methods for accessing simplex tableaux.
 * \todo  add more methods for accessing dual rays etc.
 */

class LPEngine : public Engine {
 public:
  friend class Problem;

  /// Constructor. May set default parameters/options here.
  LPEngine(){};

  /// Destructor must be implemented if memory needs to be freed
  virtual ~LPEngine(){};

  virtual void enableFactorization(){};

  virtual void disableFactorization(){};

  virtual bool IsOptimalBasisAvailable() { return true; };

  virtual void getBasics(int*){};

  virtual void getBInvARow(int, double*, double*){};

  virtual int getNumCols() { return 0; };

  virtual int getNumRows() { return 0; };

  virtual const double* getColLower() { return 0; };

  virtual const double* getColUpper() { return 0; };

  virtual const double* getRowLower() { return 0; };

  virtual const double* getRowUpper() { return 0; };

  virtual const double* getRightHandSide() { return 0; };

  virtual const double* getRowActivity() { return 0; };

  virtual const double* getOriginalTableau() { return 0; };

  virtual const int* getRowStarts() { return 0; };

  virtual const int* getIndicesofVars() { return 0; };

  virtual const int* getRowLength() { return 0; };

  virtual void loadDualWarmStart(int, double*){};
};
typedef LPEngine* LPEnginePtr;
}  // namespace Minotaur

#endif
