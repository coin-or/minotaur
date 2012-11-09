//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2011 The MINOTAUR Team
// 

/*! \brief Test code for projection problem 
 *
 * \author Jim Luedtke, Mahdi Hamzeei and the MINOTAUR Team
 */

#include <iomanip>
#include <iostream>
//#include <MinotaurConfig.h>

#include <Environment.h>
#include <Handler.h>
#include <Option.h>
#include <QGHandler.h>
#include <Problem.h>
//#include <ProjProblem.h>
#include <Engine.h>
#include <QPEngine.h>
#include <LPEngine.h>
#include <NLPEngine.h>
// Jim, we don't need specific engines here. Use Enginefactory to get them.
//#include <FilterSQPEngine.h> 
//#include <OsiLPEngine.h>
#include <NodeRelaxer.h>
#include <NodeIncRelaxer.h>
#include <LPProcessor.h>
#include <MaxVioBrancher.h>
#include <QGHandler.h>
#include <ReliabilityBrancher.h>
#include <AMPLInterface.h>
#include <AMPLJacobian.h>
#include <AMPLHessian.h>
#include <BranchAndBound.h>
#include <Presolver.h>
#include <Timer.h>
#include <LexicoBrancher.h>
#include <Logger.h>
#include <LinearHandler.h>
#include <LinearFunction.h>
#include <Operations.h>
#include <QuadraticFunction.h>
#include <Function.h>
#include <IntVarHandler.h>
#include <Solution.h>
#include <TreeManager.h>
#include <EngineFactory.h>
#include <Types.h>
#include <ProjHessianOfLag.h>
#include <Variable.h>
#include <Objective.h>
//#include <CxQuadHandler.h>

//#define DEBUG1
//#define DEBUG2
#define DEBUG3
//#define VARTYPE

using namespace Minotaur;

void show_help();
void setValue(ProblemPtr p, Double * x0);
EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p);
void WriteSolution(EnginePtr e, UInt numvars);
void WriteVarTypes(ProblemPtr probPtr, Double *x1);
UInt InnerProductInteger(ProblemPtr probPtr, Double *x1);
void WriteDiff(Double *x1, Double *x2, UInt numvars);
void Obj_Func(ProblemPtr probPtr, Double *x1);
void WriteGr(Double *x4, ProblemPtr p_inst);

Double xbar = 20.0;

void show_help()
{
  std::cout << "Usage:" << std::endl
            << "To show version: qg -v (or --show_version yes) " << std::endl
            << "To show all options: qg -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: qg --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int main(int argc, char* argv[])
{

  UInt i;
  
  EnvPtr env = (EnvPtr) new Environment();
  Timer *timer = env->getNewTimer();
  OptionDBPtr options;
  Int error = 0;

  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();  // interface to AMPL (NULL)
  ProblemPtr orig_inst;
//  ProjProblemPtr inst;
  ProblemPtr proj_inst;
  SolutionPtr sol;
//  const Double *x;
  Double *x0;
  UInt numvars;

  JacobianPtr jPtr;
  HessianOfLagPtr orig_hPtr;
  HessianOfLagPtr proj_hPtr;

  LinearFunctionPtr proj_lf;
  QuadraticFunctionPtr proj_qf;
  FunctionPtr proj_fn;

  EngineStatus nlpStatus_;
  EngineFactory *efac;

  //engines
  EnginePtr nlp_e;
  LPEnginePtr lin_e;   // lp engine 
  LoggerPtr logger_ = (LoggerPtr) new Logger(LogInfo);

  //start timing
  timer->start();


  iface = (MINOTAUR_AMPL::AMPLInterfacePtr)
    new MINOTAUR_AMPL::AMPLInterface(env,"qg");

  //parse options
  env->readOptions(argc,argv);
  options = env->getOptions();
  options->findString("interface_type")->setValue("AMPL");


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
    std::cout << "Minotaur version " << env->getVersion() << std::endl;
    goto CLEANUP;
  }

  if (options->findString("problem_file")->getValue()=="") {
    show_help();
    goto CLEANUP;
  }

  if (options->findString("problem_file")->getValue()=="") {
    show_help();
    goto CLEANUP;
  }

  std::cout << "check quasiconvexity" << std::endl;
//  if (options->findBool("Convexity_type")->getValue()==true){
//    std::cout << "quasiconexity is assumed" << std::endl;
//  }
//  else std::cout << "convexity is assumed" << std::endl;

  // load the original problem problem.
  orig_inst = iface->readInstance(options->findString("problem_file")->getValue());
  //proj_inst = orig_inst->clone();
  proj_inst = iface->readInstance(options->findString("problem_file")->getValue());
  numvars = orig_inst->getNumVars();

#ifdef DEBUG1
  proj_lf = (LinearFunctionPtr) new LinearFunction();
  proj_qf = (QuadraticFunctionPtr) new QuadraticFunction();
  
  for (VariableConstIterator v_iter = proj_inst->varsBegin(); v_iter!=proj_inst->varsEnd(); v_iter++)
  {
     proj_qf->addTerm(*(v_iter),*(v_iter),0.5);
     proj_lf->addTerm(*(v_iter), 2.0);
  }

  proj_fn = (FunctionPtr) new Function(proj_lf, proj_qf);

  proj_inst->removeObjective();
  proj_inst->newObjective(proj_fn, 0, Minimize);
#endif

  std::cout << "time used in reading instance = " << std::fixed 
    << std::setprecision(2) << timer->query() << std::endl;

  proj_inst->calculateSize();

  // display the problem
//  if (options->findBool("display_problem")->getValue()==true) {
//    orig_inst->write(std::cout);
//  }

//  if (options->findBool("display_size")->getValue()==true) {
//    proj_inst->calculateSize();
//    proj_inst->writeSize(std::cout);
//  }

  // create the jacobian
  jPtr = (MINOTAUR_AMPL::AMPLJacobianPtr) new MINOTAUR_AMPL::AMPLJacobian(iface);
  orig_inst->setJacobian(jPtr);
  proj_inst->setJacobian(jPtr);

  orig_hPtr = (HessianOfLagPtr) new MINOTAUR_AMPL::AMPLHessian(iface);
  proj_hPtr = (HessianOfLagPtr) new ProjHessianOfLag( orig_inst, orig_hPtr, numvars);
  orig_inst->setHessian(orig_hPtr);

  proj_inst->setHessian(orig_hPtr);
#ifdef DEBUG2
  proj_inst->ProjObjective(proj_inst);
  x0 = new Double [numvars];
  setValue(proj_inst, x0);
  proj_inst->ProjObjUpdate(proj_inst, x0);
#endif

  proj_inst->calculateSize();

  // Initialize engines
  nlp_e = getNLPEngine(env, proj_inst);
  nlp_e->load(proj_inst);
  nlpStatus_ = nlp_e->solve();
 
//  WriteSolution(nlp_e, numvars);
  std::cout << "solution status of proj = " << nlp_e->getStatusString() << std::endl;
  std::cout << "solution value of proj = " << nlp_e->getSolutionValue() + 0.5 * InnerProductInteger(proj_inst, x0) << std::endl;


  std::cout << "time used = " << std::fixed << std::setprecision(2)
            << timer->query() << std::endl;

  const Double *x4;
  x4 = new Double [numvars];
  x4 = nlp_e->getSolution()->getPrimal();

  Double *y4;
  y4 = new Double [numvars];
  for (i = 0; i < numvars; ++i)
    y4[i] = x4[i];

//  Double obj_val1 = 0.0;
  std::cout << "calculate obj value of proj_inst = ";
  Obj_Func(proj_inst, y4);

  std::cout << "get obj value of proj_inst = " << proj_inst->getObjValue(x4, &error) << std::endl;

  WriteGr(y4, proj_inst);

#ifdef VARTYPE
  VariablePtr v;  
  for (UInt i = 0; i < proj_inst->getNumVars(); i++)
  {
    std::cout << "(" << i;
    vars_ = proj_inst->getVariable(i);
    if (v->getType() == Binary || v->getType() == Integer)
      std::cout << ",I) ";
    else
      std::cout << ",C) ";
  }
#endif

#ifdef DEBUG3
//  EnvPtr env1 = (EnvPtr) new Environment();
//  EnginePtr nlp_e2;
//  EngineStatus nlpStatus1_; 
  
  nlp_e = getNLPEngine(env, orig_inst);
  nlp_e->load(orig_inst);

  nlpStatus_ = nlp_e->solve();

//  WriteSolution(nlp_e, numvars);
  std::cout << "solution status of orig = " << nlp_e->getStatusString() << std::endl;
  std::cout << "solution value of orig = " << nlp_e->getSolutionValue() << std::endl;

  const Double *x5;
  x5 = new Double [numvars];
  x5 = nlp_e->getSolution()->getPrimal();
  Double *y5;
  y5 = new Double [numvars];
  for (i = 0; i < numvars; ++i)
    y5[i] = x5[i];

  std::cout << "obj value of orig_inst = ";
  Obj_Func(proj_inst, y4);

  
  
//  std::cout << "time used = " << std::fixed << std::setprecision(2)
//            << timer->query() << std::endl;

//  WriteDiff(y4, y5, numvars);

#endif
  //WriteVarTypes(proj_inst, x0);

CLEANUP:
  if (iface) {
    delete iface;
  }
  delete timer;

//  delete x;
#ifdef DEBUG2
  delete x0;
//  delete x4;
//  delete y5;
#endif
  return 0;
}

void WriteVarTypes(ProblemPtr probPtr, Double *x1)
{
  VariablePtr v;  
///*
//  std::cout << "number of vars = " << probPtr->getNumVars() << std::endl;
  for (UInt i = 0; i < probPtr->getNumVars(); i++)
  {
    std::cout << "(" << i << "," << x1[i];
    v = probPtr->getVariable(i);
    if (v->getType() == Binary || v->getType() == Integer)
      std::cout << ",I) ";
    else
      std::cout << ",C) ";
  }
//*/
  std::cout << std::endl;
}


UInt InnerProductInteger(ProblemPtr probPtr, Double *x1)
{
  UInt numvar = probPtr->getNumVars();
  Double sum = 0;
  VariablePtr v;
  for (Int i = 0; i < numvar; ++i)
  {
    v = probPtr->getVariable(i);
    if (v->getType() == Binary || v->getType() == Integer)
    {
      sum += x1[i] * x1[i];
    }
  }
  return sum;
}

///*
EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL
  Bool cont=false;

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

void setValue(ProblemPtr p, Double * x0)
{
  UInt nvar = p->getNumVars();
  UInt i;
  for (i = 0; i < nvar; i++)
    x0[i] = xbar;
//  x0[0] = 0.0;
//  x0[1] = 20.0;
//  x0[2] = 0.0;
}

void Obj_Func(ProblemPtr probPtr, Double *x1)
{
  Double obj_f = 0.0;
  VariablePtr v;
  UInt numvar = probPtr->getNumVars();
  for (Int i = 0; i < numvar; ++i)
  {
    v = probPtr->getVariable(i);
    if (v->getType() == Binary || v->getType() == Integer)
      obj_f += 0.5 * (x1[i] - xbar) * (x1[i] - xbar);    
  }
  std::cout << "obj value = " << obj_f << std::endl;
//  return obj_f;
}

void WriteSolution(EnginePtr e, UInt numvars)
{
  UInt i;
  const Double *x;
//  std::cout << "numvars = " << numvars << std::endl;
  x = e->getSolution()->getPrimal();

  for (i = 0; i < numvars; i++)
    std::cout << "x[" << i << "] = " << x[i] << " ";
  std::cout << std::endl;
}

void WriteDiff(Double *x6, Double *x7, UInt numvars1)
{

  for (Int i = 0; i < numvars1; i++)
    std::cout << "(" << x6[i] << "," << x7[i] << ") ";

  std::cout << std::endl;
  std::cout << "differences: " << std::endl;
  for (UInt i = 0; i < numvars1; i++)
    if (fabs(x6[i] - x7[i]) > 1e-4)
      std::cout << i << ": " << x6[i]-x7[i] << std::endl;
}

void WriteGr(Double *x4, ProblemPtr p_inst)
{
  std::cout << "objective gradient\n";
  Int numvars = p_inst->getNumVars();
  Double *gr = new Double[numvars];
  Int error = 0;
  p_inst->getObjective()->evalGradient(x4, gr, &error);
  VariablePtr v;
  for ( UInt i = 0; i < numvars; i++)
  {
    v = p_inst->getVariable(i);
    if (v->getType() == Binary || v->getType() == Integer)
      std::cout << "Dgr[" << i+1 << "] = " << gr[i] + xbar - x4[i] << ", ";
    else 
      std::cout << "Cgr[" << i+1 << "] = " << gr[i] << ", ";
  }

}
//*/

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:

