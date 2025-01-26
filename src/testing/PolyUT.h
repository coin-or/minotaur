// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef POLYUT_H
#define POLYUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

class PolyUT : public CppUnit::TestCase {

public:
  PolyUT(std::string name) : TestCase(name) {}
  PolyUT() {}

  void monomial();
  void polynomial();
  void setUp() { }      // need not implement
  void tearDown() { }   // need not implement

  CPPUNIT_TEST_SUITE(PolyUT);
  CPPUNIT_TEST(monomial);
  CPPUNIT_TEST(polynomial);
  CPPUNIT_TEST_SUITE_END();

};

#endif

