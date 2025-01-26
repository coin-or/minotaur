// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef AMPLBQPDUT_H
#define AMPLBQPDUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <AMPLInterface.h>
#include <Problem.h>

using namespace MINOTAUR_AMPL;

class AMPLBqpdUT : public CppUnit::TestCase {

public:
  AMPLBqpdUT(std::string name) : TestCase(name) {}
  AMPLBqpdUT() {}

  void testNLP();
  void setUp() { }      // need not implement
  void tearDown() { }   // need not implement

  CPPUNIT_TEST_SUITE(AMPLBqpdUT);
  CPPUNIT_TEST(testNLP);
  CPPUNIT_TEST_SUITE_END();

private:
  AMPLInterfacePtr iface_;
};

#endif

