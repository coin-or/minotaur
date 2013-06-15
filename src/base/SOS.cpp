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
#include "SOS.h"

using namespace Minotaur;

SOS::SOS() 
: n_(0),
  priority_(0),
  type_(SOS1),
  vals_(0)
{
}


SOS::SOS(Int n, SOSType type, Double *vals, const VarVector &vars, Int priority) 
{
  n_ = n;
  priority_ = priority;
  type_ = type;
  vals_ = new Double[n];
  memcpy(vals_, vals, sizeof(Double)*n);
  vars_ = vars;
}


SOS::~SOS()
{
  vars_.clear();
  delete [] vals_;
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
