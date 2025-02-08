//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//


#ifndef MINOTAUREXCEPTION_H
#define MINOTAUREXCEPTION_H

namespace Minotaur {
  class Exception {
  public:
    Exception() { }
    Exception(const Exception&) { }		// Copy constructor
    virtual ~Exception() { }			// Destructor

  private:
    Exception& operator=(const Exception&);	// Copy assignment
  };
}

#endif

