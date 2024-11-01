//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file BnbPar.cpp
 * \brief The main function for solving instances in ampl format (.nl) by
 * using a parallel implementation of Branch-and-Bound.
 * \author Prashant Palkar, IIT Bombay
 */

#include "BnbPar.h"

#include <iomanip>
#include <iostream>
#if USE_OPENMP
#include <omp.h>
#else
#error "Cannot compile parallel algorithms: turn USE_OpenMP flag ON."
#endif

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"
#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "FixVarsHeur.h"
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
#include "ParBranchAndBound.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "Problem.h"
#include "QPEngine.h"
#include "RandomBrancher.h"
#include "RCHandler.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "ParReliabilityBrancher.h"
#include "SamplingHeur.h"
#include "Solution.h"
#include "SOS1Handler.h"
#include "SOS2Handler.h"
#include "UnambRelBrancher.h"

using namespace Minotaur;
const std::string BnbPar::me_ = "BnbPar: ";

BnbPar::BnbPar(EnvPtr env)
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

BnbPar::~BnbPar() { }

void BnbPar::doSetup()
{
  setInitialOptions_();
}

int BnbPar::getEngines_(Engine** nlp_e, LPEngine** lp_e)
{
  EngineFactory* efac = new EngineFactory(env_);
  oinst_->calculateSize();

  if(oinst_->isLinear()) {
    *lp_e = efac->getLPEngine();
  } else {
    *nlp_e = efac->getNLPEngine();
  }
  //else if(oinst_->isQP()) {
  //  *nlp_e = efac->getNLPEngine();
  //  if(*nlp_e == 0) {
  //    *nlp_e = efac->getNLPEngine();
  //  }
  //} 


  delete efac;
  return 0;
}

PresolverPtr BnbPar::presolve_(HandlerVector &handlers)
{
  PresolverPtr pres = PresolverPtr(); // NULL
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

void BnbPar::setInitialOptions_()
{
  OptionDBPtr options = env_->getOptions();
  options->findString("interface_type")->setValue("ampl");
  options->findBool("presolve")->setValue(true);
  options->findBool("nl_presolve")->setValue(true);
  options->findBool("lin_presolve")->setValue(true);
  options->findString("brancher")->setValue("parrel");
  options->findString("nlp_engine")->setValue("ipopt");
  //options->findBool("cgtoqf")->setValue(true);
  //options->findBool("separability")->setValue(true);
  //options->findBool("simplex_cut")->setValue(true);
}

void BnbPar::showHelp() const
{
  env_->getLogger()->errStream()
       << "NLP-based parallel branch-and-bound solver for convex MINLP" << std::endl
      << "**Works in parallel with a thread-safe NLP solver only "
      << "(e.g. IPOPT with MA97)**" << std::endl
      << "Usage:" << std::endl
      << "To show version: qg -v (or --display_version yes) " << std::endl
      << "To show all options: qg -= (or --display_options yes)" << std::endl
      << "To solve an instance: qg --option1 [value] "
      << "--option2 [value] ... "
      << " .nl-file" << std::endl;
}

int BnbPar::showInfo()
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
        << me_ << "NLP-based parallel branch-and-bound solver for convex MINLP"
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
      << me_ << "NLP-based parallel branch-and-bound solver for convex MINLP"
      << std::endl;
  return 0;
}

ParBranchAndBound* BnbPar::createParBab_(UInt numThreads, EnginePtr e,
                                  RelaxationPtr relCopy[],
                                  ParPCBProcessorPtr nodePrcssr[],
                                  ParNodeIncRelaxerPtr parNodeRlxr[],
                                  HandlerVector handlersCopy[],
                                  EnginePtr eCopy[])
{
  ParBranchAndBound *bab = new ParBranchAndBound(env_, oinst_);
  OptionDBPtr options = env_->getOptions();
  bab->shouldCreateRoot(false);
 
  for(UInt i = 0; i < numThreads; ++i) {
    BrancherPtr br = 0;
    eCopy[i] = e->emptyCopy();
    IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env_, oinst_);
    LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env_, oinst_);
    NlPresHandlerPtr nlhand;
    SOS2HandlerPtr s2_hand;
    RCHandlerPtr rc_hand;
    
    SOS1HandlerPtr s_hand = (SOS1HandlerPtr) new SOS1Handler(env_, oinst_);
    if (s_hand->isNeeded()) {
      s_hand->setModFlags(false, true);
      handlersCopy[i].push_back(s_hand);
    } else {
      delete s_hand;
    }

    //adding RCHandler
    if (options->findBool("rc_fix")->getValue()) {
        rc_hand = (RCHandlerPtr) new RCHandler(env_);
        rc_hand->setModFlags(false, true);
        handlersCopy[i].push_back(rc_hand);
        assert(rc_hand);
    }

    // add SOS2 handler here.
    s2_hand = (SOS2HandlerPtr) new SOS2Handler(env_, oinst_);
    if (s2_hand->isNeeded()) {
      s2_hand->setModFlags(false, true);
      handlersCopy[i].push_back(s2_hand);
    } else {
      delete s2_hand;
    }

    handlersCopy[i].push_back(v_hand);
    if (true==options->findBool("presolve")->getValue()) {
      l_hand->setModFlags(false, true);
      handlersCopy[i].push_back(l_hand);
    }
    if (!oinst_->isLinear() && 
        true==options->findBool("presolve")->getValue() &&
        true==options->findBool("use_native_cgraph")->getValue() &&
        true==options->findBool("nl_presolve")->getValue()) {
      nlhand = (NlPresHandlerPtr) new NlPresHandler(env_, oinst_);
      nlhand->setModFlags(false, true);
      handlersCopy[i].push_back(nlhand);
    }

    // report name
    env_->getLogger()->msgStream(LogExtraInfo)
        << me_ << "handlers used:" << std::endl;
    for(HandlerIterator h = handlersCopy[i].begin(); h != handlersCopy[i].end(); ++h) {
      env_->getLogger()->msgStream(LogExtraInfo)
          << me_ << (*h)->getName() << std::endl;
    }


    br = createBrancher_(handlersCopy[i], eCopy[i]);
    relCopy[i] = (RelaxationPtr) new Relaxation(oinst_, env_);
    relCopy[i]->calculateSize();
    if (options->findBool("use_native_cgraph")->getValue() ||
        relCopy[i]->isQP() || relCopy[i]->isQuadratic()) {
      relCopy[i]->setNativeDer();
    } else {
      relCopy[i]->setJacobian(oinst_->getJacobian());
      relCopy[i]->setHessian(oinst_->getHessian());
    }

    nodePrcssr[i] = (ParPCBProcessorPtr) new ParPCBProcessor(env_, eCopy[i], handlersCopy[i]);
    nodePrcssr[i]->setBrancher(br);
    
    parNodeRlxr[i] = (ParNodeIncRelaxerPtr) new ParNodeIncRelaxer(env_, handlersCopy[i]);
    parNodeRlxr[i]->setModFlag(false);
    parNodeRlxr[i]->setRelaxation(relCopy[i]);
    parNodeRlxr[i]->setEngine(eCopy[i]);
  }
  
  // when using heuristic, check if engine copy[0] should be cleared etc.
  //if (options->findBool("pardivheur")->getValue()) {
  //  ParMINLPDivingPtr div_heur;
  //  if (true==options->findBool("use_native_cgraph")->getValue() ||
  //      relCopy[0]->isQP() || relCopy[0]->isQuadratic()) {
  //    oinst_->setNativeDer();
  //  }
  //  div_heur = (ParMINLPDivingPtr) new ParMINLPDiving(env_, oinst_, eCopy[0]);
  //  bab->addPreRootHeur(div_heur);
  //}
  //if (true == options->findBool("FPump")->getValue()) {
  //  EngineFactory efac(env_);
  //  EnginePtr lpe = efac.getLPEngine();
  //  //EnginePtr nlpe = e->emptyCopy();
  //  LinFeasPumpPtr lin_feas_pump = (LinFeasPumpPtr) 
  //    new LinFeasPump(env_, oinst_, eCopy[0], lpe);
  //  bab->addPreRootHeur(lin_feas_pump);
  //}
  if (env_->getOptions()->findBool("prerootheur")->getValue() == true) {
    if (env_->getOptions()->findBool("samplingheur")->getValue() == true) {
      SamplingHeurPtr s_heur = (SamplingHeurPtr) new SamplingHeur(env_, oinst_);
      bab->addPreRootHeur(s_heur);
    }

    if (env_->getOptions()->findBool("fixvarsheur")->getValue() == true) {
      FixVarsHeurPtr f_heur = (FixVarsHeurPtr) new FixVarsHeur(env_, oinst_);
      f_heur->setHandlers(handlersCopy[0]);
      bab->addPreRootHeur(f_heur);
    }
  } 
  return bab;
}


BrancherPtr BnbPar::createBrancher_(HandlerVector handlers, EnginePtr e)
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
    if (e->getName()=="filter-sqp") {
      rel_br->setIterLim(5);
    }
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "reliability branching iteration limit = " <<
      rel_br->getIterLim() << std::endl;
    br = rel_br;
  } else if (env_->getOptions()->findString("brancher")->getValue() == "parrel") {
    ParReliabilityBrancherPtr parRel_br;
    parRel_br = (ParReliabilityBrancherPtr) new ParReliabilityBrancher(env_, handlers);
    parRel_br->setEngine(e);
    t = (oinst_->getSize()->ints + oinst_->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    parRel_br->setThresh(t);
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "setting reliability threshhold to " << t << std::endl;
    t = (UInt) oinst_->getSize()->ints + oinst_->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    parRel_br->setMaxDepth(t);
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="filter-sqp") {
      parRel_br->setIterLim(5);
    }
    env_->getLogger()->msgStream(LogExtraInfo) << me_ <<
      "reliability branching iteration limit = " <<
      parRel_br->getIterLim() << std::endl;
    br = parRel_br;
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

int BnbPar::solve(ProblemPtr p)
{
  clock_t clockTimeStart = clock();
  OptionDBPtr options = env_->getOptions();
  EnginePtr nlp_e = 0;
  LPEnginePtr lp_e = 0; // lp engine
  VarVector* orig_v = 0;
  ParBranchAndBound* parbab = 0;
  // double wallTimeStart = parbab->getWallTime();  //use Timer: to be done!!!
  double wallTimeStart = 0.0;
  PresolverPtr pres = 0;
  UInt numThreads = 0;
  ParNodeIncRelaxerPtr *parNodeRlxr = 0;
  ParPCBProcessorPtr *nodePrcssr = 0; 

  RelaxationPtr *relCopy = 0;
  HandlerVector *handlersCopy = 0;
  //EngineFactory *efac = 0;
  EnginePtr *eCopy = 0;
  //ObjectivePtr oPtr = 0;
  //NodePtr node = 0;
  //bool prune = false;
  //SolutionPoolPtr solPool = 0;
 
  std::vector<double> nlpStats(9,0);
 
  // handlers
  HandlerVector handlers;
  int err = 0;

  oinst_ = p;
  if(oinst_->isQuadratic() && true == options->findBool("cgtoqf")->getValue()) {
    env_->getLogger()->msgStream(LogInfo)
        << me_ << "Using quadratic function to store quadratic problem."
        << std::endl;
    oinst_->cg2qf();
  }

  oinst_->calculateSize();

  if(options->findBool("display_problem")->getValue() == true) {
    oinst_->write(env_->getLogger()->msgStream(LogNone), 12);
  }
  
  if (env_->getOptions()->findInt("log_level")->getValue() >= 3 ) {
        options->findBool("display_size")->setValue(true);
        options->findBool("display_presolved_size")->setValue(true);
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
    writeParBnbStatus_(parbab, wallTimeStart, clockTimeStart);
    goto CLEANUP;
  }

  // create engines for solving LPs and NLPs
  err = getEngines_(&nlp_e, &lp_e);
  if(err) {
    goto CLEANUP;
  }

  if(true == options->findBool("use_native_cgraph")->getValue()) {
    oinst_->setNativeDer();
  }

  env_->getLogger()->msgStream(LogInfo) << "Number of processors = "
    << omp_get_num_procs() << std::endl;


  numThreads = std::min(env_->getOptions()->findInt("threads")->getValue(),
                        omp_get_max_threads());
  nodePrcssr = new ParPCBProcessorPtr[numThreads];
  parNodeRlxr = new ParNodeIncRelaxerPtr[numThreads];
  relCopy = new RelaxationPtr[numThreads];
  //pCopy = new ProblemPtr[numThreads];
  handlersCopy = new HandlerVector[numThreads];
  //efac = new EngineFactory(env_);
  //lpeCopy = new LPEnginePtr[numThreads];
  eCopy = new EnginePtr[numThreads];

  // If objective is nonlinear add an extra var name eta to move objective to
  // constraint in ParBnbHandlerAdvance
  //pCopy[0] = oinst_->clone(env_);
  //oPtr = oinst_->getObjective();
  //if (!oPtr) {
  //  assert(!"No objective function in the problem!");
  //} else if (oPtr->getFunctionType() != Linear &&
  //           oPtr->getFunctionType() != Constant) {
  //  pCopy[0]->newVariable(-INFINITY,INFINITY,Continuous,"eta");
  //}

  //for(UInt i=0; i < numThreads; ++i) {
  //  lpeCopy[i] = efac->getLPEngine();
  //  eCopy[i] = nlp_e->emptyCopy();
  //  if (i > 0) {
  //    pCopy[i] = pCopy[0]->clone(env_);
  //  }
  //}

  if (numThreads > 1) {
    env_->getLogger()->msgStream(LogInfo)
      << "**Works in parallel with a thread-safe "
      << "NLP solver only (e.g. IPOPT with MA97)**" << std::endl;
  } else {
    env_->getLogger()->msgStream(LogInfo)
      << "Number of threads = " << numThreads << std::endl;
  }

  if (oinst_->isLinear()) {
    parbab = createParBab_(numThreads, lp_e, relCopy, nodePrcssr, parNodeRlxr, handlersCopy, eCopy);
  } else {
    parbab = createParBab_(numThreads, nlp_e, relCopy, nodePrcssr, parNodeRlxr, handlersCopy, eCopy);
  }
  //edit wtime
  wallTimeStart = env_->getWTime();  
  if (true==env_->getOptions()->findBool("bnbpar_deter_mode")->getValue()) {
    //assert(!"Deterministic mode not available right now!");
    parbab->parsolveSync(parNodeRlxr, nodePrcssr, numThreads);
  } else if (true==env_->getOptions()->findBool("bnbpar_oppor_mode")->getValue()) {
    parbab->parsolveOppor(parNodeRlxr, nodePrcssr, numThreads);
  } else {
    parbab->parsolve(parNodeRlxr, nodePrcssr, numThreads);
  }
  
  
  status_ = parbab->getStatus();
  lb_ = parbab->getLb();
  ub_ = parbab->getUb();
  sol_= parbab->getSolution();

  err = writeSol_(env_, orig_v, pres, sol_, status_, iface_);
  if(err) {
    goto CLEANUP;
  }
 //Take care of important engine statistics
  for (UInt i=0; i < numThreads; i++) {
    eCopy[i]->fillStats(nlpStats);
  }
  writeNLPStats_(eCopy[0]->getName(), nlpStats);
  
  //Take care of important handler statistics
  //for (UInt i=0; i < numThreads; i++) {
    //env_->getLogger()->msgStream(LogExtraInfo) << "Thread " << i << std::endl;
    //for (HandlerVector::iterator it=handlersCopy[i].begin(); it!=handlersCopy[i].end(); ++it) {
      //(*it)->writeStats(env_->getLogger()->msgStream(LogExtraInfo));
    //}
  //}

  writeParBnbStatus_(parbab, wallTimeStart, clockTimeStart);


CLEANUP:
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
  for (UInt i=0; i < numThreads; i++) {
    for (HandlerVector::iterator it=handlersCopy[i].begin();
         it!=handlersCopy[i].end(); ++it) {
      delete (*it);
    }
    if (eCopy[i]) {
      delete eCopy[i];
    }
    if (parNodeRlxr[i]) {
      delete parNodeRlxr[i];
      relCopy[i] = 0;
    }
    if (nodePrcssr[i]) {
      delete nodePrcssr[i];
    }
  }
  if (nodePrcssr) {
    delete[] nodePrcssr;
  }
  if (parNodeRlxr) {
    delete[] parNodeRlxr;
  }
  if (relCopy) {
    delete[] relCopy; } if (eCopy) {
    delete[] eCopy;
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
  if(orig_v) {
    delete orig_v;
  }
  oinst_ = 0;
  return err;
}

void BnbPar::writeParBnbStatus_(ParBranchAndBound *parbab, double wallTimeStart, clock_t clockTimeStart)
{

  if (parbab) {
 
  
 //-----------------------------------------------------------------------------------------------
    env_->getLogger()->msgStream(LogInfo)
      << me_ << std::fixed << std::setprecision(4) 
      << "best solution value = " << objSense_*parbab->getUb() << std::endl
      << me_ << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = "
      <<  objSense_*parbab->getLb() << std::endl
      << me_ << "gap = " << std::max(0.0,parbab->getUb() - parbab->getLb())
      << std::endl
      << me_ << "gap percentage = " << parbab->getPerGap() << std::endl
      << me_ << "wall time used (s) = " << std::fixed << std::setprecision(2)
      << env_->getWTime() - wallTimeStart << std::endl
      << me_ << "process time used (s) = " << std::fixed << std::setprecision(2)
      << (double)(clock() - clockTimeStart)/CLOCKS_PER_SEC << std::endl
      << me_ << "status of branch-and-bound: " 
      << getSolveStatusString(parbab->getStatus()) << std::endl;
  } else {
    env_->getLogger()->msgStream(LogInfo)
      << me_ << std::fixed << std::setprecision(4)
      << "best solution value = " << INFINITY << std::endl
      << me_ << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = " << INFINITY << std::endl
      << me_ << "gap = " << INFINITY << std::endl
      << me_ << "gap percentage = " << INFINITY << std::endl
      << me_ << "time used (s) = " << std::fixed << std::setprecision(2) 
      << env_->getTime() << std::endl 
      << me_ << "status of branch-and-bound: " 
      << getSolveStatusString(NotStarted) << std::endl;
  }
}

void BnbPar::writeNLPStats_(std::string name, std::vector<double> stats) {
  if (stats.size()) {
    std::string me = me_ + "_" + name + ": ";
    env_->getLogger()->msgStream(LogExtraInfo)
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
