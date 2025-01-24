//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

#include <unistd.h>
#include "MinotaurConfig.h"
#include "Timer.h"
#include "TimerUT.h"


CPPUNIT_TEST_SUITE_REGISTRATION(TimerUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TimerUT, "TimerUT");

using namespace Minotaur;

void TimerUT::setUp()
{
  tFactory_ = new TimerFactory();     
}

void TimerUT::tearDown()
{
  delete tFactory_;
}

void TimerUT::testSleep()
{
  double time_used;
  Timer *timer=tFactory_->getTimer();

  // start timer
  timer->start();
  // and query immediately
  time_used = timer->query();
  CPPUNIT_ASSERT(time_used <= 0.001);

  // also query after 1 seconds
  while (timer->query() < 1) {
  }
  time_used = timer->query();
  CPPUNIT_ASSERT(time_used >= 1.0);
  CPPUNIT_ASSERT(time_used <= 2.0);
  timer->stop();

  // finally sleep test
  timer->start();
  usleep(2000);
  time_used = timer->query();
  CPPUNIT_ASSERT(time_used <= 1.5);
  delete timer;
}

