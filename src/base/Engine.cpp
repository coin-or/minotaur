// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 


#include "MinotaurConfig.h"
#include "Engine.h"
#include "Logger.h"

using namespace Minotaur;

Engine::Engine()
  : logger_(LoggerPtr()) // NULL
{

}


Engine::~Engine()
{
}


std::string Engine::getStatusString()
{
  switch (status_) {
   case(ProvenOptimal):
     return "ProvenOptimal";
   case(ProvenLocalOptimal):
     return "ProvenLocalOptimal";
   case(ProvenInfeasible):
     return "ProvenInfeasible";
   case(ProvenLocalInfeasible):
     return "ProvenLocalInfeasible ";
   case(ProvenUnbounded):
     return "ProvenUnbounded";
   case(ProvenObjectiveCutOff):
     return "ProvenObjectiveCutOff";
   case(EngineIterationLimit):
     return "EngineIterationLimit";
   case(ProvenFailedCQFeas):
    return "ProvenFailedCQFeas";
   case(ProvenFailedCQInfeas):
    return "ProvenFailedCQInfeas";
   case(FailedFeas):
    return "FailedFeas";
   case(FailedInfeas):
    return "FailedInfeas";
   case(EngineError):
     return "EngineError";
   case(EngineUnknownStatus):
     return "EngineUnknownStatus";
   default:
     return "Engine status unknown type";
  }
}

