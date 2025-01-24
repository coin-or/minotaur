//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

//
// Brancher class finds suitable candidates for branching. e.g. integer
// variable, 'non convex' variable.
//

#include "MinotaurConfig.h"
#include "Brancher.h"
#include "Node.h"

using namespace Minotaur;


Brancher::Brancher()
  : logger_(LoggerPtr()) // NULL
{
}


Brancher::~Brancher()
{
}


void Brancher::updateAfterSolve(NodePtr , ConstSolutionPtr )
{
}


