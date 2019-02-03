//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file McBnb.cpp
 * \brief The main function for solving instances in ampl format (.nl) by
 * using a parallel implementation of Branch-and-Bound.
 * \author Prashant Palkar, IIT Bombay
 */

#include <iomanip>
#include <iostream>
#if USE_OPENMP
#include <omp.h>
#endif
#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "ParBndProcessor.h"
#include "BranchAndBound.h"
#include "ParBranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "LinFeasPump.h"
#include "Logger.h"
#include "LPEngine.h"
#include "MaxFreqBrancher.h"
#include "MaxVioBrancher.h"
#include "MINLPDiving.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "ParNodeIncRelaxer.h"
#include "PCBProcessor.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "Problem.h"
#include "QPEngine.h"
#include "RandomBrancher.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "SOS1Handler.h"
#include "SOS2Handler.h"
#include "Timer.h"

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"

using namespace Minotaur;
BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e);


ParBranchAndBound* createParBab(EnvPtr env, ProblemPtr p, EnginePtr e,
                                UInt numThreads,
                                RelaxationPtr relCopy[],
                                ParBndProcessorPtr nodePrcssr[],
                                ParNodeIncRelaxerPtr parNodeRlxr[])
{
  ParBranchAndBound *bab = new ParBranchAndBound(env, p);
  const std::string me("mcbnb main: ");
  OptionDBPtr options = env->getOptions();
  bab->shouldCreateRoot(false);
#if USE_OPENMP
  #pragma omp parallel for
#endif
  for(UInt i = 0; i < numThreads; i++) {
    BrancherPtr br;
    EnginePtr eCopy = e->emptyCopy();
    HandlerVector handlersCopy;
    IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
    LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env, p);
    NlPresHandlerPtr nlhand;
    SOS1HandlerPtr s_hand = (SOS1HandlerPtr) new SOS1Handler(env, p);
    SOS2HandlerPtr s2_hand;
    
    if (s_hand->isNeeded()) {
      s_hand->setModFlags(false, true);
      handlersCopy.push_back(s_hand);
    }

    // add SOS2 handler here.
    s2_hand = (SOS2HandlerPtr) new SOS2Handler(env, p);
    if (s2_hand->isNeeded()) {
      s2_hand->setModFlags(false, true);
      handlersCopy.push_back(s2_hand);
    }

    handlersCopy.push_back(v_hand);
    if (true==options->findBool("presolve")->getValue()) {
      l_hand->setModFlags(false, true);
      handlersCopy.push_back(l_hand);
    }
    if (!p->isLinear() && 
        true==options->findBool("presolve")->getValue() &&
        true==options->findBool("use_native_cgraph")->getValue() &&
        true==options->findBool("nl_presolve")->getValue()) {
      nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
      nlhand->setModFlags(false, true);
      handlersCopy.push_back(nlhand);
    }

    br = createBrancher(env, p, handlersCopy, eCopy);
    relCopy[i] = (RelaxationPtr) new Relaxation(p);
    relCopy[i]->calculateSize();
    if (options->findBool("use_native_cgraph")->getValue() ||
        relCopy[i]->isQP() || relCopy[i]->isQuadratic()) {
      relCopy[i]->setNativeDer();
    } else {
      relCopy[i]->setJacobian(p->getJacobian());
      relCopy[i]->setHessian(p->getHessian());
    }
    relCopy[i]->setInitialPoint(p->getInitialPoint());

    nodePrcssr[i] = (ParBndProcessorPtr) new ParBndProcessor(env, eCopy, handlersCopy);
    nodePrcssr[i]->setBrancher(br);

    parNodeRlxr[i] = (ParNodeIncRelaxerPtr) new ParNodeIncRelaxer(env, handlersCopy);
    parNodeRlxr[i]->setModFlag(false);
    parNodeRlxr[i]->setRelaxation(relCopy[i]);
    parNodeRlxr[i]->setEngine(eCopy);
  }

  if (0 <= options->findInt("divheur")->getValue()) {
    MINLPDivingPtr div_heur;
    EnginePtr e2 = e->emptyCopy();
    if (true==options->findBool("use_native_cgraph")->getValue() ||
        relCopy[0]->isQP() || relCopy[0]->isQuadratic()) {
      p->setNativeDer();
    }
    div_heur = (MINLPDivingPtr) new MINLPDiving(env, p, e2);
    bab->addPreRootHeur(div_heur);
  }
  if (true == options->findBool("FPump")->getValue()) {
    EngineFactory efac(env);
    EnginePtr lpe = efac.getLPEngine();
    EnginePtr nlpe = e->emptyCopy();
    LinFeasPumpPtr lin_feas_pump = (LinFeasPumpPtr) 
      new LinFeasPump(env, p, nlpe, lpe);
    bab->addPreRootHeur(lin_feas_pump);
  }
  return bab;
}

BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e)
{
  BrancherPtr br;
  UInt t;
  const std::string me("mcbnb main: ");

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
  const std::string me("mcbnb main: ");

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
  const std::string me("mcbnb main: ");

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
  const std::string me("mcbnb main: ");

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
    for (HandlerVector::iterator h=handlers.begin(); h!=handlers.end(); ++h) {
      (*h)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    }
  }
  return pres;
}


void setInitialOptions(EnvPtr env)
{
  env->getOptions()->findBool("presolve")->setValue(true);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(true);
}


void showHelp()
{
  std::cout << "NLP-based parallel branch-and-bound solver for convex MINLP"
            << std::endl
            << "**Works in parallel with a thread-safe NLP solver only "
            << "(e.g. IPOPT with MA97)**" << std::endl
            << "Usage:" << std::endl
            << "To show version: mcbnb -v (or --display_version yes) "
            << std::endl
            << "To show all options: mcbnb -= (or --display_options yes)"
            << std::endl
            << "To solve an instance: mcbnb --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("mcbnb main: ");

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
  if (sol) {
    sol = pres->getPostSol(sol);
  }

  if (env->getOptions()->findFlag("AMPL")->getValue() ||
      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(sol, status);
  } else if (sol && env->getLogger()->getMaxLevel()>=LogExtraInfo) {
    sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
  }
}


void writeBnbStatus(EnvPtr env, BranchAndBound *bab, double obj_sense)
{

  const std::string me("mcbnb main: ");
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

void writeParBnbStatus(EnvPtr env, ParBranchAndBound *parbab, double obj_sense,
                       double wallTimeStart)
{

  const std::string me("mcbnb main: ");
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


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  OptionDBPtr options;
  MINOTAUR_AMPL::AMPLInterface* iface = 0;
  ProblemPtr oinst;    // instance that needs to be solved.
  EnginePtr engine;    // engine for solving relaxations. 
  SolutionPtr sol, sol2;
  JacobianPtr jPtr;
  HessianOfLagPtr hPtr;
  ParBranchAndBound * parbab = 0;
  double WallTimeStart = parbab->getWallTime();  //use Timer: to be done!!!
  PresolverPtr pres;
  const std::string me("mcbnb main: ");
  VarVector *orig_v=0;
  HandlerVector handlers;
  int err = 0;
  double obj_sense = 1.0;
  
  UInt numThreads;
  ParBndProcessorPtr *nodePrcssr = 0; 
  ParNodeIncRelaxerPtr *parNodeRlxr = 0;
  RelaxationPtr *relCopy = 0;
  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  setInitialOptions(env);

  // Important to setup AMPL Interface first as it adds several options.
  iface = new MINOTAUR_AMPL::AMPLInterface(env, "mcbnb");

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);

  overrideOptions(env);
  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

#if USE_OPENMP
  numThreads = std::min(env->getOptions()->findInt("threads")->getValue(),
                        omp_get_num_procs());
#else
  numThreads = 1;
#endif
  nodePrcssr = new ParBndProcessorPtr[numThreads];
  parNodeRlxr = new ParNodeIncRelaxerPtr[numThreads];
  relCopy = new RelaxationPtr[numThreads]; 
  loadProblem(env, iface, oinst, &obj_sense);
  orig_v = new VarVector(oinst->varsBegin(), oinst->varsEnd());
  pres = presolve(env, oinst, iface->getNumDefs(), handlers);
  handlers.clear();
  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo) << me 
      << "status of presolve: " 
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol(env, orig_v, pres, SolutionPtr(), pres->getStatus(), iface);
    writeParBnbStatus(env, parbab, obj_sense, WallTimeStart);
    goto CLEANUP;
  }

  if (false==env->getOptions()->findBool("solve")->getValue()) {
    goto CLEANUP;
  }

  engine = getEngine(env, oinst, err);
  if (err) {
    goto CLEANUP;
  }
  if (numThreads > 1) {
    env->getLogger()->msgStream(LogInfo)
      << "**Works in parallel with a thread-safe "
      << "NLP solver only (e.g. IPOPT with MA97)**" << std::endl;
  }
  parbab = createParBab(env, oinst, engine, numThreads, relCopy,
                        nodePrcssr, parNodeRlxr);
  if (true==env->getOptions()->findBool("mcbnb_deter_mode")->getValue()) {
    parbab->parsolveSync(parNodeRlxr, nodePrcssr, numThreads);
  } else {
    parbab->parsolve(parNodeRlxr, nodePrcssr, numThreads);
  }
  
  //Take care of important bnb statistics: to be done!!!
  //parbab->writeParStats(env->getLogger()->msgStream(LogExtraInfo), nodePrcssr);
  
  //Take care of important engine statistics: to be done!!!
  //engine->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  
  //Take care of important handler statistics: to be done!!!
  //for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    //(*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  //}

  writeSol(env, orig_v, pres, parbab->getSolution(), parbab->getStatus(), iface);
  writeParBnbStatus(env, parbab, obj_sense, WallTimeStart);

CLEANUP:
  if (iface) {
    delete iface;
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
  if (parbab) {
    delete parbab;
  }
  if (orig_v) {
    delete orig_v;
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
