// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2013 The MINOTAUR Team.
// 

/**
 * \file SolutionPool.h
 * \brief Declare a container for storing solutions and their qualities.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */



#ifndef MINOTAURSOLUTIONPOOL_H
#define MINOTAURSOLUTIONPOOL_H

#include "Problem.h"
#include "Solution.h"

namespace Minotaur {

  class SolutionPool {
  public:
    /// Default constructor.
    SolutionPool();

    /// Construct a solution pool of a given size for a given problem
    SolutionPool(ProblemPtr problem, UInt limit=100);

    /// Get number of solutions in the pool
    UInt getNumSolutions() const;

    /// Put a limit on the number of solutions in the pool
    void setSizeLimit(UInt limit);

    /// Get the limit on the number of solutions in the pool
    UInt getSizeLimit() const;

    /// Get iterator for the first solution ...
    SolutionIterator solsBegin() { return sols_.begin(); }

    /// ... and the end.
    SolutionIterator solsEnd() { return sols_.end(); }

    /// Add Solution to the pool
    void addSolution(ConstSolutionPtr);

    /// Create a solution from a double array and add Solution to the pool.
    void addSolution(const Double *x, Double obj_value);

    /**
     * Get a solution with the best objective function value. Return NULL if
     * the pool is empty.
     */
    SolutionPtr getBestSolution();

    /// Get the best objective function value
    Double getBestSolutionValue() const;

  private:
    /// The solutions are stored in a vector. 
    std::vector<SolutionPtr> sols_;

    /**
     * The best solution in terms of objective function value. In case of tie,
     * the most recently found one.
     */
    SolutionPtr bestSolution_;

    /// Problem for which we are saving solutions
    ProblemPtr problem_;

    /// The limit on number of solutions in the pool.
    UInt sizeLimit_;

  };

  typedef boost::shared_ptr<SolutionPool> SolutionPoolPtr;
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
