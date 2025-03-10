//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
//


/**
 * \file KnapsackList.h
 * \brief Declare base class KnapsackList.
 * \author Serdar Yildiz, Argonne National Laboratory
 */

#ifndef MINOTAURKNAPSACKLIST_H
#define MINOTAURKNAPSACKLIST_H

#include "Problem.h"
#include "Constraint.h"

namespace Minotaur {
  /**
   * The KnapsackList class generates the list of knapsack
   * constraint pointers for a given problem.
   */
  class KnapsackList {
  public:
    // Default constructor.
    KnapsackList();

    // Constructs list from a given problem.
    KnapsackList(ProblemPtr p);

    // Destructor.
    ~KnapsackList();

    // Generates the list of knapsack constraints.
    void generateList();

    // Evaluate a constraint and decide to add it to the list.
    void evalConstraint(ConstraintConstIterator it);

    // Add a constraint to list not.
    void addConstraint(ConstraintConstIterator it);

    // Get number of knapsack constraints.
    size_t getNumKnaps() const { return list_->size(); }

    // Get number of constrainst checked for linear or not.
    UInt getNumConsChecked() const { return numConsChecked_; }

    // Get an iterator for the beginning of knapsack list.
    ConstraintIterator getListBegin() { return list_->begin(); }

    // Get an iterator for the end of the knapsack list.
    ConstraintIterator getListEnd() { return list_->end(); }


  private:
    // The problem that knapsack list is generated from.
    ProblemPtr p_;
    // A vector of pointers to kanpsack constraints.
    ConstraintVectorPtr list_;
    // Number of constraints checked.
    UInt numConsChecked_;
  };

}  //namespace Minotaur

#endif  // MINOTAURKNAPSACKLIST_H
