// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef AMPLFILTERSQPUT_H
#define AMPLFILTERSQPUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <AMPLInterface.h>
#include <Problem.h>

using namespace MINOTAUR_AMPL;

class AMPLFilterSQPUT : public CppUnit::TestCase {

public:
  AMPLFilterSQPUT(std::string name) : TestCase(name) {}
  AMPLFilterSQPUT() {}

  void testNLP();
  void testLP();
  void setUp() { }      // need not implement
  void tearDown() { }   // need not implement

  CPPUNIT_TEST_SUITE(AMPLFilterSQPUT);
  CPPUNIT_TEST(testNLP);
  CPPUNIT_TEST(testLP);
  CPPUNIT_TEST_SUITE_END();

private:
  Minotaur::EnvPtr env_;
  AMPLInterfacePtr iface_;
};

#endif

