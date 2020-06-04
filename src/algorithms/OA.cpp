//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/*! \brief (Multitree) Outer-approximation algorithm for solving convex MINLP
 *
 * \authors Meenarli Sharma and Prashant Palkar, IIT Bombay
 */

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#if USE_OPENMP
#include <omp.h>
#else
#error "Cannot compile parallel algorithms: turn USE_OpenMP flag ON."
#endif
#include "MinotaurConfig.h"
#include "AMPLHessian.h"
#include "AMPLJacobian.h"
#include "Environment.h"
#include "Handler.h"
#include "Option.h"
#include "Problem.h"
#include "Engine.h"
#include "QPEngine.h"
#include "LinFeasPump.h"
#include "LPEngine.h"
#include "Logger.h"
#include "MILPEngine.h"
#include "Modification.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeRelaxer.h"
#include "NodeIncRelaxer.h"
#include "MaxVioBrancher.h"
#include "OAHandler.h"
#include "ReliabilityBrancher.h"
#include "AMPLInterface.h"
#include "BranchAndBound.h"
#include "PCBProcessor.h"
#include "Presolver.h"
#include "Timer.h"
#include "LexicoBrancher.h"
#include "Logger.h"
#include "LinearHandler.h"
#include "IntVarHandler.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "TreeManager.h"
#include "EngineFactory.h"
#include "CxQuadHandler.h"
#include "Objective.h"

using namespace Minotaur;

EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p);
//void writeBnbStatus(EnvPtr env, BranchAndBound *bab, double obj_sense);
void writeSol(EnvPtr env, VarVector *orig_v, PresolverPtr pres,
              SolutionPtr sol, SolveStatus status,
              MINOTAUR_AMPL::AMPLInterface* iface);


void loadProblem(EnvPtr env, MINOTAUR_AMPL::AMPLInterface* iface,
                 ProblemPtr &oinst, double *obj_sense)
{
  Timer *timer     = env->getNewTimer();
  OptionDBPtr options = env->getOptions();
  JacobianPtr jac;
  HessianOfLagPtr hess;
  const std::string me("oa: ");

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

//void linearizeObj(ProblemPtr problem)
//{
  //ObjectivePtr obj;
  //std::string name = "eta";
  //obj = problem->getObjective();

  //if (!obj) {
    //assert(!"No objective function in the problem!");
  //} else if (obj->getFunctionType() != Linear) {
    //FunctionPtr fold, fnew;
    //LinearFunctionPtr lfold        = obj->getLinearFunction();
    //const QuadraticFunctionPtr qf  = obj->getQuadraticFunction();
    //const NonlinearFunctionPtr nlf = obj->getNonlinearFunction();
    //LinearFunctionPtr lfnew        = (LinearFunctionPtr) new LinearFunction();
    //VariablePtr vPtr               = problem->newVariable(-INFINITY,INFINITY,
                                                           //Continuous,name); 
    //problem->resetInitialPoint(1);
    //if (!lfold) {
      //lfold = (LinearFunctionPtr) new LinearFunction();
    //}
    //lfold->addTerm(vPtr, -1.0);
    //if (nlf) {
      //fold = (FunctionPtr) new Function(lfold,nlf);
      //if (qf) {
        //fold = (FunctionPtr) new Function(lfold,qf,nlf);
      //}
    //}
    //name = "obj_Reform";
    //problem->newConstraint(fold, -INFINITY, 0.0,name);
    //problem->removeObjective();
    //lfnew->addTerm(vPtr, 1.0);
    //fnew = (FunctionPtr) new Function(lfnew);
    //problem->newObjective(fnew, obj->getConstant(), obj->getObjectiveType());
  //}
//}



void setInitialOptions(EnvPtr env)
{
  env->getOptions()->findBool("presolve")->setValue(true);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(true);
  env->getOptions()->findBool("separability")->setValue(false);
  env->getOptions()->findBool("perspective")->setValue(false);
}


void showHelp()
{
  std::cout << "Outer-approximation algorithm for convex MINLP"
            << std::endl
            << "Usage:" << std::endl
            << "To show version: oa -v (or --show_version yes) " << std::endl
            << "To show all options: oa -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: oa --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("oa: ");

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
    env->getLogger()->msgStream(LogNone) << me <<
      "Minotaur version " << env->getVersion() << std::endl;
    env->getLogger()->msgStream(LogNone) << me 
      << "Outer-approximation algorithm for convex MINLP" << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env->getLogger()->msgStream(LogInfo)
    << me << "Minotaur version " << env->getVersion() << std::endl
    << me << "Outer-approximation algorithm for convex MINLP"
    << std::endl;
  return 0;
}


PresolverPtr presolve(EnvPtr env, ProblemPtr p, size_t ndefs, 
                      HandlerVector &handlers)
{
  PresolverPtr pres = PresolverPtr(); // NULL
  const std::string me("bnb main: ");

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


double getPerGap(double objLb, double objUb)
{
  // for minimization problems, gap = (ub - lb)/(ub) * 100
  // so that if one has a ub, she can say that the solution can not be more
  // than gap% away from the current ub.
  double gap = 0.0, etol = 1e-6;
  if (objUb >= INFINITY) {
    gap = INFINITY;
  } else if ((objUb > etol) && (fabs(objLb) < etol)) {
    gap = 100.0;
  } else {
    gap = (objUb - objLb)/(fabs(objUb)+etol) 
      * 100.0;
    if (gap<0.0) {
      gap = 0.0;
    }
  }
  return gap;
}


void writeOAStatus(EnvPtr env, double gap, double objLb, double objUb, 
                   double obj_sense, SolveStatus status, UInt iterNum,
                   double time, double totSepTime, UInt solsPerIter,
                   UInt totNumSols)
{

  const std::string me("oa main: ");
  int err = 0;

  env->getLogger()->msgStream(LogInfo)
    << me << std::fixed << std::setprecision(4) 
    << "best solution value = " << obj_sense*objUb << std::endl
    << me << std::fixed << std::setprecision(4)
    << "best bound estimate = "
    <<  obj_sense*objLb << std::endl
    << me << "gap = " << std::max(0.0, objUb - objLb)
    << std::endl
    << me << "gap percentage = " << gap << std::endl
    << me << "iterations = " << iterNum << std::endl
    << me << "solutions separated = " << totNumSols << std::endl
    << me << "separation time = " << std::setprecision(2) << totSepTime << std::endl
    << me << "mean separation time = " << std::setprecision(2)
    << totSepTime/solsPerIter << std::endl
    << me << "time used (s) = " << std::fixed << std::setprecision(2) 
    //<< env->getTime(err) << std::endl
    << time << std::endl
    << me << "status of outer approximation: " << getSolveStatusString(status) << std::endl;
  env->stopTimer(err); assert(0==err);
}


void showStatus(EnvPtr env, double objLb, double objUb, double gap, int iterNum, UInt numSols)
{
  int err = 0;
  env->getLogger()->msgStream(LogInfo)
    << std::endl
    << "oa main: " 
    << std::fixed
    << std::setprecision(1)  << "time = "  << env->getTime(err)
    << std::setprecision(4)  << " lb = "   << objLb
    << std::setprecision(4)  << " ub = "   << objUb
    << std::setprecision(2)  << " gap% = " << gap
    << " iterations = " << iterNum 
    << " solutions = " << numSols
    << std::endl
    << std::endl;
}


/// Get wall clock time
double getWallTime() {
  struct timeval time;
  if (gettimeofday(&time,NULL)) {
    // Handle error
    return 0;
  }
  return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


bool shouldStop( EnvPtr env, SolveStatus &status, double gap, int iterNum, SolutionPoolPtr solPool, double wallTimeStart)
//bool shouldStop(EnvPtr env, SolveStatus &status, double gap, SolutionPoolPtr solPool)
{
  bool stop_oa = false;
  //int err = 0;

  if (gap <= 0.0) {
    stop_oa = true;
    status = SolvedOptimal;
  } else if ( gap <= env->getOptions()->findDouble("obj_gap_percent")->getValue()) {
    stop_oa = true;
    status = SolvedGapLimit;
  //} else if (env->getTime(err) > env->getOptions()->findDouble("bnb_time_limit")->getValue()) 
  } else if (getWallTime() - wallTimeStart > env->getOptions()->findDouble("bnb_time_limit")->getValue()) {
    stop_oa = true;
    status = TimeLimitReached;
  } else if (iterNum >= env->getOptions()->findInt("oa_iter_limit")->getValue()) {
    stop_oa = true;
    status = IterationLimitReached;
  } else if (solPool->getNumSolsFound() >= (UInt) env->getOptions()->findInt("bnb_sol_limit")->getValue()) {
    stop_oa = true;
    status = SolLimitReached;
  }

  return stop_oa;
}


int main(int argc, char* argv[])
{
  EnvPtr env = (EnvPtr) new Environment();
  OptionDBPtr options;

  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();
  ProblemPtr *inst = 0;
  double obj_sense =1.0, gap = INFINITY;
  double wallTimeStart = getWallTime();
  UInt numThreads = 0;
  
  PresolverPtr pres = 0;
  EngineFactory *efac = 0;
  const std::string me("oa: ");
  SolveStatus status;
  //handlers
  HandlerVector handlers;
  LinearHandlerPtr l_hand;
  OAHandlerPtr *oa_hand = 0;

  ModVector pmod, rmod;
  NodeIncRelaxerPtr nr = 0;
  SolutionPoolPtr solPool = 0;

  //engines
  EnginePtr *nlp_e = 0;
  MILPEnginePtr milp_e = 0;
  VarVector *orig_v=0;
  int err = 0;
  UInt numSols = 1, totNumSols = 0, solsPerIter = 0;
  
  // start timing.
  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  setInitialOptions(env);

  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) 
    new MINOTAUR_AMPL::AMPLInterface(env, "qg");

  // parse options
  env->readOptions(argc, argv);
  options = env->getOptions();
  options->findString("interface_type")->setValue("AMPL");

  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

  numThreads = std::min(env->getOptions()->findInt("threads")->getValue(),
                        omp_get_num_procs());
  omp_set_num_threads(numThreads);
  inst = new ProblemPtr[numThreads];
  oa_hand = new OAHandlerPtr[numThreads]();
  nlp_e = new EnginePtr[numThreads];

  loadProblem(env, iface, inst[0], &obj_sense);

  // Initialize engines
  efac = new EngineFactory(env);
  milp_e = efac->getMILPEngine();
  if (!milp_e) {
    assert(!"No MILP engine!");
  }

  delete efac; efac = 0;

  // get presolver.
  orig_v = new VarVector(inst[0]->varsBegin(), inst[0]->varsEnd());
  pres = presolve(env, inst[0], iface->getNumDefs(), handlers);
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }
  handlers.clear();
  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo) << me 
      << "status of presolve: " 
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol(env, orig_v, pres, SolutionPtr(), pres->getStatus(), iface);
    goto CLEANUP;
  }
 
  if (options->findBool("solve")->getValue()==true) {
    if (true==options->findBool("use_native_cgraph")->getValue()) {
      inst[0]->setNativeDer();
    }
    // Initialize the handlers for branch-and-cut
    l_hand = (LinearHandlerPtr) new LinearHandler(env, inst[0]);
    l_hand->setModFlags(false, true);
    assert(l_hand);
    handlers.push_back(l_hand);

    // MILP engine required only for one of the copies (oa_hand[0])
    nlp_e[0] = getNLPEngine(env, inst[0]);
    oa_hand[0] = (OAHandlerPtr) new OAHandler(env, inst[0], nlp_e[0], milp_e);
    oa_hand[0]->setModFlags(false, true);
    assert(oa_hand[0]);
    handlers.push_back(oa_hand[0]);

    //! initialize the MILP master problem by copying variables & linear constraints and by 
    //linearizing nonlinear constraints at the solution of NLP relaxation of the problem
    bool prune = false;
    RelaxationPtr milp = RelaxationPtr();
    nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
    nr->setModFlag(false);
    milp = nr->createRootRelaxation(NodePtr(), prune);
 
    for (UInt i=1; i < numThreads; ++i) {
      inst[i] = inst[0]->clone(env);
      nlp_e[i] = getNLPEngine(env, inst[i]);
      oa_hand[i] = (OAHandlerPtr) new OAHandler(env, inst[i], nlp_e[i], NULL);
      nlp_e[i]->load(inst[i]);
      oa_hand[i]->setModFlags(false, true);
      oa_hand[i]->setRelaxation(milp);
      oa_hand[i]->setObjVar(oa_hand[0]->getObjVar());
      oa_hand[i]->setObjType(oa_hand[0]->getObjType());
      oa_hand[i]->setNonlinCons(oa_hand[0]->getNonlinCons());
      assert(oa_hand[i]);
    }

    CutManager *cutMan = NULL;
    bool *solFound = new bool[numThreads];
    bool shouldPrune, shouldCont = true;
    double inf_meas;
    UInt iterNum = 0;
    ConstSolutionPtr sol;
    ConstSolutionPtr *sol2 = new ConstSolutionPtr[numThreads];
    solPool = (SolutionPoolPtr) new SolutionPool(env, inst[0], 1);
    SeparationStatus *sepStatus = new SeparationStatus[numThreads];
    double solAbsTol = env->getOptions()->findDouble("solAbs_tol")->getValue();
    double solRelTol = env->getOptions()->findDouble("solRel_tol")->getValue();
  
    double objLb = -INFINITY, objUb = INFINITY;

    //MS: add iteration limit in termination condition
    double time = 0, sepTimeStart = 0, totSepTime = 0;
    while (shouldCont) {
      //set best ub as upper cutoff for MILP engine
      oa_hand[0]->getMILPEngine()->setUpperCutoff(objUb);

      time = wallTimeStart - getWallTime() + env->getOptions()->findDouble("bnb_time_limit")->getValue();
      if (time > 0) {
        oa_hand[0]->getMILPEngine()->setTimeLimit(time); //set remaining time as limit for MILP solver
      } else {
        if (shouldStop(env, status, gap, iterNum, solPool, wallTimeStart) || sepStatus[0]==SepaPrune) {
          break;
        }
      }
      //! solve MILP master problem
      oa_hand[0]->solveMILP(&objLb, &sol, solPool, cutMan, status);
      if (status == SolvedInfeasible) {
        if (fabs(objUb) != INFINITY) {
          objLb = objUb;
          status = SolvedOptimal;
        }
        break;
      } else if (objUb-objLb <= solAbsTol || (objUb != 0 && (objUb - objLb < fabs(objUb)*solRelTol))) {
        status = SolvedOptimal;
        break;
      }
      iterNum++;
      //MS: different MILP engine status like unbounded, infeasible, and error to be handled
      solFound[0]  = oa_hand[0]->isFeasible(sol, RelaxationPtr(), shouldPrune, inf_meas);
      if (solFound[0]) {
        const double *x = sol->getPrimal();
        solPool->addSolution(x, sol->getObjValue());
        objUb = solPool->getBestSolutionValue();
        status = SolvedOptimal;
        gap = 0;
        showStatus(env, objLb, objUb, gap, iterNum, numSols);
        break;
      }
      // Update MILP by adding OA cuts at various solutions obtained from the
      // solution pool of MILP engine
      if (options->findBool("oa_use_solutions")->getValue() == true) {
        sepTimeStart = getWallTime();
        numSols = oa_hand[0]->getMILPEngine()->getNumSols();
        // id -1 fetches the incumbent from Cplex
        int j = -1;
        while (j <= int(numSols - 2)) {
#pragma omp parallel for
          for (UInt t=0; t < numThreads; ++t) {
            if (int(j + t) <= int(numSols -2)) {
              sol2[t] = oa_hand[0]->getMILPEngine()->getSolutionFromPool(j+t);
              oa_hand[t]->setRelObj(objLb);
              oa_hand[t]->separate(sol2[t], NodePtr(), milp, cutMan, solPool, pmod, rmod, &solFound[t], &sepStatus[t]);
              if (solFound[t]) {
#pragma omp critical
                {
                  objUb = solPool->getBestSolutionValue();
                  gap = getPerGap(objLb, objUb);
                  if (shouldStop(env, status, gap, iterNum, solPool, wallTimeStart)) {
                    j = numSols;
                    shouldCont = false;
                  }
                }
              }
              if (sepStatus[t] == SepaPrune) {
#pragma omp critical
                {
                  j = numSols;
                  shouldCont = false;
                  status = SolvedOptimal;
                  objUb = solPool->getBestSolutionValue();
                }
              }
            }
          }
          j = j + numThreads;
        }
        env->getLogger()->msgStream(LogInfo) << "number of solutions used = "
          << numSols << " separation time = " << getWallTime() - sepTimeStart
          << " separation time per sol = " << (getWallTime() - sepTimeStart)*numSols
          << std::endl;
      } else {
        // Solve NLP and update MILP by adding OA cuts
        oa_hand[0]->setRelObj(objLb);
        oa_hand[0]->separate(sol, NodePtr(), milp, cutMan, solPool, pmod, rmod, &solFound[0], &sepStatus[0]);
        if (solFound[0]) {
          objUb = solPool->getBestSolutionValue();
          gap = getPerGap(objLb, objUb);
          if (shouldStop(env, status, gap, iterNum, solPool, wallTimeStart)) {
            shouldCont = false;
            break;
          }
        }
        if (sepStatus[0] == SepaPrune) {
          shouldCont = false;
          status = SolvedOptimal;
          objUb = solPool->getBestSolutionValue();
          break;
        }
      }
      totNumSols += numSols;
      totSepTime += getWallTime() - sepTimeStart;
      solsPerIter += ceil((double(numSols)/double(numThreads)));
      gap = getPerGap(objLb, objUb);
      showStatus(env, objLb, objUb, gap, iterNum, numSols);
    } // end while (objLo <= objUp) or time limit
    gap = getPerGap(objLb, objUb);
    time = -wallTimeStart + getWallTime();
    for (UInt i=0; i < numThreads; ++i) {
      nlp_e[i]->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    }
    milp_e->writeStats(env->getLogger()->msgStream(LogExtraInfo));

    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end();
         ++it) {
      (*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    }
    //MS: Other solve status and right way of writing them
    //writeSol(env, orig_v, pres, solPool->getBestSolution(), solveStatus, iface);
    solPool->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    writeOAStatus(env, gap, objLb, objUb, obj_sense, status, iterNum, time,
                  totSepTime, solsPerIter, totNumSols);
  }

CLEANUP:
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }
  for (UInt i=0; i < numThreads; i++) {
    if (inst[i]) {
      delete inst[i];
    }
    if (nlp_e[i]) {
      delete nlp_e[i];
    }
    if (oa_hand[i] && i>0) {
      delete oa_hand[i];
    }
  }
  if (milp_e) {
    delete milp_e;
  }
  if (efac) {
    delete efac;
  }
  if (solPool) {
    delete solPool;
  }
  if (nr) {
    delete nr;
  }
  if (iface) {
    delete iface;
  }
  if (pres) {
    delete pres;
  }
  if (orig_v) {
    delete orig_v;
  }
  if (env) {
    delete env;
  }
  return 0;
}


EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p)
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


void writeSol(EnvPtr env, VarVector *orig_v,
              PresolverPtr pres, SolutionPtr sol, SolveStatus status,
              MINOTAUR_AMPL::AMPLInterface* iface)
{
  Solution* final_sol = 0;
  if (sol) {
    final_sol = pres->getPostSol(sol);
  }

  if (env->getOptions()->findFlag("AMPL")->getValue() ||
      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(final_sol, status);
  } else if (final_sol && env->getLogger()->getMaxLevel()>=LogExtraInfo &&
             env->getOptions()->findBool("display_solution")->getValue()) {
    final_sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
  }

  if (final_sol) {
    delete final_sol;
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
//
