// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#include "MinotaurConfig.h"
#include "AMPLFilterSQPUT.h"
#include "AMPLHessian.h"
#include "AMPLJacobian.h"
#include "BranchAndBound.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "Option.h"

CPPUNIT_TEST_SUITE_REGISTRATION(AMPLFilterSQPUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AMPLFilterSQPUT, "AMPLFilterSQPUT");

using namespace MINOTAUR_AMPL;

void AMPLFilterSQPUT::testNLP()
{
  Minotaur::ProblemPtr inst;
  env_ = (Minotaur::EnvPtr) new Minotaur::Environment();
  env_->setLogLevel(Minotaur::LogNone);
  iface_ = (AMPLInterfacePtr) new AMPLInterface(env_);

  // read an instance
  inst = iface_->readInstance("instances/3pk");

  // set starting point if any
  inst->setInitialPoint(iface_->getInitialPoint());

  inst->calculateSize();

  if (inst->isQuadratic() || inst->isLinear()) { 
    inst->setNativeDer();
  } else {
    // create the jacobian
    AMPLJacobianPtr jPtr = (AMPLJacobianPtr) new AMPLJacobian(iface_);
    inst->setJacobian(jPtr);

    // create the hessian
    AMPLHessianPtr hPtr = (AMPLHessianPtr) new AMPLHessian(iface_);
    inst->setHessian(hPtr);
  }

  //create a new engine
  Minotaur::FilterSQPEnginePtr filter_e = (Minotaur::FilterSQPEnginePtr) 
    new Minotaur::FilterSQPEngine(env_);

  //load the problem
  filter_e->load(inst);

  //solve
  filter_e->solve();

  // get status
  Minotaur::EngineStatus status = filter_e->getStatus();
  CPPUNIT_ASSERT(status==Minotaur::ProvenLocalOptimal);

  // get objvalue
  double value = filter_e->getSolutionValue();
  CPPUNIT_ASSERT(fabs(value-1.7201185) < 1e-7);

  delete filter_e;
  delete inst;
  delete iface_;
  delete env_;
}


void AMPLFilterSQPUT::testLP()
{
  Minotaur::FilterSQPEnginePtr filter_e;
  Minotaur::ProblemPtr inst;
  Minotaur::EngineStatus status;
  double value;

  env_ = new Minotaur::Environment();
  env_->setLogLevel(Minotaur::LogNone);
  iface_ = new AMPLInterface(env_);
  // read an instance
  filter_e = new Minotaur::FilterSQPEngine(env_);
  inst = iface_->readInstance("instances/lp0");

  inst->negateObj();
  inst->setNativeDer();
  filter_e->load(inst);
  filter_e->solve();
  status = filter_e->getStatus();
  CPPUNIT_ASSERT(status==Minotaur::ProvenLocalOptimal);
  value = filter_e->getSolutionValue();
  CPPUNIT_ASSERT(fabs(value+8.4286) < 1e-4);

  delete filter_e;
  delete iface_;
  delete inst;
  delete env_;
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
