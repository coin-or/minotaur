// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 



#include <cmath>
#include <fstream>
#include <iostream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "LinearFunction.h"
#include "ProblemUT.h"
#include "ProblemSize.h"
#include "Variable.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ProblemTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(ProblemTest, "ProblemUT");

using namespace Minotaur;
using namespace std;


void 
ProblemTest::setUp()
{
  // This is instance on page 95 in Wolsey.
  //  max 4x1 - x2, 7x1 - 2 x2 <= 14, x2 <= 3, 2 x1 - 2 x2 <= 3
  env_ = new Environment();

  instance_ = new Problem(env_);
  std::vector<VariablePtr> vars;

  vars.push_back(instance_->newVariable(0.0, INFINITY, Integer));
  vars.push_back(instance_->newVariable(0.0, 3.0, Integer));
  
  // 7x1 - 2x2 <= 14
  LinearFunctionPtr lf1 = LinearFunctionPtr(new LinearFunction());
  lf1->addTerm(vars[0], 7.0);
  lf1->addTerm(vars[1], -2.0);

  FunctionPtr cf1 = FunctionPtr(new Function(lf1));
  instance_->newConstraint(cf1, -INFINITY, 14.0); 
  

  // 2x1 - 2 x2
  LinearFunctionPtr lf2 = LinearFunctionPtr(new LinearFunction());
  lf2->addTerm(vars[0], 2.0);
  lf2->addTerm(vars[1], -2.0);

  FunctionPtr cf2 = FunctionPtr(new Function(lf2)); 
  instance_->newConstraint(cf2, -INFINITY, 3.0); 

  //  max 4x1 - x2
  LinearFunctionPtr of = LinearFunctionPtr(new LinearFunction());
  of->addTerm(vars[0], 4.0);
  of->addTerm(vars[1], -1.0);

  FunctionPtr obf = FunctionPtr(new Function(of)); 
  instance_->newObjective(obf, 0.0, Maximize);      

  instance_->calculateSize(1); 
}


void ProblemTest::tearDown()
{
  delete instance_;
  delete env_;
}


void ProblemTest::testevalCon()
{

}


void ProblemTest::testConsTypes() 
{
  CPPUNIT_ASSERT(instance_->isLinear() == true); 
  CPPUNIT_ASSERT(instance_->isQP() == false); 
  CPPUNIT_ASSERT(instance_->isQuadratic() == false); 
}


void ProblemTest::testObjTypes()
{
  CPPUNIT_ASSERT(instance_->getSize()->objLinTerms == 2);
  CPPUNIT_ASSERT(instance_->getSize()->objQuadTerms == 0);
}


void ProblemTest::testVarTypes()
{
  //instance_->countVarTypes();
  CPPUNIT_ASSERT(instance_->getSize()->bins == 0);
  CPPUNIT_ASSERT(instance_->getSize()->ints == 2);
  CPPUNIT_ASSERT(instance_->getSize()->conts == 0);
  CPPUNIT_ASSERT(instance_->getSize()->objType == Linear); 

  instance_->newBinaryVariable(); 
  instance_->calculateSize(); 
  CPPUNIT_ASSERT(instance_->getSize()->bins == 1);

  instance_->setVarType(instance_->getVariable(0),Continuous); 
  instance_->calculateSize();
  CPPUNIT_ASSERT(instance_->getSize()->ints == 1);
  CPPUNIT_ASSERT(instance_->getSize()->conts == 1); 
}


void ProblemTest::testChangeBound()
{
  CPPUNIT_ASSERT(instance_->getVariable(0)->getLb() == 0.0); 
  instance_->changeBoundByInd(0, Lower, 0.5);
  instance_->changeBoundByInd(0, Upper, 0.8);  
  CPPUNIT_ASSERT(instance_->getVariable(0)->getLb() == 0.5); 
  CPPUNIT_ASSERT(instance_->getVariable(0)->getUb() == 0.8);
  instance_->changeBoundByInd(1, 0.2, 5.0); 
  CPPUNIT_ASSERT(instance_->getVariable(1)->getLb() == 0.2); 
  CPPUNIT_ASSERT(instance_->getVariable(1)->getUb() == 5.0);   
}


void ProblemTest::testDeleteVar()
{
  VariablePtr varToDelete = instance_->getVariable(0); 
  instance_->markDelete(varToDelete);
  CPPUNIT_ASSERT(instance_->isMarkedDel(varToDelete) == true);  

  instance_->delMarkedVars();
  instance_->calculateSize();  
  //CPPUNIT_ASSERT(instance_->getSize()->bins == 0);
  CPPUNIT_ASSERT(instance_->getSize()->ints == 1);  
}


void ProblemTest::testaddToObj()
{
  LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
  lf->addTerm(instance_->newVariable(0.0, 4.0, Continuous), 5.0);
  instance_->addToObj(lf); 
  delete lf;
  instance_->calculateSize(); 
  CPPUNIT_ASSERT(instance_->getSize()->objLinTerms == 3);
}

