/* 
 *     Minotaur -- It's only 1/2 bull
 *
 *     (C)opyright 2009 - 2024 The Minotaur Team.
 */

#ifndef LOGGERUT_H
#define LOGGERUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Types.h"

using namespace Minotaur;

class LoggerUT : public CppUnit::TestFixture {

public:

  void setUp() { }    // need not implement
  void tearDown() { } // need not implement

  CPPUNIT_TEST_SUITE(LoggerUT);
  CPPUNIT_TEST(testMsgStream);
  CPPUNIT_TEST(testErrStream);
  CPPUNIT_TEST_SUITE_END();

  void testMsgStream();
  void testErrStream();

};

#endif

