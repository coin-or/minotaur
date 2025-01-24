// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2024 The Minotaur Team.
// 

// /**
// \file EnvironmentUT.h
// \brief Header for testing the Environment class
// \author Ashutosh Mahajan, Argonne National Laboratory
// */

#ifndef ENVIRONMENTUT_H
#define ENVIRONMENTUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Environment.h>

using namespace Minotaur;

class EnvironmentUT : public CppUnit::TestCase {

  public:
    EnvironmentUT(std::string name) : TestCase(name) {}
    EnvironmentUT() {}

    void setUp();
    void tearDown();

    CPPUNIT_TEST_SUITE(EnvironmentUT);
    CPPUNIT_TEST(testLogger);
    CPPUNIT_TEST_SUITE_END();

    void testLogger();

  private:
    EnvPtr env_;

};

#endif

