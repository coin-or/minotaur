// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef QUADRATICFUNCTIONUT_H
#define QUADRATICFUNCTIONUT_H

#include <vector>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Eigen.h>
#include <QuadraticFunction.h>

using namespace Minotaur;

class QuadraticFunctionTest : public CppUnit::TestCase {

public:
  QuadraticFunctionTest(std::string name) : TestCase(name) {}
  QuadraticFunctionTest() {}

  void setUp();
  void tearDown();

  CPPUNIT_TEST_SUITE(QuadraticFunctionTest);
  CPPUNIT_TEST(testGetCoeffs);
  CPPUNIT_TEST(testEvaluate);
  CPPUNIT_TEST(testOperations);
  CPPUNIT_TEST(testEigen);
  CPPUNIT_TEST_SUITE_END();

  void testGetCoeffs();
  void testEvaluate();
  void testOperations();
  void testEigen();

private:
  std::vector <VariablePtr> vars_;
  QuadraticFunctionPtr q_, q1_, q2_, q3_;
};

#endif

