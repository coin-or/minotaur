// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#include <iostream>
#include <string>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>

#include "MinotaurConfig.h"

int
main (int ac, char** av)
{
  short int rValue;
  if (ac < 2) {
    std::cout << "Please specify the test to be run, or \"all\"" << std::endl;
    std::cout << "See CPPUNIT_TEST_SUITE_NAMED_REGISTRATION in source to see names of tests" 
              << std::endl;    
    return 0;
  }

  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestSuite *suite = new CppUnit::TestSuite();
  CppUnit::TestResult *results = new CppUnit::TestResult();
  CppUnit::TestResultCollector *rCollector = new CppUnit::TestResultCollector();
  CppUnit::BriefTestProgressListener *progress = 
    new CppUnit::BriefTestProgressListener();

  results->addListener(rCollector);
  results->addListener(progress);
  for (int i = 1; i < ac; ++i) {
    if (std::string(av[i]) == "all") {
      suite->addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
    } else {
      suite->addTest(CppUnit::TestFactoryRegistry::
          getRegistry(av[i]).makeTest());
    }
  }

  runner.addTest(suite);
  runner.run(*results);

  CppUnit::CompilerOutputter cOut(rCollector, std::cerr);
  cOut.write ();

  // return 0 if tests were successful
  rValue = rCollector->wasSuccessful () ? 0 : 1;

  delete progress;
  delete rCollector;
  delete results;

  return rValue;
}

