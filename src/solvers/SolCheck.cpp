//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2021 The MINOTAUR Team.
//

/**
 * \file SolCheck.cpp
 * \brief Given a solution file and a problem file, checks whether is solution
 * is feasible to the problem. 
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "SolCheck.h"
#include "Constraint.h"
#include "Environment.h"
#include "Logger.h"
#include "Operations.h"
#include "Problem.h"
#include "Reader.h"
#include "Solution.h"
#include "Timer.h"
#include <cmath>

using namespace Minotaur;
const std::string SolCheck::me_ = "mcheck: ";

SolCheck::SolCheck(EnvPtr env) 
: objSense_(1.0),
  status_(NotStarted)
{
  env_ = env;
  iface_ = 0;
}


SolCheck::~SolCheck() 
{
}


void SolCheck::doSetup()
{
}


std::string SolCheck::getAbout()
{
  std::ostringstream ostr;
        
  ostr << me_ 
       << "Minotaur version " << env_->getVersion()
       << std::endl
       << me_ << "Check a given solution for violation of constraints and find objective value"
       << std::endl
       << me_ << "Visit https://minotaur-solver.github.io/ for details" 
       << std::endl
       << std::endl;
  return ostr.str();
}


SolveStatus SolCheck::getStatus()
{
  return NotStarted;
}


void SolCheck::showHelp() const
{ 
  env_->getLogger()->errStream()
      << std::endl
      << "Usage:" << std::endl
      << "To show version: mcheck -v (or --display_version yes) " << std::endl
      << "To show all options: mcheck -= (or --display_options yes)" << std::endl
      << "To check a point: mcheck --debug_sol SOLFILE "
      << " file.[mps|nl]"
      << std::endl;
}


int SolCheck::showInfo()
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
    env_->getLogger()->msgStream(LogNone) << getAbout();
    return 1;
  }

  env_->getLogger()->msgStream(LogInfo) << getAbout();
  return 0;
}


int SolCheck::solve(ProblemPtr p)
{
  DoubleVector *px = p->getDebugSol();
  DoubleVector x;
  Variable *v;
  Constraint *c;
  const double ATOL = 1e-6;
  const double RTOL = 1e-3;
  double act;
  int i;
  int err = 0;
  size_t vvio = 0;
  size_t cvio = 0;
  size_t ivio = 0;
  LoggerPtr log = env_->getLogger();

  if (!px) {
    log->errStream() << "Error: solution is required for checks" 
      << std::endl;
    showHelp();
    return 1;
  }

  log->msgStream(LogInfo) << std::endl << me_
    << "absolute tolerance = " << ATOL << std::endl
    << me_ << "relative tolerance = " << RTOL << std::endl << std::endl;

  x = *px;

  // check integrality 
  i = 0;
  for (VariableConstIterator vit = p->varsBegin(); vit != p->varsEnd(); 
       ++vit, ++i) {
    v = *vit;
    if (v->getType() == Integer || v->getType() == Binary) {
      if (false == isInt(x[i], 1e-6)) {
        ivio++;
        log->msgStream(LogInfo) << me_ << v->getName() << " value " << x[i] << " violates integer constraint" << std::endl;
      }
    }
  }


  // check bounds on variables
  i = 0;
  for (VariableConstIterator vit = p->varsBegin(); vit != p->varsEnd(); 
       ++vit, ++i) {
    v = *vit;
    if (v->getLb() > -INFINITY) {
      if (x[i] < v->getLb() - ATOL && x[i] < v->getLb()*(1.0-RTOL)) {
        log->msgStream(LogInfo) << me_ << v->getName() << " value " << x[i] << " violates its lower bound " << v->getLb() << std::endl;
        vvio++;
      } else {
        log->msgStream(LogDebug) << me_ << v->getName() << " lower bound OK " << std::endl;
      }
    }

    if (v->getUb() < INFINITY) {
      if (x[i] > v->getUb() + ATOL && x[i] > v->getUb()*(1.0+RTOL)) {
        log->msgStream(LogInfo) << me_ << v->getName() << " value " << x[i] << " violates its upper bound " << v->getUb() << std::endl;
        vvio++;
      } else {
        log->msgStream(LogDebug) << me_ << v->getName() << " upper bound OK " << std::endl;
      }
    }
  }

  // check constraints 
  i = 0;
  for (ConstraintConstIterator cit = p->consBegin(); cit != p->consEnd(); 
       ++cit, ++i) {
    c = *cit;
    err = 0;
    act = c->getActivity(&x[0], &err);
    if (err) {
        log->msgStream(LogInfo) << me_ << "error evaluating " <<  c->getName() << std::endl;
        continue;
    }
    if (c->getLb() > -INFINITY) {
      if (c->getLb() - act > std::max(ATOL, c->getLb()*RTOL)) {
        log->msgStream(LogInfo) << me_ << c->getName() << " value " << act << " violates its lower bound " << c->getLb() << std::endl;
        cvio++;
      } else {
        log->msgStream(LogDebug) << me_ << c->getName() << " lower bound OK " << std::endl;
      }
    }

    if (c->getUb() < INFINITY) {
      if (act - c->getUb() > std::max(ATOL, c->getUb()*RTOL)) {
        log->msgStream(LogInfo) << me_ << c->getName() << " value " << act << " violates its upper bound " << c->getUb() << std::endl;
        cvio++;
      } else {
        log->msgStream(LogDebug) << me_ << c->getName() << " upper bound OK " << std::endl;
      }
    }
  }

  log->msgStream(LogInfo) << std::endl;

  log->msgStream(LogError) << me_ << "integer violations: " << ivio 
    << std::endl << me_ << "bound violations " << vvio << std::endl
    << me_ << "constraint violations " << cvio << std::endl;
  return 0;
}

