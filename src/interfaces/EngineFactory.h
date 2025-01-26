//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
//


/**
 * \file EngineFactory.h
 * \brief Declare EngineFactory class for creating different engines.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURENGINEFACTORY_H
#define MINOTAURENGINEFACTORY_H

#include "Types.h"
#include "Engine.h"
#include "LPEngine.h"
#include "NLPEngine.h"
#include "MILPEngine.h"
#include "QPEngine.h"

namespace Minotaur {

  class EngineFactory {
    public:
      /// Default constructor
      EngineFactory();

      /// Constructor with minotaur environment.
      EngineFactory(EnvPtr env);

      /// Destroy.
      ~EngineFactory();

      /// Get an engine. Returns NULL if no engine is available.
      EnginePtr getEngine();

      /// Get an LP Engine
      LPEnginePtr getLPEngine();

      /// Get an MILP Engine
      MILPEnginePtr getMILPEngine();

      /// Get a QP Engine. Returns NULL if none available.
      QPEnginePtr getQPEngine();

      /// Get an NLP Engine. Returns NULL if none available.
      NLPEnginePtr getNLPEngine();

    private:
      /// Minotaur Environment.
      EnvPtr env_;
  };

}
#endif

