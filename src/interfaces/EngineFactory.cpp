//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file EngineFactory.cpp
 * \brief EngineFactory class finds a suitable engine.
 * \author Sven Leyffer, Argonne National Laboratory
 */

//#define SPEW 1
#include "MinotaurConfig.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "LPEngine.h"
#include "NLPEngine.h"
#include "Option.h"
#include "QPEngine.h"

#ifdef USE_CBC
#include "CbcEngine.h"
#endif

#ifdef USE_CPX
#include "CplexLPEngine.h"
#include "CplexMILPEngine.h"
#endif

#ifdef USE_IPOPT
#include "IpoptEngine.h"
#endif

#ifdef USE_OSILP
#include "OsiLPEngine.h"
#endif

#ifdef USE_UNO
#include "UnoEngine.h"
#endif

#ifdef USE_FILTERSQP
#include "FilterSQPEngine.h"
#endif

#ifdef USE_BQPD
#include "BqpdEngine.h"
#endif

#ifdef USE_QPOASES
#include "qpOASESEngine.h"
#endif

using namespace Minotaur;

EngineFactory::EngineFactory()
  : env_(EnvPtr())
{
}


EngineFactory::EngineFactory(EnvPtr env)
  : env_(env)
{
}


EngineFactory::~EngineFactory()
{
  //env_.reset();
  env_ = 0;
}


LPEnginePtr EngineFactory::getLPEngine()
{
#ifdef USE_CPX
  if (env_->getOptions()->findString("lp_engine")->getValue() == "Cplex") {
    return ((CplexLPEnginePtr) new CplexLPEngine(env_));
  }
#endif
#ifdef USE_OSILP
  if (env_->getOptions()->findString("lp_engine")->getValue() != "none") {
    return ((OsiLPEnginePtr) new OsiLPEngine(env_));
  }
#endif
  return (LPEnginePtr());
}


MILPEnginePtr EngineFactory::getMILPEngine()
{
#ifdef USE_CBC
  if (env_->getOptions()->findString("milp_engine")->getValue() == "Cbc") {
    return ((CbcEnginePtr) new CbcEngine(env_));
  }
#endif
#ifdef USE_CPX
  if (env_->getOptions()->findString("milp_engine")->getValue() == "Cplex") {
    return ((CplexMILPEnginePtr) new CplexMILPEngine(env_));
  }
#endif
  return (MILPEnginePtr());
}


QPEnginePtr EngineFactory::getQPEngine()
{
#ifdef USE_BQPD
  if (env_->getOptions()->findString("qp_engine")->getValue() == "bqpd") {
    return ((BqpdEnginePtr) new BqpdEngine(env_));
  }
#endif
#ifdef USE_QPOASES
  if (env_->getOptions()->findString("qp_engine")->getValue() == "qpOASES") {
    return ((qpOASESEnginePtr) new qpOASESEngine(env_));
  }
#endif
  return (QPEnginePtr());
}


NLPEnginePtr EngineFactory::getNLPEngine()
{
  if (env_->getOptions()->findString("nlp_engine")->getValue() ==
      "filter-sqp") {
#ifdef USE_FILTERSQP
    return ((FilterSQPEnginePtr) new FilterSQPEngine(env_));
#endif
#ifdef USE_IPOPT
    return ((IpoptEnginePtr) new IpoptEngine(env_));
#endif
#ifdef USE_UNO
    return ((UnoEnginePtr) new UnoEngine(env_));
#endif
  }

  if (env_->getOptions()->findString("nlp_engine")->getValue() == "ipopt") {
#ifdef USE_IPOPT
    return ((IpoptEnginePtr) new IpoptEngine(env_));
#endif
#ifdef USE_FILTERSQP
    return ((FilterSQPEnginePtr) new FilterSQPEngine(env_));
#endif
#ifdef USE_UNO
    return ((UnoEnginePtr) new UnoEngine(env_));
#endif
  }
  if (env_->getOptions()->findString("nlp_engine")->getValue() == "uno") {
#ifdef USE_UNO
    return ((UnoEnginePtr) new UnoEngine(env_));
#endif
#ifdef USE_IPOPT
    return ((IpoptEnginePtr) new IpoptEngine(env_));
#endif
#ifdef USE_FILTERSQP
    return ((FilterSQPEnginePtr) new FilterSQPEngine(env_));
#endif
  }
  return (NLPEnginePtr());  // NULL
}

