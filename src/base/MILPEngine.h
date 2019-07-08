// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/**
 * \file MILPEngine.h
 * \author Ashutosh Mahajan, IIT Bombay.
 * \brief Declare the class MILPEngine for solving MILPs and getting solution.
 */


#ifndef MINOTAURMILPENGINE_H
#define MINOTAURMILPENGINE_H

#include "Engine.h"

namespace Minotaur {

  /**
   * The MILPEengine class is an abstract class for interfacing MILP solvers
   * (like CBC). A derived class must implement calls to the MILP solver for 
   * the methods described here.
   * 
   */

  class MILPEngine : public Engine {

    public:
      friend class Problem;

      /// Constructor. May set default parameters/options here.
      MILPEngine() {};

      /// Destructor must be implemented if memory needs to be freed
      virtual ~MILPEngine() {};

      /// Set time limit for an iteration
      virtual void setTimeLimit(double) = 0;

      /// Set upper cutoff for an iteration
      virtual void setUpperCutoff(double) = 0;

      /// Get number of solutions in the solution pool of MILP engine
      virtual UInt getNumSols() = 0;

      /// Get a particular solution from solution pool
      virtual ConstSolutionPtr getSolutionFromPool(int ) = 0;

  };
  typedef MILPEngine* MILPEnginePtr;
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
