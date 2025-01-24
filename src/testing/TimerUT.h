//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

#ifndef TIMERUT_H
#define TIMERUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace Minotaur;

class TimerUT : public CppUnit::TestCase {
  public:
    TimerUT(std::string name) : TestCase(name) {}
    TimerUT() {}

    void testSleep();
    void setUp();
    void tearDown();

    CPPUNIT_TEST_SUITE(TimerUT);
    CPPUNIT_TEST(testSleep);
    CPPUNIT_TEST_SUITE_END();

  private:
    TimerFactory *tFactory_;
};

#endif     // #define TIMERUT_H

