//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
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
#include "IntVarHandler.h"
#include "IpoptEngine.h"
#include "LinearHandler.h"
#include "PCBProcessor.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "OsiLPEngine.h"
#include "Problem.h"
#include "QGHandler.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Timer.h"
#include "AMPLHessian.h"
#include "AMPLJacobian.h"
#include "AMPLInterface.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  HandlerVector handlers;
  double objsense = 1.0;
  int err = 0;

  // Start timer and read the problem
  //env->startTimer(err); assert(err==0);
  env->getOptions()->findBool("use_native_cgraph")->setValue(false);
  env->getOptions()->findBool("expand_quad")->setValue(false);
  //env->getOptions()->findDouble("bnb_time_limit")->setValue(3600);

  MINOTAUR_AMPL::AMPLInterface* iface =
    new MINOTAUR_AMPL::AMPLInterface(env, "qg");
  ProblemPtr p = iface->readInstance(argv[1]);
  if (Maximize == p->getObjective()->getObjectiveType()) {
    p->negateObj();
    objsense = 1.0;
  }
  MINOTAUR_AMPL::AMPLJacobianPtr jac = (MINOTAUR_AMPL::AMPLJacobianPtr) new MINOTAUR_AMPL::AMPLJacobian(iface);
  p->setJacobian(jac);
  MINOTAUR_AMPL::AMPLHessianPtr hess = (MINOTAUR_AMPL::AMPLHessianPtr) new MINOTAUR_AMPL::AMPLHessian(iface);
  p->setHessian(hess);


  // create branch-and-bound object
  BranchAndBound *bab = new BranchAndBound(env, p);
  EnginePtr nlp_e = (IpoptEnginePtr) new IpoptEngine(env);
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
  NodeProcessorPtr nproc = (PCBProcessorPtr) new PCBProcessor(env, e, handlers);
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
  //bab->getSolution()->writePrimal(std::cout);
  std::cout << "best solution value = " << objsense*bab->getUb() << std::endl
    << "status of branch-and-bound: "
    << getSolveStatusString(bab->getStatus()) << std::endl;

  //finish
  delete iface;
  delete bab;
  return 0;
}


