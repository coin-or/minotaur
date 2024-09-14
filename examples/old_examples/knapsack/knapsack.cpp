#include <cmath>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "BranchAndBound.h"
#include "CGraph.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "Function.h"
#include "Handler.h"
#include "IntVarHandler.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "PCBProcessor.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Problem.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "TreeManager.h"

using namespace Minotaur;

extern "C" {
  void solveknap_(int *N, int *f, double *a, double *b, double *sol, 
		  double *objval);
}


BranchAndBound * createBab (EnvPtr env, ProblemPtr p, EnginePtr e,
			    HandlerVector &handlers)
{
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env,p);
  LinearHandlerPtr l_hand = (LinearHandlerPtr) new LinearHandler(env,p);
  ReliabilityBrancherPtr rel_br;
  PCBProcessorPtr nproc;
  NodeIncRelaxerPtr nr;
  RelaxationPtr rel;
  BranchAndBound *bab = new BranchAndBound(env,p);

  handlers.push_back(v_hand);
  handlers.push_back(l_hand);
  nproc = (PCBProcessorPtr) new PCBProcessor(env,e,handlers);

  rel_br = (ReliabilityBrancherPtr) new ReliabilityBrancher(env,handlers);
  rel_br->setEngine(e);
  rel_br->setThresh(4);
  rel_br->setMaxDepth(10);
  rel_br->setIterLim(5);
  nproc->setBrancher(rel_br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env,handlers);
  rel = (RelaxationPtr) new Relaxation(p,env);
  rel->setNativeDer();
  nr->setRelaxation(rel);
  nr->setEngine(e);
  nr->setModFlag(false);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);
  
  return bab;
}


ProblemPtr createProblem (UInt N, UInt f, double *a, double *b)
{
  VarVector vars;
  EnvPtr env = (EnvPtr) new Environment();
  ProblemPtr p = (ProblemPtr) new Problem(env);
  std::string s;
  LinearFunctionPtr lf;
  FunctionPtr fun;
  ObjectivePtr o;
  CGraphPtr cg;
  CNode *nodes[f];
  CNode *pnode;
  CNode *cnode;
  CNode *vnode;

  // create variables
  for (UInt i = 0; i < f; ++i) {
    vars.push_back(p->newVariable(1.0, (double) N, Integer));
  }
  
  // add constraint
  lf = (LinearFunctionPtr) new LinearFunction();
  for (UInt i=0; i<f; ++i) {
    lf->addTerm(vars[i], 1.0);
  }
  fun = (FunctionPtr) new Function(lf);
  p->newConstraint(fun, -INFINITY, N);

  // objective
  cg = (CGraphPtr) new CGraph();
  for (UInt i=0; i<f; ++i) {
    vnode = cg->newNode(vars[i]);
    pnode = cg->newNode(b[i]);
    cnode = cg->newNode(a[i]);
    vnode = cg->newNode(OpPowK, vnode, pnode);
    vnode = cg->newNode(OpMult, cnode, vnode);
    nodes[i] = vnode;
  }
  vnode = cg->newNode(OpSumList, nodes, f);
  cg->setOut(vnode);
  cg->finalize();

  fun = (FunctionPtr) new Function(cg);
  p->newObjective(fun, 0.0, Minimize);

  p->setNativeDer();
  return p;
}


void solve(ProblemPtr p, double *sol, double *objval)
{
  EnvPtr env = (EnvPtr) new Environment();
  FilterSQPEnginePtr e = (FilterSQPEnginePtr) new FilterSQPEngine (env);
  BranchAndBound * bab = 0;
  HandlerVector handlers;
  int err = 0;

  //env->startTimer(err); assert(err==0);
  // get branch-and-bound
  bab = createBab(env, p, e, handlers);
  bab->solve();
  *objval = bab->getUb();
  memcpy(sol, bab->getSolution()->getPrimal(), p->getNumVars()*sizeof(double));
  //env->stopTimer(err); assert(err==0);
  delete bab;
}


void solveknap_(int *N, int *f, double *a, double *b, double *sol,
		double *objval)
{
  ProblemPtr p = createProblem(*N, *f, a, b);
  solve(p, sol, objval);
  //p->clear();
}
