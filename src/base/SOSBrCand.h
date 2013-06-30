// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2013 The MINOTAUR Team.
// 

/**
 * \file SOSBrCand.h
 * \author Ashutosh Mahajan, IIT Bombay
 * \brief Declare the classes SOSBrCand for storing candidates for branching
 * based upon SOS constraints.
 */

#ifndef MINOTAURSOSBRCAND_H
#define MINOTAURSOSBRCAND_H

#include <string>
#include "Types.h"

namespace Minotaur {

class SOS;

/** 
 * \brief Derived class of BrCand, it defines candidates for branching on
 * bounds SOS type 1 and 2 constraints.
 */
class SOSBrCand : public BrCand {
public:
  /// Constructor.
  SOSBrCand();
  SOSBrCand(const SOS* sos, VarVector &left, VarVector &right, Double lsum,
            Double rsum);

  /// Destroy.
  ~SOSBrCand();

  // base class method.
  Double getDDist();

  Double getLSum() const;

  // base class method.
  std::string getName() const;

  Double getRSum() const;

  // base class method.
  Double getUDist();

  VariableConstIterator lVarsBegin() const;

  VariableConstIterator lVarsEnd() const;

  VariableConstIterator rVarsBegin() const;

  VariableConstIterator rVarsEnd() const;


private:
  /// Left branch
  VarVector lvars_;
  
  /// Right branch
  VarVector rvars_;

  Double lsum_;
  Double rsum_;

  const SOS* sos_;
};

typedef boost::shared_ptr<SOSBrCand> SOSBrCandPtr;
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
