//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file MultiStart.cpp
 * \brief The class for solving nonconvex problems using multistart
 * Branch-and-estimate heuristic.
 * \author Prashant Palkar and Ashutosh Mahajan, IIT Bombay 
 */

#include <iomanip>
#include <iostream>
#include <sys/time.h>

#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
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
#include "MsProcessor.h"
#include "MultiStart.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "PCBProcessor.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "QPEngine.h"
#include "Problem.h"
#include "RandomBrancher.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "SOS1Handler.h"
#include "SOS2Handler.h"
#include "Timer.h"
#include "TreeManager.h"

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"

using namespace Minotaur;
const std::string MultiStart::me_ = "MultiStart: ";

MultiStart::MultiStart(EnvPtr env) 
: objSense_(1.0),
  status_(NotStarted)
{
  env_ = env;
  iface_ = 0;
}


MultiStart::~MultiStart() 
{
}


BranchAndBound* MultiStart::getBab_(EnginePtr e, HandlerVector &handlers)
{
  BranchAndBound *bab = new BranchAndBound(env_, oinst_);
  NodeProcessorPtr nproc = NodeProcessorPtr(); // NULL
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env_, oinst_);
  LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env_, oinst_);
  NlPresHandlerPtr nlhand;
  NodeIncRelaxerPtr nr;
  RelaxationPtr rel;
  BrancherPtr br;
  OptionDBPtr options = env_->getOptions();
  SOS2HandlerPtr s2_hand;


  SOS1HandlerPtr s_hand = (SOS1HandlerPtr) new SOS1Handler(env_, oinst_);
  if (s_hand->isNeeded()) {
    s_hand->setModFlags(false, true);
    handlers.push_back(s_hand);
  }

  // add SOS2 handler here.
  s2_hand = (SOS2HandlerPtr) new SOS2Handler(env_, oinst_);
  if (s2_hand->isNeeded()) {
    s2_hand->setModFlags(false, true);
    handlers.push_back(s2_hand);
  }


  handlers.push_back(v_hand);
  if (true==options->findBool("presolve")->getValue()) {
    l_hand->setModFlags(false, true);
    handlers.push_back(l_hand);
  }
  if (!oinst_->isLinear() && 
      true==options->findBool("presolve")->getValue() &&
      true==options->findBool("use_native_cgraph")->getValue() &&
      true==options->findBool("nl_presolve")->getValue()) {
    nlhand = (NlPresHandlerPtr) new NlPresHandler(env_, oinst_);
    nlhand->setModFlags(false, true);
    handlers.push_back(nlhand);
  }
  //if (handlers.size()>1) {
    //nproc = (PCBProcessorPtr) new PCBProcessor(env_, e, handlers);
  //} else {
    //nproc = (BndProcessorPtr) new BndProcessor(env_, e, handlers);
  //}
  // Using only the MsProcessor
  nproc = (MsProcessorPtr) new MsProcessor(env_, e, handlers);

  br = getBrancher_(handlers, e);
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env_, handlers);
  nr->setModFlag(false);
  rel = (RelaxationPtr) new Relaxation(oinst_, env_);
  rel->calculateSize();	
  if (options->findBool("use_native_cgraph")->getValue() ||
      rel->isQP() || rel->isQuadratic()) {
    rel->setNativeDer();
  } else {
    rel->setJacobian(oinst_->getJacobian());	
    rel->setHessian(oinst_->getHessian());
  }
  nr->setRelaxation(rel);
  nr->setEngine(e);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);

  if (0 <= options->findInt("divheur")->getValue()) {
    MINLPDivingPtr div_heur;
    EnginePtr e2 = e->emptyCopy();
    if (true==options->findBool("use_native_cgraph")->getValue() ||
        rel->isQP() || rel->isQuadratic()) {
      oinst_->setNativeDer();
    }
    div_heur = (MINLPDivingPtr) new MINLPDiving(env_, oinst_, e2);
    bab->addPreRootHeur(div_heur);
  }
  if (true == options->findBool("FPump")->getValue()) {
    EngineFactory efac(env_);
    EnginePtr lpe = efac.getLPEngine();
    EnginePtr nlpe = e->emptyCopy();
    LinFeasPumpPtr lin_feas_pump = (LinFeasPumpPtr) 
      new LinFeasPump(env_, oinst_, nlpe, lpe);
    bab->addPreRootHeur(lin_feas_pump);
  }
  return bab;
}


BrancherPtr MultiStart::getBrancher_(HandlerVector handlers, EnginePtr e)
{
  BrancherPtr br;
  UInt t;

  if (env_->getOptions()->findString("brancher")->getValue() == "rel") {
    ReliabilityBrancherPtr rel_br;
    rel_br = (ReliabilityBrancherPtr) new ReliabilityBrancher(env_, handlers);
    rel_br->setEngine(e);
    t = (oinst_->getSize()->ints + oinst_->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    rel_br->setThresh(t);
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "setting reliability threshhold to " << t << std::endl;
    t = (UInt) oinst_->getSize()->ints + oinst_->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    rel_br->setMaxDepth(t);
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="Filter-SQP") {
      rel_br->setIterLim(5);
    }
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "reliability branching iteration limit = " <<
      rel_br->getIterLim() << std::endl;
    br = rel_br;
  } else if (env_->getOptions()->findString("brancher")->getValue() ==
             "maxvio") {
    br = (MaxVioBrancherPtr) new MaxVioBrancher(env_, handlers);
  } else if (env_->getOptions()->findString("brancher")->getValue() == "lex") {
    br = (LexicoBrancherPtr) new LexicoBrancher(env_, handlers);
  } else if (env_->getOptions()->findString("brancher")->getValue() == "rand") {
    br = (RandomBrancherPtr) new RandomBrancher(env_, handlers);
  } else if (env_->getOptions()->findString("brancher")->getValue() == 
             "maxfreq") {
    br = (MaxFreqBrancherPtr) new MaxFreqBrancher(env_, handlers);
  }
  env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
    "brancher used = " << br->getName() << std::endl;
  return br;
}


int MultiStart::getEngine_(Engine **e)
{
  EngineFactory efac(env_);
  bool cont=false;
  EnginePtr eptr = 0;
  int err = 0;

  oinst_->calculateSize();
  if (oinst_->isLinear()) {
    eptr = efac.getLPEngine();
    if (!eptr) {
      cont = true;
    }
  }

  if (true==cont || oinst_->isQP()) {
    eptr = efac.getQPEngine();
    if (!eptr) {
      cont = true;
    }
  }

  if (!eptr) {
    eptr = efac.getNLPEngine();
  }

  if (!eptr) {
    env_->getLogger()->errStream() <<  "No engine available for this problem."
                                  << std::endl << "exiting without solving"
                                  << std::endl;
    err = 1;
  } else {
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "engine used = " << eptr->getName() << std::endl;
  }
  *e = eptr;
  return err;
}


void loadProblem(EnvPtr env, MINOTAUR_AMPL::AMPLInterface* iface,
                 ProblemPtr &oinst, double *obj_sense)
{
  Timer *timer     = env->getNewTimer();
  OptionDBPtr options = env->getOptions();
  JacobianPtr jac;
  HessianOfLagPtr hess;
  const std::string me("msbnb main: ");

  timer->start();
  oinst = iface->readInstance(options->findString("problem_file")->getValue());
  env->getLogger()->msgStream(LogInfo) << me 
    << "time used in reading instance = " << std::fixed 
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
  const std::string me("msbnb main: ");

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
  env->getOptions()->findString("nlp_engine")->setValue("IPOPT");
  env->getOptions()->findString("qp_engine")->setValue("None"); 
  env->getOptions()->findString("brancher")->setValue("maxvio");
}


void showHelp()
{
  std::cout << "NLP-based branch-and-estimate heuristic for nonconvex MINLP."
    << std::endl
    << "**Works in parallel with a thread-safe NLP solver only "
    << "(e.g. IPOPT with MA97)**" << std::endl
    << "Usage:" << std::endl
    << "To show version: msbnb -v (or --display_version yes) " << std::endl
    << "To show all options: msbnb -= (or --display_options yes)" 
    << std::endl
    << "To solve an instance: msbnb --option1 [value] "
    << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("msbnb main: ");

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
      << "Multistart NLP-based branch-and-bound solver for nonconvex MINLP"
      << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env->getLogger()->msgStream(LogInfo)
    << me << "Minotaur version " << env->getVersion() << std::endl
    << me << "Multistart NLP-based branch-and-bound solver for nonconvex MINLP"
    << std::endl;
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
  } else if (sol && env->getLogger()->getMaxLevel()>=LogExtraInfo &&
             env->getOptions()->findBool("display_solution")->getValue()) {
    sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
  }
}


void writeBnbStatus(EnvPtr env, BranchAndBound *bab, double obj_sense)
{

  const std::string me("msbnb main: ");
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


double getWallTime() {
  struct timeval time;
  if (gettimeofday(&time,NULL)){
    //  Handle error
    return 0;
  }
  return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


int MultiStart::solve(ProblemPtr p)
{

  double WallTimeStart = getWallTime();
  EnvPtr env      = (EnvPtr) new Environment();
  OptionDBPtr options;
  MINOTAUR_AMPL::AMPLInterface* iface = 0;
  ProblemPtr oinst;      // instance that needs to be solved.
  EnginePtr engine = 0;  // engine for solving relaxations. 
  SolutionPtr sol, sol2;
  JacobianPtr jPtr;
  HessianOfLagPtr hPtr;
  BranchAndBound * bab = 0;
  PresolverPtr pres;
  const std::string me("msbnb main: ");
  VarVector *orig_v=0;
  HandlerVector handlers;
  int err = 0;
  double obj_sense = 1.0;

  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  setInitialOptions(env);

  // Important to setup AMPL Interface first as it adds several options.
  iface = new MINOTAUR_AMPL::AMPLInterface(env, "msbnb");

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);

  overrideOptions(env);
  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

  loadProblem(env, iface, oinst, &obj_sense);
  orig_v = new VarVector(oinst->varsBegin(), oinst->varsEnd());
  pres = presolve(env, oinst, iface->getNumDefs(), handlers);
  handlers.clear();
  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo) << me 
      << "status of presolve: " 
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol(env, orig_v, pres, SolutionPtr(), pres->getStatus(), iface);
    writeBnbStatus(env, bab, obj_sense);
    goto CLEANUP;
  }

  if (false==env->getOptions()->findBool("solve")->getValue()) {
    goto CLEANUP;
  }

  engine = getEngine(env, oinst, err);
  if (err) {
    goto CLEANUP;
  }

  bab = getBab_(engine, handlers);
  if (env->getOptions()->findInt("threads")->getValue() > 1) {
   env->getLogger()->msgStream(LogInfo) 
     << "**Works in parallel with a thread-safe "
     << "NLP solver only (e.g. IPOPT with MA97)**" << std::endl;
  }
  bab->solve();
  bab->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  engine->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    (*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  }

  writeSol(env, orig_v, pres, bab->getSolution(), bab->getStatus(), iface);
  writeBnbStatus(env, bab, obj_sense);

CLEANUP:
  if (engine) {
    delete engine;
  }
  if (iface) {
    delete iface;
  }
  if (bab) {
    delete bab;
  }
  if (orig_v) {
    delete orig_v;
  }
  std::cout << "msbnb main: wall clock time used (s) = "
            << getWallTime() - WallTimeStart << std::endl; 
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
