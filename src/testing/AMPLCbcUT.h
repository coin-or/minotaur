// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2021 The MINOTAUR Team.
// 

#ifndef AMPLCBCUT_H
#define AMPLCBCUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Types.h"
#include "CbcEngine.h"
#include "AMPLInterface.h"

using namespace MINOTAUR_AMPL;

class AMPLCbcUT : public CppUnit::TestCase {

public:
  AMPLCbcUT(std::string name) : TestCase(name) {}
  AMPLCbcUT() {}

  void testCbc();
  void setUp();
  void tearDown();

  CPPUNIT_TEST_SUITE(AMPLCbcUT);
  CPPUNIT_TEST(testCbc);
  CPPUNIT_TEST_SUITE_END();

private:
  Minotaur::EnvPtr env_;
  Minotaur::CbcEnginePtr e_;
  AMPLInterfacePtr iface_;
};

#endif

// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
