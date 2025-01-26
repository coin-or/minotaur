
//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

//
// Create a problem 
// Max 5x1x2 + 6x2x3 - 8x4 - x1 + 7.93
// subject to
//    x1 + x2 <= 8
//    7x2x3x4 + 3x1x2x3 <= 2
//    x1, x2, x3, x4 binary.
//

#include <iostream>
#include <iomanip>
#include <cmath>

#include "MinotaurConfig.h"
#include "BranchAndBound.h"
#include "Constraint.h"
#include "Environment.h"
#include "Engine.h"
#include "EngineFactory.h"
#include "Handler.h"
#include "IntVarHandler.h"
#include "Function.h"
#include "LexicoBrancher.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "LPEngine.h"
#include "PCBProcessor.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "MaxVioBrancher.h"
#include "Option.h"
#include "Objective.h"
#include "PolynomialFunction.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "Problem.h"
#include "QPEngine.h"
#include "QuadraticFunction.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"
#include "Types.h"
#include "Variable.h"

using namespace Minotaur;

void solveProb(ProblemPtr prob);
BranchAndBound * createBab (EnvPtr env, ProblemPtr p, EnginePtr e, 
    HandlerVector &handlers);
PresolverPtr createPres(EnvPtr env, ProblemPtr p, size_t ndefs, 
                        HandlerVector &handlers);
EnginePtr getEngine(EnvPtr env, ProblemPtr p);

void usage()
{
  std::cout << "usage: ./polynomial\n";
}


void createProb(ProblemPtr prob)
{
  VariablePtr x1, x2, x3, x4;
  FunctionPtr f;
  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf;
  PolyFunPtr pf;
  MonomialFunPtr mf;

  x1 = prob->newVariable(0, 1, Binary, "x1");
  x2 = prob->newVariable(0, 1, Binary, "x2");
  x3 = prob->newVariable(0, 1, Binary, "x3");
  x4 = prob->newVariable(0, 1, Binary, "x4");

  // objective function. In Minotaur, we always minimize, so negate the
  // function.
  qf = (QuadraticFunctionPtr) new QuadraticFunction();
  qf->incTerm(x1, x2, -5.0);
  qf->incTerm(x2, x3, -6.0);
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(x4, 8.0);
  lf->incTerm(x1, 1.0);
  f = (FunctionPtr) new Function(lf, qf);
  prob->newObjective(f, -7.93, Minimize, "name_of_obj");

  //    x1 + x2 <= 8
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->incTerm(x1, 1.0);
  lf->incTerm(x2, 1.0);
  f = (FunctionPtr) new Function(lf);
  prob->newConstraint(f, -INFINITY, 8.0, "cons-lin");

  //    7x2x3x4 + 3x1x2x3 <= 2
  pf = (PolyFunPtr) new PolynomialFunction();
  mf = (MonomialFunPtr) new MonomialFunction(1.0);
  mf->multiply(7.0, x2, 1); // multiply by 7*x2^1
  mf->multiply(1.0, x3, 1); // multiply by 1*x3^1
  mf->multiply(1.0, x4, 1); // multiply by 1*x4^1
  pf->add(mf);
  mf = (MonomialFunPtr) new MonomialFunction(3.0);
  mf->multiply(1.0, x1, 1); // multiply by 7*x2^1
  mf->multiply(1.0, x2, 1); // multiply by 7*x2^1
  mf->multiply(1.0, x3, 1); // multiply by 7*x2^1
  pf->add(mf);
  pf->createCG();
  f = (FunctionPtr) new Function(pf);
  prob->newConstraint(f, -INFINITY, 2.0, "cons-poly");
}


int main(int argc, char** argv)
{
  EnvPtr env = (EnvPtr) new Environment();
  ProblemPtr prob = (ProblemPtr) new Problem(env);

  if (argc > 1) {
    usage();
    goto CLEANUP;
  }

  createProb(prob);
  prob->write(std::cout);

  solveProb(prob);

CLEANUP:
  return 0;
}


void solveProb(ProblemPtr inst)
{
  EnvPtr env      = (EnvPtr) new Environment();
  Timer *timer    = env->getNewTimer();
  VarVector *orig_v=0;
  HandlerVector handlers;
  PresolverPtr pres;
  EnginePtr engine;    // engine for solving relaxations. 
  BranchAndBound * bab = 0;
  SolutionPtr sol, sol2;

  std::cout << "Minotaur version " << env->getVersion() << std::endl;

  timer->start();
  inst->calculateSize();
  engine = getEngine(env, inst);
  std::cout << "engine used = " << engine->getName() << std::endl;

  // get presolver.
  orig_v = new VarVector(inst->varsBegin(), inst->varsEnd());
  pres = createPres(env, inst, 0, handlers);
  pres->standardize(); 
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    std::cout << "Presolving ... " << std::endl;
    pres->solve();
    std::cout << "Finished presolving." << std::endl;
    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
      (*it)->writeStats(std::cout);
    }
  }
  handlers.clear();

  // get branch-and-bound
  bab = createBab(env, inst, engine, handlers);

  // start solving
  bab->solve();

  std::cout << "status of branch-and-bound: " 
    << getSolveStatusString(bab->getStatus()) << std::endl;
  // write solution
  sol = bab->getSolution(); // presolved solution needs translation.
  if (sol) {
    sol2 = pres->getPostSol(sol);
    sol = sol2;
    assert (sol);
    sol->writePrimal(std::cout,orig_v);
  }
  std::cout << "nodes created in branch-and-bound = " << 
    bab->getTreeManager()->getSize() << std::endl;
  std::cout << std::fixed << std::setprecision(4) << 
    "best bound estimate from remaining nodes = "
    <<  bab->getLb() << std::endl;
  std::cout << std::fixed << std::setprecision(4) << 
    "best solution value = " << bab->getUb() << std::endl;
  // bit->writeStats();
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    (*it)->writeStats(std::cout);
  }
  std::cout << "time used = " << std::fixed << std::setprecision(2) 
    << timer->query() << std::endl;

  engine->writeStats(std::cout);
  bab->getNodeProcessor()->getBrancher()->writeStats(std::cout);

CLEANUP:
  delete timer;
  if (bab) {
    delete bab;
  }
  if (orig_v) {
    delete orig_v;
  }

}


EnginePtr getEngine(EnvPtr env, ProblemPtr p)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL
  bool cont=false;

  p->calculateSize();
  if (p->isLinear()) {
    e = efac->getLPEngine();
    if (e) {
      delete efac;
      return e;
    } else {
      cont = true;
    }
  }

  if (true==cont || p->isQP()) {
    e = efac->getQPEngine();
    if (e) {
      delete efac;
      return e;
    } else {
      cont = true;
    }
  }

  e = efac->getNLPEngine();
  assert (e || (!"No engine available for this problem."));
  delete efac;
  return e;
}


BranchAndBound * createBab (EnvPtr env, ProblemPtr p, EnginePtr e, 
    HandlerVector &handlers)
{
  BranchAndBound *bab = new BranchAndBound(env, p);
  PCBProcessorPtr nproc;
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  LinearHandlerPtr l_hand = (LinearHandlerPtr) new LinearHandler(env, p);
  NlPresHandlerPtr nlhand;
  NodeIncRelaxerPtr nr;
  RelaxationPtr rel;
  BrancherPtr br;
  const std::string me("bnb main: ");

  handlers.push_back(v_hand);
  handlers.push_back(l_hand);
  if (!p->isLinear() && 
       true==env->getOptions()->findBool("use_native_cgraph")->getValue() &&
       true==env->getOptions()->findBool("nl_presolve")->getValue()) {
    nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
    handlers.push_back(nlhand);
  }
  nproc = (PCBProcessorPtr) new PCBProcessor(env, e, handlers);
  if (env->getOptions()->findString("brancher")->getValue() == "rel") {
    UInt t;
    ReliabilityBrancherPtr rel_br;
    rel_br    = (ReliabilityBrancherPtr) new ReliabilityBrancher(env, handlers);
    rel_br->setEngine(e);
    t = (p->getSize()->ints + p->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    rel_br->setThresh(t);
    std::cout << me << "setting reliability threshhold to " << t << std::endl;
    t = (UInt) p->getSize()->ints + p->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    rel_br->setMaxDepth(t);
    std::cout << me << "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="Filter-SQP") {
      rel_br->setIterLim(5);
    }
    std::cout << me << "reliability branching iteration limit = " 
              << rel_br->getIterLim() << std::endl;
    br = rel_br;
  } else if (env->getOptions()->findString("brancher")->getValue() 
      == "maxvio") {
    MaxVioBrancherPtr mbr = 
      (MaxVioBrancherPtr) new MaxVioBrancher(env, handlers);
    br = mbr;
  } else if (env->getOptions()->findString("brancher")->getValue() 
      == "lex") {
    LexicoBrancherPtr lbr = 
      (LexicoBrancherPtr) new LexicoBrancher(env, handlers);
    br = lbr;
  }
  std::cout << me << "brancher used = " << br->getName() << std::endl;
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  rel = (RelaxationPtr) new Relaxation(p, env);
  rel->calculateSize();
  rel->setNativeDer();
  //rel->setInitialPoint(p->getPrimal());
  nr->setRelaxation(rel);
  nr->setEngine(e);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);

  return bab;
}


PresolverPtr createPres(EnvPtr env, ProblemPtr p, size_t ndefs, 
                        HandlerVector &handlers)
{
  // create handlers for presolve
  PresolverPtr pres = PresolverPtr(); // NULL
  const std::string me("bnb main: ");

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
    std::cout << me << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
        ++h) {
      std::cout<<(*h)->getName()<<std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(p, env, handlers);
  return pres;
}

