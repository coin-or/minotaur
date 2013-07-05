//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2013 The MINOTAUR Team.
//

/**
 * \file SOS.cpp
 * \brief SOS class stores data regarding SOS1 and SOS2 constraints.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string.h> // for memcpy

#include "MinotaurConfig.h"
#include "Operations.h"
#include "SOS.h"

using namespace Minotaur;

SOS::SOS() 
: n_(0),
  priority_(0),
  type_(SOS1),
  weights_(0)
{
}


SOS::SOS(Int n, SOSType type, const Double *weights, const VarVector &vars,
         Int priority, Int id, std::string name) 
: id_(id),
  n_(n),
  priority_(priority),
  type_(type),
  name_(name)
{
  weights_ = new Double[n];
  memcpy(weights_, weights, sizeof(Double)*n);
  vars_ = vars;
  sort(vars_, weights_, -1);
}


SOS::~SOS()
{
  vars_.clear();
  delete [] weights_;
}


Int SOS::getId() const
{
  return id_;
}


Int SOS::getNz()
{
  return n_;
}


Int SOS::getPriority() const
{
  return priority_;
}


SOSType SOS::getType()
{
  return type_;
}


std::string SOS::getName() const
{
  return name_;
}



const Double* SOS::getWeights()
{
  return weights_;
}


VariableConstIterator SOS::varsBegin() const
{
  return vars_.begin();
}


VariableConstIterator SOS::varsEnd() const
{
  return vars_.end();
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
