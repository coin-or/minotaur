// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#ifndef LINEARFUNCTIONUT_H
#define LINEARFUNCTIONUT_H

#include <string>
#include <vector>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Function.h>

using namespace Minotaur;

class FunctionTest : public CppUnit::TestCase {

public:
  FunctionTest(std::string name) : TestCase(name) {}
  FunctionTest() {}

  void setUp();
  void tearDown();
  void testEval();
  void testGetFixVarOffset();
  void testEvalGradient();
  //void testEvalHessian() { CPPUNIT_ASSERT(!"implement me!"); }

  CPPUNIT_TEST_SUITE(FunctionTest);
  CPPUNIT_TEST(testBilinearRecognize);
  CPPUNIT_TEST(testEval);
  CPPUNIT_TEST(testGetFixVarOffset);
  CPPUNIT_TEST(testEvalGradient);
  //CPPUNIT_TEST(testEvalHessian); 

  //CPPUNIT_TEST(testGetCoeffs);
  //CPPUNIT_TEST(testGetObj);
  CPPUNIT_TEST_SUITE_END();

  void testBilinearRecognize();
  //void testGetRhs();
  //void testGetObj();

private:

  LinearFunctionPtr lf1_;
  QuadraticFunctionPtr qf1_;
  QuadraticFunctionPtr qf2_;
  std::vector<VariablePtr> vars_;

};

#endif

