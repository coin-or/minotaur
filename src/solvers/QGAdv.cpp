//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/*! \brief Quesada-Grossmann(QG) algorithm for solving convex MINLP
 *
 * \authors Jeff Linderoth, MINOTAUR Team, and Meenarli Sharma, IIT Bombay
 */

#include <iomanip>
#include <iostream>
#include <fstream>

#include <MinotaurConfig.h>
#include <AMPLHessian.h>
#include <AMPLJacobian.h>
#include <Environment.h>
#include <Constraint.h>
#include <Function.h>
#include <LinearFunction.h>
#include <QuadraticFunction.h>
#include <Handler.h>
#include <Option.h>
#include <Operations.h>
#include <Problem.h>
#include <Engine.h>
#include <QPEngine.h>
#include <LPEngine.h>
#include <Logger.h>
#include <NLPEngine.h>
#include <NonlinearFunction.h>
#include "NlPresHandler.h"
#include <NodeRelaxer.h>
#include <Relaxation.h>
#include <NodeIncRelaxer.h>
#include <MaxVioBrancher.h>
#include <QGHandlerAdvance.h>
#include <ReliabilityBrancher.h>
#include <AMPLInterface.h>
#include <BranchAndBound.h>
#include <PCBProcessor.h>
#include <Presolver.h>
#include <Timer.h>
#include <LexicoBrancher.h>
#include <Logger.h>
#include <LinearHandler.h>
#include <IntVarHandler.h>
#include <Solution.h>
#include <TreeManager.h>
#include <EngineFactory.h>
#include <CxQuadHandler.h>
#include <Objective.h>
#include "RCHandler.h"
#include <TransSep.h>

using namespace Minotaur;

EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p);
void writeBnbStatus(EnvPtr env, BranchAndBound *bab, double obj_sense);
void writeSol(EnvPtr env, VarVector *orig_v, PresolverPtr pres,
              SolutionPtr sol, SolveStatus status,
              MINOTAUR_AMPL::AMPLInterface* iface);
//void objToCons(ProblemPtr inst);
typedef struct varDetail {
  UInt free = 0, lb = 0, ub = 0, both = 0;

} varDe;


//void problemDetail(ProblemPtr p)
//{
  //double ub, lb;
  //varDe b, i, r, ib, ii;
  //VariablePtr v;
  //ConstraintPtr c;
  //UInt vbin = 0, vint = 0, vreal = 0, vib = 0, vii = 0, cl = 0, cnl = 0,
       //cllb = 0, club = 0, cboth = 0, cqf = 0, cconst = 0, oc = 0, ov = 0, ceq = 0;
  //for (VariableConstIterator it=p->varsBegin(); it!=p->varsEnd(); ++it) {
    //v = *it;
    //lb = v->getLb();
    //ub = v->getUb();
    //switch(v->getType()) {
    //case ImplBin:
      //{
        //if (ub != INFINITY && lb != -INFINITY) {
          //++(ib.both);
        //} else if (ub != INFINITY) {
          //++(ib.ub);
        //} else if (lb != -INFINITY) {
          //++(ib.lb);
        //} else {
          //++(ib.free);
        //}
        //++vib;
        //break;
      //}
    //case ImplInt:
      //{
        //if (ub != INFINITY && lb != -INFINITY) {
          //++(ii.both);
        //} else if (ub != INFINITY) {
          //++(ii.ub);
        //} else if (lb != -INFINITY) {
          //++(ii.lb);
        //} else {
          //++(ii.free);
        //}
        //++vii;
        //break;
      //}
    //case Binary:
      //{
        //if (ub != INFINITY && lb != -INFINITY) {
          //++(b.both);
        //} else if (ub != INFINITY) {
          //++(b.ub);
        //} else if (lb != -INFINITY) {
          //++(b.lb);
        //} else {
          //++(b.free);
        //}
        //++vbin;
        //break;
      //}
    //case Integer:
      //{
        //if (ub != INFINITY && lb != -INFINITY) {
          //++(i.both);
        //} else if (ub != INFINITY) {
          //++(i.ub);
        //} else if (lb != -INFINITY) {
          //++(i.lb);
        //} else {
          //++(i.free);
        //}
        //++vint;
        //break;
      //}
    //case Continuous:
      //{
        //if (ub != INFINITY && lb != -INFINITY) {
          //++(r.both);
        //} else if (ub != INFINITY) {
          //++(r.ub);
        //} else if (lb != -INFINITY) {
          //++(r.lb);
        //} else {
          //++(r.free);
        //}
        //++vreal;
        //break;
      //}
    //default:
      //++ov;
      //break;
    //}
  //}


  //for (ConstraintConstIterator it=p->consBegin(); it!=p->consEnd(); ++it) {
    //c = *it;
    //switch(c->getFunctionType()) {
    //case Quadratic:
      //++cqf;
      //break;
    //case Nonlinear:
    //case Bilinear:
    //case Polynomial:
    //case Multilinear:
      //++cnl;
      //break;
    //case Linear:
      //{
        //++cl;
        //ub = c->getUb();
        //lb = c->getLb();
        //if (ub != INFINITY && lb != -INFINITY) {
          //if (lb == ub) {
            //++ceq;
          //} else {
            //++cboth;
          //}
        //} else if (ub != INFINITY) {
          //++club;
        //} else if (lb != -INFINITY) {
          //++cllb;
        //} else {
          //std::cout << "Constraints without bound \n";
          //exit(1);
        //}
      //}
      //break;
    //case Constant:
      //++cconst;
    //default:
      //++oc;
      //break;
    //}
  //}

  //if (cl != (cllb + club + cboth + ceq)) {
    //std::cout << "lin cons not matching \n";
  //}

  //bool nlObj = 0;
  //ObjectivePtr o = p->getObjective();
  //FunctionType fType = o->getFunctionType();

  //if (!o) {
    //assert(!"need objective in QG!");
  //} else if (fType != Linear && fType != Constant) {
    //nlObj = 1;
  //}
  ///// Is obj nonlinear? and No. of nonlinear, linear, quadratic, and other constraints
  //std::cout << "nlObj, nlCons, linCons, quadCon, other: " << nlObj << " " << cnl << " " << cl << " " << cqf << " " << oc << "\n";

  ///// Linear inequality with only lb, only ub, both lb and ub (but not equal),
  ////and equality linear constraints
  //std::cout << "LinCons: lb, ub, both, eq: " << cllb << " " << club << " " << cboth << " " << ceq << "\n";

  ///// No. of variables binary, int, real, and other
  //std::cout << "BVar, IntVar, RealVar, ImpBin, ImpInt: " << vbin << " " << vint << " " << vreal << " " << vib << " " << vii << "\n";

  ///// No. of respective vars with only lb, only ub, both bounds, no bounds
  //std::cout << "BinVar: lb, ub, both, none: " << b.lb << " " << b.ub << " " << b.both << " " << b.free << "\n";
  //std::cout << "IntVar: lb, ub, both, none: " << i.lb << " " << i.ub << " " << i.both << " " << i.free << "\n";
  //std::cout << "RealVar: lb, ub, both, none: " << r.lb << " " << r.ub << " " << r.both << " " << r.free << "\n";
  //std::cout << "ImplBinVar: lb, ub, both, none: " << ib.lb << " " << ib.ub << " " << ib.both << " " << ib.free << "\n";
  //std::cout << "ImplIntVar: lb, ub, both, none: " << ii.lb << " " << ii.ub << " " << ii.both << " " << ii.free << "\n";
//}

void loadProblem(EnvPtr env, MINOTAUR_AMPL::AMPLInterface* iface,
                 ProblemPtr &oinst, double *obj_sense)
{
  Timer *timer     = env->getNewTimer();
  OptionDBPtr options = env->getOptions();
  JacobianPtr jac;
  HessianOfLagPtr hess;
  const std::string me("qg: ");

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


//void objToCons(ProblemPtr problem)
//{
  //assert(problem);
  //ObjectivePtr oPtr = problem->getObjective();
  //FunctionType objFunType = oPtr->getFunctionType();

  //if (oPtr) {
    //if (objFunType == Constant || objFunType == Linear) {
      //return;
    //}
    //FunctionPtr f = oPtr->getFunction();

    //QuadraticFunctionPtr qf = f->getQuadraticFunction();
    //NonlinearFunctionPtr nlf = f->getNonlinearFunction();
    //// add a new variable
    //std::string name = "obj_dummy_var";
    //VariablePtr vPtr = problem->newVariable(-INFINITY, INFINITY,
        //Continuous, name);
    //// add this variable to the objective
    //LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    //lf->addTerm(vPtr, 1.0);
    //problem->addToObj(lf);
    //lf->multiply(-1.0);

    //if (qf) {
      //// remove quadratic parts from the objective
      //qf = problem->removeQuadFromObj();
      //// add a new constraint containing the new variable and the quadratic.
      //// qf - lf <= 0
    //}
    //if (objFunType == Nonlinear) {
      //nlf = problem->removeNonlinFromObj();
    //}

    //if (qf && nlf) {
      //f = (FunctionPtr) new Function(lf, qf, nlf);
    //} else if (qf) {
      //f = (FunctionPtr) new Function(lf, qf);
    //} else if (nlf) {
      //f = (FunctionPtr) new Function(lf, nlf);
    //}
      //problem->newConstraint(f, -INFINITY, 0.0);
  //}
//}

void setInitialOptions(EnvPtr env)
{
  env->getOptions()->findBool("presolve")->setValue(true);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(true);
  env->getOptions()->findBool("rc_fix")->setValue(false);
}


void showHelp()
{
  std::cout << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP"
            << std::endl
            << "Usage:" << std::endl
            << "To show version: qg -v (or --display_version yes) " << std::endl
            << "To show all options: qg -= (or --display_options yes)"
            << std::endl
            << "To solve an instance: qg --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("qg: ");

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
      << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP" << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env->getLogger()->msgStream(LogInfo)
    << me << "Minotaur version " << env->getVersion() << std::endl
    << me << "Quesada-Grossmann (LP/NLP) algorithm for convex MINLP"
    << std::endl;
  return 0;
}


PresolverPtr presolve(EnvPtr env, ProblemPtr p, size_t ndefs, HandlerVector &handlers)
{
  PresolverPtr pres = PresolverPtr(); // NULL
  const std::string me("qg: ");

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
  }
  return pres;
}

//For separability detection: Check separability if problem is not linear.
//TransSepPtr sepDetection(EnvPtr env, ProblemPtr p)
void sepDetection(EnvPtr env, ProblemPtr p)
{
  TransSepPtr sep = TransSepPtr();
  const std::string me("qg: ");

  //std::cout <<"Original Problem \n";
  //p->write(std::cout);
  //std::cout <<"---------------------------------------- \n";

  if (env->getOptions()->findBool("separability")->getValue() == true) {
    if (p -> isLinear()) {
      env ->getLogger()->msgStream(LogInfo) << me
        << "Problem is linear, skipping separability detection" 
        << std::endl;
    } else {
      sep = (TransSepPtr) new TransSep(env, p);
      sep->sepDetection();
      //env ->getLogger()->msgStream(LogDebug) << me
        //<< "Is problem separable? - "<< sep->getStatus() 
        //<< std::endl;
    }
  }

  //std::cout <<"Separable Problem \n";
  //p->write(std::cout);
  //exit(1);
}



int main(int argc, char* argv[])
{
  EnvPtr env = new Environment();
  OptionDBPtr options;

  //ConstSolutionPtr xc;

  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();
  ProblemPtr inst;

  double obj_sense =1.0;

  // the branch-and-bound
  BranchAndBound *bab = 0;
  PresolverPtr pres = 0;
  EngineFactory *efac;
  const std::string me("qg: ");

  BrancherPtr br = BrancherPtr(); // NULL
  PCBProcessorPtr nproc;

  NodeIncRelaxerPtr nr;

  //handlers
  //PerspConPtr prCons;
  HandlerVector handlers;
  RCHandlerPtr rc_hand;
  IntVarHandlerPtr v_hand;
  LinearHandlerPtr l_hand;
  QGHandlerAdvancePtr qg_hand;

  //engines
  EnginePtr nlp_e = 0;

  LPEnginePtr lin_e = 0;   // lp engine
  VarVector *orig_v=0;

  int err = 0;

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

  loadProblem(env, iface, inst, &obj_sense);
     
  // Separability detection
  sepDetection(env, inst); 

  // Initialize engines
  nlp_e = getNLPEngine(env, inst); //Engine for Original problem

  efac = new EngineFactory(env);
  lin_e = efac->getLPEngine();   // lp engine
  delete efac;

  // get presolver.
  orig_v = new VarVector(inst->varsBegin(), inst->varsEnd());
  pres = presolve(env, inst, iface->getNumDefs(), handlers);
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }

  //problemDetail(inst);

  handlers.clear();
  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo) << me
      << "status of presolve: "
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol(env, orig_v, pres, SolutionPtr(), pres->getStatus(), iface);
    writeBnbStatus(env, bab, obj_sense);
    goto CLEANUP;
  }

  if (options->findBool("solve")->getValue()==true) {
    if (true==options->findBool("use_native_cgraph")->getValue()) {
      inst->setNativeDer();
    }
    if (options->findBool("rc_fix")->getValue()) {
      rc_hand = (RCHandlerPtr) new RCHandler(env);
      rc_hand->setModFlags(false, true);
      handlers.push_back(rc_hand);
      assert(rc_hand);
    }
    // Initialize the handlers for branch-and-cut
    l_hand = (LinearHandlerPtr) new LinearHandler(env, inst);
    l_hand->setModFlags(false, true);
    handlers.push_back(l_hand);
    assert(l_hand);

    v_hand = (IntVarHandlerPtr) new IntVarHandler(env, inst);
    v_hand->setModFlags(false, true);
    handlers.push_back(v_hand);
    assert(v_hand);

    qg_hand = (QGHandlerAdvancePtr) new QGHandlerAdvance(env, inst, nlp_e);
    qg_hand->setModFlags(false, true);
    //if (env->getOptions()->findInt("root_linScheme3")->getValue() > 0 ||
        //env->getOptions()->findBool("root_linGenScheme1")->getValue() ||
        //env->getOptions()->findDouble("root_linGenScheme2_per")->getValue() > 0)
    if (env->getOptions()->findInt("root_linScheme3")->getValue() > 0) {
      qg_hand->setLpEngine(lin_e);
    }

    handlers.push_back(qg_hand);
    assert(qg_hand);

    // report name
    env->getLogger()->msgStream(LogExtraInfo) << me << "handlers used:"
      << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); ++h) {
      env->getLogger()->msgStream(LogExtraInfo) << me << (*h)->getName()
        << std::endl;
    }

    // Only store bound-changes of relaxation (not problem)
    nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
    nr->setModFlag(false);
    nr->setEngine(lin_e);
    nproc = (PCBProcessorPtr) new PCBProcessor(env, lin_e, handlers);
    if (env->getOptions()->findString("brancher")->getValue() == "rel") {
      ReliabilityBrancherPtr rel_br =
        (ReliabilityBrancherPtr) new ReliabilityBrancher(env, handlers);
      rel_br->setEngine(lin_e);
      nproc->setBrancher(rel_br);
      br = rel_br;
    } else if (env->getOptions()->findString("brancher")->getValue()
               == "maxvio") {
      MaxVioBrancherPtr mbr = (MaxVioBrancherPtr)
        new MaxVioBrancher(env, handlers);
      nproc->setBrancher(mbr);
      br = mbr;
    } else if (env->getOptions()->findString("brancher")->getValue()
               == "lex") {
      LexicoBrancherPtr lbr = (LexicoBrancherPtr)
        new LexicoBrancher(env, handlers);
      br = lbr;
    }
    nproc->setBrancher(br);
    env->getLogger()->msgStream(LogExtraInfo) << me <<
      "brancher used = " << br->getName() << std::endl;

    bab = new BranchAndBound(env, inst);
    bab->setNodeRelaxer(nr);
    bab->setNodeProcessor(nproc);
    bab->shouldCreateRoot(true);

    // start solving
    bab->solve();

    bab->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    nlp_e->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    lin_e->writeStats(env->getLogger()->msgStream(LogExtraInfo));

    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end();
         ++it) {
      (*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    }

    writeSol(env, orig_v, pres, bab->getSolution(), bab->getStatus(), iface);
    writeBnbStatus(env, bab, obj_sense);
  }

CLEANUP:
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    delete (*it);
  }
  if (lin_e) {
    delete lin_e;
  }
  if (nlp_e) {
    delete nlp_e;
  }
  if (iface) {
    delete iface;
  }
  if (pres) {
    delete pres;
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
  if (inst) {
    delete inst;
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


void writeBnbStatus(EnvPtr env, BranchAndBound *bab, double obj_sense)
{

  const std::string me("qg: ");
  int err = 0;

  if (bab) {
    env->getLogger()->msgStream(LogInfo)
      << me << std::fixed << std::setprecision(4)
      << "best solution value = " << obj_sense*bab->getUb() << std::endl
      << me << std::fixed << std::setprecision(4)
      << "best bound estimate from remaining nodes = "
      << obj_sense*bab->getLb() << std::endl
      << me << "gap = " << std::max(0.0,bab->getUb() - bab->getLb())
      << std::endl
      << me << "gap percentage = " << bab->getPerGap() << std::endl
      << me << "time used (s) = " << std::fixed << std::setprecision(2)
      << env->getTime(err) << std::endl
      << me << "status of branch-and-bound = "
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
