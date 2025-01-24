// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#ifndef LAPACKUT_H
#define LAPACKUT_H

#include <vector>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Types.h"

using namespace Minotaur;

class LapackTest : public CppUnit::TestCase {

  public:
    LapackTest(std::string name) : TestCase(name) {}
    LapackTest() {}

    void setUp() {};
    void tearDown() {};

    CPPUNIT_TEST_SUITE(LapackTest);
    CPPUNIT_TEST(testWrapper);
    CPPUNIT_TEST(testEigenValues);
    CPPUNIT_TEST(testEigenValues2);
    CPPUNIT_TEST_SUITE_END();

    void testWrapper();
    void testEigenValues();
    void testEigenValues2();

  private:

};

#endif

