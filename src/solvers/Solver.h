//
// MINOTAUR -- It's only half bull!
//
// (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file Solver.h
 * \brief Define the abstract base class Solver.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef Solver_H
#define Solver_H

#include "Types.h"
#include "AMPLInterface.h"
#include "Environment.h"
#include "LPEngine.h"
#include "Presolver.h"

namespace Minotaur {
  /**
   * The Solver base class has methods common for solvers: reading instances,
   * writing files, etc. Solvers like QG, BnB, Glob etc must be derived from
   * this class. 
   */
  class Solver {
  public:
    /// Default constructor.
    Solver();

    /// Destroy.
    virtual ~Solver();

    /// Helper function to determin file type (mps, nl etc)
    virtual FileType getFileType(std::string fname);

    virtual ProblemPtr readProblem(std::string fname, std::string dname,
                                   std::string sname, int &err);

    /// Solve the problem
    virtual int solve(ProblemPtr p) = 0;

    /// get status of the last solve
    virtual SolveStatus getStatus() = 0;


  protected:
    EnvPtr env_;
    MINOTAUR_AMPL::AMPLInterface* iface_;
    virtual int writeSol_(EnvPtr env, VarVector *orig_v, PresolverPtr pres,
                          SolutionPtr sol, SolveStatus status,
                          MINOTAUR_AMPL::AMPLInterface* iface);

  private:
    const static std::string me_;

  };
}
#endif

