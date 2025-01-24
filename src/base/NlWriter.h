//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
//


/**
 * \file NlWriter.h
 * \brief Declare class NlWriter for writing nl files
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURNLWRITER_H
#define MINOTAURNLWRITER_H

#include <ios>
#include "Types.h"

namespace Minotaur {

class NonlinearFunction;
class QuadraticFunction;
typedef NonlinearFunction* NonlinearFunctionPtr;
typedef QuadraticFunction* QuadraticFunctionPtr;

/**
 * \brief Writes a problem to a .nl file. The nonlinear functions must be
 * stored in using native cgraphs for this class to work.
 */
class NlWriter {
public:
  /// Default constructor
  NlWriter(EnvPtr env);

  /// Destroy
  virtual ~NlWriter();

  /// write the nl file
  int write(ProblemPtr p, const std::string fname);

private:
  /// Environment.
  EnvPtr env_;

  /// For logging
  static const std::string me_;

  int co_(ProblemPtr p, std::ofstream &of);
  int exp_(std::ofstream &of, QuadraticFunctionPtr qf, NonlinearFunctionPtr nlf);
  int header_(ProblemPtr p, std::ofstream &of);
  int kjg_(ProblemPtr p, std::ofstream &of);
  int rb_(ProblemPtr p, std::ofstream &of);
};
}
#endif

