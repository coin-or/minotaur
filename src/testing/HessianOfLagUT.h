// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

// /**
// \file HessianOfLagUT.h
// \brief Tests for methods in class HessianOfLag
// \author Ashutosh Mahajan, Argonne National Laboratory
// */

#ifndef HESSIANOFLAGUT_H
#define HESSIANOFLAGUT_H

#include <string>
#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HessianOfLag.h"
#include "Problem.h"

using namespace Minotaur;

class HessianOfLagUT : public CppUnit::TestCase {

  public:
    HessianOfLagUT(std::string name) : TestCase(name) {}
    HessianOfLagUT() {}

    void setUp();
    void tearDown();

    CPPUNIT_TEST_SUITE(HessianOfLagUT);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testLinearEval);
    CPPUNIT_TEST(testQuadEval);
    CPPUNIT_TEST_SUITE_END();

    void testEmpty();
    void testLinearEval();
    void testQuadEval();

  private:
    EnvPtr env_;
    LinearFunctionPtr lf_;
    QuadraticFunctionPtr qf_;
    ProblemPtr instance_;
    std::vector<ConstraintPtr> cons_;
    std::vector<VariablePtr> vars_;
    FunctionPtr f_;
};

#endif

