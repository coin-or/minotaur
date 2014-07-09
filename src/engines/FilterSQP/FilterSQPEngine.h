// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2014 The MINOTAUR Team.
// 

/**
 * \file FilterSQPEngine.h
 * \author Sven Leyffer, Argonne National Laboratory.
 * Define the class FilterSQPEngine.
 */


#ifndef MINOTAURFILTERSQPENGINE_H
#define MINOTAURFILTERSQPENGINE_H

#include "NLPEngine.h"
#include "WarmStart.h"

namespace Minotaur {
  class   Environment;
  class   FilterSQPWarmStart;
  class   Problem;
  class   Solution;
  class   Timer;
  typedef boost::shared_ptr<Environment> EnvPtr;
  typedef boost::shared_ptr<FilterSQPWarmStart> FilterWSPtr;
  typedef boost::shared_ptr<const FilterSQPWarmStart> ConstFilterWSPtr;
  typedef boost::shared_ptr<Problem> ProblemPtr;
  typedef boost::shared_ptr<Solution> SolutionPtr;

  struct FilterSQPStats {
    UInt calls;     /// Total number of calls to solve.
    UInt strCalls;  /// Calls to solve while strong branching.
    Double time;    /// Sum of time taken in all calls to solve.
    Double strTime; /// time taken in strong branching alone.
    UInt iters;     /// Sum of number of iterations in all calls. 
    UInt strIters;  /// Number of iterations in strong branching alone.
  };

  class FilterSQPWarmStart : public WarmStart {
    public:

      /// Default constructor
      FilterSQPWarmStart();

      /// Copy constructor. Creates a full copy, not just copies pointers.
      FilterSQPWarmStart(ConstFilterWSPtr warm_st);

      /// Destroy
      ~FilterSQPWarmStart();
      
      // Implement WarmStart::hasInfo().
      Bool hasInfo();

      // Implement WarmStart::write().
      void write(std::ostream &out) const;

      /**
       * Overwrite the primal and dual values of warm-start. Sometimes, the
       * warm-start data is initialized and needs to be updated. This
       * should be called in place of deleting and creating a new warm-start
       * object.
       */
      void setPoint(SolutionPtr sol);

      /// Get solution.
      SolutionPtr getPoint();

    private:
      /// The starting solution that is used to warm-start.
      SolutionPtr sol_;
  };

  /**
   * FilterSQPEngine is the class that is used to solve NLP problems using
   * the FilterSQP. FilterSQP can be used to solve problems completely to
   * optimality or approximately.
   */
  class FilterSQPEngine : public NLPEngine {

    public:
      friend class Problem;

      /// Default constructor.
      FilterSQPEngine();    

      /// Constructor using given environment options.
      FilterSQPEngine(EnvPtr env);    

      /// Return an empty FilterSQPEngine pointer.
      EnginePtr emptyCopy();

      /// Method to read the problem and initialize FilterSQP.
      void load(ProblemPtr problem);

      /// Solve the problem that was loaded and report the status.
      EngineStatus solve();

      /// Report the solution value from the last solve.
      Double evalObjValue(const Double *x, Int *err);

      /**
       * Evaluate the activity of constraints and fill the values in 'c'.
       * This is a callback function that is called by filter. 
       */
      void evalCons(const Double *x, Double *c, Int *error);

      /**
       * Evaluate the gradient of objective and fill in 'a'. 
       * This is a callback function that is called by filter. 
       */
      void evalGrad(const Double *x, Double *a, Int *error);

      /**
       * Evaluate the hessian of lagrangian at point x and fill in. 
       * This is a callback function that is called by filter. 
       */
      void evalHessian(const Double *x, Double *lam, 
          const Int phase, Double *ws, Int *lws, Int *l_hess, Int *li_hess, 
          Int *error);

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

      // change objective.
      void changeObj(FunctionPtr f, Double cb);

      // Convert 'min f' to 'min -f'.
      void negateObj();

      // Base class method
      void addConstraint(ConstraintPtr c);

      // Implement Engine::changeConstraint().
      void changeConstraint(ConstraintPtr con, LinearFunctionPtr lf, 
                            Double lb, Double ub);

      // Implement Engine::changeConstraint().
      void changeConstraint(ConstraintPtr c, NonlinearFunctionPtr nlf);

      // Implement Engine::getWarmStart(). // NULL for now.
      virtual ConstWarmStartPtr getWarmStart();

      // Implement Engine::getWarmStartCopy(). // NULL for now.
      virtual WarmStartPtr getWarmStartCopy();

      // Implement Engine::loadFromWarmStart().
      virtual void loadFromWarmStart(WarmStartPtr ws);

      // Implement Engine::setIterationLimit().
      void setIterationLimit(Int limit);

      // Implement Engine::resetIterationLimit().
      void resetIterationLimit();

      /// Make settings for strong branching.
      void enableStrBrSetup();

      /// Restore settings after strong branching.
      void disableStrBrSetup();

      // Write statistics.
      void writeStats();

      // Get name.
      std::string getName() const;

      void clear();

      // base class method.
      void removeCons(std::vector<ConstraintPtr> &delcons);

      /// Destroy.
      ~FilterSQPEngine();

    private:
     
      /// Environment.
      EnvPtr env_;

      /**
       * la_ stores the sparsity pattern of jacobian. It needs to be
       * evaluated only once.
       */
      Int *la_;

      Double *ws_;

      /**
       * lws_ stores the sparsity pattern of hessian of lagrangian. It needs
       * to be evaluated only once. In our implementation:
       * lws_[0]    = number of entries in the hessian of lagrangian.
       * We set phl = 0 (phl is a common block in filter-sqp),
       * phr        = 1,
       * lws_[phr]  = lws_[1] = row index of the first entry in the
       *              hessian,
       * lws_[phr+i]= lws_[1+i] = row index of the i-th entry in the
       *              hessian,
       * phc        = hess_nz + 1,
       * lws_[phc]  = lws_[1+hess_nz] = column index of the first entry 
       *              in the hessian,
       * lws_[phc+i]= lws_[1+hess_nz+i] = column index of the i-th entry 
       *              in the hessian.
       */
      Int *lws_;

      /// Copy of lws_;
      Int *lws2_;

      /// values of constraint functions
      Double *c_;

      /// scale factors.
      Double *s_;

      /// Jacobian storage.
      Double *a_;

      /// Solution.
      Double *x_;

      /// Lagrange multipliers.
      Double *lam_;

      /// Need to multiply lagrange multipliers by -1 in callback. Storage.
      Double *mlam_;

      /// Lower bounds.
      Double *bl_;

      /// Upper bounds.
      Double *bu_;

      /// Linear (L) or Nonlinear (N).
      char *cstype_;

      /// Statistics.
      Double *rstat_; 

      /// Statistics.
      Int *istat_;


      /// Solution found by the engine. 
      SolutionPtr sol_;

      /// Number of iterations that can be performed during solve
      Int iterLimit_; 

      /// Max value of iterLimit_, when solving a relaxation
      const Int maxIterLimit_;

      /// if rstat_[4]<feasTol_, then the solution is considered feasible.
      const Double feasTol_;

      /// if lb > ub and lb < ub+bTol_, we make ub = lb.
      Double bTol_;

      /// warm start information.
      FilterWSPtr warmSt_;

      /**
       * True if we want to save warm start information of the current
       * solution for the next solve. False, if no information needs to be
       * saved. Saving information does not mean that it will be used.
       * useWs_ flag must be on to use the warm-start information.
       */
      Bool prepareWs_;

      /**
       * True if we want to use warm-start information, either from a
       * previous solve or from a user provided structure.
       */
      Bool useWs_;

      /**
       * If true, then copy the solution from the last solve. Otherwise,
       * don't copy it.
       */
      Bool saveSol_;

      /// If true, we are currently in strong-branching mode. False otherwise.
      Bool strBr_;

      /**
       * If true, reallocate space in the next solve. Important, if
       * constraints or objectives have changed.
       */
      Bool consChanged_;

      /// Timer
      Timer *timer_;

      /// Statistics.
      FilterSQPStats *stats_;

      /// Problem that is loaded, if any.
      ProblemPtr problem_;

      /// Allocate storage space for filter-sqp.
      void setStorage_(Int mxwk, Int maxa);

      /// Copy bounds from problem into filter-sqp's arrays.
      void setBounds_();

      /// Free arrays used by filter-sqp.
      void freeStorage_();

      /// Allocate space and fill sparsity pattern of the Jacobian and Hessian.
      void setStructure_();

  };
  
  typedef boost::shared_ptr<FilterSQPEngine> FilterSQPEnginePtr;
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
