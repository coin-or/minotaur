//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
//

/**
 * \file Constraint.h
 * \brief Get information about a constraint in a given Problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURCONSTRAINT_H
#define MINOTAURCONSTRAINT_H

#include <string>

#include "Types.h"

namespace Minotaur {
  class   Function;
  class   LinearFunction;
  class   QuadraticFunction;
  class   NonlinearFunction;
  typedef boost::shared_ptr<Function> FunctionPtr;
  typedef boost::shared_ptr<LinearFunction> LinearFunctionPtr;
  typedef boost::shared_ptr<NonlinearFunction> NonlinearFunctionPtr;
  typedef boost::shared_ptr<QuadraticFunction> QuadraticFunctionPtr;
  typedef std::set<std::pair<VariablePtr, FunctionType> >::const_iterator 
    VariableFunIterator;

  /**
   * The Constraint class is used to manage a constraint. The general form of
   * a constraint is:
   * 
   * l <= f(x) <= u,
   * 
   * where f(x) is a function of the variables and l, u are constraints placed
   * on f. l can be -infinity and/or u can be infinity.
   * 
   * A constraint should only be constructed through a Problem. This
   * ensures that constraints have ID's unique for that problem. A
   * constraint can thus be part of only one problem. A function (f) that
   * is part of a constraint should also not be shared at several places in
   * the same problem or amongst different problems.  Similarly,
   * constraints should be deleted only through a problem.  Changes in a
   * constraint (e.g. changing name, bounds etc) may be done directly.
   * 
   * The state of a constraint describes whether the constraint is free or
   * deleted etc. It is Fixed if each variable in the constraints has been 
   * fixed.
   * 
   * A Constraint also stores a map of variables that occur in that
   * constraint. The map tells whether the variable is in a non-linear, 
   * or quadratic etc part of the constraint. A variable that occurs 
   * once in linear and thrice in quadratic parts will have four entries in
   * the map, once for each occurrence.
   * 
   * The name of a constraint can be set by the user. There are no checks for
   * duplicate names. The ID of the constraint however is unique in the
   * problem.
   */
  class Constraint {
    public:

      /// Only Problem class can modify a Constraint. All modification methods
      /// are private.
      friend class Problem;


      /// Default constructor.
      Constraint();

      /**
       * This is the only real constructor. All others call this constructor.
       * create a constraint with an id, function f, lb, ub and the name of the 
       * constraint.
       */
      Constraint(UInt id, UInt index, FunctionPtr f, Double lb, Double ub, 
          std::string name);

      /**
       * Create a constraint with function f, lb, ub. the name of the constraint
       * is created  only when the id has been set or the setName() function is
       * used.
       */
      Constraint(FunctionPtr f, Double lb, Double ub);

      /// Destroy
      virtual ~Constraint();


      // get methods

      /// Return the unique id of the constraint.
      UInt getId() const { return id_; }

      /// Return the index of the constraint.
      UInt getIndex() const { return index_; }

      /// Get the 'l' value. or the lower bound constraint on 'f'.
      Double getLb() const { return lb_; }

      /// Get the 'u' value. or the upper bound constraint on 'f'.
      Double getUb() const { return ub_; }

      /// Get the function.
      const FunctionPtr getFunction() const { return f_; }

      /// Get the function type.
      FunctionType getFunctionType() const;

      /// Get the linear part of the constraint function 'f'.
      const LinearFunctionPtr getLinearFunction() const;

      /// Get the quadratic part of the constraint function 'f'.
      const QuadraticFunctionPtr getQuadraticFunction() const;

      /// Get the nonlinear part of the constraint function 'f'.
      const NonlinearFunctionPtr getNonlinearFunction() const;

      /// Get the current state of the constraint: freed, fixed etc.
      ConsState getState() const { return state_; }

      // Get the name of the constraint. 
      const std::string getName() const;

      /// Get the value or activity at a given point.
      Double getActivity(const Double *x, Int *error) const;

      /// display the constraint
      void write(std::ostream &out) const;

    protected:
      /// set state of the constraint.
      void setState_(ConsState state) { state_ = state; return; }

      /// Change the linear part of constraint.
      void changeLf_(LinearFunctionPtr lf);

      /**
       * Substitute a variable 'in' with rat * out.
       */
      void subst_(VariablePtr out, VariablePtr in, Double rat, Bool *instay);

      /// Add a constant to the constraint function. lb - c <= f <= ub - c.
      void add_(Double cb);

      /**
       * Set a new lower bound. could be -infinity. Can affect the state of the
       * constraint.
       */
      void setLb_(Double newLb) { lb_ = newLb; }

      /**
       * Set a new upper bound. could be infinity. Can affect the state of the
       * constraint.
       */
      void setUb_(Double newUb) { ub_ = newUb; }

      /// set an index. 
      void setIndex_(UInt n) { index_ = n; }

      /// Set name of the constraint
      void setName_(std::string name);

      /// Delete variables fixed at value val.
      void delFixedVar_(VariablePtr v, Double val);

      /**
       * convert the constraint from 
       * lb <= g(x) <= ub 
       * to
       * -ub <= -g(x) <= lb
       */
      void reverseSense_();

    private:
      /// The function 'f' in l <= f(x) <= u.
      FunctionPtr f_;

      /// 'l' [-infinity, infinity).
      Double lb_;

      /// 'u' (-infinity, infinity].
      Double ub_;

      /// free or fixed etc.
      ConsState state_;

      /// id that is unique for this constraint in the problem.
      UInt id_;         

      /// id that is unique for this constraint in the problem.
      UInt index_;         

      /// name of the constraint. could be NULL.
      std::string name_;


  };
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
