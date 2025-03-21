//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
//


/**
 * \file Reader.h
 * \brief Reader for MPS files
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURREADER_H
#define MINOTAURREADER_H

#include <string>
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

    enum MpsSec {
      MpsNone,
      MpsSense,
      MpsName,
      MpsRows,
      MpsCols,
      MpsRhs,
      MpsRang,
      MpsBoun,
      MpsQO,
      MpsQC,
      MpsEnd
    };

    /// Map of row-names (constraints and objective) and the row number
    std::map<std::string, int> rmap_;

    /// Map of column-names (variable names) and pointer to variable
    std::map<std::string, Variable *> vmap_;

    /// Problem that is being constructed
    Problem *p_;

    /// Get variable pointer for the given name. If the variable does not
    /// exist, then it is created.
    Variable *getMpsVar_(const std::string &s, VariableType vtype);

    /// Get row number (>= 0) for the given name. If the row does not exist,
    /// then it returns -1.
    int getMpsRow_(const std::string &s);
  };
}

#endif

