//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
//
/**
 * \file PerspCon.h
 * \Declare base class PerspCon. 
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

/** 
 * This class identifies structure of type f(x,z) <= b, lz <= x <= uz, z={0,1} or
 *  f(x) <= b, lz <= x <= uz, z={0,1}, for perspective cut generation. 
 **/

class PerspCon {
public:
  /// Default constructor.
  PerspCon();
  
  ///Constructs from the given problem
  PerspCon(ProblemPtr p, EnvPtr env);

  /// Destructor.
  ~PerspCon();

  /// Checks if the variables are bounded by only one binary variable.
  bool checkAllVars(ConstConstraintPtr cons, ConstVariablePtr binvar,
                    ConstVariablePtr initvar = VariablePtr(), UInt index = 0);

  /// Checks if the variables of nonlinear part are bounded by only one binary variable.
  bool checkNVars(ConstConstraintPtr cons, ConstVariablePtr binvar);
  bool checkLVars(ConstConstraintPtr cons, ConstVariablePtr binvar);

  /// Checks and provide details if a given variable is bounded by binary variable.
  bool checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar, bool pi=1);
  
  /// Checks if all the variables are continuous or at most one binary.
  /// Otherwise, cannot generate perspective cuts.
  bool checkVarTypes(ConstConstraintPtr cons, ConstVariablePtr& binvar, bool indi = 0);
  bool checkVarTypes(ConstConstraintPtr cons);

  /// Checks if a constraint is a Perspective constraint.
  bool evalConstraint(ConstConstraintPtr cons,VariablePtr& binvar);

  /// Generates list of perspective constraints for the original problem.
  void generateList();

  /// Returns a pointer to the vector that contains binary variable of
  /// perspective constraints.
  std::vector<ConstVariablePtr> getConsBinVar() const {return binVar_;}

  /// Returns total number of perspective constraints.
  UInt getNumPersp() const {return cList_.size();}

  /// Returns a pointer to the vector that contains perspective constraints.
  std::vector<ConstConstraintPtr> getPerspCons() const {return cList_;}
  
  /// Returns status of perspective reformulation
  // 1 is PR is carried out, 0 otherwise
  bool getStatus();

  /// Finds binary variables that appear in a linear constraint (involving two
  // terms) with variable var
  void initialBinary(ConstVariablePtr var, VarSetPtr binaries);

  /// Writes out information related to perspective constraints 
  void displayInfo(const std::string);

  private:
  /// Environment.
  EnvPtr env_;
  
  /// For log:
  static const std::string me_;

  ///Pointer to original problem
  ProblemPtr p_;

  ///Log
  LoggerPtr logger_;

  ///Vector of perspective constraint pointers
  std::vector<ConstConstraintPtr> cList_;
  
  //Vector of pointers to binary variables of perspective constraint
  std::vector<ConstVariablePtr> binVar_;
  
  std::vector<std::string> sType_;

  ///Returns lower bounding constraints of continuous variables
  std::vector<std::vector<std::string > > lbc_;

  ///Returns upper bounding constraints of continuous variables
  std::vector<std::vector<std::string > > ubc_;

  std::vector<std::string > initl_;
  std::vector<std::string > l_;
  std::vector<std::string > u_;
  std::vector<std::string > initu_;
  std::string st_;
 
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
