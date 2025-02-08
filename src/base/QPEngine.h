// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 


#ifndef MINOTAURQPENGINE_H
#define MINOTAURQPENGINE_H

#include "Engine.h"

namespace Minotaur {

  /**
   * The QPEengine class is an abstract class for interfacing QP solvers (like
   * bqpd). A derived class must implement calls to the QP solver for 
   * the methods described here.
   * 
   * \todo  add more methods for modifying the problem 
   * \todo  add more methods for setting solver options and parameters 
   */

  class QPEngine : public Engine {

    public:
      friend class Problem;

      /// Constructor. May set default parameters/options here.
      QPEngine() {};

      virtual ~QPEngine() {};
    protected:
      /// Destructor must be implemented.
  };
  typedef QPEngine* QPEnginePtr;
}

#endif
