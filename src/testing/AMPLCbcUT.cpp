// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
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
  EnvPtr env = (EnvPtr) new Environment();
  env->setLogLevel(LogNone);
  iface_ = (MINOTAUR_AMPL::AMPLInterfacePtr) new AMPLInterface(env);
  e_ = (CbcEnginePtr) new CbcEngine(env);
}


void AMPLCbcUT::tearDown()
{
  e_.reset();
  delete iface_;
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
