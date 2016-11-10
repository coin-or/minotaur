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

//struct PerspConStats {
  //UInt totalcons; // Total number of constraints checked.
  //UInt totalpersp; // Total number of perspective constraints obtained.
//};

///  Class for storing information regarding perspective constraints
//class ConsInfo {
  //public:
  /////Default constructor 
    //ConsInfo(){}

    /////Destructor
    //~ConsInfo(){}

    /////Returns perspective constraint
    //void prCons(ConstConstraintPtr c){constrnt_=c;}
    
    /////Returns binary variable
    //void binaryVar(ConstVariablePtr b){binVar_=b;}
    
    /////Returns continuous variables in the perspective constraint
    ////void contiVar(ConstVariablePtr c){contVar_.push_back(c);}
    
    /////Returns lower bounding constraints of continuous variables
    //void lbCons(std::string c){consLb_.push_back(c);}
    
    /////Returns upper bounding constraints of continuous variables
    //void ubCons(std::string c){consUb_.push_back(c);}
    
    /////Returns number of continuos variable in a perspective constraint
    ////int numContVar(){return contVar_.size();}

    /////Below functions are for displaying information
    /////Displays perspective constraint
    //ConstConstraintPtr displayCons(){return constrnt_;}

    /////Displays binary variable
    //ConstVariablePtr displayBinVar(){return binVar_;}

    /////Displays continuous variables in the perspective constraint
    ////ConstVariablePtr displayContVar(int num){return contVar_[num];}

    /////Displays lower bounding constraints of continuous variables
    //std::string displayLbCons(int num){return consLb_[num];}

    /////Displays upper bounding constraints of continuous variables
    //std::string displayUbCons(int num){return consUb_[num];}

  //private:
    
    /////Pointer to perspective constraint
    //ConstConstraintPtr constrnt_;

    /////Pointer to binary variable in the constraint
    //ConstVariablePtr binVar_;

    /////Vector of continuous variables in the constraint
    ////std::vector<ConstVariablePtr> contVar_;

    /////Vector of lower bounding constraints of continuous variables
    //std::vector<std::string > consLb_;

    /////Vector of upper bounding constraints of continuous variables
    //std::vector<std::string > consUb_;

  //}; //end of class ConsInfo

class PerspCon;
typedef boost::shared_ptr<PerspCon> PerspConPtr;
typedef boost::shared_ptr<const PerspCon> ConstPerspConPtr;
//typedef PerspConStats* PerspConStatsPtr;
//typedef PerspConStats const * ConstPerspConStatsPtr;
//typedef std::vector <ConsInfo> * InfoVecPtr;
//typedef std::vector <ConsInfo> const * ConstInfoVecPtr;

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

  /// Checks if a constraint is a Perspective constraint.
  bool evalConstraint(ConstConstraintPtr cons,VariablePtr& binvar);

  /// Checks if the constraint does not have any multi-variable terms
  /// that includes u and x together, i.e. constraint is separable 
  /// such that f(x) + cu <= 0.
  bool separable(ConstConstraintPtr cons, ConstVariablePtr binvar);
  
  //bool separable(NonlinearFunctionPtr nlf);

  /// Checks if all the variables are continuous or at most one binary.
  /// Otherwise, we cannot generate perspective cuts.
  bool checkVarTypes(const FunctionPtr f, ConstVariablePtr& binvar);

  /// Checks if the variables are bounded by only one binary variable.
  //bool checkVarsBounds(const FunctionPtr f, ConstVariablePtr binvar);

  /// Checks if the variables of nonlinear part are bounded by only one binary variable.
  bool checkNVars(const NonlinearFunctionPtr nlf, ConstVariablePtr binvar);

  /// Checks if the variables are bounded by only one binary variable.
  bool checkLVars(ConstConstraintPtr cons, ConstVariablePtr binvar);

  /// Checks if the lower bound of any of the variables is positive
  bool checkVarsLBounds(const FunctionPtr f);

  /// Checks if a given variable is bounded by binary variable.
  bool checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar);

  /// Generate list of perspective constraints for the original problem.
  void generateList();

  /// Generate map of variables that are in the initial variable's constraint
  /// set.
  bool initialBinary(ConstVariablePtr var, VarSetPtr binaries);
  
  /// Get total number of perspective constraints.
  UInt getNumPersp() const {return cList_.size();}

  /// Status of perspective reformulation
  // 1 is PR is carried out, 0 otherwise
  bool getStatus();

  /// Get total number of constraints checked.
  //UInt getNumConsChecked() const {return stats_->totalcons;};

  /// Get the statistics about perspective identification.
  //ConstPerspConStatsPtr getStats() const {return stats_;};

  /// Get a pointer to the vector that contains binary variable of perspective constraints.
  std::vector<ConstVariablePtr> getConsBinVar() const {return binVar_;}

 /// Get a pointer to the vector that contains perspective constraints.
  std::vector<ConstConstraintPtr> getPerspCons() const {return cList_;}

  //ConstInfoVecPtr getPerspCons() const {return perspectiveCons_;};
  
  ///Print out information related to the perspective constraints in a file
  void displayInfo();
  //void printConsInfo(ConsInfo *cinfo);

  private:
  /// Environment.
  EnvPtr env_;
  
  ///Pointer to original problem
  ProblemPtr p_;

  /// Statistics about perspective constraints.
  //PerspConStatsPtr stats_;

  ///Log
  LoggerPtr logger_;

  /// Output file.
  ofstream output_;

  /// Output file name.
  string outfile_;

  //Vector of constraints amenable to perspective reformulation
  //std::vector <ConsInfo> constraintInfo_;

  //Pointer to ConsInfo class
  //ConsInfo *cInfo_;

  //Pointer to vector of perspective constraints
  //InfoVecPtr perspConsInfo_;
  
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
