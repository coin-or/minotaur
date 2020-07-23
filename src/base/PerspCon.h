//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
//
/**
 * \file PerspCon.h
 * \brief Declare a base class PerspCon for identifying structures amenable 
 * to perspective reformulation (PR) (mention the paper) 
 * \author Meenarli Sharma, Indian Institute of Technology Bombay 
*/

#ifndef MINOTAURPERSPCON_H
#define MINOTAURPERSPCON_H

#include <map>
#include <fstream>
#include <string>
#include <unordered_map> 

#include "Timer.h"
#include "Types.h"
#include "Problem.h"
#include "Variable.h"
#include "Relaxation.h"
#include "Constraint.h"
#include "Environment.h"

namespace Minotaur {

class PerspCon;
typedef PerspCon* PerspConPtr;
typedef const PerspCon* ConstPerspConPtr;


class PerspCon {
public:
  /// Default constructor.
  PerspCon();
  
  ///Constructs from the given problem.
  PerspCon(ProblemPtr p, EnvPtr env);

  /// Destructor.
  ~PerspCon();

  /*
   * Checks if the variables in a constraint are bounded by given binary
   * variable binvar.
   */ 
  bool boundBinVar();

  /*
   * Checks if variables in the linear part of a constraint are bounded by
   * the given binary variable.
   */ 

  void checkLVars(bool *boundsok, VariablePtr var, 
                          std::unordered_map<UInt,double> *varsInfo);

  /*
   * Checks if variables in the nonlinear part of a constraint are bounded by 
   * a given binary variable.
   */ 

 void checkNVars(bool *boundsok, VariablePtr var, 
                          std::unordered_map<UInt,double> *varsInfo);

 void searchVarIdx(VariablePtr var, bool *boundsok,  
                          std::unordered_map<UInt,double> *varsInfo);

  /// Checks if a variable is bounded by a given binary variable.
  double checkVarBounds(VariablePtr var,
                        bool* varbounded);
  
  /*
   * Checks if a binary variable is present in the nonlinear part of the
   * given constraint. 
   */
  bool findBinVarsInCons(std::vector<VariablePtr>* binaries);

  /// Writes information related to perspective amenable constraints. 
  void displayInfo();

  /// Checks if a constraint is amenable to PR.
  //void evalConstraint(ConstraintPtr cons);
  void evalConstraint();

  /// Generates list of constraints amenable to PR.
  void findPRCons();


  /// Returns total number of PR.
  UInt getNumPersp() const {return consVec_.size();}

  /* 
   * Returns vector containing binary variables associated with constraints 
   * amenable to PR.
   */ 
  std::vector<VariablePtr> getPRBinVar() const {return bVarVec_;}

  /// Returns a vector containing constraints amenable to PR.
  std::vector<ConstraintPtr> getPRCons() const {return consVec_;}
  
  /// Returns vector containing structure types of constraints amenable to PR.
  std::vector<UInt> getPRStruct() const {return sType_;}
   
  /*
   * Returns 1 if problem has at least one constraint amenable to PR,
   * otherwise 0.
   */  
  bool getStatus();



  void populate(std::unordered_map<UInt,double> varsInfo);
  /* Returns a vector containing values to which variables in the 
   * linear part of the constraints amenable to PR are fixed to.
   */
  //std::vector<VariableGroup > getXLV() const {return lVarFixVal_;}
 
  /* Returns a vector containing values to which variables in the 
   * nonlinear part of the constraints amenable to PR are fixed to.
   */
  //std::vector<VariableGroup > getXNV() const {return nlVarFixVal_;}

  private:
  
  /// Environment.
  EnvPtr env_;
  
  /// Pointer to original problem.
  ProblemPtr p_;

  /// Log
  LoggerPtr logger_;

  double absTol_;

  /// Timer
  //Timer *timer_;

  /// For log.
  static const std::string me_;
  
  ConstraintPtr cons_;
  
  VariablePtr bVar_;

  /// Vector of struture types of constraints amenable to PR.
  std::vector<UInt> sType_;

  /// Vector of perspective constraint pointers
  std::vector<ConstraintPtr> consVec_;
  
  /* Vector of pointers to binary variables associated with constraints
   * amenable to PR.
   */
  std::vector<VariablePtr> bVarVec_;

  /// Vector of pointers to binary variable that are fixed to 1
  //std::vector<VariablePtr> fixbvar_;
 
  /* Vector containing values to which variables in the 
   * nonlinear part of a constraint amenable to PR are fixed to.
   */ 
  //std::vector<VariableGroup> nlVarFixVal_;

  /* Returns a vector containing values to which variables in the 
   * linear part of a constraint amenable to PR are fixed to.
   */
  //std::vector<VariableGroup> lVarFixVal_;
  std::vector<std::unordered_map<UInt,double>> varsInfoVec_;
}; 

}

#endif // MINOTAURPERSPCON_H

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
