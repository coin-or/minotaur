//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file Glob.cpp
 * \brief The Glob class for solving MIQCQP instances using ampl (.nl) format.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>
#include <cmath>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Engine.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "Glob.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "LPEngine.h"
#include "MaxVioBrancher.h"
#include "NodeIncRelaxer.h"
#include "NLPEngine.h"
#include "NLPMultiStart.h"
#include "NlPresHandler.h"
#include "Objective.h"
#include "Option.h"
#include "PCBProcessor.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "Problem.h"
#include "QG.h"
#include "QuadHandler.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "SimpleTransformer.h"
#include "QuadTransformer.h"
#include "Solution.h"
#include "Solver.h"
#include "Timer.h"
#include "Transformer.h"
#include "TreeManager.h"

#include "AMPLInterface.h"

using namespace Minotaur;
const std::string Glob::me_ = "mntr-glob: ";

Glob::Glob(EnvPtr env)
: objSense_(1.0),
  status_(NotStarted)
{
  env_ = env;
  iface_  = 0;
  inst_ = 0;
  newp_ = 0;
}

Glob::~Glob()
{
}

void Glob::doSetup() {
  int err = 0;
  env_->startTimer(err);

  setInitialOptions_();

  iface_ = (MINOTAUR_AMPL::AMPLInterfacePtr)
    new MINOTAUR_AMPL::AMPLInterface(env_, "glob");
}

EnginePtr Glob::getEngine_()
{
  EngineFactory *efac = new EngineFactory(env_);
  EnginePtr e = EnginePtr(); // NULL
  e = efac->getLPEngine();
  if (!e) {
    env_->getLogger()->errStream() << me_
      << "Cannot find an LP engine. Cannot proceed!" << std::endl;
  } 

  delete efac;
  return e;
}

NLPEnginePtr Glob::getNLPEngine_()
{
  EngineFactory *efac = new EngineFactory(env_);
  NLPEnginePtr e = NLPEnginePtr(); // NULL
  e = efac->getNLPEngine();
  if (!e) {
    env_->getLogger()->errStream() << me_
      << "Cannot find an NLP engine. Cannot proceed!" << std::endl;
  } 

  delete efac;
  return e;
}

int Glob::transform_(ProblemPtr &newp, HandlerVector &handlers) {
  TransformerPtr trans;
  int status = 0;
  std::string tr = env_->getOptions()->findString("transformer")->getValue();

  handlers.clear();
  if (tr == "simp") {
    trans = (SimpTranPtr) new SimpleTransformer(env_, inst_, getEngine_(),
                                                getNLPEngine_());
  } else {//if (tr == "quad") {
    trans = (QuadTranPtr) new QuadTransformer(env_, inst_);
  }
  trans->reformulate(newp, handlers, status);
  
  env_->getLogger()->msgStream(LogInfo) << me_
    << "handlers used in transformer: " << std::endl;
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end();
       ++it) {
    env_->getLogger()->msgStream(LogInfo) << "  " << (*it)->getName()
                                         << std::endl;
  }
  delete trans;
  return status;
}

BranchAndBound * Glob::createBab_(EnginePtr e, HandlerVector &handlers) {
  BranchAndBound *bab = new BranchAndBound(env_, newp_);
  PCBProcessorPtr nproc;
  NodeIncRelaxerPtr nr;
  BrancherPtr br = 0;

  if (env_->getOptions()->findString("brancher")->getValue() == "rel") {
    UInt t;
    ReliabilityBrancherPtr rel_br;
    rel_br = (ReliabilityBrancherPtr) new ReliabilityBrancher(env_, handlers);
    rel_br->setEngine(e);
    t = (newp_->getSize()->ints + newp_->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    rel_br->setThresh(t);
    env_->getLogger()->msgStream(LogExtraInfo) << me_
      << "setting reliability threshhold to " << t << std::endl;
    t = (UInt) newp_->getSize()->ints + newp_->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    rel_br->setMaxDepth(t);
    env_->getLogger()->msgStream(LogExtraInfo) << me_
      << "setting reliability maxdepth to " << t << std::endl;
    env_->getLogger()->msgStream(LogExtraInfo) << me_
      << "reliability branching iteration limit = " 
      << rel_br->getIterLim() << std::endl;
    br = rel_br;
  } else if (env_->getOptions()->findString("brancher")->getValue() 
      == "maxvio") {
    MaxVioBrancherPtr mbr = 
      (MaxVioBrancherPtr) new MaxVioBrancher(env_, handlers);
    br = mbr;
  } else if (env_->getOptions()->findString("brancher")->getValue() 
      == "lex") {
    LexicoBrancherPtr lbr = 
      (LexicoBrancherPtr) new LexicoBrancher(env_, handlers);
    br = lbr;
  }
  env_->getLogger()->msgStream(LogExtraInfo) << me_
    << "brancher used = " << br->getName() << std::endl;
  nproc = (PCBProcessorPtr) new PCBProcessor(env_, e, handlers);
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env_, handlers);
  nr->setProblem(newp_);
  nr->setEngine(e);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(true);

  if (env_->getOptions()->findBool("msheur")->getValue() == true && 
      (newp_->getSize()->bins == 0 && newp_->getSize()->ints == 0)) {
    EnginePtr nlp_e = getNLPEngine_();
    newp_->setNativeDer();
    NLPMSPtr ms_heur = (NLPMSPtr) new NLPMultiStart(env_, newp_, nlp_e);
    bab->addPreRootHeur(ms_heur); 
  }

  return bab;
}

PresolverPtr Glob::createPres_(HandlerVector &handlers) {
  // create handlers for presolve
  PresolverPtr pres = PresolverPtr(); // NULL

  inst_->calculateSize();
  if (env_->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler = (LinearHandlerPtr) new LinearHandler(env_, inst_);
    handlers.push_back(lhandler);
    if (inst_->isQP() || inst_->isQuadratic() || inst_->isLinear()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    lhandler->setPreOptDualFix(true);

    if (!inst_->isLinear() &&
         (inst_->isQP() || inst_->isQuadratic())) {
      if (true==env_->getOptions()->findBool("cgtoqf")->getValue()) {
        QuadHandlerPtr qhand = (QuadHandlerPtr) new QuadHandler(env_, inst_);
        handlers.push_back(qhand);
      } else {
        if (true == env_->getOptions()->findBool("nl_presolve")->getValue()) {
          NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env_, inst_);
          handlers.push_back(nlhand);
        }
      }
    }

    if (!(inst_->isLinear() || inst_->isQP() || inst_->isQuadratic()) && 
         true==env_->getOptions()->findBool("nl_presolve")->getValue()) {
      NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env_, inst_);
      handlers.push_back(nlhand);
    }

    // write the names.
    env_->getLogger()->msgStream(LogExtraInfo) << me_ 
      << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
        ++h) {
      env_->getLogger()->msgStream(LogExtraInfo) << "  "
        << (*h)->getName()<<std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(inst_, env_, handlers);
  pres->standardize();
  return pres;
}

DoubleVector Glob::getSolution()
{
  DoubleVector x;
  return x;
}

SolveStatus Glob::getStatus()
{
  return status_;
}

void Glob::setInitialOptions_()
{
  OptionDBPtr options = env_->getOptions();
  options->findString("interface_type")->setValue("AMPL");
  options->findBool("presolve")->setValue(true);
  options->findBool("nl_presolve")->setValue(true);
  options->findBool("lin_presolve")->setValue(true);
  options->findBool("msheur")->setValue(true);
  options->findString("brancher")->setValue("maxvio");
  options->findString("nlp_engine")->setValue("IPOPT");
  options->findBool("cgtoqf")->setValue(true);
}

int Glob::solve(ProblemPtr inst) {
  EnginePtr engine = 0;
  HandlerVector handlers;
  VarVector *orig_v=0;
  PresolverPtr pres = 0, pres2 = 0;
  int err = 0;
  ProblemPtr newp = 0;
  BranchAndBound *bab = 0;
  OptionDBPtr options = env_->getOptions();

  inst_ = inst;
  if (options->findBool("cgtoqf")->getValue()==1) {
    inst_->cg2qf();
  }

  // display the problem
  inst_->calculateSize();
  if (options->findBool("display_problem")->getValue()==true) {
    inst_->write(env_->getLogger()->msgStream(LogNone), 9);
  }
  if (options->findBool("display_size")->getValue()==true) {
    inst_->writeSize(env_->getLogger()->msgStream(LogNone));
  }

  if (inst_->getObjective() &&
      inst_->getObjective()->getObjectiveType()==Maximize) {
    objSense_ = -1.0;
    env_->getLogger()->msgStream(LogInfo) << me_
      << "objective sense: maximize (will be converted to Minimize)"
      << std::endl;
  } else {
    env_->getLogger()->msgStream(LogInfo) << me_
      << "objective sense: minimize" << std::endl;
  }
  // Get the right engine.
  engine = getEngine_();
  env_->getLogger()->msgStream(LogExtraInfo) << me_
    << "engine used = " << engine->getName() << std::endl;

  // get presolver.
  handlers.clear();
  orig_v = new VarVector(inst_->varsBegin(), inst_->varsEnd());
  pres = createPres_(handlers);
  if (env_->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
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


  inst_->setNativeDer();
  err = transform_(newp, handlers);
  assert(0==err || 2==err); // return status 2 means problem is convex

  if (err==2) {
    // call QG
    env_->getLogger()->msgStream(LogInfo) << me_ <<
      "All constraints and objective found to be convex" << std::endl
      << "Problem is forwarded to QG - convex MINLP solver" << std::endl;
    QG qg(env_);
    if (0!=qg.showInfo()) {
      goto CLEANUP;
    }

    err = qg.solve(inst_);
    goto CLEANUP;
  }

  newp_ = newp;
  env_->getLogger()->msgStream(LogExtraInfo) << me_
    << "Presolving transformed problem ... " << std::endl;
  pres2 = (PresolverPtr) new Presolver(newp_, env_, handlers);

  pres2->solve();
  env_->getLogger()->msgStream(LogExtraInfo) << me_
    << "Finished presolving transformed problem" << std::endl;

  // get branch-and-bound
  bab = createBab_(engine, handlers);

  if (false==env_->getOptions()->findBool("solve")->getValue()) {
    goto CLEANUP;
  }

  // solve
  bab->solve();
  bab->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  engine->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    (*it)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  }

  writeSol_(env_, orig_v, pres, bab->getSolution(), bab->getStatus(), iface_);
  writeStatus_(bab);

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
  if (pres2) {
    delete pres2;
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
  if (newp_) {
    delete newp;
  }
  if (orig_v) {
    delete orig_v;
  }
  inst_ = 0;
  return 0;
}

void Glob::showHelp() const {
  std::cout << "global optimization for general QCQP" << std::endl
            << "Usage:" << std::endl
            << "To show version: glob -v (or --display_version yes)"
            << std::endl
            << "To show all options: glob -= (or --display_options yes)" 
            << std::endl
            << "To solve an instance: glob --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int Glob::showInfo()
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
      << env_->getVersion() << std::endl 
      << me_ << "global optimization for nonconvex QCQP" << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env_->getLogger()->msgStream(LogInfo)
    << me_ << "Minotaur version " << env_->getVersion() << std::endl
    << me_ << "global optimization for nonconvex QCQP" << std::endl;
  return 0;
}

void Glob::writeStatus_(BranchAndBound *bab) {
  int err = 0;

  if (bab) {
    status_ = bab->getStatus();
    env_->getLogger()->msgStream(LogInfo)
      << me_ << std::fixed << std::setprecision(4) 
      << "best solution value = " << objSense_*bab->getUb() << std::endl
      << me_ << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = "
      <<  objSense_*bab->getLb() << std::endl
      << me_ << "gap = " << std::max(0.0,bab->getUb() - bab->getLb())
      << std::endl
      << me_ << "gap percentage = " << bab->getPerGap() << std::endl
      << me_ << "time used = " << std::fixed << std::setprecision(2) 
      << env_->getTime(err) << std::endl
      << me_ << "status of branch-and-bound: " 
      << getSolveStatusString(status_) << std::endl;
    env_->stopTimer(err); assert(0==err);
  } else {
    env_->getLogger()->msgStream(LogInfo)
      << me_ << std::fixed << std::setprecision(4)
      << "best solution value = " << INFINITY << std::endl
      << me_ << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = " << INFINITY << std::endl
      << me_ << "gap = " << INFINITY << std::endl
      << me_ << "gap percentage = " << INFINITY << std::endl
      << me_ << "time used = " << std::fixed << std::setprecision(2) 
      << env_->getTime(err) << std::endl 
      << me_ << "status of branch-and-bound: " 
      << getSolveStatusString(NotStarted) << std::endl;
    env_->stopTimer(err); assert(0==err);
  }
}

//ProblemPtr Glob::loadProblem()
//{
//  OptionDBPtr options = env_->getOptions();
//  Timer *timer = env_->getNewTimer();
//  ProblemPtr inst = 0;
//
//  if (options->findBool("use_native_cgraph")->getValue()==false) {
//    options->findBool("use_native_cgraph")->setValue(true); 
//    env->getLogger()->msgStream(LogExtraInfo) << me 
//      << "Setting value of 'use_native_cgraph option' to True" << std::endl;
//  }
//
//  // load the problem.
//  timer->start();
//  inst = iface_->readInstance(options->findString("problem_file")->getValue());
//  env_->getLogger()->msgStream(LogInfo) << me 
//    << "time used in reading instance = " << std::fixed 
//    << std::setprecision(2) << timer->query() << std::endl;
//
//  if (options->findBool("cgtoqf")->getValue()==1) {
//    inst->cg2qf();
//  }
//
//  // display the problem
//  inst->calculateSize();
//  if (options->findBool("display_problem")->getValue()==true) {
//    inst->write(env->getLogger()->msgStream(LogNone), 9);
//  }
//  if (options->findBool("display_size")->getValue()==true) {
//    inst->writeSize(env->getLogger()->msgStream(LogNone));
//  }
//
//  if (inst->getObjective() &&
//      inst->getObjective()->getObjectiveType()==Maximize) {
//    objSense_ = -1.0;
//    env->getLogger()->msgStream(LogInfo) << me 
//      << "objective sense: maximize (will be converted to Minimize)"
//      << std::endl;
//  } else {
//    env->getLogger()->msgStream(LogInfo) << me 
//      << "objective sense: minimize" << std::endl;
//  }
//  delete timer;
//  return inst;
//}

//void writeSol(EnvPtr env, VarVector *orig_v,
//              PresolverPtr pres, SolutionPtr sol, SolveStatus status,
//              MINOTAUR_AMPL::AMPLInterface* iface)
//{
//  if (sol) {
//    sol = pres->getPostSol(sol);
//  }
//
//  if (env->getOptions()->findFlag("AMPL")->getValue() ||
//      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
//    iface->writeSolution(sol, status);
//  } else if (sol && env->getLogger()->getMaxLevel()>=LogExtraInfo &&
//             env->getOptions()->findBool("display_solution")->getValue()) {
//    sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
//  }
//  if (sol) {
//    delete sol;
//  } 
//}

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
