//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file simple-bnb.cpp
 * \brief Example of branch-and-bound with presolve
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "AMPLInterface.h"
#include "AMPLJacobian.h"
#include "AMPLHessian.h"
#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "IpoptEngine.h"
#include "LinearHandler.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "Presolver.h"
#include "Problem.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  double objsense = 1.0;
  int err = 0;
  LinHandlerPtr lhand;
  NlPresHandlerPtr nlhand;
  PresolverPtr pres = PresolverPtr(); // NULL

  // Start timer and read the problem
  env->startTimer(err); assert(err==0);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("expand_quad")->setValue(false);
  env->getOptions()->findDouble("bnb_time_limit")->setValue(3600);

  MINOTAUR_AMPL::AMPLInterface* iface =
    new MINOTAUR_AMPL::AMPLInterface(env, "bnb");
  ProblemPtr p = iface->readInstance(argv[1]);

  lhand = (LinHandlerPtr) new LinearHandler(env, p);
  handlers.push_back(l_hand);
  nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
  handlers.push_back(nlhand);
  pres = (PresolverPtr) new Presolver(p, env, handlers);
  pres->standardize(); 
  pres->solve();
  handlers.clear();

  // create branch-and-bound object
  BranchAndBound *bab = new BranchAndBound(env, p);

  // setup handlers
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

  //rel->calculateSize();
  //if (rel->isQP()) {
  //  rel->setNativeDer();
  //} else {
  //  rel->setJacobian(jac);
  //  rel->setHessian(hess);
  //}
  nr->setRelaxation(rel);
  nr->setEngine(e);
  nr->setModFlag(false);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);

  // start solving
  bab->solve();
  bab->writeStats(std::cout);
  //bab->getSolution()->writePrimal(std::cout);
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
