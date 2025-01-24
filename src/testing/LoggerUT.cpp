// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 


#include "MinotaurConfig.h"
#include "LoggerUT.h"
#include "Logger.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(LoggerUT, "LoggerUT");

using namespace Minotaur;

void LoggerUT::testMsgStream()
{
  // should not appear in output
  LoggerPtr l = (LoggerPtr) new Logger(LogInfo);
  l->msgStream(LogDebug) << ":\n error in logger msgStream! debug messages "
    << "should not appear at info level.\n";
  delete l;

  // should appear in output
  l = (LoggerPtr) new Logger(LogDebug);
  l->msgStream(LogError) << ": logger ok! ";

  delete l;
}

void LoggerUT::testErrStream()
{
  // should appear in error
  LoggerPtr l = (LoggerPtr) new Logger(LogError);
  l->errStream() << ": logger ok! ";
  delete l;

  // should not appear in error
  l = (LoggerPtr) new Logger(LogNone);
  l->errStream() << ":\n error in logger errStream! error messages "
    " should not appear at LogNone level! \n";
  delete l;
}

