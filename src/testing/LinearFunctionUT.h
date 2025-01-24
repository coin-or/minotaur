// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#ifndef LINEARFUNCTIONUT_H
#define LINEARFUNCTIONUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Problem.h>
#include <LinearFunction.h>

using namespace Minotaur;

class LinearFunctionTest : public CppUnit::TestCase {

public:
  LinearFunctionTest(std::string name) : TestCase(name) {}
  LinearFunctionTest() {}

  void setUp();
  void tearDown();

  CPPUNIT_TEST_SUITE(LinearFunctionTest);
  CPPUNIT_TEST(testGetCoeffs);
  CPPUNIT_TEST(testGetObj);
  CPPUNIT_TEST(testOperations);
  CPPUNIT_TEST(testFix);
  CPPUNIT_TEST_SUITE_END();

  void testGetCoeffs();
  void testGetObj();
  void testOperations();
  void testFix();

private:
  EnvPtr env_;
  ProblemPtr instance_;
};

#endif

