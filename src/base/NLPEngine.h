//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file NLPEngine.h
 * \brief Declare NLPEngine Class for solving nonlinear problems using a
 * nonlinear solver.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURNLPENGINE_H
#define MINOTAURNLPENGINE_H

#include "Engine.h"

namespace Minotaur {

  /**
   * The NLPEngine class is an abstract class for interfacing NLP solvers (like
   * Ipopt). A derived class must implement calls to the NLP solver for 
   * the methods described here.
   * 
   * Usually, NLP solvers generate a sequence of points and ask back the
   * function evaluation, gradient of objective function, jacobian of the
   * constraints, hessian of the lagrangean etc. A derived class of NLPEngine
   * should therefore implment methods that can return the desired information
   * to the solver. 
   * 
   * \todo add more methods for modifying the problem 
   * \todo add more methods for setting solver options and parameters 
   */
  class NLPEngine : public Engine
  {
    public:
      friend class Problem;

      /// Default constructor.
      NLPEngine() {};

      /// Default destructor.
      virtual ~NLPEngine() {};

  };
  typedef NLPEngine* NLPEnginePtr;
}

#endif
