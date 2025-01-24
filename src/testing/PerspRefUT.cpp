// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#include <cmath>

#include "MinotaurConfig.h"
#include "PerspRefUT.h"
#include "Types.h"
#include "CGraph.h"
#include "CNode.h"
#include "Variable.h"


CPPUNIT_TEST_SUITE_REGISTRATION(PerspRefUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(PerspRefUT, "PerspRefUT");

using namespace Minotaur;

void PerspRefUT::iterative()
{
  int err = 0;
  NonlinearFunctionPtr p;
  double val;
  double a[3];

  CPPUNIT_ASSERT(cg_); 
  p = cg_->getPersp(z_, 1e-5, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(p); 

  a[0] = a[1] = 0.0; a[2] = 1.0;
  val = p->eval(a, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(fabs(val) < 1e-12); 

  a[0] = a[1] = 4.0; a[2] = 1.0;
  val = p->eval(a, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(fabs(val-32.0) < 1e-12); 

  a[0] = a[1] = 4.0; a[2] = 2.0;
  val = p->eval(a, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(fabs(val-16.0) < 1e-4); 

  delete p;
}


void PerspRefUT::recursive()
{
  int err = 0;
  CGraph* p;
  double val;
  double a[3];

  CPPUNIT_ASSERT(cg_); 
  p = getPerspRec(cg_, z_, 1e-5, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(p); 

  a[0] = a[1] = 0.0; a[2] = 1.0;
  val = p->eval(a, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(fabs(val) < 1e-12); 

  a[0] = a[1] = 4.0; a[2] = 1.0;
  val = p->eval(a, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(fabs(val-32.0) < 1e-12); 

  a[0] = a[1] = 4.0; a[2] = 2.0;
  val = p->eval(a, &err);
  CPPUNIT_ASSERT(err == 0); 
  CPPUNIT_ASSERT(fabs(val-16.0) < 1e-4); 

  delete p;
}


CGraph* PerspRefUT::getPerspRec(CGraph* f, VariablePtr z, double eps, int *err) 
{
  CNode *znode = 0;
  CNode *anode = 0;
  CGraph* p = 0;

  if (f->hasVar(z)) {
    *err = 1;
    return 0;
  }

  p = new CGraph();
  // create a node (z(1-eps)+eps)
  znode = p->newNode(z);
  if (eps>0.0) {
    anode = p->newNode(1.0-eps);
    znode = p->newNode(OpMult, anode, znode); // z*(1-eps)
    anode = p->newNode(eps);
    znode = p->newNode(OpPlus, anode, znode); // eps + z*(1-eps)
  } 

  // recurse
  anode = getPerspRec_(p, f->getOut(), znode, f);

  // final multiplication
  anode = p->newNode(OpMult, anode, znode);
  p->setOut(anode);
  p->finalize();

  return p;
}


CNode* PerspRefUT::getPerspRec_(CGraph* p, const CNode *node, CNode *znode, CGraph* f)
{
  CNode *newl  = 0;
  CNode *newr = 0;
  if (OpVar == node->getOp()) {
    newl = p->newNode(f->getVar(node));
    return (p->newNode(OpDiv, newl, znode));
  } else if (OpNum == node->getOp()) {
    return (p->newNode(node->getVal()));
  } else if (1 == node->numChild()) {
    newl = getPerspRec_(p, node->getL(), znode, f);
    return (p->newNode(node->getOp(), newl, newl));
  } else if (2 == node->numChild()) {
    newl = getPerspRec_(p, node->getL(), znode, f);
    newr = getPerspRec_(p, node->getR(), znode, f);
    return (p->newNode(node->getOp(), newl, newr));
  } else if (2 < node->numChild()) {
    CNode **childr = new CNode*[node->numChild()];
    CNode** c1 = node->getListL();
    CNode** c2 = node->getListR();
    int i = 0;
    while(c1 < c2) {
      childr[i] = getPerspRec_(p, *c1, znode, f);
    }
    return (p->newNode(node->getOp(), childr, node->numChild()));
  }
  return 0;
}


void PerspRefUT::setUp()
{
  cg_ = new CGraph();
  x_ = new Variable(0, 0, 0.0, 10.0, Continuous, "x");
  y_ = new Variable(1, 1, 0.0, 10.0, Continuous, "y");

  CNode *n0, *n1;
  n0 = cg_->newNode(x_);
  n0 = cg_->newNode(OpSqr, n0, 0);

  n1 = cg_->newNode(y_);
  n1 = cg_->newNode(OpSqr, n1, 0);

  n0 = cg_->newNode(OpPlus, n0, n1);

  cg_->setOut(n0);
  cg_->finalize();

  z_ = (VariablePtr) new Variable(2, 2, 0.0, 1.0, Continuous, "z");
}


void PerspRefUT::tearDown()
{
  delete cg_;
  delete x_;
  delete y_;
  delete z_;
}


