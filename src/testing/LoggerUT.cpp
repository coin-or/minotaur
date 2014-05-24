// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
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
  LoggerPtr lPtr = (LoggerPtr) new Logger(LogInfo);
  lPtr->MsgStream(LogDebug) << ":\n error in logger MsgStream! debug messages "
    << "should not appear at info level.\n";

  // should appear in output
  lPtr = (LoggerPtr) new Logger(LogDebug);
  lPtr->MsgStream(LogError) << ": logger ok! ";
}

void LoggerUT::testErrStream()
{
  // should appear in error
  LoggerPtr lPtr = (LoggerPtr) new Logger(LogError);
  lPtr->ErrStream() << ": logger ok! ";

  // should not appear in error
  lPtr = (LoggerPtr) new Logger(LogNone);
  lPtr->ErrStream() << ":\n error in logger ErrStream! error messages "
    " should not appear at LogNone level! \n";
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
