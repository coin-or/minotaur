//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file SolChecker.cpp
 * \brief Given a solution file and a problem file, checks whether is solution
 * is feasible to the problem. 
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "SolChecker.h"
#include "Constraint.h"
#include "Environment.h"
#include "Logger.h"
#include "Problem.h"
#include "Reader.h"
#include "Solution.h"
#include "Timer.h"

using namespace Minotaur;
const std::string SolChecker::me_ = "mcheck: ";

SolChecker::SolChecker(EnvPtr env) 
: objSense_(1.0),
  status_(NotStarted)
{
  env_ = env;
  iface_ = 0;
}


SolChecker::~SolChecker() 
{
}


void SolChecker::doSetup()
{
}


SolveStatus SolChecker::getStatus()
{
  return NotStarted;
}


void SolChecker::showHelp() const
{ 
  env_->getLogger()->errStream()
      << std::endl
      << "Check whether a given solution satisfies all constraints of a given problem" << std::endl
      << "Usage:" << std::endl
      << "To show version: mcheck -v (or --display_version yes) " << std::endl
      << "To show all options: mcheck -= (or --display_options yes)" << std::endl
      << "To solve an instance: mcheck --option1 [value] "
      << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int SolChecker::showInfo()
{
  OptionDBPtr options = env_->getOptions();

  if (options->findBool("display_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    return 1;
  }

  if (options->findBool("display_help")->getValue() ||
      options->findFlag("?")->getValue()) {
    showHelp();
    return 1;
  }

  if (options->findBool("display_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    env_->getLogger()->msgStream(LogNone) << me_ <<
      "Minotaur version " << env_->getVersion() << std::endl;
    env_->getLogger()->msgStream(LogNone) << me_ 
      << "NLP based Branch-and-bound algorithm for convex MINLP" << std::endl;
    env_->getLogger()->msgStream(LogNone) << me_ 
      << "Visit https://minotaur-solver.github.io/ for details" << std::endl;
    return 1;
  }

  env_->getLogger()->msgStream(LogInfo)
    << me_ << "Minotaur version " << env_->getVersion() << std::endl
    << me_ << "NLP based Branch-and-bound algorithm for convex MINLP" << std::endl
    << me_ << "Visit https://minotaur-solver.github.io/ for details" << std::endl;
  return 0;
}


int SolChecker::solve(ProblemPtr p)
{
  env_->getLogger()->msgStream(LogNone) << "Not implemented yet" << std::endl;
  return 0;
}

