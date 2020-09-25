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
#include <forward_list>
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

typedef struct impliVarsInfo {
  VariablePtr var;
  std::vector<double> fixedVal;
} impliVar;

typedef struct prConsInfo {
  UInt type; // 1 if singleton, 2 if hyperplane, 3 both
  UInt numVarInNonLin; // # of vars in nonlinear part
  VariablePtr binVar;
  ConstraintPtr cons;
  bool binVal, bisect;
  /* bisect = 1, if binVar in absent in cons, or present in only the linear part
   * of cons with negative coefficient, or fixed x with z= 1 lies in the PR
   * region
   */ 
  /// Vector of variables in the nonlinear and linear parts that are fixed to non-zero values
  VariableGroup nNonzeroVar, lNonzeroVar;

} prCons;

typedef struct prObjInfo {
  bool isPR = 0;
  UInt numVarInNonLin; // # of vars in nonlinear part
  // type is always 2 (hyperplane)
  VariablePtr binVar;
  bool binVal, bisect;
  /// Vector of variables in the nonlinear parts that are fixed to non-zero values
  VariableGroup nNonzeroVar;
} prObj;

class PerspCon {
public:
  /// Default constructor.
  PerspCon();
  
  ///Constructs from the given problem.
  PerspCon(EnvPtr env, ProblemPtr p);

  /// Destructor.
  ~PerspCon();

  /// Generates list of constraints amenable to PR.
  void findPRCons();
  
  /// Returns a vector containing constraints amenable to PR.
  std::vector<prCons> getPRCons() const {return prConsVec_;}

  prObj getPRObj() const {return prObj_;}

  bool getStatus();

private:
  /*
   * Checks if the variables in a constraint are bounded by given binary
   * variable binvar.
   */ 
  bool boundBinVar_();

  void checkBinPos_();

  bool isControlled_(std::vector<VariablePtr> binaries);

  /*
   * Checks if variables in the linear part of a constraint are bounded by
   * the given binary variable.
   */ 

  //void checkLVars_(bool *boundsok, VariablePtr var, 
                          //std::unordered_map<UInt,double> *varsInfo);

  /*
   * Checks if variables in the nonlinear part of a constraint are bounded by 
   * a given binary variable.
   */ 

 //void checkNVars_(bool *boundsok, VariablePtr var, 
                          //std::unordered_map<UInt,double> *varsInfo);

  void removeSingleton_();

  /// Checks if a variable is bounded by a given binary variable.
  double checkVarBounds_(VariablePtr var, bool* varbounded);
  
  /*
   * Checks if a binary variable is present in the nonlinear part of the
   * given constraint. 
   */
  void findBinVarsInFunc_(std::vector<VariablePtr>* binaries);

  /// Writes information related to perspective amenable constraints. 
  void displayInfo_();

  void implications_();

  void deriveImpli_(VariablePtr var);

  /// Checks if a constraint is amenable to PR.
  //void evalConstraint(ConstraintPtr cons);
  void detect_();

  /// Returns total number of PR.
  //UInt getNumPersp_() const {return consVec_.size();}

  /* 
   * Returns vector containing binary variables associated with constraints 
   * amenable to PR.
   */ 
  //std::vector<VariablePtr> getPRBinVar_() const {return bVarVec_;}


  
  /// Returns vector containing structure types of constraints amenable to PR.
  //std::vector<UInt> getPRStruct_() const {return sType_;}
   
  /*
   * Returns 1 if problem has at least one constraint amenable to PR,
   * otherwise 0.
   */  

  bool checkNVars_(double *x, VariableGroup &nVarVal);
  bool checkLVars_(double *x, VariableGroup &lVarVal);

  void delGUBList_();

  bool twoTermsFunc_(ConstraintPtr c, VariablePtr var, 
                            std::forward_list<impliVar> *varList, bool z);



  bool multiTermsFunc_(ConstraintPtr c, VariablePtr var, 
                            std::forward_list<impliVar> *varList, 
double val, bool z);

  void fixBinaryVar_(VariablePtr var, bool z);

  bool isBinary_(VariablePtr var);

  void addImplications_(ConstraintPtr c, bool z0);

  bool addImplications_(std::forward_list<impliVar> *varList);

  void populate_(UInt type, VariableGroup nVarVal, VariableGroup lVarVal);

  /* Returns a vector containing values to which variables in the 
   * linear part of the constraints amenable to PR are fixed to.
   */
  //std::vector<VariableGroup > getXLV() const {return lVarFixVal_;}
 
  /* Returns a vector containing values to which variables in the 
   * nonlinear part of the constraints amenable to PR are fixed to.
   */
  //std::vector<VariableGroup > getXNV() const {return nlVarFixVal_;}

  /// Environment.
  EnvPtr env_;
  
  /// Pointer to original problem.
  ProblemPtr p_;

  /// Log
  LoggerPtr logger_;

  double absTol_;

  /// Timer
  Timer *timer_;

  /// For log.
  static const std::string me_;
  
  ConstraintPtr cons_;
  
  VariablePtr bVar_;
  
  UInt sType_;

  bool binVal_;

  bool isObj_;
  
  bool isInFunc_;

  /// Vector of struture types of constraints amenable to PR.
  //std::vector<UInt> sType_;

  /// Vector of perspective constraint pointers
  //std::vector<ConstraintPtr> consVec_;
  
  /* Vector of pointers to binary variables associated with constraints
   * amenable to PR.
   */
  //std::vector<VariablePtr> bVarVec_;

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
  //std::vector<std::unordered_map<UInt,double>> varsInfoVec_;
  
  std::vector<prCons> prConsVec_;

  prObj prObj_;
  
  /// Implication graph using
  std::unordered_map<VariablePtr, std::forward_list<impliVar>> impli0_;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>> impli1_;

  std::forward_list<ConstraintPtr > gubList0_;
  std::forward_list<ConstraintPtr > gubList1_;
  
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator 
    iit_;
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
