//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file simple-bnb-pres.cpp
 * \brief Example of a very simple branch-and-bound. With initial presolve.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "AMPLInterface.h"
#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "LinearHandler.h"
#include "IntVarHandler.h"
#include "IpoptEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "Presolver.h"
#include "Problem.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Timer.h"
#include "TreeManager.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  double objsense = 1.0;
  int err = 0;
  PresolverPtr pres;
  LinearHandlerPtr lhandler;

  // Start timer and read the problem
  env->startTimer(err); assert(err==0);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("expand_quad")->setValue(false);
  env->getOptions()->findDouble("bnb_time_limit")->setValue(3600);

  MINOTAUR_AMPL::AMPLInterface* iface =
    new MINOTAUR_AMPL::AMPLInterface(env, "bnb");
  ProblemPtr p = iface->readInstance(argv[1]);
  if (Maximize == p->getObjective()->getObjectiveType()) {
    p->negateObj();
    objsense = -1.0;
  }

  lhandler = (LinearHandlerPtr) new LinearHandler(env, p);
  lhandler->setPreOptPurgeVars(true);
  lhandler->setPreOptPurgeCons(true);
  lhandler->setPreOptCoeffImp(true);
  handlers.push_back(lhandler);
  handlers.push_back((NlPresHandlerPtr) new NlPresHandler(env, p));
  pres = (PresolverPtr) new Presolver(p, env, handlers);
  pres->solve();

  // create branch-and-bound object
  BranchAndBound *bab = new BranchAndBound(env, p);

  // setup handlers
  handlers.clear();
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  handlers.push_back(v_hand);

  // setup engine for solving relaxations and branching
  EnginePtr e = (IpoptEnginePtr) new IpoptEngine(env);
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
  rel->setNativeDer();

  nr->setRelaxation(rel);
  nr->setEngine(e);
  nr->setModFlag(false);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);

  // start solving
  bab->solve();
  bab->writeStats(std::cout);
  std::cout << "best solution value = " << objsense*bab->getUb() << std::endl
    << "status of branch-and-bound: "
    << getSolveStatusString(bab->getStatus()) << std::endl;
  //bab->getSolution()->writePrimal(std::cout);

  //finish
  delete iface;
  delete bab;
  return 0;
}


