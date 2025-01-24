// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file BrCand.cpp
 * \author Ashutosh Mahajan, Argonne National Laboratory.
 * \brief Define the classes BranchCand for storing candidates for branching.
 */

#include <iostream>

#include "MinotaurConfig.h"
#include "BrCand.h"
#include "Variable.h"

using namespace Minotaur;


bool Minotaur::CompareScore(BrCandPtr c1, BrCandPtr c2)
{
  if (c1->getScore() > c2->getScore()) {
    return false;
  } else if (c1->getScore() < c2->getScore()) {
    return true;
  } else if (c1->getPCostIndex() < c2->getPCostIndex()) {
    return true;
  } else if (c1->getPCostIndex() > c2->getPCostIndex()) {
    return false;
  } else {
    //std::cout << "problem here: " << c1->getPCostIndex() << " " <<
    // c2->getPCostIndex() << std::endl;
    return false;
  }
}

BrCand::BrCand()
 : branches_(0)
{
}

void BrCand::setDist(double, double )
{
}


