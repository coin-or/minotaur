// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2013 The MINOTAUR Team.
// 

/**
 * \file OsiLPEngine.h 
 * \brief Define an interface to the OSI-LP solver.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAUROSILPENGINE_H
#define MINOTAUROSILPENGINE_H

#include "LPEngine.h"
#include "WarmStart.h"

class CoinWarmStart;
class OsiSolverInterface;


namespace Minotaur {

  class   Timer;
  class   Environment;
  class   Problem;
  class   Solution;
  class   WarmStart;
  typedef boost::shared_ptr<Environment> EnvPtr;
  typedef boost::shared_ptr<Problem> ProblemPtr;
  typedef boost::shared_ptr<Solution> SolutionPtr;
  typedef boost::shared_ptr<WarmStart> WarmStartPtr;

  /// Statistics
  struct OsiLPStats {
    UInt calls;     /// Total number of calls to solve.
    UInt strCalls;  /// Calls to solve while strong branching.
    Double time;    /// Sum of time taken in all calls to solve.
    Double strTime; /// time taken in strong branching alone.
    UInt iters;     /// Sum of number of iterations in all calls. 
    UInt strIters;  /// Number of iterations in strong branching alone.
  };

  typedef enum {
    OsiClpEngine,
    OsiCpxEngine,
    OsiGlpkEngine,
    OsiGrbEngine,
    OsiMskEngine,
    OsiSpxEngine,
    OsiXprEngine,
    OsiUndefEngine
  } OsiLPEngineName;

  /// Actual implementation of warm start for OsiLP Engine.
  class OsiLPWarmStart : public WarmStart {
  public:

    friend class Problem;

    /// Default constructor. 
    OsiLPWarmStart();

    /// Destroy.
    virtual ~OsiLPWarmStart();

    // Implement Engine::hasInfo().
    virtual bool hasInfo();

    // Implement Engine::write().
    virtual void write(std::ostream &out) const;

    /** 
     * Save the given coin-warm start. If must_delete is true, it is our
     * responsibility to free it.
     */
    virtual void setCoinWarmStart(CoinWarmStart *coin_ws, Bool must_delete);

    /// Get the warm-start description.
    virtual CoinWarmStart * getCoinWarmStart() const;

  private:
    /** 
     * COIN provides the warm start basis. For now, we don't need our own
     * data structures.
     */
    CoinWarmStart *coinWs_;

    /** 
     * If true, we must delete the warm-start description. If it is false,
     * we should never delete it.
     */
    Bool mustDelete_;

  };
  typedef boost::shared_ptr<OsiLPWarmStart>OsiLPWarmStartPtr;
  typedef boost::shared_ptr<const OsiLPWarmStart>ConstOsiLPWarmStartPtr;


  /// The OsiLPEngine engine uses the OSI interface to CLP solver.
  class OsiLPEngine : public LPEngine {

  public:
    /// Default constructor.
    OsiLPEngine();    

    /// Constructor with an environment.
    OsiLPEngine(EnvPtr env);

    /// Destroy. 
    ~OsiLPEngine();

    /// Return an empty OsiLPEngine pointer.
    EnginePtr emptyCopy();

    /** 
     * Load the problem into the engine. We create arrays of variables and
     * constraints, the A matrix, rhs, objective etc from the problem and
     * initialize the LP solver.
     */
    void load(ProblemPtr problem);

    /** 
     * Clear the problem.
     */
    void clear();

    /** 
     * Solve the problem that was loaded. Calls resolve() function of Osi.
     * The resolve() function ``smartly'' decides what method of clp should
     * be called.
     */
    EngineStatus solve();

    /// Return the solution value of the objective after solving the LP.
    Double getSolutionValue();

    // Implement Engine::getSolution().
    ConstSolutionPtr getSolution();

    // Implement Engine::getStatus().
    EngineStatus getStatus();

    /** 
     * Return the number of iterations it took to solve the last problem.
     */
    Int getIterationCount();

    // Change bounds on a constraint.
    void changeBound(ConstraintPtr cons, BoundType lu, Double new_val);

    // Implement Engine::changeBound(VariablePtr, BoundType, Double).
    void changeBound(VariablePtr var, BoundType lu, Double new_val);

    // Implement Engine::changeBound(VariablePtr, Double, Double).
    void changeBound(VariablePtr var, Double new_lb, Double new_ub);

    // change the objective function.
    void changeObj(FunctionPtr f, Double cb);

    // Convert 'min f' to 'min -f'.
    void negateObj();

    // Create a new solver (cplex, or clp or ..)
    OsiSolverInterface *newSolver_(OsiLPEngineName ename);

    // Implement Engine::addConstraint().
    virtual void addConstraint(ConstraintPtr);

    // Implement Engine::changeConstraint().
    void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf, 
                          Double lb, Double ub);

    // Implement Engine::getWarmStart().
    // See OsiLPSolver.hpp to see Osi's description of warm-start pointer.
    // It differs from solver to solver. We just return NULL for now.
    virtual ConstWarmStartPtr getWarmStart() 
    {assert(!"not implemented!"); return WarmStartPtr();};

    // Implement Engine::getWarmStartCopy().
    virtual WarmStartPtr getWarmStartCopy();

    // Implement Engine::loadFromWarmStart().
    virtual void loadFromWarmStart(const WarmStartPtr ws);

    // Implement Engine::setIterationLimit().
    virtual void setIterationLimit(Int limit);

    // base class method.
    virtual void removeCons(std::vector<ConstraintPtr> &delcons);

    // Implement Engine::resetIterationLimit().
    virtual void resetIterationLimit();

    // Implement Engine::enableStrBrSetup()
    void enableStrBrSetup();

    // Implement Engine::disableStrBrSetup()
    void disableStrBrSetup();

    /// Writes an LP file of the loaded LP.
    void writeLP(const char *filename) const;

    /// Return the osilp interface. For hacks.
    OsiSolverInterface * getSolver();

    /// Write statistics.
    void writeStats();

    // get name.
    std::string getName() const;

  private:
    /// True if any bound on variable on constraint was changed after 
    /// previous solve.
    Bool bndChanged_;

    /// True if a constraint (not it bound) was changed after previous solve.
    Bool consChanged_;

    /// Environment.
    EnvPtr env_;

    /// Name of the engine: OsiCpx, OsiClp etc.
    OsiLPEngineName eName_;

    /// True if objective was changed after previous solve.
    Bool objChanged_;

    /** 
     * Pointer to the OSI solver interface that will actually be called to
     * solve the instance.
     */
    OsiSolverInterface *osilp_;

    /// True if strong-branching.
    Bool strBr_;

    /// Timer for OsiLP solves. Includes time spent in strong branching.
    Timer *timer_;

    /// Statistics.
    OsiLPStats *stats_;

    /// Solution.
    SolutionPtr sol_;

    /// Problem that is loaded, if any.
    ProblemPtr problem_;

    /// The maximum limit that can be set on Osi solver. 
    static const Int maxIterLimit_ = 9999999;
  };
  
  typedef boost::shared_ptr<OsiLPEngine> OsiLPEnginePtr;

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
