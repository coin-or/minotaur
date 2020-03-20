//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file McQG.cpp
 * \brief The main function for solving instances in ampl format (.nl) by
 * using a parallel implementation of Advanced Quesada-Grossmann (QG)
 * algorithm for solving convex MINLPs.
 * \author Meenarli Sharma, Prashant Palkar, IIT Bombay
 */

#include <iomanip>
#include <iostream>
#if USE_OPENMP
#include <omp.h>
#else
#error "Cannot compile parallel algorithms: turn USE_OpenMP flag ON."
#endif
#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "LinFeasPump.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "LPEngine.h"
#include "MaxFreqBrancher.h"
#include "MaxVioBrancher.h"
#include "ParMINLPDiving.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "Node.h"
#include "NodeRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "ParNodeIncRelaxer.h"
#include "ParPCBProcessor.h"
#include "ParQGBranchAndBound.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "Problem.h"
#include "QPEngine.h"
#include "RandomBrancher.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "ParReliabilityBrancher.h"
#include "Solution.h"
#include "SOS1Handler.h"
#include "SOS2Handler.h"
#include "ParQGHandlerAdvance.h"
#include "Timer.h"

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"

using namespace Minotaur;
BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e);


ParQGBranchAndBound* createParBab(EnvPtr env, UInt numThreads, NodePtr &node,
                                  RelaxationPtr relCopy[], ProblemPtr pCopy[],
                                  ParPCBProcessorPtr nodePrcssr[],
                                  ParNodeIncRelaxerPtr parNodeRlxr[],
                                  HandlerVector handlersCopy[],
                                  LPEnginePtr lpeCopy[], EnginePtr eCopy[])
{
  ParQGBranchAndBound *bab = new ParQGBranchAndBound(env, pCopy[0]);
  const std::string me("mcqgadv main: ");
  OptionDBPtr options = env->getOptions();
  bab->shouldCreateRoot(false);
 
  for(UInt i = 0; i < numThreads; ++i) {
    BrancherPtr br = 0;
    LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env, pCopy[i]);
    l_hand->setModFlags(false, true);
    handlersCopy[i].push_back(l_hand);
    assert(l_hand);

    IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, pCopy[i]);
    v_hand->setModFlags(false, true);
    handlersCopy[i].push_back(v_hand);
    assert(v_hand);

    ParQGHandlerAdvancePtr qg_hand = (ParQGHandlerAdvancePtr) new ParQGHandlerAdvance(env, pCopy[i], eCopy[i]);
    qg_hand->setModFlags(false, true);
    qg_hand->loadProbToEngine();
    if (i>0) {
      qg_hand->nlCons();
    }
    handlersCopy[i].push_back(qg_hand);
    assert(qg_hand);

    br = createBrancher(env, pCopy[i], handlersCopy[i], lpeCopy[i]);
    nodePrcssr[i] = (ParPCBProcessorPtr) new ParPCBProcessor(env, lpeCopy[i], handlersCopy[i]);
    nodePrcssr[i]->setBrancher(br);
    parNodeRlxr[i] = (ParNodeIncRelaxerPtr) new ParNodeIncRelaxer(env, handlersCopy[i]);
    if (i==0) {
      node = (NodePtr) new Node ();
      bool prune = false;
      relCopy[0] = parNodeRlxr[0]->createRootRelaxation(node, prune);
    } else {
      relCopy[i] = (RelaxationPtr) new Relaxation(relCopy[0], env);
      parNodeRlxr[i]->setRelaxation(relCopy[i]);
      qg_hand->setRelaxation(relCopy[i]);
      qg_hand->setObjVar();
    }
    relCopy[i]->setProblem(pCopy[i]);
    parNodeRlxr[i]->setModFlag(false);
    parNodeRlxr[i]->setEngine(lpeCopy[i]);
  }
  
  // when using heuristic, check if engine copy[0] should be cleared etc.
  if (options->findBool("pardivheur")->getValue()) {
    ParMINLPDivingPtr div_heur;
    if (options->findBool("divheurLP")->getValue()) {
      RelaxationPtr lp = (RelaxationPtr) new Relaxation(relCopy[0], env);
      lp->setNativeDer();
      div_heur = (ParMINLPDivingPtr) new ParMINLPDiving(env, lp, lpeCopy[0]);
      div_heur->setAltEngine(eCopy[0]);
      div_heur->setOrigProb(pCopy[0]);
    }
    else {
      div_heur = (ParMINLPDivingPtr) new ParMINLPDiving(env, pCopy[0], eCopy[0]);
    }
    bab->addPreRootHeur(div_heur);
  }
  return bab;
}


BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e)
{
  BrancherPtr br = 0;
  UInt t;
  const std::string me("mcqgadv main: ");

  if (env->getOptions()->findString("brancher")->getValue() == "rel") {
    ReliabilityBrancherPtr rel_br;
    rel_br = (ReliabilityBrancherPtr) new ReliabilityBrancher(env, handlers);
    rel_br->setEngine(e);
    t = (p->getSize()->ints + p->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    rel_br->setThresh(t);
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "setting reliability threshhold to " << t << std::endl;
    t = (UInt) p->getSize()->ints + p->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    rel_br->setMaxDepth(t);
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="Filter-SQP") {
      rel_br->setIterLim(5);
    }
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "reliability branching iteration limit = " <<
      rel_br->getIterLim() << std::endl;
    br = rel_br;
  } else if (env->getOptions()->findString("brancher")->getValue() == "parRel") {
    ParReliabilityBrancherPtr parRel_br;
    parRel_br = (ParReliabilityBrancherPtr) new ParReliabilityBrancher(env, handlers);
    parRel_br->setEngine(e);
    t = (p->getSize()->ints + p->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    parRel_br->setThresh(t);
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "setting reliability threshhold to " << t << std::endl;
    t = (UInt) p->getSize()->ints + p->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    parRel_br->setMaxDepth(t);
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="Filter-SQP") {
      parRel_br->setIterLim(5);
    }
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "reliability branching iteration limit = " <<
      parRel_br->getIterLim() << std::endl;
    br = parRel_br;
  } else if (env->getOptions()->findString("brancher")->getValue() ==
             "maxvio") {
    br = (MaxVioBrancherPtr) new MaxVioBrancher(env, handlers);
  } else if (env->getOptions()->findString("brancher")->getValue() == "lex") {
    br = (LexicoBrancherPtr) new LexicoBrancher(env, handlers);
  } else if (env->getOptions()->findString("brancher")->getValue() == "rand") {
    br = (RandomBrancherPtr) new RandomBrancher(env, handlers);
  } else if (env->getOptions()->findString("brancher")->getValue() ==
             "maxfreq") {
    br = (MaxFreqBrancherPtr) new MaxFreqBrancher(env, handlers);
  }
  env->getLogger()->msgStream(LogExtraInfo) << me <<
    "brancher used = " << br->getName() << std::endl;
  return br;
}


EnginePtr getEngine(EnvPtr env, ProblemPtr p, int &err)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL
  bool cont=false;
  const std::string me("mcqgadv main: ");

  err = 0;
  p->calculateSize();
  if (p->isLinear()) {
    e = efac->getLPEngine();
    if (!e) {
      cont = true;
    }
  }

  if (true==cont || p->isQP()) {
    e = efac->getQPEngine();
    if (!e) {
      cont = true;
    }
  }

  if (!e) {
    e = efac->getNLPEngine();
  }

  if (!e) {
    env->getLogger()->errStream() << "No engine available for this problem."
                                  << std::endl << "exiting without solving"
                                  << std::endl;
    err = 1;
  } else {
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "engine used = " << e->getName() << std::endl;
  }
  delete efac;
  return e;
}


void loadProblem(EnvPtr env, MINOTAUR_AMPL::AMPLInterface* iface,
                 ProblemPtr &oinst, double *obj_sense)
{
  Timer *timer     = env->getNewTimer();
  OptionDBPtr options = env->getOptions();
  JacobianPtr jac;
  HessianOfLagPtr hess;
  const std::string me("mcqgadv main: ");

  timer->start();
  oinst = iface->readInstance(options->findString("problem_file")->getValue());
  env->getLogger()->msgStream(LogInfo) << me 
    << "time used in reading instance (s) = " << std::fixed 
    << std::setprecision(2) << timer->query() << std::endl;

  // display the problem
  oinst->calculateSize();
  if (options->findBool("display_problem")->getValue()==true) {
    oinst->write(env->getLogger()->msgStream(LogNone), 12);
  }
  if (options->findBool("display_size")->getValue()==true) {
    oinst->writeSize(env->getLogger()->msgStream(LogNone));
  }
  // create the jacobian
  if (false==options->findBool("use_native_cgraph")->getValue()) {
    jac = (MINOTAUR_AMPL::AMPLJacobianPtr) 
      new MINOTAUR_AMPL::AMPLJacobian(iface);
    oinst->setJacobian(jac);

    // create the hessian
    hess = (MINOTAUR_AMPL::AMPLHessianPtr)
      new MINOTAUR_AMPL::AMPLHessian(iface);
    oinst->setHessian(hess);
  }

  // set initial point
  oinst->setInitialPoint(iface->getInitialPoint(), 
      oinst->getNumVars()-iface->getNumDefs());

  if (oinst->getObjective() &&
      oinst->getObjective()->getObjectiveType()==Maximize) {
    *obj_sense = -1.0;
    env->getLogger()->msgStream(LogInfo) << me 
      << "objective sense: maximize (will be converted to Minimize)"
      << std::endl;
  } else {
    *obj_sense = 1.0;
    env->getLogger()->msgStream(LogInfo) << me 
      << "objective sense: minimize" << std::endl;
  }

  delete timer;
}


void overrideOptions(EnvPtr env)
{
  env->getOptions()->findString("interface_type")->setValue("AMPL");
}


PresolverPtr presolve(EnvPtr env, ProblemPtr p, size_t ndefs, 
                      HandlerVector &handlers)
{
  PresolverPtr pres = PresolverPtr(); // NULL
  const std::string me("mcqgadv main: ");

  p->calculateSize();
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    LinHandlerPtr lhandler = (LinHandlerPtr) new LinearHandler(env, p);
    handlers.push_back(lhandler);
    if (p->isQP() || p->isQuadratic() || p->isLinear() ||
        true==env->getOptions()->findBool("use_native_cgraph")->getValue()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    if (ndefs>0) {
      lhandler->setPreOptDualFix(false);
    } else {
      lhandler->setPreOptDualFix(true);
    }

    if (!p->isLinear() && 
         true==env->getOptions()->findBool("use_native_cgraph")->getValue() && 
         true==env->getOptions()->findBool("nl_presolve")->getValue() 
       ) {
      NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
      handlers.push_back(nlhand);
    }

    // write the names.
    env->getLogger()->msgStream(LogExtraInfo) << me 
      << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end();
        ++h) {
      env->getLogger()->msgStream(LogExtraInfo) << me 
        << (*h)->getName() << std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(p, env, handlers);
  pres->standardize(); 
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
  return pres;
}


void setInitialOptions(EnvPtr env)
{
  env->getOptions()->findBool("presolve")->setValue(true);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(true);
  //env->getOptions()->findString("brancher")->setValue("parRel");
  //env->getOptions()->findString("nlp_engine")->setValue("IPOPT");
  //env->getOptions()->findString("qp_engine")->setValue("None");
}


void showHelp()
{
  std::cout << "Quesada-Grossmann (LP/NLP) parallel algorithm for convex MINLP"
            << std::endl
            << "Requires a thread-safe NLP and LP solver."
            //<< "(e.g. IPOPT with MA97)**" << std::endl
            << std::endl
            << "Usage:" << std::endl
            << "To show version: mcqgadv -v (or --show_version yes) "
            << std::endl
            << "To show all options: mcqgadv -= (or --show_options yes)"
            << std::endl
            << "To solve an instance: mcqgadv --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("mcqgadv main: ");

  if (options->findBool("display_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    return 1;
  }

  if (options->findBool("display_help")->getValue() ||
      options->findFlag("?")->getValue()) {
    showHelp();
    return 1;
  }

  if (options->findBool("display_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    env->getLogger()->msgStream(LogNone) << me << "Minotaur version "
      << env->getVersion() << std::endl << me 
      << "NLP-based parallel branch-and-bound solver for convex MINLP"
      << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env->getLogger()->msgStream(LogInfo)
    << me << "Minotaur version " << env->getVersion() << std::endl
    << me << "NLP-based parallel branch-and-bound solver for convex MINLP" << std::endl;
  return 0;
}


void writeSol(EnvPtr env, VarVector *orig_v,
              PresolverPtr pres, SolutionPtr sol, SolveStatus status,
              MINOTAUR_AMPL::AMPLInterface* iface)
{
  Solution* final_sol = 0;
  if (sol) {
    final_sol = pres->getPostSol(sol);
  }

  if (env->getOptions()->findFlag("AMPL")->getValue() ||
      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(final_sol, status);
  } else if (final_sol && env->getLogger()->getMaxLevel()>=LogExtraInfo &&
             env->getOptions()->findBool("display_solution")->getValue()) {
    final_sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
  }

  if (final_sol) {
    delete final_sol;
  }
}


void writeBnbStatus(EnvPtr env, BranchAndBound *bab, double obj_sense)
{

  const std::string me("mcqgadv main: ");
  int err = 0;

  if (bab) {
    env->getLogger()->msgStream(LogInfo)
      << me << std::fixed << std::setprecision(4) 
      << "best solution value = " << obj_sense*bab->getUb() << std::endl
      << me << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = "
      <<  obj_sense*bab->getLb() << std::endl
      << me << "gap = " << std::max(0.0,bab->getUb() - bab->getLb())
      << std::endl
      << me << "gap percentage = " << bab->getPerGap() << std::endl
      << me << "time used (s) = " << std::fixed << std::setprecision(2) 
      << env->getTime(err) << std::endl
      << me << "status of branch-and-bound: " 
      << getSolveStatusString(bab->getStatus()) << std::endl;
    env->stopTimer(err); assert(0==err);
  } else {
    env->getLogger()->msgStream(LogInfo)
      << me << std::fixed << std::setprecision(4)
      << "best solution value = " << INFINITY << std::endl
      << me << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = " << INFINITY << std::endl
      << me << "gap = " << INFINITY << std::endl
      << me << "gap percentage = " << INFINITY << std::endl
      << me << "time used (s) = " << std::fixed << std::setprecision(2) 
      << env->getTime(err) << std::endl 
      << me << "status of branch-and-bound: " 
      << getSolveStatusString(NotStarted) << std::endl;
    env->stopTimer(err); assert(0==err);
  }
}

void writeParBnbStatus(EnvPtr env, ParQGBranchAndBound *parbab, double obj_sense,
                       double wallTimeStart)
{

  const std::string me("mcqgadv main: ");
  int err = 0;

  if (parbab) {
    env->getLogger()->msgStream(LogInfo)
      << me << std::fixed << std::setprecision(4) 
      << "best solution value = " << obj_sense*parbab->getUb() << std::endl
      << me << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = "
      <<  obj_sense*parbab->getLb() << std::endl
      << me << "gap = " << std::max(0.0,parbab->getUb() - parbab->getLb())
      << std::endl
      << me << "gap percentage = " << parbab->getPerGap() << std::endl
      << me << "time used (s) = " << std::fixed << std::setprecision(2) 
      << parbab->getWallTime() - wallTimeStart << std::endl
      << me << "status of branch-and-bound: " 
      << getSolveStatusString(parbab->getStatus()) << std::endl;
    env->stopTimer(err); assert(0==err);
  } else {
    env->getLogger()->msgStream(LogInfo)
      << me << std::fixed << std::setprecision(4)
      << "best solution value = " << INFINITY << std::endl
      << me << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = " << INFINITY << std::endl
      << me << "gap = " << INFINITY << std::endl
      << me << "gap percentage = " << INFINITY << std::endl
      << me << "time used (s) = " << std::fixed << std::setprecision(2) 
      << env->getTime(err) << std::endl 
      << me << "status of branch-and-bound: " 
      << getSolveStatusString(NotStarted) << std::endl;
    env->stopTimer(err); assert(0==err);
  }
}


void writeParQGStats(EnvPtr env, ParQGBranchAndBound *parbab, UInt numThreads,
                     HandlerVector handlersCopy[])
{
  if (parbab) {
    const std::string me("ParQGHandlerAdvance: ");
    UInt nlpSolved = 0, nlpInf = 0, nlpFeas = 0, nlpItLim = 0, numCuts = 0,
         numLinCuts = 0, numFracCuts = 0;
    for (UInt i=0; i < numThreads; i++) {
      for (HandlerVector::iterator it=handlersCopy[i].begin(); it!=handlersCopy[i].end(); ++it) {
        if ((*it)->getName() == "ParQGHandlerAdvance (Quesada-Grossmann)") {
          ParQGHandlerAdvancePtr parqgHand = dynamic_cast <ParQGHandlerAdvance*> (*it);
          nlpSolved += parqgHand->getStats()->nlpS;
          nlpInf += parqgHand->getStats()->nlpI;
          nlpFeas += parqgHand->getStats()->nlpF;
          nlpItLim += parqgHand->getStats()->nlpIL;
          numCuts += parqgHand->getStats()->cuts;
          numFracCuts += parqgHand->getStats()->fracCuts;
          if (i == 0) {
            numLinCuts += parqgHand->getStats()->rcuts;
          }
        }
      }
    }

    env->getLogger()->msgStream(LogInfo)
      << me << "number of nlps solved                       = "
      << nlpSolved << std::endl
      << me << "number of infeasible nlps                   = "
      << nlpInf << std::endl
      << me << "number of feasible nlps                     = "
      << nlpFeas << std::endl
      << me << "number of nlps hit engine iterations limit  = "
      << nlpItLim << std::endl
      << me << "number of extra root Linearizations         = "
      << numLinCuts << std::endl
      << me << "number of fractional cuts added             = "
      << numFracCuts << std::endl
      << me << "number of cuts added                        = "
      << numCuts << std::endl;
  }
}


void writeLPStats(EnvPtr env, std::string name, std::vector<double> stats) {
  if (stats.size()) {
    std::string me = name + ": ";
    env->getLogger()->msgStream(LogExtraInfo)
      << me << "total calls            = " << UInt(stats[0]) << std::endl
      << me << "strong branching calls = " << UInt(stats[1]) << std::endl
      << me << "total time in solving  = " << stats[2] << std::endl
      << me << "time in str branching  = " << stats[3] << std::endl
      << me << "total iterations       = " << UInt(stats[4]) << std::endl
      << me << "strong br iterations   = " << UInt(stats[5]) << std::endl;
  }
}


void writeNLPStats(EnvPtr env, std::string name, std::vector<double> stats) {
  if (stats.size()) {
    std::string me = name + ": ";
    env->getLogger()->msgStream(LogExtraInfo)
      << me << "total calls            = " << UInt(stats[0]) << std::endl
      << me << "calls to Optimize      = " << UInt(stats[1]) << std::endl
      << me << "calls to ReOptimize    = " << UInt(stats[2]) << std::endl
      << me << "strong branching calls = " << UInt(stats[3]) << std::endl
      << me << "total time in solving  = " << stats[4] << std::endl
      << me << "total time in presolve = " << stats[5] << std::endl
      << me << "time in str branching  = " << stats[6] << std::endl
      << me << "total iterations       = " << UInt(stats[7]) << std::endl
      << me << "strong br iterations   = " << UInt(stats[8]) << std::endl;
  }
}


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  MINOTAUR_AMPL::AMPLInterface* iface = 0;
  ProblemPtr oinst;      // instance that needs to be solved.
  EnginePtr engine = 0;  // engine for solving relaxations. 
  ParQGBranchAndBound * parbab = 0;
  double wallTimeStart = parbab->getWallTime();  //use Timer: to be done!!!
  PresolverPtr pres = 0;
  const std::string me("mcqgadv main: ");
  VarVector *orig_v=0;
  HandlerVector handlers;
  int err = 0;
  double obj_sense = 1.0;
  
  UInt numThreads = 0;
  ParPCBProcessorPtr *nodePrcssr = 0; 
  ParNodeIncRelaxerPtr *parNodeRlxr = 0;
  ProblemPtr *pCopy = 0;
  RelaxationPtr *relCopy = 0;
  HandlerVector *handlersCopy = 0;
  EngineFactory *efac = 0;
  LPEnginePtr *lpeCopy = 0;
  EnginePtr *eCopy = 0;
  ObjectivePtr oPtr = 0;
  NodePtr node = 0;
  std::string name = "";

  std::vector<double> lpStats(6,0);
  std::vector<double> nlpStats(9,0);
 
  env->startTimer(err);

  if (err) {
    goto CLEANUP;
  }

  setInitialOptions(env);

  // Important to setup AMPL Interface first as it adds several options.
  iface = new MINOTAUR_AMPL::AMPLInterface(env, "mcqg");

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);

  overrideOptions(env);
  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

  loadProblem(env, iface, oinst, &obj_sense);

  orig_v = new VarVector(oinst->varsBegin(), oinst->varsEnd());
  pres = presolve(env, oinst, iface->getNumDefs(), handlers);
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }
  handlers.clear();
  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo) << me 
      << "status of presolve: " 
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol(env, orig_v, pres, SolutionPtr(), pres->getStatus(), iface);
    writeParBnbStatus(env, parbab, obj_sense, wallTimeStart);
    goto CLEANUP;
  }

  if (true==env->getOptions()->findBool("use_native_cgraph")->getValue()) {
    oinst->setNativeDer();
  }

  if (false==env->getOptions()->findBool("solve")->getValue()) {
    goto CLEANUP;
  }

  engine = getEngine(env, oinst, err);
  if (err) {
    goto CLEANUP;
  }

  numThreads = std::min(env->getOptions()->findInt("threads")->getValue(),
                        omp_get_num_procs());
  nodePrcssr = new ParPCBProcessorPtr[numThreads];
  parNodeRlxr = new ParNodeIncRelaxerPtr[numThreads];
  relCopy = new RelaxationPtr[numThreads];
  pCopy = new ProblemPtr[numThreads];
  handlersCopy = new HandlerVector[numThreads];
  efac = new EngineFactory(env);
  lpeCopy = new LPEnginePtr[numThreads];
  eCopy = new EnginePtr[numThreads];

  // If objective is nonlinear add an extra var name eta to move objective to
  // constraint in ParQGHandlerAdvance
  pCopy[0] = oinst->clone(env);
  oPtr = oinst->getObjective();
  if (!oPtr) {
    assert(!"No objective function in the problem!");
  } else if (oPtr->getFunctionType() != Linear &&
             oPtr->getFunctionType() != Constant) {
    name = "eta";
    pCopy[0]->newVariable(-INFINITY,INFINITY,Continuous,name);
  }

  for(UInt i=0; i < numThreads; ++i) {
    lpeCopy[i] = efac->getLPEngine();
    eCopy[i] = engine->emptyCopy();
    if (i > 0) {
      pCopy[i] = pCopy[0]->clone(env);
    }
  }

  if (numThreads > 1) {
    env->getLogger()->msgStream(LogInfo)
      << "Number of threads = " << numThreads 
      << ". Requires a thread-safe LP and NLP solver." << std::endl;
  }
  parbab = createParBab(env, numThreads, node, relCopy, pCopy, nodePrcssr,
                        parNodeRlxr, handlersCopy, lpeCopy, eCopy);
  parbab->parsolve(parNodeRlxr, nodePrcssr, numThreads);
  
  //Take care of important bnb statistics
  //parbab->writeParStats(env->getLogger()->msgStream(LogExtraInfo), nodePrcssr);
  
  //Take care of important engine statistics
  for (UInt i=0; i < numThreads; i++) {
    lpeCopy[i]->fillStats(lpStats);
    eCopy[i]->fillStats(nlpStats);
  }
  writeLPStats(env, lpeCopy[0]->getName(), lpStats);
  writeNLPStats(env, eCopy[0]->getName(), nlpStats);
  
  //Take care of important handler statistics
  //for (UInt i=0; i < numThreads; i++) {
    //env->getLogger()->msgStream(LogExtraInfo) << "Thread " << i << std::endl;
    //for (HandlerVector::iterator it=handlersCopy[i].begin(); it!=handlersCopy[i].end(); ++it) {
      //(*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    //}
  //}

  writeSol(env, orig_v, pres, parbab->getSolution(), parbab->getStatus(), iface);
  writeParQGStats(env, parbab, numThreads, handlersCopy);
  writeParBnbStatus(env, parbab, obj_sense, wallTimeStart);

CLEANUP:
  if (engine) {
    delete engine;
  }
  if (efac) {
    delete efac;
  }
  if (iface) {
    delete iface;
  }
  if (pres) {
    delete pres;
  }
  for (UInt i=0; i < numThreads; i++) {
    for (HandlerVector::iterator it=handlersCopy[i].begin();
         it!=handlersCopy[i].end(); ++it) {
      delete (*it);
    }
    if (lpeCopy[i]) {
      delete lpeCopy[i];
    }
    if (eCopy[i]) {
      delete eCopy[i];
    }
    if (pCopy[i]) {
      delete pCopy[i];
    }
    if (parNodeRlxr[i]) {
      delete parNodeRlxr[i];
      relCopy[i] = 0;
    }
    if (nodePrcssr[i]) {
      delete nodePrcssr[i];
    }
  }
  if (node) {
    delete node;
  }
  if (nodePrcssr) {
    delete[] nodePrcssr;
  }
  if (parNodeRlxr) {
    delete[] parNodeRlxr;
  }
  if (relCopy) {
    delete[] relCopy;
  }
  if (pCopy) {
    delete[] pCopy;
  }
  if (eCopy) {
    delete[] eCopy;
  }
  if (handlersCopy) {
    delete[] lpeCopy;
  }
  if (handlersCopy) {
    delete[] handlersCopy;
  }
  if (parbab) {
    if (parbab->getNodeRelaxer()) {
      delete parbab->getNodeRelaxer();
    }
    if (parbab->getNodeProcessor()) {
      delete parbab->getNodeProcessor();
    }
    delete parbab;
  }
  if (oinst) {
    delete oinst;
  }
  if (orig_v) {
    delete orig_v;
  }
  if (env) {
    delete env;
  }
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
