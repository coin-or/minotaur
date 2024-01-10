// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2024 The Minotaur Team.
// 

/**
 * \file CplexLPEngine.h 
 * \brief Define an interface to the Cplex LP solver.
 * \author Prashant Palkar and Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURCPLEXLPENGINE_H
#define MINOTAURCPLEXLPENGINE_H
#include <ilcplex/cplexx.h>
#include <string.h>
#include "LPEngine.h"
#include "STOAHandler.h"
#include "WarmStart.h"


namespace Minotaur {

  class   Timer;
  class   Environment;
  class   Problem;
  class   Solution;
  class   WarmStart;
  typedef Environment* EnvPtr;
  typedef Problem* ProblemPtr;
  typedef Solution* SolutionPtr;
  typedef WarmStart* WarmStartPtr;

  /// Statistics
  struct CplexLPStats {
    UInt calls;     /// Total number of calls to solve.
    UInt strCalls;  /// Calls to solve while strong branching.
    double time;    /// Sum of time taken in all calls to solve.
    double strTime; /// time taken in strong branching alone.
    UInt iters;     /// Sum of number of iterations in all calls.
    UInt strIters;  /// Number of iterations in strong branching alone.
  };

  class CpxLPWarmStart;
  typedef CpxLPWarmStart* CpxLPWarmStartPtr;
  typedef const CpxLPWarmStart* ConstCpxLPWarmStartPtr;

  /// Class for saving and using Warm-start information in Cplex LP engine.
  class CpxLPWarmStart : public WarmStart {

  public:
    /// Default constructor
    CpxLPWarmStart();

    /// Destroy
    ~CpxLPWarmStart();

    /// Return the soluton that can be used as starting point.
    //CpxLPSolPtr getPoint();

    // Implement WarmStart::hasInfo().
    bool hasInfo();

    /*
     Overwrite the primal and dual values of warm-start. Sometimes, the
     warm-start data is initialized and needs to be updated. This
     should be called in place of deleting and creating a new warm-start
     object. */
    //void setPoint(CpxLPSolPtr sol);

    // Implement WarmStart::write().
    void write(std::ostream &) const {};

    // Return varstat array from warmstart
    //int *getVarStat() const { return varstat_;}

    // Return constat array from warmstart
    //int *getConStat() const { return constat_;}

    // Return the size of constat_ (number of constraints)
    //int getNumCons() const { return numCons_;}

    // Return solution from warmstart
    SolutionPtr getSolution() const { return sol_;}

    // Return the size of constat_ (number of constraints)
    //void setNumCons(int num) { numCons_ = num;}

    // Set varstat array for warmstart
    //void setVarStat(int *varstat, int numvars);

    // Set constat array for warmstart
    //void setConStat(int *constat, int numrows);

  private:
    /**
     * If true, we must delete the warm-start description. If it is false,
     * we should never delete it.
     */
    bool mustDelete_;

    /// The starting solution that is used to warm-start.
    SolutionPtr sol_;

    /// An array to store the constraint warmstart info
    //int * constat_;

    /// An array to store the variable warmstart info
    //int * varstat_;

    /// Size of constat_
    int numCons_;
  };


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


  /// The CplexLPEngine class can be called to solve LP problems
  class CplexLPEngine : public LPEngine {
  public:
    
    //static int lazycallback(CPXCENVptr env, void *cbdata, int wherefrom,
             //void *cbhandle, int *useraction_p);
    
    void doNothing();
    
    /// Constructor with an environment.
    CplexLPEngine(EnvPtr env);

    /// Destroy. 
    ~CplexLPEngine();

    // Implement Engine::addConstraint().
    void addConstraint(ConstraintPtr);

    // Change bounds on a constraint.
    void changeBound(ConstraintPtr cons, BoundType lu, double new_val);

    // Implement Engine::changeBound(VariablePtr, BoundType, double).
    void changeBound(VariablePtr var, BoundType lu, double new_val);

    // Implement Engine::changeBound(VariablePtr, double, double).
    void changeBound(VariablePtr var, double new_lb, double new_ub);

    // Implement Engine::changeConstraint().
    void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf, 
                          double lb, double ub);

    // Implement Engine::changeConstraint().
    void changeConstraint(ConstraintPtr c, NonlinearFunctionPtr nlf);

    // change the objective function.
    void changeObj(FunctionPtr f, double cb);

    /// Clear the problem.
    void clear();

    void disableStrBrSetup();

    /// Return an empty CplexLPEngine pointer.
    EnginePtr emptyCopy();

    void enableStrBrSetup();

    // Implement Engine::fillStats()
    void fillStats(std::vector<double> &);

    /// Return the solution value of the objective after solving the LP.
    double getSolutionValue();

    // Implement Engine::getSolution().
    ConstSolutionPtr getSolution();

    // Implement Engine::getStatus().
    EngineStatus getStatus();

    // Return name of the solver.
    std::string getName() const;

    ConstWarmStartPtr getWarmStart() {return WarmStartPtr();};

    // Implement Engine::getWarmStartCopy().
    WarmStartPtr getWarmStartCopy();

    /** 
     * Load the problem into the engine. We create arrays of variables and
     * constraints, the A matrix, rhs, objective etc from the problem and
     * initialize the LP solver.
     */
    void load(ProblemPtr problem);

    void loadFromWarmStart(const WarmStartPtr){};

    // Convert 'min f' to 'min -f'.
    void negateObj();

    // Print a point x
    void printx(double *, UInt );

    // Print a point x
    void printx(const int *, UInt );
    
    // base class method.
    void removeCons(std::vector<ConstraintPtr> &delcons);

    // Implement Engine::resetIterationLimit().
    void resetIterationLimit();

    // Implement Engine::setIterationLimit().
    void setIterationLimit(int);

    // Implement Engine::setDualObjLimit().
    int setDualObjLimit(double);

    // Implement Engine::setTimeLimit().
    void setTimeLimit(double);

    // Implement the LP solve() function of Cplex
    EngineStatus solve();

    /// Writes an LP file of the loaded problem.
    void writeLP(const char *filename) const;

    /// Writes an LP file in the directory of problem file.
    void writeLP();

    /// Write statistics.
    void writeStats(std::ostream &out) const;

  private:

    /// Cplex Environment
    CPXENVptr cpxenv_;

    /// Cplex Problem pointer
    CPXLPptr cpxlp_;
   
    /// Cplex Status
    int cpxstatus_;

    /// Time limit for the iteration
    double timeLimit_;

    /// True if any bound on variable on constraint was changed after 
    /// previous solve.
    bool bndChanged_;

    /// True if a constraint (not it bound) was changed after previous solve.
    bool consChanged_;

    /// Environment.
    EnvPtr env_;

    /// The maximum limit that can be set on the solver. 
    int maxIterLimit_;

    /// String name used in log messages.
    static const std::string me_;

    /// True if objective was changed after previous solve.
    bool objChanged_;

    /// True if objective was changed after previous solve.
    double dualObjLimit_;

    /// Problem that is loaded, if any.
    ProblemPtr problem_;

    /// Solution.
    SolutionPtr sol_;

    /// Statistics.
    CplexLPStats *stats_;

    /// True if strong-branching.
    bool strBr_;

    /// Warm start.
    CpxLPWarmStartPtr ws_;

    /// Timer for solves. 
    Timer *timer_;

    /// Actually load the problem. Called before _every_ solve.
    void load_();
  };
  
  typedef CplexLPEngine* CplexLPEnginePtr;
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
