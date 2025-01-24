// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2024 The Minotaur Team.
// 

/**
 * \file NodeProcessor.cpp
 * \brief Define two functions of NodeProcessor class. 
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#include "MinotaurConfig.h"
#include "NodeProcessor.h"

using namespace Minotaur;

void NodeProcessor::setBrancher(BrancherPtr brancher)
{
  brancher_ = brancher;
}


void NodeProcessor::processRootNode(NodePtr node, RelaxationPtr rel, 
    SolutionPoolPtr s_pool)
{
  process(node, rel, s_pool);
}


