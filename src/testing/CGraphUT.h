// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#ifndef CGRAPHUT_H
#define CGRAPHUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Types.h>
using namespace Minotaur;

class CGraphUT : public CppUnit::TestCase {

public:
  CGraphUT(std::string name) : TestCase(name) {}
  CGraphUT() {}

  void setUp() { }      // need not implement
  void tearDown() { }   // need not implement
  void testIdentical();
  void testLin();
  void testQuad();

  CPPUNIT_TEST_SUITE(CGraphUT);
  CPPUNIT_TEST(testIdentical);
  CPPUNIT_TEST(testLin);
  CPPUNIT_TEST(testQuad);
  CPPUNIT_TEST_SUITE_END();

};

#endif

