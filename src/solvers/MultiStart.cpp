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
  } else {
    delete s_hand;
  }

  // add SOS2 handler here.
  s2_hand = (SOS2HandlerPtr) new SOS2Handler(env_, oinst_);
  if (s2_hand->isNeeded()) {
    s2_hand->setModFlags(false, true);
    handlers.push_back(s2_hand);
  } else {
    delete s2_hand;
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
  BrancherPtr br = 0;
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


DoubleVector MultiStart::getSolution()
{
  DoubleVector x;
  return x;
}


SolveStatus MultiStart::getStatus()
{
  return status_;
}


PresolverPtr MultiStart::presolve_(HandlerVector &handlers)
{
  PresolverPtr pres = 0; // NULL

  oinst_->calculateSize();
  if (env_->getOptions()->findBool("presolve")->getValue() == true) {
    LinHandlerPtr lhandler = (LinHandlerPtr) new LinearHandler(env_, oinst_);
    handlers.push_back(lhandler);
    if (oinst_->isQP() || oinst_->isQuadratic() || oinst_->isLinear() ||
        true==env_->getOptions()->findBool("use_native_cgraph")->getValue()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    if (iface_ && iface_->getNumDefs()>0) {
      lhandler->setPreOptDualFix(false);
    } else {
      lhandler->setPreOptDualFix(true);
    }

    if (!oinst_->isLinear() && 
        true==env_->getOptions()->findBool("use_native_cgraph")->getValue() && 
        true==env_->getOptions()->findBool("nl_presolve")->getValue() 
       ) {
      NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env_, oinst_);
      handlers.push_back(nlhand);
    }

    // write the names.
    env_->getLogger()->msgStream(LogExtraInfo) << me_ 
      << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
         ++h) {
       env_->getLogger()->msgStream(LogExtraInfo) << me_ 
        << (*h)->getName() << std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(oinst_, env_, handlers);
  pres->standardize(); 
  if (env_->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
    for (HandlerVector::iterator h=handlers.begin(); h!=handlers.end(); ++h) {
      (*h)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
    }
  }
  return pres;
}


void MultiStart::setInitialOptions_()
{
  env_->getOptions()->findString("nlp_engine")->setValue("IPOPT");
  env_->getOptions()->findString("qp_engine")->setValue("None"); 
  env_->getOptions()->findString("brancher")->setValue("maxvio");
}


void MultiStart::showHelp() const
{
  env_->getLogger()->errStream()
      << "NLP-based branch-and-estimate heuristic for nonconvex MINLP."
      << std::endl
      << "** Shared memory parallelism works with a thread-safe NLP solver only "
      << "(e.g. IPOPT with MA97). **" << std::endl
      << "Usage:" << std::endl
      << "To show version: multistart -v (or --display_version yes) "
      << std::endl
      << "To show all options: multistart -= (or --display_options yes)" 
      << std::endl
      << "To solve an instance: multistart --option1 [value] "
      << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int MultiStart::showInfo()
{
  OptionDBPtr options = env_->getOptions();

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
    env_->getLogger()->msgStream(LogNone) << me_ << "Minotaur version "
      << env_->getVersion() << std::endl << me_ 
      << "Multistart NLP-based branch-and-bound solver for nonconvex MINLP"
      << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env_->getLogger()->msgStream(LogInfo)
    << me_ << "Minotaur version " << env_->getVersion() << std::endl
    << me_ << "Multistart NLP-based branch-and-bound solver for nonconvex MINLP"
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


int MultiStart::writeBnbStatus_(BranchAndBound *bab)
{

  int err = 0;

  if (bab) {
    env_->getLogger()->msgStream(LogInfo)
      << me_ << std::fixed << std::setprecision(4) 
      << "best solution value = " << objSense_*bab->getUb() << std::endl
      << me_ << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = "
      <<  objSense_*bab->getLb() << std::endl
      << me_ << "gap = " << std::max(0.0,bab->getUb() - bab->getLb())
      << std::endl
      << me_ << "gap percentage = " << bab->getPerGap() << std::endl
      << me_ << "time used (s) = " << std::fixed << std::setprecision(2) 
      << env_->getTime(err) << std::endl
      << me_ << "status of branch-and-bound: " 
      << getSolveStatusString(bab->getStatus()) << std::endl;
  } else {
    env_->getLogger()->msgStream(LogInfo)
      << me_ << std::fixed << std::setprecision(4)
      << "best solution value = " << INFINITY << std::endl
      << me_ << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = " << INFINITY << std::endl
      << me_ << "gap = " << INFINITY << std::endl
      << me_ << "gap percentage = " << INFINITY << std::endl
      << me_ << "time used (s) = " << std::fixed << std::setprecision(2) 
      << env_->getTime(err) << std::endl
      << me_ << "status of branch-and-bound: " 
      << getSolveStatusString(NotStarted) << std::endl;
  }
  return 0;
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
  double walltime_start = getWallTime();
  EnginePtr engine = 0;  // engine for solving relaxations. 
  BranchAndBound * bab = 0;
  PresolverPtr pres;
  VarVector *orig_v=0;
  HandlerVector handlers;
  int err = 0;
  OptionDBPtr options = env_->getOptions();

  setInitialOptions_();

  oinst_ = p;
  oinst_->calculateSize();
  if (options->findBool("display_problem")->getValue()==true) {
    oinst_->write(env_->getLogger()->msgStream(LogNone), 12);
  }
  if (options->findBool("display_size")->getValue()==true) {
    oinst_->writeSize(env_->getLogger()->msgStream(LogNone));
  }


  // setup the jacobian and hessian
  if (false==options->findBool("use_native_cgraph")->getValue()) {
    JacobianPtr jac = new MINOTAUR_AMPL::AMPLJacobian(iface_);
    oinst_->setJacobian(jac);

    // create the hessian
    HessianOfLagPtr hess = new MINOTAUR_AMPL::AMPLHessian(iface_);
    oinst_->setHessian(hess);

    // set initial point
    oinst_->setInitialPoint(iface_->getInitialPoint(), 
                            oinst_->getNumVars()-iface_->getNumDefs());
  }

  if (oinst_->getObjective() &&
      oinst_->getObjective()->getObjectiveType()==Maximize) {
    objSense_ = -1.0;
    env_->getLogger()->msgStream(LogInfo) << me_ 
      << "objective sense: maximize (will be converted to Minimize)"
      << std::endl;
  } else {
    objSense_ = 1.0;
    env_->getLogger()->msgStream(LogInfo) << me_ 
      << "objective sense: minimize" << std::endl;
  }


  // First store all original variables in a vector, then presolve.
  // Keep a pointer to presolver for postsolving after the main solve.
  orig_v = new VarVector(oinst_->varsBegin(), oinst_->varsEnd());
  pres = presolve_(handlers);
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }
  handlers.clear();

  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env_->getLogger()->msgStream(LogInfo) << me_ 
      << "status of presolve: " 
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol_(env_, orig_v, pres, pres->getSolution(), pres->getStatus(), iface_);
    goto CLEANUP;
  }

  err = getEngine_(&engine);
  if (err) {
    goto CLEANUP;
  }

  bab = getBab_(engine, handlers);

  if (env_->getOptions()->findInt("threads")->getValue() > 1) {
   env_->getLogger()->msgStream(LogError) 
     << "Warning: ** Multiple threads work with a thread-safe "
     << "NLP solver only (e.g. IPOPT with MA97) **" << std::endl;
  }
  bab->solve();
  bab->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  engine->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    (*it)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  }

  writeSol_(env_, orig_v, pres, bab->getSolution(), bab->getStatus(), iface_);
  writeBnbStatus_(bab);

CLEANUP:
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }
  if (engine) {
    delete engine;
  }
  if (pres) {
    delete pres;
  }
  if (orig_v) {
    delete orig_v;
  }
  if (bab) {
    if (bab->getNodeRelaxer()) {
      delete bab->getNodeRelaxer();
    }
    if (bab->getNodeProcessor()) {
      delete bab->getNodeProcessor();
    }
    delete bab;
  }
  oinst_ = 0;
  env_->getLogger()->msgStream(LogInfo) << me_ << "wall clock time used (s) = "
                                       << getWallTime() - walltime_start 
                                       << std::endl; 
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
