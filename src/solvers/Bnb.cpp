//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file Bnb.cpp
 * \brief The Bnb class for solving instances in ampl format (.nl) by
 * using Branch-and-Bound alone.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SppHeur.h"

#include "EngineFactory.h"
#include "NlWriter.h"
#include "Reader.h"

#include "Bnb.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "LexicoBrancher.h"
#include "LinFeasPump.h"
#include "MINLPDiving.h"
#include "MaxFreqBrancher.h"
#include "MaxInfBrancher.h"
#include "MaxVioBrancher.h"
#include "NodeIncRelaxer.h"
#include "PCBProcessor.h"
#include "Presolver.h"
#include "RandomBrancher.h"
#include "ReliabilityBrancher.h"
#include "TreeManager.h"

#include "FixVarsHeur.h"
#include "IntVarHandler.h"
#include "LinearHandler.h"
#include "NlPresHandler.h"
#include "RCHandler.h"
#include "SOS1Handler.h"
#include "SOS2Handler.h"
#include "SamplingHeur.h"

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"

using namespace Minotaur;
const std::string Bnb::me_ = "Bnb: ";

Bnb::Bnb(EnvPtr env)
  : objSense_(1.0),
    status_(NotStarted)
{
  env_ = env;
  iface_ = 0;
}

Bnb::~Bnb() { }

BranchAndBound* Bnb::getBab_(Engine* engine, HandlerVector& handlers)
{
  BranchAndBound* bab = new BranchAndBound(env_, oinst_);
  NodeProcessorPtr nproc = 0;
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env_, oinst_);
  LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env_, oinst_);
  NlPresHandlerPtr nlhand = 0;
  NodeIncRelaxerPtr nr = 0;
  RelaxationPtr rel = 0;
  BrancherPtr br = 0;
  OptionDBPtr options = env_->getOptions();
  SOS2HandlerPtr s2_hand;
  RCHandlerPtr rc_hand;

  SOS1HandlerPtr s_hand = (SOS1HandlerPtr) new SOS1Handler(env_, oinst_);
  if(s_hand->isNeeded()) {
    s_hand->setModFlags(false, true);
    handlers.push_back(s_hand);
  } else {
    delete s_hand;
  }

  //adding RCHandler
  if(options->findBool("rc_fix")->getValue()) {
    rc_hand = (RCHandlerPtr) new RCHandler(env_);
    rc_hand->setModFlags(false, true);
    handlers.push_back(rc_hand);
    assert(rc_hand);
  }

  // add SOS2 handler here.
  s2_hand = (SOS2HandlerPtr) new SOS2Handler(env_, oinst_);
  if(s2_hand->isNeeded()) {
    s2_hand->setModFlags(false, true);
    handlers.push_back(s2_hand);
  } else {
    delete s2_hand;
  }

  handlers.push_back(v_hand);
  if(true == options->findBool("presolve")->getValue()) {
    l_hand->setModFlags(false, true);
    handlers.push_back(l_hand);
  } else {
    delete l_hand;
  }
  if(!oinst_->isLinear() && true == options->findBool("presolve")->getValue() &&
     true == options->findBool("use_native_cgraph")->getValue() &&
     true == options->findBool("nl_presolve")->getValue()) {
    nlhand = (NlPresHandlerPtr) new NlPresHandler(env_, oinst_);
    nlhand->setModFlags(false, true);
    handlers.push_back(nlhand);
  }
  if(handlers.size() > 1) {
    nproc = (PCBProcessorPtr) new PCBProcessor(env_, engine, handlers);
  } else {
    nproc = (BndProcessorPtr) new BndProcessor(env_, engine, handlers);
  }
  br = getBrancher_(handlers, engine);
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env_, handlers);
  nr->setModFlag(false);
  rel = (RelaxationPtr) new Relaxation(oinst_, env_);
  rel->calculateSize();
  if(options->findBool("use_native_cgraph")->getValue() || rel->isQP() ||
     rel->isQuadratic()) {
    rel->setNativeDer();
  } else {
    rel->setJacobian(oinst_->getJacobian());
    rel->setHessian(oinst_->getHessian());
  }

  nr->setRelaxation(rel);
  nr->setEngine(engine);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);
  // NlWriter wr(env_);
  // wr.write(rel, "test1234.nl");

  if(0 <= options->findInt("divheur")->getValue()) {
    MINLPDivingPtr div_heur;
    EnginePtr e2 = engine->emptyCopy();
    if(true == options->findBool("use_native_cgraph")->getValue() ||
       rel->isQP() || rel->isQuadratic()) {
      oinst_->setNativeDer();
    }
    div_heur = (MINLPDivingPtr) new MINLPDiving(env_, oinst_, e2);
    bab->addPreRootHeur(div_heur);
  }

  if(options->findBool("prerootheur")->getValue() == true) {
    if(options->findBool("sppheur")->getValue() == true) {
      SppHeur* sp = new SppHeur(env_, oinst_);
      bab->addPreRootHeur(sp);
    }

    if(options->findBool("samplingheur")->getValue() == true) {
      SamplingHeurPtr s_heur = (SamplingHeurPtr) new SamplingHeur(env_, oinst_);
      bab->addPreRootHeur(s_heur);
    }

    if(options->findBool("fixvarsheur")->getValue() == true) {
      FixVarsHeurPtr f_heur = (FixVarsHeurPtr) new FixVarsHeur(env_, oinst_);
      f_heur->setHandlers(handlers);
      bab->addPreRootHeur(f_heur);
    }
  }

  if(true == options->findBool("FPump")->getValue()) {
    EngineFactory efac(env_);
    EnginePtr lpe = efac.getLPEngine();
    EnginePtr nlpe = engine->emptyCopy();
    LinFeasPumpPtr lin_feas_pump =
        (LinFeasPumpPtr) new LinFeasPump(env_, oinst_, nlpe, lpe);
    bab->addPreRootHeur(lin_feas_pump);
  }
  return bab;
}

BrancherPtr Bnb::getBrancher_(HandlerVector handlers, EnginePtr e)
{
  BrancherPtr br = 0;
  UInt t;
  std::string brancher = env_->getOptions()->findString("brancher")->getValue();

  if(brancher == "rel" || brancher == "hybrid") {
    ReliabilityBrancherPtr rel_br;
    rel_br = (ReliabilityBrancherPtr) new ReliabilityBrancher(env_, handlers);
    rel_br->setEngine(e);
    t = (oinst_->getSize()->ints + oinst_->getSize()->bins) / 10;
    t = std::max(t, (UInt)2);
    t = std::min(t, (UInt)4);
    rel_br->setThresh(t);
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "setting reliability threshhold to " << t << std::endl;
    t = (UInt)oinst_->getSize()->ints + oinst_->getSize()->bins / 20 + 2;
    t = std::min(t, (UInt)10);
    rel_br->setMaxDepth(t);
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "setting reliability maxdepth to " << t << std::endl;
    if(e->getName() == "filter-sqp") {
      rel_br->setIterLim(5);
    }
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_
        << "reliability branching iteration limit = " << rel_br->getIterLim()
        << std::endl;
    br = rel_br;
  } else if(env_->getOptions()->findString("brancher")->getValue() ==
            "maxvio") {
    br = (MaxVioBrancherPtr) new MaxVioBrancher(env_, handlers);
  } else if(env_->getOptions()->findString("brancher")->getValue() == "lex") {
    br = (LexicoBrancherPtr) new LexicoBrancher(env_, handlers);
  } else if(env_->getOptions()->findString("brancher")->getValue() == "rand") {
    br = (RandomBrancherPtr) new RandomBrancher(env_, handlers);
  } else if(env_->getOptions()->findString("brancher")->getValue() ==
            "maxfreq") {
    br = (MaxFreqBrancherPtr) new MaxFreqBrancher(env_, handlers);
  }
    else if(env_->getOptions()->findString("brancher")->getValue() ==
            "maxinf") {
    MaxInfBrancherPtr inf_br;  
    inf_br = (MaxInfBrancherPtr) new MaxInfBrancher(env_, handlers);
    inf_br->setEngine(e);
    t = (oinst_->getSize()->ints + oinst_->getSize()->bins) / 10;
    t = std::max(t, (UInt)2);
    t = std::min(t, (UInt)4);
    inf_br->setThresh(t);
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "setting reliability threshhold to " << t << std::endl;
    t = (UInt)oinst_->getSize()->ints + oinst_->getSize()->bins / 20 + 2;
    t = std::min(t, (UInt)10);
    inf_br->setMaxDepth(t);
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "setting reliability maxdepth to " << t << std::endl;
    if(e->getName() == "filter-sqp") {
      inf_br->setIterLim(5);
    }
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_
        << "Inference branching iteration limit = " << inf_br->getIterLim()
        << std::endl;
    br = inf_br;
  }
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << "brancher used = " << br->getName() << std::endl;
  return br;
}

void Bnb::doSetup()
{
  setInitialOptions_();
}

int Bnb::getEngine_(Engine** e)
{
  EngineFactory efac(env_);
  bool cont = false;
  EnginePtr eptr = 0;
  int err = 0;

  oinst_->calculateSize();
  if(oinst_->isLinear()) {
    eptr = efac.getLPEngine();
    if(!eptr) {
      cont = true;
    }
  }

  if(true == cont || oinst_->isQP()) {
    eptr = efac.getQPEngine();
    if(!eptr) {
      cont = true;
    }
  }

  if(!eptr) {
    eptr = efac.getNLPEngine();
  }

  if(!eptr) {
    env_->getLogger()->errStream()
        << "No engine available for this problem." << std::endl
        << "exiting without solving" << std::endl;
    err = 1;
  } else {
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "engine used = " << eptr->getName() << std::endl;
  }
  *e = eptr;
  return err;
}

DoubleVector Bnb::getSolution()
{
  DoubleVector x;
  return x;
}

SolveStatus Bnb::getStatus()
{
  return status_;
}

PresolverPtr Bnb::presolve_(HandlerVector& handlers)
{
  PresolverPtr pres = 0;

  oinst_->calculateSize();
  if(env_->getOptions()->findBool("presolve")->getValue() == true) {
    LinHandlerPtr lhandler = (LinHandlerPtr) new LinearHandler(env_, oinst_);
    handlers.push_back(lhandler);
    if(oinst_->isQP() || oinst_->isQuadratic() || oinst_->isLinear() ||
       true == env_->getOptions()->findBool("use_native_cgraph")->getValue()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    if(iface_ && iface_->getNumDefs() > 0) {
      lhandler->setPreOptDualFix(false);
    } else {
      lhandler->setPreOptDualFix(true);
    }

    if(!oinst_->isLinear() &&
       true == env_->getOptions()->findBool("use_native_cgraph")->getValue() &&
       true == env_->getOptions()->findBool("nl_presolve")->getValue()) {
      NlPresHandlerPtr nlhand =
          (NlPresHandlerPtr) new NlPresHandler(env_, oinst_);
      handlers.push_back(nlhand);
    }

    // write the names.
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "handlers used in presolve:" << std::endl;
    for(HandlerIterator h = handlers.begin(); h != handlers.end(); ++h) {
      env_->getLogger()->msgStream(LogExtraInfo)
          << me_ << (*h)->getName() << std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(oinst_, env_, handlers);
  pres->standardize();
  if(env_->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }

  return pres;
}

void Bnb::setInitialOptions_()
{
  OptionDBPtr options = env_->getOptions();
  options->findString("interface_type")->setValue("ampl");
  //options->findBool("presolve")->setValue(true);
  //options->findBool("nl_presolve")->setValue(true);
  //options->findBool("lin_presolve")->setValue(true);
  //options->findString("brancher")->setValue("parrel");
  //options->findString("nlp_engine")->setValue("ipopt");
  //options->findBool("cgtoqf")->setValue(true);
  //options->findBool("separability")->setValue(true);
  //options->findBool("simplex_cut")->setValue(true);
}

void Bnb::showHelp() const
{
  env_->getLogger()->errStream()
      << "NLP based Branch-and-bound algorithm for convex MINLP" << std::endl
      << "Usage:" << std::endl
      << "To show version: bnb -v (or --display_version yes) " << std::endl
      << "To show all options: bnb -= (or --display_options yes)" << std::endl
      << "To solve an instance: bnb --option1 [value] "
      << "--option2 [value] ... "
      << " .nl-file" << std::endl;
}

int Bnb::showInfo()
{
  OptionDBPtr options = env_->getOptions();

  if(options->findBool("display_options")->getValue() ||
     options->findFlag("=")->getValue()) {
    options->write(std::cout);
    return 1;
  }

  if(options->findBool("display_help")->getValue() ||
     options->findFlag("?")->getValue()) {
    showHelp();
    return 1;
  }

  if(options->findBool("display_version")->getValue() ||
     options->findFlag("v")->getValue()) {
    env_->getLogger()->msgStream(LogNone)
        << me_ << "Minotaur version " << env_->getVersion() << std::endl;
    env_->getLogger()->msgStream(LogNone)
        << me_ << "NLP based Branch-and-bound algorithm for convex MINLP"
        << std::endl;
    return 1;
  }

  env_->getLogger()->msgStream(LogInfo)
      << me_ << "Minotaur version " << env_->getVersion() << std::endl
      << me_ << "NLP based Branch-and-bound algorithm for convex MINLP"
      << std::endl;
  return 0;
}

int Bnb::solve(ProblemPtr p)
{
  BranchAndBound* bab = 0;
  EnginePtr engine = 0; // engine for solving relaxations.
  PresolverPtr pres = 0;
  VarVector* orig_v = 0;
  HandlerVector handlers;
  int err = 0;
  OptionDBPtr options = env_->getOptions();

  env_->initRand();

  oinst_ = p;
  oinst_->calculateSize();
  if(options->findBool("display_problem")->getValue() == true) {
    oinst_->write(env_->getLogger()->msgStream(LogNone), 12);
  }
  
  if (env_->getOptions()->findInt("log_level")->getValue() >= 3 ) {
        options->findBool("display_size")->setValue(true);
        options->findBool("display_presolved_size")->setValue(true);
  }
  
  if (options->findBool("display_size")->getValue() == true) {
    oinst_->writeSize(env_->getLogger()->msgStream(LogNone));
    env_->getLogger()->msgStream(LogInfo) << me_ << "Starting constraint classification\n";
    oinst_->classifyCon();
    env_->getLogger()->msgStream(LogInfo) << me_ << "Finished constraint classification\n";
  }

  // setup the jacobian and hessian
  if(false == options->findBool("use_native_cgraph")->getValue()) {
    JacobianPtr jac = new MINOTAUR_AMPL::AMPLJacobian(iface_);
    oinst_->setJacobian(jac);

    // create the hessian
    HessianOfLagPtr hess = new MINOTAUR_AMPL::AMPLHessian(iface_);
    oinst_->setHessian(hess);

    // set initial point
    oinst_->setInitialPoint(iface_->getInitialPoint(),
                            oinst_->getNumVars() - iface_->getNumDefs());
  }

  if(oinst_->getObjective() &&
     oinst_->getObjective()->getObjectiveType() == Maximize) {
    objSense_ = -1.0;
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "objective sense: maximize (will be converted to Minimize)"
        << std::endl;
  } else {
    objSense_ = 1.0;
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "objective sense: minimize" << std::endl;
  }

  // First store all original variables in a vector, then presolve.
  // Keep a pointer to presolver for postsolving after the main solve.
  orig_v = new VarVector(oinst_->varsBegin(), oinst_->varsEnd());
  pres = presolve_(handlers);
  for(HandlerVector::iterator it = handlers.begin(); it != handlers.end();
      ++it) {
    delete(*it);
  }
  handlers.clear();

  if(Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env_->getLogger()->msgStream(LogInfo)
        << me_
        << "status of presolve: " << getSolveStatusString(pres->getStatus())
        << std::endl;
    writeSol_(env_, orig_v, pres, pres->getSolution(), pres->getStatus(),
              iface_);
    goto CLEANUP;
  }
  if(options->findBool("solve")->getValue() == false) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Solve option is set to 0, Stopping further processing."
        << std::endl;
    goto CLEANUP; // Return early to stop processing.
  } 

  err = getEngine_(&engine);
  if(err) {
    goto CLEANUP;
  }

  bab = getBab_(engine, handlers);

  bab->solve();
  bab->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  engine->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  for(HandlerVector::iterator it = handlers.begin(); it != handlers.end();
      ++it) {
    (*it)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  }

  writeSol_(env_, orig_v, pres, bab->getSolution(), bab->getStatus(), iface_);
  writeBnbStatus_(bab);

CLEANUP:
  for(HandlerVector::iterator it = handlers.begin(); it != handlers.end();
      ++it) {
    delete(*it);
  }
  if(engine) {
    delete engine;
  }
  if(pres) {
    delete pres;
  }
  if(orig_v) {
    delete orig_v;
  }
  if(bab) {
    if(bab->getNodeRelaxer()) {
      delete bab->getNodeRelaxer();
    }
    if(bab->getNodeProcessor()) {
      delete bab->getNodeProcessor();
    }
    delete bab;
  }
  oinst_ = 0;
  return 0;
}

void Bnb::writeBnbStatus_(BranchAndBound* bab)
{
  if(bab) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << std::fixed << std::setprecision(4)
        << "best solution value = " << objSense_ * bab->getUb() << std::endl
        << me_ << std::fixed << std::setprecision(4)
        << "best bound estimate from remaining nodes = "
        << objSense_ * bab->getLb() << std::endl
        << me_ << "gap = " << std::max(0.0, bab->getUb() - bab->getLb())
        << std::endl
        << me_ << "gap percentage = " << bab->getPerGap() << std::endl
        << me_ << "cpu time used (s) = " << std::fixed << std::setprecision(2)
        << env_->getTime() << std::endl
        << me_ << "wall time used (s) = " << std::fixed << std::setprecision(2)
        << env_->getwTime() << std::endl
        << me_ << "status of branch-and-bound = "
        << getSolveStatusString(bab->getStatus()) << std::endl;
  } else {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << std::fixed << std::setprecision(4)
        << "best solution value = " << INFINITY << std::endl
        << me_ << std::fixed << std::setprecision(4)
        << "best bound estimate from remaining nodes = " << INFINITY
        << std::endl
        << me_ << "gap = " << INFINITY << std::endl
        << me_ << "gap percentage = " << INFINITY << std::endl
        << me_ << "cpu time used (s) = " << std::fixed << std::setprecision(2)
        << env_->getTime() << std::endl
        << me_ << "wall time used (s) = " << std::fixed << std::setprecision(2)
        << env_->getwTime() << std::endl
        << me_
        << "status of branch-and-bound: " << getSolveStatusString(NotStarted)
        << std::endl;
  }
  return;
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
