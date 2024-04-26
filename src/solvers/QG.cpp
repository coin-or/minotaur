//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file QG.cpp
 * \brief Define class for Quesada-Grossmann (QG) algorithm for solving MINLPs
 * \author Ashutosh Mahajan and Meenarli Sharma, IIT Bombay
 */

#include "QG.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "FixVarsHeur.h"
#include "Handler.h"
#include "HybridBrancher.h"
#include "IntVarHandler.h"
#include "LPEngine.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "MaxVioBrancher.h"
#include "MinotaurConfig.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "PCBProcessor.h"
#include "Presolver.h"
#include "Problem.h"
#include "QGHandler.h"
#include "QPEngine.h"
#include "RCHandler.h"
#include "Reader.h"
#include "ReliabilityBrancher.h"
#include "SamplingHeur.h"
#include "Timer.h"
#include "TransSep.h"
#include "Types.h"

using namespace Minotaur;
const std::string QG::me_ = "QG: ";

QG::QG(EnvPtr env)
  : objSense_(1.0),
    sol_(0),
    lb_(-INFINITY),
    ub_(INFINITY),
    status_(NotStarted)
{
  env_ = env;
  iface_ = 0;
  ownIface_ = true;
}

QG::~QG() { }

void QG::doSetup()
{
  int err = 0;
  env_->startTimer(err);

  setInitialOptions_();
}

int QG::getEngines_(Engine** nlp_e, LPEngine** lp_e)
{
  EngineFactory* efac = new EngineFactory(env_);
  oinst_->calculateSize();

  *lp_e = efac->getLPEngine();

  if(oinst_->isLinear()) {
    *nlp_e = efac->getLPEngine();
  } else if(oinst_->isQP()) {
    *nlp_e = efac->getQPEngine();
    if(*nlp_e == 0) {
      *nlp_e = efac->getNLPEngine();
    }
  } else {
    *nlp_e = efac->getNLPEngine();
  }

  delete efac;
  return 0;
}

PresolverPtr QG::presolve_(HandlerVector& handlers)
{
  PresolverPtr pres = 0;

  oinst_->calculateSize();
  if(env_->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler =
        (LinearHandlerPtr) new LinearHandler(env_, oinst_);
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
      NlPresHandlerPtr nlhand = new NlPresHandler(env_, oinst_);
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
  pres = new Presolver(oinst_, env_, handlers);
  pres->standardize();
  if(env_->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
  return pres;
}

void QG::sepDetection()
{
  TransSepPtr sep = TransSepPtr();
  if(env_->getOptions()->findBool("separability")->getValue() == true) {
    if(oinst_->isLinear()) {
      env_->getLogger()->msgStream(LogInfo)
          << me_ << "No separability detection as problem is linear."
          << std::endl;
    } else {
      sep = (TransSepPtr) new TransSep(env_, oinst_);
      sep->sepDetection();
    }
  }
}

void QG::setInitialOptions_()
{
  OptionDBPtr options = env_->getOptions();
  options->findString("interface_type")->setValue("AMPL");
  options->findBool("presolve")->setValue(true);
  options->findBool("nl_presolve")->setValue(true);
  options->findBool("lin_presolve")->setValue(true);
  options->findString("brancher")->setValue("rel");
  options->findString("nlp_engine")->setValue("IPOPT");
  options->findBool("cgtoqf")->setValue(true);
  options->findBool("separability")->setValue(true);
  options->findBool("simplex_cut")->setValue(true);
}

void QG::showHelp() const
{
  env_->getLogger()->errStream()
      << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP" << std::endl
      << "Usage:" << std::endl
      << "To show version: qg -v (or --display_version yes) " << std::endl
      << "To show all options: qg -= (or --display_options yes)" << std::endl
      << "To solve an instance: qg --option1 [value] "
      << "--option2 [value] ... "
      << " .nl-file" << std::endl;
}

int QG::showInfo()
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
        << me_ << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP"
        << std::endl;
    return 1;
  }

  // code for printing whether we use cgtoqf or not

  if(options->findBool("cgtoqf")->getValue() == 1) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Using quadratic function to store quadratic problem."
        << std::endl;
  } else {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Using cgraph function to store non-quadratic problem."
        << std::endl;
  }
  // code ended

  env_->getLogger()->msgStream(LogInfo)
      << me_ << "Minotaur version " << env_->getVersion() << std::endl
      << me_ << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP"
      << std::endl;
  return 0;
}

int QG::solve(ProblemPtr p)
{
  OptionDBPtr options = env_->getOptions();
  Timer* timer = env_->getNewTimer();
  EnginePtr nlp_e = 0;
  LPEnginePtr lp_e = 0; // lp engine
  VarVector* orig_v = 0;
  BranchAndBound* bab = 0;
  PresolverPtr pres = 0;
  BrancherPtr br = BrancherPtr(); // NULL
  PCBProcessorPtr nproc;
  NodeIncRelaxerPtr nr;

  // handlers
  HandlerVector handlers;
  IntVarHandlerPtr v_hand;
  LinearHandlerPtr l_hand;
  QGHandlerPtr qg_hand;
  RCHandlerPtr rc_hand;
  int err = 0;

  oinst_ = p;
  if(oinst_->isQuadratic() && true == options->findBool("cgtoqf")->getValue()) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Using quadratic function to store quadratic problem."
        << std::endl;
    oinst_->cg2qf();
  }

  oinst_->calculateSize();

  timer->start();
  if(options->findBool("display_problem")->getValue() == true) {
    oinst_->write(env_->getLogger()->msgStream(LogNone), 12);
  }

  if(options->findBool("display_size")->getValue() == true) {
    oinst_->writeSize(env_->getLogger()->msgStream(LogNone));
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Starting constraint classification\n";
    oinst_->classifyCon();
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Finished constraint classification\n";
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

  // get presolver.
  orig_v = new VarVector(oinst_->varsBegin(), oinst_->varsEnd());
  pres = presolve_(handlers);
  for(HandlerVector::iterator it = handlers.begin(); it != handlers.end();
      ++it) {
    delete(*it);
  }
  handlers.clear();
  status_ = pres->getStatus();
  if(Finished != status_ && NotStarted != status_) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "status of presolve: " << getSolveStatusString(status_)
        << std::endl;
    writeSol_(env_, orig_v, pres, pres->getSolution(), status_, iface_);
    goto CLEANUP;
  }

  // transform to exploit separability
  sepDetection();

  // create engines for solving LPs and NLPs
  err = getEngines_(&nlp_e, &lp_e);
  if(err) {
    goto CLEANUP;
  }

  if(true == options->findBool("use_native_cgraph")->getValue()) {
    oinst_->setNativeDer();
  }

  if(options->findBool("rc_fix")->getValue() && 0) {
    rc_hand = (RCHandlerPtr) new RCHandler(env_);
    rc_hand->setModFlags(false, true);
    handlers.push_back(rc_hand);
    assert(rc_hand);
  }
  // Initialize the handlers for branch-and-cut
  l_hand = (LinearHandlerPtr) new LinearHandler(env_, oinst_);
  l_hand->setModFlags(false, true);
  handlers.push_back(l_hand);
  assert(l_hand);

  v_hand = (IntVarHandlerPtr) new IntVarHandler(env_, oinst_);
  v_hand->setModFlags(false, true);
  handlers.push_back(v_hand);
  assert(v_hand);

  qg_hand = (QGHandlerPtr) new QGHandler(env_, oinst_, nlp_e);
  qg_hand->setModFlags(false, true);

  handlers.push_back(qg_hand);
  assert(qg_hand);

  // report name
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << "handlers used:" << std::endl;
  for(HandlerIterator h = handlers.begin(); h != handlers.end(); ++h) {
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << (*h)->getName() << std::endl;
  }

  // Only store bound-changes of relaxation (not problem)
  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env_, handlers);
  nr->setModFlag(false);
  nr->setEngine(lp_e);
  nproc = (PCBProcessorPtr) new PCBProcessor(env_, lp_e, handlers);
  if(env_->getOptions()->findString("brancher")->getValue() == "rel" ||
     env_->getOptions()->findString("brancher")->getValue() == "weak" ||
     env_->getOptions()->findString("brancher")->getValue() == "hybrid") {
    ReliabilityBrancherPtr rel_br =
        (ReliabilityBrancherPtr) new ReliabilityBrancher(env_, handlers);
    rel_br->setEngine(lp_e);
    nproc->setBrancher(rel_br);
    br = rel_br;
  } else if(env_->getOptions()->findString("brancher")->getValue() ==
            "maxvio") {
    MaxVioBrancherPtr mbr =
        (MaxVioBrancherPtr) new MaxVioBrancher(env_, handlers);
    nproc->setBrancher(mbr);
    br = mbr;
  } else if(env_->getOptions()->findString("brancher")->getValue() == "lex") {
    LexicoBrancherPtr lbr =
        (LexicoBrancherPtr) new LexicoBrancher(env_, handlers);
    br = lbr;
  } else if(env_->getOptions()
                ->findString("brancher")
                ->getValue()
                .find("strong") != std::string::npos) {
    HybridBrancherPtr hyb_br =
        (HybridBrancherPtr) new HybridBrancher(env_, handlers);
    hyb_br->setEngine(lp_e);
    if(env_->getOptions()->findString("brancher")->getValue() == "btstrong") {
      hyb_br->doStronger();
      hyb_br->setProblem(oinst_);
    }
    br = hyb_br;
  }
  nproc->setBrancher(br);
  env_->getLogger()->msgStream(LogExtraInfo)
      << me_ << "brancher used = " << br->getName() << std::endl;

  bab = new BranchAndBound(env_, oinst_);
  bab->setNodeRelaxer(nr);
  bab->setNodeProcessor(nproc);
  bab->shouldCreateRoot(true);

  if(env_->getOptions()->findBool("prerootheur")->getValue() == true) {
    if(env_->getOptions()->findBool("samplingheur")->getValue() == true) {
      SamplingHeurPtr s_heur = (SamplingHeurPtr) new SamplingHeur(env_, oinst_);
      bab->addPreRootHeur(s_heur);
    }

    if(env_->getOptions()->findBool("fixvarsheur")->getValue() == true) {
      FixVarsHeurPtr f_heur = (FixVarsHeurPtr) new FixVarsHeur(env_, oinst_);
      f_heur->setHandlers(handlers);
      bab->addPreRootHeur(f_heur);
    }
  }

  // start solving
  bab->solve();

  status_ = bab->getStatus();
  lb_ = bab->getLb();
  ub_ = bab->getUb();
  sol_ = bab->getSolution();

  bab->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  nlp_e->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  lp_e->writeStats(env_->getLogger()->msgStream(LogExtraInfo));

  for(HandlerVector::iterator it = handlers.begin(); it != handlers.end();
      ++it) {
    (*it)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
  }

  err = writeSol_(env_, orig_v, pres, sol_, status_, iface_);
  if(err) {
    goto CLEANUP;
  }
  err = writeBnbStatus_(bab);

CLEANUP:
  for(HandlerVector::iterator it = handlers.begin(); it != handlers.end();
      ++it) {
    delete(*it);
  }
  if(lp_e) {
    delete lp_e;
  }
  if(nlp_e) {
    delete nlp_e;
  }
  if(iface_ && ownIface_) {
    delete iface_;
    iface_ = 0;
  }
  if(pres) {
    delete pres;
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
  if(orig_v) {
    delete orig_v;
  }
  if(timer) {
    delete timer;
  }
  oinst_ = 0;
  return err;
}

int QG::writeBnbStatus_(BranchAndBound* bab)
{
  int err = 0;

  if(bab) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << std::fixed << std::setprecision(4)
        << "best solution value = " << objSense_ * ub_ << std::endl
        << me_ << std::fixed << std::setprecision(4)
        << "best bound estimate from remaining nodes = " << objSense_ * lb_
        << std::endl
        << me_ << "gap = " << std::max(0.0, ub_ - lb_) << std::endl
        << me_ << "gap percentage = " << bab->getPerGap() << std::endl
        << me_ << "time used (s) = " << std::fixed << std::setprecision(2)
        << env_->getTime(err) << std::endl
        << me_
        << "status of branch-and-bound = " << getSolveStatusString(status_)
        << std::endl;
    env_->stopTimer(err);
  } else {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << std::fixed << std::setprecision(4)
        << "best solution value = " << INFINITY << std::endl
        << me_ << std::fixed << std::setprecision(4)
        << "best bound estimate from remaining nodes = " << INFINITY
        << std::endl
        << me_ << "gap = " << INFINITY << std::endl
        << me_ << "gap percentage = " << INFINITY << std::endl
        << me_ << "time used (s) = " << std::fixed << std::setprecision(2)
        << env_->getTime(err) << std::endl
        << me_
        << "status of branch-and-bound: " << getSolveStatusString(NotStarted)
        << std::endl;
    env_->stopTimer(err);
  }
  return err;
}

/**
void writeSol_(EnvPtr env, VarVector* orig_v, PresolverPtr pres,
               SolutionPtr sol, SolveStatus status,
               MINOTAUR_AMPL::AMPLInterface* iface)
{
  Solution* final_sol = 0;
  if(sol) {
    final_sol = pres->getPostSol(sol);
  }

  if(env->getOptions()->findFlag("AMPL")->getValue() ||
     true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(final_sol, status);
  } else if(final_sol && env->getLogger()->getMaxLevel() >= LogExtraInfo &&
            env->getOptions()->findBool("display_solution")->getValue()) {
    final_sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
  } 

  if(final_sol) {
    delete final_sol;
  }
}
*/

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
