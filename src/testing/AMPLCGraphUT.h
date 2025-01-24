// 
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2024 The Minotaur Team.
//

#ifndef AMPLCGRAPHUT_H
#define AMPLCGRAPHUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <AMPLInterface.h>
#include <Problem.h>

using namespace MINOTAUR_AMPL;


// read instance using ampl and test:
// Number of variables and their types,
// Number of constraints and their types,
// Function evaluations,
// Gradient evaluations,
// Hessian evaluations.

class AMPLCGraphUT : public CppUnit::TestCase {

public:
  AMPLCGraphUT(std::string name) : TestCase(name) {}
  AMPLCGraphUT() {}

  void setUp();         
  void tearDown();   // need not implement

  void testSize();
  void testVariables();
  void testConstraints();
  void testObjective();
  void testHessian();
  void testJacobian();
  void testAllFuns();
  void testObjectiveGradient();
  void testNl();
  

  CPPUNIT_TEST_SUITE(AMPLCGraphUT);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testVariables);
  CPPUNIT_TEST(testConstraints);
  CPPUNIT_TEST(testHessian);
  CPPUNIT_TEST(testJacobian);
  CPPUNIT_TEST(testObjective);
  CPPUNIT_TEST(testObjectiveGradient);
  CPPUNIT_TEST(testAllFuns);
  CPPUNIT_TEST(testNl);

  CPPUNIT_TEST_SUITE_END();

};

#endif

