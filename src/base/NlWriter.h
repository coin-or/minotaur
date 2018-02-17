//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
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

  /**
   * \brief Writes a problem to a .nl file
   */
  class NlWriter {
  public:
    /// Default constructor
    NlWriter();

    /// Destroy
    virtual ~NlWriter();

    /// write the nl file
    int write(ProblemPtr p, const std::string fname);

  private:
    /// For logging
    static const std::string me_;

    int writeCO_(ProblemPtr p, std::ofstream &of);
    int writeHeader_(ProblemPtr p, std::ofstream &of);
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
