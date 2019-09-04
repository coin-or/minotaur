//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file MiDfo.cpp
 * \brief The main function for solving mixed-integer derivative-free
 * optimization problems, by solving a sequence of piecewise linear
 * underestimator mixed-integer linear programs (MILPs). The initial
 * master model can be read from an ampl format (.nl). The MILPs are solved
 * using branch-and-bound implemented within MINOTAUR.
 * \author Prashant Palkar, IIT Bombay
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "Constraint.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "Function.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearFunction.h"
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
BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e);

BranchAndBound* createBab(EnvPtr env, ProblemPtr p, EnginePtr e, 
                          HandlerVector &handlers)
{
  BranchAndBound *bab = new BranchAndBound(env, p);
  NodeProcessorPtr nproc = NodeProcessorPtr(); // NULL
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env, p);
  NlPresHandlerPtr nlhand;
  NodeIncRelaxerPtr nr;
  RelaxationPtr rel;
  BrancherPtr br;
  const std::string me("midfo main: ");
  OptionDBPtr options = env->getOptions();
  SOS2HandlerPtr s2_hand;

  SOS1HandlerPtr s_hand = (SOS1HandlerPtr) new SOS1Handler(env, p);
  if (s_hand->isNeeded()) {
    s_hand->setModFlags(false, true);
    handlers.push_back(s_hand);
  }
  
  // add SOS2 handler here.
  s2_hand = (SOS2HandlerPtr) new SOS2Handler(env, p);
  if (s2_hand->isNeeded()) {
    s2_hand->setModFlags(false, true);
    handlers.push_back(s2_hand);
  }
  
  
  handlers.push_back(v_hand);
  if (true==options->findBool("presolve")->getValue()) {
    l_hand->setModFlags(false, true);
    handlers.push_back(l_hand);
  }
  if (!p->isLinear() && 
       true==options->findBool("presolve")->getValue() &&
       true==options->findBool("use_native_cgraph")->getValue() &&
       true==options->findBool("nl_presolve")->getValue()) {
    nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
    nlhand->setModFlags(false, true);
    handlers.push_back(nlhand);
  }
  if (handlers.size()>1) {
    nproc = (PCBProcessorPtr) new PCBProcessor(env, e, handlers);
  } else {
    nproc = (BndProcessorPtr) new BndProcessor(env, e, handlers);
  }
  br = createBrancher(env, p, handlers, e);
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  nr->setModFlag(false);
  rel = (RelaxationPtr) new Relaxation(p);
  rel->calculateSize();
  if (options->findBool("use_native_cgraph")->getValue() ||
      rel->isQP() || rel->isQuadratic()) {
    rel->setNativeDer();
  } else {
    rel->setJacobian(p->getJacobian());
    rel->setHessian(p->getHessian());
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
  const std::string me("midfo main: ");

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
  const std::string me("midfo main: ");

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
    env->getLogger()->errStream() <<  "No engine available for this problem."
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
  const std::string me("midfo main: ");

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
  const std::string me("midfo main: ");

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
  std::cout << "NLP-based branch-and-bound solver for convex MINLP" << std::endl
            << "Usage:" << std::endl
            << "To show version: midfo -v (or --display_version yes) "
            << std::endl
            << "To show all options: midfo -= (or --display_options yes)" 
            << std::endl
            << "To solve an instance: midfo --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("midfo main: ");

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
      << "NLP-based branch-and-bound solver for convex MINLP" << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env->getLogger()->msgStream(LogInfo)
    << me << "Minotaur version " << env->getVersion() << std::endl
    << me << "NLP-based branch-and-bound solver for convex MINLP" << std::endl;
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

  const std::string me("midfo main: ");
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


void print2dVec(std::vector<std::vector<double> > output)
{
   std::cout << std::endl;
   for (std::vector<std::vector<double> >::size_type i=0; i<output.size(); i++) {
      for (std::vector<double>::size_type j = 0; j < output[i].size(); j++) {
        std::cout << output[i][j] << ' ';
      }
      std::cout << std::endl;
   }
   std::cout << std::endl;
}


void print2dVec(std::vector<std::vector<UInt> > output)
{
   std::cout << std::endl;
   for (std::vector<std::vector<double> >::size_type i=0; i<output.size(); i++) {
      for (std::vector<double>::size_type j = 0; j < output[i].size(); j++) {
        std::cout << output[i][j] << ' ';
      }
      std::cout << std::endl;
   }
   std::cout << std::endl;
}


int getn(const char* myFile)
{
  std::ifstream myReadFile;
  myReadFile.open(myFile);
  UInt count=0;
  if (myReadFile) {
    std::string line;
    while (std::getline(myReadFile, line)) {
      // Parse the line using a string stream
      std::istringstream row(line);
      double field;
      while (row>> field) {
        //std::cout<< field <<'\t';
        count++;
      }
      //std::cout<<count<<" columns \n";
      break;
      //std::cout<<'\n';
    }
  }
  return count;
}


std::vector<std::vector<double> > readInput(const char* myFile, UInt numFields)
{
  std::vector<std::vector<double> > data;
  std::ifstream myReadFile;
  myReadFile.open(myFile);
  while (!myReadFile.eof()) { 
    for(int i = 0; i < 1; i++) {
      std::vector<double> tmpVec;
      double temp;
      for (UInt j = 0; j < numFields; j++) {
        myReadFile >> temp;
        tmpVec.push_back(temp);
      }    
      data.push_back(tmpVec);
    }    
  }
  //Remove the extra last row (tackle later)
  unsigned rowToDelete = data.size()-1;
  if (data.size() > rowToDelete) {
    data.erase(data.begin() + rowToDelete);
  }
  return data;
}


double evalFunction(std::vector<double>point) {
  double fval = 0;
  for (std::vector<std::vector<double> >::size_type i=0; i<point.size(); i++) {
    fval+= point[i]*point[i];
  }
  return fval;
}


void findCombs(UInt N, UInt K, std::vector<std::vector<UInt> >&combs)
{
   std::vector<UInt>temp;
   std::string bitmask(K, 1);  // K leading 1's
   bitmask.resize(N, 0);       // N-K trailing 0's

   // print integers and permute bitmask
   do {
       for (UInt i = 0; i < N; ++i) // [0..N-1] integers
       {
           if (bitmask[i]) {
             //std::cout << " " << i;
             temp.push_back(i);
           }
       }
       //std::cout << std::endl;
       temp.push_back(N);
       combs.push_back(temp);
       temp.clear();
   } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
   return;
}


SolutionPtr bnbMain(int argc, char** argv, ProblemPtr &oinst, MINOTAUR_AMPL::AMPLInterface* &iface, double* lb)
{
  SolutionPtr msol;
  EnvPtr env      = (EnvPtr) new Environment();
  OptionDBPtr options;
  //MINOTAUR_AMPL::AMPLInterface* iface = 0;
  //ProblemPtr oinst;    // instance that needs to be solved.
  EnginePtr engine = 0;    // engine for solving relaxations. 
  SolutionPtr sol, sol2;
  JacobianPtr jPtr;
  HessianOfLagPtr hPtr;
  BranchAndBound * bab = 0;
  PresolverPtr pres;
  const std::string me("bnb main: ");
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
  iface = new MINOTAUR_AMPL::AMPLInterface(env, "bnb");

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);

  overrideOptions(env);
  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

  if (!oinst) {
    loadProblem(env, iface, oinst, &obj_sense);
  }

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

  bab = createBab(env, oinst, engine, handlers);
  bab->solve();
  bab->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  engine->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    (*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
  }

  writeSol(env, orig_v, pres, bab->getSolution(), bab->getStatus(), iface);
  writeBnbStatus(env, bab, obj_sense);

  msol = (SolutionPtr) new Solution(pres->getPostSol(bab->getSolution()));
  *lb = obj_sense*bab->getUb();

CLEANUP:
  if (engine) {
    delete engine;
  }
  //if (iface) {
    //delete iface;
  //}
  if (bab) {
    delete bab;
  }
  if (orig_v) {
    delete orig_v;
  }

  //return 0;
  return msol;
}


int main(int argc, char** argv)
{
  EnvPtr menv      = (EnvPtr) new Environment();
  //OptionDBPtr options;
  MINOTAUR_AMPL::AMPLInterface* iface = 0;
  ProblemPtr oinst;    // instance that needs to be solved.
  //EnginePtr engine;    // engine for solving relaxations.
  SolutionPtr msol;
  //JacobianPtr jPtr;
  //HessianOfLagPtr hPtr;
  //BranchAndBound * bab = 0;
  //PresolverPtr pres;
  //const std::string me("midfo main: ");
  //VarVector *orig_v=0;
  //HandlerVector handlers;
  int merr = 0;
  //double obj_sense = 1.0;

  //MIDFO RELATED PARAMETERS
  bool shouldStop = false;
  UInt maxIter = 10, iter = 0;
  int Mlambda = 1000;
  std::vector<UInt>bigM;
  double lb = -INFINITY, ub = INFINITY, etol=1e-6, epsLambda = 5e-4;
  
  const char* inputFile = "MIDFOInputcp2dEx0.txt";      //initial interpolation points
  UInt n = getn(inputFile);
  std::vector<std::vector<double> > interpPoints = readInput(inputFile, n);     //interpolation points
  std::cout<<"\nINITIAL INTERPOLATION POINTS: read from file " << *inputFile << "\n";
  print2dVec(interpPoints);             
  std::vector<double>funcVals;                  //function values of interpolation points
  std::vector<double>temp;
  std::vector<double>::iterator minAt;
  const double* mipSol;
  UInt i=0;
  VariableConstIterator vit;
  VariableConstIterator vxbeg;
  VariablePtr v, veta;

  ProblemPtr cutGen = (ProblemPtr) new Problem();       //cut generating subproblem
  EnginePtr cutGenEngine;                               //engine for solving relaxations
  EngineStatus engineStatus;
  FunctionPtr f;                                        //objective function for cut-generating subproblem (null)
  std::vector<LinearFunctionPtr> cutGenCon(n+1);        //constraint functions for cut-generating subproblem
  std::vector<FunctionPtr> fp(n+1);                     //function pointers for constraints of cut-generating subproblem
  std::vector<LinearFunctionPtr> cuts;                  //cuts for the master problem
  std::vector<FunctionPtr> fm(n+1);                     //function pointers for cuts for master problem
  LinearFunctionPtr lf;

  std::vector<std::vector<UInt> >combs;
  std::vector<std::vector<UInt> >combsN;

  double **a = new double*[n+1];
  for(i = 0; i < n+1; ++i) {
    a[i] = new double[n+1];
  }
  const double *b = new double[n+1];
  double *c = new double[n+1];
  VariableConstIterator vbeg,vend;
  std::stringstream sstm;

  //READING THE MASTER MIP FROM .nl file (for now)
  //------------------------------
  //Generate initial cuts
  //Initialize data
  //Generate initial master MIP
  //------------------------------

  //Calculate initial function values (may be provided)
  for (std::vector<std::vector<double> >::size_type j=0; j<interpPoints.size(); j++) {
    funcVals.push_back(evalFunction(interpPoints[j]));
  }

  minAt = std::min_element(funcVals.begin(), funcVals.end());
  //std::cout << "min element at: " << std::distance(funcVals.begin(), minAt);
  ub = *minAt;

  //ITERATIONS BEGIN
  while (shouldStop == false) {
    iter++;
    std::cout<<"\n    ITERATION "<< iter <<"   "<<std::endl;
    std::cout<<"\n lb = "<<lb;
    std::cout<<"\n ub = "<<ub;

    //SOLVE THE MASTER (MILP)---------------------
    msol = bnbMain(argc, argv, oinst, iface, &lb);
    //--------------------------------------------
    if (!msol) {
      std::cout<<"\n Master MIP NOT SOLVED..ABORTING\n";
      break;
    }

    //CHECK STOPPING CRITERION
    if (fabs(lb - ub) < etol) {
      //Include epsilon checks later!
      //bool shouldStop = checkEpsilon();
      shouldStop = true;
      std::cout<<"\nPRIMAL AND DUAL BOUNDS CONVERGED..\n"<<std::endl;
    } else if (iter >= maxIter) {
      shouldStop = true;
      std::cout<<"\nITERATION LIMIT REACHED..\n"<<std::endl;
    } else {
 
      //EVALUATE FUNCTION AT NEW POINT
      mipSol = msol->getPrimal();
      if (temp.size()) { temp.clear();}
      i=0;
      for (vit=oinst->varsBegin(); 
         vit!=oinst->varsEnd(); ++vit, ++i) {
        if (((*vit)->getName()).find("x")!= std::string::npos) {
          temp.push_back(mipSol[i]);
          std::cout<<(*vit)->getName() << " = " << mipSol[i]<<"\n";
        }
      }
      
      //Get iterator for x variables (CAN BE NULL LATER IN THE CODE)
      //for (vit=oinst->varsBegin(); vit!=oinst->varsEnd(); ++vit) {
        //if (((*vit)->getName()).find("x")!= std::string::npos) {
          //vxbeg = vit;
          //break;
        //}
      //}

      if (!temp.size()) {
        std::cout << "\n VARIABLES X NOT FOUND IN SOLUTION! ABORTING...\n"; 
        break;
      }
      interpPoints.push_back(temp);
      std::cout<<"\nNEW SET OF INTERPOLATION POINTS\n";
      print2dVec(interpPoints);             
      funcVals.push_back(evalFunction(temp));
      minAt = std::min_element(funcVals.begin(), funcVals.end());

      //Update upper bound and best solution
      ub = *minAt;

      //FIND ALL NEW CUT COMBINATIONS
      UInt m = interpPoints.size();
      findCombs(m-1, n, combs);
      std::cout<<"\nNEW SET OF CUT COMBINATIONS\n";
      print2dVec(combs);

      //FIND CUTS USING THE NEW POINT
      for (UInt k=0; k<combs.size(); k++) {
        // Create the cut generating subproblem for the first time (LP)
        if (iter == 1 && k == 0) {
          //Update M(i) for first iteration (iter==1)
          //Find constraint with eta variable
          //Find the linear function associated with each constraint
          //for {j in 1..J} {
            //let M[j]:=0;
            //for {i in 1..n} {
                  //if c[j,i]<0 then {
                    //M[j] := M[j] + c[j,i]*lbx[i];
                  //}
                  //else {
                    //M[j] := M[j] + c[j,i]*ubx[i];		 
                  //}
            //}
            //M[j]:= M[j] + d[j] - lb;		 
          //}

          cutGen->newObjective(f, 0, Minimize, "noObj");
          sstm.str("");
          for (i=0; i<n;i++) {
            sstm <<"coef"<<i;
            cutGen->newVariable(-INFINITY, INFINITY, Continuous, sstm.str());
            sstm.str("");
          }
          cutGen->newVariable(-INFINITY, INFINITY, Continuous, "intercept");

          vbeg = cutGen->varsBegin();
          vend = cutGen->varsEnd();

          for (i=0; i<n+1; i++) {
            for (UInt j=0; j<n; j++) {
              a[i][j] = interpPoints[combs[k][i]][j];
            }
            a[i][n] = 1;        //coefficient of intercept always 1
            cutGenCon[i] = (LinearFunctionPtr) new LinearFunction(a[i], vbeg, vend, etol);
            sstm << "fitPoints" <<i;
            fp[i] = (FunctionPtr) new Function(cutGenCon[i]);
            cutGen->newConstraint(fp[i], funcVals[combs[k][i]], funcVals[combs[k][i]], sstm.str());
            sstm.str("");
          }
          
          cutGenEngine = getEngine(menv, cutGen, merr);
          if (merr) {
            std::cout<<"Error in getting engine for cut-generating subproblem.\n";
          }
        } else {
          cutGenEngine->clear();    
          
          for (i=0; i<n+1; i++) {
            for (UInt j=0; j<n; j++) {
              a[i][j] = interpPoints[combs[k][i]][j];
            }
            a[i][n] = 1;
            lf = (LinearFunctionPtr) new LinearFunction(a[i], vbeg, vend, etol);
            //std::cout<<"\ncon before "; cutGenCon[i]->write(std::cout);
            cutGenCon[i]->clearAll();
            cutGenCon[i]->add(lf);
            //std::cout<<"\ncon after "; cutGenCon[i]->write(std::cout);
            fp[i] = (FunctionPtr) new Function(cutGenCon[i]);
            cutGen->changeBound(cutGen->getConstraint(i), funcVals[combs[k][i]], funcVals[combs[k][i]]);
          }
        }

        //cutGen->write(std::cout);
        cutGenEngine->load(cutGen);    
        std::cout<<"SOLVING CUT GENERATING SUBPROBLEM.." << std::endl;
        cutGenEngine->solve();
        engineStatus = cutGenEngine->getStatus();
        std::cout << "engine status = " 
                  << cutGenEngine->getStatusString() << std::endl;
        if(engineStatus == ProvenOptimal || engineStatus == ProvenLocalOptimal) {
          cutGenEngine->getSolution()->writePrimal(std::cout);
        
          //ADD CUTS TO MASTER PROBLEM
          b = cutGenEngine->getSolution()->getPrimal();
          UInt M = 0; 
          //Get iterator for x variables
          for (vit=oinst->varsBegin(); vit!=oinst->varsEnd(); ++vit) {
            if (((*vit)->getName()).find("x")!= std::string::npos) {
              vxbeg = vit;
              break;
            }
          }
          //Calculate bigM for new cuts
          for (i=0; i<n; i++, vit++) {
            c[i] = b[i];
            if (c[i] > 0) {
              M += (*(vxbeg+i))->getUb()*c[i];
            } else {
              M += (*(vxbeg+i))->getLb()*c[i];
            }
          }
          M += b[n] - lb;       
          bigM.push_back(M);
          std::cout<<"M   "<<M<<"\n";
          
          //Add constraints to master
          //(1) Ccut1
          std::cout<<"\n";
          //Get iterator for eta variable
          for (vit=oinst->varsBegin(); 
             vit!=oinst->varsEnd(); ++vit, ++i) {
            if (((*vit)->getName()).find("eta")!= std::string::npos) {
              veta = *vit;
              break;
            }
          }

          std::vector<VariablePtr>zvar;
          cuts.push_back((LinearFunctionPtr) new LinearFunction(c, vxbeg, vxbeg + n, etol));
          for (i=0; i < n+1; i++) { 
            sstm.str(""); sstm << "z_" << cuts.size() << "_" << i;      //variable name
            zvar.push_back(oinst->newBinaryVariable(sstm.str()));        //n+1 z variables                        
            cuts.back()->addTerm(zvar.back(), bigM.back());
          }
          cuts.back()->addTerm(veta, -1);
          sstm.str(""); sstm << "Ccut1_" << cuts.size();                //cut name (includes id)
          fm.push_back((FunctionPtr) new Function(cuts.back()));
          (oinst->newConstraint(fm.back(), -INFINITY, bigM.back()-b[n], sstm.str()));
          //(2) Ccut2
          std::cout<<"\n";
          LinearFunctionPtr lcut = cuts.back()->clone();
          lcut->removeVar(veta,0);
          sstm.str(""); sstm << "eCut" << cuts.size();                //cut name (includes id)
          VariablePtr veCut = oinst->newVariable(-INFINITY, INFINITY, Continuous, sstm.str());
          lcut->addTerm(veCut, -1);
          sstm.str(""); sstm << "Ccut2_" << cuts.size();              //cut name (includes id)
          FunctionPtr feCut = (FunctionPtr) new Function(lcut);
          (oinst->newConstraint(feCut, bigM.back()-b[n], bigM.back()-b[n], sstm.str()));
          //(3) ConeDef1
          std::cout<<"\n";
          for (vit=oinst->varsBegin(); vit!=oinst->varsEnd(); ++vit) {
            if (((*vit)->getName()).find("x")!= std::string::npos) {
              vxbeg = vit;
              //std::cout<<"\nPtr to x[1]: "<<(*vxbeg)<<" "<<(*vxbeg)->getName();
              //if (*(vxbeg+1)) {
                //std::cout<<"\nPtr to x[2]: "<<(*(vxbeg+1))<<(*(vxbeg+1))->getName();
              //} else {
                //std::cout<< "Pointer error at x[2]\n";
              //}
              break;
            }
          }
          std::vector<std::vector<VariablePtr> >lambda;
          for (UInt p=0; p < n+1; p++) {
            lambda.resize(p+1);
            for (UInt r=0; r < n; r++) {
              sstm.str(""); sstm << "lambda_" << cuts.size() << "_" << p << "_" << r;
              lambda[p].push_back(oinst->newVariable(-INFINITY, INFINITY, Continuous, sstm.str()));
            }
          } 
          for (UInt p=0; p < n+1; p++) {
            for (i=0; i < n; i++) {
              LinearFunctionPtr coneDef1 = (LinearFunctionPtr) new LinearFunction();
              UInt r=0;
              for (UInt q=0; q<n+1; q++) {
                double norm = 0;
                if (q!=p) {
                  for (UInt s=0; s < n; s++) {
                    norm += pow(interpPoints[combs[k][p]][s] - interpPoints[combs[k][q]][s], 2);
                  }
                  norm = sqrt(norm);
                  //coneDef1->addTerm(lambda[p][r], (interpPoints[combs[k][p]][i] 
                                                   //- interpPoints[combs[k][q]][i])/norm);
                  coneDef1->addTerm(lambda[p][r], (interpPoints[combs[k][p]][i] - interpPoints[combs[k][q]][i]));
                  r++;
                }
              }
              for (vit=oinst->varsBegin(); vit!=oinst->varsEnd(); ++vit) {
                if (((*vit)->getName()).find("x")!= std::string::npos) {
                  vxbeg = vit;
                  //std::cout<<"\nPtr to x[1]: "<<(*vxbeg)<<" "<<(*vxbeg)->getName();
                  //if (*(vxbeg+1)) {
                    //std::cout<<"\nPtr to x[2]: "<<(*(vxbeg+1))<<(*(vxbeg+1))->getName();
                  //} else {
                    //std::cout<< "Pointer error at x[2]\n";
                  //}
                  break;
                }
              }
              coneDef1->addTerm((*(vxbeg+i)),-1);
              sstm.str(""); sstm << "CconeDef1_" << cuts.size() << "_" << p << "_" << i; 
              FunctionPtr fconeDef1 = (FunctionPtr) new Function(coneDef1);
              (oinst->newConstraint(fconeDef1, - interpPoints[combs[k][p]][i],
                                    - interpPoints[combs[k][p]][i], sstm.str()));
            }
          }
          //(4) ConeDef2
          std::cout<<"\n";
          for (UInt p=0; p < n+1; p++) {
            for (UInt r=0; r < n; r++) {
              LinearFunctionPtr coneDef2 = (LinearFunctionPtr) new LinearFunction();
              coneDef2->addTerm(lambda[p][r], -1);
              coneDef2->addTerm(zvar[p], Mlambda);
              sstm.str(""); sstm << "CconeDef2_" << cuts.size() << "_" << p << "_" << r; 
              FunctionPtr fconeDef2 = (FunctionPtr) new Function(coneDef2);
              (oinst->newConstraint(fconeDef2, -INFINITY, Mlambda, sstm.str()))->write(std::cout);
            }
          } 
          //(5) CuLambda1
          std::cout<<"\n";
          std::vector<std::vector<VariablePtr> >wvar;
          for (UInt p=0; p < n+1; p++) {
            wvar.resize(p+1);
            for (UInt r=0; r < n; r++) {
              sstm.str(""); sstm << "w_" << cuts.size() << "_" << p << "_" <<r;  //variable name
              wvar[p].push_back(oinst->newBinaryVariable(sstm.str()));           //w variables
              LinearFunctionPtr wLambda1 = (LinearFunctionPtr) new LinearFunction();
              wLambda1->addTerm(lambda[p][r], 1);
              wLambda1->addTerm(wvar[p][r], -Mlambda);
              sstm.str(""); sstm << "CuLambda1_" << cuts.size() << "_" << p << "_" << r; 
              FunctionPtr fwLambda1 = (FunctionPtr) new Function(wLambda1);
              (oinst->newConstraint(fwLambda1, -INFINITY, -epsLambda, sstm.str()));
            }
          } 
          //(6) CuLambda2
          std::cout<<"\n";
          for (UInt p=0; p < n+1; p++) {
            LinearFunctionPtr wLambda2 = (LinearFunctionPtr) new LinearFunction();
            for (UInt r=0; r < n; r++) {
              wLambda2->addTerm(wvar[p][r], 1);
            }
            wLambda2->addTerm(zvar[p], -1);
            sstm.str(""); sstm << "CuLambda2_" << cuts.size() << "_" << p; 
            FunctionPtr fwLambda2 = (FunctionPtr) new Function(wLambda2);
            (oinst->newConstraint(fwLambda2, -INFINITY, n - 1, sstm.str()));
          } 
          //(7) CuLambda3
          std::cout<<"\n";
          for (UInt p=0; p < n+1; p++) {
            LinearFunctionPtr wLambda3 = (LinearFunctionPtr) new LinearFunction();
            for (UInt r=0; r < n; r++) {
              wLambda3->addTerm(wvar[p][r], -1);
            }
            wLambda3->addTerm(zvar[p], n);
            sstm.str(""); sstm << "CuLambda3_" << cuts.size() << "_" << p; 
            FunctionPtr fwLambda3 = (FunctionPtr) new Function(wLambda3);
            (oinst->newConstraint(fwLambda3, -INFINITY, 0, sstm.str()));
          } 
          //break;

        } else {
          std::cout<<"\nPOINTS NOT GEOMETRICALLY WELL-POSED...\n";
        }
      } //for ends for new cuts
    } //else for stopping conditions ends
  } //while ends
  
  //Write optimal solution
  if (msol) {
    mipSol = msol->getPrimal();
    i=0;
    for (vit=oinst->varsBegin();
        vit!=oinst->varsEnd(); ++vit, ++i) {
      if (((*vit)->getName()).find("x")!= std::string::npos) {
        std::cout << ((*vit)->getName()) << "   " << mipSol[i] << std::endl;
      }
    }
  }
  std::cout<<"\nFinal set of interpolation points\n";
  print2dVec(interpPoints);             
  minAt = std::min_element(funcVals.begin(), funcVals.end());
  std::cout<<"BEST SOLUTION:\n";
  for (UInt j=0; j < n; j++) {
    std::cout<<"x["<<j+1<<"] = "
      <<(int)interpPoints[std::distance(funcVals.begin(), minAt)][j]<<std::endl;
  }
  //oinst->write(std::cout);

//CLEANUP:
  //if (iface) {
    //delete iface;
  //}
  //if (bab) {
    //delete bab;
  //}
  //if (orig_v) {
    //delete orig_v;
  //}

  for(i = 0; i < n+1; ++i) {
        delete [] a[i];
  }
  delete [] a;

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
