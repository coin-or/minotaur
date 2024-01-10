// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
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
