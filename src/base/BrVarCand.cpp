// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2024 The Minotaur Team.
// 

/**
 * \file BrVarCand.cpp
 * \author Ashutosh Mahajan, IIT Bombay
 * \brief Define the classes BrVarCand for storing candidates for branching on
 * variables.
 */

#include <iostream>

#include "MinotaurConfig.h"
#include "BrVarCand.h"
#include "Variable.h"

using namespace Minotaur;


BrVarCand::BrVarCand(VariablePtr var, int i, double d, double u)
: dDist_(d),
  uDist_(u),
  var_(var)
{ 
  pCostIndex_ = i;
  score_ = 0.0;
  h_ = HandlerPtr();
  prefDir_ = UpBranch;
}


BrVarCand::~BrVarCand()
{
  //var_.reset(); need to check it.
  //h_.reset();
  h_ = 0;
}


double BrVarCand::getDDist()
{
  return dDist_;
}


std::string BrVarCand::getName() const 
{
  return var_->getName();
}


double BrVarCand::getUDist()
{
  return uDist_;
}


VariablePtr BrVarCand::getVar()
{
  return var_;
}


void BrVarCand::setDist(double ddist, double udist)
{
  dDist_ = ddist;
  uDist_ = udist;
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
