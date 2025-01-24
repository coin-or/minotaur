//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

#include <iostream>

#include "MinotaurConfig.h"
#include "Logger.h"

using namespace Minotaur;

Logger::Logger(LogLevel max_level) 
  : maxLevel_(max_level),  nb_(), nout_(&nb_)
{

}


Logger::~Logger() 
{
  // Destructor 
}


std::ostream& Logger::msgStream(LogLevel level) const 
{
  if (level <= maxLevel_) { 
    return std::cout; 
  }
  return nout_;
}


std::ostream& Logger::errStream() const 
{ 
  if (maxLevel_ > LogNone) { 
    return std::cerr; 
  } else {
    return nout_;
  }
}

