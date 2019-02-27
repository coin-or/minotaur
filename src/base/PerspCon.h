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
using std::ofstream;
#include <string>

#include "Timer.h"
#include "Types.h"
#include "Problem.h"
#include "Variable.h"
#include "Relaxation.h"
#include "Constraint.h"
#include "Environment.h"

namespace Minotaur {

class PerspCon;
typedef boost::shared_ptr<PerspCon> PerspConPtr;
typedef boost::shared_ptr<const PerspCon> ConstPerspConPtr;


/**
 * Handler for convex constraints amenable to perspective
 * reformulation. It considers nonlinear constraints in the form f(x) <= b.
 */
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
  bool boundBinVar(ConstraintPtr cons, VariablePtr& binvar);

  /*
   * Checks if variables in the linear part of a constraint are bounded by
   * the given binary variable.
   */ 

 void checkLVars(ConstraintPtr cons, VariablePtr binvar,
                                 bool* lboundsok,
                                 VariableGroup *lVarFixVal,
                                 VariableGroup *nlVarFixVal);

  /*
   * Checks if variables in the nonlinear part of a constraint are bounded by 
   * a given binary variable.
   */ 
  void checkNVars(ConstraintPtr cons, VariablePtr binvar,
                                 bool* boundsok,
                                 VariableGroup *nlVarFixVal);


  /// Checks if a variable is bounded by a given binary variable.
  double checkVarBounds(VariablePtr var, VariablePtr binvar,
                        bool* varbounded);
  
  /*
   * Checks if a binary variable is present in the nonlinear part of the
   * given constraint. 
   */
  bool checkVarTypes(ConstraintPtr cons, VarSetPtr binaries);
  
  /// Writes information related to perspective amenable constraints. 
  void displayInfo();

  /// Check if a constraint is amenable to PR.
  void evalConstraint(ConstraintPtr cons);

  /// Generate list of constraints amenable to PR.
  void findPRCons();


  /// Return total number of PR.
  UInt getNumPersp() const {return cons_.size();}

  /* 
   * Return vector containing binary variables associated with constraints 
   * amenable to PR.
   */ 
  std::vector<VariablePtr> getPRBinVar() const {return binvar_;}

  /// Returns a vector containing constraints amenable to PR.
  std::vector<ConstraintPtr> getPRCons() const {return cons_;}
  
  /// Return vector containing structure types of constraints amenable to PR.
  std::vector<int> getPRStruct() const {return sType_;}
   
  /*
   * Return 1 if problem has at least one constraint amenable to PR,
   * otherwise 0.
   */  
  bool getStatus();

  /*
   * Return 1 and value to which variable is fixed for z=0, otherwise 0
   */ 
  bool ifFixed(double coeffV, double lb, double ub, double* rBnd, 
                             std::vector<double>* xub, std::vector<double>* xlb);

  void populate (ConstraintPtr cons, VariablePtr binvar, VariableGroup nlVarFixVal,
               VariableGroup lVarFixVal);
  /* Returns a vector containing values to which variables in the 
   * linear part of the constraints amenable to PR are fixed to.
   */
  std::vector<VariableGroup > getXLV() const {return lVarFixVal_;}
 
  /* Returns a vector containing values to which variables in the 
   * nonlinear part of the constraints amenable to PR are fixed to.
   */
  std::vector<VariableGroup > getXNV() const {return nlVarFixVal_;}

  private:
  
  /// Environment.
  EnvPtr env_;
  
  /// Pointer to original problem.
  ProblemPtr p_;

  /// Log
  LoggerPtr logger_;

  /// Timer
  //Timer *timer_;

  /// For log.
  static const std::string me_;

  /// Vector of struture types of constraints amenable to PR.
  std::vector<int> sType_;

  /// Vector of perspective constraint pointers
  std::vector<ConstraintPtr> cons_;
  
  /* Vector of pointers to binary variables associated with constraints
   * amenable to PR.
   */
  std::vector<VariablePtr> binvar_;

  /// Vector of pointers to binary variable that are fixed to 1
  //std::vector<VariablePtr> fixbvar_;
 
  /* Vector containing values to which variables in the 
   * nonlinear part of a constraint amenable to PR are fixed to.
   */ 
  std::vector<VariableGroup> nlVarFixVal_;

  /* Returns a vector containing values to which variables in the 
   * linear part (and not in nonlinear part) of a constraint amenable to PR are fixed to.
   */
  std::vector<VariableGroup> lVarFixVal_;
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
