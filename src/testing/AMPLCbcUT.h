// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef AMPLCBCUT_H
#define AMPLCBCUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Types.h"
#include "CbcEngine.h"
#include "AMPLInterface.h"

using namespace MINOTAUR_AMPL;

class AMPLCbcUT : public CppUnit::TestCase {

public:
  AMPLCbcUT(std::string name) : TestCase(name) {}
  AMPLCbcUT() {}

  void testCbc();
  void setUp();
  void tearDown();

  CPPUNIT_TEST_SUITE(AMPLCbcUT);
  CPPUNIT_TEST(testCbc);
  CPPUNIT_TEST_SUITE_END();

private:
  Minotaur::EnvPtr env_;
  Minotaur::CbcEnginePtr e_;
  AMPLInterfacePtr iface_;
};

#endif

