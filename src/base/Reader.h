//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2021 The MINOTAUR Team.
//


/**
 * \file Reader.h
 * \brief Reader for MPS files
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURREADER_H
#define MINOTAURREADER_H

#include "Types.h"
#include "Variable.h"

namespace Minotaur {

  class Function;
  class LinearFunction;
  struct ProblemSize;
  class SOS;

  /**
   * \brief Read an LP or MILP instance from an MPS file
   *
   * MPS files are read following the CPLEX 12.8 documentation
   */
  class Reader {
  public:
    /// Default constructor
    Reader(EnvPtr env);

    /// Destroy
    ~Reader();

    /// Read a file and return the Problem object
    ProblemPtr readMps(std::string fname, int &err);

    /// Read a solution file and store it in Problem for debugging
    int readSol(ProblemPtr p, std::string sname);

  private:
    /// Environment
    EnvPtr env_;

    /// Pointer to the log manager. All output messages are sent to it.
    LoggerPtr logger_;

    /// For logging
    static const std::string me_;
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
