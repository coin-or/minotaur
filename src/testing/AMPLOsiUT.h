// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#ifndef AMPLOSIUT_H
#define AMPLOSIUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Types.h"
#include "Environment.h"
#include "OsiLPEngine.h"
#include "AMPLInterface.h"

using namespace MINOTAUR_AMPL;

class AMPLOsiUT : public CppUnit::TestCase {

public:
  AMPLOsiUT(std::string name) : TestCase(name) {}
  AMPLOsiUT() {}

  void testOsiLP();
  void testOsiLP2();
  void testOsiWarmStart();
  void testOsiBnB();
  void setUp();
  void tearDown();

  CPPUNIT_TEST_SUITE(AMPLOsiUT);
  CPPUNIT_TEST(testOsiLP);
  CPPUNIT_TEST(testOsiLP2);
  CPPUNIT_TEST(testOsiWarmStart);
  CPPUNIT_TEST(testOsiBnB);
  CPPUNIT_TEST_SUITE_END();

private:
  Minotaur::EnvPtr env_;
  AMPLInterfacePtr iface_;
  Minotaur::OsiLPEnginePtr engine_ptr_;
};

#endif

