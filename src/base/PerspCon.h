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
  bool boundBinVar(ConstConstraintPtr cons, VariablePtr& binvar);

  /*
   * Checks if variables in the linear part of a constraint are bounded by
   * the given binary variable.
   */ 
  std::vector<double> checkLVars(ConstConstraintPtr cons,
                                 ConstVariablePtr binvar,
                                 bool* lboundsok);

  /*
   * Checks if variables in the nonlinear part of a constraint are bounded by 
   * a given binary variable.
   */ 
  std::vector<double> checkNVars(ConstConstraintPtr cons,
                                 ConstVariablePtr binvar,
                                 bool* boundsok);


  /// Checks if a variable is bounded by a given binary variable.
  double checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar,
                        bool* varbounded);
  
  /*
   * Checks if a binary variable is present in the nonlinear part of the
   * given constraint. 
   */
  bool checkVarTypes(ConstConstraintPtr cons, VarSetPtr binaries);
  
  /// Writes information related to perspective amenable constraints. 
  void displayInfo();

  /// Checks if a constraint is amenable to PR.
  void evalConstraint(ConstConstraintPtr cons,VariablePtr& binvar);

  /// Generates list of constraints amenable to PR.
  void generateList();


  /// Returns total number of PR.
  UInt getNumPersp() const {return cons_.size();}

  /* 
   * Returns vector containing binary variables associated with constraints 
   * amenable to PR.
   */ 
  std::vector<ConstVariablePtr> getPRBinVar() const {return binvar_;}

  /// Returns a vector containing constraints amenable to PR.
  std::vector<ConstConstraintPtr> getPRCons() const {return cons_;}
  
  /// Returns vector containing structure types of constraints amenable to PR.
  std::vector<char> getPRStruct() const {return sType_;}
   
  /*
   * Returns 1 if problem has at least one constraint amenable to PR,
   * otherwise 0.
   */  
  bool getStatus();

  /* Returns a vector containing values to which variables in the 
   * linear part of the constraints amenable to PR are fixed to.
   */
  std::vector<std::vector<double> > getXLV() const {return lviv_;}
 
  /* Returns a vector containing values to which variables in the 
   * nonlinear part of the constraints amenable to PR are fixed to.
   */
  std::vector<std::vector<double> > getXNV() const {return nviv_;}

  private:
  
  /// Environment.
  EnvPtr env_;
  
  /// Pointer to original problem.
  ProblemPtr p_;

  /// Log
  LoggerPtr logger_;

  /// For log.
  static const std::string me_;

  /// Vector of struture types of constraints amenable to PR.
  std::vector<char> sType_;

  /// Vector of perspective constraint pointers
  std::vector<ConstConstraintPtr> cons_;
  
  /* Vector of pointers to binary variables associated with constraints
   * amenable to PR.
   */
  std::vector<ConstVariablePtr> binvar_;

  /// Vector of pointers to binary variable that are fixed to 1
  std::vector<VariablePtr> fixbvar_;
 
  /* Vector containing values to which variables in the 
   * nonlinear part of a constraint amenable to PR are fixed to.
   */ 
  std::vector<std::vector<double> > nviv_;

  /* Returns a vector containing values to which variables in the 
   * linear part of a constraint amenable to PR are fixed to.
   */
  std::vector<std::vector<double> > lviv_;
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
