//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file simple-qg.cpp
 * \brief Example of a simple implementation of QG Algorithm
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "IntVarHandler.h"
#include "LinearHandler.h"
#include "LPProcessor.h"
#include "NodeIncRelaxer.h"
#include "Option.h"
#include "OsiLPEngine.h"
#include "Problem.h"
#include "QGHandler.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"
#include "AMPLInterface.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  int err = 0;

  // Start timer and read the problem
  env->startTimer(err); assert(err==0);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  MINOTAUR_AMPL::AMPLInterface* iface =
    new MINOTAUR_AMPL::AMPLInterface(env, "bnb");
  ProblemPtr p = iface->readInstance(argv[1]);
  p->setNativeDer();

  // create branch-and-bound object
  BranchAndBound *bab = new BranchAndBound(env, p);
  EnginePtr nlp_e = (FilterSQPEnginePtr) new FilterSQPEngine(env);
  EnginePtr e = (OsiLPEnginePtr) new OsiLPEngine(env);

  // setup handlers
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  LinearHandlerPtr l_hand = (LinearHandlerPtr) new LinearHandler(env, p);
  QGHandlerPtr q_hand = (QGHandlerPtr) new QGHandler(env, p, nlp_e); 
  l_hand->setModFlags(false, true);
  q_hand->setModFlags(false, true);
  handlers.push_back(v_hand);
  handlers.push_back(l_hand);
  handlers.push_back(q_hand);

  // setup engine for solving relaxations and branching
  ReliabilityBrancherPtr rel_br = (ReliabilityBrancherPtr) new
                                  ReliabilityBrancher(env, handlers);
  rel_br->setEngine(e);

  // node processor
  NodeProcessorPtr nproc = (LPProcessorPtr) new LPProcessor(env, e, handlers);
  nproc->setBrancher(rel_br);
  bab->setNodeProcessor(nproc);

  // node relaxer
  NodeIncRelaxerPtr nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  nr->setEngine(e);
  nr->setModFlag(false);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(true);

  // start solving
  bab->solve();
  bab->writeStats(std::cout);
  bab->getSolution()->writePrimal(std::cout);
  std::cout << "best solution value = " << bab->getUb() << std::endl;

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
