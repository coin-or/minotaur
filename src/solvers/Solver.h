//
// Minotaur -- It's only half bull!
//
// (C)opyright 2009 - 2025 The Minotaur Team.
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
                                   
    virtual bool timeCheck();

    /// Solve the problem
    virtual int solve(ProblemPtr p) = 0;

    /// get status of the last solve
    virtual SolveStatus getStatus() = 0;

    /// get solution of the last solve
    virtual SolutionPtr getSol() = 0;

    void setIface(MINOTAUR_AMPL::AMPLInterface* iface);


  protected:
    /// Minotaur Environment
    EnvPtr env_;

    /// AMPL Interface
    MINOTAUR_AMPL::AMPLInterface* iface_;

    /// True if iface was initialized by this solver, false if provided by a
    /// calling function.
    bool ownIface_;

    virtual int writeSol_(EnvPtr env, VarVector *orig_v, 
                          SolutionPtr sol, SolveStatus status,
                          MINOTAUR_AMPL::AMPLInterface* iface);

  private:
    const static std::string me_;

  };
}
#endif

