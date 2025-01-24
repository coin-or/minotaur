// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#include <cmath>
#include "AMPLCbcUT.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "Function.h"
#include "LinearFunction.h"
#include "Option.h"
#include "Problem.h"

#include "AMPLInterface.h"


CPPUNIT_TEST_SUITE_REGISTRATION(AMPLCbcUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AMPLCbcUT, "AMPLCbcUT");

using namespace Minotaur;


void AMPLCbcUT::setUp()
{
  env_ = (EnvPtr) new Environment();
  env_->setLogLevel(LogNone);
  iface_ = (MINOTAUR_AMPL::AMPLInterfacePtr) new AMPLInterface(env_);
  e_ = (CbcEnginePtr) new CbcEngine(env_);
}


void AMPLCbcUT::tearDown()
{
  delete e_;
  delete iface_;
  delete env_;
}


void AMPLCbcUT::testCbc()
{
  char file_name[] = "instances/milp";
  ProblemPtr p = iface_->readInstance(file_name);
  FunctionPtr f = FunctionPtr(); // NULL
  EngineStatus status;

  e_->load(p);
  status =  e_->solve();
  CPPUNIT_ASSERT(e_->getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(fabs(e_->getSolutionValue()-1.00000) < 1e-5);

  p->changeObj(f, 2.0);
  status = e_->solve();
  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(e_->getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(fabs(e_->getSolutionValue()-3.0) < 1e-5);

  p->negateObj();
  status = e_->solve();
  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(e_->getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(fabs(e_->getSolutionValue()+1.0) < 1e-5);

  delete p;

}


