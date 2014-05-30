//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file simple-bnb.cpp
 * \brief Example of a very simple branch-and-bound.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "AMPLInterface.h"
#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "IntVarHandler.h"
#include "NodeIncRelaxer.h"
#include "Option.h"
#include "Problem.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"

using namespace Minotaur;

int main()
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  int err = 0;

  env->startTimer(err);

  // Read the problem
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  MINOTAUR_AMPL::AMPLInterface* iface =
    new MINOTAUR_AMPL::AMPLInterface(env, "bnb");
  ProblemPtr p = iface->readInstance("FLay02M.nl");
  p->setNativeDer();

  // create branch-and-bound object
  BranchAndBound *bab = new BranchAndBound(env, p);

  // setup handlers
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  handlers.push_back(v_hand);

  // setup engine for solving relaxations and branching
  EnginePtr e = (FilterSQPEnginePtr) new FilterSQPEngine(env);
  ReliabilityBrancherPtr rel_br = (ReliabilityBrancherPtr) new
                                  ReliabilityBrancher(env, handlers);
  rel_br->setEngine(e);

  // node processor
  NodeProcessorPtr nproc = (BndProcessorPtr) new BndProcessor(env, e, handlers);
  nproc->setBrancher(rel_br);
  bab->setNodeProcessor(nproc);

  // node relaxer
  NodeIncRelaxerPtr nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  RelaxationPtr rel = (RelaxationPtr) new Relaxation(p);
  nr->setRelaxation(rel);
  nr->setEngine(e);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);

  // start solving
  bab->solve();
  bab->writeStats();
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
