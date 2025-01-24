// 
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2024 The Minotaur Team.
//

#ifndef TRANSFORMERUT_H
#define TRANSFORMERUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <AMPLInterface.h>
#include <Problem.h>
#include <TransPoly.h>

//using namespace MINOTAUR_AMPL;


// read instance using ampl and test:
// Number of variables and their types,
// Number of constraints and their types,
// Function evaluations,
// Gradient evaluations,
// Hessian evaluations.

class TransformerUT : public CppUnit::TestCase {

public:
  friend class Minotaur::Transformer;

  TransformerUT(std::string name) : TestCase(name) {}
  TransformerUT() {}

  void setUp();         
  void tearDown();   // need not implement

  void testSize();
  void testMakeObjectiveLinear();
  

  CPPUNIT_TEST_SUITE(TransformerUT);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testMakeObjectiveLinear);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST_SUITE_END();

private:
  Minotaur::EnvPtr env_;
  MINOTAUR_AMPL::AMPLInterfacePtr iface_;
  Minotaur::ProblemPtr inst_;
};

#endif

