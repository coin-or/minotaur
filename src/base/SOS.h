//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2013 The MINOTAUR Team.
//


/**
 * \file SOS.h
 * \brief Declare data structures for an SOS-I and SOS-II constraint
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURSOS_H
#define MINOTAURSOS_H

#include "Types.h"
#include "Variable.h"

namespace Minotaur {


class SOS {
public:
  /// Default constructor
  SOS();

  SOS(Int n, SOSType type, Double *vals, const VarVector &vars, Int priority);

  /// Destroy
  virtual ~SOS();

  Int getNz();

  const Double* getVals();

  VariableConstIterator varsBegin() const { return vars_.begin(); };
  VariableConstIterator varsEnd() const  { return vars_.end(); };

private:
  /// Number of elements in the SOS
  Int n_;

  /// Priority of this SOS over others.
  Int priority_;

  /// \brief Type of SOS
  SOSType type_;
      
  /// Values
  Double * vals_;

  /// Vector of variables.
  VarVector vars_;

};
}
#endif

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
