// 
//
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file ConBoundMod.h
 * \brief Declare the class ConBoundMod. It is used to save modifications in a
 * bound of a constraint. Also declare ConBoundMod2 that is used to change both
 * bounds of a constraint. Also declare ConMod that is used to change
 * nonlinear part of the constraint
 * \author Meenarli Sharma, Indian Institute of Technology Bombay
 */


#ifndef MINOTAURCONBOUNDMOD_H
#define MINOTAURCONBOUNDMOD_H

#include "Modification.h"
#include "NonlinearFunction.h"

namespace Minotaur {
  class Engine;
  class ConBoundMod;
  typedef Engine* EnginePtr;
  typedef ConBoundMod* ConBoundModPtr;
  typedef std::vector <ConBoundModPtr> ConBoundModVector;
  typedef ConBoundModVector::iterator ConBoundModIter;
  typedef ConBoundModVector::const_iterator ConBoundModConstIter;


  /// Modification of a single bound on a constraint
  class ConBoundMod : public Modification {
    public:
      /// Construct.
      ConBoundMod(ConstraintPtr con, BoundType lu, double new_val);

      /// Destroy.
      ~ConBoundMod();

      // base class method.
      ModificationPtr fromRel(RelaxationPtr, ProblemPtr ) const
      {return ModificationPtr();};

      // base class method.
      ModificationPtr toRel(ProblemPtr, RelaxationPtr) const
      {return ModificationPtr();};

      /// Get the constraint whose bound is changed.
      ConstraintPtr getCon() const;

      /// Get the type of bound that is changed: lower or upper.
      BoundType getLU() const;

      /// Get new value of the bound.
      double getNewVal() const;

      // Implement Modification::applyToProblem().
      void applyToProblem(ProblemPtr problem);

      // Implement Modification::undoToProblem().
      void undoToProblem(ProblemPtr problem);

      // Implement Modification::write().
      void write(std::ostream &out) const;

    private:
      /// Lower or upper bound.
      BoundType lu_;

      /// The new value of the bound.
      double newVal_;

      /// The old value of the bound.
      double oldVal_;

      /// The constraint whose bounds are modified.
      ConstraintPtr con_;
  };

  class ConBoundMod2;
  typedef ConBoundMod2* ConBoundMod2Ptr;
  typedef const ConBoundMod2* ConstConBoundMod2Ptr;  
  typedef std::vector < ConBoundMod2Ptr > ConBoundMod2Vector;
  typedef ConBoundMod2Vector::iterator ConBoundMod2Iter;
  typedef ConBoundMod2Vector::const_iterator ConBoundMod2ConstIter;

  /// Modification of a both bounds on a constraint.
  class ConBoundMod2 : public Modification {
    public:
      /// Construct.
      ConBoundMod2(ConstraintPtr con, double new_lb, double new_ub);

      /// Destroy.
      ~ConBoundMod2();

      // Implement Modification::applyToProblem().
      void applyToProblem(ProblemPtr problem);

      // base class method.
      ModificationPtr fromRel(RelaxationPtr, ProblemPtr ) const
      {return ModificationPtr();};

      /// Get the constraint whose bound is changed.
      ConstraintPtr getCon() const;

      /// Get new value of the bound.
      double getNewLb() const;

      /// Get new value of the bound.
      double getNewUb() const;

      // base class method.
      ModificationPtr toRel(ProblemPtr, RelaxationPtr) const
      {return ModificationPtr();};

      // Implement Modification::undoToProblem().
      void undoToProblem(ProblemPtr problem);

      // Implement Modification::write().
      void write(std::ostream &) const {};

    private:
      /// The new lower bound.
      double newLb_;

      /// The new upper bound.
      double newUb_;

      /// Old lower bound.
      double oldLb_;

      /// Old upper bound.
      double oldUb_;

      /// The constraint whose bounds are modified.
      ConstraintPtr con_;
  };


  class ConMod;
  typedef ConMod* ConModPtr;
  typedef const ConMod* ConstConModPtr;  
  typedef std::vector < ConModPtr > ConModVector;
  typedef ConModVector::iterator ConModIter;
  typedef ConModVector::const_iterator ConModConstIter;

  /// Modification of a both bounds on a constraint.
  class ConMod : public Modification {
    public:
      /// Construct.
      ConMod(ConstraintPtr con, NonlinearFunctionPtr new_nlf);

      /// Destroy.
      ~ConMod();

      // Implement Modification::applyToProblem().
      void applyToProblem(ProblemPtr problem);

      // base class method.
      ModificationPtr fromRel(RelaxationPtr, ProblemPtr ) const
      {return ModificationPtr();};

      /// Get the constraint whose bound is changed.
      ConstraintPtr getCon() const;

      /// Get new nonlinearFunction 
      NonlinearFunctionPtr getNewNlf() const;


      // base class method.
      ModificationPtr toRel(ProblemPtr, RelaxationPtr) const
      {return ModificationPtr();};

      // Implement Modification::undoToProblem().
      void undoToProblem(ProblemPtr problem);

      // Implement Modification::write().
      void write(std::ostream &) const {};

    private:
      /// The new nonlinear function
      NonlinearFunctionPtr newNlf_;


      /// The old nonlinear function
      NonlinearFunctionPtr oldNlf_;

      /// The constraint whose bounds are modified.
      ConstraintPtr con_;
  };

}

#endif


