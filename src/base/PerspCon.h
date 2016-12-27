//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
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
using std::string;

#include "Problem.h"
#include "Relaxation.h"
#include "Constraint.h"
#include "Variable.h"
#include "Types.h"
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
  bool checkLVars(ConstConstraintPtr cons, ConstVariablePtr binvar);

  /// Checks if the variables of nonlinear part are bounded by only one binary variable.
  bool checkNVars(const NonlinearFunctionPtr nlf, ConstVariablePtr binvar);

  /// Checks if a given variable is bounded by binary variable.
  bool checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar);

  /// Checks if all the variables are continuous or at most one binary.
  /// Otherwise, we cannot generate perspective cuts.
  bool checkVarTypes(ConstConstraintPtr cons, ConstVariablePtr& binvar);

  ///Print out information related to the perspective constraints in a file
  void displayInfo();

  /// Checks if a constraint is a Perspective constraint.
  bool evalConstraint(ConstConstraintPtr cons,VariablePtr& binvar);

  /// Generate list of perspective constraints for the original problem.
  void generateList();

  /// Get a pointer to the vector that contains binary variable of perspective constraints.
  std::vector<ConstVariablePtr> getConsBinVar() const {return binVar_;}

  /// Get total number of perspective constraints.
  UInt getNumPersp() const {return cList_.size();}

  /// Get a pointer to the vector that contains perspective constraints.
  std::vector<ConstConstraintPtr> getPerspCons() const {return cList_;}
  
  /// Status of perspective reformulation
  // 1 is PR is carried out, 0 otherwise
  bool getStatus();

  /// Find binary variables that appear in a linear constraint (involving two
  // terms) with variable var
  void initialBinary(ConstVariablePtr var, VarSetPtr binaries);
  
  private:
  /// Environment.
  EnvPtr env_;
  
  ///Pointer to original problem
  ProblemPtr p_;

  ///Log
  LoggerPtr logger_;

  /// Output file.
  ofstream output_;

  /// Output file name.
  string outfile_;

  ///Vector of perspective constraint pointers
  std::vector<ConstConstraintPtr> cList_;
  
  //Vector of pointers to binary variables of perspective constraint
  std::vector<ConstVariablePtr> binVar_;

  ///Returns lower bounding constraints of continuous variables
  std::vector<std::vector<std::string > > lbc_;

  ///Returns upper bounding constraints of continuous variables
  std::vector<std::vector<std::string > > ubc_;

  std::vector<std::string > l_;
  std::vector<std::string > u_;

/*
  // Uncomment to determine structure of the constraints amenable to PR
   // Vector of structure info of constraints amenable to PR
  std::vector<UInt > sType_;
  UInt s_;
*/
 
}; // end of class PerspCon.


} // end of namespace.

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
