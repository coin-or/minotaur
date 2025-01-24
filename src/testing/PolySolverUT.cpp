// 
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2024 The Minotaur Team.
//

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Option.h"
#include "PolySolverUT.h"
#include "Variable.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PolySolverUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(PolySolverUT, "PolySolverUT");
using namespace MINOTAUR_AMPL;
using namespace std;

// #
// # This is the simple "Luedtke Generalized Pooling Problem"
// #  from our MINLP paper
// #

// var x1 >= -1, <= 4;
// var x2 >= -2.4, <= 4;
// var x3 >= -0.5, <= 6;
// var x4 >= -0.5, <= 6;
// var x5 >= 1, <= 6;
// var x6 >= 1, <= 5;
// var x7 >= 1, <= 3;
// var x8 >= 1, <= 3;

// minimize Obj: 4*(x1+x2+x3) + 3*(x4+x5) + 3.5*(x6+x7) + 2.5*x8 ;

// subject to
//   c1: x1*x2*x3*x4 - x1*x2 - x4*x5 + x5 + x6 >= 230 ;

//   c2: x3*x4*x5*x6 - x1*x4 - x6*x7 + x2 + x8 = -2 ;


void PolySolverUT::setUp()
{
  env_ = new Minotaur::Environment();
  env_->getOptions()->findBool("expand_poly")->setValue(true);
  env_->setLogLevel(Minotaur::LogNone);
  iface_ = new AMPLInterface(env_);
  inst_ = iface_->readInstance("instances/luedtke-1");
}


void PolySolverUT::tearDown()
{
  delete inst_;
  delete iface_;
  delete env_;
}


void PolySolverUT::testSize()
{
  CPPUNIT_ASSERT(inst_->getNumCons() == 2);
  CPPUNIT_ASSERT(inst_->getNumVars() == 8);
}


