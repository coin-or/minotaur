//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2013 The MINOTAUR Team.
//

/**
 * \file QPDive.cpp
 * \brief The main function for solving instances in ampl format (.nl) by
 * using QP based diving in Branch-and-Bound.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "CxQuadHandler.h" // for presolving
#include "EngineFactory.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "LPEngine.h"
#include "MaxVioBrancher.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "Option.h"
#include "Presolver.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "QPDProcessor.h"
#include "QPDRelaxer.h"
#include "QPEngine.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"

#include "AMPLInterface.h"

using namespace Minotaur;

BranchAndBound * createBab (EnvPtr env, ProblemPtr p, EnginePtr qe, 
                            EnginePtr e, HandlerVector &handlers);
PresolverPtr createPres(EnvPtr env, ProblemPtr p, Size_t ndefs,
                        HandlerVector &handlers);
EnginePtr getEngine(EnvPtr env, ProblemPtr p);
void show_help();

void show_help()
{
  std::cout << "Usage:" << std::endl
            << "To show version: qpd -v (or --show_version yes) " << std::endl
            << "To show all options: qpd -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: qpd --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  OptionDBPtr options;

  // interface to AMPL (NULL)
  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();  
  ProblemPtr inst;    // instance that needs to be solved
  EnginePtr engine;    
  EnginePtr qe = EnginePtr();
  SolutionPtr sol, sol2;
  BranchAndBound * bab = 0; // the branch-and-bound
  PresolverPtr pres;
  VarVector *orig_v=0;
  const std::string me("qpd main: ");
  EngineFactory *efac;
  HandlerVector handlers;
  Int err = 0;

  // start timing.
  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }


  options = env->getOptions();
  options->findString("nlp_engine")->setValue("IPOPT");
  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) 
    new MINOTAUR_AMPL::AMPLInterface(env, "qpd");

  // parse options
  env->readOptions(argc, argv);
  options->findString("interface_type")->setValue("AMPL");
  // in bnb, modify bounds on original problem, not QP.
  options->findBool("modify_rel_only")->setValue(false);
  options->findBool("use_native_cgraph")->setValue(true);
  //options->findBool("presolve")->setValue(false);

  // check if user needs help.
  if (options->findBool("show_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    goto CLEANUP;
  }

  if (options->findBool("show_help")->getValue() ||
      options->findFlag("?")->getValue()) {
    show_help();
    goto CLEANUP;
  }

  if (options->findBool("show_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    std::cout << me << "Minotaur version " << env->getVersion() << std::endl;
#if DEBUG
    std::cout << me; 
    env->writeFullVersion(std::cout);
    std::cout << std::endl;
#endif
    goto CLEANUP;
  }

  if (options->findString("problem_file")->getValue()=="") {
    show_help();
    goto CLEANUP;
  }

  std::cout << me << "Minotaur version " << env->getVersion() << std::endl;
  // load the problem.
  inst = iface->readInstance(options->findString("problem_file")->getValue());
  std::cout << "time used in reading instance = " << std::fixed 
    << std::setprecision(2) << env->getTime(err) << std::endl; assert(0==err);

  // display the problem
  inst->calculateSize();
  if (options->findBool("display_problem")->getValue()==true) {
    inst->write(std::cout);
  }
  if (options->findBool("display_size")->getValue()==true) {
    inst->writeSize(std::cout);
  }

  // Get the right engine.
  engine = getEngine(env, inst);
  efac = new EngineFactory(env);
  qe = efac->getQPEngine();
  delete efac;
  assert(qe || (!"QP Engine not available!")); 

  // set initial point
  inst->setInitialPoint(iface->getInitialPoint(), 
      inst->getNumVars()-iface->getNumDefs());
   
  // get presolver.
  orig_v = new VarVector(inst->varsBegin(), inst->varsEnd());
  pres = createPres(env, inst, iface->getNumDefs(), handlers);
  pres->standardize(); 
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    std::cout << me << "Presolving ... " << std::endl;
    pres->solve();
    std::cout << me << "Finished presolving." << std::endl;
    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
      (*it)->writeStats(std::cout);
    }
  }
  handlers.clear();

  inst->setNativeDer();
  inst->setNativeDer();
  // get branch-and-bound
  bab = createBab(env, inst, engine, qe, handlers);

  // solve
  if (options->findBool("solve")->getValue()==true) {
    // start solving
    bab->solve();

    std::cout << me << "status of branch-and-bound: " 
      << getSolveStatusString(bab->getStatus()) << std::endl;
    // write solution
    sol = bab->getSolution(); // presolved solution needs translation.
    if (sol) {
      sol2 = pres->getPostSol(sol);
      sol = sol2;
    }
    if (options->findFlag("AMPL")->getValue()) {
      iface->writeSolution(sol, bab->getStatus());
    } else if (sol) {
      sol->writePrimal(std::cout,orig_v);
    }
    std::cout << me << "nodes created in branch-and-bound = " << 
      bab->getTreeManager()->getSize() << std::endl;
    std::cout << me << std::fixed << std::setprecision(4) << 
      "best bound estimate from remaining nodes = " <<  bab->getLb() 
      << std::endl;
    std::cout << me << std::fixed << std::setprecision(4) << 
      "best solution value = " <<  bab->getUb() << std::endl;
    // bit->writeStats();
    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
      (*it)->writeStats(std::cout);
    }
  }
  engine->writeStats();
  qe->writeStats();
  bab->getNodeProcessor()->getBrancher()->writeStats();
  bab->getNodeProcessor()->writeStats(std::cout);
  std::cout << me << "time used = " << std::fixed << std::setprecision(2) 
    << env->getTime(err) << std::endl; assert(0==err);


CLEANUP:
  if (iface) {
    delete iface;
  }
  if (bab) {
    delete bab;
  }
  if (orig_v) {
    delete orig_v;
  }

  return 0;
}


EnginePtr getEngine(EnvPtr env, ProblemPtr p)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL

  p->calculateSize();
  if (p->isLinear()) {
    e = efac->getLPEngine();
    if (e) {
      delete efac;
      return e;
    } 
  }

  e = efac->getNLPEngine();
  assert (e || (!"No engine available for this problem."));
  delete efac;
  return e;
}


BranchAndBound * createBab (EnvPtr env, ProblemPtr p, EnginePtr e, 
                            EnginePtr qe, HandlerVector &handlers)
{
  BranchAndBound *bab = new BranchAndBound(env, p);
  QPDProcessorPtr nproc;
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  LinearHandlerPtr l_hand = (LinearHandlerPtr) new LinearHandler(env, p);
  BrancherPtr br;
  QPDRelaxerPtr nr;
  const std::string me("qpd main: ");

  handlers.push_back(v_hand);
  handlers.push_back(l_hand);
  nproc = (QPDProcessorPtr) new QPDProcessor(env, p, e, qe, handlers);
  if (env->getOptions()->findString("brancher")->getValue() == "rel") {
    UInt t = 0;
    ReliabilityBrancherPtr rbr = (ReliabilityBrancherPtr)
      new ReliabilityBrancher(env, handlers);
    rbr->setEngine(qe);
    t = (p->getSize()->ints + p->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    rbr->setThresh(t);
    std::cout << me << "setting reliability threshhold to " << t << std::endl;
    t = (UInt) p->getSize()->ints + p->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    rbr->setMaxDepth(t);
    std::cout << me << "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="Filter-SQP") {
      rbr->setIterLim(5);
    }
    std::cout << me << "reliability branching iteration limit = " 
              << rbr->getIterLim() << std::endl;
    rbr->setTrustCutoff(false);
    nproc->setBrancher(rbr);
  } else if (env->getOptions()->findString("brancher")->getValue() == 
             "maxvio") {
    br    = (MaxVioBrancherPtr) new MaxVioBrancher(env, handlers);
    nproc->setBrancher(br);
  } else if (env->getOptions()->findString("brancher")->getValue() == "lex") {
    br = (LexicoBrancherPtr) new LexicoBrancher(env, handlers);
    nproc->setBrancher(br);
  }
  bab->setNodeProcessor(nproc);

  nr = (QPDRelaxerPtr) new QPDRelaxer(env, p, qe, e);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(true);

  return bab;

}


PresolverPtr createPres(EnvPtr env, ProblemPtr p, Size_t ndefs,
                        HandlerVector &handlers)
{
  // create handlers for presolve
  PresolverPtr pres = PresolverPtr(); // NULL
  p->calculateSize();
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler = (LinearHandlerPtr) new LinearHandler(env, p);
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
    std::cout << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
        ++h) {
      std::cout<<(*h)->getName()<<std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(p, env, handlers);
  return pres;
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
