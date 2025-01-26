// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef PERSPREFUT_H
#define PERSPREFUT_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "CGraph.h"
using namespace Minotaur;

class PerspRefUT : public CppUnit::TestCase {

public:
  PerspRefUT(std::string name) : TestCase(name) {}
  PerspRefUT() {}

  void iterative();
  void recursive();
  CGraph* getPerspRec(CGraph* cg, VariablePtr z, double eps, int *err);
  CNode* getPerspRec_(CGraph* nlf, const CNode *node, CNode *znode, CGraph* f);
  void setUp();      // need not implement
  void tearDown();   // need not implement

  CPPUNIT_TEST_SUITE(PerspRefUT);
  CPPUNIT_TEST(iterative);
  CPPUNIT_TEST(recursive);
  CPPUNIT_TEST_SUITE_END();

private:
  CGraph* cg_;
  VariablePtr x_;
  VariablePtr y_;
  VariablePtr z_;

};

#endif

