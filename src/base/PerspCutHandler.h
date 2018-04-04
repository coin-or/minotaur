//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.h
 * \brief Declare PerspCutHandler class for perspective reformulation (PR).
 * It detects constraints amenable to perspective reformulation and generates
 * perspective cuts whenever needed. 
 * \author Meenarli Sharma, Indian Institute of Technology
 */

#ifndef MINOTAURPERSPCUTHANDLER_H
#define MINOTAURPERSPCUTHANDLER_H

#include "Handler.h"
#include "Problem.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"

namespace Minotaur {

  class PerspCutHandler;
  typedef boost::shared_ptr<PerspCutHandler> PerspCutHandlerPtr;
  typedef boost::shared_ptr<const PerspCutHandler> ConstPerspCutHandlerPtr;
  
  class PerspCutHandler : public Handler {
  public:

  /// Default constructor.
  PerspCutHandler();
  
  /// Constructor.
  PerspCutHandler(EnvPtr env, ProblemPtr problem, std::vector<ConstConstraintPtr> cpr, 
            std::vector<ConstVariablePtr> bpr, std::vector<char> spr,
            std::vector<std::vector<double> > nviv,
            std::vector<std::vector<double> > lviv, bool indi = true);

  /// Destructor.
  ~PerspCutHandler();

  /// Generates perspective cuts.
  bool generateCut(RelaxationPtr rel, ConstSolutionPtr sol, UInt it); 
 
  /// Does nothing.
  virtual Branches getBranches(BrCandPtr, DoubleVector &,
                               RelaxationPtr, SolutionPoolPtr)
    {return Branches();}; 

  // Does nothing.
  virtual void getBranchingCandidates(RelaxationPtr,
                                      const DoubleVector &, ModVector &,
                                      BrVarCandSet &, BrCandVector &,
                                      bool &) {};

  /// Does nothing.
  virtual ModificationPtr getBrMod(BrCandPtr, DoubleVector &,
                                   RelaxationPtr, BranchDirection)
    {return ModificationPtr();};

  /// Returns name of the handler.
  std::string getName() const;

  /// Returns vector of perspective constraint pointers
  std::vector<ConstConstraintPtr> getPRCons() {return cons_;};
  
  /// Returns vector of pointers to binary variables of perspective constraint
  std::vector<ConstVariablePtr> getPRBinVar() {return binvar_;};

  std::vector<char> getPRStruct() {return sType_;};
 
  /// Checks feasibility of constraints amenable to PR at the given solution.
  bool isFeasible(ConstSolutionPtr, RelaxationPtr, bool &, double &);
  //{return true;};

    /// Does nothing.
  SolveStatus presolve(PreModQ *, bool *) {return Finished;};

  /// Does nothing.
  virtual bool presolveNode(RelaxationPtr, NodePtr,
                            SolutionPoolPtr, ModVector &,
                            ModVector &) {return false;};

  /// Generates variables for PR reformulation
  double * prVars(ConstSolutionPtr sol, UInt rnv, UInt itn);


  /// Does nothing.
  void relaxInitFull(RelaxationPtr, bool * ) {};
  
  /// Does nothing.
  void relaxInitInc(RelaxationPtr, bool * ) {};
  
  /// Does nothing.
  void relaxNodeFull(NodePtr, RelaxationPtr, bool * ) {};

  /// Does nothing.
  void relaxNodeInc(NodePtr, RelaxationPtr, bool * ) {};

  /// Separates current solution. 
  void separate(ConstSolutionPtr, NodePtr, RelaxationPtr, CutManager *cutman,
                SolutionPoolPtr, bool *, SeparationStatus * status);
 
  /// Writes statistics.
  void writeStats(std::ostream &out) const;
  
private:
  
  /// Environment.
  EnvPtr env_;
  
  /// Problem for which the handler is created.
  ProblemPtr minlp_;
  
  /// Log.
  LoggerPtr logger_;
 
  /// Tolerance for accepting a new solution value: absolute threshold.
  double solAbsTol_;
  
  /// Tolerance for accepting a new solution value: relative threshold.
  double solRelTol_;
  
  /// Tolerance for checking integrality.
  double intTol_;

  // Number of perspective cuts generated.
  UInt numCuts_;
  
  /// For log:
  static const std::string me_;
  
  /// Vector of perspective constraint pointers
  std::vector<ConstConstraintPtr> cons_;
  
  /// Vector of pointers to binary variables of perspective constraints
  std::vector<ConstVariablePtr> binvar_;

  /// Vector of pointers to structure type of perspective constraints
  std::vector<char> sType_;
  
  /// Vector of pointers to values to which variables in the nonlinear part of
  /// the constraints amenable to PR are fixed to
  std::vector<std::vector<double> > nviv_;
 
 /// Vector of pointers to values to which variables in the linear part of
 // the constraints amenable to PR are fixed to 
  std::vector<std::vector<double> > lviv_;

 
  /// indi_ = 1, if perspective cuts rae to be added at binary variable taking
  /// value 1
  bool indi_;

  };

}

#endif // MINOTAURPERSPCUTHANDLER_H

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
