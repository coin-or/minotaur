// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file AnalyticalCenter.h
 * \Briefly declare a class for finding analytical center of the feasible region.
 * \Author Meenarli Sharma, Indian Institute of Techonology Bombay.
 */

#ifndef MINOTAURANALYTICALCENTER_H
#define MINOTAURANALYTICALCENTER_H

#include <stack>

#include "Engine.h"
#include "Logger.h"
#include "Problem.h"
#include "Timer.h"

#include "Solution.h"

namespace Minotaur {

class AnalyticalCenter {

private: 
  /// Pointer to environment.
  EnvPtr env_;

  /// Pointer to original problem.
  ProblemPtr minlp_;
  
  Timer *timer_;

  /// Log.
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// NLP/QP Engine used to approximate the center of the feasible region
  EnginePtr nlpe_;

  /// Absolute tolerance for constraint feasibility.
  double solAbsTol_;
  
  public:
  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  AnalyticalCenter(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe);
 
  /// Destroy.
  ~AnalyticalCenter();

  // Find center by adding auxiliary variable only to the nonlinear constraints
  // and keeping linear constraints as they are.
  void modifyOnlyNonlinear(double * &solC);

  // Find center by adding auxiliary variable to all inequalties, including
  // variable bounds.
  void modifyWhole(double * &solC);

private:
  void solveNLP_(double * &solC);

  };

  typedef AnalyticalCenter* AnalyticalCenterPtr;
}
#endif

