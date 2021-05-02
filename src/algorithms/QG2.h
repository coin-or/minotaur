//
// MINOTAUR -- It's only half bull!
//
// (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file QG.h
 * \brief Define the QG class.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef QG2_H
#define QG2_H

#include "Types.h"
#include "AMPLInterface.h"
#include "LPEngine.h"
#include "Presolver.h"

namespace Minotaur {
  /**
   * The QG class sets up methods for solving a convex MINLP instance using
   * the LP/NLP based Quesada Grossmann algorithm.
   */
  class QG2 {
    public:
      /// Default constructor.
      QG2(EnvPtr env);

      /// Destroy.
      ~QG2();

      FileType getFileType(std::string fname);

      /// show help messages
      void showHelp() const;

      /// Display information
      int showInfo();

      /// Read the problem from a file. The filename should be available in
      /// env->options.
      int loadProblem();

      /// Read the problem from a file. The filename is provided as an input.
      int loadProblem(std::string fname);

      /// Load the problem provided as an input.
      int loadProblem(ProblemPtr p);

      /// Solve the problem
      int solve();

      /// Write the solution to a solution file or display, depending on the
      /// options.
      int writeSol();

    private:
      EnvPtr env_;
      MINOTAUR_AMPL::AMPLInterface* iface_;
      const static std::string me_;
      double objSense_;
      ProblemPtr oinst_;

      int getEngines_(Engine **nlp_e, LPEngine **lp_e);
      PresolverPtr presolve_(HandlerVector &handlers);
  };
}
#endif

