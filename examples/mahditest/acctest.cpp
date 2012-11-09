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

#include <CNode.h>
#include <CGraph.h>
#include <Constraint.h>
#include <Function.h>
#include <NonlinearFunction.h>
#include <Objective.h>


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
#include <HessianOfLag.h>
#include <Jacobian.h>
#include <Variable.h>
#include <Objective.h>

//#include <CxQuadHandler.h>

//#define DEBUG1
//#define DEBUG2
#define DEBUG3
//#define VARTYPE

using namespace Minotaur;


void show_help()
{
  std::cout << "Usage:" << std::endl
            << "To show version: qg -v (or --show_version yes) " << std::endl
            << "To show all options: qg -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: qg --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}
// return the appropriate Engine
EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p);

// calculate the ||x-b||
Double norm_(const Double *x_, std::vector<Double> b_, Int n_);

// return log(x^2 + y^2)
FunctionPtr getFunc1();

// return x^2+3y + cos(x^2+3y-30)
FunctionPtr getFunc2();

// return Log Barrier function
FunctionPtr createLogBar();

ProblemPtr inst;

FunctionPtr logbar;

void add_cut(const Double *x_, std::vector<Double>b_, Int m_);

std::vector<VariablePtr> vars;
std::vector<ConstraintPtr> cons;

std::vector<VariablePtr>slack;

std::vector<FunctionPtr>fn1;
//FunctionPtr fn2;
std::vector<Double>rhs;

UInt numVarOrig;
//int main(int argc, char* argv[])
int main()
{

  EnvPtr env = (EnvPtr) new Environment();
//  Timer *timer = env->getTimer();
//  OptionDBPtr options;

//  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();
  inst = ProblemPtr( new Problem);
//  Problem inst = new Problem();
//  SolutionPtr sol;
//  UInt numvars;
  
//  JacobianPtr jPtr;
//  HessianOfLagPtr hPtr;

  EngineStatus nlpStatus_;
  EngineFactory *efac;

  //engines
  EnginePtr nlp_e;
  LPEnginePtr lin_e;   // lp engine 
//  LoggerPtr logger_ = (LoggerPtr) new Logger(LogInfo);


  vars.push_back(inst->newVariable(0.0,15,Continuous,"X"));
  vars.push_back(inst->newVariable(0,20,Integer,"Y"));
  numVarOrig = vars.size();
  
  fn1.push_back(getFunc1());
//  cons.push_back(inst->newConstraint(fn1, -INFINITY, 5, "1st")); 
  rhs.push_back(5);

  fn1.push_back(getFunc2());
//  fn2 = getFunc2();
//  cons.push_back(inst->newConstraint(fn2, -INFINITY, 45, "2nd"));
  rhs.push_back(45);
 
  FunctionPtr fn3;
  LinearFunctionPtr lf3 = LinearFunctionPtr(new LinearFunction);
//  vars.push_back(inst->newVariable(0.0,INFINITY,Continuous,"S1"));
  lf3->addTerm(vars[0],-5);
  lf3->addTerm(vars[1],3);
//  lf3->addTerm(vars[2],1);
  fn3 = FunctionPtr(new Function(lf3));
  cons.push_back(inst->newConstraint(fn3,-INFINITY,15,"3rd"));

  FunctionPtr fn4;
  LinearFunctionPtr lf4 = LinearFunctionPtr(new LinearFunction);
//  vars.push_back(inst->newVariable(0.0,INFINITY,Continuous,"S2"));
  lf4->addTerm(vars[0],15);
  lf4->addTerm(vars[1],2);
//  lf4->addTerm(vars[3],1);
  fn4 = FunctionPtr(new Function(lf4));
  cons.push_back(inst->newConstraint(fn4,-INFINITY,150,"4rd"));
  
  FunctionPtr fn5;
  LinearFunctionPtr lf5 = LinearFunctionPtr(new LinearFunction);
//  vars.push_back(inst->newVariable(0.0,INFINITY,Continuous,"S3"));
  lf5->addTerm(vars[0],5);
  lf5->addTerm(vars[1],-1);
//  lf5->addTerm(vars[4],1);
  fn5 = FunctionPtr(new Function(lf5));
  cons.push_back(inst->newConstraint(fn5,-INFINITY,55,"5rd"));

  FunctionPtr objfn;
  LinearFunctionPtr lfobj = LinearFunctionPtr(new LinearFunction);
  lfobj->addTerm(vars[0],-1);
  lfobj->addTerm(vars[1],1);
  objfn = FunctionPtr(new Function(lfobj));
  FunctionPtr objfn1 = createLogBar();

  QuadraticFunctionPtr qf = (QuadraticFunctionPtr) new QuadraticFunction();
  LinearFunctionPtr lf2 = (LinearFunctionPtr) new LinearFunction();
  qf->addTerm(vars[0],vars[0], 0.5);
  qf->addTerm(vars[1],vars[1], 1.0);
  lf2->addTerm(vars[0], -5);
  lf2->addTerm(vars[1], 6);
  FunctionPtr objfn2 = (FunctionPtr) new Function(lf2, qf);
  inst->newObjective(objfn1, 0.0, Minimize, "ACCTest");


  inst->calculateSize(); 
  inst->write(std::cout);


  /* **************** */
  /// Hessian and Jacobian for Log Barrier Objective Function

   // Jacobian
  JacobianPtr jPtr = (JacobianPtr) new Jacobian(cons, vars.size());
  inst->setJacobian(jPtr);

 // Creating Hessian
  HessianOfLagPtr hPtr = (HessianOfLagPtr) new HessianOfLag(inst.get());
  inst->setHessian(hPtr);

  /* **************** */


  /* **************** */
  // Jacobian and Hessian for Quadratic Objective Function

  //inst->setQuadraticJacobian();
  //inst->setQuadraticHessian();
  /* **************** */


  inst->calculateSize();
  inst->write(std::cout);

  // Solve
  env->getOptions()->findString("nlp_engine")->setValue("IPOPT");
  nlp_e = getNLPEngine(env, inst);
  nlp_e->load(inst);
  std::cout << nlp_e->getName() << std::endl;
  nlpStatus_ = nlp_e->solve();
  const Double *x = new Double[vars.size()];
 
  std::cout << "Solution status of inst = " << nlp_e->getStatusString() << std::endl;
    x = nlp_e->getSolution()->getPrimal();
    for (UInt i = 0; i < vars.size(); i++)
      std::cout << "x[" << i << "]=" << x[i] << "\n";


  Int iter = 0;
  while (norm_(x, rhs, 2)>1e-4 && iter < 10)
  {
    nlp_e->clear();
    add_cut(x, rhs, 2);
    iter++; 
    std::cout << "1\n";
    objfn1 = createLogBar();
//    objfn1->write(std::cout);
    inst->removeObjective();
    inst->newObjective(objfn1, 0, Minimize);
    //delete jPtr;
    //delete hPtr;
    std::cout << "Problem\n";
    inst->write(std::cout);

    inst->calculateSize(true);
    // Jacobian
    std::cout << "Jac\n";
    jPtr = (JacobianPtr) new Jacobian(cons, vars.size());
    inst->setJacobian(jPtr);

    // Creating Hessian
    std::cout << "Hess\n";
    HessianOfLagPtr hPtr1 = (HessianOfLagPtr) new HessianOfLag(inst.get());
    inst->setHessian(hPtr1);
    
    nlp_e->load(inst);
    nlpStatus_ = nlp_e->solve();
    x = nlp_e->getSolution()->getPrimal();
  
    std::cout << "X = " << x[0] << std::endl;
    std::cout << "Y = " << x[1] << std::endl;
    
  }

CLEANUP:
//  if (iface) {
//    delete iface;
//  }
//  delete timer;

//  delete x;
#ifdef DEBUG2
  delete x0;
//  delete x4;
//  delete y5;
#endif
  return 0;
}

void add_cut(const Double *x_, std::vector<Double>b_, Int m_)
{
  Double fval = 0.0;
  Int error = 1e-4;
  Int eps = 1e-5;
  Double *grd = new Double[vars.size()];
  LinearFunctionPtr lf_a;
  FunctionPtr fn_a;
  Double c_a;
  ConstraintPtr newcon;
  VariablePtr v;

  Double *x1_ = new Double[numVarOrig];
  Double *grd1 = new Double[numVarOrig];
  for (Int j = 0; j < numVarOrig; j++)
    x1_[j] = x_[j];

  for (Int i = 0; i < m_; i++)
  {
    fval = fn1[i]->eval(x1_, &error)-b_[i];
    if (fval > eps)
    {
      fn1[i]->evalGradient(x1_, grd, &error);
      for (Int j = 0; j < numVarOrig; j++)
        grd1[j] = grd[j];
    
      std::cout << "grd[0] = " << grd[0] << ", grd[1] = " << grd[1] << "\n";
      std::cout << "grd[2] = " << grd[2] << ", grd[3] = " << grd[3] << "\n";
      
      lf_a = (LinearFunctionPtr) new LinearFunction(grd1, inst->varsBegin(), inst->varsEnd(), 1e-2);
      v = inst->newVariable(0.0, INFINITY, Continuous, "S");
      vars.push_back(v);
      slack.push_back(v);
      lf_a->addTerm( v , 1.0);
      fn_a = (FunctionPtr) new Function(lf_a);
      c_a = InnerProduct(x1_, grd1, numVarOrig);
      cons.push_back(inst->newConstraint(fn_a, c_a, c_a, "lnrzt_cut"));
    }
  }
}

Double norm_(const Double *x_, std::vector<Double>b_, Int m_)
{
  Double l2 = 0.0;
  Double fval = 0.0;
  Int error = 1e-4;
  for (Int i = 0; i < m_; i++)
  {
    fval = fn1[i]->eval(x_,&error)-b_[i];
    std::cout << "f_" << i << " = " << fval << "\n";
    if (fval > 1e-4)
      l2 += fval*fval;
  }
  return l2;
}

FunctionPtr createLogBar()
{
//  std::vector<CNode *> cnode_; 
  CNodeVector cnode_;
  CNode *node0, *node1, *node2, *node3, *node4, *node5, *node6;

//  CNode *cnode;

  FunctionPtr fnobj;
  CGraphPtr cgobj = CGraphPtr(new CGraph);

  UInt nodenum = 0;
  UInt logterms = 0;
//  std::cout << "1st\n";
  node3 = cgobj->newNode(-0.0);
  Double lb;
  for (UInt i = 0; i < numVarOrig; i++)
  {
    lb = vars[i]->getLb();
    node1 = cgobj->newNode(vars[i]);
    if (lb < -1e-4) {
      node0 = cgobj->newNode(-1.0*lb);
      node2 = cgobj->newNode(OpPlus, node1, node0);
    } else {
      node0 = cgobj->newNode(lb);
      node2 = cgobj->newNode(OpMinus, node1, node0);
    }

    node4 = cgobj->newNode(OpLog, node2, NULL);
    if (i > 0){
      node6 = cgobj->newNode(OpMinus, node6, node4);
    } else
      node6 = cgobj->newNode(OpMinus, node3, node4);
  } 
  for (UInt i = 0; i < numVarOrig; i++)
  {
    node1 = cgobj->newNode(vars[i]);
    node0 = cgobj->newNode(vars[i]->getUb());
    node2 = cgobj->newNode(OpMinus, node0, node1);
    node4 = cgobj->newNode(OpLog, node2, NULL);
    node6 = cgobj->newNode(OpMinus, node6, node4);
  } 
//  cnode_.push_back(node6);
//  Int noden = cnode_.size();
//  std::cout << "2nd\n";
  CNode *node_1, *node_2, *node_3;
  VariablePtr v;
  if (slack.size() > 0)
  {
    for (UInt i = 0; i < slack.size(); i++)
    {
      v = slack[i];
      node_1 = cgobj->newNode(v);

      node_2 = cgobj->newNode(OpLog, node_1, NULL);
      node6 = cgobj->newNode(OpMinus, node6, node_2);
    }
  }
  cgobj->setOut(node6);
  cgobj->finalize();

  FunctionPtr fobj = (FunctionPtr) new Function(cgobj);
//  std::cout << "cgobj\n";
//  cgobj->write(std::cout);
//  std::cout << "\n";
  return fobj;
}

FunctionPtr getFunc1()
{
  CNode *n01, *n11, *n21, *n31, *n41, *n51, *n61;
  CGraphPtr cgraph1 = CGraphPtr(new CGraph);
  FunctionPtr fn1;

  n01 = cgraph1->newNode(vars[0]);
  n61 = cgraph1->newNode(2);
  n11 = cgraph1->newNode(OpSqr, n01, NULL);
  n21 = cgraph1->newNode(vars[1]);
  n31 = cgraph1->newNode(OpSqr, n21, NULL);
  n41 = cgraph1->newNode(OpPlus, n11, n31);
  n51 = cgraph1->newNode(OpLog, n41, NULL);

  cgraph1->setOut(n51);
  cgraph1->finalize();

  fn1 = (FunctionPtr) new Function(cgraph1);
  return fn1;
}

FunctionPtr getFunc2()
{
  CNode *n002, *n012, *n022, *n032, *n042, *n052, *n062, *n072, *n082, *n092, *n102, *n112, *n122, *n132, *n142, *n152, *n162, *n172, *n182;
  CGraphPtr cgraph2 = CGraphPtr(new CGraph);
  FunctionPtr fn2;

  n002 = cgraph2->newNode(vars[0]);
  n012 = cgraph2->newNode(vars[1]);
  n152 = cgraph2->newNode(2);
  n022 = cgraph2->newNode(OpPowK, n002, n152);  //X^2
  n032 = cgraph2->newNode(3.0);
  n042 = cgraph2->newNode(OpMult, n032, n012);  //3Y
  n052 = cgraph2->newNode(OpPlus, n022, n042);  //X^2 + 3Y
  n062 = cgraph2->newNode(2);
  n072 = cgraph2->newNode(3.0);
  n082 = cgraph2->newNode(vars[0]);             //X
  n092 = cgraph2->newNode(OpPowK, n082, n062);  //X^2
  n102 = cgraph2->newNode(vars[1]);             //Y
  n112 = cgraph2->newNode(OpMult, n102, n072);  //3Y
  n122 = cgraph2->newNode(OpPlus, n092, n112);  //X^2 + 3Y
  n162 = cgraph2->newNode(30.0); 
  n172 = cgraph2->newNode(OpMinus, n052, n162); //X^2 + 3Y - 30
  n132 = cgraph2->newNode(OpCos, n172, NULL);   // cos(X^2 + 3Y -30) 
   
  n182 = cgraph2->newNode(OpPlus, n132, n052);  // X^2 + 3Y + cos(X^2 + 3Y -30) 

  cgraph2->setOut(n122);
  cgraph2->finalize();

  fn2 = (FunctionPtr) new Function(cgraph2);
  return fn2;
}

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


//*/

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:

