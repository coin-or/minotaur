// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2014 The MINOTAUR Team.
// 

/// \file BqpdEngine.h
/// \author Sven Leyffer, Argonne National Laboratory.
/// Declare the class BqpdEngine.


#ifndef MINOTAURBQPDENGINE_H
#define MINOTAURBQPDENGINE_H

#include <boost/shared_ptr.hpp>

#include "QPEngine.h"

namespace Minotaur {

  class   Timer;
  class   Environment;
  class   Problem;
  class   Solution;
  typedef boost::shared_ptr<Environment> EnvPtr;
  typedef boost::shared_ptr<Problem> ProblemPtr;
  typedef boost::shared_ptr<Solution> SolutionPtr;

  struct BqpdStats {
    UInt calls;     /// Total number of calls to solve.
    UInt strCalls;  /// Calls to solve while strong branching.
    Double time;    /// Sum of time taken in all calls to solve.
    Double strTime; /// Time taken in strong branching alone.
    Double cTime;   /// Time taken in copying data for strong-branching.
    UInt iters;     /// Sum of number of iterations in all calls. 
    UInt strIters;  /// Number of iterations in strong branching alone.
  };

  class BqpdData;
  /**
   * BqpdEngine is used to solve QP problems using bqpd. 
   * bqpd finds a KT point for the bounded QP problem
   * 
   *      minimize    f(x) = ct.x + xt.G.x/2
   * 
   *      subject to  l <= [I : A]t.x <= u                  (t = transpose)
   * 
   * where x and c are n-vectors, G is a symmetric n*n matrix, and A is an
   * n*m matrix. If G is also positive semi-definite then the KT point is a
   * global solution, else usually a local solution. The method may also be
   * used efficiently to solve an LP problem (G=0). bqpd can be used to solve
   * problems completely to optimality or approximately.
   */

  class BqpdEngine : public QPEngine {

    public:
      friend class Problem;

      /// Default constructor.
      BqpdEngine();    

      /// Constructor using given environment options.
      BqpdEngine(EnvPtr env);    

      /// Return an empty BqpdEngine pointer.
      EnginePtr emptyCopy();

      /// Method to read the problem and initialize bqpd.
      void load(ProblemPtr problem);

      /// Method to unload the current problem
      void clear();

      /// Solve the problem that was loaded and report the status.
      EngineStatus solve();

      /// Report the solution value from the last solve. 
      Double getSolutionValue();

      /// Report the solution.
      ConstSolutionPtr getSolution();

      /// Report the status of the last solve.
      EngineStatus getStatus();

      // Change bound on a constraint.
      void changeBound(ConstraintPtr cons, BoundType lu, Double new_val);

      // Change bound on a variable.
      void changeBound(VariablePtr var, BoundType lu, Double new_val);

      // Implement Engine::changeBound(VariablePtr, Double, Double).
      void changeBound(VariablePtr var, Double new_lb, Double new_ub);

      // Implement Engine::addConstraint() */
      void addConstraint(ConstraintPtr);

      // change objective.
      void changeObj(FunctionPtr f, Double cb);

      // Convert 'min f' to 'min -f'.
      void negateObj();

      // Implement Engine::changeConstraint().
      void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf, 
                            Double lb, Double ub);

      // base class method
      void changeConstraint(ConstraintPtr con, NonlinearFunctionPtr nlf);

      // Implement Engine::getWarmStart(). // NULL for now.
      ConstWarmStartPtr getWarmStart() {return WarmStartPtr(); }; 

      // Implement Engine::getWarmStartCopy(). // NULL for now.
      WarmStartPtr getWarmStartCopy() {return WarmStartPtr(); };

      // Implement Engine::loadFromWarmStart().
      void loadFromWarmStart(WarmStartPtr ) {};

      // Implement Engine::setIterationLimit().
      void setIterationLimit(Int limit);

      // Implement Engine::resetIterationLimit().
      void resetIterationLimit();

      // Implement Engine::enableStrBrSetup()
      void enableStrBrSetup();

      // Implement Engine::disableStrBrSetup()
      void disableStrBrSetup();

      // delete constraints.
      void removeCons(std::vector<ConstraintPtr> &delcons);

      // Write statistics.
      void writeStats(std::ostream &out) const;

      // get name.
      std::string getName() const;

      /// Destroy.
      ~BqpdEngine();

    private:

      /**
       * True is some variable bounds are relaxed. BQPD has difficulty
       * hot-strating in this case.
       */
      Bool bndRelaxed_;

      /// No. of bound changes since last solve.
      UInt bndChanges_;

      const Double bTol_;

      /// Checkpoint copy of fStart_.
      BqpdData *chkPt_;

      /// If a constraint is modified, this is set to true. 
      Bool consModed_;

      /// Array to calculate dual solution for constraints.
      Double *dualCons_;

      /// Array to calculate dual solution for variables.
      Double *dualX_;

      /// Environment.
      EnvPtr env_;

      /// Information for full start
      BqpdData *fStart_;

      /// Bounds are considered infinite if their value goes beyond this value.
      const Double infty_;

      /// Number of iterations that can be performed during solve
      Int iterLimit_; 

      /// Max value of iterLimit_, when solving a relaxation
      const Int maxIterLimit_;

      /// Constant part of the obj.
      Double objOff_;

      /// If the previous call was a strong-branching call.
      Bool prevStrBr_;

      /// Problem that is loaded, if any.
      ProblemPtr problem_;

      /**
       * If true, we should try to resolve in a different mode when error is
       * reported.
       */
      Bool resolveError_;

      /// Solution found by the engine. 
      SolutionPtr sol_;

      /// Statistics.
      BqpdStats *stats_;

      /// True if currently doing strong-branching iterations. False otherwise.
      Bool strBr_;

      /// Timer for bqpd solves.
      Timer *timer_;

      /// Mode used for warm starting: 1-6
      Int wsMode_;

      /// Allocate the data structures for Bqpd.
      void load_();

      /**
       * Actually call bqpd to solve a QP using a specific mode. It is called
       * after all data has been set.
       */
      void solve_(Int mode, Double &f);

      /// Fill sparsity pattern and values of the gradients.
      void setGradient_();

      /// Fill sparsity pattern and values of the Hessian.
      void setHessian_();

      /// Copy primal and dual values of the solution from bqpd.
      void storeSol_(Double f);

      /// Set the intial point for solving the QP.
      void setInitialPoint_();

      /// Copy variable bounds from the problem.
      void setVarBounds_();

      /// Copy constraint bounds from the problem.
      void setConsBounds_();

      /// Free the memory allocated
      void freeProb_();
  };


  /// Information for restarting from the previous optimal solution.
  class BqpdData {
    public: 
      /// Constructor.
      BqpdData(UInt n_t, UInt m_t, int kmax_t, UInt maxa_t, UInt lh1_t,
               UInt nJac, Bool zero=true);

      /// Allocate space and copy.
      BqpdData *clone();

      /// Only copy. No space allocation.
      void copyFrom(const BqpdData* rhs);

      /// Display all data.
      void write(std::ostream &out) const;

      /// Destroy.
      ~BqpdData();

      /// Number of variables.
      UInt n;

      /// Number of constraints.
      UInt m;

      /// kmax given to bqpd
      int kmax;

      /// Number of nonzeros in Hessian.
      UInt lh1;

      /// Number of nonzeros in Jacobian.
      UInt nJac;

      /// Size of a
      UInt maxa;

      /// Initial point for solving QP.
      Double *x;

      /// Residuals/multipliers.
      Double *r;

      /// Steepest-edge normalization coefficients .
      Double *e;

      /// Denominators for ratio tests.
      Double *w;

      /// Gradient vector of f(x).
      Double *g;

      /// Indices of the active constraints .
      Int *ls;

      /// Workspace associated with recursion.
      Double *alp;

      /// Workspace associated with recursion.
      Int *lp;

      /// Information on return from bqpd.
      Int *info;

      /// Lower bounds for variables and constraints.
      Double *bl;

      /// Upper bounds for variables and constraints.
      Double *bu;

      /// Storage for jacobian.
      Double *a;

      /// Storage for jacobian.
      Int *la;

      /// Storage for hessian values and other things.
      Double *ws;

      /// Storage for hessian indices and other things.
      Int *lws;

      /// Pointer to equality constraints, used by bqpd.
      Int peq;

      // Dimension of reduced-space, set only when mode>=0.
      Int k;
  };

  
  typedef boost::shared_ptr<BqpdEngine> BqpdEnginePtr;
} // end namespace Minotaur 

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
