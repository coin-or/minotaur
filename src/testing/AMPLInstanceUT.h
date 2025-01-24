/* 
 *     Minotaur -- It's only 1/2 bull
 *
 *     (C)opyright 2009 - 2024 The Minotaur Team.
 */

#ifndef AMPLINSTANCEUT_H
#define AMPLINSTANCEUT_H

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

class AMPLInstanceUT : public CppUnit::TestCase {

public:
  AMPLInstanceUT(std::string name) : TestCase(name) {}
  AMPLInstanceUT() {}

  void setUp();         
  void tearDown();   // need not implement

  void testSize();
  void testVariables();
  void testConstraints();
  void testObjective();
  void testJacobian();
  void testObjectiveGradient();
  

  CPPUNIT_TEST_SUITE(AMPLInstanceUT);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testVariables);
  CPPUNIT_TEST(testConstraints);
  CPPUNIT_TEST(testJacobian);
  CPPUNIT_TEST(testObjective);
  CPPUNIT_TEST(testObjectiveGradient);

  CPPUNIT_TEST_SUITE_END();

private:
  Minotaur::EnvPtr env_;
  AMPLInterfacePtr iface_;
  Minotaur::ProblemPtr inst_;
};

#endif

