//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file UnoEngine.h
 * \brief Declare an interface to the Uno nlp-solver.
 * \author Sumanto Kar, IE & OR, Indian Institute of Technology Bombay
 *
 * This file contains declartions for interface to Uno.
 */

#ifndef MINOTAURUnoENGINE_H
#define MINOTAURUnoENGINE_H

#include "NLPEngine.h"
#include "Solution.h"
#include "Uno.hpp"
#include "WarmStart.h"
#include "ingredients/constraint_relaxation_strategy/ConstraintRelaxationStrategyFactory.hpp"
#include "ingredients/globalization_mechanism/GlobalizationMechanismFactory.hpp"
#include "model/ModelFactory.hpp"
#include "tools/Logger.hpp"
#include "tools/Options.hpp"

namespace uno
{
class UnoApplication;
class UnoFunInterface;
class Model;
}  // namespace uno

namespace Minotaur
{

class Environment;
class UnoEngine;
class Problem;
class Timer;
typedef Environment* EnvPtr;
typedef UnoEngine* UnoEnginePtr;
typedef Problem* ProblemPtr;

// thread based statistics
struct UnoStats
{
  UInt calls;       ///< Total number of calls to solve.
  UInt opt;         ///< No. of calls to Uno's Optimize
  UInt reopt;       ///< No. of calls to Uno's ReOptimize
  UInt iters;       ///< Sum of number of iterations in all calls.
  double ptime;     ///< Sum of time taken in all calls to presolve.
  UInt strCalls;    ///< Calls to solve while strong branching.
  UInt strIters;    ///< Number of iterations in strong branching alone.
  double strTime;   ///< time taken in strong branching alone.
  double time;      ///< Sum of time taken in all calls to solve.
  double walltime;  ///<
};

/// The solution obtained from/for Uno.
class UnoSolution;
typedef UnoSolution* UnoSolPtr;
typedef const UnoSolution* ConstUnoSolPtr;

class UnoSolution : public Solution
{
public:
  /// Default constructor
  UnoSolution();

  /// Construct a solution for a problem from an array of doubles.
  UnoSolution(const double* x, ProblemPtr problem);

  /**
   * Construct a solution for a problem from an array of doubles and
   * objective value.
   */
  UnoSolution(const double* x, double objval, ProblemPtr problem);

  /// Copy constructor
  UnoSolution(ConstUnoSolPtr);

  /// Destroy.
  ~UnoSolution();

  /**
   * Return a pointer to the dual associated with the lower bound on
   * variables.
   */
  const double* getLowerDualOfVars() const
  {
    return dualXLow_;
  };

  /**
   * Return a pointer to the dual associated with the upper bound on
   * variables.
   */
  const double* getUpperDualOfVars() const
  {
    return dualXUp_;
  };

  // base class
  void setDualOfVars(const double*)
  {
    assert(!"implement me!");
  };

  /// Set duals of variable-bounds: both the lower and upper.
  void setDualOfVars(const double* lower, const double* upper);

  /// Write to an output.
  void write(std::ostream& out) const;

private:
  /// dual of lower bounds.
  double* dualXLow_;

  /// dual of upper bounds.
  double* dualXUp_;
};

class UnoWarmStart;
typedef UnoWarmStart* UnoWarmStartPtr;
typedef const UnoWarmStart* ConstUnoWarmStartPtr;

/// Class for saving and using Warm-start information in Uno.
class UnoWarmStart : public WarmStart
{
public:
  /// Default constructor
  UnoWarmStart();

  /// Copy constructor. Creates a full copy, not just copies pointers.
  UnoWarmStart(ConstUnoWarmStartPtr ws);

  /// Destroy
  ~UnoWarmStart();

  /// Return the soluton that can be used as starting point.
  UnoSolPtr getPoint();

  // Implement WarmStart::hasInfo().
  bool hasInfo();

  /**
   * Overwrite the primal and dual values of warm-start. Sometimes, the
   * warm-start data is initialized and needs to be updated. This
   * should be called in place of deleting and creating a new warm-start
   * object.
   */
  void setPoint(UnoSolPtr sol);

  // Implement WarmStart::write().
  void write(std::ostream& out) const;

private:
  /// The starting solution that is used to warm-start.
  UnoSolPtr sol_;
};

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

class UnoEngine : public NLPEngine
{
public:
  friend class Problem;

  /// Default constructor
  UnoEngine(EnvPtr env);

  /// Destroy
  ~UnoEngine();

  // Base class method
  void addConstraint(ConstraintPtr c);

  void changeBound(ConstraintPtr cons, BoundType lu, double new_val);

  // Implement Engine::changeBound(VariablePtr, BoundType, double).
  void changeBound(VariablePtr var, BoundType lu, double new_val);

  // Implement Engine::changeBound(VariablePtr, double, double).
  void changeBound(VariablePtr var, double new_lb, double new_ub);

  // Implement Engine::changeConstraint().
  void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf, double lb,
                        double ub);

  // Implement Engine::changeConstraint().
  void changeConstraint(ConstraintPtr, NonlinearFunctionPtr);

  // change objective.
  void changeObj(FunctionPtr f, double cb);

  /// Clear the loaded problem, if any, from the engine.
  void clear();

  /// Restore settings after strong branching.
  void disableStrBrSetup();

  /// Return an empty UnoEngine pointer.
  EnginePtr emptyCopy();

  /// Make settings for strong branching.
  void enableStrBrSetup();

  // Implement Engine::fillStats()
  void fillStats(std::vector<double>&);

  // get name.
  std::string getName() const;

  // Implement Engine::getSolution().
  ConstSolutionPtr getSolution();

  // Implement Engine::getSolutionValue().
  double getSolutionValue();

  // Implement Engine::getStatus().
  EngineStatus getStatus();

  // Implement Engine::getWarmStart().
  ConstWarmStartPtr getWarmStart();

  // Implement Engine::getWarmStartCopy().
  WarmStartPtr getWarmStartCopy();

  /// Load the problem into Uno. We create the TNLP interface to Uno.
  void load(ProblemPtr problem);

  // Implement Engine::loadFromWarmStart().
  void loadFromWarmStart(const WarmStartPtr ws);

  // Convert 'min f' to 'min -f'.
  void negateObj();

  // base class method.
  void removeCons(std::vector<ConstraintPtr>& delcons);

  // Implement Engine::resetIterationLimit().
  void resetIterationLimit();

  // Implement Engine::setIterationLimit().
  void setIterationLimit(int limit);

  // Implement Engine::setDualObjLimit().
  int setDualObjLimit(double)
  {
    return 1;
  };

  void setOptionsForSingleSolve();

  void setOptionsForRepeatedSolve();

  /**
   * Solve the problem that was loaded. Uses the TNLP interface to Uno
   * for solving the NLP.
   */
  EngineStatus solve();

  /// Write statistics.
  void writeStats(std::ostream& out) const;

private:
  /// True if a bound on variable has been modified since last change.
  bool bndChanged_;

  /// True if a constraint has been modified since last change.
  bool consChanged_;

  /// Optimal dual values associated with x >= l.
  // double *dual_x_l_;

  /// Optimal dual values associated with x <= u.
  // double *dual_x_u_;

  /// Optimal dual values associated with l <= g(x) <= u.
  // double *dual_g_;

  /// Environment.
  EnvPtr env_;

  /// If lb>ub+etol_, declare infeasible.
  const double etol_;

  /// True right after loading a problem, changes to false after the first
  /// call to Optimize() or ReOptimize()
  // bool justLoaded_;

  /// Where to put logs.
  // LoggerPtr logger_;

  /// The maximum limit that can be set on Uno.
  static const int maxIterLimit_ = 3000;

  /// String name used in log messages.
  static const std::string me_;

  // uno::UnoApplication  *myapp_;

  // usually gets freed automatically when myapp_ is deleted.
  // In case myapp_->Optimize or myapp_->Reoptimize are
  // not called even once, then we have to free it (see
  // UnoStats::opt, reopt).
  // uno::UnoFunInterface *mynlp_;

  /**
   * True if we want to save warm start information of the current
   * solution for the next solve. False, if no information needs to be
   * saved. Saving information does not mean that it will be used.
   * useWs_ flag must be on to use the warm-start information.
   */
  // bool prepareWs_;

  /// Try to solve without calling Uno.
  // bool presolve_();

  /// Problem that is loaded, if any.
  ProblemPtr problem_;
  std::unique_ptr<uno::Uno> uno_;
  std::shared_ptr<uno::Iterate> initial_iterate;
  std::shared_ptr<uno::Model> model_;
  std::unique_ptr<uno::Model> uno_model_;
  uno::Options options;
  /**
   * The solution obtained from last solve. The sol_ in
   * UnoFunInterface points to the same array as this one. So the
   * solution from Uno is copied to this array only once. This array
   * does not get freed in UnoFunInterface and so must be freed here.
   */
  UnoSolPtr sol_;

  /// Statistics.
  // UnoStats *stats_;

  /// True if strong branching. False otherwise.
  // bool strBr_;

  /**
   * Timer for Uno solves. Includes time spent in call-backs:
   * evaluating functions and derivatives.
   */
  // Timer *timer_;

  /**
   * True if we want to use warm-start information, either from a
   * previous solve or from a user provided structure.
   */
  // bool useWs_;

  /// warm start information.
  // UnoWarmStartPtr ws_;

  /// warm start information only for strong branching.
  // UnoWarmStartPtr wsSb_;

  /// Set problem specific options to make Uno faster
  void setOptionsForProb_();
};
}  // namespace Minotaur

#endif
