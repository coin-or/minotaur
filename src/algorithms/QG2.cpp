//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file QG2.cpp
 * \brief Define QG class for solving MINLPs
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <algorithm>
#include <cmath>
#include <ostream>
#include <iomanip>


#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "EngineFactory.h"
#include "Handler.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "LPEngine.h"
#include "MaxVioBrancher.h"
#include "NodeIncRelaxer.h"
#include "NlPresHandler.h"
#include "Option.h"
#include "PCBProcessor.h"
#include "QG2.h"
#include "QGHandler.h"
#include "QPEngine.h"
#include "RCHandler.h"
#include "Presolver.h"
#include "Problem.h"
#include "ReliabilityBrancher.h"
#include "NLPEngine.h"
#include "Objective.h"
#include "Reader.h"
#include "Timer.h"
#include "Types.h"

#include "AMPLInterface.h"
#include "AMPLJacobian.h"
#include "AMPLHessian.h"

using namespace Minotaur;
const std::string QG2::me_ = "QG2: ";


QG2::QG2(EnvPtr env) 
: env_(env),
  iface_(0),
  objSense_(1.0),
  oinst_(0)
{
}


QG2::~QG2() 
{
}


FileType QG2::getFileType(std::string fname)
{
  FileType ft = FileTypeNone;
  size_t pos = fname.find_last_of('.');

  if (pos != std::string::npos) {
    std::string ext = fname.substr(pos);
    for (size_t i=0; i<ext.length(); ++i) {
      ext[i] = std::toupper(ext[i]);
    }
    if (ext==".MPS") {
      ft = MPS;
    } else if (ext == ".NL") {
      ft = NL;
    } 
  } 
  return ft;
}


int QG2::getEngines_(Engine **nlp_e, LPEngine **lp_e)
{
  EngineFactory *efac = new EngineFactory(env_);
  oinst_->calculateSize();

  if (oinst_->isLinear()) {
    env_->getLogger()->errStream() << me_ 
      << "input problem is linear. Can not solve." << std::endl;
    delete efac;
    return 1;
  }

  if (oinst_->isQP()) {
    *nlp_e = efac->getQPEngine();
  } else {
    *nlp_e = efac->getNLPEngine();
  }

  *lp_e = efac->getLPEngine();
  delete efac;
  return 0;
}


int QG2::loadProblem()
{
  int err = 0;
  std::string fname = env_->getOptions()->findString("problem_file")->getValue();
  err = loadProblem(fname);

  return err;
}


int QG2::loadProblem(std::string fname)
{
  FileType ft;
  OptionDBPtr options = env_->getOptions();
  int err = 0;
  Timer *timer     = env_->getNewTimer();

  timer->start();
  ft = getFileType(fname);
  iface_ = 0;

  if (ft==MPS) {
    Reader rdr(env_);
    options->findString("interface_type")->setValue("mps");
    oinst_ = rdr.readMps(fname, err);
    if (err) {
      delete timer;
      return 1;
    }
  } else if ((ft==NL) || options->findString("interface_type")->getValue()=="AMPL") {
    iface_ = new MINOTAUR_AMPL::AMPLInterface(env_, "qg2");
    options->findString("interface_type")->setValue("AMPL");
    oinst_ = iface_->readInstance(fname);
    env_->getLogger()->msgStream(LogInfo) << me_ 
      << "time used in reading instance = " << std::fixed 
      << std::setprecision(2) << timer->query() << std::endl;
  } else {
    env_->getLogger()->errStream() << me_
      << "Unable to read the problem from file " << fname 
      << " Either provide a file with .nl or .mps extension or use -AMPL flag"
      << std::endl;
    delete timer;
    return 1;
  }
  return 0;
}


int QG2::loadProblem(ProblemPtr p)
{
  oinst_ = p;
  return 0;
}


PresolverPtr QG2::presolve_(HandlerVector &handlers)
{
  PresolverPtr pres = 0;

  oinst_->calculateSize();
  if (env_->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler = (LinearHandlerPtr) new LinearHandler(env_, oinst_);
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
      NlPresHandlerPtr nlhand = new NlPresHandler(env_, oinst_);
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
  pres = new Presolver(oinst_, env_, handlers);
  pres->standardize(); 
  if (env_->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
  return pres;
}


void QG2::showHelp() const
{ 
  env_->getLogger()->errStream()
      << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP" << std::endl
      << "Usage:" << std::endl
      << "To show version: qg -v (or --display_version yes) " << std::endl
      << "To show all options: qg -= (or --display_options yes)" << std::endl
      << "To solve an instance: qg --option1 [value] "
      << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int QG2::showInfo()
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
    env_->getLogger()->msgStream(LogNone) << me_ <<
      "Minotaur version " << env_->getVersion() << std::endl;
    env_->getLogger()->msgStream(LogNone) << me_ 
      << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP" << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env_->getLogger()->msgStream(LogInfo)
    << me_ << "Minotaur version " << env_->getVersion() << std::endl
    << me_ << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP"
    << std::endl;
  return 0;
}


int QG2::solve()
{
  OptionDBPtr options = env_->getOptions();
  oinst_->calculateSize();
  Timer *timer = env_->getNewTimer();
  EnginePtr nlp_e = 0;
  LPEnginePtr lp_e = 0;   // lp engine 
  VarVector *orig_v=0;
  BranchAndBound *bab = 0;
  PresolverPtr pres = 0;
  BrancherPtr br = BrancherPtr(); // NULL
  PCBProcessorPtr nproc;
  NodeIncRelaxerPtr nr;

  //handlers
  HandlerVector handlers;
  IntVarHandlerPtr v_hand;
  LinearHandlerPtr l_hand;
  QGHandlerPtr qg_hand;
  RCHandlerPtr rc_hand;

  int err = 0;


  timer->start();
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

  // get presolver.
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
    //writeSol(env_, orig_v, pres, SolutionPtr(), pres->getStatus(), iface_);
    //writeBnbStatus(env_, bab, objSense_);
    goto CLEANUP;
  }

  // create engines for solving LPs and NLPs
  err = getEngines_(&nlp_e, &lp_e); 
  if (err) {
    goto CLEANUP;
  }

  if (true==options->findBool("use_native_cgraph")->getValue()) {
    oinst_->setNativeDer();
  }
  if (options->findBool("rc_fix")->getValue() && 0) {
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
    env_->getLogger()->msgStream(LogExtraInfo) << me_ << "handlers used:"
      << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); ++h) {
      env_->getLogger()->msgStream(LogExtraInfo) << me_ << (*h)->getName()
        << std::endl;
    }

    // Only store bound-changes of relaxation (not problem)
    nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env_, handlers);
    nr->setModFlag(false);
    nr->setEngine(lp_e);
    nproc = (PCBProcessorPtr) new PCBProcessor(env_, lp_e, handlers);
    if (env_->getOptions()->findString("brancher")->getValue() == "rel") {
      ReliabilityBrancherPtr rel_br = 
        (ReliabilityBrancherPtr) new ReliabilityBrancher(env_, handlers);
      rel_br->setEngine(lp_e);
      nproc->setBrancher(rel_br);
      br = rel_br;
    } else if (env_->getOptions()->findString("brancher")->getValue()
               == "maxvio") {
      MaxVioBrancherPtr mbr = (MaxVioBrancherPtr) 
        new MaxVioBrancher(env_, handlers);
      nproc->setBrancher(mbr);
      br = mbr;
    } else if (env_->getOptions()->findString("brancher")->getValue()
               == "lex") {
      LexicoBrancherPtr lbr = (LexicoBrancherPtr) 
        new LexicoBrancher(env_, handlers);
      br = lbr;
    }
    nproc->setBrancher(br);
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "brancher used = " << br->getName() << std::endl;

    bab = new BranchAndBound(env_, oinst_);
    bab->setNodeRelaxer(nr);
    bab->setNodeProcessor(nproc);
    bab->shouldCreateRoot(true);

    // start solving
    bab->solve();

    bab->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
    nlp_e->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
    lp_e->writeStats(env_->getLogger()->msgStream(LogExtraInfo));

    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end();
         ++it) {
      (*it)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
    }

    //writeSol(env_, orig_v, pres, bab->getSolution(), bab->getStatus(), iface_);
    //writeBnbStatus(env_, bab, objSense_);

CLEANUP:
    delete timer;

}


int QG2::writeSol()
{
//   Solution* final_sol = 0;
//   if (sol) {
//     final_sol = pres->getPostSol(sol);
//   }
// 
//   if (env->getOptions()->findFlag("AMPL")->getValue() ||
//       true == env->getOptions()->findBool("write_sol_file")->getValue()) {
//     iface->writeSolution(final_sol, status);
//   } else if (final_sol && env->getLogger()->getMaxLevel()>=LogExtraInfo &&
//              env->getOptions()->findBool("display_solution")->getValue()) {
//     final_sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
//   }
// 
//   if (final_sol) {
//     delete final_sol;
//   }
}


