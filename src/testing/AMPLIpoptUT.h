// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

#ifndef AMPLIPOPTUT_H
#define AMPLIPOPTUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include <AMPLInterface.h>
#include <Problem.h>

using namespace MINOTAUR_AMPL;

class AMPLIpoptUT : public CppUnit::TestCase {

public:
  AMPLIpoptUT(std::string name) : TestCase(name) {}
  AMPLIpoptUT() {}

  void testNLP();
  void setUp() { }      // need not implement
  void tearDown() { }   // need not implement

  CPPUNIT_TEST_SUITE(AMPLIpoptUT);
  CPPUNIT_TEST(testNLP);
  CPPUNIT_TEST_SUITE_END();

private:
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
