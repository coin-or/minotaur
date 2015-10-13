//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file simple-glob.cpp
 * \brief A simple demonstration of global optimization of QCQPs.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "IpoptEngine.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "LPEngine.h"
#include "LPProcessor.h"
#include "OsiLPEngine.h"
#include "MaxVioBrancher.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "Problem.h"
#include "Presolver.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "SimpleTransformer.h"
#include "Solution.h"
#include "Timer.h"
#include "Transformer.h"

#include "AMPLInterface.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  int err = 0;
  double objsense = 1.0;
  ProblemPtr p, newp;

  // Start timer and read the problem
  env->startTimer(err); assert(err==0);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findDouble("bnb_time_limit")->setValue(600);
  env->getOptions()->findInt("pres_freq")->setValue(1);

  MINOTAUR_AMPL::AMPLInterface* iface =
    new MINOTAUR_AMPL::AMPLInterface(env, "s-glob");
  p = iface->readInstance(argv[1]);
  p->setNativeDer();
  if (Maximize == p->getObjective()->getObjectiveType()) {
    p->negateObj();
    objsense = -1.0;
  }

  SimpTranPtr trans = (SimpTranPtr) new SimpleTransformer(env, p);
  trans->reformulate(newp, handlers, err); assert(0==err);

  // create branch-and-bound object
  BranchAndBound *bab = new BranchAndBound(env, newp);
  EnginePtr nlp_e = (IpoptEnginePtr) new IpoptEngine(env);
  EnginePtr e = (OsiLPEnginePtr) new OsiLPEngine(env);

  PresolverPtr pres = (PresolverPtr) new Presolver(newp, env, handlers);
  pres->solve();
  
  // brancher
  MaxVioBrancherPtr m_br = (MaxVioBrancherPtr) new
                                  MaxVioBrancher(env, handlers);
  //rel_br->setEngine(e);

  // node processor
  NodeProcessorPtr nproc = (LPProcessorPtr) new LPProcessor(env, e, handlers);
  nproc->setBrancher(m_br);
  bab->setNodeProcessor(nproc);

  // node relaxer
  NodeIncRelaxerPtr nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  nr->setEngine(e);
  nr->setModFlag(true);
  nr->setProblem(newp);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(true);

  // start solving
  bab->solve();
  bab->writeStats(std::cout);
  e->writeStats(std::cout);
  std::cout << "best solution value = " << objsense*bab->getUb() << std::endl
    << "status of branch-and-bound: "
    << getSolveStatusString(bab->getStatus()) << std::endl;

  //finish
  delete iface;
  delete bab;
  return 0;
}

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
