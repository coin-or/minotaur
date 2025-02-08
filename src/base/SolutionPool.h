// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
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
#include "Types.h"
//#include <sys/time.h>

namespace Minotaur {

  class Environment;
  class Timer;

  class SolutionPool {
  public:
    /// Default constructor.
    SolutionPool();

    /// Destroy
    ~SolutionPool();

    /// Construct a solution pool of a given size for a given problem
    SolutionPool(EnvPtr env, ProblemPtr problem, UInt limit=100);

    /// Add Solution to the pool
    void addSolution(ConstSolutionPtr);

    /// Save the root relaxation solution
    //void setRootSolution(ConstSolutionPtr);

    /// Get number of solutions in the pool
    UInt getNumSols() const;

    /// Get number of solutions in the pool
    UInt getNumSolsFound() const;

    /// Get the limit on the number of solutions in the pool
    UInt getSizeLimit() const;

    /// Get wall clock time
    //double getWallTime() {
      //struct timeval time;
      //if (gettimeofday(&time,NULL)) {
        //// Handle error
        //return 0;
      //}
      //return (double)time.tv_sec + (double)time.tv_usec * .000001;
    //}

    /// Put a limit on the number of solutions in the pool
    void setSizeLimit(UInt limit);

    /// Get iterator for the first solution ...
    SolutionIterator solsBegin() { return sols_.begin(); }

    /// ... and the end.
    SolutionIterator solsEnd() { return sols_.end(); }

    /// Create a solution from a double array and add Solution to the pool.
    void addSolution(const double *x, double obj_value);

    /**
     * Get a solution with the best objective function value. Return NULL if
     * the pool is empty.
     */
    SolutionPtr getBestSolution();

    /**
     * Get a solution with the best objective function value. Return NULL if
     * the pool is empty.
     */
    //SolutionPtr getRootSolution();

    /// Get the best objective function value
    double getBestSolutionValue() const;

    /// Get the best objective function value
    //double getRootSolutionValue() const;

    /// Write statistics to the outstream.
    void writeStats(std::ostream &out) const; 

  private:
    /// The solutions are stored in a vector. 
    std::vector<SolutionPtr> sols_;

    /**
     * The best solution in terms of objective function value. In case of tie,
     * the most recently found one.
     */
    SolutionPtr bestSolution_;

    /**
     * The best solution in terms of objective function value. In case of tie,
     * the most recently found one.
     */
    //SolutionPtr rootSolution_;

    /// For logging.
    const static std::string me_;

    /// The number of solutions added to the pool.
    UInt numSolsFound_;

    /// Problem for which we are saving solutions
    ProblemPtr problem_;

    /// The limit on number of solutions in the pool.
    UInt sizeLimit_;

    /// Time when the best solution is found.
    double timeBest_;

    /// Time when the first solution is found.
    double timeFirst_;

    /// Global timer.
    const Timer* timer_;

    /// Wall clock start time.
    //double wallTimeStart_;

  };

  typedef SolutionPool* SolutionPoolPtr;
}
#endif

